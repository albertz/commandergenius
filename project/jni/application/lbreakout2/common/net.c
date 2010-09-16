#include "net.h"
#include "../client/lbreakout.h"
#include <SDL.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

/* Basic layer for network communication. A single UDP socket
 * is bound to a port. Received packages are stored in one
 * global buffer.
 * Socket addresses are wrapped in a struct called NetAddr to
 * which raw data may be sent as unreliable datagrams.
 *
 * Error/message stream: sys_printf()
 */

int (*sys_printf)(const char *format, ...) = printf;

/* ********** UDP SOCKET ********** */

#ifdef SDL_NET_ENABLED
UDPsocket SDL_socket = 0;
UDPpacket *SDL_packet = 0;
#else
int	net_socket = -1;
#endif
NetAddr net_local_addr;
NetAddr	net_sender_addr;
char	net_buffer[MAX_MSG_SIZE + PACKET_HEADER_SIZE];
int 	net_buffer_cur_size = 0;

int	net_show_drops = 1;
#ifdef NET_DEBUG_MSG
int	net_show_packets = 1;
#else
int	net_show_packets = 0;
#endif

#ifdef NETWORK_ENABLED
#ifndef SDL_NET_ENABLED
static void sockaddr_to_netaddr( struct sockaddr *sa, NetAddr *addr )
{
	struct sockaddr_in *sin;
	addr->sa = *sa;
	/* if AF_INET get ip */
	if ( sa->sa_family == AF_INET ) {
		sin = (struct sockaddr_in*)sa;
		*(int *)&addr->inet_ip = *(int *)&sin->sin_addr; /* wow */
	}
}

static void netaddr_to_sockaddr( NetAddr *addr, struct sockaddr *sa  )
{
	*sa = addr->sa;
}

/* resolve name but leave port 0 */
static bool resolve_hostname( char *host, struct sockaddr *sa, int *sa_len )
{
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;
	struct hostent *hostlist;
	
	/* resolve the host's address via DNS lookup */
	if ( ( hostlist = gethostbyname( host ) ) == 0 ) {
		sys_printf( _("unable to resolve %s: %s\n"), host, strerror( errno) );
		return 0;
	}
	
	/* put host address to sa struct. simply use the first resolved address */
	*sa_len = sizeof(*sa);
	memset( sa, 0, *sa_len );
	switch ( hostlist->h_addrtype ) {
		case AF_INET:
			sin = (struct sockaddr_in*)sa;
			sin->sin_family = AF_INET;
			memcpy( &sin->sin_addr, hostlist->h_addr_list[0], hostlist->h_length );
			break;
		case AF_INET6:
			sin6 = (struct sockaddr_in6*)sa;
			sin6->sin6_family = AF_INET6;
			memcpy( &sin6->sin6_addr, hostlist->h_addr_list[0], hostlist->h_length );
			break;
		default:
			sys_printf( "unable to resolve: address family %i not supported\n",
				    hostlist->h_addrtype );
			return 0;
	}

	return 1;
}

static void net_get_local_addr( int port )
{
	char			buf[MAXHOSTNAMELEN];
	struct sockaddr_in	sa;
	int			sa_len;

	gethostname(buf, MAXHOSTNAMELEN);
	buf[MAXHOSTNAMELEN-1] = 0;

	resolve_hostname( buf, (struct sockaddr*)&sa, &sa_len ); /* port is missing */
	sa.sin_port = htons((short)port);
	
	sockaddr_to_netaddr( (struct sockaddr *)&sa, &net_local_addr );
}
#endif
#endif

/* open UDP socket */
bool net_init( int port )
{
#ifdef NETWORK_ENABLED
#ifdef SDL_NET_ENABLED
    if(SDLNet_Init()==-1) {
        sys_printf("SDLNet_Init: %s\n", SDLNet_GetError());
        return 0;
    }
    SDL_socket=SDLNet_UDP_Open(port);
    if(!SDL_socket) {
        sys_printf("SDLNet_UDP_Open: %s\n", SDLNet_GetError());
        return 0;
    }
    SDL_packet = SDLNet_AllocPacket(MAX_MSG_SIZE + PACKET_HEADER_SIZE);
    if(SDL_packet==0) {
        sys_printf("cannot allocate packet: out of memory\n" );
        return 0;
    }
    /* net_local_addr is unset */
    net_local_addr.SDL_address.host = 0;
    net_local_addr.SDL_address.port = 0;
    return 1;
#else
    struct sockaddr_in sa;
    int sa_len;
    int fcntl_args, i, new_args;

    if ( (net_socket = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 )
        sys_printf( "couldn't create socket: %s\n", strerror(errno) );
    else
        if ( fcntl( net_socket, F_GETFL, &fcntl_args) < 0 )
            sys_printf( "couldn't get fcntl args: %s\n", strerror(errno) );
        else
        {
            new_args = O_NONBLOCK;
            /* no idea, no idea... fcntl seems to work differently since kernel 2.6 
            if (fcntl_args&O_APPEND) new_args|=O_APPEND;
            if (fcntl_args&O_ASYNC)  new_args|=O_ASYNC;
            if (fcntl_args&O_DIRECT) new_args|=O_DIRECT;*/
            if ( fcntl( net_socket, F_SETFL, new_args ) < 0 )
                sys_printf( "couldn't set fcntl args: %s\n", strerror(errno) );
            else {
                sa_len = sizeof(sa);
                memset( &sa, 0, sa_len );
                sa.sin_family = AF_INET;
                sa.sin_port = htons((short)port);
                sa.sin_addr.s_addr = htonl(INADDR_ANY); /* all interfaces */

                i = 10; /* try ten successive ports */
                while ( bind( net_socket, (struct sockaddr*)&sa, sa_len ) < 0 ) {
                    sys_printf( "binding to port %i failed: %s\n", port, strerror(errno) );
                    sa.sin_port = htons((short)port++);
                    if ( --i == 0 ) {
                        close( net_socket );
                        return 0;
                    }
                }

                net_get_local_addr( port );
                sys_printf( _("UDP socket bound to %s:%i\n"), 
                        net_addr_to_string( &net_local_addr ), port ); 

                return 1;
            }
        }
    return 0;
#endif
#else
    return 0;
#endif
}

void net_shutdown( void )
{
#ifdef NETWORK_ENABLED
#ifdef SDL_NET_ENABLED
    SDLNet_UDP_Close(SDL_socket); SDL_socket = 0;
    SDLNet_FreePacket(SDL_packet); SDL_packet = 0;
    SDLNet_Quit();
#else
    close( net_socket );
    sys_printf( "UDP socket closed\n" );
#endif
#endif
}

void net_send_packet( NetAddr *to, int len, void *data )
{
#ifdef NETWORK_ENABLED
#ifdef SDL_NET_ENABLED
    SDL_packet->channel = -1;
    memcpy( SDL_packet->data, data, len );
    SDL_packet->len = len;
    SDL_packet->address = to->SDL_address;
    SDLNet_UDP_Send(SDL_socket,-1,SDL_packet);
#else
    struct sockaddr	sa;

    /* empty packets are not send */
    if ( len == 0 ) {
        sys_printf( "net_send_packet: null length packet\n" );
        return;
    }

    netaddr_to_sockaddr( to, &sa );

    if ( sendto( net_socket, data, len, 0, &sa, sizeof(sa) ) < 0 ) {
        if (errno == EWOULDBLOCK)
            return;
        if (errno == ECONNREFUSED)
            return;
        sys_printf( "net_send_packet: %s\n", strerror(errno) );
    }
#endif
#endif
}

bool net_recv_packet( void )
{
#ifdef NETWORK_ENABLED
    int 		ret;
#ifdef SDL_NET_ENABLED
    ret = SDLNet_UDP_Recv(SDL_socket, SDL_packet);
    if ( ret == -1 ) {
        sys_printf("SDLNet_UDP_Recv: %s\n", SDLNet_GetError());
        return 0;
    }
    if ( ret == 1 ) {
        memcpy( net_buffer, SDL_packet->data, SDL_packet->len );
        net_buffer_cur_size =  SDL_packet->len;
        net_sender_addr.SDL_address = SDL_packet->address;
        return 1;
    }
    net_buffer_cur_size = 0;
    return 0;
#else
    struct sockaddr	sa;
    int		sa_len = sizeof(sa);

    ret = recvfrom( net_socket, 
            net_buffer, sizeof(net_buffer), 
            0, 
            &sa, &sa_len);
    if (ret == -1) {
        if (errno == EWOULDBLOCK)
            return 0;
        if (errno == ECONNREFUSED)
            return 0;
        sys_printf( "net_recv_packet: %s\n", strerror(errno));
        return 0;
    }

    net_buffer_cur_size = ret;

    sockaddr_to_netaddr( &sa, &net_sender_addr );

    return (ret>0);
#endif
#else
    return 0;
#endif
}

/* if host contains trailing :xxxxx 'port' is overwritten */
bool net_build_addr( NetAddr *addr, char *host, int port )
{
#ifdef NETWORK_ENABLED
    char                *ptr;
#ifndef SDL_NET_ENABLED
    struct sockaddr     sa;
    int                 sa_len;
#endif
    if ( (ptr = strchr( host, ':' )) ) {
        *ptr = 0;
        port = atoi( ptr+1 );
    }

#ifdef SDL_NET_ENABLED
    if ( SDLNet_ResolveHost(&addr->SDL_address, host, port ) == -1 )
    {
        sys_printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
        return 0;
    }
    return 1;
#else
    memset( addr, 0, sizeof( NetAddr ) );
    if ( resolve_hostname( host, &sa, &sa_len ) ) {
        switch ( sa.sa_family ) {
            case AF_INET:
                ((struct sockaddr_in*)&sa)->sin_port = htons((short)port);
                break;
            case AF_INET6:
                ((struct sockaddr_in6*)&sa)->sin6_port = htons((short)port);
                break;
            default:
                /* not reached as resolve_hostname allows the same types */
                return 0;
        }
        sockaddr_to_netaddr( &sa, addr );
        return 1;
    }
    return 0;
#endif
#else
    return 0;
#endif
}

/* compare host and port */
bool net_compare_addr( NetAddr *a1, NetAddr *a2 )
{
#ifdef NETWORK_ENABLED
#ifdef SDL_NET_ENABLED
    if ( a1->SDL_address.host == a2->SDL_address.host )
    if ( a1->SDL_address.port == a2->SDL_address.port )
        return 1;
    return 0;
#else
    /* by now only AF_INET addresses may be compared */
    if ( a1->sa.sa_family == AF_INET && a2->sa.sa_family == AF_INET ) {
        if ( a1->inet_ip[0] == a2->inet_ip[0] )
            if ( a1->inet_ip[1] == a2->inet_ip[1] )
                if ( a1->inet_ip[2] == a2->inet_ip[2] )
                    if ( a1->inet_ip[3] == a2->inet_ip[3] )
                        if ( ((struct sockaddr_in*)&a1->sa)->sin_port == 
                                ((struct sockaddr_in*)&a2->sa)->sin_port )
                            return 1;
        return 0;
    }

    sys_printf( "only AF_INET addresses may be compared yet\n" );
    return 0;
#endif
#else
    return 0;
#endif
}

char *net_addr_to_string( NetAddr *_addr ) /* not allocated but static */
{
#ifdef NETWORK_ENABLED
#ifdef SDL_NET_ENABLED
    int ip[4];
    static char host_ip[64];
    char *host;
    if(!(host=SDLNet_ResolveIP(&_addr->SDL_address)))
    {
        //printf("SDLNet_ResolveIP: %s\n", SDLNet_GetError());
        ip[0] = _addr->SDL_address.host & 0xff;
        ip[1] = (_addr->SDL_address.host>>8) & 0xff;
        ip[2] = (_addr->SDL_address.host>>16) & 0xff;
        ip[3] = (_addr->SDL_address.host>>24) & 0xff;
        snprintf( host_ip, 64, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3] );
        return host_ip;
    }
    return host;
#else
    static char str[64];
    int len = 64;
    struct sockaddr *addr = &_addr->sa;
    struct sockaddr_in *sin;
    struct sockaddr_in6 *sin6;
    /*struct sockaddr_un *sun;*/

    str[len-1] = 0;
    snprintf( str, len, "undefined" );

    switch ( addr->sa_family ) {
        case AF_INET:
            sin = (struct sockaddr_in*)addr;
            if ( !inet_ntop( AF_INET, &sin->sin_addr, str, len ) ) {
                sys_printf( "inet_ntop failed: %s\n", strerror( errno ) );
            }
            break;
        case AF_INET6:
            sin6 = (struct sockaddr_in6*)addr;
            if ( !inet_ntop( AF_INET6, &sin6->sin6_addr, str, len ) ) {
                sys_printf( "inet_ntop failed: %s\n", strerror( errno ) );
            }
            break;
        case AF_UNIX:
            /*sun = (struct sockaddr_un *)addr;
              if( sun->sun_path[0] == 0 ) {
              snprintf( str, len, "unknown" );
              sys_printf( "net_query_ip: unknown unix path\n" );
              }   
              else
              snprintf( str, sun->sun_path, len );*/
            break;
        default:
            snprintf( str, len, "unknown" );
            break;
    }

    return str;
#endif
#else
    return 0;
#endif
}

/* ********** MESSAGE ********** */

char *msg_buf = 0;
int  *msg_buf_cur_size = 0;
int  msg_buf_max_size = 0;
int  msg_write_overflow = 0;

#ifdef NET_DEBUG_MSG
static void msg_print_raw( int len, char *buf )
{
	int i;
	for ( i = 0; i < len; i++ )
		printf( "%02x ", (unsigned char) buf[i] );
	printf( "\n" );
}
#endif

static char* msg_get_writeable_space( int size )
{
	if ( *msg_buf_cur_size + size > msg_buf_max_size ) {
		msg_write_overflow = 1;
		*msg_buf_cur_size = 0;
	}
	return msg_buf + *msg_buf_cur_size;
}

void msg_begin_writing( char *buf, int *cur_size, int max_size )
{
	msg_buf = buf; 
	msg_buf_cur_size = cur_size;
	msg_buf_max_size = max_size;
	*msg_buf_cur_size = 0;
	msg_write_overflow = 0;
}
void msg_write_int8  ( int c )
{
	unsigned char *ptr = msg_get_writeable_space( 1 );
	ptr[0] = (char)c;
	*msg_buf_cur_size += 1;
}
void msg_write_int16 ( int s )
{
	unsigned char *ptr = msg_get_writeable_space( 2 );
	ptr[0] = s & 0xff;
	ptr[1] = (s>>8) & 0xff;
	*msg_buf_cur_size += 2;
}
void msg_write_int32 ( int i )
{
	unsigned char *ptr = msg_get_writeable_space( 4 );
	ptr[0] = i & 0xff;
	ptr[1] = (i>>8) & 0xff;
	ptr[2] = (i>>16) & 0xff;
	ptr[3] = (i>>24) & 0xff;
	*msg_buf_cur_size += 4;
}
void msg_write_string( char *str )
{
	char *ptr = msg_get_writeable_space( strlen(str)+1 );
	if ( strlen(str)+1 > msg_buf_max_size ) return; /* would cause segfault */
	strcpy( ptr, str );
	*msg_buf_cur_size += strlen(str)+1;
}
void msg_printf      ( char *format, ... )
{
	char buf[MAX_MSG_SIZE];
	va_list args;
	
	va_start( args, format );
	vsnprintf( buf, MAX_MSG_SIZE, format, args ); buf[MAX_MSG_SIZE-1] = 0;
	va_end( args );
	msg_write_string( buf );
}
void msg_write       ( int len, void *data )
{
	unsigned char *ptr = msg_get_writeable_space( len );
	if ( len > msg_buf_max_size ) return; /* would cause segfault */
	memcpy( ptr, data, len );
	*msg_buf_cur_size += len;
}
int  msg_get_max_size()
{
	return msg_buf_max_size;
}
bool msg_write_failed()
{
	return msg_write_overflow;
}

int msg_read_exceeded = 0;
int msg_read_pos = 0;

static char* msg_get_readable_space( int size )
{
	if ( msg_read_pos + size > net_buffer_cur_size ) {
		msg_read_exceeded = 1;
		msg_read_pos = 0;
	}
	return net_buffer + msg_read_pos;
	
}

bool  msg_is_connectionless()
{
	return ( *(int*)net_buffer == 0 );
}
void  msg_begin_reading()
{
	msg_read_exceeded = 0;
	msg_read_pos = 0;
}
void  msg_begin_connectionless_reading()
{
	msg_read_exceeded = 0;
	msg_read_pos = PACKET_HEADER_SIZE;
}
int   msg_read_int8 ( void )
{
	unsigned char *ptr = msg_get_readable_space( 1 );
	msg_read_pos += 1;
	return ptr[0];
}
int   msg_read_int16( void )
{
	unsigned char *ptr = msg_get_readable_space( 2 );
	msg_read_pos += 2;
	return ptr[0] + (ptr[1]<<8);
}
int   msg_read_int32( void )
{
	unsigned char *ptr = msg_get_readable_space( 4 );
	msg_read_pos += 4;
	return ptr[0] + (ptr[1]<<8) + (ptr[2]<<16) + (ptr[3]<<24);
}
char* msg_read_string( void )
{
	static char buf[MAX_MSG_SIZE]; /* can't be bigger including trailing \0 */
	char c;
	int i = 0;

	while ( 1 ) {
		c = msg_read_int8();
		buf[i++] = c;
		if ( i == MAX_MSG_SIZE ) break; /* to be sure */
		if ( c == 0 ) break;
	}
	buf[MAX_MSG_SIZE-1] = 0;

	return buf;
}
void  msg_read( int len, char *buf )
{
	unsigned char *ptr = msg_get_readable_space( len );
	msg_read_pos += len;
	memcpy( buf, ptr, len );
}

bool  msg_read_failed( void )
{
	return msg_read_exceeded;
}

/* ********** TRANSMISSION ********** */

/* transmit a connectionless message */
void net_transmit_connectionless( NetAddr *addr, int len, char *data )
{
	int cur_size;
	char packet[PACKET_HEADER_SIZE + MAX_MSG_SIZE];

	memset( packet, 0, PACKET_HEADER_SIZE ); /* connectionless header */
	msg_begin_writing( packet + PACKET_HEADER_SIZE, &cur_size, MAX_MSG_SIZE );
	msg_write( len, data );
	if ( msg_write_failed() ) return;
			
	net_send_packet( addr, PACKET_HEADER_SIZE + cur_size, packet );
	
	if ( net_show_packets )
		sys_printf( "--> %s: connectionless size=%i\n",
			net_addr_to_string( addr ),
			cur_size + PACKET_HEADER_SIZE );
}

void socket_init( NetSocket *sock, NetAddr *addr )
{
    memset( sock, 0, sizeof( NetSocket ) );

    sock->remote_addr = *addr;
    sock->outgoing_id = 1; /* as id 0 means connectionless */
    sock->idle_since = time(0);

    //sys_printf( "%s: socket initiated\n", net_addr_to_string( addr ) );
}

void socket_reinit( NetSocket *socket ) /* with same address */
{
	NetAddr addr = socket->remote_addr;
	socket_init( socket, &addr );
}

void socket_print_stats( NetSocket *sock )
{
	sys_printf ( "%s: STATISTICS\n  drops: %i(%i packets)\n  %i good packets\n",
		net_addr_to_string( &sock->remote_addr ), 
		sock->drop_count, sock->dropped_packet_count, 
		sock->good_packet_count );
}

/* transmit CODE_RED/BLUE message and re-transmit dropped CODE_RED messages.
 * length 0 messages are not transmitted. re-transmission of CODE_RED messages
 * is handled in any case */
void socket_transmit( NetSocket *sock, int code, int len, char *data )
{
	char packet[PACKET_HEADER_SIZE + MAX_MSG_SIZE];
	int is_code_red = 0, cur_size = 0;

	/* do not use sockets with fatal error */
	if ( sock->fatal_error ) return;
	
	/* add code red message to buffer */
	if ( code == CODE_RED ) {
		if ( sock->code_red_buf_cur_size + len > MAX_MSG_SIZE ) {
			sock->fatal_error = 1; /* overflowed */
			return;
		}
		memcpy( sock->code_red_buf + sock->code_red_buf_cur_size, data, len );
		sock->code_red_buf_cur_size += len;
		/* do not send the message if waiting for a receipt */
		if ( sock->outgoing_code_red > 0 && !sock->retransmit_code_red_buf )
			return;
		else
			is_code_red = 1;
	}

	if ( sock->retransmit_code_red_buf )
		is_code_red = 1;
	
	msg_begin_writing( packet, &cur_size, PACKET_HEADER_SIZE + MAX_MSG_SIZE );
	msg_write_int32( sock->outgoing_id | (is_code_red<<31) );
	msg_write_int32( sock->incoming_code_red );

	if ( is_code_red )
		sock->outgoing_code_red = sock->outgoing_id;
	
	sock->outgoing_id++;

	/* if code red buffer was not received resend it and clear the flag.
	 * if not waiting for a receipt (thus an initial code red message)
	 * send it as well */
	if ( is_code_red ) {
		msg_write( sock->code_red_buf_cur_size, sock->code_red_buf );
		sock->retransmit_code_red_buf = 0;
	}
	
	/* add code blue packet if space is left */
	if ( code == CODE_BLUE && cur_size + len <= msg_get_max_size() )
		msg_write( len, data );

	/* no data? */
	if ( cur_size == PACKET_HEADER_SIZE ) return;
	
	/* send packet */
	net_send_packet( &sock->remote_addr, cur_size, packet );
#ifdef NET_DEBUG_MSG
	printf( "OUT: "); msg_print_raw( cur_size, packet );
#endif
	
	if ( net_show_packets )
		sys_printf( "--> %s: out=%i(%s) size=%i in=%i(RED:%i)\n",
			net_addr_to_string( &sock->remote_addr ),
			sock->outgoing_id-1, is_code_red?"RED":"BLUE",
			cur_size,
			sock->incoming_id, sock->incoming_code_red );
}

/* update socker by net_packet header if net_sender_addr is sock.remote_addr 
 * and set read pointer to packet payload. if false is returned packet is
 * not parsed. */
bool socket_process_header( NetSocket *sock )
{
	int id, is_code_red, last_ack_code_red, drops = 0;
	
	/* do not process fatal socks */
	if ( sock->fatal_error ) return 0;
	
	/* correct sock? */
	if ( !net_compare_addr( &net_sender_addr, &sock->remote_addr ) )
		return 0;

#ifdef NET_DEBUG_MSG
	printf( " IN: " ); msg_print_raw( net_buffer_cur_size, net_buffer );
#endif

	/* read header */
	msg_begin_reading();
	id = msg_read_int32(); /* new incoming id, the difference to sock->incoming_id is 
				  the number of dropped packets */
	is_code_red = id >> 31; /* highest bit tells wether code red packet */
	id &= ~(1<<31);
	last_ack_code_red = msg_read_int32(); /* last code red packet remote received */

	/* duplicated packets are not handled */
	if ( id <= sock->incoming_id ) {
		if ( net_show_drops )
			sys_printf( "%s: out of order packet %i at %i\n", 
				net_addr_to_string( &sock->remote_addr ),
				id, sock->incoming_id );
		return 0;
	}
	
	/* despite dropped packets this packet is handled */
	if ( id > sock->incoming_id+1 ) {
		drops = id - (sock->incoming_id+1);
		sock->dropped_packet_count += drops;
		sock->drop_count++;
		if ( net_show_drops )
			sys_printf( "%s: dropped %i packets at %i\n", 
				net_addr_to_string( &sock->remote_addr ),
				drops, sock->incoming_id+1 );
	}
	sock->good_packet_count++;

	sock->incoming_id = id; /* last incoming packet (no matter wether red or blue) */
	if ( is_code_red )
		sock->incoming_code_red = id; /* last code red packet socket received */
		
	if ( sock->outgoing_code_red ) {
		if ( sock->outgoing_code_red == last_ack_code_red ) {
			/* was received so we can clear code_red buffer */
			sock->outgoing_code_red = 0;
			sock->code_red_buf_cur_size = 0;
		}
		else
		if ( sock->outgoing_code_red > last_ack_code_red ) {
			/* resend code_red buffer as it was dropped */
			sock->retransmit_code_red_buf = 1;
		}
		else {
			/* fatal error occured as remote received a newer code red packet
			 * although we had no receipt for the current one */
			sock->fatal_error = 1;
		}
	}

	sock->idle_since = time(0); /* reset idle time stamp */

	/* if packet is empty mark it as unreadable */
	if ( msg_read_pos == net_buffer_cur_size ) 
		return 0;

	return 1;
}

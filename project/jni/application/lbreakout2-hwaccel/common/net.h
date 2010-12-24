#ifndef __NET_H
#define __NET_H

#ifdef NETWORK_ENABLED
#ifdef SDL_NET_ENABLED
  #include <SDL_net.h>
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <sys/param.h>
  #include <sys/time.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <netinet/tcp.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <errno.h>
  #include <arpa/inet.h>
  #include <sys/poll.h>
  #include <signal.h>
#endif
#endif

#ifndef bool
  #define bool int
#endif

typedef struct {
#ifdef NETWORK_ENABLED
#ifdef SDL_NET_ENABLED
    IPaddress           SDL_address;
#else
    struct sockaddr     sa;
    unsigned char       inet_ip[4]; /* AF_INET dotted ip */
#endif
#endif
} NetAddr;

#define MAX_MSG_SIZE 1016
#define PACKET_HEADER_SIZE 8

typedef struct {
	NetAddr		remote_addr;
	int		fatal_error;	/* code red buffer has been flooded */
	
	int		idle_since; 	/* global time stamp when last packet was received (secs) */
	
	int		drop_count; 		/* number of drops */
	int		dropped_packet_count;	/* number of dropped packets */	
	int		good_packet_count;	/* number of successfully processed packets */
	
	int		incoming_id;	/* last incoming remote outgoing_id */
	int		outgoing_id;	/* packet id increased every sent */
	int		incoming_code_red; /* id of last incoming code red packet. is sent
					      in the header to let remote know if any code
					      red packets were dropped */
	int		outgoing_code_red; /* last outgoing_id of code_red packet if socket
					   is still waiting for reception acknowledgment. if
					   packet was delivered successfully this is cleared 
					   to 0 as well as code_red_buf */
	int		retransmit_code_red_buf; /* retransmit buffer as net_process_header
						    has received an incoming_code_red less
						    than outgoing_code_red */
	
	char		code_red_buf[MAX_MSG_SIZE];
	char		code_red_buf_cur_size;
} NetSocket;

extern NetAddr 		net_local_addr;
extern NetAddr		net_sender_addr;

bool net_init( int port );
void net_shutdown( void );
void net_send_packet( NetAddr *to, int len, void *data );
bool net_recv_packet( void ); /* into net_packet from net_sender_addr */

bool net_build_addr( NetAddr *addr, char *host, int port );
bool net_compare_addr( NetAddr *a1, NetAddr *a2 );
char *net_addr_to_string( NetAddr *addr ); /* not allocated but static */

void msg_begin_writing( char *buf, int *cur_size, int max_size );
void msg_write_int8  ( int c );
void msg_write_int16 ( int s );
void msg_write_int32 ( int i );
void msg_write_string( char *str );
void msg_printf      ( char *format, ... );
void msg_write       ( int len, void *data );
int  msg_get_max_size();
bool msg_write_failed(); /* check wether overflow occured */

bool  msg_is_connectionless(); 
void  msg_begin_reading(); /* of net_packet, starts at header */
void  msg_begin_connectionless_reading(); /* of net_packet, starts after header as useless data */
int   msg_read_int8 ( void );
int   msg_read_int16( void );
int   msg_read_int32( void );
char* msg_read_string( void );
void  msg_read( int len, char *buf );
bool  msg_read_failed(); /* check wether data couldn't be extracted */

void socket_init( NetSocket *sock, NetAddr *addr );
void socket_reinit( NetSocket *socket ); /* with same address */
void socket_print_stats( NetSocket *sock );

/* transmit a connectionless message */
void net_transmit_connectionless( NetAddr *addr, int len, char *data );

/* transmit CODE_RED/BLUE message and re-transmit dropped CODE_RED messages.
 * length 0 messages are not transmitted but re-transmission of CODE_RED messages
 * is handled in any case */
#define CODE_BLUE 0
#define CODE_RED  1
void socket_transmit( NetSocket *sock, int code, int len, char *data );

/* update socket by net_packet header if net_sender_addr is socket.remote_addr 
 * and set read pointer to packet payload. if false is returned packet is
 * not parsed. */
bool socket_process_header( NetSocket *sock ); 

#endif


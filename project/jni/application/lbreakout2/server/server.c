/***************************************************************************
                          server.c  -  description
                             -------------------
    begin                : 03/03/11  
    copyright            : (C) 2003 by Michael Speck
    email                : kulkanie@gmx.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***** INCLUDES ************************************************************/

#include <dirent.h>
#include "server.h"
#include "server_game.h"

/***** EXTERNAL VARIABLES **************************************************/

extern int net_buffer_cur_size, msg_read_pos;

/***** FORWARDED DECLARATIONS **********************************************/

#ifdef NETWORK_ENABLED
static void signal_handler( int signal );
static void broadcast_all( int len, char *data );
#endif

/***** LOCAL TYPE DEFINITIONS **********************************************/

enum { CHANNEL_MAIN_ID = 1 };

/***** LOCAL VARIABLES *****************************************************/

int server_port = 8000;    /* server is listening at this port */
List *channels = 0;       /* chat channels */
List *games = 0;           /* running games */
List *levelsets = 0;       /* loaded levelsets */
int global_id = 1;         /* global id counter increased each type an object 
                  (user,channel,game) is added (id:1 is channel MAIN) */
int server_halt = 0;       /* when set, server will go down after some seconds */
int server_halt_since = 0; /* global time (in secs) passed since halt command */
char errbuf[128];       /* used to compile error messages */
char msgbuf[MAX_MSG_SIZE];       /* used to compile messages */
int  msglen = 0; 
int  user_limit = 50;        /* maximum number of users that may be logged in (0: unlimited)*/
int  user_count = 0;        /* number of logged in users */
char greetings[256]; /* welcome message send to user on login */
char admin_pwd[20] = "";    /* admin password (if any) */
char datadir[128] = SRC_DIR;    /* levels are loaded from here */
int  server_fps = 20;          /* communication frame rate */
int  server_frame_delay = 50; /* delay between server frames */
int  server_recv_limit = -1; /* number of packets parsed in one go (-1 == unlimited) */
int  server_def_bot_num = 1; /* number of 800's and 1000's bots to be 
                                created on startup */

/* these default channels are known by the client and the only
 * ones shown in the list. there id starts at 1 for MAIN increased
 * by 1 with each step */
ServerChannel *main_channel = 0;  /* pointer to MAIN */
int default_channel_count = 1;
char *default_channels[1];

/***** LOCAL FUNCTIONS *****************************************************/

#ifdef NETWORK_ENABLED

static void server_init_halt()
{
    printf( _("server is going down...\n") );
    server_halt_since = time(0);
    server_halt = 1;
    errbuf[0] = MSG_ERROR;
    sprintf( errbuf+1, _("SERVER IS GOING DOWN!!!") );
    broadcast_all( 2+strlen(errbuf+1), errbuf );
}

void send_info( ServerUser *user, int type, char *format, ... )
{
    va_list args;

    if ( user->no_comm ) return;
    
    va_start( args, format );
    vsnprintf( msgbuf+1, MAX_MSG_SIZE-1, format, args );
    va_end( args );
    msgbuf[0] = type;
    
    socket_transmit( &user->socket, CODE_BLUE, 2+strlen(msgbuf+1), msgbuf );
}

/* channel_add/delete don't require client updates as the only
 * channels that are shown in the list are already known by name
 * and id by the client. additional channels can be entered by typing
 * in the name. A pointer is returned to simplify transfer of users.
 */
static ServerChannel* channel_add( char *name )
{
    ServerChannel *channel = salloc( 1, sizeof( ServerChannel ) );

    strncpy(channel->name,name,20);
    channel->id    = global_id++;
    channel->users = list_create( LIST_AUTO_DELETE, LIST_NO_CALLBACK );

    printf( _("channel added: %s\n"), channel->name );
    list_add( channels, channel );

    return channel;
}
static void channel_delete( void *ptr )
{
    ServerChannel *channel = (ServerChannel*)ptr;
    if ( channel ) {
        printf( _("channel deleted: %s (%i users)\n"), channel->name, channel->users->count );
        if ( channel->users )
            list_delete( channel->users );
        free( channel );
    }
}

ServerChannel *channel_find_by_name( char *name )
{
    ServerChannel *channel = 0;
    list_reset( channels );
    while ( ( channel = list_next( channels ) ) )
        if ( !strcmp( channel->name, name ) )
            return channel;
    return 0;
}

static void channel_add_user( ServerChannel *channel, ServerUser *user );
static void channel_add_bot( ServerChannel *channel, char *name, int level )
{
    NetAddr addr;
    ServerUser *user = salloc( 1, sizeof( ServerUser) );
    
    /* add a bot user to channel which can be challenged
     * but does nothing else */
        strncpy(user->name,name,16);
    user->id = global_id++;
        user->no_comm = 1;
    user->bot = 1;
        user->bot_level = level; /* playing level */
    net_build_addr( &addr, "localhost", 8000 );
    socket_init( &user->socket, &addr );
    channel_add_user( channel, user );
}

static void create_default_channels()
{
    int i = 0;
    
    for ( i = 0; i < default_channel_count; i++ )
        channel_add( default_channels[i] );
    main_channel = (ServerChannel*)list_first( channels );
}

static int is_default_channel( ServerChannel *channel )
{
    int i = 0;
    
    for ( i = 0; i < default_channel_count; i++ )
        if ( !strcmp( default_channels[i], channel->name ) ) return 1;
    return 0;
}

static void channel_broadcast( ServerChannel *channel, int len, char *data )
{
    int urgent = 0;
    ServerUser *user;

    /* urgent messages are always sent even to hidden users */
    if ( data[0] == MSG_ERROR || 
         data[0] == MSG_ADD_USER || data[0] == MSG_REMOVE_USER ||
         data[0] == MSG_SET_COMM_DELAY )
        urgent = 1;
    
    /* deliver it */
    list_reset( channel->users );
    while ( ( user = list_next( channel->users ) ) )
        if ( !user->no_comm )
        if ( urgent || !user->hidden )
            socket_transmit( &user->socket, CODE_BLUE, len, data );
}

/* broadcast message to all users in all channels even the hidden ones */
static void broadcast_all( int len, char *data )
{
    ServerChannel *channel;
    
    list_reset( channels );
    while ( ( channel = list_next( channels ) ) )
        channel_broadcast( channel, len, data );
}

static void channel_add_user( ServerChannel *channel, ServerUser *user )
{
    if ( channel == 0 ) return;
    if ( user == 0 ) return;
    
    list_add( channel->users, user );
    printf( _("user added: %s (%i) from %s\n"),
        user->name, user->id, net_addr_to_string( &user->socket.remote_addr ) );
    
    /* inform all users in channel (including this one if not hidden) */
    msg_begin_writing( msgbuf, &msglen, 128 );
    msg_write_int8( MSG_ADD_USER );
    msg_write_int32( user->id );
    msg_write_string( user->name );
    channel_broadcast( channel, msglen, msgbuf );

        /* don't count dummies */
        if ( !user->bot ) user_count++;
}

void channel_remove_user( ServerChannel *channel, ServerUser *user )
{
    if ( channel == 0 ) return;
    if ( user == 0 ) return;
    
    user->hidden = 1; /* this user does not require the following update */
    
    /* inform all users in channel */
    msg_begin_writing( msgbuf, &msglen, 128 );
    msg_write_int8( MSG_REMOVE_USER );
    msg_write_int32( user->id );
    channel_broadcast( channel, msglen, msgbuf );

        /* don't count dummies */
        if ( !user->bot ) user_count--;
    /* remove */
    printf( _("user removed: %s (%i)\n"), user->name, user->id );
    list_delete_item( channel->users, user );

    /* if empty channel and not default channel delete it */
    if ( channel->users->count == 0 && !is_default_channel( channel ) )
        list_delete_item( channels, channel );
}

void channel_hide_user( ServerChannel *channel, ServerUser *user, int hide )
{
    if ( channel == 0 ) return;
    if ( user == 0 ) return;
    if ( user->hidden == hide ) return; /* nothing changes */

    
    /* broadcast update to all users in channel */
    if ( hide ) {
        msg_begin_writing( msgbuf, &msglen, 128 );
        msg_write_int8( MSG_REMOVE_USER );
        msg_write_int32( user->id );
        channel_broadcast( channel, msglen, msgbuf );
        user->hidden = hide;
    } else {
        user->hidden = hide;
        msg_begin_writing( msgbuf, &msglen, 128 );
        msg_write_int8( MSG_ADD_USER );
        msg_write_int32( user->id );
        msg_write_string( user->name );
        channel_broadcast( channel, msglen, msgbuf );
    }
}

static void channel_kick_user( ServerChannel *channel, ServerUser *user, char *reason )
{
    if ( channel == 0 ) return;
    if ( user == 0 ) return;

    snprintf( errbuf, 128, _("You have been kicked! Reason: %s"), reason );
    send_info( user, MSG_ERROR, errbuf );

    errbuf[0] = MSG_DISCONNECT;
    socket_transmit( &user->socket, CODE_BLUE, 1, errbuf );
    
    printf( _("user kicked (%s): %s (%i)\n"), reason, user->name, user->id );
    channel_remove_user( channel, user );
}

/* transfer user to new channel and send nescessary updates */
void send_full_update( ServerUser *user, ServerChannel *channel );
void channel_transfer_user( ServerChannel *old, ServerChannel *new, ServerUser *user )
{
    /* same channel? */
    if ( old == new ) return;

    /* mute user as he will receive a complete update
       after the transfer */
    user->hidden = 1;
    
    /* transfer */
    msg_begin_writing( msgbuf, &msglen, 128 );
    msg_write_int8( MSG_REMOVE_USER );
    msg_write_int32( user->id );
    channel_broadcast( old, msglen, msgbuf );

    list_transfer( old->users, new->users, user );
    
    if ( old->users->count == 0 && !is_default_channel( old ) )
        list_delete_item( channels, old );
    
    msg_begin_writing( msgbuf, &msglen, 128 );
    msg_write_int8( MSG_ADD_USER );
    msg_write_int32( user->id );
    channel_broadcast( new, msglen, msgbuf );

    /* update */
    user->hidden = 0;
    msg_begin_writing( msgbuf, &msglen, 128 );
    msg_write_int8( MSG_SERVER_INFO );
    msg_printf( _("you have entered a new channel: %s"), new->name );
    socket_transmit( &user->socket, CODE_BLUE, msglen, msgbuf );
    msg_begin_writing( msgbuf, &msglen, 128 );
    msg_write_int8( MSG_SET_CHANNEL );
    msg_write_string( new->name );
    socket_transmit( &user->socket, CODE_BLUE, msglen, msgbuf );
    
    send_full_update( user, new );
}
            
/* Send a list of all users in user's channel including itself.
 *
 * FIXME: Sending each single entry is a not very nice.
 */
void send_full_update( ServerUser *user, ServerChannel *channel )
{
    ServerUser *u;

    msgbuf[0] = MSG_PREPARE_FULL_UPDATE;
    socket_transmit( &user->socket, CODE_BLUE, 1, msgbuf );

    /* users */
    list_reset( channel->users );
    while ( ( u = list_next( channel->users ) ) ) {
        msg_begin_writing( msgbuf, &msglen, 32 );
        msg_write_int8( MSG_ADD_USER );
        msg_write_int32( u->id );
        msg_write_string( u->name );
        socket_transmit( &user->socket, CODE_BLUE, msglen, msgbuf );
    }
}
    

/* ServerUser *find_user_by_name
 * IN    char    *name
 *
 * Search all channels for a user by that name.
 */
static ServerUser* find_user_by_name( char *name )
{
    ServerUser *user;
    ServerChannel *channel;
    
    list_reset( channels );
    while ( ( channel = list_next( channels ) ) ) {
        list_reset( channel->users );
        while ( ( user = list_next( channel->users ) ) )
            if ( !strcmp( user->name, name ) )
                return user;
    }
    
    return 0;
}

/* ServerUser *search_user
 * IN    char    *name
 *
 * Search all channels for a user by that name and return
 * the channel as well.
 */
static ServerUser* search_user( char *name, ServerChannel **channel )
{
    ServerUser *user;
    
    list_reset( channels );
    while ( ( *channel = list_next( channels ) ) ) {
        list_reset( (*channel)->users );
        while ( ( user = list_next( (*channel)->users ) ) )
            if ( !strcmp( user->name, name ) )
                return user;
    }
    
    return 0;
}

/* ServerUser *find_user_by_id
 * IN    int    id
 *
 * Search all channels for a user by that id.
 */
static ServerUser* find_user_by_id( int id )
{
    ServerUser *user;
    ServerChannel *channel;
    
    list_reset( channels );
    while ( ( channel = list_next( channels ) ) ) {
        list_reset( channel->users );
        while ( ( user = list_next( channel->users ) ) )
            if ( user->id == id )
                return user;
    }
    
    return 0;
}

/* ServerUser *find_user_by_addr
 * IN    NetAddr        *addr
 *
 * Search wether a user already uses this net address.
 */
static ServerUser* find_user_by_addr( NetAddr *addr )
{
    ServerUser *user;
    ServerChannel *channel;
    
    list_reset( channels );
    while ( ( channel = list_next( channels ) ) ) {
        list_reset( channel->users );
        while ( ( user = list_next( channel->users ) ) )
            if ( net_compare_addr( addr, &user->socket.remote_addr ) )
                return user;
    }
    
    return 0;
}

/* void handle_connectionless_packet
 *
 * By now only connection attempts can be found in this category.
 * So check wether the packet contains a valid request (or send
 * error messages if it doesn't) and add a new user to channel
 * MAIN.
 */
static void handle_connectionless_packet( void )
{
    char        name[20], pwd[20], buf[128];
    int        protocol;
    ServerUser    *user = 0;
    int         i;
    
    msg_begin_connectionless_reading();

    if ( msg_read_int8() != MSG_CONNECT ) return;

    protocol = msg_read_int8();
        strncpy(name,msg_read_string(),20); name[19] = 0;
        strncpy(pwd,msg_read_string(),20); pwd[19] = 0;
    if ( msg_read_failed() ) {
        sprintf( errbuf+1, _("Login data corrupted, please retry.") );
        goto failure;
    }
        if ( !is_alphanum(name) ) {
            strcpy(errbuf+1, 
                _("Your username may only contain letters, digits and underscores.\n") );
            goto failure;
        }
    
    /* check wether this user already exists. if so the LOGIN_OKAY
     * message was dropped. */
    if ( (user = find_user_by_addr( &net_sender_addr )) )
        if ( strcmp( user->name, name ) )
            user = 0; /* somebody else though same box */
    
    /* check data for validity */
    if ( protocol != PROTOCOL ) {
        if ( protocol < PROTOCOL )
            sprintf( errbuf+1, _("Your protocol is out of date, please update.") );
        else
            sprintf( errbuf+1, _("Server uses an older protocol (%i), sorry."),
                PROTOCOL );
        goto failure;
    }
    if ( user_count >= user_limit && user == 0/*else user exists already but wasn't informed*/ ) {
        sprintf( errbuf+1, _("Server is full!") );
        goto failure;
    }
    if ( name[0] == 0 ) {
        sprintf( errbuf+1, _("Please enter a name!") );
        goto failure;
    }
    if ( strchr( name, ' ' ) ) {
        sprintf( errbuf+1, _("Your name must not contain blanks! (But can have underscores.)") );
        goto failure;
    }
    if ( (user==0 && find_user_by_name( name )) || !strcmp( name, _("admin") ) ) {
        sprintf( errbuf+1, _("This name is already in use. Please choose another one.") );
        goto failure;
    }
    /* password is currently unused */

    /* data successfully extracted and checked. if this is not a
     * user whos LOGIN_OKAY was dropped, create a new one. */
    if ( user == 0 ) {
        user = salloc( 1, sizeof( ServerUser ) );
        user->id = global_id++;
        if ( admin_pwd[0] != 0 && !strcmp( admin_pwd, name ) ) {
            strncpy(user->name,_("admin"),20);
            user->admin = 1;
        }
        else
                    strncpy(user->name,name,20);
        socket_init( &user->socket, &net_sender_addr );
        user->hidden = 1; /* don't get the ADD_USER message */
        channel_add_user( main_channel, user );
        user->hidden = 0;
    }
    
    /* tell user that it is accepted */
    msg_begin_writing( msgbuf, &msglen, 32 );
    msg_write_int8( MSG_LOGIN_OKAY );
    msg_write_int32( user->id );
    msg_write_string( user->name );
    socket_transmit( &user->socket, CODE_BLUE, msglen, msgbuf );
    msg_begin_writing( msgbuf, &msglen, 2+strlen(greetings) );
    msg_write_int8( MSG_SERVER_INFO );
    msg_write_string( greetings );
    socket_transmit( &user->socket, CODE_BLUE, msglen, msgbuf );
        msg_begin_writing( msgbuf, &msglen, 128 );
        msg_write_int8( MSG_SERVER_INFO );
        if ( user_count == 1 )
            strcpy ( buf , _("1 user online") );
        else
            snprintf( buf, 128, _("%i users online"), user_count );
        msg_write_string( buf );
    socket_transmit( &user->socket, CODE_BLUE, msglen, msgbuf );
    msg_begin_writing( msgbuf, &msglen, 4 );
    msg_write_int8( MSG_SET_COMM_DELAY );
    msg_write_int16( server_frame_delay );
    socket_transmit( &user->socket, CODE_BLUE, msglen, msgbuf );
    
    /* send default channels */
    msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
    msg_write_int8( MSG_CHANNEL_LIST );
    msg_write_int8( default_channel_count ); 
    for ( i = 0; i < default_channel_count; i++ )
        msg_write_string( default_channels[i] );
    socket_transmit( &user->socket, CODE_BLUE, msglen, msgbuf );

    /* send levelset names */
    msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
    msg_write_int8( MSG_LEVELSET_LIST );
    msg_write_int8( levelsets->count );
    list_reset( levelsets );
    for ( i = 0; i < levelsets->count; i++ )
        msg_write_string( list_next( levelsets ) );
    socket_transmit( &user->socket, CODE_BLUE, msglen, msgbuf );

    send_full_update( user, main_channel );

    return;

failure:
    /* send error message as connectionless one as we have no
     * connection so far */
    errbuf[0] = MSG_ERROR;
    net_transmit_connectionless( &net_sender_addr, 2+strlen(errbuf+1), errbuf );
}

/* void handle_command
 * IN    ServerUser    *user
 * IN    char        *cmd_line
 *
 * Handle command send by the user. The admin has more commands available
 * the a normal user.
 */
static void handle_command( ServerUser *user, ServerChannel *channel, char *cmd_line )
{
    List *args = parser_explode_string( cmd_line, ' ' );
    char *cmd = list_first( args );
    char *name, *limit, *text;
    int val;
    ServerUser *target, *remote;
    ServerChannel *target_channel;
    LevelSet *lset;
    char buf[128];
    FILE *file;
    
    if ( cmd == 0 ) return;
    
    if ( !strcmp( cmd, _("search") ) ) {
        if ( (name = list_next( args )) == 0 ) {
            send_info( user, MSG_SERVER_INFO, _("search: specify a user name!") );
            return;
        }
        if ( (target = search_user( name, &target_channel )) == 0 )
            send_info( user, MSG_SERVER_INFO, _("search: user is not online.") );
        else {
            sprintf( buf, _("search: %s: in channel %s: %s"),
                target->name, target_channel->name,
                user->game?_("playing"):_("chatting") );
            send_info( user, MSG_SERVER_INFO, buf );
        }
    } else
    if ( !strcmp( cmd, _("version") ) ) {
        sprintf( buf, _("transmission protocol: %i"), PROTOCOL );
        send_info( user, MSG_SERVER_INFO, buf );
    } else
    if ( !strcmp( cmd, _("info") ) ) {
        sprintf( buf, _("user limit: %i#frame rate: %i#packet limit: %i"), 
            user_limit, server_fps, server_recv_limit );
        send_info( user, MSG_SERVER_INFO, buf );
    } else
    if ( !strcmp( cmd, _("addset") ) && user->admin ) {
        if ( (name = list_next( args )) == 0 ) {
            send_info( user, MSG_SERVER_INFO, _("addset: name missing") );
            return;
        }
        /* can find levelset? */
        if ( (file = levelset_open( name, "r" )) ) {
            fclose( file );
            lset = levelset_load( name, 0 );
            if ( lset ) {
                list_add( levelsets, lset );
                msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
                msg_write_int8( MSG_ADD_LEVELSET );
                msg_write_string( name );
                if ( !msg_write_failed() )
                    broadcast_all( msglen, msgbuf );
            }
            else
                send_info( user, MSG_SERVER_INFO, _("addset: file %s corrupted"), name );
        }
        else
            send_info( user, MSG_SERVER_INFO, _("addset: file %s not found"), name );
    } else
    if ( !strcmp( cmd, _("addbot") ) && user->admin ) {
        if ( (name = list_next( args )) == 0 ) {
            send_info( user, MSG_SERVER_INFO, _("addbot: name missing") );
            return;
        }
        if ( (target = find_user_by_name( name )) ) {
            send_info( user, MSG_SERVER_INFO, _("addbot: bot '%s' exists"), name );
            return;
        }
        if ( (limit = list_next( args )) == 0 ) {
            send_info( user, MSG_SERVER_INFO, _("addbot: strength missing") );
            return;
        }
        channel_add_bot( channel, name, atoi(limit) );
        
    } else
    if ( !strcmp( cmd, _("delbot") ) && user->admin ) {
        if ( (name = list_next( args )) == 0 ) {
            send_info( user, MSG_SERVER_INFO, _("delbot: name missing") );
            return;
        }
        if ( (target = find_user_by_name( name )) == 0 ) {
            send_info( user, MSG_SERVER_INFO, _("delbot: bot '%s' not found"), name );
            return;
        }
        channel_remove_user( channel, target );
    } else
    if ( !strcmp( cmd, _("set") ) && user->admin ) {
        if ( (name = list_next( args )) == 0 ) {
            send_info( user, MSG_SERVER_INFO, _("set: variable missing") );
            return;
        }
        if ( (limit = list_next( args )) == 0 ) {
            send_info( user, MSG_SERVER_INFO, _("set: value missing") );
            return;
        }
        val = atoi( limit );
        if ( !strcmp( name, _("userlimit") ) ) {
            user_limit = val;
            sprintf( buf, _("userlimit: set to %i"), user_limit );
        }
        else
        if ( !strcmp( name, _("packetlimit") ) ) {
            server_recv_limit = val;
            sprintf( buf, _("packetlimit: set to %i"), server_recv_limit );
        }
        else
        if ( !strcmp( name, _("fps") ) ) {
            server_fps = val;
            server_frame_delay = 1000/val;
            sprintf( buf, _("fps: set to %i"), server_fps );

            msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
            msg_write_int8( MSG_SET_COMM_DELAY );
            msg_write_int16( server_frame_delay );
            broadcast_all( msglen, msgbuf );
        }
        send_info( user, MSG_SERVER_INFO, buf );
    } else
    if ( !strcmp( cmd, _("kick") ) && user->admin ) {
        if ( (name = list_next( args )) == 0 ) {
            send_info( user, MSG_SERVER_INFO, _("kick: specify a user name!") );
            return;
        }
        if ( (target = search_user( name, &target_channel )) == 0 )
            send_info( user, MSG_SERVER_INFO, _("kick: user is not online.") );
        else {
            if ( target->game ) {
                /* bring em out of the game and tell the remote
                 * that the game is killed*/
                remote = ((ServerGame*)target->game)->users[0];
                if ( remote == target )
                    remote = ((ServerGame*)target->game)->users[1];
                errbuf[0] = MSG_ERROR;
                sprintf( errbuf+1, _("Sorry, but your opponent has been kicked!") );
                socket_transmit( 
                    &remote->socket, CODE_BLUE, 
                    2+strlen(errbuf+1), errbuf );
                server_game_remove( (ServerGame*)target->game ); 
            }
            msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
            msg_write_int8( MSG_SERVER_INFO );
            msg_printf( _("ADMIN has kicked %s."), target->name );
            broadcast_all( msglen, msgbuf );
            channel_kick_user( target_channel, target, _("admin kick") );
        }
        
    } else
    if ( !strcmp( cmd, _("admin_says") ) && user->admin ) {
        if ( (text = list_next( args )) == 0 ) {
            send_info( user, MSG_SERVER_INFO, _("info: a message is required!") );
            return;
        }
        /* don't show just the first word */
        if ( (text = strchr( cmd_line, ' ' )) == 0 ) return; /* will never happen */
        msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
        msg_write_int8( MSG_SERVER_INFO );
        msg_printf( _("ADMIN says: %s"), text+1 /*don't double the space*/ );
        broadcast_all( msglen, msgbuf );
    } else
    if ( !strcmp( cmd, _("halt") ) && user->admin ) {
        server_init_halt();
    }
    else {
        send_info( user, MSG_SERVER_INFO, _("unknown command: %s"), cmd );
    }
}

/* void parse_packet_channel
 * IN    ServerChannel    *channel
 * IN    ServerUser    *user
 *
 * Check all messages in packet from user who is located in channel and
 * not playing. The header has been successfully processed and the read 
 * pointer is at the beginning of the first message. If a message occurs
 * that is not handled, the rest of the packet is skipped as we don't
 * know its further format.
 */
static void parse_packet_channel( ServerUser *user, ServerChannel *channel )
{
    int id;
    unsigned char type;
    char name[16];
    ServerUser *recv;
    ServerGameCtx ctx;
    ServerChannel *newchannel;
    
    while ( 1 ) {
        type = (unsigned)msg_read_int8();
        
        if ( msg_read_failed() ) break; /* no more messages */

        switch ( type ) {
            case MSG_HEARTBEAT:
                /* updates the socket information automatically
                 * so connection is not closed */
                break;
            case MSG_DISCONNECT:
                user->no_comm = 1; /* receive no more messages */
                                printf( _("%s (%i) disconnected\n"), user->name, user->id );
                channel_remove_user( channel, user );
                break;
            case MSG_QUIT_GAME:
                /* if player looks at error message and breaks up
                 * game (e.g. waiting for stats) it will send this
                 * message which is simply ignored */
                break;
            case MSG_COMMAND:
                handle_command( user, channel, msg_read_string() );
                break;
            case MSG_UNHIDE:
                if ( user->hidden )
                    channel_hide_user( channel, user, 0 );
                break;
            case MSG_CHATTER:
                /* if UNHIDE was dropped user can become visible 
                 * again by simply chatting */
                if ( user->hidden )
                    channel_hide_user( channel, user, 0 );
                
                msg_begin_writing( msgbuf, &msglen, 128 );
                msg_write_int8( MSG_CHATTER );
                msg_printf( "<%s> %s", user->name, msg_read_string() );
                if ( !msg_write_failed() )
                    channel_broadcast( channel, msglen, msgbuf );
                break;
            case MSG_WHISPER:
                id = msg_read_int32();
                recv = find_user_by_id( id ); /* all channels */
                if ( recv == 0 ) {
                    sprintf( errbuf, _("There is no user by that name.") );
                    send_info( user, MSG_ERROR, errbuf );
                } else {
                    msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
                    msg_write_int8( MSG_CHATTER );
                    msg_printf( "<%s> %s", user->name, msg_read_string() );
                    if ( !msg_write_failed() ) {
                        socket_transmit( 
                            &recv->socket, 
                            CODE_BLUE, msglen, msgbuf );
                        socket_transmit( 
                            &user->socket, 
                            CODE_BLUE, msglen, msgbuf );
                    }
                }
                break;
            case MSG_ENTER_CHANNEL:
                                strncpy(name,msg_read_string(),16);
                if ( strchr( name, ' ' ) ) {
                    sprintf( errbuf, _("Channel name must not contain blanks!") );
                    send_info( user, MSG_ERROR, errbuf );
                    break;
                }
                newchannel = channel_find_by_name( name );
                if ( newchannel == 0 ) newchannel = channel_add( name );
                channel_transfer_user( channel, newchannel, user );
                break;
            case MSG_OPEN_GAME:
                ctx.challenger = user;
                id = msg_read_int32();
                ctx.challenged = find_user_by_id( id );
                strncpy(ctx.name,msg_read_string(),20);
                ctx.name[19] = 0;
                ctx.diff = msg_read_int8();
                ctx.rounds = msg_read_int8();
                ctx.frags = msg_read_int8();
                ctx.balls = msg_read_int8();
                errbuf[0] = 0;
                if ( msg_read_failed() )
                    sprintf( errbuf, _("OpenGame message corrupted!") );
                if ( ctx.challenged == 0 )
                    sprintf( errbuf, _("User with id %i does not exist!"), id );
                if ( errbuf[0] != 0 )
                    send_info( user, MSG_ERROR, errbuf );
                else
                    server_game_add( channel, &ctx );
                break;
            default:
                printf( _("channel %i: %s: invalid message %x: skipping %i bytes\n"),
                    channel->id, 
                    net_addr_to_string( &user->socket.remote_addr), type, 
                    net_buffer_cur_size - msg_read_pos );
                msg_read_pos = net_buffer_cur_size;
                break;

        }
    }
}

/* void find_send_user
 * OUT    ServerUser    **user
 *
 * Check all channels and games for the user who's socket address equals 
 * net_sender_addr and return a pointer to it or set '*user' 0 else.
 *
 * This is a linear search and should be improved.
 */
static void find_send_user( ServerUser **user, ServerChannel **channel, ServerGame **game )
{
    *user = 0;    
    *channel = 0;
    *game = 0;
    
    list_reset( channels );
    while ( ( *channel = list_next( channels ) ) ) {
        list_reset( (*channel)->users );
        while ( ( *user = list_next( (*channel)->users ) ) )
            if ( net_compare_addr( &net_sender_addr, &(*user)->socket.remote_addr ) ) {
                /* we have found the user. check if it is playing a game */
                if ( (*user)->game )
                    *game = (ServerGame*)(*user)->game; /* is a void pointer */
                return;
            }
    }
}

/* void remove_zombies
 * 
 * Close overflowed connections and users that were idle for too long.
 */
static void remove_zombies( void )
{
    ServerUser *user, *peer;
    ServerChannel *channel;
    int    cur_time = time(0); 
    
    list_reset( channels );
    while ( ( channel = list_next( channels ) ) ) {
        list_reset( channel->users );
        while ( ( user = list_next( channel->users ) ) ) {
            if ( user->bot ) continue; /* are never removed */
            if ( user->socket.fatal_error ||
                 cur_time >= user->socket.idle_since + 60 ) {
                /* either the code red buffer overflowed or the
                 * user did not send the heartbeat: a zombie! */
                /* bring users to channel if playing */
                if ( user->game ) {
                    peer = user->player_id==0?
                           ((ServerGame*)user->game)->users[1]:
                           ((ServerGame*)user->game)->users[0];
                    send_info( peer, MSG_ERROR, 
                        _("Remote player has disconnected!") ); 
                    server_game_remove( (ServerGame*)user->game ); 
                }
                channel_kick_user( channel, user, _("zombie") );
            }
        }
    }
}

/* void handle
 * IN    int    ms    milliseconds passed since last call
 *
 * Receive all packets at the single UDP socket and check wether it is
 * connectionless (connection requests) or belongs to a connection (socket).
 * Find the user and parse all messages in the packet.
 *
 * Remove any zombies. (no heartbeat, buffer overflow)
 *
 * Update games.
 */
static void handle( int ms )
{
    int        recv_limit;
    ServerUser    *user = 0;
    ServerChannel    *channel = 0;
    ServerGame     *game = 0;
    
    recv_limit = server_recv_limit; /* limited number of packets if not -1 */
    while ( net_recv_packet() && ( recv_limit==-1 || recv_limit > 0) ) {
        /* handle connectionless packets (login requests) */
        if ( msg_is_connectionless() ) {
            handle_connectionless_packet();
            continue;
        }

        /* find the sending user and its channel by comparing 
         * net_sender_addr. */
        find_send_user( &user, &channel, &game );
        if ( user == 0 ) continue;

        /* check if this is a valid packet and update the socket */
        if ( !socket_process_header( &user->socket ) ) continue;

        /* extract the messages */
        if ( game )
            parse_packet_game( game, user );
        else
        if ( channel )
            parse_packet_channel( user, channel );
        
        if ( recv_limit != -1 ) recv_limit--;
    }

    remove_zombies();

    update_games( ms );
}

/* (re)load all levelsets from the datadir */
static int load_levelsets( void )
{
    DIR *hdir;
    struct dirent *dirent;
    
    list_clear( levelsets );

    hdir = opendir( SRC_DIR "/levels" );
    if ( hdir == 0 ) {
        printf( _("couldn't open directory %s!\n"), SRC_DIR "/levels" );
        return 0;
    }

    while ( (dirent = readdir( hdir )) ) {
        if ( dirent->d_name[0] == '.' ) continue;
        if ( dirent->d_name[0] != 'N' || dirent->d_name[1] != '_' ) continue;
        list_add( levelsets, levelset_load( dirent->d_name, 0 ) );
    }
    printf( _("loaded %i levelsets from directory %s\n"), 
        levelsets->count, SRC_DIR "/levels" );
    
    closedir( hdir );
    return 1;
}

/* display help of command line options */
static void display_help()
{
    printf( _("Usage:\n  lbreakout2server\n") );
    printf( _("    [-p <SERVER_PORT>]      Bind server to this port (Default: %i).\n"), server_port );
    printf( _("    [-l <USER_LIMIT>]       Maximum number of users that can login to server.\n") );
    printf( _("    [-m <WELCOME_FILE>]     The text in this file is send to new users on login.\n") );
    printf( _("    [-a <ADMIN_PWD>]        The user logging in as <ADMIN_PWD> will become\n") );
    printf( _("                            the administrator named 'admin'.\n") );
/*    printf( _("    [-D <DATADIR>]          In this directory the network levelsets are located.\n") );
    printf( _("                            Note: To upload levelsets as admin the directory\n") );
    printf( _("                            must be writeable.\n") );*/
    printf( _("    [-f <FRAMERATE>]        Number of send/recv handlings in a second.\n") );
    printf( _("                            (Default: 33)\n") );
        printf( _("    [-b <BOTNUM>]           Number of paddle bots with 800 and 1000 strength\n"));
        printf( _("                            each. (Default: 0)\n") );
    exit( 0 );
}

/* Parse the command line. */
static void parse_args( int argc, char **argv )
{
    int i, len;
    FILE *file;
    
    for ( i = 0; i < argc; i++ ) {
        if ( !strcmp( "-p", argv[i] ) )
            if ( argv[i + 1] )
                server_port = atoi( argv[i + 1] );
        if ( !strcmp( "-l", argv[i] ) )
            if ( argv[i + 1] )
                user_limit = atoi( argv[i + 1] );
        if ( !strcmp( "-f", argv[i] ) )
            if ( argv[i + 1] ) {
                server_fps = atoi(argv[i + 1]);
                server_frame_delay = 1000/server_fps;
            }
        if ( !strcmp( "-D", argv[i] ) )
            if ( argv[i + 1] )
                strncpy(datadir,argv[i + 1],128);
        if ( !strcmp( "-h", argv[i] ) || !strcmp( "--help", argv[i] ) )
            display_help();
        if ( !strcmp( "-m", argv[i] ) )
            if ( argv[i + 1] ) {
                file = fopen( argv[i+1], "r" );
                if ( file == 0 )
                {
                    printf( _("greetings file not found, setting directly: %s\n"), argv[i+1] );
                    snprintf(greetings,256,"%s",argv[i+1]);
                }
                else {
                    len = fread( greetings, 1, 255, file );
                    greetings[len] = 0;
                    fclose( file );
                    printf( _("greetings loaded: %s\n"), argv[i+1] );
                }
            }
        if ( !strcmp( "-a", argv[i] ) )
            if ( argv[i + 1] )
                strncpy(admin_pwd,argv[i + 1],15);
                if ( !strcmp( "-b", argv[i] ) )
                    if ( argv[i + 1] )
                        server_def_bot_num = atoi(argv[i + 1]);
    }
}

/* Initiate network connection and lists. */
static void finalize()
{
    /* disconnect all users */
    errbuf[0] = MSG_DISCONNECT;
    broadcast_all( 1, errbuf );
    
    /* free lists */
    if ( channels )
        list_delete( channels );
    if ( games )
        list_delete( games ); 
    if ( levelsets )
        list_delete( levelsets ); 
    
    /* close server socket */
    net_shutdown();

    /* free default channel title */    
    free(default_channels[0]);

    printf( _("server halted\n") );
}
static void init( int argc, char **argv )
{
    char name[16];
    int  id = 1, j;

    /* i18n */
#ifdef ENABLE_NLS
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
    
    /* initiate sdl timer */
    SDL_Init( SDL_INIT_TIMER );
    
    /* set signal handler to cleanly shutdown by CTRL-C */
    signal( SIGINT, signal_handler );

    /* welcome message */
    snprintf(greetings,256,_("Welcome to LBreakout2 online!"));

    /* default channel */
    default_channels[0] = strdup(_("MAIN"));
    
    /* parse command line options */
    parse_args( argc, argv );
    
    /* open single UDP socket */
    if ( !net_init( server_port ) ) exit(1);
    
    /* create empty lists */
    channels = list_create( LIST_AUTO_DELETE, channel_delete );
    games    = list_create( LIST_AUTO_DELETE, server_game_delete );
    levelsets= list_create( LIST_AUTO_DELETE, levelset_list_delete );
    if ( channels == 0 || games == 0 || levelsets == 0 ) exit(1);
    
    /* load levelset names */
    load_levelsets();

    printf( _("user limit is %i\n"), user_limit );
    printf( _("FPS: %i (delay: %i ms)\n"), 1000/server_frame_delay, server_frame_delay );
    
    /* add default channels */
    create_default_channels();

        /* add default bots */
        for ( j = 0; j < server_def_bot_num; j++,id++ )
        {
            snprintf( name, 16, _("BOT%i-800"), id );
            channel_add_bot( main_channel, name, 800 );
        }
        for ( j = 0; j < server_def_bot_num; j++,id++ )
        {
            snprintf( name, 16, _("BOT%i-1000"), id );
            channel_add_bot( main_channel, name, 1000 );
        }
        
    /* build angle table */
    init_angles();
}

static void signal_handler( int signal )
{
    switch ( signal ) {
        case SIGINT:
            if ( server_halt ) break;
            server_init_halt();
            break;
    }
}

#endif

/***** PUBLIC FUNCTIONS ****************************************************/

int main( int argc, char **argv )
{
#ifdef NETWORK_ENABLED
    int last_ticks, cur_ticks;
    int ms = 0;

    set_random_seed(); /* set random seed */
    init( argc, argv );
 
    /* loop and handle messages until shutdown */
    last_ticks = cur_ticks = SDL_GetTicks();
    while ( 1 ) {
        last_ticks = cur_ticks; cur_ticks = SDL_GetTicks(); 
        ms += cur_ticks - last_ticks;
        
        if ( ms > server_frame_delay ) {
            handle( ms );
            ms -= server_frame_delay;
        }
        
        if ( server_halt && time( 0 ) > server_halt_since + 5 )
            break;
        
        SDL_Delay( 5 );
    }
    
    finalize();
#else
    printf( _("LBreakout2 has been compiled without network support.\n") );
#endif

    return 0;
}

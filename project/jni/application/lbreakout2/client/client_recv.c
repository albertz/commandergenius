/***************************************************************************
                          client_recv.c  -  description
                             -------------------
    begin                : Sat Oct 26 12:02:57 CEST 2002
    copyright            : (C) 2002 by Michael Speck
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
 
#include <stdarg.h>
#include "lbreakout.h"
#include "client_data.h"
#include "game.h"
#include "../game/game.h"
#include "../gui/gui.h"

/*
====================================================================
Externals
====================================================================
*/
#ifdef NETWORK_ENABLED
extern NetSocket client;
extern int client_is_connected;
#endif
extern GuiWidget *dlg_info;
extern GuiWidget *dlg_confirm;
extern GuiWidget *dlg_chatroom;
extern GuiWidget *label_channel;
extern GuiWidget *label_info;
extern GuiWidget *label_confirm;
extern GuiWidget *label_stats;
extern GuiWidget *label_winner;
extern GuiWidget *list_chatter;
extern GuiWidget *list_levels;
extern GuiWidget *list_users;
extern GuiWidget *list_games;
extern List *client_users;
extern List *client_channels;
extern List *client_levelsets;
extern int client_state;
extern List *levels;
extern int net_buffer_cur_size, msg_read_pos;
extern char net_buffer[MAX_MSG_SIZE + PACKET_HEADER_SIZE];
/* CHALLENGE */
extern char client_name[16];
extern int client_recv_limit;
extern char *mp_diff_names[];
extern char mp_levelset[16];
extern int mp_peer_id;
extern char mp_peer_name[16];
extern int mp_levelset_version;
extern int mp_levelset_update;
extern int mp_level_count;
extern int levelset_version, levelset_update;
extern int mp_diff, mp_rounds, mp_frags, mp_balls;
extern ClientUser *client_user;
extern void client_run_game( int challenger );
extern void client_disconnect(); /* from client.c */
extern int client_comm_delay;

/*
====================================================================
Popup info dialogue and set status to INFO.
====================================================================
*/
void client_popup_info( char *format, ... )
{
	char buffer[256];
	va_list args;
	va_start( args, format );
	vsnprintf( buffer, 256, format, args );
	va_end( args );
	gui_label_set_text( label_info, buffer );
	gui_widget_show( dlg_info );
	client_state = CLIENT_INFO;
}
/*
====================================================================
Popup confirm dialogue and _keep_ status.
====================================================================
*/
void client_popup_confirm( char *format, ... )
{
	char buffer[256];
	va_list args;
	va_start( args, format );
	vsnprintf( buffer, 256, format, args );
	va_end( args );
	gui_label_set_text( label_confirm, buffer );
	gui_widget_show( dlg_confirm );
}
    
/*
====================================================================
Parse all messages in net_packet.
====================================================================
*/
#ifdef NETWORK_ENABLED
static void client_parse_packet()
{
	int i, num;
	char name[16];
	unsigned char type;
	int handled;
	
	while ( 1 ) {
		type = (unsigned)msg_read_int8(); handled = 0;
		
		if ( msg_read_failed() ) break; /* no more messages */

		switch ( type ) {
		    	case MSG_PREPARE_FULL_UPDATE:
				/* do only clear users as channels and
				 * levelsets are fixed */
				list_clear( client_users ); client_user = 0;
				handled = 1;
				break;
			case MSG_ERROR:
				client_printf_chatter( 1, _("ERROR: %s"), msg_read_string() );
				handled = 1;
				break;
			case MSG_BUSY:
				if ( client_state == CLIENT_AWAIT_ANSWER ||
				     client_state == CLIENT_AWAIT_TRANSFER_CONFIRMATION )
					client_popup_info( 
						_("%s is busy at the moment."), 
						mp_peer_name );
				handled = 1;
				break;
			case MSG_DISCONNECT:
				client_disconnect();
				handled = 1;
				break;
			case MSG_SET_COMM_DELAY:
				client_comm_delay = msg_read_int16();
				printf( _("comm_delay set to %i\n"), client_comm_delay );
				handled = 1;
				break;
			/* chatter */
			case MSG_SERVER_INFO:
				client_add_chatter( msg_read_string(), 1 );
				handled = 1;
				break;
			case MSG_CHATTER:
				client_add_chatter( msg_read_string(), 0 );
				handled = 1;
				break;
			/* users */
			case MSG_ADD_USER:
				num = msg_read_int32();
				snprintf( name, 16, "%s", msg_read_string() ); name[15] = 0;
				if ( msg_read_failed() ) break;
				client_add_user( num, name );
				gui_list_update( 
						list_users, 
						client_users->count );
				/* re-select current entry */
				if ( client_user ) {
					num = list_check( client_users, client_user );
					if ( num != -1 )
						gui_list_select( list_users, 0, num, 1 );
				}
				handled = 1;
				break;
			case MSG_REMOVE_USER:
				num = msg_read_int32();
				if ( msg_read_failed() ) break;
				client_remove_user( num );
				gui_list_update( 
					list_users, 
					client_users->count );
				/* re-select current entry */
				if ( client_user ) {
					num = list_check( client_users, client_user );
					if ( num != -1 )
						gui_list_select( list_users, 0, num, 1 );
				}
				handled = 1;
				break;
			case MSG_CHANNEL_LIST:
				list_clear( client_channels );
				num = msg_read_int8();
				for ( i = 0; i < num; i++ )
					list_add( client_channels, strdup(msg_read_string()) );
				handled = 1;
				break;
			case MSG_LEVELSET_LIST:
				list_clear( client_levelsets );
				num = msg_read_int8();
				for ( i = 0; i < num; i++ )
					list_add( client_levelsets, strdup(msg_read_string()) );
    				gui_list_update( list_levels, client_levelsets->count );
				handled = 1;
				break;
			case MSG_ADD_LEVELSET:
				list_add( client_levelsets, strdup(msg_read_string()) );
    				gui_list_update( list_levels, client_levelsets->count );
				handled = 1;
				break;
			case MSG_SET_CHANNEL:
				/* we only need to update the name */
				gui_label_set_text( label_channel, msg_read_string() );
				handled = 1;
				break;
			/* challenge */
			case MSG_CHALLENGE:
				/* the user may only be challenged if client state is NONE
				   because otherwise he is doing something that shouldn't be
				   interrupted */
				if ( client_state != CLIENT_NONE ) {
					msg_begin_writing( msgbuf, &msglen, 128 );
					msg_write_int8( MSG_BUSY );
					msg_write_int32( msg_read_int32() );
					client_transmit( CODE_BLUE, msglen, msgbuf );
					break;
				}
				snprintf( mp_peer_name, 15, "%s", msg_read_string() );
				snprintf( mp_levelset, 16, "%s", msg_read_string() );
				mp_diff = msg_read_int8();
				mp_rounds = msg_read_int8();
				mp_frags = msg_read_int8();
				mp_balls = msg_read_int8();
				if ( msg_read_failed() ) break;
				client_popup_confirm( _("    You have been challenged!##"\
						"    Challenger: %13s#"\
						"    Levelset:   %13s#"\
						"    Difficulty: %13s#"\
						"    Rounds:     %13i#"\
						"    Frag Limit: %13i#"\
						"    Balls:      %13i"),
						mp_peer_name, mp_levelset, mp_diff_names[mp_diff],
						mp_rounds, mp_frags, mp_balls );
				client_state = CLIENT_ANSWER;
				handled = 1;
				break;
			case MSG_REJECT_CHALLENGE:
				handled = 1;
				if ( client_state != CLIENT_AWAIT_ANSWER ) break;
				client_popup_info( 
					_("%s is too scared to accept your challenge."), 
					mp_peer_name );
				break;
			case MSG_CANCEL_GAME:
				handled = 1;
				if ( client_state != CLIENT_ANSWER ) break;
				gui_widget_hide( dlg_confirm );
				client_popup_info( _("%s got cold feet."), mp_peer_name );
				break;
			case MSG_ACCEPT_CHALLENGE:
				handled = 1;
				if ( client_state != CLIENT_AWAIT_ANSWER ) break;
				gui_widget_hide( dlg_info );

				/* play */
				gui_disable_event_filter();
				if ( client_game_init_network( mp_peer_name, mp_diff ) )
					client_game_run();
				client_game_finalize();
				gui_enable_event_filter();

				gui_widget_draw( dlg_chatroom );
				stk_display_fade( STK_FADE_IN, STK_FADE_DEFAULT_TIME );
				break;

			/* dummy parse game packets that may arrive after the QUIT_GAME
			 * message was sent because ADD_USER commands may be in the
			 * package and these we should get. */
			case MSG_PADDLE_STATE:
				comm_unpack_paddle_dummy( net_buffer, &msg_read_pos );
				handled = 1;
				break;
			case MSG_SHOT_POSITIONS:
				comm_unpack_shots_dummy( net_buffer, &msg_read_pos );
				handled = 1;
				break;
			case MSG_BALL_POSITIONS:
				comm_unpack_balls_dummy( net_buffer, &msg_read_pos );
				handled = 1;
				break;
			case MSG_SCORES:
				comm_unpack_scores_dummy( net_buffer, &msg_read_pos );
				handled = 1;
				break;
			case MSG_BRICK_HITS:
				comm_unpack_brick_hits_dummy( net_buffer, &msg_read_pos );
				handled = 1;
				break;
			case MSG_NEW_EXTRAS:
				comm_unpack_collected_extras_dummy( net_buffer, &msg_read_pos );
				handled = 1;
				break;
			case MSG_ROUND_OVER:
				i = msg_read_int8();
				handled = 1;
				break;
			case MSG_LAST_ROUND_OVER:
				i = msg_read_int8();
				handled = 1;
				break;

		}

		if ( !handled ) {
			printf( _("chat: state %i: invalid message %x: skipping %i bytes\n"),
				client_state, type, net_buffer_cur_size - msg_read_pos );
			msg_read_pos = net_buffer_cur_size;
		}
	}
}
#endif

/*
====================================================================
Receive packets from server. Use the TIME_PASSED event therefore.
Send a heartbeat every three seconds as well.
====================================================================
*/
void client_recv_packet( GuiWidget *widget, GuiEvent *event )
{
#ifdef NETWORK_ENABLED
	int recv_limit;
	static int last_heartbeat = 0;
	
	if ( !client_is_connected ) return;
	if ( event->type != GUI_TIME_PASSED ) return;

	/* heartbeat? */
	if ( time(0) >= last_heartbeat + 10 ) {
		last_heartbeat = time(0);
		msgbuf[0] = MSG_HEARTBEAT;
		client_transmit( CODE_BLUE, 1, msgbuf );
	}

	recv_limit = client_recv_limit; /* limited number of packets if not -1 */
	while ( net_recv_packet() && ( recv_limit==-1 || recv_limit > 0) ) {

		/* check if this is a valid packet and update the socket */
		if ( msg_is_connectionless() )
			msg_begin_connectionless_reading();
		else
			if ( !socket_process_header( &client ) ) 
				continue;
		client_parse_packet();
		
		if ( recv_limit != -1 ) recv_limit--;
	}
#endif
}

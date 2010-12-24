/***************************************************************************
                          client_handlers.c  -  description
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
 
#include "lbreakout.h"
#include "config.h"
#include "../gui/gui.h"
#include "client_data.h"
#include "client_handlers.h"
#include "comm.h"
#include "game.h"

/*
====================================================================
Externals
====================================================================
*/
extern Config config;
extern List *client_users;
extern List *client_games;
extern List *client_channels;
extern List *client_levelsets;
extern char *client_levelset;
extern ClientUser *client_user;
extern char chatter[CHAT_LINE_COUNT][CHAT_LINE_WIDTH];
extern GuiWidget *dlg_connect;
extern GuiWidget *dlg_info;
extern GuiWidget *dlg_confirm;
extern GuiWidget *dlg_chatroom;
extern GuiWidget *dlg_channels;
extern GuiWidget *dlg_stats;
extern GuiWidget *dlg_pauseroom;
extern GuiWidget *dlg_help;
extern GuiWidget *label_info;
extern GuiWidget *label_stats;
extern GuiWidget *label_winner;
extern GuiWidget *label_channel;
extern GuiWidget *edit_server;
extern GuiWidget *edit_username;
extern GuiWidget *list_chatter;
extern GuiWidget *edit_chatter;
extern GuiWidget *list_levels;
extern GuiWidget *list_users;
extern GuiWidget *list_channels;
extern GuiWidget *edit_channel;
extern GuiWidget *edit_pausechatter;
extern GuiWidget *list_help;
extern int levelset_version, levelset_update;
extern List *levels;
extern void client_popup_info( char *format, ... );
extern void client_run_game( int challenger );
extern int client_topic_count;
extern char *client_helps[];
extern Text *client_help_text;

/*
====================================================================
Client
====================================================================
*/
#ifdef NETWORK_ENABLED
NetSocket client; /* client socket to the game server */
#endif
int client_is_connected; /* wether 'client' is a valid uplink */
char client_error[128]; /* error message */
int client_id; /* id assigned by server */
char client_name[16]; /* our local username */
int client_state = CLIENT_NONE;
int client_recv_limit;

/*
====================================================================
Challenge data 
====================================================================
*/
char *mp_diff_names[] = { "Easy", "Medium", "Hard" };
char mp_levelset[16]; /* name of levelset we play */
int mp_peer_id;
char mp_peer_name[16]; /* remote player we want to play with */
int mp_levelset_version;
int mp_levelset_update; /* version of levelset */
int mp_level_count; /* number of levels in set */
int mp_diff, mp_rounds, mp_frags, mp_balls; /* game configuration */

extern void close_pause_chat( void );

/*
====================================================================
Disconnect from current server if any.
====================================================================
*/
void client_disconnect()
{
#ifdef NETWORK_ENABLED
	char buf[128];
	
	if ( !client_is_connected ) return;
	
	/* disconnect */
	socket_print_stats( &client );
	sprintf( buf, _("disconnected from %s"), 
		 net_addr_to_string(&client.remote_addr) );
	client_add_chatter( buf, 1 );
	buf[0] = MSG_DISCONNECT;
	client_transmit( CODE_BLUE, 1, buf );
	client_is_connected = 0;
	client_data_clear();
	gui_label_set_text( label_channel, "MAIN" );
#endif
}

/*
====================================================================
Try to connect to a game server. Retry twice every second
or quit then.
====================================================================
*/
void client_connect( GuiWidget *widget, GuiEvent *event )
{
#ifdef NETWORK_ENABLED
	NetAddr 	newaddr;
	int		attempt = 0;
	int		type;
	char 		server[128];

	if ( event->type != GUI_CLICKED ) return;

	/* close the connect window */
	gui_widget_hide( dlg_connect );

	/* disconnect from current server */
	client_disconnect();
	
	/* extract ip and port and build a new socket out of it */
	gui_edit_get_text( edit_server, server, 128, 0, -1 );
	snprintf( config.server, 64, "%s", server );
	if ( !net_build_addr( &newaddr, server, 0 ) ) {
		client_printf_chatter( 1, _("ERROR: address %s does not resolve"), config.server );
		return;
	}
	socket_init( &client, &newaddr );

	/* get username */
	gui_edit_get_text( edit_username, 
			config.username, 16, 0,-1 );
	
	/* build connect message */
	msg_begin_writing( msgbuf, &msglen, 64 );
	msg_write_int8( MSG_CONNECT );
	msg_write_int8( PROTOCOL );
	msg_write_string( config.username );
	msg_write_string( _("unused") ); /* passwd */
	
	while ( attempt < 3 ) {
		client_printf_chatter( 1, "%s: %s...", 
			config.server, 
			attempt==0?_("connecting"):_("retry") );
		stk_display_update( STK_UPDATE_ALL );
		net_transmit_connectionless( &newaddr, msglen, msgbuf );

		SDL_Delay( 1000 );

		while ( net_recv_packet() ) {
			if ( msg_is_connectionless() )
				msg_begin_connectionless_reading();
			else
			if ( !socket_process_header( &client ) ) 
				continue;
			
			type = msg_read_int8();
			switch ( type ) {
				case MSG_LOGIN_OKAY:
					client_id = msg_read_int32();
					strcpy( client_name, msg_read_string() );
					client_printf_chatter( 1, _("%s: connected!"), config.server );
					client_is_connected = 1;
					return;
				case MSG_ERROR:
					client_printf_chatter( 1, _("ERROR: connection refused: %s"),
						msg_read_string() );
					return;
			}
		}
		
		attempt++;
	}
	client_add_chatter( _("ERROR: server does not respond"), 1 );
#endif
}

/*
====================================================================
Open/close the connection window.
====================================================================
*/
void client_open_connect_window( 
    GuiWidget *widget, GuiEvent *event )
{
	if ( event->type == GUI_CLICKED )
		gui_widget_show( dlg_connect );
}
void client_close_connect_window( 
    GuiWidget *widget, GuiEvent *event )
{
	if ( event->type == GUI_CLICKED )
		gui_widget_hide( dlg_connect );
}
    
/*
====================================================================
Close chatroom and return to LBreakout's menu.
====================================================================
*/
void client_quit( GuiWidget *widget, GuiEvent *event )
{
	if ( event->type == GUI_CLICKED )
		gui_widget_hide( dlg_chatroom );
	/* disconnect is handled in client_run to cover
	 * stk_quit_requests as well */
}

/*
====================================================================
Close the info window and clear state.
====================================================================
*/
void client_close_info( GuiWidget *widget, GuiEvent *event )
{
#ifdef NETWORK_ENABLED
	if ( event->type == GUI_CLICKED ) {
		gui_widget_hide( dlg_info );
		msg_begin_writing( msgbuf, &msglen, 128 );
		switch ( client_state ) {
			case CLIENT_AWAIT_ANSWER:
				msg_write_int8( MSG_CANCEL_GAME );
				break;
		}
		client_transmit( CODE_BLUE, msglen, msgbuf );
		client_state = CLIENT_NONE;
	}
#endif
}

/*
====================================================================
Send chatter this function is either called by the send button
or by the edit.
====================================================================
*/
void client_send_chatter( 
    GuiWidget *widget, GuiEvent *event )
{
#ifdef NETWORK_ENABLED
	char buf[MAX_CHATTER_SIZE + 1];
	if ( (  widget->type == GUI_EDIT && 
		event->type == GUI_KEY_RELEASED &&
		event->key.keysym == SDLK_RETURN ) ||
		( widget->type == GUI_BUTTON &&
		event->type == GUI_CLICKED ) ) {
		/* get message */
		gui_edit_get_text( edit_chatter, 
				buf, MAX_CHATTER_SIZE + 1, 0,-1 );
		/* clear chat edit */
		gui_edit_set_text( edit_chatter, "" );

		msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
		/* a prepended '/' means this is a command */
		if ( buf[0] == '/' ) {
			msg_write_int8( MSG_COMMAND );
			msg_write_string( buf+1 );
		}
		else {
			msg_write_int8( MSG_CHATTER );
			msg_write_string( buf );
		}
		client_transmit( CODE_BLUE, msglen, msgbuf );
	}
#endif
}
/*
====================================================================
Whisper chatter if a user is selected.
====================================================================
*/
void client_whisper_chatter( 
    GuiWidget *widget, GuiEvent *event )
{
#ifdef NETWORK_ENABLED
	char buf[MAX_CHATTER_SIZE + 1];
	
	if ( event->type != GUI_CLICKED ) return;
	
	/* get message */
	gui_edit_get_text( edit_chatter, 
			buf, MAX_CHATTER_SIZE + 1, 0,-1 );
	
	/* send to selected user */
	if ( client_user ) {
		msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
		msg_write_int8( MSG_WHISPER );
		msg_write_int32( client_user->id );
		msg_write_string( buf );
		client_transmit( CODE_BLUE, msglen, msgbuf );

		/* clear chat edit */
		gui_edit_set_text( edit_chatter, "" );
	}
	else
		client_add_chatter( _("You must select a user to whisper!"), 1 );
#endif
}

/*
====================================================================
Update selected peer and levelset.
====================================================================
*/
void client_handle_user_list( 
    GuiWidget *widget, GuiEvent *event )
{
    if ( event->type == GUI_ITEM_SELECTED )
        client_user = list_get( client_users, event->item.y );
    else
        if ( event->type == GUI_ITEM_UNSELECTED )
            client_user = 0;
}
void client_handle_levelset_list( 
    GuiWidget *widget, GuiEvent *event )
{
    if ( event->type == GUI_ITEM_SELECTED )
        client_levelset = list_get( client_levelsets, 
            event->item.y );
    else
        if ( event->type == GUI_ITEM_UNSELECTED )
            client_levelset = 0;
}

/*
====================================================================
Handle confirmation/cancelling of confirmation dialogue.
====================================================================
*/
void client_confirm( GuiWidget *widget, GuiEvent *event )
{
#ifdef NETWORK_ENABLED
	if ( event->type != GUI_CLICKED ) return;
	gui_widget_hide( dlg_confirm );
	msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
	switch ( client_state ) {
		case CLIENT_ANSWER:
			msg_write_int8( MSG_ACCEPT_CHALLENGE );
			client_transmit( CODE_BLUE, msglen, msgbuf );

			/* play */
			gui_disable_event_filter();
			if ( client_game_init_network( mp_peer_name, mp_diff ) )
				client_game_run();
			client_game_finalize();
			gui_enable_event_filter();

			gui_widget_draw( dlg_chatroom );
			stk_display_fade( STK_FADE_IN, STK_FADE_DEFAULT_TIME );
			break;
	}
#endif
}
void client_cancel( GuiWidget *widget, GuiEvent *event )
{
#ifdef NETWORK_ENABLED
	if ( event->type != GUI_CLICKED ) return;
	gui_widget_hide( dlg_confirm );
	msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
	switch ( client_state ) {
		case CLIENT_ANSWER:
			msg_write_int8( MSG_REJECT_CHALLENGE );
			break;
	}
	client_transmit( CODE_BLUE, msglen, msgbuf );
	client_state = CLIENT_NONE;
#endif
}

/*
====================================================================
Challenge selected user.
====================================================================
*/
void client_challenge( GuiWidget *widget, GuiEvent *event )
{
#ifdef NETWORK_ENABLED
	if ( event->type != GUI_CLICKED ) return;
	
	/* everything valid? */
	if ( client_user == 0 ) {
		client_popup_info( _("You must select a user for a challenge.") );
		return;
	}
	if ( client_levelset == 0 ) {
		client_popup_info( _("You must select a levelset for a challenge.") );
		return;
	}
	if ( client_user->id == client_id ) {
		client_popup_info( _("You can't challenge yourself.") );
		return;
	}
	
	strcpy( mp_peer_name, client_user->name );
	mp_peer_id = client_user->id;
	strcpy( mp_levelset, client_levelset );
	mp_diff = config.mp_diff;
	mp_rounds = config.mp_rounds;
	mp_balls = config.mp_balls;
	mp_frags = config.mp_frags;

	/* challenger, challenged, levelset, diff, rounds, frags, balls */
	msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
	msg_write_int8( MSG_OPEN_GAME );
	msg_write_int32( mp_peer_id );
	msg_write_string( mp_levelset );
	msg_write_int8( mp_diff );
	msg_write_int8( mp_rounds );
	msg_write_int8( mp_frags );
	msg_write_int8( mp_balls );
	client_transmit( CODE_BLUE, msglen, msgbuf );
	
	client_popup_info( _("You have challenged %s. Let's see what (s)he says..."), mp_peer_name );
	client_state = CLIENT_AWAIT_ANSWER;
#endif
}

/*
====================================================================
Update multiplayer network configuration.
====================================================================
*/
void client_update_difficulty( GuiWidget *widget, GuiEvent *event )
{
    if ( event->type != GUI_CHANGED ) return;
    gui_spinbutton_get_value( widget, &config.mp_diff );
    config.mp_diff--;
}
void client_update_rounds( GuiWidget *widget, GuiEvent *event )
{
    if ( event->type != GUI_CHANGED ) return;
    gui_spinbutton_get_value( widget, &config.mp_rounds );
}
void client_update_frags( GuiWidget *widget, GuiEvent *event )
{
    if ( event->type != GUI_CHANGED ) return;
    gui_spinbutton_get_value( widget, &config.mp_frags );
}
void client_update_balls( GuiWidget *widget, GuiEvent *event )
{
    if ( event->type != GUI_CHANGED ) return;
    gui_spinbutton_get_value( widget, &config.mp_balls );
}

/*
====================================================================
Open channel selector
====================================================================
*/
void client_select_channel( GuiWidget *widget, GuiEvent *event )
{
    if ( event->type != GUI_CLICKED ) return;
    /* select first channel (we always have MAIN) */
    gui_list_update( list_channels, client_channels->count );
    if ( client_channels->count > 0 ) {
        gui_edit_set_text( edit_channel, list_first( client_channels ) );
        gui_list_select( list_channels, 0,0, 1 );
    }
    gui_widget_show( dlg_channels );
    client_state = CLIENT_SELECT_CHANNEL;
}
/*
====================================================================
Handle channel (un)selection.
====================================================================
*/
void client_handle_channel_list( 
    GuiWidget *widget, GuiEvent *event )
{
	char *name;
    /* if a channel is selected the name is copied into the edit.
       unselecting does not change anything. the channel by the 
       caption in the edit is opened on enter_channel() */
    if ( event->type == GUI_ITEM_SELECTED ) {
        name = list_get( client_channels, event->item.y );
        if ( name ) gui_edit_set_text( edit_channel, name );
    }
}
/*
====================================================================
Close channel selector or enter new channel.
====================================================================
*/
void client_enter_channel( GuiWidget *widget, GuiEvent *event )
{
#ifdef NETWORK_ENABLED
	char buf[16];
	
	if ( event->type != GUI_CLICKED ) return;
	
	gui_widget_hide( dlg_channels );
	client_state = CLIENT_NONE;
	
	/* retreive name of channel we want to enter */
	buf[0] = 0;
	gui_edit_get_text( edit_channel, buf, 16, 0,-1 );
	
	/* send it */
	msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
	msg_write_int8( MSG_ENTER_CHANNEL );
	msg_write_string( buf );
	client_transmit( CODE_BLUE, msglen, msgbuf );
#endif
}
void client_cancel_channel( GuiWidget *widget, GuiEvent *event )
{
	if ( event->type != GUI_CLICKED ) return;
	gui_widget_hide( dlg_channels );
	client_state = CLIENT_NONE;
}

/*
====================================================================
Close statistics
====================================================================
*/
void client_close_stats( GuiWidget *widget, GuiEvent *event )
{
	if ( event->type != GUI_CLICKED ) return;
	gui_widget_hide( dlg_stats );
	client_state = CLIENT_NONE;
	gui_label_set_text( label_stats, _("Awaiting stats...") );
	gui_label_set_text( label_winner, "..." );
}

/*
====================================================================
Send chatter to gamepeer in pauseroom when ENTER was pressed.
====================================================================
*/
void client_send_pausechatter( GuiWidget *widget, GuiEvent *event )
{
#ifdef NETWORK_ENABLED
	char buf[MAX_CHATTER_SIZE + 1];
	if ( widget->type == GUI_EDIT && 
			event->type == GUI_KEY_RELEASED &&
			event->key.keysym == SDLK_RETURN ) {
		/* get message */
		sprintf( buf, "<%s> ", client_name );
		gui_edit_get_text( edit_pausechatter, 
				buf+strlen(buf), MAX_CHATTER_SIZE + 1, 0,-1 );
		/* clear chat edit */
		gui_edit_set_text( edit_pausechatter, "" );
		/* deliver message to remote ... */
		msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
		msg_write_int8( MSG_CHATTER );
		msg_write_string( buf );
		client_transmit( CODE_BLUE, msglen, msgbuf );
		client_add_pausechatter( buf, 0 );
	}
#endif
}

/*
====================================================================
Close pauseroom.
====================================================================
*/
void client_close_pauseroom( GuiWidget *widget, GuiEvent *event )
{
	if ( event->type != GUI_CLICKED ) return;
	close_pause_chat();
	comm_send_short( MSG_UNPAUSE );
}

/*
====================================================================
Popup help dialogue.
====================================================================
*/
void client_popup_help( GuiWidget *widget, GuiEvent *event )
{
	if ( event->type != GUI_CLICKED ) return;
	gui_widget_show( dlg_help );
	client_state = CLIENT_HELP;
}
/*
====================================================================
Close help dialogue.
====================================================================
*/
void client_close_help( GuiWidget *widget, GuiEvent *event )
{
	if ( event->type != GUI_CLICKED ) return;
	gui_widget_hide( dlg_help );
}
/*
====================================================================
Select topic and display help text.
====================================================================
*/
void client_handle_topic_list( GuiWidget *widget, GuiEvent *event )
{
	if ( event->type != GUI_ITEM_SELECTED ) return;
	if ( event->item.y >= client_topic_count ) return;
	gui_list_goto( list_help, 0 );
	if ( client_help_text ) delete_text( client_help_text );
	client_help_text = create_text( client_helps[event->item.y], 41 );
	gui_list_update( list_help, client_help_text->count );
}

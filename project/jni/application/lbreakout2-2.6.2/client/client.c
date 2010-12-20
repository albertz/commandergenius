/***************************************************************************
                          client.c  -  description
                             -------------------
    begin                : Sun Oct 20 12:02:57 CEST 2002
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
#include "event.h"
#include "../gui/gui.h"
#include "client_data.h"
#include "client_handlers.h"
#include "client.h"

/*
====================================================================
Externals
====================================================================
*/
extern SDL_Surface *cr_wallpaper;
extern Config config;
extern SDL_Surface **bkgnds;
extern StkFont *chat_font_error;
extern StkFont *chat_font_name;
extern StkFont *chat_font_normal;
extern char chatter[CHAT_LINE_COUNT][CHAT_LINE_WIDTH];
extern char pause_chatter[CHAT_LINE_COUNT][CHAT_LINE_WIDTH];
extern List *client_levelsets;
extern List *client_users;
extern List *client_games;
extern List *client_channels;
extern void client_recv_packet( GuiWidget *widget, GuiEvent *event );
extern int client_id;
extern int client_topic_count;
extern char *client_topics[];
extern Text *client_help_text;
extern NetSocket client;
extern int client_is_connected;
extern int client_state;
extern int client_recv_limit;
extern int (*sys_printf)(const char *format, ...);

/*
====================================================================
Client GUI
====================================================================
*/
GuiWidget *dlg_connect = 0;
GuiWidget *dlg_info = 0;
GuiWidget *dlg_confirm = 0;
GuiWidget *dlg_chatroom = 0;
GuiWidget *dlg_channels = 0;
GuiWidget *dlg_stats = 0;
GuiWidget *dlg_pauseroom = 0;
GuiWidget *dlg_help = 0;
/* important children */
GuiWidget *label_channel;
GuiWidget *label_info;
GuiWidget *label_confirm;
GuiWidget *label_winner;
GuiWidget *label_stats;
GuiWidget *edit_server;
GuiWidget *edit_username;
GuiWidget *list_chatter;
GuiWidget *edit_chatter;
GuiWidget *list_levels;
GuiWidget *list_users;
GuiWidget *list_channels;
GuiWidget *edit_channel;
GuiWidget *list_pausechatter;
GuiWidget *edit_pausechatter;
GuiWidget *list_topic;
GuiWidget *list_help;

#define HINT_CONNECT _("Specify the game server as address:port where "\
"the address can be a name or a dotted IP.#If the connection fails have a look "\
"at the messages in the chat window to see the reason.")

/*
====================================================================
LOCALS
====================================================================
*/

#ifdef NETWORK_ENABLED
/*
====================================================================
Render help topic.
====================================================================
*/
static int client_render_help(
    int x, int y, SDL_Surface *buffer )
{
    if ( !client_help_text ) return 0;
    if ( y >= client_help_text->count ) return 0;
    chat_font_normal->align = STK_FONT_ALIGN_LEFT;
    stk_font_write( chat_font_normal, 
        buffer, 2,0, STK_OPAQUE, 
        client_help_text->lines[y] );
    return 1;
}
/*
====================================================================
Render help topic.
====================================================================
*/
static int client_render_topic(
    int x, int y, SDL_Surface *buffer )
{
    if ( y >= client_topic_count ) return 0;
    chat_font_normal->align = STK_FONT_ALIGN_LEFT;
    stk_font_write( chat_font_normal, 
        buffer, 2,0, STK_OPAQUE, 
        client_topics[y] );
    return 1;
}
/*
====================================================================
Render chat line.
====================================================================
*/
static int client_render_chatter( 
    int x, int y, SDL_Surface *buffer )
{
    char *ptr = 0, *name_end = 0;
    StkFont *font = 0;
    if ( y < 0 || y >= CHAT_LINE_COUNT ) return 0;
    if ( chatter[y][0] == '!' ) {
        font = chat_font_error;
        ptr = chatter[y] + 1;
    }
    else {
        ptr = chatter[y];
        font = chat_font_normal;
    }
    font->align = STK_FONT_ALIGN_LEFT;
    stk_font_write( font, buffer, 2,0, STK_OPAQUE, ptr );
    if ( chatter[y][0] == '<' )
    if ( ( name_end = strchr( chatter[y], '>' ) ) ) {
        name_end[1] = 0;
        stk_font_write( 
            chat_font_name, buffer, 2,0, STK_OPAQUE, chatter[y] );
        name_end[1] = ' ';
    }
    return 1;
}
static int client_render_pause_chatter( 
    int x, int y, SDL_Surface *buffer )
{
    char *ptr = 0, *name_end = 0;
    StkFont *font = 0;
    if ( y < 0 || y >= CHAT_LINE_COUNT ) return 0;
    if ( pause_chatter[y][0] == '!' ) {
        font = chat_font_error;
        ptr = pause_chatter[y] + 1;
    }
    else {
        ptr = pause_chatter[y];
        font = chat_font_normal;
    }
    font->align = STK_FONT_ALIGN_LEFT;
    stk_font_write( font, buffer, 2,0, STK_OPAQUE, ptr );
    if ( pause_chatter[y][0] == '<' )
    if ( ( name_end = strchr( pause_chatter[y], '>' ) ) ) {
        name_end[1] = 0;
        stk_font_write( 
            chat_font_name, buffer, 2,0, STK_OPAQUE, pause_chatter[y] );
        name_end[1] = ' ';
    }
    return 1;
}
/*
====================================================================
Render levelsets.
====================================================================
*/
static int client_render_levelset( 
    int x, int y, SDL_Surface *buffer )
{
    if ( y >= client_levelsets->count ) return 0;
    chat_font_normal->align = STK_FONT_ALIGN_LEFT;
    stk_font_write( chat_font_normal, 
        buffer, 2,0, STK_OPAQUE, 
        list_get( client_levelsets, y ) );
    return 1;
}
/*
====================================================================
Render user
====================================================================
*/
static int client_render_user( 
    int x, int y, SDL_Surface *buffer )
{
    ClientUser *user;
    StkFont *font;
    if ( y >= client_users->count ) return 0;
    if ( ( user = list_get( client_users, y ) ) == 0 ) return 0;
    if ( user->id == client_id )
        font = chat_font_name;
    else
        font = chat_font_normal;
    font->align = STK_FONT_ALIGN_LEFT;
    stk_font_write( font, buffer, 2,0, STK_OPAQUE, user->name );
    return 1;
}
/*
====================================================================
Render channel.
====================================================================
*/
static int client_render_channel( 
    int x, int y, SDL_Surface *buffer )
{
    if ( y >= client_channels->count ) return 0;
    chat_font_normal->align = STK_FONT_ALIGN_LEFT;
    stk_font_write( chat_font_normal, 
        buffer, 2,0, STK_OPAQUE, 
        (char*)list_get( client_channels, y ) );
    return 1;
}
#endif    

/*
====================================================================
PUBLICS
====================================================================
*/

/*
====================================================================
Build the GUI and everything nescessary for the client chatroom.
client_create() installs client_delete() as exit function.
====================================================================
*/
void client_create( void )
{
#ifdef NETWORK_ENABLED
    GuiWidget *parent;
    SDL_Surface *surface;
    /* users, channels, games */
    client_data_create();
    /* CONNECT WINDOW */
    dlg_connect = gui_box_create( 0, -1,-1, 400, 224, 0, 0 );
    parent = dlg_connect;
    /* hint label */
    gui_label_create( 
        parent, 10, 10, parent->width - 20, 100, 1, 
        0, 4, 0, STK_FONT_ALIGN_LEFT, HINT_CONNECT );
    /* server edit */
    gui_label_create( 
        parent, 10, 120, 180, 20, 0, 
        0, 2, 0, STK_FONT_ALIGN_RIGHT, _("Server:") );
    edit_server = gui_edit_create(
        parent, parent->width - 190, 120, 180, 20,
        0, 2, 0, 22, config.server );
    /* user name */
    gui_label_create( 
        parent, 10, 150, 180, 20, 0, 
        0, 2, 0, STK_FONT_ALIGN_RIGHT, _("Username:") );
    edit_username = gui_edit_create(
        parent, parent->width - 190, 150, 180, 20,
        0, 2, 0, 13, config.username );
    /* cancel, connect buttons */
    gui_button_create(
        parent, parent->width/2 -160, 180, 
        140, 24, client_connect, 0,0,0,0,0, _("Connect") );
    gui_button_create(
        parent, parent->width/2 +20, 180, 
        140, 24, client_close_connect_window, 0,0,0,0,0, _("Cancel") );
    /* INFO */
    dlg_info = gui_box_create( 0, -1,-1, 300, 114, 0, 1 );
    label_info = gui_label_create( dlg_info, -1,10,276,60,0, 0,
        2, 0, STK_FONT_ALIGN_LEFT, _("no info") );
    gui_button_create( dlg_info, 
        -1, dlg_info->height - 34,
        100, 24, client_close_info, 0,0,0,0,0,
        _("Close") );
    /* CONFIRM */
    dlg_confirm = gui_box_create( 0, -1,-1, 300, 160, 0, 1 );
    label_confirm = gui_label_create( dlg_confirm, -1,10,276,110,0, 0,
        2, 0, STK_FONT_ALIGN_LEFT, _("no info") );
    gui_button_create( dlg_confirm, 
        dlg_confirm->width/2-110, dlg_confirm->height - 30,
        100, 20, client_confirm, 0,0,0,0,0,
        _("OK") );
    gui_button_create( dlg_confirm, 
        dlg_confirm->width/2+10, dlg_confirm->height - 30,
        100, 20, client_cancel, 0,0,0,0,0,
        _("Cancel") );
    /* CHATROOM */
    dlg_chatroom = gui_box_create( 0, 0,0,640,480, 
        client_recv_packet, 1 );
    gui_widget_enable_event( dlg_chatroom, GUI_TIME_PASSED );
    surface = gui_widget_get_surface( dlg_chatroom );
    stk_surface_apply_wallpaper( 
        surface, 0,0,-1,-1, cr_wallpaper, STK_OPAQUE );
    /* chat window */
    list_chatter = gui_list_create( dlg_chatroom, 0,0,470,444, NULL,
        0, 1, 14, 20, client_render_chatter, GUI_LIST_NO_SELECT );
    /* edit + send/whisper buttons */
    gui_button_create( dlg_chatroom, 330,448,60,24, 
        client_send_chatter, 0,0,0,0,0, _("Send") );
    gui_button_create( dlg_chatroom, 390,448,80,24, 
        client_whisper_chatter, 0,0,0,0,0, _("Whisper") );
    edit_chatter = gui_edit_create( dlg_chatroom, 0,448,327,24,
        client_send_chatter, 2, 0, MAX_CHATTER_SIZE, "" );
    gui_widget_enable_event( edit_chatter, GUI_KEY_RELEASED );
    gui_widget_set_default_key_widget( dlg_chatroom, edit_chatter );
    /* channel button + users in channel */
    label_channel = gui_label_create( dlg_chatroom, 
        474,0,142,18,1, NULL,
        2, 0, STK_FONT_ALIGN_CENTER_X, _("MAIN") );
    gui_button_create( dlg_chatroom, 616,0,18,18, 
        client_select_channel,
        0,0,0,0,0, _("C") );
    list_users = gui_list_create( dlg_chatroom, 
        474,20,160,142, client_handle_user_list,
        1, 1, 14, 20, client_render_user, 
        GUI_LIST_SINGLE_SELECT );
    /* levels label, levels, transfer button */
    gui_label_create( dlg_chatroom, 474,164,160,18,1, NULL,
        2, 0, STK_FONT_ALIGN_CENTER_X, _("Levelsets") );
    list_levels = gui_list_create( dlg_chatroom, 
        474,184,160,120, client_handle_levelset_list,
        1, 1, 14, 20, client_render_levelset, 
        GUI_LIST_SINGLE_SELECT );
    /* box with level settings */
    parent = gui_box_create( dlg_chatroom,
        474,306,160,94, NULL, 0 );
    gui_label_create( parent, 2,2,102,20,0, NULL,
        2, NULL, STK_FONT_ALIGN_LEFT, _("Difficulty:") );
    gui_spinbutton_create( parent, 102,2,50,20, 
        client_update_difficulty,
        1, 3, 1, config.mp_diff+1 );
    gui_label_create( parent, 2,2+22,102,20,0, NULL,
        2, NULL, STK_FONT_ALIGN_LEFT, _("Rounds:") );
    gui_spinbutton_create( parent, 102,2+22,50,20,
        client_update_rounds,
        1, 3, 1, config.mp_rounds );
    gui_label_create( parent, 2,2+44,102,20,0, NULL,
        2, NULL, STK_FONT_ALIGN_LEFT, _("Frags:") );
    gui_spinbutton_create( parent, 102,2+44,50,20, 
        client_update_frags,
        5, 30, 5, config.mp_frags );
    gui_label_create( parent, 2,2+66,102,20,0, NULL,
        2, NULL, STK_FONT_ALIGN_LEFT, _("Balls:") );
    gui_spinbutton_create( parent, 102,2+66,50,20,
        client_update_balls,
        1, 6, 1, config.mp_balls );
    /* challenge & disconnect */
    gui_button_create( dlg_chatroom, 
        474,402,160,20, client_challenge,
        0,0,0,0,0, _("Challenge") );
    gui_button_create( dlg_chatroom,
	474,dlg_chatroom->height-44,160,20,
	client_open_connect_window,
	0,0,0,0,0, _("Connect") );
    gui_button_create( dlg_chatroom, 
        534,dlg_chatroom->height-20,100,20, 
        client_quit,
        0,0,0,0,0, _("Quit") );
    gui_button_create( dlg_chatroom, 
        474,dlg_chatroom->height-20,50,20, 
        client_popup_help,
        0,0,0,0,0, _("Help") );
    /* CHANNELS selector */
    dlg_channels = gui_box_create( 0, -1,-1, 200, 244, NULL, 1 );
    list_channels = gui_list_create( dlg_channels, -1,10, 176,150, 
        client_handle_channel_list, 0, 1, 14, 20, 
        client_render_channel, GUI_LIST_SINGLE_SELECT );
    gui_label_create( dlg_channels, -1,160, 176,20,0, NULL,
        2, 0, STK_FONT_ALIGN_LEFT, _("Channel:") );
    edit_channel = gui_edit_create( dlg_channels, 
        -1,180, 176,20, NULL,
        2, 0, 12, "" );
    gui_widget_set_default_key_widget( dlg_channels, edit_channel );
    gui_edit_set_filter( edit_channel, GUI_EDIT_ALPHANUMERICAL2 );
    gui_button_create( dlg_channels, 
        dlg_channels->width/2-90,210,80,20, 
        client_enter_channel, 0,0,0,0,0, _("Enter") );
    gui_button_create( dlg_channels, 
        dlg_channels->width/2+10,210,80,20, 
        client_cancel_channel, 0,0,0,0,0, _("Cancel") );
    /* STATISTICS */
    dlg_stats = gui_box_create( 0, -1,-1,386,260, NULL, 1 );
    gui_label_create( dlg_stats, -1,10,360,20,1, NULL,
        0, 0, STK_FONT_ALIGN_CENTER_X, _("Statistics") );
    label_winner = gui_label_create( dlg_stats, 
        -1,40,360,16,0, NULL,
        0, 0, STK_FONT_ALIGN_CENTER_X, "..." );
    label_stats = gui_label_create( dlg_stats, 
        -1,62,360,150,0, NULL,
        0, 0, STK_FONT_ALIGN_LEFT, _("Awaiting stats...") );
    gui_button_create( dlg_stats, -1,220,160,20,
        client_close_stats, 0,0,0,0,0, _("Close") );
    /* PAUSEROOM */
    dlg_pauseroom = gui_box_create( 0, -1,-1,480,366, NULL, 1 );
    list_pausechatter = gui_list_create( 
        dlg_pauseroom, 0,0,470,300, NULL,
        0, 1, 14, 20, client_render_pause_chatter, 
        GUI_LIST_NO_SELECT );
    edit_pausechatter = gui_edit_create( 
        dlg_pauseroom, 0,303,470,24,
        client_send_pausechatter, 2, 0, MAX_CHATTER_SIZE, "" );
    gui_widget_enable_event( edit_pausechatter, GUI_KEY_RELEASED );
    gui_widget_set_default_key_widget( dlg_pauseroom, edit_pausechatter );
    gui_button_create( dlg_pauseroom, -1,334,100,20,
        client_close_pauseroom, 0,0,0,0,0, _("Close") );
    /* HELP */
    dlg_help = gui_box_create( 0, -1,-1,548,358, NULL, 1 );
    gui_label_create( dlg_help, -1,10,470,20,1, NULL,
        0, 0, STK_FONT_ALIGN_CENTER_X, _("Network Quick Help") );
    list_topic = gui_list_create( 
        dlg_help, 10,40,160,270, client_handle_topic_list,
        0, 1, 14, 20, client_render_topic,
        GUI_LIST_SINGLE_SELECT );
    list_help = gui_list_create( 
        dlg_help, 180,40,350,300, NULL,
        0, 1, 14, 20, client_render_help, 
        GUI_LIST_NO_SELECT );
    gui_button_create( dlg_help, 10,320,160,20, client_close_help,
        0,0,0,0,0, _("Close") );
    /* INITS */
    /* empty chatter */
    memset( chatter, 0, sizeof( chatter ) );
    /* update chatboxes */
    gui_list_update( list_chatter, CHAT_LINE_COUNT );
    gui_list_update( list_pausechatter, CHAT_LINE_COUNT );
    gui_list_goto( list_chatter, -1 );
    gui_list_goto( list_pausechatter, -1 );
    /* set help topics */
    gui_list_update( list_topic, client_topic_count );
    /* call client_delete at exit */
    atexit( client_delete );
#endif
}

/*
====================================================================
Free memory allocated by client_create( void )
====================================================================
*/
void client_delete( void )
{
#ifdef NETWORK_ENABLED
    client_data_delete();
    printf( _("Client finalized\n") );
#endif
}

/*
====================================================================
Run the connect dialogue & chatroom until user closes the connect
dialogue.
====================================================================
*/
#ifdef NETWORK_ENABLED
static int client_printf_info( const char *format, ... )
{
	va_list args;
	char string[MAX_CHATTER_SIZE];
	
	va_start( args, format );
	vsnprintf( string, MAX_CHATTER_SIZE, format, args );
	va_end( args );
	
	client_add_chatter( string, 1 );
	return strlen(string);
}
#endif
void client_run( void )
{
#ifdef NETWORK_ENABLED
	/* init UDP socket */
	sys_printf = client_printf_info;
	net_init( atoi(config.local_port) );
	sys_printf = printf;
	client_add_chatter( _("Press 'Connect' to connect to a game server."), 1 );

	client_state = CLIENT_NONE;
	client_recv_limit = config.recv_limit;

	gui_enable_event_filter();
	gui_widget_show( dlg_chatroom );
	stk_display_fade( STK_FADE_IN, STK_FADE_DEFAULT_TIME );
	gui_run( dlg_chatroom );
	stk_display_fade( STK_FADE_OUT, STK_FADE_DEFAULT_TIME );
	gui_disable_event_filter();

	if ( client_is_connected ) {
		/* tell server we are off */
		client_disconnect();
		/* finalize UDP socket */
		socket_print_stats( &client );
	}
	net_shutdown();
#endif
}

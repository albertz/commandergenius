/***************************************************************************
                          client_handlers.h  -  description
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
 
#ifndef __CLIENT_HANDLERS_H
#define __CLIENT_HANDLERS_H

/*
====================================================================
Disconnect from current server if any.
====================================================================
*/
void client_disconnect();
	
/*
====================================================================
Try to connect to a game server. Retry twice every three seconds
or quit then.
====================================================================
*/
void client_connect( GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Open/close the connection window.
====================================================================
*/
void client_open_connect_window( GuiWidget *widget, GuiEvent *event );
void client_close_connect_window( GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Close chatroom and return to LBreakout's menu.
====================================================================
*/
void client_quit( GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Close the info window and clear state.
====================================================================
*/
void client_close_info( GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Send chatter this function is either called by the send button
or by the edit.
====================================================================
*/
void client_send_chatter( 
    GuiWidget *widget, GuiEvent *event );
/*
====================================================================
Whisper chatter if a user is selected.
====================================================================
*/
void client_whisper_chatter( 
    GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Update selected peer and levelset.
====================================================================
*/
void client_handle_user_list( 
    GuiWidget *widget, GuiEvent *event );
void client_handle_levelset_list( 
    GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Handle confirmation/cancelling of confirmation dialogue.
====================================================================
*/
void client_confirm( GuiWidget *widget, GuiEvent *event );
void client_cancel( GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Challenge selected user.
====================================================================
*/
void client_challenge( GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Update multiplayer network configuration.
====================================================================
*/
void client_update_difficulty( GuiWidget *widget, GuiEvent *event );
void client_update_rounds( GuiWidget *widget, GuiEvent *event );
void client_update_frags( GuiWidget *widget, GuiEvent *event );
void client_update_balls( GuiWidget *widget, GuiEvent *event );
void client_update_port( GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Allow user to transfer a levelset.
====================================================================
*/
void client_listen( GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Initiate levelset transfer.
====================================================================
*/
void client_transfer( GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Open channel selector
====================================================================
*/
void client_select_channel( GuiWidget *widget, GuiEvent *event );
/*
====================================================================
Handle channel (un)selection.
====================================================================
*/
void client_handle_channel_list( 
    GuiWidget *widget, GuiEvent *event );
/*
====================================================================
Close channel selector or enter new channel.
====================================================================
*/
void client_enter_channel( GuiWidget *widget, GuiEvent *event );
void client_cancel_channel( GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Close statistics
====================================================================
*/
void client_close_stats( GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Send chatter to gamepeer in pauseroom when ENTER was pressed.
====================================================================
*/
void client_send_pausechatter( GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Close pauseroom.
====================================================================
*/
void client_close_pauseroom( GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Popup help dialogue.
====================================================================
*/
void client_popup_help( GuiWidget *widget, GuiEvent *event );
/*
====================================================================
Close help dialogue.
====================================================================
*/
void client_close_help( GuiWidget *widget, GuiEvent *event );
/*
====================================================================
Select topic and display help text.
====================================================================
*/
void client_handle_topic_list( GuiWidget *widget, GuiEvent *event );

#endif


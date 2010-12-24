/***************************************************************************
                          client_data.h  -  description
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

#ifndef __CLIENT_DATA_H
#define __CLIENT_DATA_H

/*
====================================================================
Client states
====================================================================
*/
enum {
    CLIENT_NONE = 0,

    /* ingame stats lie in between */
    
    CLIENT_INFO = 1000, /* not open to any challenges/transfers */
    CLIENT_AWAIT_ANSWER, /* wait for answer to a challenge */
    CLIENT_ANSWER, /* answer to a challenge */
    CLIENT_CONFIRM_TRANSFER, /* say yes or no to transfer */
    CLIENT_AWAIT_TRANSFER_CONFIRMATION, /* wait for answer on 
                                           transfer offer */
    CLIENT_RECEIVE, /* receive level data */
    CLIENT_LISTEN, /* listen to user for a levelset */
    CLIENT_SELECT_CHANNEL, /* selecting a channel */
    CLIENT_STATS, /* looking at game stats */
    CLIENT_PLAY, /* playing game */
    CLIENT_HELP /* looking at help */
};

/*
====================================================================
Chatter definitions.
====================================================================
*/
enum { 
    CHAT_LINE_COUNT = 200,
    CHAT_LINE_WIDTH = 64, /* includes \0 */
    MAX_CHATTER_SIZE = 100

};

/*
====================================================================
Client data structs
====================================================================
*/
typedef struct {
    int  id;
    char name[16];
} ClientUser;

/* transmit via client's socket if client_is_connected is True */
void client_transmit( int type, int len, char *data );

/*
====================================================================
Create/delete client's data structs.
====================================================================
*/
void client_data_create( void );
void client_data_delete( void );

/*
====================================================================
Clear all data structs
====================================================================
*/
void client_data_clear( void );

/*
====================================================================
Add/remove/find users/games/channels. Do not update the GUI.
====================================================================
*/
void client_add_user( int id, char *name );
void client_remove_user( int id );
ClientUser* client_find_user( int id );

/*
====================================================================
Add chatter to chat window. If 'info' is true the text is
displayed red and without indention.
====================================================================
*/
void client_add_chatter( char *string, int info );

/*
====================================================================
Add chatter to chat window. If 'info' is true the text is
displayed red and without indention.
====================================================================
*/
void client_printf_chatter( int info, char *format, ... );

/*
====================================================================
Add chatter to pause chat window. If 'info' is true the text is
displayed red and without indention.
====================================================================
*/
void client_add_pausechatter( char *string, int info );

#endif

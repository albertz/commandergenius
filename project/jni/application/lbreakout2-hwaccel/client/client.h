/***************************************************************************
                          client.h  -  description
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

#ifndef __CLIENT_H
#define __CLIENT_H

/*
====================================================================
This module provides the client GUI (connect dialogue & chatroom)
which is run by calling client_run()
====================================================================
*/

/*
====================================================================
Build the GUI and everything nescessary for the client chatroom.
client_create() installs client_delete() as exit function.
====================================================================
*/
void client_create( void );

/*
====================================================================
Free memory allocated by client_create( void )
====================================================================
*/
void client_delete( void );

/*
====================================================================
Run the connect dialogue & chatroom until user closes the connect
dialogue.
====================================================================
*/
void client_run( void );

#endif

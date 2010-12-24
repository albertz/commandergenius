/***************************************************************************
                          comm.h  -  description
                             -------------------
    begin                : Fri Aug 2 2002
    copyright            : (C) 2001 by Michael Speck
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

#ifndef __COMM_CLIENT_H
#define __COMM_CLIENT_H

void comm_send_paddle( Paddle *paddle );
void comm_send_short( int msg ); /* single byte messages */

/* receive any data but only handle such messages valid
 * in the current state */
void comm_recv( void );

/* send a heartbeat every 2 seconds except in the actual game as we
 * send paddle updates there */
void comm_send_heartbeat( void );

#endif

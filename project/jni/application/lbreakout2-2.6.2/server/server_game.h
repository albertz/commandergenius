/***************************************************************************
                          server_game.h  -  description
                             -------------------
    begin                : 03/03/19
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

#ifndef __SERVER_GAME_H
#define __SERVER_GAME_H

/***** INCLUDES ************************************************************/

/***** TYPE DEFINITIONS ****************************************************/

/***** PUBLIC FUNCTIONS ****************************************************/

/* Add a new game by the context information, hide both users
 * and send a challenge message to the challenged user. */
void server_game_add( ServerChannel *channel, ServerGameCtx *ctx );

/* Free game memory. */
void server_game_delete( void *ptr );

/* unhide the users to their chat channel and delete the game.
 * if game was beyond state AWAIT_ACCEPT the game stats are send
 */
void server_game_remove( ServerGame *game );

/* void parse_packet_game
 * IN	ServerGame	*game
 * IN	ServerUser		*user
 *
 * Check all messages in packet from user who is currently within
 * a game. The header has already been successfully processed and the 
 * read pointer is at the beginning of the first message.
 */
void parse_packet_game( ServerGame *game, ServerUser *user );

/* void update_games
 * IN	int	ms	milliseconds passed since last call
 *
 * Update the objects of all games that are actually playing.
 */
void update_games( int ms );
	
#endif


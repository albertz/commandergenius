/***************************************************************************
                          player.c  -  description
                             -------------------
    begin                : Thu Sep 6 2001
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

#include "../client/lbreakout.h"
#include "../game/levels.h"
#include "player.h"

int current_player = 0;
int player_count = 0;
Player players[MAX_PLAYERS];

/*
====================================================================
Add this player to the list and increase the counter until
MAX_PLAYERS is reached.
Return Value: True if successful
====================================================================
*/
int player_add( char *name, int lives, Level *level )
{
	if ( player_count == MAX_PLAYERS ) return 0;
	memset( &players[player_count], 0, sizeof( Player ) );
	strcpy( players[player_count].name, name );
	players[player_count].lives = lives;
	player_init_level( &players[player_count], level, 0 );
	player_count++;
	return 1;
}
/*
====================================================================
Get first player.
Return Value: first player in list
====================================================================
*/
Player* players_get_first()
{
	current_player = -1;
	return players_get_next();
}
/*
====================================================================
Get next player in list (cycle: return first player after
last player).
Return Value: current player
====================================================================
*/
Player* players_get_next()
{
	if ( players_count() == 0 ) return 0;
	do {
		current_player++;
		if ( current_player == player_count ) current_player = 0;
	}
	while ( players[current_player].lives == 0 );
	return &players[current_player];
}
/*
====================================================================
player_count players give id's 0,1,...,player_count-1. Select
the player with id 'current' as current player. The id used is 
the absolute one, not the relative one resulting from dead players.
Return Value: current player
====================================================================
*/
Player* players_set_current( int current )
{
    if ( current < 0 || current >= player_count ) return 0;
    current_player = current;
	return &players[current_player];
}
/*
====================================================================
Reset player counter.
====================================================================
*/
void players_clear()
{
	player_count = 0;
}
/*
====================================================================
Return number of players still in game (lives > 0)
====================================================================
*/
int players_count()
{
	int i;
	int count = 0;
	for ( i = 0; i < player_count; i++ )
		if ( players[i].lives > 0 )
			count++;
	return count;
}

/* set level_id and init snapshot with the new leveldata */
void player_init_level( Player *player, Level *level, int id )
{
	player->level_id = id;
	player->snapshot = *level;
}



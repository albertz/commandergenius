/***************************************************************************
                          player.h  -  description
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

/*
====================================================================
Add this player to the list and increase the counter until
MAX_PLAYERS is reached.
Return Value: True if successful
====================================================================
*/
int player_add( char *name, int lives, Level *level );
/*
====================================================================
Get first player. (and set current id to 0)
Return Value: first player in list
====================================================================
*/
Player* players_get_first();
/*
====================================================================
Get next player in list (cycle: return first player after
last player, update current id).
Return Value: current player
====================================================================
*/
Player* players_get_next();
/*
====================================================================
player_count players give id's 0,1,...,player_count-1. Select
the player with id 'current' as current player. The id used is 
the absolute one, not the relative one resulting from dead players.
Return Value: current player
====================================================================
*/
Player* players_set_current( int current );
/*
====================================================================
Reset player counter.
====================================================================
*/
void players_clear();
/*
====================================================================
Return number of players still in game (lives > 0)
====================================================================
*/
int players_count();

/* set level_id and init snapshot with the new leveldata */
void player_init_level( Player *player, Level *level, int id );



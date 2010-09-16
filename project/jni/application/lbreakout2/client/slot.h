/***************************************************************************
                          slot.h  -  description
                             -------------------
    begin                : Sat Dec 15 2001
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

#ifndef __SLOT_H
#define __SLOT_H

#include "hint.h"

typedef struct {
    char setname[32];
    int  diff;
    int  player_count; /* 0 - MAX_PLAYERS-1 */
    char player_names[MAX_PLAYERS][32];
    int  player_cur_level_id[MAX_PLAYERS];
    int  cur_player; /* 0 - MAX_PLAYERS-1 */
    int  player_scores[MAX_PLAYERS];
    int  player_lives[MAX_PLAYERS];
    int  gameSeed; /* for bonus levels and FREAKOUT order */
} GameSlot;

/*
====================================================================
Load/save a game to a slot. 'slot_load' returns 1 on success and 
updates gs. 'slot_save' takes content of 'gs' and saves it.
'slot_delete' deletes the savegame.
====================================================================
*/
int slot_load( int slot_id, GameSlot *gs );
int slot_save( int slot_id, GameSlot *gs );
int slot_delete( int slot_id );

/*
====================================================================
Build info hint from game saved to slot 'slot_id'.
====================================================================
*/
void slot_update_hint( int slot_id, Hint *hint );

#endif

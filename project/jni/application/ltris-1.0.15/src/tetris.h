/***************************************************************************
                          tetris.h  -  description
                             -------------------
    begin                : Tue Dec 25 2001
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

#ifndef __TETRIS_H
#define __TETRIS_H

/*
====================================================================
Load/delete all tetris resources.
====================================================================
*/
void tetris_create();
void tetris_delete();

/*
====================================================================
Initiate/clear a new game from config data.
After tetris_init() the screen is drawn completely though not
updated to use the fade effect.
====================================================================
*/
int  tetris_init();
void tetris_clear();

/*
====================================================================
Run an successfully initated game.
====================================================================
*/
void tetris_run();

/*
====================================================================
Run a number of CPU games to get an average score gained so you'll
see if your analyze algorithm in cpu.c cpu_analyze_bowl() sucks
or rocks!
====================================================================
*/
void tetris_make_stat();

#endif

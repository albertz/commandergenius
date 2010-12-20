/***************************************************************************
                          shrapnells.h  -  description
                             -------------------
    begin                : Sat Sep 8 2001
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
Create high_res or low_res shrapnells.
====================================================================
*/
enum {
    SHR_TYPE_COUNT = 7
};

typedef struct {
    float x, y;
} Vector;

typedef struct {
    SDL_Surface *pic;
    Vector      v;
    Vector      g; /* gravity */
    float       x, y;
    float       alpha;
} Shrapnell;

/*
====================================================================
Initate or delete shrapnell list
====================================================================
*/
void shrapnells_init();
void shrapnells_delete();

/*
====================================================================
Create a single shrapnell and add to list.
====================================================================
*/
void shrapnell_create( int x, int y, int w, int h, float vx, float vy, float gx, float gy );
/*
====================================================================
Create shrapnells from position in offscreen with the size
of bricks
====================================================================
*/
void shrapnells_create( int x, int y, int w, int h, int type );
/*
====================================================================
Remove all shrapnells
====================================================================
*/
void shrapnells_reset();
/*
====================================================================
Show, hide all shrapnells.
====================================================================
*/
void shrapnells_hide();
void shrapnells_show();
/*
====================================================================
Update and remove expired shrapnells
====================================================================
*/
void shrapnells_update( int ms );

/*
====================================================================
Brick explosion animations.
====================================================================
*/
void exp_load();
void exp_delete();
void exps_clear();
void exp_create( int x, int y );
void exps_hide();
void exps_show();
void exps_update( int ms );

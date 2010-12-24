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

typedef struct {
    SDL_Surface *pic;
    Vector      v;
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
Create shrapnells from surface (surf,sx,sy,sw,sh) and put it to
screen position (px,py). The vector imp is the 
impuls and type the type of what caused the destruction.
====================================================================
*/
void shrapnells_create( SDL_Surface *surf,
    int sx, int sy, int sw, int sh, int px, int py, 
    int type, Vector imp );
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
void exps_set_dark( int dark );

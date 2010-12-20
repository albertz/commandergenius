/***************************************************************************
                          hint.h  -  description
                             -------------------
    begin                : Sun Jan 6 2002
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
 
#ifndef __HINT_H
#define __HINT_H

typedef struct {
    Text *text; /* contents */
    int sx, sy, sw, sh; /* screen geometry */
} Hint;

/*
====================================================================
Load/delete hint resources.
====================================================================
*/
void hint_load_res();
void hint_delete_res();

/*
====================================================================
Create a hint. The passed x,y position is where the end of the
Sprechblasenspitze is. Contents is converted to lines (duplicated).
====================================================================
*/
Hint* hint_create( int x, int y, char *contents );
void hint_delete( Hint *hint );

/*
====================================================================
Set the background on which the hints will be displayed.
====================================================================
*/
void hint_set_bkgnd( SDL_Surface *bkgnd );

/*
====================================================================
Set this hint as actual one. If NULL was passed no hint will be
handled.
====================================================================
*/
void hint_set( Hint *hint );

/*
====================================================================
Set hint to this position.
====================================================================
*/
void hint_set_pos( Hint *hint, int x, int y );

/*
====================================================================
Show/hide/update _current_ hint.
====================================================================
*/
void hint_hide();
void hint_show();
void hint_update( int ms );

#endif

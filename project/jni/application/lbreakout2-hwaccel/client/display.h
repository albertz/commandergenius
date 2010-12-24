/***************************************************************************
                          display.h  -  description
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

#ifndef __DISPLAY_H
#define __DISPLAY_H

typedef struct {
    SDL_Surface *surf;
    int x, y, w, h;
    char text[32];
    int digits;
    int  value;
    float cur_value; /* smoothly approximates destination 'value' */
    int use_alpha; /* if true we darken the background a bit. this 
                      is true by default */
  int is_highlighted; /* use highlight font */
} Display;

/*
====================================================================
Add a display. The display is cleared by displays_clear() so
the returned display is just a pointer.
The contents of the display is 'text'+'value'. Both may be changed
with the display_set_text/value() functions.
'digits' is the maxium size of 'value'
====================================================================
*/
Display* displays_add( int x, int y, int w, int h, char *text, int value, int digits );
/*
====================================================================
Remove all displays
====================================================================
*/
void displays_clear();

/*
====================================================================
Draw displays
====================================================================
*/
void displays_hide();
void displays_show();

/*
====================================================================
Update displays
====================================================================
*/
void displays_update( int ms );

/*
====================================================================
Modify display
====================================================================
*/
void display_set_text( Display *display, char *text );
void display_set_value( Display *display, int value );
void display_set_value_directly( Display *display, int value );
void display_set_highlight( Display *display, int on );

#endif

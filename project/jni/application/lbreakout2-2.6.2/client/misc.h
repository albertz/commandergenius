/***************************************************************************
                          misc.h  -  description
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

#ifndef __MISC_H
#define __MISC_H

/*
====================================================================
Target of a shot or a ball.
====================================================================
*/
enum { SIDE_UNDEFINED = -1, SIDE_TOP = 0, SIDE_RIGHT, SIDE_BOTTOM, SIDE_LEFT, 
							CORNER_UPPER_LEFT, CORNER_UPPER_RIGHT, 
							CORNER_LOWER_LEFT, CORNER_LOWER_RIGHT };
/*
====================================================================
Load background according to id and draw background to offscreen.
Return Value: loaded background surface
====================================================================
*/
void bkgnd_draw( SDL_Surface *bkgnd, int id );
/*
====================================================================
Confirm request. Darkens/gray screen a bit and display text.
Return Value: True if successful
====================================================================
*/
enum{ CONFIRM_YES_NO, CONFIRM_ANY_KEY, CONFIRM_PAUSE };
int confirm( StkFont *font, char *str, int type );
/*
====================================================================
Display a info message (gray screen a bit and display text), 
send a MSG_READY when player has clicked and wait for a remote answer
(timeout 10 secs). Waiting may be cancelled by pressing ESCAPE which
results in sending a MSG_GAME_EXITED.
Return Value: True if both peers clicked to continue, False if the
connection was cancelled for some reason.
====================================================================
*/
int display_info( StkFont *font, char *str, NetSocket *peer );
/*
====================================================================
Create shadow surface for specified region in surface.
Return Value: Shadow surface
====================================================================
*/
SDL_Surface* create_shadow( SDL_Surface *surf, int x, int y, int w, int h );

/*
====================================================================
Enter a string and return True if ENTER received and False
if ESCAPE received.
====================================================================
*/
int enter_string( StkFont *font, char *caption, char *edit, int limit );

/*
====================================================================
Display text blinking.
====================================================================
*/
void write_text_with_cursor( StkFont *sfnt, SDL_Surface *dest,
    int x, int y, char *str, int alpha);

/*
====================================================================
Enter nuke mode and allow player to disintegrate single bricks
by spending 5% of his/her score.
====================================================================
*/
void game_nuke( void );

/* gray screen and display a formatted text, directly update the
 * screen */
void display_text( StkFont *font, char *format, ... );

#endif

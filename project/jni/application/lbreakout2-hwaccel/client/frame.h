/***************************************************************************
                          frame.h  -  description
                             -------------------
    begin                : Fri Sep 7 2001
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
This file handles all the graphical stuff concering the frame
including drawing of score, lifes and bonus info.
====================================================================
*/

/*
====================================================================
Create frame by loading and assembling all graphics and loading
additional graphics.
====================================================================
*/
void frame_create();
/*
====================================================================
Inititate the frame to the according game type.
Set the position of the extras displayed, enable life lamps etc.
====================================================================
*/
void frame_init();
/*
====================================================================
Free all resources created by frame_create()
====================================================================
*/
void frame_delete();
/*
====================================================================
Draw frame to offscreen.
====================================================================
*/
void frame_draw();
/*
====================================================================
Add life lamps at left side of frame in offscreen
====================================================================
*/
void frame_draw_lives( int lives, int max_lives );
/*
====================================================================
Add one new life at offscreen and screen (as this will happen in
game )
====================================================================
*/
void frame_add_life();
/*
====================================================================
Switch of a life lamp
====================================================================
*/
void frame_remove_life();
/*
====================================================================
Display extra information on right side of screen.
====================================================================
*/
void frame_info_hide();
void frame_info_show();

/*
====================================================================
Blink the warp icon.
====================================================================
*/
void frame_warp_icon_hide();
void frame_warp_icon_show();
void frame_warp_icon_update( int ms );

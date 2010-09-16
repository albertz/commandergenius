/***************************************************************************
                          bricks.h  -  description
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
Draw all bricks to offscreen surface.
====================================================================
*/
void bricks_draw();
/*
====================================================================
Add brick with clipped shadow to offscreen. To draw a brick without 
shadow check use brick_draw().
====================================================================
*/
void brick_draw_complex( int mx, int my, int px, int py );
/*
====================================================================
Draw brick to passed surface
====================================================================
*/
void brick_draw( SDL_Surface *surf, int map_x, int map_y, int shadow );

/* handle a received brick action */
void client_handle_brick_hit( BrickHit *hit );

/* redraw all bricks */
void client_redraw_all_bricks();

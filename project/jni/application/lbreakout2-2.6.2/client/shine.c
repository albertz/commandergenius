/***************************************************************************
                          shine.c  -  description
                             -------------------
    begin                : Thu Sep 13 2001
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

#include "lbreakout.h"
#include "config.h"

extern 	SDL_Surface *stk_display;
extern 	Config config;
extern 	SDL_Surface *offscreen;
extern 	SDL_Surface *shine_pic;
extern  Game *game;
float   shine_change = 0.020; /* per ms */
int     shine_frame = 6; /* maximum frame */
float   shine_cur = 0; /* current frame */
int     shine_x, shine_y; /* position where current shine is drawn */
int     shine_recreate = 1;
Delay   shine_delay; /* delay between shines */

/*
====================================================================
Load/delete resources
====================================================================
*/
void shine_load()
{
	delay_set( &shine_delay, 200 );
}
void shine_delete()
{
}
/*
====================================================================
Recreate shine on a random but valid brick
====================================================================
*/
void shine_create()
{
	int x_add, y_add, x, y;
	shine_cur = 0;
	shine_x = 0; shine_y = 0;

	if ( !config.anim ) return;

	x = (rand() % (BRICK_WIDTH - 2)) + 1;
	y = (rand() % (BRICK_HEIGHT - 2)) + 1;
	x_add = rand() % 2 == 0 ? 1 : -1;
	y_add = rand() % 2 == 0 ? 1 : -1;

	while (x > 0 && x < MAP_WIDTH - 1 && y > 0 && y < MAP_HEIGHT - 1) {
		if (game->bricks[x][y].type != MAP_EMPTY && game->bricks[x][y].id != INVIS_BRICK_ID ) {
			shine_x = x * BRICK_WIDTH;
			shine_y = y * BRICK_HEIGHT;
			break;
		}
		x += x_add; y += y_add;
	}
	/* if creation succeeded don't create any more shines */
	if ( shine_x != 0 && shine_y != 0 ) shine_recreate = 0;
}
/*
====================================================================
Reset (delete( shine
====================================================================
*/
void shine_reset()
{
	shine_x = shine_y = 0;
	shine_recreate = 1;
	delay_reset( &shine_delay );
}
/*
====================================================================
Show/hide current shine
====================================================================
*/
void shine_hide()
{
	if (shine_x == 0 && shine_y == 0) return;
	stk_surface_blit( offscreen, shine_x, shine_y,
			BRICK_WIDTH, BRICK_HEIGHT,
			stk_display, shine_x, shine_y );
	stk_display_store_drect();
}
void shine_show()
{
	if (shine_x == 0 && shine_y == 0) return;
	stk_surface_blit( shine_pic, (int)shine_cur * BRICK_WIDTH, 0,
			BRICK_WIDTH, BRICK_HEIGHT,
			stk_display, shine_x, shine_y );
}
/*
   ====================================================================
   Update shine
   ====================================================================
   */
void shine_update( int ms )
{
	/* recreate shine? */
	if ( shine_recreate ) {
		shine_x = shine_y = 0; /* if recreation is demanded delete old shine */
		if ( delay_timed_out( &shine_delay, ms ) ) shine_create();
	}
	else {
		shine_cur += shine_change * ms;
		if (shine_cur > shine_frame) {
			shine_recreate = 1;
			delay_reset( &shine_delay );
		}
	}
}

/***************************************************************************
                          shots.c  -  description
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

#include "lbreakout.h"
#include "../game/game.h"

extern SDL_Surface *shot_pic;
extern SDL_Surface *shot_shadow;
extern int shot_w, shot_h;
extern float shot_v_y;
float shot_cur_fr = 0.0;
int shot_fr_num = 4;
float shot_fpms = 0.01;
int shot_alpha = 128;
extern SDL_Surface *offscreen;
extern SDL_Surface *stk_display;
extern SDL_Rect stk_drect;
extern int shadow_size;
extern Game *game;

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Hide and show shots
====================================================================
*/
void shots_hide()
{
	ListEntry  *entry = game->shots->head->next;
	Shot        *shot;
	while ( entry != game->shots->tail ) {
		shot = entry->item;
		entry = entry->next;
		stk_surface_blit( offscreen, 
				(int)shot->x,(int)shot->y,
				shot_w + shadow_size,shot_h + shadow_size,
				stk_display, (int)shot->x,(int)shot->y );
		stk_display_store_rect( &shot->update_rect );
	}
}
void shots_show()
{
	ListEntry  *entry = game->shots->head->next;
	Shot       *shot;
	int        x, y;
	stk_surface_clip( stk_display, 0, 0, 
			stk_display->w - BRICK_WIDTH, stk_display->h );
	while ( entry != game->shots->tail ) {
		shot = entry->item;
		entry = entry->next;
		x = (int)shot->x; y = (int)shot->y;
		stk_surface_alpha_blit( shot_shadow, 
				0, 0, shot_w, shot_h, 
				stk_display, x + shadow_size, y + shadow_size,
				SHADOW_ALPHA );
		stk_surface_alpha_blit( shot_pic, (int)shot->cur_fr * shot_w, 0,
				shot_w, shot_h, stk_display, x, y, shot_alpha );

		shot->update_rect.x = x;
		shot->update_rect.y = y;
		shot->update_rect.w = shot_w + shadow_size;
		shot->update_rect.h = shot_h + shadow_size;
		stk_display_store_rect( &shot->update_rect );
	}
	stk_surface_clip( stk_display, 0, 0, 0, 0 );
}
void shots_alphashow( int alpha )
{
    ListEntry  *entry = game->shots->head->next;
    Shot        *shot;
    while ( entry != game->shots->tail ) {
        shot = entry->item;
        stk_surface_alpha_blit( shot_pic, (int)shot->cur_fr * shot_w, 0,
            shot_w, shot_h, stk_display, (int)shot->x, (int)shot->y,
            alpha );
        stk_display_store_rect( &shot->update_rect );
        entry = entry->next;
    }
}

/* update animation of shots */
void client_shots_update( int ms )
{
	Shot *shot;

	shot_cur_fr += ms * shot_fpms;
	if (shot_cur_fr >= shot_fr_num) 
		shot_cur_fr -= shot_fr_num;
	
	list_reset( game->shots );
	while ( (shot = list_next(game->shots)) )
		shot->cur_fr = shot_cur_fr; /* else there would be no animation 
					       as reset the list with each communicator
					       call */
}



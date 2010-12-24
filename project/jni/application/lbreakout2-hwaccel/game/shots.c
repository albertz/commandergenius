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

#include "../client/lbreakout.h"
#include "mathfuncs.h"
#include "balls.h"
#include "bricks.h"

int shot_w = 10;
int shot_h = 10;
float shot_v_y = 0.2;
extern Game *cur_game;

int last_shot_fire_x = -1; /* HACK: used to play local sound */

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Compute target of shot.
====================================================================
*/
void shot_get_target( Shot *shot ) {
    int mx = (int)(shot->x + 3) / BRICK_WIDTH;
    int my = (int)(shot->y + 3 + ((shot->dir==1)?5:0) ) / BRICK_HEIGHT;
    memset( &shot->target, 0, sizeof(Target) );
    while ( cur_game->bricks[mx][my].id == -1 ) 
        my += shot->dir;
    shot->target.mx = mx; shot->target.my = my;
    mx = (int)(shot->x + 6) / BRICK_WIDTH;
    if (mx != shot->target.mx) {
        my = (int)(shot->y + 3 + ((shot->dir==1)?5:0) ) / BRICK_HEIGHT;
        while(cur_game->bricks[mx][my].id == -1) 
            my += shot->dir;
        if (my == shot->target.my)
            shot->next_too = 1;
        else
            if ( ( shot->dir == -1 && my > shot->target.my ) ||
                 ( shot->dir ==  1 && my < shot->target.my ) ) {
                shot->target.mx = mx;
                shot->target.my = my;
                shot->next_too = 0;
            }
    }
    shot->target.cur_tm = 0;
    shot->target.time = abs( 
		(int)((shot->y + 3 + 
		((shot->dir==1)?5:0) - 
		(shot->target.my * BRICK_HEIGHT + ((shot->dir==-1)?(BRICK_HEIGHT - 1):0) )) / 
		fabs(shot_v_y)) );
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Create new shot at position (centered).
'signum' of direction determines into which direction the shot 
vertically goes.
====================================================================
*/
void shot_create( Paddle *paddle )
{
    Shot *shot = calloc( 1, sizeof( Shot ) );
    shot->cur_fr = 0;
    shot->paddle = paddle;
    shot->dir = (paddle->type == PADDLE_TOP) ? 1 : -1;
    shot->x = paddle->x + ( paddle->w >> 1 ) - (shot_w >> 1);
    shot->y = paddle->y + ( paddle->h >> 1 ) - (shot_h >> 1);
    shot->get_target = 1;
    list_add( cur_game->shots, shot );

    cur_game->mod.fired_shot_count++;
    last_shot_fire_x = shot->x; /* HACK: used to play local sound */
}

/*
====================================================================
Set 'get_target' flag so target is updated next time 
'shots_update' is called. -1 means to update all shots.
====================================================================
*/
void shots_check_targets( int mx, int my )
{
    Shot        *shot;
    list_reset( cur_game->shots );
    while ( ( shot = list_next( cur_game->shots ) ) )
        if ( mx == -1 || (shot->target.mx == mx && shot->target.my == my) )
            shot->get_target = 1;
}

/*
====================================================================
Update position of shots and check if bricks get destroyed.
A list of all hit bricks is returned (at maximum 
PADDLE_WEAPON_AMMO * 4)
====================================================================
*/
void shots_update( int ms )
{
	int i;
	ListEntry  *entry = cur_game->shots->head->next;
	Shot        *shot;

	while ( entry != cur_game->shots->tail ) {
		shot = entry->item;
		if ( shot->get_target ) { /* new target? */
			shot_get_target(shot);
			shot->get_target = 0;
		}
		shot->y += shot->dir * ms * shot_v_y;
		shot->target.cur_tm += ms;
		entry = entry->next;
		/* kill 'out of screen' shots */
		if ( shot->y + shot_h < 0 || shot->y > 480 ) {
			shot->paddle->weapon_ammo++; /* give back used shot */
			list_delete_entry( cur_game->shots, entry->prev );
			continue;
		}
		/* check hits */
		if (shot->target.cur_tm > shot->target.time) {
			if ( brick_hit( shot->target.mx, shot->target.my, 
					0, SHR_BY_SHOT, vector_get( 0, shot->dir ), 
					shot->paddle ) ) {
				shots_check_targets( shot->target.mx, shot->target.my );
                		balls_check_targets( shot->target.mx, shot->target.my );
			}
			if (shot->next_too)
			if ( brick_hit( shot->target.mx + 1, shot->target.my, 
					0, SHR_BY_SHOT, vector_get( 0, shot->dir ), 
					shot->paddle ) ) {
				shots_check_targets(shot->target.mx + 1, shot->target.my);
                		balls_check_targets( shot->target.mx, shot->target.my );
			}
			shot->paddle->weapon_ammo++; /* give back used shot */
			list_delete_entry( cur_game->shots, entry->prev );
			continue;
		}
		/* in multiplayer we check if we hit the opponent if so we steal
		   him a 1000 points */
		for ( i = 0; i < cur_game->paddle_count; i++ )
			if ( cur_game->paddles[i] != shot->paddle )
			if ( shot->x + shot_w > cur_game->paddles[i]->x )
			if ( shot->x < cur_game->paddles[i]->x + cur_game->paddles[i]->w )
			if ( shot->y + shot_h > cur_game->paddles[i]->y )
			if ( shot->y < cur_game->paddles[i]->y + cur_game->paddles[i]->h ) {
				if ( (cur_game->paddles[i]->score -= 1000) < 0 )
					cur_game->paddles[i]->score = 0;
				shot->paddle->score += 1000;
				shot->paddle->weapon_ammo++;
				list_delete_entry( cur_game->shots, entry->prev );
				break;
			}
	}
}


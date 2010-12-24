/***************************************************************************
                          extras.c  -  description
                             -------------------
    begin                : Sun Sep 9 2001
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
#include "levels.h"
#include "paddle.h"
#include "bricks.h"
#include "balls.h"
#include "mathfuncs.h"
#include "extras.h"

extern int ball_w, ball_dia;
extern Game *cur_game;

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Public
====================================================================
*/

/*
====================================================================
Create new extra at position
====================================================================
*/
Extra *extra_create( int type, int x, int y, int dir )
{
	Extra *e = salloc( 1, sizeof( Extra ) );
	e->type = type;
	e->offset = type * BRICK_WIDTH;
	e->x = x; e->y = y;
	e->dir = dir;
	e->alpha = 0;
	return e;
}

/*
====================================================================
Use extra when paddle collected it
====================================================================
*/
void extra_use( Paddle *paddle, int extra_type )
{
	Ball 	*b;
	int 	i, j;

	if ( cur_game->diff->allow_maluses ) {
		while( extra_type == EX_RANDOM )
			extra_type = rand() % (EX_NUMBER);
	} else {
		while ( extra_type == EX_RANDOM || extra_is_malus( extra_type ) )
			extra_type = rand() % (EX_NUMBER);
	}

	/* store modification */
	i = cur_game->paddles[0]==paddle?0:1;
	if ( cur_game->mod.collected_extra_count[i] < MAX_MODS )
		cur_game->mod.collected_extras[i][cur_game->mod.collected_extra_count[i]++] = 
			extra_type;
	/* statistics */
	paddle->extras_collected++;

	switch (extra_type) {
		case EX_SCORE200:
			paddle->score += cur_game->diff->score_mod * 200 / 10;
			break;
		case EX_SCORE500:
			paddle->score += cur_game->diff->score_mod * 500 / 10;
			break;
		case EX_SCORE1000:
			paddle->score += cur_game->diff->score_mod * 1000 / 10;
			break;
		case EX_SCORE2000:
			paddle->score += cur_game->diff->score_mod * 2000 / 10;
			break;
		case EX_SCORE5000:
			paddle->score += cur_game->diff->score_mod * 5000 / 10;
			break;
		case EX_SCORE10000:
			paddle->score += cur_game->diff->score_mod * 10000 / 10;
			break;
		case EX_GOLDSHOWER:
			paddle->extra_time[EX_GOLDSHOWER] += TIME_GOLDSHOWER;
			paddle->extra_active[EX_GOLDSHOWER] = 1;
			break;
		case EX_LIFE:
			/* adding life is handled by client */
			break;
		case EX_SHORTEN:
			paddle_init_resize( paddle, -1);
			break;
		case EX_LENGTHEN:
			paddle_init_resize( paddle, 1);
			break;
		case EX_BALL:
			b = ball_create(
				paddle->x + (paddle->w - ball_w) / 2, 
				paddle->y + ((paddle->type == PADDLE_TOP)?paddle->h:-ball_dia) );
			b->paddle = paddle;
			ball_set_random_angle( b, cur_game->ball_v );
			b->get_target = 1;
			list_add( cur_game->balls, b );
			break;
		case EX_WALL:
			paddle->extra_time[EX_WALL] += TIME_WALL;
			if ( paddle->extra_active[EX_WALL] ) break;
			paddle->extra_active[extra_type] = 1;
			if ( paddle->wall_y == 0 ) {
				for (i = 1; i < MAP_WIDTH - 1; i++) {
					cur_game->bricks[i][0].type = MAP_WALL;
					cur_game->bricks[i][0].id = 0;
				}
			}
			else
				for (i = 1; i < MAP_WIDTH - 1; i++) {
					cur_game->bricks[i][MAP_HEIGHT - 1].type = MAP_WALL;
					cur_game->bricks[i][MAP_HEIGHT - 1].id = 0;
				}
			paddle->wall_alpha = 0;
			balls_check_targets( -1, 0 );
			break;
		case EX_METAL:
			cur_game->extra_time[EX_METAL] += TIME_METAL;
			cur_game->extra_active[extra_type] = 1;
			balls_set_type( BALL_METAL );
			/* other ball extras are disabled */
			if ( cur_game->extra_active[EX_EXPL_BALL] ) {
				cur_game->extra_active[EX_EXPL_BALL] = 0;
				cur_game->extra_time[EX_EXPL_BALL] = 0;
			}
			if ( cur_game->extra_active[EX_WEAK_BALL] ) {
				cur_game->extra_active[EX_WEAK_BALL] = 0;
				cur_game->extra_time[EX_WEAK_BALL] = 0;
			}
			break;
		case EX_FROZEN:
			paddle->extra_time[EX_FROZEN] = TIME_FROZEN;
			paddle->extra_active[extra_type] = 1;
			paddle_freeze( paddle, 1 );
			break;
		case EX_WEAPON:
			paddle->extra_time[EX_WEAPON] += TIME_WEAPON;
			paddle->extra_active[extra_type] = 1;
			weapon_install( paddle, 1 );
			break;
		case EX_SLIME:
			paddle->extra_time[EX_SLIME] += TIME_SLIME;
			paddle->extra_active[extra_type] = 1;
			paddle_set_slime( paddle, 1 );
			break;
		case EX_FAST:
			if ( cur_game->extra_active[EX_SLOW] ) {
				cur_game->extra_time[EX_SLOW] = 0;
				cur_game->extra_active[EX_SLOW] = 0;
			}
			cur_game->extra_time[EX_FAST] += TIME_FAST;
			cur_game->extra_active[extra_type] = 1;
                        cur_game->ball_v = cur_game->ball_v_max;
                        balls_set_velocity( cur_game->balls, cur_game->ball_v );
			break;
		case EX_SLOW:
			if ( cur_game->extra_active[EX_FAST] ) {
				cur_game->extra_time[EX_FAST] = 0;
				cur_game->extra_active[EX_FAST] = 0;
			}
			cur_game->extra_time[EX_SLOW] += TIME_SLOW;
			cur_game->extra_active[extra_type] = 1;
			cur_game->ball_v = cur_game->ball_v_min;
                        balls_set_velocity( cur_game->balls, cur_game->ball_v );
			break;
		case EX_CHAOS:
			cur_game->extra_time[EX_CHAOS] += TIME_CHAOS;
			cur_game->extra_active[extra_type] = 1;
			balls_set_chaos( 1 );
			break;
		case EX_DARKNESS:
			cur_game->extra_time[EX_DARKNESS] += TIME_DARKNESS;
			cur_game->extra_active[extra_type] = 1;
			break;
		case EX_GHOST_PADDLE:
			paddle->extra_time[EX_GHOST_PADDLE] += TIME_GHOST_PADDLE;
			paddle->extra_active[extra_type] = 1;
			paddle_set_invis( paddle, 1 );
			break;
		case EX_TIME_ADD:
			for ( i = 0; i < EX_NUMBER; i++ )
				if ( cur_game->extra_time[i] )
					cur_game->extra_time[i] += 7000;
			for ( i = 0; i < EX_NUMBER; i++ ) {
				for ( j = 0; j < cur_game->paddle_count; j++ )
					if ( cur_game->paddles[j]->extra_time[i] )
						cur_game->paddles[j]->extra_time[i] += 7000;
			}
			break;
		case EX_EXPL_BALL:
			balls_set_type( BALL_EXPL );
			cur_game->extra_time[EX_EXPL_BALL] += TIME_EXPL_BALL;
			cur_game->extra_active[extra_type] = 1;
			/* other ball extras are disabled */
			if ( cur_game->extra_active[EX_METAL] ) {
				cur_game->extra_active[EX_METAL] = 0;
				cur_game->extra_time[EX_METAL] = 0;
			}
			if ( cur_game->extra_active[EX_WEAK_BALL] ) {
				cur_game->extra_active[EX_WEAK_BALL] = 0;
				cur_game->extra_time[EX_WEAK_BALL] = 0;
			}
			break;
		case EX_WEAK_BALL:
			balls_set_type( BALL_WEAK );
			cur_game->extra_time[EX_WEAK_BALL] += TIME_WEAK_BALL;
			cur_game->extra_active[extra_type] = 1;
			/* other ball extras are disabled */
			if ( cur_game->extra_active[EX_METAL] ) {
				cur_game->extra_active[EX_METAL] = 0;
				cur_game->extra_time[EX_METAL] = 0;
			}
			if ( cur_game->extra_active[EX_EXPL_BALL] ) {
				cur_game->extra_active[EX_EXPL_BALL] = 0;
				cur_game->extra_time[EX_EXPL_BALL] = 0;
			}
			break;
		case EX_BONUS_MAGNET:
			paddle_set_attract( paddle, ATTRACT_BONUS );
			paddle->extra_time[EX_BONUS_MAGNET] += TIME_BONUS_MAGNET;
			paddle->extra_active[extra_type] = 1;
			if ( paddle->extra_active[EX_MALUS_MAGNET] ) {
				paddle->extra_active[EX_MALUS_MAGNET] = 0;
				paddle->extra_time[EX_MALUS_MAGNET] = 0;
			}
			break;
		case EX_MALUS_MAGNET:
			paddle_set_attract( paddle, ATTRACT_MALUS );
			paddle->extra_time[EX_MALUS_MAGNET] += TIME_MALUS_MAGNET;
			paddle->extra_active[extra_type] = 1;
			if ( paddle->extra_active[EX_BONUS_MAGNET] ) {
				paddle->extra_active[EX_BONUS_MAGNET] = 0;
				paddle->extra_time[EX_BONUS_MAGNET] = 0;
			}
			break;
		case EX_DISABLE:
			/* set all active extra times to 1 so they will expire next
			   prog cycle */ 
			for ( i = 0; i < EX_NUMBER; i++ )
				if ( cur_game->extra_time[i] )
					cur_game->extra_time[i] = 1;
			for ( i = 0; i < EX_NUMBER; i++ ) {
				for ( j = 0; j < cur_game->paddle_count; j++ )
					if ( cur_game->paddles[j]->extra_time[i] )
						cur_game->paddles[j]->extra_time[i] = 1;
			}
			break;
		default:
			/* it wasn't used so delete mod */
			i = cur_game->paddles[0]==paddle?0:1;
			cur_game->mod.collected_extra_count[i]--;
			break;
			
	}
}
/*
====================================================================
Update extras
====================================================================
*/
void extras_update( int ms )
{
	Extra       *ex;
	int i, j;
	int magnets;
	Paddle *magnet;

	/* check extra_time of limited extras */

	/* general extras */
	for ( i = 0; i < EX_NUMBER; i++ )
		if ( cur_game->extra_time[i] )
		if ( (cur_game->extra_time[i] -= ms) <= 0 ) {
			cur_game->extra_time[i] = 0;
			/* expired */
			switch ( i ) {
				case EX_EXPL_BALL:
				case EX_WEAK_BALL:
				case EX_METAL: 
					balls_set_type( BALL_NORMAL ); 
					break;
				case EX_SLOW:
				case EX_FAST:
					cur_game->ball_v = cur_game->diff->v_start + 
						cur_game->diff->v_add * cur_game->speedup_level;
                                        balls_set_velocity( cur_game->balls, cur_game->ball_v );
					break;
				case EX_CHAOS:
					balls_set_chaos( 0 );
					break;
			}
			/* set deactivated */
			cur_game->extra_active[i] = 0; 
		}

	/* paddlized extras */
	for ( j = 0; j < cur_game->paddle_count; j++ )
	for ( i = 0; i < EX_NUMBER; i++ )
		/* extra_time of wall is updated in wall_update() */
		if ( cur_game->paddles[j]->extra_time[i] && i != EX_WALL )
		if ( (cur_game->paddles[j]->extra_time[i] -= ms) <= 0 ) {
			cur_game->paddles[j]->extra_time[i] = 0;
			/* expired */
			switch ( i ) {
				case EX_SLIME:
				        paddle_set_slime( cur_game->paddles[j], 0 );
					/* release all balls from paddle */
					balls_detach_from_paddle( cur_game->paddles[j],
								  ((rand()%2==1)?-1:1) );
					break;
				case EX_WEAPON: weapon_install( cur_game->paddles[j], 0 ); break;
				case EX_FROZEN:
					paddle_freeze( cur_game->paddles[j], 0 );
					break;
				case EX_GHOST_PADDLE:
					paddle_set_invis( cur_game->paddles[j], 0 );
					break;
				case EX_BONUS_MAGNET:
				case EX_MALUS_MAGNET:
					paddle_set_attract( cur_game->paddles[j], ATTRACT_NONE );
					break;
			}
			/* set deactivated */
			cur_game->paddles[j]->extra_active[i] = 0; /* wall is handled in wall_...() */
		}

	/* move extras and check if paddle was hit */
	list_reset( cur_game->extras );
	while ( ( ex = list_next( cur_game->extras ) ) ) {
		/* if only one paddle has a magnet active all extras will 
		 * be attracted by this paddle else the extras 'dir' is used 
		 */
		magnets = 0; magnet = 0;
		for ( i = 0; i < cur_game->paddle_count; i++ )
			if ( paddle_check_attract( cur_game->paddles[i], ex->type ) ) {
				magnets++;
				magnet = cur_game->paddles[i]; /* last magnet */
			}
		if ( magnets != 1 ) {
			/* either no or more than one magnet so use default */
			if ( ex->dir > 0 )
				ex->y += 0.05 * ms;
			else
				ex->y -= 0.05 * ms;
		}
		else {
			/* 'magnet' is the paddle that will attract this extra */
			if ( magnet->type == PADDLE_TOP )
				ex->y -= 0.05 * ms;
			else
				ex->y += 0.05 * ms;
			if ( ex->x + ( BRICK_WIDTH >> 1 ) < magnet->x + ( magnet->w >> 1 ) ) {
				ex->x += 0.05 * ms;
				if ( ex->x + ( BRICK_WIDTH >> 1 ) > magnet->x + ( magnet->w >> 1 ) )
					ex->x = magnet->x + ( magnet->w >> 1 ) - ( BRICK_WIDTH >> 1 );
			}
			else {
				ex->x -= 0.05 * ms;
				if ( ex->x + ( BRICK_WIDTH >> 1 ) < magnet->x + ( magnet->w >> 1 ) )
					ex->x = magnet->x + ( magnet->w >> 1 ) - ( BRICK_WIDTH >> 1 );
			}
		}
		/* if out of screen, kill this extra */
		if ( ex->y >= 480 || ex->y + BRICK_HEIGHT < 0 ) {
			list_delete_current( cur_game->extras );
			continue;
		}
		for ( j = 0; j < cur_game->paddle_count; j++ ) {
			/* contact with paddle core ? */
			if ( paddle_solid( cur_game->paddles[j] ) )
			if ( ex->x + BRICK_WIDTH > cur_game->paddles[j]->x )
			if ( ex->x < cur_game->paddles[j]->x + cur_game->paddles[j]->w - 1 )
			if ( ex->y + BRICK_HEIGHT > cur_game->paddles[j]->y )
			if ( ex->y < cur_game->paddles[j]->y + cur_game->paddles[j]->h ) {
				/* any extra except EX_JOKER is simply used */
				if ( ex->type != EX_JOKER ) {
					extra_use( cur_game->paddles[j], ex->type );
					list_delete_current( cur_game->extras );
					break;
				}
				/* use EX_JOKER and work through all active extras */
				/* the mod is only stored to play the sound */
				if ( cur_game->mod.collected_extra_count[j] < MAX_MODS )
					cur_game->mod.collected_extras[j][cur_game->mod.collected_extra_count[j]++] = EX_JOKER;
				list_reset( cur_game->extras );
				while ( ( ex = list_next( cur_game->extras ) ) ) {
					if ( ex->type != EX_JOKER )
					if ( ex->type != EX_SHORTEN )
					if ( ex->type != EX_FROZEN )
					if ( ex->type != EX_FAST )
					if ( ex->type != EX_RANDOM )
					if ( ex->type != EX_DARKNESS )
					if ( ex->type != EX_GHOST_PADDLE )
					if ( ex->type != EX_CHAOS )
					if ( ex->type != EX_DISABLE )
					if ( ex->type != EX_MALUS_MAGNET )
					if ( ex->type != EX_WEAK_BALL ) {
						extra_use( cur_game->paddles[j], ex->type );
						extra_use( cur_game->paddles[j], ex->type );
					}
					list_delete_current( cur_game->extras );
				}
				break;
			}
		}
	}
}

/* wall */
void walls_update( int ms )
{
	int i, j;

	for ( j = 0; j < cur_game->paddle_count; j++ )
		if ( cur_game->paddles[j]->extra_active[EX_WALL] ) {
			if ( cur_game->paddles[j]->extra_time[EX_WALL] > 0 ) {
				if ( (cur_game->paddles[j]->extra_time[EX_WALL] -= ms) < 0 )
					cur_game->paddles[j]->extra_time[EX_WALL] = 0;
				/* still appearing? */
				if (cur_game->paddles[j]->wall_alpha < 255)
				if ( (cur_game->paddles[j]->wall_alpha += 0.25 * ms) > 255 ) 
					cur_game->paddles[j]->wall_alpha = 255;
			}
			else
			if ( (cur_game->paddles[j]->wall_alpha -= 0.25 * ms) < 0 ) {
				cur_game->paddles[j]->wall_alpha = 0;
				cur_game->paddles[j]->extra_active[EX_WALL] = 0;
				if ( cur_game->paddles[j]->wall_y == 0 )
					for (i = 1; i < MAP_WIDTH - 1; i++) 
						cur_game->bricks[i][0].type = MAP_EMPTY;
				else
					for (i = 1; i < MAP_WIDTH - 1; i++) 
						cur_game->bricks[i][MAP_HEIGHT - 1].type = MAP_EMPTY;
				balls_check_targets( -1, 0 );
			}
		}
}

int extra_is_malus( int type )
{
	if ( type == EX_SHORTEN ) return 1;
	if ( type == EX_FROZEN ) return 1;
	if ( type == EX_FAST ) return 1;
	if ( type == EX_DARKNESS ) return 1;
	if ( type == EX_GHOST_PADDLE ) return 1;
	if ( type == EX_CHAOS ) return 1;
	if ( type == EX_DISABLE ) return 1;
	if ( type == EX_MALUS_MAGNET ) return 1;
	if ( type == EX_WEAK_BALL ) return 1;
	return 0;
}


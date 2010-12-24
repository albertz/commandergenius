/***************************************************************************
                          paddle.c  -  description
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

#include "../client/lbreakout.h"
#include "shots.h"
#include "paddle.h"

extern Game *cur_game;
int paddle_cw = 18, paddle_ch = 18; /* size of a paddle component */

/*
====================================================================
Create/delete a paddle. x is centered assuming a screen width of
640. ball_ammo is the number of additional balls paddle
 may bring to game. 
====================================================================
*/
Paddle* paddle_create( 
	int score, int type, int y, int size, int min_size, int max_size, int ball_ammo )
{
	Paddle *paddle = salloc( 1, sizeof( Paddle ) );
	delay_set( &paddle->resize_delay, 40 );
	paddle->score = score;
	paddle->len = size;
	paddle->start_len = size;
	paddle->min_len = min_size;
	paddle->max_len = max_size;
	paddle->w = (paddle->len + 2) * paddle_cw;
	paddle->wanted_w = paddle->w;
	paddle->h = paddle_ch;
	paddle->y = y;
	paddle->type = type;
	if ( paddle->type == PADDLE_TOP )
		paddle->wall_y = 0;
	else
		paddle->wall_y = 480 - BRICK_HEIGHT;
	paddle->wall_alpha = 0;
	paddle->x = ((MAP_WIDTH * BRICK_WIDTH) - paddle->w) / 2; /* centered */
	paddle->cur_x = paddle->x;
	paddle->friction = 0.3;
	paddle->friction_delay = 0;
	/* reset weapon */
	paddle->weapon_ammo = PADDLE_WEAPON_AMMO;
	/* reset slime&frozen */
	paddle->slime = 0;
	paddle->frozen = 0;
	/* attract */
	paddle->attract = ATTRACT_NONE;
	/* no ivisiblivty */
	paddle->invis = 0;
	/* ammo */
	paddle->ball_ammo = ball_ammo;
	paddle->start_ball_ammo = ball_ammo;
	/* ammo display */
	paddle->ball_ammo_disp_y = 4;
	if ( paddle->type == PADDLE_BOTTOM ) 
		paddle->ball_ammo_disp_y += (MAP_HEIGHT-1)*BRICK_HEIGHT;
	
	return paddle;
}
void paddle_delete( Paddle *paddle )
{
	if ( paddle ) free( paddle );
}
void paddle_reset( Paddle *paddle )
{
	/* bad hack, isn't it? */
	Paddle *new = paddle_create( 
		0/*clear score*/, paddle->type, paddle->y, paddle->start_len,
		paddle->min_len, paddle->max_len, paddle->start_ball_ammo );
	*paddle = *new;
	paddle_delete( new );
}

void paddle_freeze( Paddle *paddle, int freeze )
{
	paddle->frozen = freeze;
	/* Goddammit what a shitty hack!!! hope I never change input stuff
	   again. but who cares anyway? isn't this source all screwed up
	   already? so let's go on until chaos reigns!!! multiple exclamation
	   marks are a sure sign of a deseased mind, I read somewhere. This
	   is not true!!! I am NOT not mental!!! Hey, if you read this, drop
	   me a note. Just curious whether someone ever happened to do so. */
	SDL_GetRelativeMouseState( &freeze, &freeze );
}
void paddle_set_slime( Paddle *paddle, int slime )
{
	paddle->slime = slime;
}
int paddle_slimy( Paddle *paddle )
{
	return paddle->slime;
}

/*
====================================================================
Set if paddle attracts boni/mali or none.
====================================================================
*/
void paddle_set_attract( Paddle *paddle, int attract )
{
	paddle->attract = attract;
	/* no graphical change yet */
}
int paddle_attract_malus( Paddle *paddle )
{
	return ( paddle->attract == ATTRACT_MALUS );
}
int paddle_attract_bonus( Paddle *paddle )
{
	return ( paddle->attract == ATTRACT_BONUS );
}
int paddle_check_attract( Paddle *paddle, int type )
{
	switch ( type ) {
		/* bonus */
		case EX_SCORE200:
		case EX_SCORE500:
		case EX_SCORE1000:
		case EX_SCORE2000:
		case EX_SCORE5000:
		case EX_SCORE10000:
		case EX_GOLDSHOWER:
		case EX_LENGTHEN:
		case EX_LIFE:
		case EX_SLIME:
		case EX_METAL:
		case EX_BALL:
		case EX_WALL:
		case EX_WEAPON:
		case EX_SLOW:
		case EX_JOKER:
		case EX_EXPL_BALL:
		case EX_BONUS_MAGNET:
			if ( paddle_attract_bonus( paddle ) )
				return 1;
			return 0;
			/* penalty */
		case EX_SHORTEN:
		case EX_FAST:
			/*
			   case EX_SPIN_RIGHT:
			   case EX_SPIN_LEFT:
			   */
		case EX_MALUS_MAGNET:
		case EX_WEAK_BALL:
		case EX_DARKNESS:
		case EX_GHOST_PADDLE:
		case EX_FROZEN:
			if ( paddle_attract_malus( paddle ) )
				return 1;
			return 0;
			/* neutral */
		case EX_DISABLE:
		case EX_CHAOS:
		case EX_TIME_ADD:
		case EX_RANDOM:
			return 0;
	}
	return 0;
}

/*
====================================================================
(De)activate ghostly behaviour: paddle is only seen when moved at
maximum 200ms before.
====================================================================
*/
void paddle_set_invis( Paddle *paddle, int invis )
{
	if ( invis ) paddle->invis_delay = PADDLE_INVIS_DELAY;
	paddle->invis = invis;
}
int  paddle_solid( Paddle *paddle )
{
	/* does paddle is visible? */
	if ( !paddle->invis ) return 1;
	if ( paddle->invis_delay ) return 1;
	return 0;
}

/*
====================================================================
Init paddle resize (the change between wanted_w and w MUST be
even (else the paddle shrinks til eternity)
====================================================================
*/
int paddle_init_resize( Paddle *paddle, int c )
{
	/* resize possbile */
	if ( paddle->len + c > paddle->max_len || paddle->len + c < paddle->min_len ) 
		return 0;
	/* get wanted width */
	paddle->len += c;
	paddle->wanted_w = (paddle->len + 2) * paddle_cw;
	/* reset delay */
	delay_reset( &paddle->resize_delay );
	return 1;
}
/*
====================================================================
Actually resize paddle and return the -1 for shrinkage and 1 
for expansion.
====================================================================
*/
int paddle_resize( Paddle *paddle, int ms )
{
	if ( paddle->w == paddle->wanted_w ) return 0;
	if ( !delay_timed_out( &paddle->resize_delay, ms ) ) return 0;
	/* change size and position */
	if ( paddle->w < paddle->wanted_w ) {
		paddle->w += 2;
		paddle->cur_x -= 1;
		paddle->x = (int)paddle->cur_x;
		/* check range */
		if (paddle->x < BRICK_WIDTH)
			paddle->x = BRICK_WIDTH;
		if (paddle->x + paddle->w > 640 - BRICK_WIDTH)
			paddle->x = 640 - BRICK_WIDTH - paddle->w;
		return 1;
	}	
	else {
		paddle->w -= 2;
		paddle->cur_x += 1;
		paddle->x = (int)paddle->cur_x;
		return -1;
	}
}

/*
====================================================================
Update paddle resize and fire weapon.
====================================================================
*/
void paddle_update( Paddle *paddle, int ms )
{
	Ball *ball;
	int result;
    
	/* invisiblity */
    	if ( paddle->invis ) /* extra active */
	if ( paddle->invis_delay > 0 && (paddle->invis_delay-=ms) <= 0 )
		paddle->invis_delay = 0;
	
	/* handle weapon */
	if ( paddle->weapon_inst ) {
		if ( paddle->fire_left || paddle->fire_right )
		if ( paddle->weapon_ammo > 0 ) 
		if ( (paddle->weapon_fire_delay-=ms) <= 0 ) {
			paddle->weapon_fire_delay = WEAPON_FIRE_RATE;
			shot_create( paddle );
			paddle->weapon_ammo--;
		}
	}

	/* check for resize */
	if ( ( result = paddle_resize( paddle, ms ) ) ) {
		list_reset( cur_game->balls );
		while ( ( ball = list_next( cur_game->balls ) ) )
			/* adjust attached balls */
			if ( ball->attached && ball->paddle == paddle ) {
				if ( result < 0 ) {
					/* shrinked */
					if ( ball->cur.x > ( paddle->w >> 1 ) ) {
						ball->cur.x -= 2;
						ball->x = (int)ball->cur.x;
					}	
				}
				else {
					/* expanded */
					if ( ball->cur.x > ( paddle->w >> 1 ) ) {
						ball->cur.x += 2;
						ball->x = (int)ball->cur.x;
					}	
				}
			}
	}
}
/*
====================================================================
De/Activate weapon
====================================================================
*/
void weapon_install( Paddle *paddle, int install )
{
	if ( install ) paddle->weapon_fire_delay = WEAPON_FIRE_RATE;
	paddle->weapon_inst = install;
}


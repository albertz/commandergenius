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

#include "lbreakout.h"
#include "config.h"
#include "paddle.h"
#include "../game/game.h"

/*
====================================================================
Paddle & plasma weapon
====================================================================
*/
extern SDL_Surface *paddle_pic; /* paddle tiles: left, middle, right for each paddle type */
extern SDL_Surface *paddle_shadow;
extern SDL_Surface *weapon_pic;
extern SDL_Surface *ammo_pic;
extern SDL_Surface *weapon_shadow;
extern int paddle_cw, paddle_ch;
int weapon_w = 14;
int weapon_h = 18;
int weapon_fr_num = 4;
float weapon_fpms = 0.006;

/*
====================================================================
Other stuff
====================================================================
*/
extern SDL_Surface *stk_display;
extern SDL_Rect stk_drect;
extern SDL_Surface *offscreen;
extern int shadow_size;
extern Game *game;
extern Config config;

/*
====================================================================
Hide and show paddle on screen.
====================================================================
*/
void paddle_hide( Paddle *paddle )
{
	paddle->update_rect.x = paddle->x;
	paddle->update_rect.y = paddle->y;
	paddle->update_rect.w = paddle->w + shadow_size;
	paddle->update_rect.h = paddle->h + shadow_size;
	stk_surface_blit( offscreen, paddle->x,  paddle->y,
			paddle->update_rect.w, paddle->update_rect.h, 
			stk_display, paddle->x, paddle->y );
	/* store rect if it won't be drawn */
	if ( paddle->invis )
		if ( paddle->invis_delay == 0 ) 
			stk_display_store_rect( &paddle->update_rect );
}
void paddle_show_shadow( Paddle *paddle )
{
	int aux_w = 0, i;
	int rem_middle_w; /* remaining points to draw in the middle */
	if ( paddle->invis ) return;
	if ( game->extra_active[EX_DARKNESS] ) return;
	stk_surface_clip( stk_display, 0, 0, stk_display->w - BRICK_WIDTH, stk_display->h );
	/* left end */
	stk_surface_alpha_blit( paddle_shadow, paddle->pic_x_offset,0,paddle_cw, paddle_ch,
			stk_display, paddle->x + shadow_size, paddle->y + shadow_size,
			SHADOW_ALPHA );
	/* middle part */
	for ( i = paddle_cw, rem_middle_w = paddle->w - ( paddle_cw << 1 ); 
	      rem_middle_w > 0; i += paddle_cw, rem_middle_w -= paddle_cw ) {
		if ( rem_middle_w > paddle_cw )
			aux_w = paddle_cw;
		else
			aux_w = rem_middle_w;
		stk_surface_alpha_blit( paddle_shadow, 
			paddle->pic_x_offset+paddle_cw,0,aux_w, paddle_ch,
			stk_display, paddle->x + shadow_size + i, paddle->y + shadow_size,
			SHADOW_ALPHA );
	}
	/* right end */
	i = paddle->w - paddle_cw;	
	stk_surface_alpha_blit( paddle_shadow, 
		paddle->pic_x_offset+(paddle_cw<<1),0,paddle_cw, paddle_ch,
		stk_display, paddle->x + shadow_size + i, paddle->y + shadow_size,
		SHADOW_ALPHA );
	stk_surface_clip( stk_display, 0, 0, 0, 0 );
}
void paddle_show( Paddle *paddle )
{
	int i, aux_w = 0;
	int rem_middle_w; /* remaining points to draw in the middle */
	/* if invisible return or show alpha */
	if ( paddle->invis ) {
		if ( paddle->invis_delay != 0 ) 
			paddle_alphashow( paddle, 128 );
		return;
	}
	/* paddle */
	/* left end */
	stk_surface_blit( paddle_pic, paddle->pic_x_offset,paddle->pic_y_offset,
			paddle_cw, paddle_ch, stk_display, paddle->x, paddle->y );
	/* middle part */
	for ( i = paddle_cw, rem_middle_w = paddle->w - ( paddle_cw << 1 ); 
	      rem_middle_w > 0; i += paddle_cw, rem_middle_w -= paddle_cw ) {
		if ( rem_middle_w > paddle_cw )
			aux_w = paddle_cw;
		else
			aux_w = rem_middle_w;
		stk_surface_blit( paddle_pic, 
				paddle->pic_x_offset+paddle_cw,paddle->pic_y_offset,
				aux_w, paddle_ch, stk_display, paddle->x + i, paddle->y );
	}
	i = paddle->w - paddle_cw;	
	/* right end */
	stk_surface_blit( paddle_pic, 
			paddle->pic_x_offset+(paddle_cw<<1),paddle->pic_y_offset,
			paddle_cw, paddle_ch, stk_display, paddle->x + i, paddle->y );
	/* weapon */
	if ( paddle->weapon_inst )
		stk_surface_blit( weapon_pic, 
				(int)paddle->weapon_cur * weapon_w, 0, weapon_w, weapon_h,
				stk_display, paddle->x + ((paddle->w - weapon_w)>>1), paddle->y );
	/* update */
	if ( paddle->x < paddle->update_rect.x ) {
		/* movement to left */
		paddle->update_rect.w += paddle->update_rect.x - paddle->x;
		paddle->update_rect.x = paddle->x;
	}
	else {
		/* movement to right */
		paddle->update_rect.w += paddle->x - paddle->update_rect.x;
	}
	stk_display_store_rect( &paddle->update_rect );
}
void paddle_alphashow( Paddle *paddle, int a )
{
	int i, aux_w;
	int rem_middle_w;
	/* left end */
	stk_surface_alpha_blit( paddle_pic, paddle->pic_x_offset,paddle->pic_y_offset,
			paddle_cw, paddle_ch, stk_display, paddle->x, paddle->y, a );
	/* middle part */
	for ( i = paddle_cw, rem_middle_w = paddle->w - ( paddle_cw << 1 ); 
	      rem_middle_w > 0; i += paddle_cw, rem_middle_w -= paddle_cw ) {
		if ( rem_middle_w > paddle_cw )
			aux_w = paddle_cw;
		else
			aux_w = rem_middle_w;
		stk_surface_alpha_blit( paddle_pic, 
			paddle->pic_x_offset+paddle_cw,paddle->pic_y_offset,
			aux_w, paddle_ch, stk_display, paddle->x + i, paddle->y, a );
	}
	i = paddle->w - paddle_cw;	
	/* right end */
	stk_surface_alpha_blit( paddle_pic, paddle->pic_x_offset+(paddle_cw<<1),paddle->pic_y_offset,
			paddle_cw, paddle_ch, stk_display, paddle->x + i, paddle->y, a );
	if ( paddle->weapon_inst )
		stk_surface_alpha_blit( weapon_pic, 
				(int)paddle->weapon_cur * weapon_w, 0, weapon_w, weapon_h,
				stk_display, paddle->x + ((paddle->w - weapon_w)>>1), paddle->y, a );
	/* update */
	if ( paddle->x < paddle->update_rect.x ) {
		/* movement to left */
		paddle->update_rect.w += paddle->update_rect.x - paddle->x;
		paddle->update_rect.x = paddle->x;
	}
	else {
		/* movement to right */
		paddle->update_rect.w += paddle->x - paddle->update_rect.x;
	}
	stk_display_store_rect( &paddle->update_rect );
}

/*
====================================================================
Update paddle animation (weapon)
====================================================================
*/
void client_paddle_update( Paddle *paddle, int ms )
{
	Ball *ball;
	int result;
    
	/* invisiblity */
    	if ( paddle->invis ) /* extra active */
	if ( paddle->invis_delay > 0 && (paddle->invis_delay-=ms) <= 0 )
		paddle->invis_delay = 0;
	
	if ( paddle->weapon_inst ) {
		/* animation */
		if ( (paddle->weapon_cur+=ms*weapon_fpms) >= weapon_fr_num) 
			paddle->weapon_cur -= weapon_fr_num;
	}
	
	/* check for resize */
	if ( ( result = paddle_resize( paddle, ms ) ) ) {
		list_reset( game->balls );
		while ( ( ball = list_next( game->balls ) ) )
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
Handle events (used for local paddle): Update the coordinates
and the state of both fire buttons (left/right). The influence
of that on the weapon is handled in paddle_update() and the
influence on balls is handled in balls_update().
NEW: While the middle button is pressed, the balls will be
accelerated to maximum speed.

This is the local equivalent to comm_update_remote_paddle().
====================================================================
*/
void paddle_handle_events( Paddle *paddle, int ms )
{
	Uint8	*keystate;
	Uint8	buttonstate;
	int	xoff, yoff;
	int	diff, old_paddle_x = paddle->x; 
	int	has_moved = 0;
	
	/* a frozen paddle does nothing */
	if ( paddle->frozen ) {
#ifdef PADDLE_FRICTION
		paddle->v_x = 0;
#endif
		return;
	}

	/* get SDL input states */
	keystate = SDL_GetKeyState(0);
	buttonstate = SDL_GetRelativeMouseState( &xoff, &yoff );

	/* mouse motion */
	if ( xoff != 0 ) {
		if ( config.invert ) xoff = -xoff;
		paddle->cur_x += config.motion_mod * xoff;
#ifdef MOUSE_WARP
		/* reset mouse pointer as current solution does not work for
		   BeOS */
		SDL_EventState( SDL_MOUSEMOTION, SDL_IGNORE );
		SDL_WarpMouse(stk_display->w >> 1, stk_display->h >> 1);
		SDL_EventState( SDL_MOUSEMOTION, SDL_ENABLE );
#endif
#ifdef PADDLE_FRICTION
		paddle->friction_delay = 200;
#endif
		has_moved = 1;
	}
	
	/* keys */
	if ( keystate[config.k_left] ) {
		paddle->cur_x -= config.key_speed * (ms << keystate[config.k_turbo]);
		paddle->friction_delay = 0; paddle->v_x = 0.0;
		has_moved = 1;
	}
	if ( keystate[config.k_right] ) {
		paddle->cur_x += config.key_speed * (ms << keystate[config.k_turbo]);
		paddle->friction_delay = 0; paddle->v_x = 0.0;
		has_moved = 1;
	}


	/* update friction delay */
#ifdef PADDLE_FRICTION
	if ( paddle->friction_delay > 0 && (paddle->friction_delay-=ms)<=0) {
		paddle->friction_delay = 0;
		paddle->v_x = 0.0;
	}
#endif
	
	/* check paddle when moved */
	if ( has_moved ) {
		/* check range and set absolute position */
		if (paddle->cur_x < BRICK_WIDTH)
			paddle->cur_x = BRICK_WIDTH;
		if (paddle->cur_x + paddle->w >= stk_display->w - BRICK_WIDTH)
			paddle->cur_x = stk_display->w - BRICK_WIDTH - paddle->w;
		paddle->x = paddle->cur_x;

		/* set paddle speed so that it will modify balls */
#ifdef PADDLE_FRICTION
		diff = paddle->x - old_paddle_x;
		paddle->v_x = (float)(diff) / ms;
		if ( xoff != 0 ) {
			/* limit mouse speed */
			if (paddle->v_x > 5.0) paddle->v_x = 5.0;
			if (paddle->v_x < -5.0) paddle->v_x = -5.0;
			paddle->v_x /= 5;
		}
#endif
		
		/* visible for some time */
		paddle->invis_delay = PADDLE_INVIS_DELAY;
	}

	/* update the fire flags (left/right) */
	if ( buttonstate & SDL_BUTTON(1) || keystate[config.k_lfire] )
		paddle->fire_left = 1;
	else
		paddle->fire_left = 0;
	if ( buttonstate & SDL_BUTTON(3) || keystate[config.k_rfire] )
		paddle->fire_right = 1;
	else
		paddle->fire_right = 0;

        /* update maxspeed request - handled in ../game/balls.c::balls_update */
        paddle->maxballspeed_request_old = paddle->maxballspeed_request;
        if ( buttonstate & SDL_BUTTON(2) || keystate[config.k_maxballspeed] )
            paddle->maxballspeed_request = 1;
        else
            paddle->maxballspeed_request = 0;
        //printf( "%d\n", paddle->maxballspeed_request );

	/* update return key */
	if (  buttonstate & SDL_BUTTON(2) || keystate[config.k_return] )
		paddle->ball_return_key_pressed = 1;
	else
		paddle->ball_return_key_pressed = 0;
}

/* draw ammo in network game */
void paddle_ammo_hide( Paddle *paddle )
{
	int x;
	
	x = (stk_display->w - paddle->ball_ammo*ammo_pic->w) / 2;
	stk_surface_blit( offscreen, x,paddle->ball_ammo_disp_y, 
		paddle->ball_ammo*ammo_pic->w, ammo_pic->h, 
		stk_display, x,paddle->ball_ammo_disp_y );
	stk_display_store_drect();
}
void paddle_ammo_show( Paddle *paddle )
{
	int i, x;

	x = (stk_display->w - paddle->ball_ammo*ammo_pic->w) / 2;
	for ( i = 0; i < paddle->ball_ammo; i++, x += ammo_pic->w ) {
		stk_surface_blit( 
			ammo_pic, 0,0,-1,-1, 
			stk_display, x, paddle->ball_ammo_disp_y );
		stk_display_store_drect();
	}
}


/***************************************************************************
                          balls.c  -  description
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

#include "lbreakout.h"
#include "../game/game.h"
#include "bricks.h"

extern SDL_Surface *ball_pic; /* ball pictures */
extern SDL_Surface *ball_shadow;
int ball_pic_x_offset = 0; /* display ball at this position */
extern int ball_w, ball_h;
float ball_metal_alpha_change = 1.2; /* pulse factor */
float ball_metal_alpha = 0; /* alpha of ball when blue */
extern SDL_Surface *offscreen;
extern SDL_Surface *stk_display;
extern int shadow_size;
extern Game *game;

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Show/hide all balls
====================================================================
*/
void balls_hide()
{
    ListEntry *entry = game->balls->head->next;
    Ball *ball;
    int bx, by, bw, bh;
    while ( entry != game->balls->tail ) {
        ball = (Ball*)entry->item;
        entry = entry->next;
        /* balls position; add paddle pos if attached */
        bx = ball->x; by = ball->y;
        bw = ball_w + shadow_size;
        bh = ball_h + shadow_size;
        if (ball->attached) {
            bx += ball->paddle->x;
            by += ball->paddle->y;
        }
        /* blit background */
        stk_surface_blit( offscreen, 
            bx, by, bw,bh,
            stk_display, bx, by );
        stk_display_store_drect();
    }
}
void balls_show_shadow()
{
    ListEntry *entry = game->balls->head->next;
    int bx, by;
    int mx, my;
    Ball *ball;
    if ( game->extra_active[EX_DARKNESS] ) return;
    while ( entry != game->balls->tail ) {
        ball = entry->item;
        entry = entry->next;
        /* balls position; add paddle pos if attached */
        bx = ball->x;
        by = ball->y;
        if (ball->attached) {
            bx += ball->paddle->x;
            by += ball->paddle->y;
        }
        /* show ball -- no shadow if darkness -- no shadow if going back home */
        if ( !ball->moving_back ) {
            stk_surface_clip( stk_display, 0, 0, stk_display->w - BRICK_WIDTH, stk_display->h );
            stk_surface_alpha_blit( ball_shadow, 0, 0, ball_w, ball_h,
                stk_display, bx + shadow_size, by + shadow_size,
                (game->extra_active[EX_METAL])?(((int)ball_metal_alpha)>>1):SHADOW_ALPHA );
            stk_display_store_drect();
            /* redraw nearby bricks */
            stk_surface_clip( stk_display, bx + shadow_size, by + shadow_size, ball_w, ball_h );
            /* check the three outer ocrners of the shadow if there's a brick */
            mx = ( bx + shadow_size + ball_w ) / BRICK_WIDTH;
            my = ( by + shadow_size ) / BRICK_HEIGHT;
            if ( my < MAP_HEIGHT - 1 )
                if ( mx < MAP_WIDTH - 1 && game->bricks[mx][my].type != MAP_EMPTY )
                    brick_draw( stk_display, mx, my, 0 );
            mx = ( bx + shadow_size + ball_w ) / BRICK_WIDTH;
            my = ( by + shadow_size + ball_h ) / BRICK_HEIGHT;
            if ( my < MAP_HEIGHT - 1 )
                if ( mx < MAP_WIDTH - 1 && game->bricks[mx][my].type != MAP_EMPTY )
                    brick_draw( stk_display, mx, my, 0 );
            mx = ( bx + shadow_size ) / BRICK_WIDTH;
            my = ( by + shadow_size + ball_h ) / BRICK_HEIGHT;
            if ( my < MAP_HEIGHT - 1 )
                if ( mx < MAP_WIDTH - 1 && game->bricks[mx][my].type != MAP_EMPTY )
                    brick_draw( stk_display, mx, my, 0 );
            stk_surface_clip( stk_display, 0, 0, 0, 0 );
        }
    }
}
void balls_show()
{
    ListEntry *entry = game->balls->head->next;
    Ball *ball;
    int bx, by;
    while ( entry != game->balls->tail ) {
        ball = entry->item;
        entry = entry->next;
        /* balls position; add paddle pos if attached */
        bx = ball->x;
        by = ball->y;
        if (ball->attached) {
            bx += ball->paddle->x;
            by += ball->paddle->y;
        }
        if ( game->extra_active[EX_METAL] ) 
            stk_surface_alpha_blit( ball_pic, ball_pic_x_offset, 0,
                ball_w, ball_h, stk_display, bx, by,
                ball_metal_alpha );
        else
            stk_surface_blit( ball_pic, ball_pic_x_offset, 0,
                ball_w, ball_h, stk_display, bx, by );
        stk_display_store_drect();
    }
}
void balls_alphashow( int alpha )
{
    ListEntry *entry = game->balls->head->next;
    Ball *b;
    int bx, by;
    while ( entry != game->balls->tail ) {
        b = entry->item;
        /* balls position; add paddle pos if attached */
        bx = b->x;
        by = b->y;
        if (b->attached) {
            bx += b->paddle->x;
            by += b->paddle->y;
        }
        /* show ball */
        stk_surface_alpha_blit( ball_pic, ball_pic_x_offset, 0,
            ball_w, ball_h, stk_display, bx, by, alpha );
        entry = entry->next;
    }
}

/* reflect ball at brick but destroy nothing */
void client_brick_reflect( Ball *b )
{
	float old_vx;
	Vector n;
	int reflect;
	int chaos_reflect;

	/* time left? */
	if (b->target.cur_tm < b->target.time) return;

	/* will reflect? */
	reflect = 1;
	if ( game->extra_active[EX_METAL] )
	if ( game->bricks[b->target.mx][b->target.my].type != MAP_WALL )
		reflect = 0;

	/* will reflect chaotic? */
	chaos_reflect = 0;
	if ( game->extra_active[EX_CHAOS] || 
	     game->bricks[b->target.mx][b->target.my].type == MAP_BRICK_CHAOS )
		chaos_reflect = 1;

	/* we have a target and so we have a reset position and even if the ball's
	   not reflected the position must be reset */
	b->cur.x = b->target.x; b->x = (int)b->cur.x;
	b->cur.y = b->target.y; b->y = (int)b->cur.y;

	if ( reflect ) {
		game->mod.paddle_reflected_ball_count++;
		old_vx = b->vel.x;
		if ( !chaos_reflect ) {
			/* normal reflection */
			n.x = (1-2*b->target.perp_vector.x*b->target.perp_vector.x)*b->vel.x + 
			      ( -2*b->target.perp_vector.x*b->target.perp_vector.y)*b->vel.y;
			n.y = ( -2*b->target.perp_vector.x*b->target.perp_vector.y)*b->vel.x + 
			      (1-2*b->target.perp_vector.y*b->target.perp_vector.y)*b->vel.y;
			b->vel.x = n.x;
			b->vel.y = n.y;
		}
		else {
			b->vel.x = ((float)RANDOM( -10000, 10000 )) / 10000;
			b->vel.y = (float)(RANDOM( -10000, 10000 )) / 10000;
		}	
		/* only use 2 degree steps */
		b->angle = vec2angle( &b->vel );
		angle2vec( b->angle, &b->vel );
		if ( b->target.side >= CORNER_UPPER_LEFT && !chaos_reflect )
			ball_mask_vel( b, old_vx, BALL_ADD_ENTROPY );
		else
			ball_mask_vel( b, old_vx, BALL_NO_ENTROPY );
	}
	
	/* mark target as disabled so it won't get stuck at the 
	   bottom of the screen but keep the target position so 
	   that we know what needs an update. */
	b->target.exists = 0;
}

/* update energy ball animation */
void client_balls_update( int ms )
{
	Vector old; /* old position of ball before update */
	Ball *ball;
	
	/* modify alpha when metal */
	if ( game->extra_active[EX_METAL] ) {
		ball_metal_alpha += ball_metal_alpha_change * ms;
		if ( ball_metal_alpha >= 255 || ball_metal_alpha <= 0 ) {
			ball_metal_alpha_change = -ball_metal_alpha_change;
			if ( ball_metal_alpha < 0 ) ball_metal_alpha = 0;
			if ( ball_metal_alpha > 255 ) ball_metal_alpha = 255;
		}	
	}

	/* new position if NOT attached. the communicator has set the velocity
	 * 0 if a ball is inanimated */
	if ( game->game_type == GT_NETWORK ) {
		list_reset( game->balls );
		while ( (ball = list_next( game->balls )) ) {
			if ( ball->attached ) continue;
			if ( ball->vel.x == 0 && ball->vel.y == 0 ) continue;
			
			old.x = ball->cur.x;
			old.y = ball->cur.y;
			
			ball->cur.x += ball->vel.x * ms;
			ball->cur.y += ball->vel.y * ms;
			ball->x = (int)ball->cur.x;
			ball->y = (int)ball->cur.y;
		}
	}
}


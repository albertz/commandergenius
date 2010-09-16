/***************************************************************************
                          frame.c  -  description
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
#include "player.h"
#include "display.h"
#include "frame.h"

extern SDL_Surface *stk_display;
extern SDL_Surface *offscreen;
extern SDL_Surface *bkgnd; /* background + frame */
extern SDL_Surface *brick_pic; /* brick graphics in a horizontal order */
/* parts of the frame: left, top, right */
extern SDL_Surface *frame_left, *frame_top, *frame_right;
extern SDL_Surface *frame_left_shadow, *frame_top_shadow, *frame_right_shadow;
extern SDL_Surface *frame_mp_left, *frame_mp_right;
extern SDL_Surface *frame_mp_left_shadow, *frame_mp_right_shadow;
extern SDL_Surface *lamps; /* life lamps */
float score = 0; /* current score */
int dest_score; /* final score */
int score_x_offset = 54; /* display score at this position (right align)*/
int score_x, score_y = BRICK_HEIGHT / 2 - 2; /* center position where to write score */
int new_life_y; /* if new life lamp is drawn do this at this y position */
int name_x = 404, name_y = BRICK_HEIGHT / 2 - 2; /* offset (right side of screen) */
extern int shadow_size;
extern Config config;
int info_x;
int info_y; /* absolute position is info_y + info_offset */
int info_offsets[EX_NUMBER]; /* offset at which extra info is displayed */
int paddle_info_y[4];
int paddle_info_offsets[EX_NUMBER]; /* specialized extras */
extern SDL_Surface *extra_pic;
#ifdef AUDIO_ENABLED
extern StkSound *wav_life_up, *wav_life_down;
#endif
extern StkFont *display_font;
int warp_blink; 
Delay warp_delay;
int warp_x, warp_y;
int warp_blinks;
extern SDL_Surface *warp_pic;
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
Create frame by loading and assembling all graphics and loading
additional graphics.
====================================================================
*/
void frame_create()
{
    delay_set( &warp_delay, 500 );
    warp_x = stk_display->w - BRICK_WIDTH + ( BRICK_WIDTH - warp_pic->w ) / 2;
    warp_y = stk_display->h - warp_pic->h;
    warp_blink = 0;
}

/*
====================================================================
Inititate the frame to the according game type.
Set the position of the extras displayed, enable life lamps etc.
====================================================================
*/
void frame_init()
{
    /* setup position of extra info */
    info_x = stk_display->w - BRICK_WIDTH;
    if ( game->game_type == GT_NETWORK ) {
        /* general */
        info_y = 178;
        info_offsets[EX_METAL] = 26;
        info_offsets[EX_WEAK_BALL] = 26;
        info_offsets[EX_EXPL_BALL] = 26;
        info_offsets[EX_FAST] = 52;
        info_offsets[EX_SLOW] = 52;
        info_offsets[EX_CHAOS] = 78;
        /* paddles */
        paddle_info_y[0] = 271;
        paddle_info_y[1] = 7;
        paddle_info_offsets[EX_SLIME] = 26;
        paddle_info_offsets[EX_WEAPON] = 52;
        paddle_info_offsets[EX_WALL] = 78;
        paddle_info_offsets[EX_GOLDSHOWER] = 104;
        paddle_info_offsets[EX_GHOST_PADDLE] = 130;
        paddle_info_offsets[EX_BONUS_MAGNET] = 156;
        paddle_info_offsets[EX_MALUS_MAGNET] = 156;
    }
    else {
        /* in single player we ignore the second paddle
           and model general and paddle0 extras so that
           they result in the old layout */
        info_y = 48; paddle_info_y[0] = 48;
        paddle_info_offsets[EX_GOLDSHOWER] = 30;
        paddle_info_offsets[EX_SLIME] = 60;
        info_offsets[EX_METAL] = 90;
        info_offsets[EX_WEAK_BALL] = 90;
        info_offsets[EX_EXPL_BALL] = 90;
        paddle_info_offsets[EX_WALL] = 120;
        paddle_info_offsets[EX_WEAPON] = 150;
        info_offsets[EX_FAST] = 180;
        info_offsets[EX_SLOW] = 180;
        info_offsets[EX_CHAOS] = 210;
        paddle_info_offsets[EX_GHOST_PADDLE] = 240;
        paddle_info_offsets[EX_BONUS_MAGNET] = 270;
        paddle_info_offsets[EX_MALUS_MAGNET] = 270;
    }
}

/*
====================================================================
Free all resources created by frame_create()
====================================================================
*/
void frame_delete()
{
}
/*
====================================================================
Draw frame to offscreen and to bkgnd as frame won't change while
playing.
====================================================================
*/
void frame_draw()
{
    SDL_Surface *fr_left = (game->game_type==GT_LOCAL)?frame_left:frame_mp_left;
    SDL_Surface *fr_right = (game->game_type==GT_LOCAL)?frame_right:frame_mp_right;
    SDL_Surface *fr_left_shadow = 
	    (game->game_type==GT_LOCAL)?frame_left_shadow:frame_mp_left_shadow;
    SDL_Surface *fr_right_shadow = 
	    (game->game_type==GT_LOCAL)?frame_right_shadow:frame_mp_right_shadow;
    /* left and right part are always drawn */
    /* left part */
    stk_surface_alpha_blit( fr_left_shadow, 0,0,-1,-1,
        offscreen, shadow_size, shadow_size, SHADOW_ALPHA );
    stk_surface_blit( fr_left, 0,0,-1,-1, offscreen, 0,0 );
    stk_surface_alpha_blit( fr_left_shadow, 0,0,-1,-1,
        bkgnd, shadow_size, shadow_size, SHADOW_ALPHA );
    stk_surface_blit( fr_left, 0,0,-1,-1, bkgnd, 0,0 );
    if ( game->game_type == GT_LOCAL ) {
        /* add top */
        stk_surface_alpha_blit( frame_top_shadow, 0,0,-1,-1,
            offscreen, fr_left->w + shadow_size, shadow_size, SHADOW_ALPHA );
        stk_surface_blit( 
            frame_top, 0,0,-1,-1, offscreen, fr_left->w,0 );
        stk_surface_alpha_blit( frame_top_shadow, 0,0,-1,-1,
            bkgnd, fr_left->w + shadow_size, shadow_size, SHADOW_ALPHA );
        stk_surface_blit( 
            frame_top, 0,0,-1,-1, bkgnd, fr_left->w,0 );
    }
    /* right part */
    stk_surface_alpha_blit( fr_right_shadow, 0,0,-1,-1,
        offscreen, stk_display->w - fr_right_shadow->w + shadow_size, 
        shadow_size, SHADOW_ALPHA );
    stk_surface_blit( fr_right, 0,0,-1,-1, offscreen, 
        stk_display->w - fr_right->w,0 );
    stk_surface_alpha_blit( fr_right_shadow, 0,0,-1,-1,
        bkgnd, stk_display->w - fr_right_shadow->w + shadow_size, 
        shadow_size, SHADOW_ALPHA );
    stk_surface_blit( fr_right, 0,0,-1,-1, bkgnd, 
        stk_display->w - fr_right->w,0 );
}
/*
====================================================================
Add life lamps at left side of frame in offscreen
====================================================================
*/
void frame_draw_lives( int lives, int max_lives )
{
    int i,y;
    /* substract one life to have the same result like in old LBreakout */
    /* at maximum ten lamps may be displayed */
    for ( i = 0; i < 10; i++ ) {
        if ( i < lives - 1 )
            y = BRICK_HEIGHT;
        else
            if ( i < max_lives - 1 )
                y = 0;
            else
                y = BRICK_HEIGHT * 2;
        stk_surface_blit( lamps, 0,y, BRICK_WIDTH, BRICK_HEIGHT,
            offscreen, 0, ( MAP_HEIGHT - i - 1 ) * BRICK_HEIGHT );
    }
    /* get position of next lamp */
    new_life_y = stk_display->h - lives * BRICK_HEIGHT;
}
/*
====================================================================
Add one new life at offscreen and screen (as this will happen in
game )
====================================================================
*/
void frame_add_life()
{
	if ( !game->extra_active[EX_DARKNESS] ) {
        stk_surface_blit( lamps, 0, BRICK_HEIGHT, 
            BRICK_WIDTH, BRICK_HEIGHT,
            stk_display, 0, new_life_y );
        stk_display_store_drect();
	}		
    new_life_y -= BRICK_HEIGHT;
#ifdef AUDIO_ENABLED
    stk_sound_play_x( 20, wav_life_up );
#endif
}
/*
====================================================================
Switch of a life lamp
====================================================================
*/
void frame_remove_life()
{
    new_life_y += BRICK_HEIGHT;
    if ( new_life_y >= stk_display->h ) return;
	if ( !game->extra_active[EX_DARKNESS] ) {
        stk_surface_blit( lamps, 0, 0, BRICK_WIDTH, BRICK_HEIGHT,
            stk_display, 0, new_life_y );
        stk_display_store_drect();
	}		
#ifdef AUDIO_ENABLED
    stk_sound_play_x( 20, wav_life_down );
#endif
}
/*
====================================================================
Display extra information on right side of screen.
====================================================================
*/
void frame_info_hide()
{
    int i, j;
    if ( !config.bonus_info ) return;
    for ( i = 0; i < EX_NUMBER; i++ )
        if ( info_offsets[i] > 0 && game->extra_active[i] ) {
            stk_surface_blit( offscreen, info_x, info_y + info_offsets[i],
                BRICK_WIDTH, BRICK_HEIGHT,
                stk_display, info_x, info_y + info_offsets[i] );
            stk_display_store_drect();
        }
    for ( i = 0; i < EX_NUMBER; i++ )
        for ( j = 0; j < game->paddle_count; j++ )
            if ( paddle_info_offsets[i] > 0 && game->paddles[j]->extra_active[i] ) {
                stk_surface_blit( offscreen, info_x, 
                    paddle_info_y[j] + paddle_info_offsets[i],
                    BRICK_WIDTH, BRICK_HEIGHT,
                    stk_display, info_x, paddle_info_y[j] + paddle_info_offsets[i] );
                stk_display_store_drect();
            }
}
void frame_info_show()
{
    char str[12];
    int i, j;
    if ( !config.bonus_info ) return;
    display_font->align = STK_FONT_ALIGN_CENTER_X | STK_FONT_ALIGN_CENTER_Y;
    for ( i = 0; i < EX_NUMBER; i++ ) {
        if ( info_offsets[i] > 0 && game->extra_active[i] ) {
            /* picture */
            stk_surface_fill( 
                stk_display, info_x, info_y + info_offsets[i], 
                BRICK_WIDTH, BRICK_HEIGHT, 0x0 );
            stk_surface_alpha_blit( extra_pic, i * BRICK_WIDTH, 0,
                BRICK_WIDTH, BRICK_HEIGHT,
                stk_display, info_x, info_y + info_offsets[i], 128 );
            /* remaining extra_time */
            sprintf(str, "%i", (game->extra_time[i] / 1000) + 1);
            /* write text */
            stk_font_write( display_font, stk_display,
                        info_x + ( BRICK_WIDTH >> 1 ), info_y + info_offsets[i] + ( BRICK_HEIGHT >> 1 ),
                        -1, str );
        }
	}
    for ( i = 0; i < EX_NUMBER; i++ )
        for ( j = 0; j < game->paddle_count; j++ )
            if ( paddle_info_offsets[i] > 0 && game->paddles[j]->extra_active[i] ) {
                if ( i == EX_WALL && game->paddles[j]->extra_time[i] <= 0 ) continue;
                /* picture */
                stk_surface_fill( 
                    stk_display, info_x, paddle_info_y[j] + paddle_info_offsets[i], 
                    BRICK_WIDTH, BRICK_HEIGHT, 0x0 );
                stk_surface_alpha_blit( extra_pic, i * BRICK_WIDTH, 0,
                    BRICK_WIDTH, BRICK_HEIGHT,
                    stk_display, info_x, paddle_info_y[j] + paddle_info_offsets[i], 128 );
                /* remaining extra_time */
                sprintf(str, "%i", (game->paddles[j]->extra_time[i] / 1000) + 1);
                /* write text */
                stk_font_write( display_font, stk_display,
                            info_x + ( BRICK_WIDTH >> 1 ), 
                            paddle_info_y[j] + paddle_info_offsets[i] + ( BRICK_HEIGHT >> 1 ),
                            -1, str );
            }
}

/*
====================================================================
Blink the warp icon.
====================================================================
*/
void frame_warp_icon_hide()
{
        if ( game->game_type != GT_LOCAL ) return;
	if ( game->level_type == LT_PINGPONG ) return;
	if ( game->bricks_left > game->warp_limit ) return;
	stk_surface_blit( offscreen, warp_x,warp_y,-1,-1,
			stk_display, warp_x, warp_y );
	stk_display_store_drect();
}
void frame_warp_icon_show()
{
        if ( game->game_type != GT_LOCAL ) return;
	if ( game->level_type == LT_PINGPONG ) return;
	if ( game->warp_limit == 0 ) return;
	if ( game->bricks_left > game->warp_limit ) return;
	if ( !warp_blink ) return;
	if ( !game->extra_active[EX_DARKNESS] )
		stk_surface_blit( warp_pic, 0,0,-1,-1,
				stk_display, warp_x, warp_y );
	else
		stk_surface_alpha_blit( warp_pic, 0,0,-1,-1,
				stk_display, warp_x, warp_y, 128 );
}
void frame_warp_icon_update( int ms )
{
	//printf( "%i > %i\n", game->bricks_left, game->warp_limit );
        if ( game->game_type != GT_LOCAL ) return;
	if ( game->level_type == LT_PINGPONG ) return;
	if ( game->bricks_left > game->warp_limit ) return;
	if ( warp_blinks == 0 ) {
		warp_blink = 1;
		return;
	}
	if ( delay_timed_out( &warp_delay, ms ) ) {
		warp_blink = !warp_blink;
		if ( warp_blink )
			warp_blinks--;
	}
}

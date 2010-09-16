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

#include "lbreakout.h"
#include "../game/game.h"
#include "bricks.h"
#include "shrapnells.h"
#include "player.h"
#include "paddle.h"
#include "frame.h"

extern Player *cur_player;
extern SDL_Surface *stk_display;
extern SDL_Rect stk_drect;
extern SDL_Surface *offscreen;
extern SDL_Surface *bkgnd; /* background + frame */
extern SDL_Surface *offscreen_backup;
extern int ball_w, ball_dia;
extern SDL_Surface *brick_pic;
extern Brick bricks[MAP_WIDTH][MAP_HEIGHT];
extern SDL_Surface *extra_pic; /* graphics */
extern SDL_Surface *extra_shadow;
extern SDL_Surface *paddle_pic, *weapon_pic;
extern SDL_Surface *ball_pic;
extern SDL_Surface *shot_pic;
extern StkFont *display_font;
extern SDL_Surface *wall_pic;
extern int shadow_size;
#ifdef AUDIO_ENABLED
extern StkSound *wav_score, *wav_metal, *wav_std, *wav_wall, *wav_joker;
extern StkSound *wav_goldshower, *wav_life_up;
extern StkSound *wav_speedup, *wav_speeddown;
extern StkSound *wav_chaos, *wav_darkness, *wav_ghost;
extern StkSound *wav_timeadd, *wav_expl_ball, *wav_weak_ball;
extern StkSound *wav_bonus_magnet, *wav_malus_magnet, *wav_disable;
extern StkSound *wav_attach; /* handled by balls.c */
extern StkSound *wav_expand, *wav_shrink, *wav_frozen;
#endif
extern Game *game; /* client game context */
extern int paddle_ch;
extern int ball_pic_x_offset;

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

/* Play sounds and handle graphical effects. */
void client_handle_collected_extra( Paddle *paddle, int extra_type )
{
	int i, j;
#ifdef AUDIO_ENABLED
        int px = paddle->x+(paddle->w>>1);
#endif
	Extra *ex;

	switch (extra_type) {
		case EX_JOKER:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_joker );
#endif
			list_reset( game->extras );
			while ( ( ex = list_next( game->extras ) ) ) {
				stk_display_store_rect( &ex->update_rect );
				list_delete_current( game->extras );
			}
			break;
		case EX_SCORE200:
		case EX_SCORE500:
		case EX_SCORE1000:
		case EX_SCORE2000:
		case EX_SCORE5000:
		case EX_SCORE10000:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_score );
#endif
			break;
		case EX_GOLDSHOWER:
			paddle->extra_time[EX_GOLDSHOWER] += TIME_GOLDSHOWER;
			paddle->extra_active[EX_GOLDSHOWER] = 1;
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_goldshower );
#endif
			break;
		case EX_LIFE:
			if ( game->game_type == GT_LOCAL ) {
				if ( paddle->player->lives < game->diff->max_lives ) {
					paddle->player->lives++;
					frame_add_life();
				}
			}
			break;
		case EX_SHORTEN:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_shrink );
#endif
			paddle_init_resize( paddle, -1);
			break;
		case EX_LENGTHEN:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_expand );
#endif
			paddle_init_resize( paddle, 1);
			break;
		case EX_BALL:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_std );
#endif
			break;
		case EX_WALL:
			paddle->extra_time[EX_WALL] += TIME_WALL;
			if ( paddle->extra_active[EX_WALL] ) {
#ifdef AUDIO_ENABLED
				stk_sound_play_x( px, wav_std );
#endif
				break;
			}
			paddle->extra_active[extra_type] = 1;
			paddle->wall_alpha = 0;
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_wall );
#endif
			break;
		case EX_METAL:
			game->extra_time[EX_METAL] += TIME_METAL;
			game->extra_active[extra_type] = 1;
			ball_pic_x_offset = ball_w;
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_metal );
#endif
			/* other ball extras are disabled */
			if ( game->extra_active[EX_EXPL_BALL] ) {
				game->extra_active[EX_EXPL_BALL] = 0;
				game->extra_time[EX_EXPL_BALL] = 0;
			}
			if ( game->extra_active[EX_WEAK_BALL] ) {
				game->extra_active[EX_WEAK_BALL] = 0;
				game->extra_time[EX_WEAK_BALL] = 0;
			}
			break;
		case EX_FROZEN:
			paddle->extra_time[EX_FROZEN] = TIME_FROZEN;
			paddle->extra_active[extra_type] = 1;
			paddle->pic_y_offset = paddle_ch*2;
			paddle->frozen = 1; /* the server sided paddle is blocked
					       so does the same with the local copy */
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_frozen );
#endif
			break;
		case EX_WEAPON:
			paddle->extra_time[EX_WEAPON] += TIME_WEAPON;
			paddle->extra_active[extra_type] = 1;
			weapon_install( paddle, 1 );
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_std );
#endif
			break;
		case EX_SLIME:
			paddle->extra_time[EX_SLIME] += TIME_SLIME;
			paddle->extra_active[extra_type] = 1;
			if ( !paddle->frozen )
				paddle->pic_y_offset = paddle_ch;
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_attach );
#endif
			break;
		case EX_FAST:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_speedup );
#endif
			if ( game->extra_active[EX_SLOW] ) {
				game->extra_time[EX_SLOW] = 0;
				game->extra_active[EX_SLOW] = 0;
			}
			game->extra_time[EX_FAST] += TIME_FAST;
			game->extra_active[extra_type] = 1;
			break;
		case EX_SLOW:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_speeddown );
#endif
			if ( game->extra_active[EX_FAST] ) {
				game->extra_time[EX_FAST] = 0;
				game->extra_active[EX_FAST] = 0;
			}
			game->extra_time[EX_SLOW] += TIME_SLOW;
			game->extra_active[extra_type] = 1;
			break;
		case EX_CHAOS:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_chaos );
#endif
			game->extra_time[EX_CHAOS] += TIME_CHAOS;
			game->extra_active[extra_type] = 1;
			break;
		case EX_DARKNESS:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_darkness );
#endif
			game->extra_time[EX_DARKNESS] += TIME_DARKNESS;
			if ( game->extra_active[EX_DARKNESS] ) break;
			/* backup offscreen and turn it black */
			stk_surface_fill( offscreen, 0,0,-1,-1, 0x0 );
			stk_surface_fill( stk_display, 0,0,-1,-1, 0x0 );
			stk_display_store_rect( 0 );
			/* set alpha keys to 128 */
			SDL_SetAlpha( paddle_pic, SDL_SRCALPHA, 128 );
			SDL_SetAlpha( weapon_pic, SDL_SRCALPHA, 128 );
			SDL_SetAlpha( extra_pic, SDL_SRCALPHA, 128 );
			SDL_SetAlpha( ball_pic, SDL_SRCALPHA, 128 );
			SDL_SetAlpha( shot_pic, SDL_SRCALPHA, 128 );
			SDL_SetAlpha( display_font->surface, SDL_SRCALPHA, 128 );
			/* use dark explosions */
			exps_set_dark( 1 );
			game->extra_active[extra_type] = 1;
			break;
		case EX_GHOST_PADDLE:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_ghost );
#endif
			paddle->extra_time[EX_GHOST_PADDLE] += TIME_GHOST_PADDLE;
			paddle->extra_active[extra_type] = 1;
			paddle_set_invis( paddle, 1 );
			break;
		case EX_TIME_ADD:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_timeadd );
#endif
			for ( i = 0; i < EX_NUMBER; i++ )
				if ( game->extra_time[i] )
					game->extra_time[i] += 7000;
			for ( i = 0; i < EX_NUMBER; i++ ) {
				for ( j = 0; j < game->paddle_count; j++ )
					if ( game->paddles[j]->extra_time[i] )
						game->paddles[j]->extra_time[i] += 7000;
			}
			break;
		case EX_EXPL_BALL:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_expl_ball );
#endif
			ball_pic_x_offset = ball_w*2;
			game->extra_time[EX_EXPL_BALL] += TIME_EXPL_BALL;
			game->extra_active[extra_type] = 1;
			/* other ball extras are disabled */
			if ( game->extra_active[EX_METAL] ) {
				game->extra_active[EX_METAL] = 0;
				game->extra_time[EX_METAL] = 0;
			}
			if ( game->extra_active[EX_WEAK_BALL] ) {
				game->extra_active[EX_WEAK_BALL] = 0;
				game->extra_time[EX_WEAK_BALL] = 0;
			}
			break;
		case EX_WEAK_BALL:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_weak_ball );
#endif
			ball_pic_x_offset = ball_w*3;
			game->extra_time[EX_WEAK_BALL] += TIME_WEAK_BALL;
			game->extra_active[extra_type] = 1;
			/* other ball extras are disabled */
			if ( game->extra_active[EX_METAL] ) {
				game->extra_active[EX_METAL] = 0;
				game->extra_time[EX_METAL] = 0;
			}
			if ( game->extra_active[EX_EXPL_BALL] ) {
				game->extra_active[EX_EXPL_BALL] = 0;
				game->extra_time[EX_EXPL_BALL] = 0;
			}
			break;
		case EX_BONUS_MAGNET:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_bonus_magnet );
#endif
			paddle_set_attract( paddle, ATTRACT_BONUS );
			paddle->extra_time[EX_BONUS_MAGNET] += TIME_BONUS_MAGNET;
			paddle->extra_active[extra_type] = 1;
			if ( paddle->extra_active[EX_MALUS_MAGNET] ) {
				paddle->extra_active[EX_MALUS_MAGNET] = 0;
				paddle->extra_time[EX_MALUS_MAGNET] = 0;
			}
			break;
		case EX_MALUS_MAGNET:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_malus_magnet );
#endif
			paddle_set_attract( paddle, ATTRACT_MALUS );
			paddle->extra_time[EX_MALUS_MAGNET] += TIME_MALUS_MAGNET;
			paddle->extra_active[extra_type] = 1;
			if ( paddle->extra_active[EX_BONUS_MAGNET] ) {
				paddle->extra_active[EX_BONUS_MAGNET] = 0;
				paddle->extra_time[EX_BONUS_MAGNET] = 0;
			}
			break;
		case EX_DISABLE:
#ifdef AUDIO_ENABLED
			stk_sound_play_x( px, wav_disable );
#endif
			/* set all active extra times to 1 so they will expire next
			   prog cycle */ 
			for ( i = 0; i < EX_NUMBER; i++ )
				if ( game->extra_time[i] )
					game->extra_time[i] = 1;
			for ( i = 0; i < EX_NUMBER; i++ ) {
				for ( j = 0; j < game->paddle_count; j++ )
					if ( game->paddles[j]->extra_time[i] )
						game->paddles[j]->extra_time[i] = 1;
			}
			break;
	}
}
/*
====================================================================
Show, hide extras
====================================================================
*/
void extras_hide()
{
    ListEntry  *entry = game->extras->head->next;
    Extra       *ex;
    while ( entry != game->extras->tail ) {
        ex = entry->item;
        entry = entry->next;
        stk_surface_blit( offscreen, 
            (int)ex->x, (int)ex->y,
            BRICK_WIDTH + shadow_size, BRICK_HEIGHT + shadow_size, 
            stk_display, (int)ex->x, (int)ex->y );
        ex->update_rect = stk_drect;
	stk_display_store_rect( &ex->update_rect );
    }
}
void extras_show_shadow()
{
    ListEntry  *entry = game->extras->head->next;
    Extra       *extra;
    while ( entry != game->extras->tail ) {
        extra = entry->item;
        stk_surface_clip( stk_display, 0,0,
            stk_display->w - BRICK_WIDTH, stk_display->h );
        stk_surface_alpha_blit( 
            extra_shadow, extra->type * BRICK_WIDTH, 0,
            BRICK_WIDTH, BRICK_HEIGHT,
            stk_display, (int)extra->x + shadow_size, (int)extra->y + shadow_size,
            ((int)extra->alpha)>>1 );
        stk_surface_clip( stk_display, 0,0,-1,-1 );
        entry = entry->next;
    }
}
void extras_show()
{
    ListEntry  *entry = game->extras->head->next;
    Extra       *ex;
    int         x, y;
    while ( entry != game->extras->tail ) {
        ex = entry->item;
        entry = entry->next;
        x = (int)ex->x; y = (int)ex->y;
        stk_surface_alpha_blit( extra_pic, ex->offset, 0,
            BRICK_WIDTH, BRICK_HEIGHT, stk_display, x, y, ex->alpha );
#if 0
        if ( x < ex->update_rect.x ) {
            /* movement to left */
            ex->update_rect.w += ex->update_rect.x - x;
            ex->update_rect.x = x;
        }
        else
            /* movement to right */
            ex->update_rect.w += x - ex->update_rect.x;
        if ( ex->dir == -1 ) {
            /* movement up */
            ex->update_rect.h += ex->update_rect.y - y;
            ex->update_rect.y = y;
            if ( ex->update_rect.y < 0 ) {
                ex->update_rect.h += ex->update_rect.y;
                ex->update_rect.y = 0;
            }
        }
        else {
            /* movement down */
            ex->update_rect.h += y - ex->update_rect.y;
            if ( ex->update_rect.y + ex->update_rect.h >= stk_display->h )
                ex->update_rect.h = stk_display->h - ex->update_rect.y;
        }
        stk_display_store_rect( &ex->update_rect );
#endif
	ex->update_rect.x = x;
	ex->update_rect.y = y;
	ex->update_rect.w = BRICK_WIDTH + shadow_size;
	ex->update_rect.h = BRICK_HEIGHT + shadow_size;
        stk_display_store_rect( &ex->update_rect );
    }
}
void extras_alphashow( int alpha )
{
    ListEntry  *entry = game->extras->head->next;
    Extra       *ex;
    int         x, y;
    while ( entry != game->extras->tail ) {
        ex = entry->item;
        x = (int)ex->x;
        y = (int)ex->y;
        stk_surface_alpha_blit( extra_pic, ex->type * BRICK_WIDTH, 0,
            BRICK_WIDTH, BRICK_HEIGHT, stk_display, x, y, alpha );
        stk_display_store_rect( &ex->update_rect );
        entry = entry->next;
    }
}

/* move the extras as in extras_update but do not collect them */
void client_extras_update( int ms )
{
	Extra       *ex;
	int i, j;
	int magnets;
	Paddle *magnet;

	/* check extra_time of limited extras */
	/* general extras */
	for ( i = 0; i < EX_NUMBER; i++ )
		if ( game->extra_time[i] ) {
			if ( (game->extra_time[i] -= ms) <= 0 ) {
				game->extra_time[i] = 0;
				/* expired */
				switch ( i ) {
					case EX_EXPL_BALL:
					case EX_WEAK_BALL:
					case EX_METAL:
						ball_pic_x_offset = 0;
						break;
					case EX_DARKNESS:
						/* restore offscreen */
						stk_surface_blit( bkgnd, 0,0,-1,-1, offscreen, 0,0 );
						bricks_draw();
						if ( game->game_type == GT_LOCAL )
							frame_draw_lives( cur_player->lives, 
									  game->diff->max_lives );
						/* back to screen */
						stk_surface_blit( 
							offscreen, 0,0,-1,-1, 
							stk_display, 0,0 );
						stk_display_store_rect( 0 );
						/* set alpha keys to OPAQUE */
						SDL_SetAlpha( paddle_pic, 0,0 );
						SDL_SetAlpha( extra_pic, 0,0 );
						SDL_SetAlpha( ball_pic, 0,0 );
						SDL_SetAlpha( shot_pic, 0,0 );
						SDL_SetAlpha( display_font->surface, 0,0 );
						/* use bright explosions */
						exps_set_dark( 0 );
						break;
				}
				/* set deactivated */
				game->extra_active[i] = 0; 
			}
		}
	/* paddlized extras */
	for ( j = 0; j < game->paddle_count; j++ )
	for ( i = 0; i < EX_NUMBER; i++ )
		/* extra_time of wall is updated in client_walls_update() */
		if ( game->paddles[j]->extra_time[i] && i != EX_WALL )
		if ( (game->paddles[j]->extra_time[i] -= ms) <= 0 ) {
			game->paddles[j]->extra_time[i] = 0;
			/* expired */
			switch ( i ) {
				case EX_FROZEN:
				case EX_SLIME:
					if ( game->paddles[j]->extra_time[EX_SLIME] > 0 )
						game->paddles[j]->pic_y_offset = paddle_ch;
					else
						game->paddles[j]->pic_y_offset = 0;
					game->paddles[j]->frozen = 0;
					break;
				case EX_WEAPON: 
					weapon_install( game->paddles[j], 0 ); 
					break;
				case EX_GHOST_PADDLE:
					paddle_set_invis( game->paddles[j], 0 );
					break;
				case EX_BONUS_MAGNET:
				case EX_MALUS_MAGNET:
					paddle_set_attract( game->paddles[j], ATTRACT_NONE );
					break;
			}
			/* set deactivated */
			game->paddles[j]->extra_active[i] = 0; /* wall is handled in wall_...() */
		}
		
	/* move extras and check if paddle was hit */
	list_reset( game->extras );
	while ( ( ex = list_next( game->extras ) ) ) {
		/* if only one paddle has a magnet active all extras will 
		 * be attracted by this paddle else the extras 'dir' is used 
		 */
		magnets = 0; magnet = 0;
		for ( i = 0; i < game->paddle_count; i++ )
			if ( paddle_check_attract( game->paddles[i], ex->type ) ) {
				magnets++;
				magnet = game->paddles[i]; /* last magnet */
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
		if ( !game->extra_active[EX_DARKNESS] ) {
			if ( ex->alpha < 255 ) {
				ex->alpha += 0.25 * ms;
				if (ex->alpha > 255)
					ex->alpha = 255;
			}
		}
		else {
			if ( ex->alpha < 128 ) {
				ex->alpha += 0.25 * ms;
				if (ex->alpha > 128)
					ex->alpha = 128;
			}
		}
		/* if out of screen forget this extra */
		if ( ex->y >= stk_display->h || ex->y + BRICK_HEIGHT < 0 ) {
			stk_display_store_rect( &ex->update_rect );
			list_delete_current( game->extras );
			continue;
		}
		for ( j = 0; j < game->paddle_count; j++ ) {
			/* contact with paddle core ? */
			if ( paddle_solid( game->paddles[j] ) )
			if ( ex->x + BRICK_WIDTH > game->paddles[j]->x )
			if ( ex->x < game->paddles[j]->x + game->paddles[j]->w - 1 )
			if ( ex->y + BRICK_HEIGHT > game->paddles[j]->y )
			if ( ex->y < game->paddles[j]->y + game->paddles[j]->h ) {
				/* remove extra but don't handle it */
				stk_display_store_rect( &ex->update_rect );
				list_delete_current( game->extras );
			}
		}
	}
}

/* wall */
void walls_hide()
{
    int j;
    for ( j = 0; j < game->paddle_count; j++ )
        if ( game->paddles[j]->extra_active[EX_WALL] ) {
            stk_surface_blit( offscreen, BRICK_WIDTH, game->paddles[j]->wall_y,
                wall_pic->w, wall_pic->h,
                stk_display, BRICK_WIDTH, game->paddles[j]->wall_y );
            stk_display_store_drect();
        }
}
void walls_show()
{
    int j;
    for ( j = 0; j < game->paddle_count; j++ )
        if ( game->paddles[j]->extra_active[EX_WALL] )
            stk_surface_alpha_blit( wall_pic, 0,0,
                wall_pic->w, wall_pic->h,
                stk_display, BRICK_WIDTH, game->paddles[j]->wall_y,
                (int)game->paddles[j]->wall_alpha );
}
void walls_alphashow( int alpha )
{
    int j;
    for ( j = 0; j < game->paddle_count; j++ )
        if ( game->paddles[j]->extra_active[EX_WALL] )
            stk_surface_alpha_blit( wall_pic, 0,0,
                wall_pic->w, wall_pic->h,
                stk_display, BRICK_WIDTH, game->paddles[j]->wall_y,
                alpha );
}
void client_walls_update( int ms )
{
	int j;

	for ( j = 0; j < game->paddle_count; j++ )
		if ( game->paddles[j]->extra_active[EX_WALL] ) {
			if ( game->paddles[j]->extra_time[EX_WALL] > 0 ) {
				if ( (game->paddles[j]->extra_time[EX_WALL] -= ms) < 0 )
					game->paddles[j]->extra_time[EX_WALL] = 0;
				/* still appearing? */
				if (game->paddles[j]->wall_alpha < 255)
				if ( (game->paddles[j]->wall_alpha += 0.25 * ms) > 255 ) 
					game->paddles[j]->wall_alpha = 255;
			}
			else
			if ( (game->paddles[j]->wall_alpha -= 0.25 * ms) < 0 ) {
				game->paddles[j]->wall_alpha = 0;
				game->paddles[j]->extra_active[EX_WALL] = 0;
			}
		}
}


/***************************************************************************
                          bricks.c  -  description
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

#include <math.h>
#include "../game/game.h"
#include "lbreakout.h"
#include "shrapnells.h"
#include "bricks.h"

extern SDL_Surface *stk_display;
extern SDL_Surface *offscreen; /* offscreen with bricks, background, frame */
extern SDL_Surface *bkgnd; /* background picture (includes frame) */
extern SDL_Surface *warp_pic;
extern SDL_Surface *brick_pic; /* brick graphics in a horizontal order */
extern int shadow_size;
extern SDL_Surface *brick_shadow; /* shadow mask */
extern int shine_x, shine_y, shine_recreate;
#ifdef AUDIO_ENABLED
extern StkSound *wav_boom;
#endif
extern Game *game;

/*
====================================================================
Locals
====================================================================
*/

/* remove a brick and release the extra (which will have no effect
 * on collection) update the graphics. no growth or explosions
 * of other bricks are initiated as these are send by network */
void client_brick_remove( int mx, int my, int type, Vector imp, Paddle *paddle, int no_sound )
{
  int px, py, w, h, i, j, shadow, anim_brick_id;
  int dir;

  /* add explosion animation */
  if ( type == SHR_BY_EXPL_WITH_EXPL ) {
    exp_create( mx * BRICK_WIDTH + ( BRICK_WIDTH >> 1 ), 
		my * BRICK_HEIGHT + ( BRICK_HEIGHT >> 1 ) );
#ifdef AUDIO_ENABLED
  if (!no_sound)
    stk_sound_play_x( mx * BRICK_WIDTH + ( BRICK_WIDTH >> 1 ), wav_boom );
#endif
    type = SHR_BY_EXPL; /* WITH_EXPL is not known */
  }
    
	/* decrease brick count if no indestructible brick was destroyed */
	if ( game->bricks[mx][my].dur != -1 ) {
		game->bricks_left--;
		
		/* adjust warp limit if this was a grown brick; then limit
		 * has to decrease again */
		if (IS_GROWN_BRICK_CHAR(game->bricks[mx][my].brick_c))
			game->warp_limit--;
	}

  /* before removing the brick, store the brick id for animation */
  anim_brick_id = game->bricks[mx][my].id;
    
  /* remove brick from map */
  game->bricks[mx][my].brick_c = ' ';
  game->bricks[mx][my].id = -1;
  game->bricks[mx][my].dur = -1;
  game->bricks[mx][my].exp_time = -1;
  game->bricks[mx][my].heal_time = -1;
  game->bricks[mx][my].type = MAP_EMPTY;
    
  /* get screen position */
  px = mx * BRICK_WIDTH;
  py = my * BRICK_HEIGHT;
    
  /* release extra if one exists. in opposite to the server the goldshower
   * 1000P is not released here but in the hit handle function if the gold_shower
   * flag is set to avoid releasing 'ghost' extras due to latency. */
  dir = ( paddle->type == PADDLE_TOP ) ? -1 : 1;
  if ( game->bricks[mx][my].extra != EX_NONE ) {
    if ( game->diff->allow_maluses || !extra_is_malus( game->bricks[mx][my].extra ) )
      list_add( game->extras, extra_create( game->bricks[mx][my].extra, px, py, dir ) );
  }
  game->bricks[mx][my].extra = EX_NONE;
  game->bricks[mx][my].extra_c = ' ';

  /* in case of darkness no (graphical) remove nescessary */
  if ( game->extra_active[EX_DARKNESS] ) {
#ifdef AUDIO_ENABLED
  if (!no_sound)
    stk_sound_play_x( px, wav_boom );
#endif
    return;
  }		

  shrapnells_create( brick_pic, anim_brick_id * BRICK_WIDTH, 0, 
		     BRICK_WIDTH, BRICK_HEIGHT,
		     px, py, type, imp );
  /* recreate shine if needed */
  if (px == shine_x && py == shine_y) shine_recreate = 1;
  /* clear offscreen */
  w = BRICK_WIDTH + shadow_size; h = BRICK_HEIGHT + shadow_size;
  if ( px + w > stk_display->w - BRICK_WIDTH ) 
    w = stk_display->w - BRICK_WIDTH - px;
  stk_surface_clip( offscreen, px, py, w, h );
  stk_surface_blit( bkgnd, px, py, w, h, offscreen, px, py );
  /* if shadow redraw close bricks */
  for ( i = mx - 1; i <= mx + 1; i++ )
    for ( j = my - 1; j <= my + 1; j++ ) {
      if ( i > 0 && j > 0 && i < MAP_WIDTH - 1 ) {
	if ( game->bricks[i][j].type != MAP_EMPTY ) {
	  if ( i <= mx && j <= my ) shadow = 1; else shadow = 0;
	  brick_draw( offscreen, i, j, shadow );
	}
      }
    }
  stk_surface_clip( offscreen, 0, 0, 0, 0 );
  /* update screen */
  stk_surface_blit( offscreen, px, py, w, h, stk_display, px, py );
  stk_display_store_drect();
#ifdef AUDIO_ENABLED
  if (!no_sound)
    stk_sound_play_x( px, wav_boom );
#endif
}

static void client_brick_heal( int x, int y )
{
	game->bricks[x][y].dur++;
	game->bricks[x][y].id++;
	if ( !game->extra_active[EX_DARKNESS] ) {
		brick_draw( offscreen, x, y, 0 );
		brick_draw( stk_display, x, y, 0 );
		stk_display_store_drect();
	}
}

static void client_brick_grow( int x, int y, int id )
{
	Brick *brick = &game->bricks[x][y];
	int px, py;
	int isReplace = 0;
	
	if (game->bricks[x][y].type!=MAP_EMPTY)
		isReplace = 1;
	
	/* add brick */
	//brick->id = RANDOM( BRICK_GROW_FIRST, BRICK_GROW_LAST );
	brick->id = id;
	brick->brick_c = brick_get_char( brick->id );
	brick->type = MAP_BRICK;
	brick->score = game->diff->score_mod * BRICK_SCORE / 10;
	brick->dur = 1;
	/* keep the extra that is already assigned to this position */
	brick->exp_time = -1;
	brick->heal_time = -1;
	
	/* XXX mark grown bricks by upper case. with this trick we can store 
	 * this information in the level snapshot. */
	brick->brick_c -= 32; /* f->F, ... */

	if (!isReplace) {
		game->bricks_left++;

		/* adjust warp limit (grown bricks don't help hitting limit) */
		game->warp_limit++;
	}
	
	if ( !game->extra_active[EX_DARKNESS] ) {
		px = x * BRICK_WIDTH;
		py = y * BRICK_HEIGHT;
		if (isReplace)
			brick_draw( offscreen, x, y, 0 );
		else
			brick_draw_complex( x, y, px, py );
		stk_surface_blit( offscreen, px, py, 
				BRICK_WIDTH + shadow_size, 
				BRICK_HEIGHT + shadow_size,
				stk_display, px, py );
		stk_display_store_drect();
	}
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Draw all bricks to offscreen surface.
====================================================================
*/
void bricks_draw()
{
	int i, j;
	if ( offscreen == 0 ) return;
	stk_surface_clip( offscreen, 0, 0, stk_display->w - BRICK_WIDTH, stk_display->h );
	for ( j = 1; j < MAP_HEIGHT - 1; j++ )
		for ( i = 1; i < MAP_WIDTH - 1; i++ )
			if ( game->bricks[i][j].id >= 0 )
				brick_draw( offscreen, i, j, 1 );
	stk_surface_clip( offscreen, 0,0,0,0 );
}
/*
====================================================================
Draw brick to passed surface
====================================================================
*/
void brick_draw( SDL_Surface *surf, int map_x, int map_y, int shadow )
{
	int x = map_x * BRICK_WIDTH, y = map_y * BRICK_HEIGHT;
	/* dont draw invisible bricks */
	if ( game->bricks[map_x][map_y].id == INVIS_BRICK_ID ) return;
	/* add shadow */
	if ( shadow ) {
		stk_surface_alpha_blit( brick_shadow, 
				game->bricks[map_x][map_y].id * BRICK_WIDTH, 0, 
				BRICK_WIDTH, BRICK_HEIGHT,
				surf, x + shadow_size, y + shadow_size, SHADOW_ALPHA );
	}
	/* brick if not frame brick */
	if ( map_x == 0 || map_y == 0 || map_x == MAP_WIDTH - 1 ) return;
	stk_surface_blit( brick_pic, 
			game->bricks[map_x][map_y].id * BRICK_WIDTH, 0, 
			BRICK_WIDTH, BRICK_HEIGHT, surf, x,y );
}
/*
====================================================================
Add brick with clipped shadow to offscreen. To draw a brick without 
shadow check use brick_draw().
====================================================================
*/
void brick_draw_complex( int mx, int my, int px, int py )
{
	brick_draw( offscreen, mx, my, 1 );
	/* redraw surrounding bricks */
	stk_surface_clip( offscreen, px + shadow_size, py + shadow_size, BRICK_WIDTH, BRICK_HEIGHT );
	if ( mx + 1 == MAP_WIDTH - 1 ) {
		/* right frame part */
		stk_surface_blit( bkgnd, px + BRICK_WIDTH, py,
				BRICK_WIDTH, ( BRICK_HEIGHT << 1 ),
				offscreen, px + BRICK_WIDTH, py );
	}
	else {
		brick_draw( offscreen, mx + 1, my, 0 );
		brick_draw( offscreen, mx + 1, my + 1, 0 );
	}
	if ( game->bricks[mx][my + 1].type != MAP_EMPTY )
		brick_draw( offscreen, mx, my + 1, 0 );
	stk_surface_clip( offscreen, 0, 0, 0, 0 );
}
/*
====================================================================
Make brick at mx,my loose 'points' duration. It must have been
previously checked that this operation is completely valid.
It does not update net_bricks or the player's duration reference.
====================================================================
*/
void client_brick_loose_dur( int mx, int my, int points )
{
	int px, py;
	int refresh_h, refresh_w;
	while ( points-- > 0 ) {
		game->bricks[mx][my].dur--;
		game->bricks[mx][my].id--;
		/* adjust brick character:
		 * a,b,c - multiple hits
		 * v - invisible */
		if ( game->bricks[mx][my].brick_c == 'v' )
			game->bricks[mx][my].brick_c = 'c';
		else
			game->bricks[mx][my].brick_c--; /* successive order */
		if ( !game->extra_active[EX_DARKNESS]) {
			px = mx * BRICK_WIDTH;
			py = my * BRICK_HEIGHT;
			refresh_w = BRICK_WIDTH+shadow_size;
			refresh_h = BRICK_HEIGHT+shadow_size;
			/* copy background as old brick may have different transparency 
			   do this in three parts to save computation */
			stk_surface_blit( bkgnd, px, py, 
					shadow_size, BRICK_HEIGHT,
					offscreen, px, py );
			stk_surface_blit( bkgnd, px + shadow_size, py, 
					BRICK_WIDTH - shadow_size, BRICK_HEIGHT + shadow_size,
					offscreen, px + shadow_size, py );
			stk_surface_blit( bkgnd, px + BRICK_WIDTH, py + shadow_size,
					shadow_size, BRICK_HEIGHT,
					offscreen, px + BRICK_WIDTH, py + shadow_size );
			/* draw brick + surrounding */
			brick_draw_complex( mx, my, px, py );
			/* udpate screen */
			stk_surface_blit( offscreen, px, py, 
					refresh_w, refresh_h, stk_display, px, py );
			stk_display_store_drect();
		}
	}
}

/* handle a received brick action */
void client_handle_brick_hit( BrickHit *hit )
{
  Paddle *paddle;
  Vector imp;
	
  paddle = (hit->paddle==PADDLE_BOTTOM)?game->paddles[PADDLE_BOTTOM]:game->paddles[PADDLE_TOP];
  angle2vec( hit->degrees, &imp );
	
  switch ( hit->type )
    {
    case HT_HIT:
      client_brick_loose_dur( hit->x, hit->y, 1 );
      break;
    case HT_REMOVE:
      if (hit->draw_explosion)
	hit->dest_type = SHR_BY_EXPL_WITH_EXPL;
      client_brick_remove( hit->x, hit->y, hit->dest_type, imp, paddle, hit->no_sound );
      /* gold shower extra */
      if ( hit->gold_shower )
	list_add( game->extras,
		  extra_create( EX_SCORE1000,
				hit->x*BRICK_WIDTH, hit->y*BRICK_HEIGHT, 
				(hit->paddle==PADDLE_TOP)?-1:1 ) );
      break;
    case HT_HEAL:
      client_brick_heal( hit->x, hit->y );
      break;
    case HT_GROW:
      client_brick_grow( hit->x, hit->y, hit->brick_id );
      break;
    }
}

/* redraw all bricks (leaving shadows in transparent frame parts) */
void client_redraw_all_bricks()
{
    stk_surface_blit( bkgnd, BRICK_WIDTH, BRICK_HEIGHT, 
                      stk_display->w - BRICK_WIDTH*2,stk_display->h - BRICK_HEIGHT,
                      offscreen, BRICK_WIDTH, BRICK_HEIGHT );
    bricks_draw();
    stk_surface_blit( offscreen, BRICK_WIDTH, BRICK_HEIGHT, 
                      stk_display->w - BRICK_WIDTH*2,stk_display->h - BRICK_HEIGHT,
                      stk_display, BRICK_WIDTH, BRICK_HEIGHT );
//    stk_display_update( STK_UPDATE_ALL );
    stk_display_store_drect();
}

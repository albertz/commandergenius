/***************************************************************************
                          game.c  -  description
                             -------------------
    begin                : 03/03/19
    copyright            : (C) 2003 by Michael Speck
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

/***** INCLUDES ************************************************************/

#include "../client/lbreakout.h"
#include "game.h"
#include "bricks.h"
#include "paddle.h"
#include "balls.h"
#include "shots.h"
#include "extras.h"

/***** EXTERNAL VARIABLES **************************************************/

extern int ball_w, ball_dia;

/***** EXPORTS *************************************************************/

Game *cur_game = 0;

/***** FORWARDED DECLARATIONS **********************************************/

/***** LOCAL TYPE DEFINITIONS **********************************************/

/***** LOCAL VARIABLES *****************************************************/

static GameDiff diffs[DIFF_COUNT] = {
    { 9, 12,  8, 12, 20,  5, 0.10, 0.0016, 0.20, 32000, 0 },
    { 6,  9,  0,  2,  8,  8, 0.24, 0.0016, 0.40, 1800,  1 },
    { 5,  7,  0,  1,  6, 10, 0.27, 0.0016, 0.43, 1800,  1 },
    { 4,  5,  0,  1,  4, 13, 0.30, 0.0015, 0.45, 1800,  1 }
};

/* in network game the ball is slower and the paddle is bigger */
static GameDiff net_diffs[DIFF_COUNT] = {
    { 6, 9, 1, 2, 8, 8,  0.18, 0.0012, 0.30, 1800, 1 },
    { 5, 7, 1, 2, 6, 10, 0.21, 0.0012, 0.33, 1800, 1 },
    { 4, 5, 1, 2, 4, 13, 0.24, 0.0011, 0.35, 1800, 1 }
};

/***** LOCAL FUNCTIONS *****************************************************/

static void game_setup_title_and_author( Game *game, Level *level )
{
  switch (level->type)
    {
    case LT_NORMAL:
      snprintf( game->title, 32, "%s", level->name );
      snprintf( game->author, 32, "%s", level->author );
      break;
    case LT_JUMPING_JACK:
      snprintf( game->title, 32, _("Jumping Jack") );
      snprintf( game->author, 32, _("Bonus Level") );
      break;
    case LT_OUTBREAK:
        snprintf( game->title, 32, _("Outbreak") );
        snprintf( game->author, 32, _("Bonus Level") );
        break;
    case LT_BARRIER:
        snprintf( game->title, 32, _("Barrier") );
        snprintf( game->author, 32, _("Bonus Level") );
        break;
    case LT_SITTING_DUCKS:
        snprintf( game->title, 32, _("Sitting Ducks") );
        snprintf( game->author, 32, _("Bonus Level") );
        break;
    case LT_HUNTER:
        snprintf( game->title, 32, _("Hunter") );
        snprintf( game->author, 32, _("Bonus Level") );
        break;
    case LT_DEFENDER:
        snprintf( game->title, 32, _("Defender") );
        snprintf( game->author, 32, _("Bonus Level") );
        break;
    default:
      snprintf( game->title, 32, _("Unknown Level Type") );
      snprintf( game->author, 32, "???" );
      break;
    }
}

/***** PUBLIC FUNCTIONS ****************************************************/

/* create/delete game context */
Game *game_create( int game_type, int diff, int rel_warp_limit )
{
	Game *game = salloc( 1, sizeof( Game ) );

	/* set diff and game type */
	game->game_type = game_type;
	if ( game_type == GT_LOCAL )
		game->diff = &diffs[diff];
	else
		game->diff = &net_diffs[diff];
	game->rel_warp_limit = rel_warp_limit;
	
	/* create lists */
	game->shots = list_create( LIST_AUTO_DELETE, LIST_NO_CALLBACK );
	game->exp_bricks = list_create( LIST_NO_AUTO_DELETE, LIST_NO_CALLBACK );
	game->heal_bricks = list_create( LIST_NO_AUTO_DELETE, LIST_NO_CALLBACK );
	game->extras = list_create( LIST_AUTO_DELETE, LIST_NO_CALLBACK );
	game->balls = list_create( LIST_AUTO_DELETE, LIST_NO_CALLBACK );

	/* set ball speed */
	game->ball_v_min = game->diff->v_start;
	game->ball_v_max = game->diff->v_max;
	delay_set( &game->speedup_delay, game->diff->v_delay );
	
	/* create paddles */
	game->paddle_count = (game_type==GT_NETWORK)?2:1;
	/* bottom */
        game->paddles[PADDLE_BOTTOM] = paddle_create( 0, PADDLE_BOTTOM, 
		(MAP_HEIGHT-2)*BRICK_HEIGHT, 
		game->diff->paddle_size, 
		game->diff->paddle_min_size, game->diff->paddle_max_size,
		0 );
	/* top */
	if ( game_type == GT_NETWORK )
        	game->paddles[PADDLE_TOP] = paddle_create( 0, PADDLE_TOP,
			BRICK_HEIGHT+2, 
			game->diff->paddle_size, 
			game->diff->paddle_min_size, game->diff->paddle_max_size,
			0 );
	
	return game;
}
void game_delete( Game **_game )
{
	Game *game = *_game;
	int i;
	
	if ( game == 0 ) return;
	
	/* delete paddles */
	for ( i = 0; i < game->paddle_count; i++ )
		paddle_delete( game->paddles[i] );
	
	/* delete lists */
	if ( game->shots ) list_delete( game->shots );
	if ( game->exp_bricks ) list_delete( game->exp_bricks );
	if ( game->heal_bricks ) list_delete( game->heal_bricks );
	if ( game->extras ) list_delete( game->extras );
	if ( game->balls ) list_delete( game->balls );

    /* delete bonus level stuff */
    if (game->blDuckPositions) free(game->blDuckPositions);
	if (game->blInvaders) free(game->blInvaders);
    
    free( game );
	*_game = 0;
}

/* finalize single game level. the level_type is determined by
 * counting the bricks. the data of 'level' is copied and modified
 * while playing. */
void game_init( Game *game, Level *level )
{
  int i;
  Ball *ball;

  game->level_over = 0;
  game->isBonusLevel = 0;
  game->totalBonusLevelScore = 0;

  /* set title and author */
  game_setup_title_and_author( game, level );

  /* set level type. is level::type except for normal level with no bricks: pingpong */
  if (level->type==LT_NORMAL && game->game_type == GT_NETWORK && game->brick_count == 0 )
    game->level_type = LT_PINGPONG;
  else
  {
      game->level_type = level->type;
      if (game->level_type!=LT_NORMAL) game->isBonusLevel = 1;
  }
  
  /* clear extras */
  memset( game->extra_active, 0, sizeof( game->extra_active ) );
  memset( game->extra_time, 0, sizeof( game->extra_time ) );

  /* set ball speed */
  game->ball_v = game->ball_v_min;
  game->speedup_level = 0;
	
  /* clear maxballspeed_request */
  if ( game->game_type == GT_LOCAL )
    {
      cur_game->paddles[0]->maxballspeed_request = 0;
      cur_game->paddles[0]->maxballspeed_request_old = 0;
    }
        
  /* attach one ball to each paddle */
  list_clear( game->balls );
  for ( i = 0; i < game->paddle_count; i++ ) {
    if ( game->paddles[i]->type == PADDLE_BOTTOM )
      ball = ball_create((game->paddles[i]->w - ball_w) / 2, -ball_dia );
    else
      ball = ball_create((game->paddles[i]->w - ball_w) / 2, game->paddles[i]->h );
    ball->attached = 1;
    ball->paddle = game->paddles[i];
    ball->paddle->attached_ball_count = 1;
    ball_set_random_angle( ball, game->ball_v );
    list_add( game->balls, ball );
  }
  
  /* do bricks as last to have influence on balls to keep bonus level stuff in one place */
  /* setup bricks (from level data or from special level type; this includes setting the
     bonus level data if any */
  bricks_init( game, game->game_type, level, game->diff->score_mod, game->rel_warp_limit );
    
}

/* reset level/in_game data */
void game_finalize( Game *game )
{
	int i;
	
	/* reset lists */
	list_clear( game->balls );
	list_clear( game->extras );
	list_clear( game->shots );
	list_clear( game->heal_bricks );
	list_clear( game->exp_bricks );

	/* reset paddles (and their statistics which are only for 
	 * the currently played level) */
	for ( i = 0; i < game->paddle_count; i++ )
		paddle_reset( game->paddles[i] );

	/* reset updates */
	game_reset_mods();
}

/* set the game context the subfunctions will apply their changes to */
void game_set_current( Game *game )
{
	cur_game = game;
}

/* set score of paddle 'id'. 0 is bottom paddle and 1 is top paddle */
void game_set_score( int id, int score ) 
{
	if ( id < 0 || id >= cur_game->paddle_count ) return;
	cur_game->paddles[id]->score = score;
}

/* set number of additional balls a paddle can fire (all paddles) */
void game_set_ball_ammo( int ammo )
{
	int i;
	for ( i = 0; i < cur_game->paddle_count; i++ ) {
		cur_game->paddles[i]->ball_ammo = ammo;
		cur_game->paddles[i]->start_ball_ammo = ammo;
	}
}

/* set the number of points required to win a PINGPONG level */
void game_set_frag_limit( int limit )
{
	cur_game->frag_limit = limit;
}

/* set wether to use convex paddle */
void game_set_convex_paddle( int convex )
{
	cur_game->paddle_is_convex = convex;
}

/* set wether balls are returned to a paddle by pressing fire.
 * the alternative is that they automatically return. */
void game_set_ball_auto_return( int auto_return )
{
	cur_game->balls_return_by_click = !auto_return;
}

/* set wether balls are fired at random angle or wether the
 * left/right fire keys are used */
void game_set_ball_random_angle( int random )
{
	cur_game->balls_use_random_angle = random;
}

/* set the speed of balls will have in accelerated state */
void game_set_ball_accelerated_speed( float speed )
{
    cur_game->accelerated_ball_speed = speed;
}

/* update state of a paddle. x or y may be 0 which is not a valid value.
 * in this case the property is left unchanged */
void game_set_paddle_state( int id, int x, int y, int left_fire, int right_fire, int return_key )
{
	Paddle *paddle = 0;
	
	if ( id < 0 || id >= cur_game->paddle_count ) return;
	
	paddle = cur_game->paddles[id];
	if ( x != 0 ) { paddle->x = x; paddle->cur_x = x; }
	if ( y != 0 ) paddle->y = y;
	paddle->fire_left = left_fire;
	paddle->fire_right = right_fire;
	paddle->ball_return_key_pressed = return_key;
}

/* move objects, modify game data, store brick hits and collected extras.
 * return wether level has been finished and the id of the winning paddle
 * in network games. -1 is a draw. level_over and winner is saved in the
 * game struct. */
void game_update( int ms )
{
	int i;

	extras_update( ms );
	walls_update( ms );
	shots_update( ms );
	bricks_update( ms );
	for ( i = 0; i < cur_game->paddle_count; i++ )
	  {
		paddle_update( cur_game->paddles[i], ms );
		/* release all balls from paddle if invisible */
		if (!paddle_solid(cur_game->paddles[i]))
		    balls_detach_from_paddle( cur_game->paddles[i], ((rand()%2==1)?-1:1) );
	  }
		  
	balls_update( ms );

	/* level finished? */
	cur_game->level_over = 0;
	if ( cur_game->game_type == GT_LOCAL ) {
		/* local game */
		if ( cur_game->bricks_left == 0 ) cur_game->level_over = 1;
		if ( cur_game->balls->count == 0 ) cur_game->level_over = 1;
	} else {
		/* network game */
		if ( cur_game->level_type != LT_PINGPONG ) {
			if ( cur_game->bricks_left == 0 ) cur_game->level_over = 1;
		} 
		else
		if ( cur_game->paddles[PADDLE_TOP]->score >= cur_game->frag_limit ||
		     cur_game->paddles[PADDLE_BOTTOM]->score >= cur_game->frag_limit )
			cur_game->level_over = 1;
	}

	/* if so, determine winner */
	if ( cur_game->level_over ) {
		if ( cur_game->game_type == GT_LOCAL ) {
			if ( cur_game->bricks_left == 0 || cur_game->isBonusLevel )
				cur_game->winner = PADDLE_BOTTOM; /* praise */
			else
				cur_game->winner = PADDLE_TOP; /* swear */
		} else {
			cur_game->winner = PADDLE_BOTTOM;
			if ( cur_game->game_type == GT_NETWORK ) {
				if ( cur_game->paddles[PADDLE_TOP]->score >
						cur_game->paddles[PADDLE_BOTTOM]->score )
					cur_game->winner = PADDLE_TOP;
				else
					if ( cur_game->paddles[PADDLE_TOP]->score == 
							cur_game->paddles[PADDLE_BOTTOM]->score )
						cur_game->winner = -1;
			}
		}
	}
}

/* get the modifications that occured in game_update() */

/* get current score of player. return 0 if player does not exist */
int game_get_score( int id, int *score )
{
	if ( id < 0 || id >= cur_game->paddle_count ) return 0;
	*score = cur_game->paddles[id]->score;
	return 1;
}

/* get number of ball reflections */
int game_get_reflected_ball_count( void )
{
	return cur_game->mod.brick_reflected_ball_count+
	       cur_game->mod.paddle_reflected_ball_count;
}

/* get number of ball reflections on bricks */
int game_get_brick_reflected_ball_count( void )
{
	return cur_game->mod.brick_reflected_ball_count;
}

/* get number of ball reflections on paddle */
int game_get_paddle_reflected_ball_count( void )
{
	return cur_game->mod.paddle_reflected_ball_count;
}

/* get number of newly attached balls */
int game_get_attached_ball_count( void )
{
	return cur_game->mod.attached_ball_count;
}

/* get number of fired shots no matter which paddle */
int game_get_fired_shot_count( void )
{
	return cur_game->mod.fired_shot_count;
}

/* hit bricks since last call to game_update() */
BrickHit *game_get_brick_hits( int *count )
{
	*count = cur_game->mod.brick_hit_count;
	return cur_game->mod.brick_hits;
}

/* get a list of extras collected by paddle id */
int *game_get_collected_extras( int id, int *count )
{
	*count = 0;
	if ( id < 0 || id >= cur_game->paddle_count ) return 0;
	*count = cur_game->mod.collected_extra_count[id];
	return cur_game->mod.collected_extras[id];
}

/* get a snapshot of the level data which is the brick states
 * converted to the original file format. this can be used to
 * overwrite a levels data when player changes in alternative
 * game */
void game_get_level_snapshot( Level *shot )
{
	int i, j;
	int y_off;
	
	if ( cur_game->game_type == GT_NETWORK ) 
		y_off = ( MAP_HEIGHT - EDIT_HEIGHT ) / 2;
	else
		y_off = 1;

	for ( i = 0; i < EDIT_WIDTH; i++ )
	for ( j = 0; j < EDIT_HEIGHT; j++ ) {
		shot->bricks[i][j] = cur_game->bricks[i+1][j+y_off].brick_c;
		shot->extras[i][j] = cur_game->bricks[i+1][j+y_off].extra_c;
	}
}

/* reset the modification of game_update() */
void game_reset_mods( void )
{
	memset( &cur_game->mod, 0, sizeof( GameMod ) );
}

/* update a statistics struct by the level stats of a paddle.
 * updates the win/loss/draw as well. the played_rounds entry
 * is simply increased everytime this function is called */
void game_update_stats( int id, GameStats *stats )
{
	Paddle *paddle;
	if ( id < 0 || id >= cur_game->paddle_count ) return;
	
	/* this should be called before game_finalize() as the
	 * stats will be cleared there */
	paddle = cur_game->paddles[id];
	
	stats->total_score += paddle->score;
    if ( stats->total_score < 0 ) stats->total_score = 0;
	stats->balls_reflected += paddle->balls_reflected;
	stats->balls_lost += paddle->balls_lost;
	stats->bricks_cleared += paddle->bricks_cleared;
	stats->total_brick_count += cur_game->brick_count;
	stats->extras_collected += paddle->extras_collected;
	stats->total_extra_count += cur_game->extra_count;

	if ( cur_game->winner == -1 )
		stats->draws++;
	else
	if ( cur_game->winner == id )
		stats->wins++;
	else
		stats->losses++;
	stats->played_rounds++;
}

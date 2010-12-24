/***************************************************************************
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

#ifndef __GAME_H
#define __GAME_H

/***** INCLUDES ************************************************************/

#include "../client/lbreakout.h"
#include "mathfuncs.h"
#include "levels.h"
#include "extras.h"
#include "balls.h"
#include "shots.h"
#include "bricks.h"
#include "paddle.h"
#include "comm.h"

/***** TYPE DEFINITIONS ****************************************************/

/***** PUBLIC FUNCTIONS ****************************************************/

/* create/delete game context 
   game_type is either GT_LOCAL, GT_NETWORK_CLIENT, GT_NETWORK_SERVER.
   if it is GT_NETWORK_..., it is changed to GT_NETWORK while setting 
   isServerSidedGame */
Game *game_create( int game_type, int diff, int rel_warp_limit );
void game_delete( Game **game );

/* finalize single game level. the level_type is determined by
 * counting the bricks. the data of 'level' is copied and modified
 * while playing. */
void game_init( Game *game, Level *level );

/* reset level/in_game data */
void game_finalize( Game *game );

/* set the game context the subfunctions will apply their changes to */
void game_set_current( Game *game );

/* set score of paddle 'id'. 0 is bottom paddle and 1 is top paddle */
void game_set_score( int id, int score );

/* set number of additional balls a paddle can fire (all paddles) */
void game_set_ball_ammo( int ammo );

/* set the number of points required to win a PINGPONG level */
void game_set_frag_limit( int limit );

/* set wether to use convex paddle */
void game_set_convex_paddle( int convex );

/* set wether balls are returned to a paddle by pressing fire.
 * the alternative is that they automatically return. */
void game_set_ball_auto_return( int auto_return );

/* set wether balls are fired at random angle or wether the
 * left/right fire keys are used */
void game_set_ball_random_angle( int random );

/* set the speed of balls will have in accelerated state */
void game_set_ball_accelerated_speed( float speed );

/* update state of a paddle. x or y may be 0 which is not a valid value.
 * in this case the property is left unchanged */
void game_set_paddle_state( int id, int x, int y, int left_fire, int right_fire, int return_key );

/* move objects, modify game data, store brick hits and collected extras.
 * return wether level has been finished and the id of the winning paddle
 * in network games. -1 is a draw. level_over and winner is saved in the
 * game struct. */
void game_update( int ms );

/* get the modifications that occured in game_update() */

/* get current score of player. return 0 if player does not exist */
int game_get_score( int id, int *score );

/* get number of ball reflections */
int game_get_reflected_ball_count( void );
int game_get_brick_reflected_ball_count( void );
int game_get_paddle_reflected_ball_count( void );

/* get number of newly attached balls */
int game_get_attached_ball_count( void );

/* get number of fired shots no matter which paddle */
int game_get_fired_shot_count( void );

/* hit bricks since last call to game_update() */
BrickHit *game_get_brick_hits( int *count );

/* get a list of extras collected by paddle id */
int *game_get_collected_extras( int id, int *count );

/* get a snapshot of the level data which is the brick states
 * converted to the original file format. this can be used to
 * overwrite a levels data when player changes in alternative
 * game */
void game_get_level_snapshot( Level *snapshot );

/* reset the modification of game_update() */
void game_reset_mods( void );

/* update a statistics struct by the level stats of a paddle.
 * updates the win/loss/draw as well. the played_rounds entry
 * is simply increased everytime this function is called */
void game_update_stats( int id, GameStats *stats );

#endif


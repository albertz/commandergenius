/***************************************************************************
                          local_game.h  -  description
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

/* in game client states */
enum {
	CS_NONE = 0,
	CS_RECV_LEVEL,	/* wait for level data from server */
	CS_SCORE_TABLE,	/* score table in local game */
	CS_FINAL_TABLE, /* final score table in local game */
	CS_FINAL_PLAYER_INFO, /* info that player cleared all levels */
	CS_RECV_STATS, /* wait for server to send stats */
	CS_FINAL_STATS, /* looking at final network player stats */
	CS_GET_READY,	/* await click from user */
	CS_PLAY,	/* render frame */
	CS_NEXT_PLAYER,	/* cycle to next player in local game */
	CS_NEXT_LEVEL,	/* init next level for player in local game */
	CS_RESTART_LEVEL, /* player begins at reset level next time */
	CS_LOOSE_LIFE,	/* take a snapshot and decrease lives */
	CS_ROUND_RESULT,/* await click and start level receive after it */
	CS_GAME_OVER,	/* all levels are played */
    CS_CONFIRM_WARP,
	CS_CONFIRM_QUIT,
	CS_CONFIRM_RESTART,
    CS_CONFIRM_CONTINUE,
	CS_FATAL_ERROR,	/* display error and quit game after it */
	CS_PAUSE,
    CS_BONUS_LEVEL_SCORE, /* show score gained in bonus level */
    CS_GET_READY_FOR_NEXT_LEVEL /* wait for click when showing score of bonus level */
};

/* create various resources like shrapnells */
void client_game_create();
void client_game_delete();

/* create network/local game context and initiate game state:
 * network needs to receive the level data and a local game
 * has to load the next level */
int client_game_init_local( char *setname );
int client_game_init_network( char *opponent_name, int diff );

/* create local game context and initiate game state
 * as given from slot 'slot_id'. */
int client_game_resume_local( int slot_id );

/* finalize a game and free anything allocated by init process */
void client_game_finalize();

/* run the state driven loop until game is broken up or finished */
void client_game_run( void );

/* test a level until all balls got lost */
void client_game_test_level( Level *level );


/***************************************************************************
                          server.h  -  description
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

#ifndef __SERVER_H
#define __SERVER_H

/***** INCLUDES ************************************************************/

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#include <SDL.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include "../common/tools.h"
#include "../common/net.h"
#include "../common/messages.h"
#include "../common/list.h"
#include "../common/parser.h"
#include "../game/game.h"

/* i18n */
#include "../common/gettext.h"
#if ENABLE_NLS
#define _(str) gettext (str)
#else
#define _(str) (str)
#endif

/***** TYPE DEFINITIONS ****************************************************/

typedef struct {
	int 	   id;		/* global user id */
	char       name[20];
        int        no_comm;     /* user does not communicate: bots and disconnected
                                   users have this set True */
	int	   bot;	        /* if True this is a paddle bot */
        int        bot_level;   /* paddle speed in pix/sec */
	NetSocket  socket;	/* transmission socket */
	int	   hidden;	/* True if chatroom user is not visible
				   (e.g. looking at game statistics or playing) */
	int	   admin;	/* wether user logged in with admit rights */
	void	   *game;	/* FIXME: I'M A BAD HACK: points to the current ServerGame
	                           if this user is playing */
	int	   player_id;	/* 0 - challenger in game or 1 - challenged */
} ServerUser;

typedef struct {
	char       name[20];
	int	   id;
	List	   *users;	
} ServerChannel;

enum {
	SERVER_AWAIT_ACCEPT, /* the first user has created the game and is already
				added. a challenge note is sent to the challenged
				user. in this state the server awaits either a 
				ACCEPTED or REFUSED message by the remote 
				or a CANCEL, DISCONNECT by the challenger */
	SERVER_AWAIT_READY, /* await ready message from both users, then play */
	SERVER_PLAY,	/* now the game is actually running: valid messages are
			   paddle updates, pause requests, chat messages and exits */
	SERVER_PAUSE	/* game is paused and players may exchange chat messages
			   until one sends an UNPAUSE */
}; /* states of a server game */

typedef struct {
	ServerUser	*challenger;
	ServerUser	*challenged;	/* the two users that will play */

	char		name[20];	/* name of the levelset */
	int		diff;		/* difficulty level */
	int		rounds;		/* per level */
	int		frags;		/* DM: frags required to win */
	int		balls;		/* DM: number of balls a paddle may fire */
} ServerGameCtx;

typedef struct {
	int		id;		/* users join a game by sending its id */
	int		state;		/* current state as above */ 
	ServerUser	*users[2];	/* pointers to chat channel:
				   	   0 - challenger
				   	   1 - challenged */
	ServerChannel	*channel;	/* the channel the users are in */
	
	LevelSet	*set;		/* pointer to the current set */
	int		rounds_per_level;
	int		rounds;		/* level count * rounds per level */
	int		cur_round;	/* id of current round. level id
					   is cur_round / rounds */
	int		cur_level;	/* id of current level */
	
	int		ready[2];	/* wether user has sent READY */
	Game		*game;		/* the actual game data used by server */
	GameStats	stats[2];	/* game stats: 
					   0 - challenger
					   1 - challenged */
	int		game_over;	/* True after last round was finalized
					   and stats have been send. */
} ServerGame;

/***** PUBLIC FUNCTIONS ****************************************************/

#endif

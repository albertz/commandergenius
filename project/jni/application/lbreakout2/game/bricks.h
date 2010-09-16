/***************************************************************************
                          bricks.h  -  description
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

#ifndef __BRICKS_H
#define __BRICKS_H

/* extra conversion item */
typedef struct {
	int  type; /* extra of type */
	char c;   /* is assigned to this character */
} Extra_Conv;
/* brick conversion item */
typedef struct {
	char c;   /* is assigned to this character */
	int  type; /* extra of type */
	int  id; /* pic id */
	int  dur;
	int  score; 
} Brick_Conv;

/*
====================================================================
Init bricks from level data, set the warp limit (percent) and 
add regenerating bricks. As this function is called when 
initializing a level it does not use the 'cur_game' context.
'score_mod' is percentual and 100 means normal score.
====================================================================
*/
void bricks_init( Game *game, int game_type, Level *level, int score_mod, int rel_warp_limit  );
/*
====================================================================
Hit brick and remove if destroyed. 'metal' means the ball
destroys any brick with the first try.
type and imp are used for shrapnell creation.
'extra' contains the pushed extra if one was released.
'paddle' is the paddle that initiated hit either by shot or ball.
Return true on destruction
====================================================================
*/
int brick_hit( int mx, int my, int metal, int type, Vector imp, Paddle *paddle );
/*
====================================================================
Make brick at mx,my loose 'points' duration. It must have been
previously checked that this operation is completely valid.
It does not update net_bricks or the player's duration reference.
====================================================================
*/
void brick_loose_dur( int mx, int my, int points );

/*
====================================================================
Initiate a brick explosion.
====================================================================
*/
void brick_start_expl( int x, int y, int time, Paddle *paddle );

/* add a modification to the list. if 'mod' is HT_HIT and the
 * tile is empty it is an HT_REMOVE. 'type' is the type of
 * the responsible source and 'src' its impact vector. */
void bricks_add_mod( int x, int y, int mod, int dest_type, Vector imp, Paddle *paddle );
void bricks_add_grow_mod( int x, int y, int id );

/* update regeneration and explosion of bricks */
void bricks_update( int ms );

/* return the character that represents the brick with this type id */
char brick_get_char( int type );

#endif

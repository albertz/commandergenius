/***************************************************************************
                          paddle.h  -  description
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

#ifndef __PADDLE_H
#define __PADDLE_H

/*
====================================================================
Create/delete a paddle. x is centered assuming a screen width of
640. ball_ammo is the number of additional balls paddle
 may bring to game. 
====================================================================
*/
Paddle* paddle_create( 
	int score, int type, int y, int size, int min_size, int max_size, int ball_ammo );
void paddle_delete( Paddle *paddle );
void paddle_reset( Paddle *paddle ); /* will clear stats and score too */

void paddle_freeze( Paddle *paddle, int freeze );
void paddle_set_slime( Paddle *paddle, int slime );
int paddle_slimy( Paddle *paddle );

/*
====================================================================
Set if paddle attracts boni/mali or none.
====================================================================
*/
void paddle_set_attract( Paddle *paddle, int attract );
int paddle_attract_malus( Paddle *paddle );
int paddle_attract_bonus( Paddle *paddle );
int paddle_check_attract( Paddle *paddle, int type );

/*
====================================================================
(De)activate ghostly behaviour: paddle is only seen when moved at
maximum 200ms before.
====================================================================
*/
void paddle_set_invis( Paddle *paddle, int invis );
int  paddle_solid( Paddle *paddle );

/*
====================================================================
Init paddle resize (the change between wanted_w and w MUST be
even (else the paddle shrinks til eternity)
====================================================================
*/
int paddle_init_resize( Paddle *paddle, int c );
/*
====================================================================
Actually resize paddle and return the -1 for shrinkage and 1 
for expansion.
====================================================================
*/
int paddle_resize( Paddle *paddle, int ms );

/*
====================================================================
Update paddle resize and fire weapon.
====================================================================
*/
void paddle_update( Paddle *paddle, int ms );
/*
====================================================================
De/Activate weapon
====================================================================
*/
void weapon_install( Paddle *paddle, int install );

#endif


/***************************************************************************
                          shots.h  -  description
                             -------------------
    begin                : Sat Sep 8 2001
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

#ifndef __SHOTS_H
#define __SHOTS_H

/*
====================================================================
Create new shot at position (centered).
'signum' of direction determines into which direction the shot 
vertically goes.
====================================================================
*/
void shot_create( Paddle *paddle );

/*
====================================================================
Set 'get_target' flag so target is updated next time 
'shots_update' is called. -1 means to update all shots.
====================================================================
*/
void shots_check_targets( int mx, int my );

/*
====================================================================
Update position of shots and check if bricks get destroyed.
A list of all hit bricks is returned (at maximum 
PADDLE_WEAPON_AMMO * 4)
====================================================================
*/
void shots_update( int ms );

#endif


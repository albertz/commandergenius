/***************************************************************************
                          extras.h  -  description
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

#ifndef __EXTRAS_H
#define __EXTRAS_H

/*
====================================================================
Create new extra at position
====================================================================
*/
Extra *extra_create( int type, int x, int y, int dir );
/*
====================================================================
Use extra when paddle collected it
====================================================================
*/
void extra_use( Paddle *paddle, int type );
/*
====================================================================
Update extras
====================================================================
*/
void extras_update( int ms );
/*
====================================================================
Wall
====================================================================
*/
void walls_update( int ms );


int extra_is_malus( int type );

#endif


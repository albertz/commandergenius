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

/* Play sounds and handle graphical effects. */
void client_handle_collected_extra( Paddle *paddle, int extra_type );

/* move the extras as in extras_update but do not collect them */
void client_extras_update( int ms );

/*
====================================================================
Show, hide extras
====================================================================
*/
void extras_hide();
void extras_show_shadow();
void extras_show();
void extras_alphashow( int alpha );
/*
====================================================================
Wall
====================================================================
*/
void walls_hide();
void walls_show();
void walls_alphashow( int alpha );
void client_walls_update( int ms );


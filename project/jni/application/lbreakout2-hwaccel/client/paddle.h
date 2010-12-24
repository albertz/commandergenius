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

/*
====================================================================
Hide and show paddle on screen.
====================================================================
*/
void paddle_hide( Paddle *paddle );
void paddle_show_shadow( Paddle *paddle );
void paddle_show( Paddle *paddle );
void paddle_alphashow( Paddle *paddle, int a );
/*
====================================================================
Update paddle animation (weapon)
====================================================================
*/
void client_paddle_update( Paddle *paddle, int ms );
/*
====================================================================
Handle events (used for local paddle)
====================================================================
*/
void paddle_handle_events( Paddle *paddle, int ms );

/* draw ammo in network game */
void paddle_ammo_hide( Paddle *paddle );
void paddle_ammo_show( Paddle *paddle );


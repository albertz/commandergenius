/***************************************************************************
                          balls.h  -  description
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

#ifndef __BALLS_H
#define __BALLS_H

/*
====================================================================
Load/delete ball graphics
====================================================================
*/
void ball_load();
void ball_delete();
/*
====================================================================
Create ball at position
====================================================================
*/
Ball* ball_create( int x, int y );
/*
====================================================================
Set a special ball property like metal ball.
====================================================================
*/
void balls_set_type( int type );
/*
====================================================================
Set chaotic behaviour (random relfection)
====================================================================
*/
void balls_set_chaos( int chaos );
/*
====================================================================
Clear ball list and attach one ball to paddle
====================================================================
*/
void balls_reset();
/*
====================================================================
Show/hide all balls
====================================================================
*/
void balls_hide();
void balls_show_shadow();
void balls_show();
void balls_alphashow( int alpha );
/*
====================================================================
Update balls and detach attached balls if fire was pressed.
====================================================================
*/
void balls_update( int ms );
/*
====================================================================
All balls with target mx,my will have there 'get_target' flag
set True so they compute a new target next time balls_update()
is called. If 'mx' is -1 all balls will set their flag.
====================================================================
*/
void balls_check_targets(int mx, int my);
/*
====================================================================
Adjust velocity of ball to spare out any illegal values.
Add a little entropy to the vector if 'entropy' is True.
====================================================================
*/
void ball_mask_vel(Ball *b, float old_vx, int entropy );
/*
====================================================================
Get target for a ball.
input :
    Ball* b
function :
    -check if ball b hits a brick and if so:
    -compute the hitten brick in lev_map (int mx, my)
    -the reset position of the ball after destroying the brick (float x, y)
    -the time in milliseconds it takes the ball to hit the brick from its current position
    by using ball_v as velocity (int time)
    -the side at which the ball hits; might be LEFT, RIGHT, TOP, BOTTOM (int side)
    -the reflection vector (float a); if reflecting at an horizontal wall it's a = {0, 1} else a = {1, 0}
====================================================================
*/
void ball_get_target( Ball *b );
/*
====================================================================
Increase velocity acording to vel_change
====================================================================
*/
void balls_inc_vel( int ms );
/*
====================================================================
Return all balls that have ball->return_allowed True to the paddle
(if they touched this one as last).
====================================================================
*/
void balls_return( Paddle *paddle );

/* set random starting angle for ball according to its paddle */
void ball_set_random_angle( Ball *ball, double ball_v );

/*
====================================================================
Check if the ball is on paddle's level and an reflect is
possible.
====================================================================
*/
int ball_paddle_contact_possible( Ball *ball, Paddle *paddle, Vector old );

/*
====================================================================
Check reflection of ball at paddle. 'old' is the position of
the ball before update. Used to compute direction.
====================================================================
*/
void ball_check_paddle_reflection( Ball *ball, Paddle *paddle );

/*
====================================================================
Set velocity of all balls and get new targets if any.
====================================================================
*/
void balls_set_velocity( List *balls, double vel );

/*
====================================================================
Detach all balls to the passed direction (-1 or 1) and return True
if there were any balls detached. As balls within walls are not
fired the result may differ from paddle::attached_ball_count!
====================================================================
*/
int balls_detach_from_paddle( Paddle *paddle, int dir );
  
/*
====================================================================
Clear contents of target.
====================================================================
*/
void ball_clear_target( Target *t );

#endif


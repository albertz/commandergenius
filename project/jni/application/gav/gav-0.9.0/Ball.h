/* -*- C++ -*- */
/*
  GAV - Gpl Arcade Volleyball
  
  Copyright (C) 2002
  GAV team (http://sourceforge.net/projects/gav/)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _BALL_H_
#define _BALL_H_

#include <SDL.h>
#include <math.h>
#include <map>
#include "FrameSeq.h"
#include "Team.h"
#include "globals.h"
#include "Theme.h"

#define ELASTIC_SMOOTH (0.8)

#define MIN_NET_SPEED (10)

//#define NET_X    (312)
//#define NET_X    (configuration.SCREEN_WIDTH / 2 - 8)
//#define NET_Y    (configuration.SCREEN_HEIGHT /2 + ((3*configuration.SCREEN_HEIGHT)/200))
//#define CEILING_Y  24
//#define LEFT_WALL  7 
//#define RIGHT_WALL(bw) (configuration.SCREEN_WIDTH - bw - 16)

typedef enum {
  BALL_ORIG
} ball_t;

class Ball { 
 private:
  FrameSeq * _frames;
  int _speed;
  float _angle;
  int _spdx, _spdy;
  int _x, _y;
  int _oldx, _oldy;
  int _frameIdx;
  int _radius;
  int _beginning;
  int _accelY;
  int _side;
  int _scorerSide;
  int _scoredTime;
  std::map<Team *, int> _collisionCount;
  Player * _inCollisionWith;

  void loadFrameSeq(ball_t t) {
    switch (t) {
    case BALL_ORIG:
      _frames = new LogicalFrameSeq(CurrentTheme->ball(),
				    configuration.ballFrameConf.nBallFrames);
      break;
    }
  }


  void assignPoint(int side, Team *t);
  bool approaching(int spdx, float spdy);

  // evaluates a collision ("sprite"-wise)
  bool spriteCollide(Player *p);

  // evaluate a collision
  bool collide(Player *p);  
  void update_internal(Player * pl);

  //  void assignPoint(int side, Team *t);

  bool netPartialCollision(int, int);
  bool netFullCollision(int, int);
  
  inline void update_direction(int ticks)
  {
    _spdy -= (_accelY * ticks / 1000);
  }

  void resetCollisionCount();

 public:
  Ball(ball_t t, int x = -1, int y = -1) : _x(x), _y(y), _frameIdx(0) {
    _spdy = 0;
    _spdx = 0;
    loadFrameSeq(t);
    _radius = _frames->width() / 2;
    //cerr << "radius: " << _radius << endl;
    _beginning = 0;
    _accelY = 0; // as soon as the speed becomes <> 0, it becomes 500
    _inCollisionWith = NULL;
    _side = -1;
    if ( _x < 0 )
      _x = (configuration.SCREEN_WIDTH / 2) +
	((configuration.SCREEN_WIDTH * _side) / 4) ;
    if ( _y < 0 )
      _y = (configuration.SCREEN_HEIGHT * 2) / 3;
    _scorerSide = _scoredTime = 0;
  }

  void resetPos(int x, int y);

  inline int speed() { return _speed; }
  inline void speed(int v) { _speed = v; }
  inline int x() { return _x; }
  inline int y() { return _y; }
  inline int radius() { return _radius; }
  inline float angle() { return _angle; }
  inline void angle(float v) { _angle = v; }

  inline int spdx() { return _spdx; }
  inline int spdy() { return _spdy; }
  inline int frame() { return _frameIdx; }
  inline void setX(int x) { _x = x; }
  inline void setY(int y) { _y = y; }
  inline int gravity()   { return _accelY; }
  void updateFrame(int passed);
  void update(int passed, Team *tleft, Team *tright);
  float distance(int, int);

  
  void draw(SDL_Surface *scr = screen);

  ~Ball() {
    delete(_frames);
  }
};

#endif // _BALL_H_

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

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <SDL.h>
#include "FrameSeq.h"
#include <string>
#include "Theme.h"
#include "globals.h"

#define RESET_WALK_SEQUENCE (100000)

class Team;
class ControlsArray;

#define NUM_TYPES (5)
typedef enum {
  PL_TYPE_MALE_LEFT = 1,
  PL_TYPE_FEMALE_LEFT,
  PL_TYPE_MALE_RIGHT,
  PL_TYPE_FEMALE_RIGHT

} pl_type_t;

#define NUM_STATES (3)
typedef enum {
  PL_STATE_STILL = 0,
  PL_STATE_WALK,
  PL_STATE_JUMP
} pl_state_t;

typedef enum {
  PL_CTRL_HUMAN,
  PL_CTRL_AI,
  PL_CTRL_REMOTE
} pl_ctrl_t;

class Player {
protected:
  FrameSeq * _frames;
  std::string _name;
  pl_type_t  _type;
  pl_state_t _state;
  int _frameIdx;
  int _speed;
  int _x, _y;
  int _speedX;        // actual x speed
  float _speedY;
  int _plId;
  int _oif;
  int _currStateFrameDelay, _currFrameB, _currFrameE;
  Team *_team;
  int _overallPassed;
  float _displx, _disply;

  char *_fileNames[NUM_TYPES];

public:
  Player() {};
  Player(Team *team, std::string name, pl_type_t type, int idx, int speed) {
    init(team, name, type, idx, speed);
  }
  void init(Team *team, std::string name, pl_type_t type, int idx, int speed) {

    _fileNames[PL_TYPE_MALE_LEFT]    = 
	(char *)malloc(sizeof(char)*(MAXPATHLENGTH+1));
    strncpy(_fileNames[PL_TYPE_MALE_LEFT], 
	    CurrentTheme->leftmale(), MAXPATHLENGTH);

    _fileNames[PL_TYPE_MALE_RIGHT]   = 
	(char *)malloc(sizeof(char)*(MAXPATHLENGTH+1));
    strncpy(_fileNames[PL_TYPE_MALE_RIGHT], 
	    CurrentTheme->rightmale(), MAXPATHLENGTH);

    _fileNames[PL_TYPE_FEMALE_LEFT]  = 
	(char *)malloc(sizeof(char)*(MAXPATHLENGTH+1));
    strncpy(_fileNames[PL_TYPE_FEMALE_LEFT], 
	    CurrentTheme->leftfemale(), MAXPATHLENGTH);

    _fileNames[PL_TYPE_FEMALE_RIGHT] = 
	(char *)malloc(sizeof(char)*(MAXPATHLENGTH+1));
    strncpy(_fileNames[PL_TYPE_FEMALE_RIGHT], 
	    CurrentTheme->rightfemale(), MAXPATHLENGTH);

    _team   = team;
    _name   = name;
    _plId   = idx;
    _oif    = -1;
    _type   = type;
    _state  = PL_STATE_STILL;
    _frameIdx = configuration.playerFrameConf.playerStillB - 1;
    _speed  = speed;
    _speedX = 0;
    _speedY = 0.0;
    _overallPassed = 0;
    _displx = _disply = 0.0;
    Player::loadFrames();
    _y      = GROUND_LEVEL();
  }

  virtual ~Player() {
      free(_fileNames[PL_TYPE_MALE_LEFT]);
      free(_fileNames[PL_TYPE_MALE_RIGHT]);
      free(_fileNames[PL_TYPE_FEMALE_LEFT]);
      free(_fileNames[PL_TYPE_FEMALE_RIGHT]);
      delete(_frames);
  }

  inline int GROUND_LEVEL() { return(configuration.FLOOR_ORD -_frames->height());}  // (346)

  inline std::string name() {return _name;}
  
  inline int id() { return _plId; }
  inline int orderInField() { return _oif; }
  inline void setOIF(int oif) { _oif = oif; }

  inline pl_type_t type() {return _type;}

  inline pl_state_t state() {return _state;}
  bool setState(pl_state_t s, bool force = false);

  inline int speed() {return _speed;}
  inline void setSpeed(int s) {_speed = s;}
  
  inline float speedY() {return _speedY;}
  inline void setSpeedY(float s) {_speedY = s;}
  
  int speedX();
  inline int x() {return _x;}
  inline void setX(int x) {_x = x;}
  
  inline int y() {return _y;}
  inline void setY(int y) {_y = y;}
  inline Team *team() {return(_team);}

  void loadFrames() {
    _frames = new LogicalFrameSeq(_fileNames[_type],
				  configuration.playerFrameConf.nPlayerFrames);
  }

  void updateFrame(int ticks, bool changed);
  void updateClient(int ticks, pl_state_t state);

  void update(int ticks, ControlsArray *ca);
  void draw(SDL_Surface * screen);

  inline int width() { return _frames->width(); }
  inline int height() { return _frames->height(); }

  int minX();
  int maxX();
  int minY();


  bool collidesWith(FrameSeq *fs, int idx, SDL_Rect *rect);

  virtual pl_ctrl_t getCtrl() { return PL_CTRL_HUMAN; };

};

#endif

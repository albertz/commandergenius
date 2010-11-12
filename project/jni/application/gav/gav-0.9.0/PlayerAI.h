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

#ifndef __PLAYERAI_H__
#define __PLAYERAI_H__

#include "Player.h"
#include "ControlsArray.h"

class Ball;

class PlayerAI : public Player {
    Ball * _b;
protected:
    int _highestpoint;
public:
    PlayerAI(Team *team, std::string name, 
	     pl_type_t type, int idx, int speed,
	     Ball *b) {
	init(team, name, type, idx, speed);
	_b = b;
	_highestpoint = 0;
    }
    
    virtual pl_ctrl_t getCtrl() { return PL_CTRL_AI; }
    
    virtual triple_t planAction();
    
};

#endif

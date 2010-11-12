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

#ifndef __MENUITEMBALLSPEED_H__
#define __MENUITEMBALLSPEED_H__

#include <SDL.h>
#include "MenuItem.h"
#include "globals.h"

#define BALL_SPEED_ITEMS 3

class MenuItemBallSpeed: public MenuItem {
  char * _item[BALL_SPEED_ITEMS];
  int _currItem;

public:
  MenuItemBallSpeed() {
    if (configuration.ballAmplify == DEFAULT_BALL_AMPLIFY)
      _currItem = 0;
    else if ( configuration.ballAmplify ==
	      DEFAULT_BALL_AMPLIFY + BALL_SPEED_INC )
      _currItem = 1;
    else
      _currItem = 2;
    _item[0] = "Normal";
    _item[1] = "Fast";
    _item[2] = "Too Fast!";
    setLabel();
  }
  
  void setLabel() {
    label = std::string("Ball Speed: ") + 
      std::string(_item[_currItem]);
  }

  int execute(std::stack<Menu *> &s) {
    _currItem = (_currItem + 1) % BALL_SPEED_ITEMS;
    configuration.ballAmplify =
      DEFAULT_BALL_AMPLIFY + BALL_SPEED_INC * _currItem;
    setLabel();
    return(0);
  }
};

#endif

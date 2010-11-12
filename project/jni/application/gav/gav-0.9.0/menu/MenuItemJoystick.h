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

#ifndef __MENUITEMJOYSTICK_H__
#define __MENUITEMJOYSTICK_H__

#include <SDL.h>
#include "MenuItem.h"
#include "globals.h"

class MenuItemJoystick: public MenuItem {
  int _joyId;
  int _plId;
  char nstr[10];

 public:
  MenuItemJoystick(int plId, int init) {
    _joyId = init;
    _plId = plId;
    setLabel();
  }
  
  void setLabel() {
    label = "Player ";

    sprintf(nstr, "%d", _plId/2 + 1);
    label += std::string(nstr);
    if ( _plId % 2 )
      label += " right: ";
    else
      label += " left: ";

    if ( _joyId == -1 )
      label += "Not Present";
    else {
      sprintf(nstr, "%d", _joyId);
      label += "Joystick " + std::string(nstr);
    }
  }

  int execute(std::stack<Menu *> &s) {
    _joyId = (_joyId+1+1)%(controlsArray->getNJoysticks()+1) - 1;
    controlsArray->assignJoystick(_plId, _joyId);
    setLabel();
    return(0);
  }
};

#endif // __MENUITEMJOYSTICK_H__

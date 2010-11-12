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

#ifndef __MENUITEMSOUND_H__
#define __MENUITEMSOUND_H__

#include <SDL.h>
#include "MenuItem.h"
#include "globals.h"

class MenuItemSound: public MenuItem {
public:
  MenuItemSound() {
    label = std::string(configuration.sound?"Sound: On":"Sound: Off");
  }

  int execute(std::stack<Menu *> &s) {
    configuration.sound = (!configuration.sound);
    label = std::string(configuration.sound?"Sound: On":"Sound: Off");
    return(0);
  }
};

#endif // __MENUITEMSOUND_H__

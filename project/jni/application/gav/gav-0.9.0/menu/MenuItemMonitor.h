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

#ifndef __MENUITEMMONITOR_H__
#define __MENUITEMMONITOR_H__

#include <SDL.h>
#include "MenuItem.h"
#include "globals.h"

class MenuItemMonitor: public MenuItem {
public:
  MenuItemMonitor() {
    setLabel();
  }
  
  void setLabel() {
    std::string monitor;

    switch ( configuration.monitor_type ) {
    case MONITOR_NORMAL:
      monitor = "Normal";
      break;
    case MONITOR_OLD:
      monitor = "Old";
      break;
    case MONITOR_VERYOLD:
      monitor = "Very old";
      break;
    case MONITOR_VERYVERYOLD:
      monitor = "Very, very old";
      break;
    }

    label = std::string("Monitor Type: ") + monitor;
  }

  void apply() {
    if ( !configuration.monitor_type )
      SDL_SetAlpha(background->surface(), 0, 0);
    else
      SDL_SetAlpha(background->surface(), SDL_SRCALPHA | SDL_RLEACCEL,
		   128 - (configuration.monitor_type * 30));
  }
  
  int execute(std::stack<Menu *> &s) {
    configuration.monitor_type =
      (configuration.monitor_type + 1)%4;

    apply();
    setLabel();
    return(0);
  }
};

#endif

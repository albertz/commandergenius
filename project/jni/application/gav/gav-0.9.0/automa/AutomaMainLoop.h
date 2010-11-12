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

#include <SDL.h>
#include "Automa.h"
#include "InputState.h"
#include "globals.h"

#ifndef _AUTOMAMAINLOOP_H_
#define _AUTOMAMAINLOOP_H_

enum { STATE_MENU = 1,    // navigate menu tree
       STATE_PLAYING, // play the game
       STATE_CLIENT   // start the game as a client
};

class AutomaMainLoop : public Automa {
private:
  InputState *_is;
  int transition(int retval);
  
public:
  AutomaMainLoop();

  virtual int start();
};

#endif

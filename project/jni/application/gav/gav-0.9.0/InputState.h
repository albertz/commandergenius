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

#ifndef __INPUTSTATE_H__
#define __INPUTSTATE_H__

#include <string.h>
#include <SDL.h>
#include "globals.h"

class InputState {
private:
  Uint8 _f[12];
  Uint8 *keystate;
  SDL_Event event;

public:
  InputState() {}
  virtual void getInput();
  virtual Uint8 *getF() { return _f; }
  virtual Uint8 *getKeyState() { return keystate; }
  virtual SDL_Event getEvent() { return event; }
  virtual SDL_Event getEventWaiting();
  virtual ~InputState();
};

#endif


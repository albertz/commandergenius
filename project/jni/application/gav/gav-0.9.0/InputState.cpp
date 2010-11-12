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

#include <stdlib.h>
#include <SDL.h>
#include "InputState.h"

SDL_Event InputState::getEventWaiting() {
  SDL_Event e;
  SDL_WaitEvent(&e);
  return(e);
}

void InputState::getInput() {
  SDL_PumpEvents();
  
  /* Grab a snapshot of the keyboard. */
  keystate = SDL_GetKeyState(NULL); // SHOULD THIS BE DELETED AT SOME POINT??
  
  if ( SDL_PollEvent(&event) ) {
    if (event.type == SDL_QUIT)
      exit(0);
  }

  for ( int i = 0; i < 12; i++ )
    _f[i] = keystate[SDLK_F1 + i];
}

InputState::~InputState() {
}


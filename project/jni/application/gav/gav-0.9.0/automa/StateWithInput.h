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

#ifndef __STATENET_H__
#define __STATENET_H__

#include <SDL.h>
#include "AutomaMainLoop.h"
#include <string>

class StateWithInput {
 public:
  StateWithInput() {}
  
  std::string deleteOneChar(std::string s) {
    if ( s.length() < 1 )
      return(s);

    char s2[s.length()];

    strncpy(s2, s.c_str(), s.length() - 1);
    s2[s.length() - 1] = 0;
    return(std::string(s2));
  }

  short getKeyPressed(InputState *is, bool blocking = true) {
    SDL_keysym keysym;
    SDL_Event event;
    while ( 1 ) {
      if (!blocking && (!SDL_PollEvent(NULL)))
	return -1;
      if ( (event = is->getEventWaiting()).type != SDL_KEYDOWN ) {
	continue;
      }
      keysym = event.key.keysym;
      while ( is->getEventWaiting().type != SDL_KEYUP );
      char *kn = SDL_GetKeyName(keysym.sym);
      // printf("\"%s\"\n", kn);
      if ( strlen(kn) == 1 )
	return((signed char)(*kn));
      else if ( !strcmp(kn, "return") )
	return(SDLK_RETURN);
      else if ( !strcmp(kn, "backspace") )
	return(SDLK_BACKSPACE);
      else if ( !strcmp(kn, "escape") )
	return(SDLK_ESCAPE);
      else
	continue;

    }
    return -1;
  }

};

#endif

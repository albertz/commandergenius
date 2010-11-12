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

#ifndef __MENUKEYS_H__
#define __MENUKEYS_H__

#include <SDL.h>
#include "MenuItem.h"
#include "globals.h"

enum { PL1L, PL1R, PL1J, PL2L, PL2R, PL2J };

class MenuKeys: public Menu {
  int state;
  bool isPressed, isStillPressed;
  int _base;

public:
  MenuKeys(int base) {
    _base = base;
    state = PL1L;
    isPressed = false;
    isStillPressed = false;
  }

  virtual int execute(InputState *is, std::stack<Menu *> &s) {
    char numb[20];
    SDL_Rect rect;
    SDL_Event event = is->getEvent();
    rect.y = 30;
    rect.x = 30;    
    isStillPressed = (isPressed && (event.type == SDL_KEYDOWN));
    if ( !isStillPressed )
      isPressed = false;

    sprintf(numb, "Player %d ", 2*_base + ((state <= PL1J)?1:2));
    std::string st(numb);

    switch ( state ) {
    case PL1L:
      st += "Left: ";
      cga->printXY(screen, &rect, st.c_str());
      if ( ( event.type != SDL_KEYDOWN ) || isStillPressed )
	return(0);
      controlsArray->setControl(2*_base, CNTRL_LEFT, event.key.keysym.sym);
      break;
    case PL1R:
      st += "Right: ";
      cga->printXY(screen, &rect, st.c_str());
      if ( ( event.type != SDL_KEYDOWN ) || isStillPressed )
	return(0);
      controlsArray->setControl(2*_base, CNTRL_RIGHT, event.key.keysym.sym);
      break;
    case PL1J:
      st += "Jump: ";
      cga->printXY(screen, &rect, st.c_str());
      if ( ( event.type != SDL_KEYDOWN ) || isStillPressed )
	return(0);
      controlsArray->setControl(2*_base, CNTRL_JUMP, event.key.keysym.sym);
      break;
    case PL2L:
      st += "Left: ";
      cga->printXY(screen, &rect, st.c_str());
      if ( ( event.type != SDL_KEYDOWN ) || isStillPressed )
	return(0);
      controlsArray->setControl(2*_base + 1, CNTRL_LEFT, event.key.keysym.sym);
      break;
    case PL2R:
      st += "Right: ";
      cga->printXY(screen, &rect, st.c_str());
      if ( ( event.type != SDL_KEYDOWN ) || isStillPressed )
	return(0);
      controlsArray->setControl(2*_base + 1, CNTRL_RIGHT, event.key.keysym.sym);
      // event.key.keysym.sym
      break;
    case PL2J:
      st += "Jump: ";
      cga->printXY(screen, &rect, st.c_str());
      if ( ( event.type != SDL_KEYDOWN ) || isStillPressed )
	return(0);
      controlsArray->setControl(2*_base + 1, CNTRL_JUMP, event.key.keysym.sym);
      // event.key.keysym.sym
      break;
    }

    isPressed = ( event.type == SDL_KEYDOWN );

    if ( state == PL2J ) {
      s.pop();
      state = 0;
    } else
      state++;

    return(NO_TRANSITION);
  }
};

#endif

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
#include "AutomaMainLoop.h"
#include "InputState.h"
#include "globals.h"
#include "StateMenu.h"
#include "StatePlaying.h"
#include "MenuItemFullScreen.h"
#ifndef NONET
#include "StateClient.h"
#endif

AutomaMainLoop::AutomaMainLoop()
{
  _is = new InputState();

  StateMenu *sm = new StateMenu();
  addState(STATE_MENU, sm);
  _curr = STATE_MENU;
  StatePlaying *sp = new StatePlaying();
  addState(STATE_PLAYING, sp);
#ifndef NONET
  StateClient *sc = new StateClient();
  addState(STATE_CLIENT, sc);
#endif
  _prev = -1;
}

int AutomaMainLoop::transition(int retval)
{
  if ( retval == NO_TRANSITION )
    return(_curr);

#if 0
  switch ( _curr ) {
  case STATE_MENU:
    return STATE_PLAYING;
    break;
  case STATE_PLAYING:
    return STATE_MENU;
    break;
#ifndef NONET
  case STATE_CLIENT:
    return STATE_MENU;
    break;
#endif
  }
#endif
  return retval; // _curr;
}

int AutomaMainLoop::start()
{
  unsigned int prevTicks = 0;
  // unsigned int prevDrawn = 0;
  unsigned int frames = 0;
  unsigned int milliseconds;
  
  while ( 1 ) {
    int ticks = SDL_GetTicks();
    
    if ( prevTicks == 0 ) {
      prevTicks = ticks;
      continue;
    }
    
    frames++;
    milliseconds += ticks - prevTicks;
    if ( milliseconds >= 1000 )
      frames = milliseconds = 0;
    
    _is->getInput();
    if ( _is->getF()[9] ) {
      std::stack<Menu *> s;
      MenuItemFullScreen().execute(s);
    }

    // execute the _curr state's code, and transact
    int retval  = _states[_curr]->execute(_is,
					  ticks, prevTicks,
					  (_prev != _curr));
    _prev = _curr;
    _curr = transition(retval);

    if ( (ticks - prevTicks) < configuration.mill_per_frame )
      SDL_Delay(configuration.mill_per_frame - (ticks - prevTicks));
    
    prevTicks = ticks;
  }

  return(0);
}

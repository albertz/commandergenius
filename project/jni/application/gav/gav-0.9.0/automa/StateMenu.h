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

#ifndef __STATEMENU_H__
#define __STATEMENU_H__

#include <iostream>
#include "globals.h"
#include "State.h"
#include "Automa.h"
#include "MenuRoot.h"
#include "Menu.h"
#include "MenuItem.h"
#include "MenuItemSubMenu.h"
#include "MenuItemPlay.h"
#include "MenuItemExit.h"
#include "MenuItemNotImplemented.h"
#include "SoundMgr.h"

class StateMenu : public State {
  private:
  MenuRoot *_mr;

 public:
  StateMenu() {
    _mr = mroot;
  };

  int execute(InputState *is, unsigned int ticks, unsigned int prevTicks,
	      int firstTime) {
    static int lastExec = ticks;
#ifdef AUDIO
    if ( firstTime )
      soundMgr->playSound(SND_BACKGROUND_MENU, true);
#endif // AUDIO

    if ( (ticks - lastExec) > 50 ) {
      SDL_Rect r;
      r.x = r.y = 0;
      r.h = background->height();
      r.w = background->width();
      background->blit(0, screen, &r);
      //SDL_BlitSurface(background, &r, screen, &r);
      int ret = _mr->execute(is);
      SDL_Flip(screen);
      lastExec = ticks;
      return(ret);
    }

    if ( is->getF()[0] ) {
      return(NO_TRANSITION + 1);
    }
    return(NO_TRANSITION);
  }
};

#endif // __STATEMENU_H__

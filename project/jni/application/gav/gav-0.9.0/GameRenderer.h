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

#ifndef _GAMERENDERER_H_
#define _GAMERENDERER_H_

#include <SDL.h>
#include "FrameSeq.h"
#include "globals.h"

class GameRenderer {
  float _ratioX;
  float _ratioY;

 public:
  GameRenderer() {
    _ratioX = 1.0;
    _ratioY = 1.0;
  }

  GameRenderer(int aw, int ah, int lw, int lh) {
    _ratioX = (float) aw / (float) lw;
    _ratioY = (float) ah / (float) lh;
  }
    
  // if the frame param is omitted, the whole surface is blitted to dest
  void display(SDL_Surface *dest, SDL_Rect *rect, FrameSeq *what,
	       int frame = 0);

  SDL_Rect convertCoordinates(SDL_Rect *rect);
};

#endif // _GAMERENDERER_H_


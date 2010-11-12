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

#include <string.h>
#include "globals.h"
#include "GameRenderer.h"
#include "ScreenFont.h"

/* Converts to SCREEN coordinates! Should me moved into GameRenderer or something! */
void ScreenFont::printXY(SDL_Surface *dest, SDL_Rect *rect, const char * str,
			 bool wrapAround, FrameSeq *bg)
{
  SDL_Rect r;
  
  int _charWid = _frames->screenWidth();

  r = gameRenderer->convertCoordinates(rect);
  r.h = _frames->getActualFrameSeq()->height();
  r.w = _frames->getActualFrameSeq()->width();

  const char *run = str;
  while ( *run ) {

    if ( wrapAround )
      //r.x = (r.x + dest->w) % dest->w;
      r.x = (r.x + configuration.resolution.x) % configuration.resolution.x;
    
    if ( ((*run) >= _fst) && ((*run) < (_fst + _nchars)) ) {
      if ( bg )
	bg->getActualFrameSeq()->blitRect(0, dest, &r);
      _frames->getActualFrameSeq()->blit((int) ((*run) - _fst), dest, &r);
    }
    run++;
    r.x += _charWid;
  }
}

void ScreenFont::printRow(SDL_Surface *dest, int row, const char *str,
			  FrameSeq *bg)
{
  SDL_Rect rect;
  /* draw menu items labels */
  rect.y = configuration.CEILING + configuration.env.h/100 +
    row * charHeight();

  rect.x = (configuration.env.w / 2) - strlen(str)*(charWidth())/2;
  printXY(dest, &rect, str, false, bg);
}

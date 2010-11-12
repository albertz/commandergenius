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

#ifndef __LOGICALFRAMESEQ_H__
#define __LOGICALFRAMESEQ_H__

#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include <math.h>
//#include <SDL_rotozoom.h>
#include "ResizeSurface.h"
#include "globals.h"
#include "FrameSeq.h"

double exactScaleFactor(int startw, double zoom, int step);

class LogicalFrameSeq : public FrameSeq {
 protected:
  FrameSeq *_actualFrameSeq;

public:
  LogicalFrameSeq(const char * filename, int nframes, bool useAlpha = true)
    : FrameSeq(filename, nframes, useAlpha) {

    // if ( ratioX == 0.0 )
      float ratioX = ::configuration.resolution.ratioX;
      // if ( ratioY == 0.0 )
      float ratioY = ::configuration.resolution.ratioY;
    
    ratioX = exactScaleFactor(_surface->w, ratioX, nframes);

    // SDL_Surface *as = zoomSurface(_surface, ratioX, ratioY, SMOOTHING_OFF);
    SDL_Surface *as = resizeSurface(_surface, (int) round(ratioX*_surface->w),
				    (int) round(ratioY*_surface->h));

    //printf("NS->w: %d\n", as->w);
    _actualFrameSeq = new FrameSeq(as, nframes, useAlpha);

    /* Modify _width according to the new computed ratioX */
    _width = (int) (((double) _actualFrameSeq->width()) / ratioX);
  }
  
  virtual ~LogicalFrameSeq() {
    delete(_actualFrameSeq);
  }

  virtual SDL_Surface *surface() { return _actualFrameSeq->surface(); }

  virtual void blit(int idx, SDL_Surface * dest, SDL_Rect * rect);
  
  /* These two functions are needed when it is required to reason in terms of actual
     pixels (as in ScreenFonts to make sure the font is nicely laid out. In order
     to be able to exchange between FrameSeq and LogicalFrameSeq, however, it has been
     necessary to add these to FrameSeq as well. Should have worked that out better... */

  virtual FrameSeq *getActualFrameSeq() { return _actualFrameSeq; }
  virtual int screenWidth() { return _actualFrameSeq->width(); }
};

#endif // __LOGICALFRAMESEQ_H__

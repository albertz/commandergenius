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

#ifndef __FRAMESEQ_H__
#define __FRAMESEQ_H__

#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>

extern SDL_Surface *screen;

class FrameSeq {  
protected:
  SDL_Surface * _surface;
  int _nframes;
  int _width;
  int _height;

  void setSurfaceAndFrames(SDL_Surface *sfc, int nframes, bool useAlpha) {
    _surface = SDL_DisplayFormat(sfc);
    if ( useAlpha )
      SDL_SetColorKey(_surface, SDL_SRCCOLORKEY,
		      (Uint32) SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_FreeSurface(sfc);
    _nframes = nframes;
    _width = _surface->w / _nframes;
    _height = _surface->h;
  }

public:
  FrameSeq (const char * filename, int nframes, bool useAlpha) {
    SDL_Surface * temp;

    if ((temp = IMG_Load(filename)) == NULL) {
      fprintf(stderr, "FrameSeq: cannot load %s\n", filename);
      exit(-1);
    }
    setSurfaceAndFrames(temp, nframes, useAlpha);
  }

  FrameSeq(SDL_Surface *sfc, int nframes, bool useAlpha) {
    setSurfaceAndFrames(sfc, nframes, useAlpha);
  }
  
  virtual ~FrameSeq() {
    SDL_FreeSurface(_surface);
  }
  
  virtual SDL_Surface *surface() { return _surface; }

  inline int width() { return _width; }
  inline int height() { return _height; }

  /* Blits an entire frame to screen */
  virtual void blit(int idx, SDL_Surface * dest, SDL_Rect * rect);

  /* Blits part of a frame to screen */
  void blitRect(int idx, SDL_Surface * dest, SDL_Rect * rect);
  bool collidesWith(FrameSeq *fs, int idx1, int idx2, SDL_Rect * rect1,
		    SDL_Rect * rect2);

  virtual FrameSeq *getActualFrameSeq() { return this; } // bit ugly
  virtual int screenWidth() { return _width; }  // bit ugly
  
};

#endif

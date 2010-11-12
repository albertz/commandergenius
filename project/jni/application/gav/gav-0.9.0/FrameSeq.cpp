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

#include <iostream>
#include "FrameSeq.h"

#define max(x, y)  ((x)>(y)?(x):(y))

#define min(x, y)  ((x)<(y)?(x):(y))

void FrameSeq::blitRect(int idx, SDL_Surface * dest, SDL_Rect * rect) {
  SDL_Rect r;

  r.x = (idx % _nframes) * _width;
  r.y = 0;
  r.w = rect->w;
  r.h = rect->h;
  SDL_BlitSurface(_surface, &r, dest, rect);
}

void FrameSeq::blit(int idx, SDL_Surface * dest, SDL_Rect * rect) {
  SDL_Rect r;

  r.x = (idx % _nframes) * _width;
  r.y = 0;
  r.w = _width;
  r.h = _surface->h;
  SDL_BlitSurface(_surface, &r, dest, rect);
}

Uint32 CreateHicolorPixel(SDL_PixelFormat *fmt, Uint8 red,
			  Uint8 green, Uint8 blue) {
  return( ((red >> fmt->Rloss) << fmt->Rshift) +
	  ((green >> fmt->Gloss) << fmt->Gshift) +
	  ((blue >> fmt->Bloss) << fmt->Bshift) );
}

inline Uint32 getPix(void *v, int i, Uint8 bpp) {

  switch ( bpp ) {
  case 4:  return (Uint32) ((Uint32 *)v)[i]; break;
  case 3:  
    std::cerr << "Unsupported pixel format: please, report to the GAV team!\n";
    exit(0);
    break;
    //(Uint32) ((Uint24 *)v)[i]; break; there's no such thing as Uint24!
  case 2:  return (Uint32) ((Uint16 *)v)[i]; break;
  case 1:  return (Uint32) ((Uint8 *)v)[i]; break;
  }

  std::cerr << "Unsupported pixel format (" << bpp <<
    "): please, report to the GAV team!\n";
  exit(0);

  return 0;
}

bool
FrameSeq::collidesWith(FrameSeq *fs, int idx1, int idx2, SDL_Rect * rect1,
		       SDL_Rect * rect2) {
  int xmin = max(rect1->x, rect2->x);
  int xmax = min(rect1->x + _width, rect2->x + fs->_width);
  int ymin = max(rect1->y, rect2->y);
  int ymax = min(rect1->y + _height, rect2->y + fs->_height);
  
  if ( (xmin > xmax) || (ymin > ymax) ) return(false);

  void *pix1 = _surface->pixels;
  void *pix2 = fs->_surface->pixels;
  
  Uint8 pixfmt = screen->format->BytesPerPixel;
  Uint32 empty_pixel = 0;
  // faster than CreateHicolorPixel(screen->format, 0, 0, 0);

  int sp1 = _surface->pitch / pixfmt;
  int sp2 = fs->_surface->pitch / pixfmt;
  int xdisp1 = ((idx1 % _nframes) * _width);
  int xdisp2 = ((idx2 % fs->_nframes) * fs->_width);
		
  while ( ymin < ymax ) { // was ymin <= ymax
    int xrun = xmin;
    while ( xrun < xmax ) { // was xrun <= xmax
      int p1off = sp1 * (ymin - rect1->y) + xdisp1 + (xrun - rect1->x);
      int p2off = sp2 * (ymin - rect2->y) + xdisp2 + (xrun - rect2->x);
      if ( ( getPix(pix1, p1off, pixfmt) != empty_pixel ) &&
	   ( getPix(pix2, p2off, pixfmt) != empty_pixel) ) {
	return(true);
      }
      xrun++;
    }
    ymin++;
  }

  return(false);
}



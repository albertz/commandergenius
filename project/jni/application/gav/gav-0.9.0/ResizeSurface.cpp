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


/*
 *  These functions are derived from SDL_buffer.
 */

#include <stdlib.h>
#include "ResizeSurface.h"

typedef struct tColorRGBA {
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
} tColorRGBA;


/*
 * 32bit Zoomer with optional anti-aliasing by bilinear interpolation.
 * Zoomes 32bit RGBA/ABGR 'src' surface to 'dst' surface.
 * Forked from SDL_rotozoom.c, LGPL (c) A. Schiffler
*/
static int zoomSurfaceRGBA (SDL_Surface * src, SDL_Surface * dst)
{
    int x, y,			//current dst->x and dst->y
	    xscale, yscale,	//src/dst proportions * 65536
		*sx_offset, *sy_offset,		//row/columnt increment tables
		*csx_offset, *csy_offset,	//pointers to current row/columnt increment element
		csx, csy,
		sgap, dgap;

	tColorRGBA *sp, *csp, *dp;


    // Variables setup

	xscale = (int) (65536.0 * (double) src->w / (double) dst->w);
	yscale = (int) (65536.0 * (double) src->h / (double) dst->h);


	// Allocate memory for row increments
	sx_offset = (int*)malloc((dst->w + 1) * sizeof(Uint32));
	sy_offset = (int*)malloc((dst->h + 1) * sizeof(Uint32));

    if (sx_offset == NULL || sy_offset == NULL) {
		free(sx_offset);
		free(sy_offset);
		return -1;
    }

	// Precalculate row increments
    csx = 0;
    csx_offset = sx_offset;
    for (x = 0; x <= dst->w; x++) {
		*csx_offset = csx;
		csx_offset++;
		csx &= 0xffff;
		csx += xscale;
    }

	csy = 0;
    csy_offset = sy_offset;
    for (y = 0; y <= dst->h; y++) {
		*csy_offset = csy;
		csy_offset++;
		csy &= 0xffff;
		csy += yscale;
    }


    // Pointer setup
    sp = csp = (tColorRGBA *) src->pixels;
    dp = (tColorRGBA *) dst->pixels;
    sgap = src->pitch - src->w * 4;
    dgap = dst->pitch - dst->w * 4;

    // Switch between interpolating and non-interpolating code


    // Non-Interpolating Zoom
    
    csy_offset = sy_offset;
    for (y = 0; y < dst->h; y++) {
      sp = csp;
      csx_offset = sx_offset;
      
      for (x = 0; x < dst->w; x++) {
	// Draw
	*dp = *sp;
	
	// Advance source pointers
	csx_offset++;
	sp += (*csx_offset >> 16);
	
	// Advance destination pointer
	dp++;
      }
      
      // Advance source pointer
      csy_offset++;
      csp = (tColorRGBA *) ((Uint8 *) csp + (*csy_offset >> 16) * src->pitch);
      
      // Advance destination pointers
      dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
    }
    

    // Remove temp arrays
    free(sx_offset);
    free(sy_offset);

    return 0;
}


SDL_Surface * resizeSurface (SDL_Surface * buf, int width, int height) {
  SDL_PixelFormat * format = buf->format;
  
  int dst_w, dst_h;
  
  Uint32 Rmask,Gmask,Bmask,Amask;
  SDL_Surface * dest;
  SDL_Surface * tmp;
  bool toFree = false;
  
  if (format->BitsPerPixel == 32) {
    Rmask = format->Rmask;
    Gmask = format->Gmask;
    Bmask = format->Bmask;
    Amask = format->Amask;
  } else {
    Rmask = 0x000000ff;
    Gmask = 0x0000ff00;
    Bmask = 0x00ff0000;
    Amask = 0xff000000;
  }
  
  if (format->BitsPerPixel != 32 ||
      format->Rmask != Rmask ||
      format->Gmask != Gmask ||
      format->Bmask != Bmask) {
    // New source surface is 32bit with defined RGBA ordering.
    // Note that Amask has been ignored in test
    
    tmp = SDL_CreateRGBSurface (SDL_SWSURFACE, buf->w, buf->h, 32, 
				Rmask, Gmask, Bmask, Amask);
    SDL_BlitSurface (buf, NULL, tmp, NULL);
    toFree = true;
  } else
    tmp = buf;
  
  dst_w = width;
  dst_h = height;
  
  // Alloc space to completely contain the zoomed surface
  // Target surface is 32bit with source RGBA/ABGR ordering
  // (note that buf->zoom is already NULL)
  dest = SDL_CreateRGBSurface (SDL_SWSURFACE, width, height, 32, 
			       Rmask, Gmask, Bmask, Amask);
  
  
  SDL_LockSurface(tmp);
  SDL_LockSurface(dest);
  
  zoomSurfaceRGBA(tmp, dest);
  
  // Turn on source-alpha support
  SDL_SetAlpha(dest, SDL_SRCALPHA, 255);
  
  SDL_UnlockSurface(tmp);
  SDL_UnlockSurface(dest);
  
  if (toFree) SDL_FreeSurface(tmp);
  
  // Return destination surface
  return dest;
}


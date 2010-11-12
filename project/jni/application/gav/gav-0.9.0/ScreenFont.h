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

#ifndef _SCREENFONT_H_
#define _SCREENFONT_H_

#include <SDL.h>
#include "LogicalFrameSeq.h"

#define FONT_FIRST_CHAR ' '
#define FONT_NUMBER 104

class FrameSeq;

class ScreenFont {
private:
  FrameSeq *_frames;
  char _fst; // first character
  unsigned char _nchars;

public:
  ScreenFont(const char *fname, char fst, unsigned char n) :
    _fst(fst), _nchars(n) {
    _frames = new LogicalFrameSeq(fname, (int) n);
  }

  ~ScreenFont() {
    delete(_frames);
  }

  void printXY(SDL_Surface *dest, SDL_Rect *r, const char * str,
	       bool wrapAround = true, FrameSeq *background = NULL);
  void printRow(SDL_Surface *dest, int row, const char *str,
	        FrameSeq *bg = NULL);
  inline int charWidth() { return(_frames->width()); }
  inline int charHeight() { return(_frames->height()); }
};

#endif // _SCREENFONT_H_

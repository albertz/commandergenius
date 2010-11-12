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

#ifndef _AUTOMA_H_
#define _AUTOMA_H_

#include <vector>
#include "State.h"

#define NO_TRANSITION (0)

class Automa {
protected:
  State *_states[100]; // max 100 states
  int _curr; // current state index
  int _prev;
  int _size;

public:
  Automa() {
    _size = 0;
    for ( int i = 0; i < 100; _states[i++] = NULL );
  };
  virtual int addState(int idx, State *state) {
    _states[idx] = state;
    return(_size++);
  }
  virtual int start() { return(0); }
  virtual ~Automa() {
    for ( int i = 0; i < 100; i++ )
      if ( _states[i] )
	delete(_states[i]);
  }
};

#endif // _AUTOMA_H_

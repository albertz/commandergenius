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

#ifndef _STATE_H_
#define _STATE_H_

#include<string>
#include<iostream>
#include "InputState.h"

class State {

  
public:
  std::string _name; // for debug
  State() {};
  inline void setName(char *name) { _name = std::string(name); }
  inline std::string getName() { return(_name); }
  virtual int execute(InputState *is, unsigned int ticks,
		      unsigned int prevticks, int firstTime) {
    std::cout << "unextended state: " << _name << std::endl;
    return(0); // NO_TRANSITION
  }; 
  virtual ~State() {};
};

#endif // _STATE_H_

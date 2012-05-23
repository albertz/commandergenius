/*************************************************************************** 
  alienBlaster 
  Copyright (C) 2004 
  Paul Grathwohl, Arne Hormann, Daniel Kuehn, Soenke Schwardt

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
***************************************************************************/
#ifndef INPUT_H
#define INPUT_H

#include "SDL.h"
#include "SdlForwardCompat.h"

class Input;

extern Input input;

class Input {

private:
  Uint16 currentJoystick;
  SDL_Joystick *joystick;
  
public:
  Input();
  ~Input();
  void getJoystick();
  void freeJoystick();
  const SDLKey translate(const SDL_Event& event);
  const bool isPressed(const SDL_Event& event);

  static const Input input();
};

#endif //#ifndef INPUT_H

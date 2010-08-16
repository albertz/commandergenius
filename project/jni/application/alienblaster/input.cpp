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
using namespace std;

#include <vector>
#include "input.h"
#include "SDL.h"
#include "settings.h"

Input input;

const int initialSensitivity = 4096;

Input::Input(){
  joystick = 0;
  getJoystick();
}

Input::~Input(){
  freeJoystick();
  joystick = 0;
}

void Input::getJoystick() {
  if (joystick) return;
  if (!SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
    SDL_JoystickEventState(SDL_ENABLE);
    if (SDL_NumJoysticks() > 0) {
      joystick = SDL_JoystickOpen(0);
    } else {
      freeJoystick();
    }    
  }
}    

void Input::freeJoystick() { 
  if (joystick) {
    SDL_JoystickClose(joystick);
    joystick = 0;
  }    
  SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

#ifdef DEBUG
const bool pressed(const SDL_Event& event) {
  if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_KEYDOWN) return true;
  if (event.type == SDL_JOYAXISMOTION) return event.jaxis.value / 3200;
  return false;
}

void debugEvent(const SDL_Event& event, const SDLKey code) {
    if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP) {
    printf("Joystick button%d event: %s (%d)\n",
      event.jbutton.button, (pressed(event)) ? "pressed" : "released", code);
  } else if (event.type == SDL_JOYAXISMOTION && pressed(event)) {
    printf("Joystick axis%d event: %s (%d)\n",
      event.jaxis.axis, (event.jaxis.value > 0) ? "raised" : "lowered", code);
  }
}

const SDLKey translateEvent(const SDL_Event& event) {
  if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
    return event.key.keysym.sym;
  }
  if (event.type == SDL_JOYAXISMOTION) {
    return (SDLKey) (1024 + event.jaxis.axis * 2 + ((event.jaxis.value > 0) ? 1 : 0));
  }
  if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP) {
    return (SDLKey) (1040 + event.jbutton.button);
  }
  return (SDLKey) 0;
}
#endif

const SDLKey Input::translate(const SDL_Event& event) {
#ifdef DEBUG
  debugEvent(event, translateEvent(event));
#endif
  if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
    return event.key.keysym.sym;
  }    
  if (event.type == SDL_JOYAXISMOTION) {
    return (SDLKey) (1024 + event.jaxis.axis * 2 + ((event.jaxis.value > 0) ? 1 : 0));
  }
  if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP) {
    return (SDLKey) (1040 + event.jbutton.button);
  }    
  return (SDLKey) 0;
}    

const bool Input::isPressed(const SDL_Event& event) {
  if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN) return true;
  if (event.type == SDL_JOYAXISMOTION) return event.jaxis.value / initialSensitivity;
  return false;
}

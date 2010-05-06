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
#ifndef SHOTS_H
#define SHOTS_H

#include <vector>
#include "SDL.h"
#include "geometry.h"


class Shot;

/* manages the shot-objects */
class Shots {
  vector<Shot *> shots;

  public:
  Shots();
  ~Shots();
  
  void addShot(Shot *shot);
  // deletes shots, that timed out or hit
  void expireShots();
  // deletes all shots (if a nuke is used, this will happen)
  void deleteAllShots();
  
  // moves the shots and checks, whether a shot hit a racer or an enemy
  void moveAndCollide( int dT );

  // draw the shots
  void drawShadows(SDL_Surface *screen);
  void drawAirShots(SDL_Surface *screen);
  void drawGroundShots(SDL_Surface *screen);
  void drawGroundAirShots(SDL_Surface *screen);
  
  Shot* getNearestRocket(Vector2D position);
  bool existsRocket();
};

#endif

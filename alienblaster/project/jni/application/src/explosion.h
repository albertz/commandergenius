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
#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "SDL.h"
#include "geometry.h"
#include <string>
#include "global.h"

class Explosion {

  // a sprite, that contains horizontally all animationframes of the explosion.
  // it is assumed, that every frame is quadratic.
  SdlCompat_AcceleratedSurface *sprite;

  // how many frames does this explosion have?
  int nrAnimStages;
  // which frame is now?
  int actAnimStage;
  // how long should one frame last (ms)
  int timePerStage;
  // how long is the current explosion already active
  int timeLived;
  // at what timeLived starts the next frame?
  int timeNextAnimStage;

  // the explosion can be deleted
  bool expired;

  int sndExplosion;

  Vector2D pos;
  Vector2D vel; // the explosion moves - yeah
  ExplosionTypes explosionType;

  public:
  Explosion(string fn, const Vector2D &position, 
	    const Vector2D &velocity, const ExplosionTypes &explosionType);
  ~Explosion();
  // updates the position and the counters
  void update( int dT );
  void drawAirExplosion(SdlCompat_AcceleratedSurface *screen);
  void drawGroundExplosion(SdlCompat_AcceleratedSurface *screen);
  bool isExpired() { return expired; }
};

#endif

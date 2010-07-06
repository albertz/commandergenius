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
#ifndef WRECK_H
#define WRECK_H

#include "SDL.h"
#include "geometry.h"
#include <string>
#include "global.h"

class Wreck {
  SdlCompat_AcceleratedSurface *sprite;

  Vector2D pos;
  Vector2D vel;
  WreckTypes wreckType;

  public:
  Wreck( Vector2D position, WreckTypes wreckType );
  ~Wreck();
  void update( int dT );
  void draw(SdlCompat_AcceleratedSurface *screen);
  bool isExpired();
  void deleteWreck();

  Vector2D getPos() { return pos; }
  Vector2D getVel() { return vel; }
  WreckTypes getType() { return wreckType; }
};

#endif

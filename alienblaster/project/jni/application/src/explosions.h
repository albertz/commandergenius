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
#ifndef EXPLOSIONS_H
#define EXPLOSIONS_H

#include <vector>
#include "SDL.h"
#include "SdlForwardCompat.h"

class Explosion;

/* manages the explosion-objects */
class Explosions {
  vector<Explosion *> explosions;
  
  public:
  Explosions();
  ~Explosions();
  
  inline unsigned int getNrExplosions() { return explosions.size(); }

  void addExplosion( Explosion *explosion );
  // moves the explosions and updates the counters
  void updateExplosions( int dT );
  // deletes the explosions, that have timed out
  void expireExplosions();
  // draws all explosions
  void drawAirExplosions( SdlCompat_AcceleratedSurface *screen );
  void drawGroundExplosions( SdlCompat_AcceleratedSurface *screen );
};

#endif

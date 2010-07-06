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
#ifndef SHIELD_GLOW_H
#define SHIELD_GLOW_H

#include "SDL.h"
#include "geometry.h"
#include "global.h"

class ShieldGlow {
  SdlCompat_AcceleratedSurface *spriteShieldGlow;
  int nrFrames;
  int frameWidth;
  int halfFrameWidth;
  int halfFrameHeight;
  int timePerFrame;

  public:
  ShieldGlow( ShipTypes shipType );
  ~ShieldGlow();

  void draw( SdlCompat_AcceleratedSurface *screen, Vector2D drawPos, int time );
  
};

#endif

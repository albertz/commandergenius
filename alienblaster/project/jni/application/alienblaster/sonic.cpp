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
#include <iostream>
#include <math.h>
#include "sonic.h"
#include "global.h"
#include "surfaceDB.h"
#include "geometry.h"
#include "video.h"

using namespace std;

Sonic::Sonic() {
  sonicBall = surfaceDB.loadSurface( FN_SONIC_EFFECT, true );
  timeStage = 0;
  active = false;
  waveLength = 6.0f;
}

void Sonic::setPos( Vector2D pos1, Vector2D pos2 ) {
  this->pos1 = pos1;
  this->pos2 = pos2;
  active = true;
}

void Sonic::drawAtPos( SdlCompat_AcceleratedSurface *screen, Vector2D pos1, Vector2D pos2 ) {
  setPos( pos1, pos2 );
  draw( screen );
}

void Sonic::draw( SdlCompat_AcceleratedSurface *screen ) {
  if ( !active ) return;
  timeStage += 2;
  timeStage = (timeStage % (int)(waveLength + 0.5f)) + 1;
  SDL_Rect rect;
  rect.w = sonicBall->w;
  rect.h = sonicBall->h;
  Vector2D sonicIncrement = pos2 - pos1;
  float dist = sonicIncrement.getLength() - waveLength;
  sonicIncrement.setLength((float) timeStage);
  Vector2D sonicActual = pos1;
  sonicActual += sonicIncrement;
  sonicIncrement.setLength(waveLength);
  
  while (dist >= 5) {
    sonicActual += sonicIncrement;
    dist -= waveLength;
    rect.x = (int) sonicActual.getX() - sonicBall->w / 2;
    rect.y = (int) sonicActual.getY() - sonicBall->h / 2;
    SDL_BlitSurface( sonicBall, 0, screen, &rect );
  }
}

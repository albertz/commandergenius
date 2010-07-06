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

#include "smokePuff.h"
#include "surfaceDB.h"
#include "global.h"

SmokePuff::SmokePuff( Vector2D position, Vector2D velocity, SmokePuffTypes whichType ) {
  
  smokePuffType = whichType;
  pos = position;
  vel = velocity;

  sprite = surfaceDB.loadSurface( FN_SMOKE_PUFF[ smokePuffType ],
				  SMOKE_PUFF_ALPHA_BLENDING );

  expired = false;
  nrAnimStages = sprite->w / sprite->h;
  timePerStage = LIFETIME_SMOKE_PUFF[ smokePuffType ] / nrAnimStages;
  actAnimStage = 0;
  timeLived = 0;
  timeNextAnimStage = timePerStage;
}

SmokePuff::~SmokePuff() {}

void SmokePuff::update( int dT ) {
  pos += vel * dT / 1000.0;
  timeLived += dT;
  if ( timeLived > timeNextAnimStage ) {
    timeNextAnimStage += timePerStage;
    actAnimStage++;
    if (actAnimStage == nrAnimStages) expired = true;
  }
}

void SmokePuff::drawSmokePuff( SdlCompat_AcceleratedSurface *screen ) {
  if (expired) return;

  SDL_Rect dest;
  dest.x = lroundf(pos.getX()) - sprite->w / (2*nrAnimStages);
  dest.y = lroundf(pos.getY()) - sprite->h / 2;
  dest.w = sprite->w / nrAnimStages;
  dest.h = sprite->h;

  SDL_Rect src;
  src.x = actAnimStage * sprite->w / nrAnimStages;
  src.y = 0;
  src.w = sprite->w / nrAnimStages;
  src.h = sprite->h;

  SDL_BlitSurface( sprite, &src, screen, &dest );
}

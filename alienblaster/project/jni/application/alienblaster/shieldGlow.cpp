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
#include "shieldGlow.h"
#include "surfaceDB.h"

ShieldGlow::ShieldGlow( ShipTypes shipType ) {
  switch ( shipType ) {
  case LIGHT_FIGHTER: 
    {
      spriteShieldGlow = surfaceDB.loadSurface( FN_LIGHT_FIGHTER_SHIELD_DAMAGED, true );
      break;
    }
  case HEAVY_FIGHTER:
    {
      spriteShieldGlow = surfaceDB.loadSurface( FN_HEAVY_FIGHTER_SHIELD_DAMAGED, true );
      break;
    }
  }
  nrFrames = spriteShieldGlow->w / spriteShieldGlow->h;
  timePerFrame = RACER_SHIELD_DAMAGE_LIFETIME / nrFrames;
  frameWidth = spriteShieldGlow->w / nrFrames;
  halfFrameWidth = frameWidth / 2;
  halfFrameHeight = spriteShieldGlow->h / 2;
}

ShieldGlow::~ShieldGlow() {}

void ShieldGlow::draw( SdlCompat_AcceleratedSurface *screen, Vector2D pos, int time ) {
  if ( time < 0 || RACER_SHIELD_DAMAGE_LIFETIME < time ) return;
  
  int actFrame = time / timePerFrame;

  SDL_Rect src, dest;
  src.x = actFrame*frameWidth;
  src.y = 0;
  src.w = frameWidth;
  src.h = spriteShieldGlow->h;
  dest.x = lroundf(pos.getX()) - halfFrameWidth;
  dest.y = lroundf(pos.getY()) - halfFrameHeight;
  dest.w = frameWidth;
  dest.h = spriteShieldGlow->h;

  SDL_BlitSurface( spriteShieldGlow, &src, screen, &dest );
}


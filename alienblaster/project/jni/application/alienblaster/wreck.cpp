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
#include "wreck.h"
#include "surfaceDB.h"
#include "global.h"
#include "options.h"

Wreck::Wreck(Vector2D position, WreckTypes wreckType) {

  this->wreckType = wreckType;

  pos = position;
  vel = Vector2D(0, SCROLL_SPEED);

  switch (wreckType) {
  case WRECK_FIGHTER:
    {
      string fn = LVL_WRECK_FIGHTER;
      levelConf->getStr( LVL_WRECK_FIGHTER, fn);
      sprite = surfaceDB.loadSurface( fn );
      break;
    }
  case WRECK_BOMBER:
    {
      string fn = LVL_WRECK_BOMBER;
      levelConf->getStr( LVL_WRECK_BOMBER, fn);
      sprite = surfaceDB.loadSurface( fn );
      break;
    }
  case WRECK_TANK:
    {
      string fn = LVL_WRECK_TANK;
      levelConf->getStr( LVL_WRECK_TANK, fn);
      sprite = surfaceDB.loadSurface( fn );
      break;
    }
  case WRECK_BOSS_1_BACKGROUND:
    {
      string fn = LVL_WRECK_BOSS_BACKGROUND;
      levelConf->getStr( LVL_WRECK_BOSS_BACKGROUND, fn);
      sprite = surfaceDB.loadSurface( fn );
      break;
    }
  case WRECK_BOSS_1_DESTROYED:
    {
      string fn = LVL_WRECK_BOSS_DESTROYED;
      levelConf->getStr( LVL_WRECK_BOSS_DESTROYED, fn);
      sprite = surfaceDB.loadSurface( fn );
      break;
    }
  case WRECK_BOSS_2_DESTROYED:
    {
      sprite = surfaceDB.loadSurface( FN_WRECK_BOSS_2_DESTROYED );
      float correlateToBackground = actBackgroundPos - truncf(actBackgroundPos);
      pos = Vector2D( position.getX(), truncf(position.getY()) - correlateToBackground );
      break;
    }
  default:
    {
      string fn = LVL_WRECK_FIGHTER;
      levelConf->getStr( LVL_WRECK_FIGHTER, fn);
      sprite = surfaceDB.loadSurface( fn );
      break;
    }
  }
}


Wreck::~Wreck() {
}


void Wreck::update( int dT ) {
  if ( scrollingOn ) {
    pos += vel * dT / 1000.0;
  }
}


void Wreck::draw(SdlCompat_AcceleratedSurface *screen) {
  SDL_Rect r;
  r.x = lroundf(pos.getX()) - sprite->w / 2;
  r.y = lroundf(pos.getY()) - sprite->h / 2;
  r.w = sprite->w;
  r.h = sprite->h;
  SDL_BlitSurface( sprite, 0, screen, &r );
}


bool Wreck::isExpired() {
  return ( pos.getY() > SCREEN_HEIGHT );
}


void Wreck::deleteWreck() {
  pos.setY( SCREEN_HEIGHT + 500 );
}

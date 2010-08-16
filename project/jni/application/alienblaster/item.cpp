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
#include "item.h"
#include "boundingBox.h"
#include "surfaceDB.h"
#include "global.h"

Item::Item(const Vector2D &position, const Vector2D &velocity, ItemTypes itemType) {

  this->itemType = itemType;

  pos = position;
  vel = velocity;
  timeLived = 0;

  switch (itemType) {
  case ITEM_PRIMARY_UPGRADE:
    {
      sprite = surfaceDB.loadSurface( FN_ITEM_PRIMARY_UPGRADE );
      break;
    }
  case ITEM_DUMBFIRE_DOUBLE:
    {
      sprite = surfaceDB.loadSurface( FN_ITEM_DUMBFIRE_DOUBLE );
      break;
    }
  case ITEM_KICK_ASS_ROCKET:
    {
      sprite = surfaceDB.loadSurface( FN_ITEM_KICK_ASS_ROCKET );
      break;
    }
  case ITEM_HELLFIRE:
    {
      sprite = surfaceDB.loadSurface( FN_ITEM_HELLFIRE );
      break;
    }
  case ITEM_MACHINE_GUN:
    {
      sprite = surfaceDB.loadSurface( FN_ITEM_MACHINE_GUN );
      break;
    }
  case ITEM_HEALTH:
    {
      sprite = surfaceDB.loadSurface( FN_ITEM_HEALTH );
      break;
    }
  case ITEM_HEATSEEKER:
    {
      sprite = surfaceDB.loadSurface( FN_ITEM_HEATSEEKER );
      break;
    }
  case ITEM_NUKE:
    {
      sprite = surfaceDB.loadSurface( FN_ITEM_NUKE );
      break;
    }
  case ITEM_DEFLECTOR:
    {
      sprite = surfaceDB.loadSurface( FN_ITEM_DEFLECTOR );
      break;
    }
  case ITEM_LASER:
    {
      sprite = surfaceDB.loadSurface( FN_ITEM_LASER );
      break;
    }
  case ITEM_ENERGY_BEAM:
    {
      sprite = surfaceDB.loadSurface( FN_ITEM_ENERGY_BEAM );
      break;
    }
      
  default:
    {
      cout << "Item(): unexpected itemType: " << itemType << endl;
      sprite = surfaceDB.loadSurface( FN_ITEM_PRIMARY_UPGRADE );
      break;
    }
  }

  boundingBox = new BoundingBox( lroundf(pos.getX() - sprite->w / 2.0),
				 lroundf(pos.getY() - sprite->h / 2.0),
				 sprite->w, sprite->h );
}

Item::~Item() {
  delete boundingBox;
}


void Item::update( int dT ) {
  pos += vel * dT / 1000.0;
  updateBoundingBox();
  timeLived += dT;
}

void Item::deleteItem() {
  timeLived = ITEM_LIFETIME;
}

void Item::updateBoundingBox() {
  boundingBox->moveUpperBound( lroundf(pos.getY() - sprite->h * 0.5) );
  boundingBox->moveLeftBound( lroundf(pos.getX() - sprite->w * 0.5) );
}

void Item::draw(SdlCompat_AcceleratedSurface *screen) {
  SDL_Rect r;
  r.x = lroundf(pos.getX()) - sprite->w / 2;
  r.y = lroundf(pos.getY()) - sprite->h / 2;
  r.w = sprite->w;
  r.h = sprite->h;
  SDL_BlitSurface( sprite, 0, screen, &r );
}

BoundingBox *Item::getBoundingBox() {
  return boundingBox;
}


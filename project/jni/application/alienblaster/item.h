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
#ifndef ITEM_H
#define ITEM_H

#include "SDL.h"
#include "geometry.h"
#include <string>
#include "global.h"

class BoundingBox;

class Item {
  SdlCompat_AcceleratedSurface *sprite;
  BoundingBox *boundingBox;

  Vector2D pos;
  Vector2D vel;
  ItemTypes itemType;

  int timeLived;

  void updateBoundingBox();

  public:
  Item( const Vector2D &position, const Vector2D &velocity, ItemTypes itemType );
  ~Item();

  void update( int dT );
  
  void deleteItem();

  void draw(SdlCompat_AcceleratedSurface *screen);

  inline bool isExpired() { return (timeLived >= ITEM_LIFETIME); }
  inline Vector2D getPos() { return pos; }
  inline Vector2D getVel() { return vel; }
  inline void pickedUp() { timeLived = ITEM_LIFETIME; }
  inline ItemTypes getType() { return itemType; }
  BoundingBox *getBoundingBox();
};

#endif

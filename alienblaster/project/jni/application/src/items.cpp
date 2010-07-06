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
#include "item.h"
#include "items.h"
#include "surfaceDB.h"
#include "geometry.h"
#include "global.h"
#include "racers.h"

Items::Items() {
  timeNextItemAppear = 
    ITEM_APPEAR_DELAY + 
    (rand() % ITEM_APPEAR_RAND_DELAY);
}

Items::~Items() {
  vector<Item *>::iterator i;
  for (i = items.begin(); i != items.end(); ++i) {
    delete *i;
  }
}

void Items::addItem(Item *item) {
  if (item) {
    items.push_back(item);
  }
}

void Items::expireItems() {
  unsigned int i = 0;
  while ( i < items.size() ) {
    if ( items[i]->isExpired() ) {
      delete items[i];
      items.erase(items.begin() + i);
    } else {
      i++;
    }
  }
}

void Items::update( int dT ) {
  vector<Item *>::iterator i;
  for (i = items.begin(); i != items.end(); ++i) {
    (*i)->update( dT );
  }
}

void Items::draw(SdlCompat_AcceleratedSurface *screen) {
  vector<Item *>::iterator i;
  for (i = items.begin(); i != items.end(); ++i) {
    (*i)->draw(screen);
  }
}

Item *Items::getItem(unsigned int idx) {
  return items[idx];
}


void Items::generateItemNow( Vector2D pos, Vector2D vel ) {
  if ( pos.getX() < 10 ) pos.setX( 10 );
  if ( pos.getX() > SCREEN_WIDTH-10 ) pos.setX( SCREEN_WIDTH-10 );
  if ( pos.getY() < 100 && vel.getY() < 5 ) vel.setY( 5 );
  
  int itemType;
  // 10 tries for a correct item
  for ( int i = 0; i < 10; i++ ) {
    itemType = getRandValue( ITEM_APPEAR_CHANCES, NR_ITEM_TYPES );
    if ( ( racers->isShipTypeActive( LIGHT_FIGHTER ) &&
	   racers->isShipTypeActive( HEAVY_FIGHTER ) ) ||
	 ( racers->isShipTypeActive( LIGHT_FIGHTER ) &&
	   ( itemType == ITEM_PRIMARY_UPGRADE ||
	     itemType == ITEM_DUMBFIRE_DOUBLE ||
	     itemType == ITEM_KICK_ASS_ROCKET ||
	     itemType == ITEM_HELLFIRE ||
	     itemType == ITEM_MACHINE_GUN ||
	     itemType == ITEM_HEALTH ||
	     itemType == ITEM_HEATSEEKER ||
	     itemType == ITEM_NUKE ||
	     itemType == ITEM_ENERGY_BEAM ) ) ||
	 ( racers->isShipTypeActive( HEAVY_FIGHTER ) &&
	   ( itemType == ITEM_PRIMARY_UPGRADE ||
	     itemType == ITEM_DUMBFIRE_DOUBLE ||
	     itemType == ITEM_KICK_ASS_ROCKET ||
	     itemType == ITEM_HEALTH ||
	     itemType == ITEM_HEATSEEKER ||
	     itemType == ITEM_NUKE ||
	     itemType == ITEM_DEFLECTOR ||
	     itemType == ITEM_LASER ) ) ) {
      Item *item = new Item( pos, vel, (ItemTypes)itemType );
      addItem( item );
      break;
    }
  }
}

void Items::generate( int dT ) {
  timeNextItemAppear -= dT;
  
  if ( timeNextItemAppear < 0 ) {
    timeNextItemAppear = ITEM_APPEAR_DELAY + (rand() % ITEM_APPEAR_RAND_DELAY);
    generateItemNow( Vector2D( 150 + (rand() % 340), -20 ),
		     Vector2D( (rand() % lroundf(SCROLL_SPEED)) - SCROLL_SPEED / 2,
			       SCROLL_SPEED + (rand() % lroundf(SCROLL_SPEED)) ) );
  }
}

void Items::deleteAllItems() {
  vector<Item *>::iterator i;
  for (i = items.begin(); i != items.end(); ++i) {
    (*i)->deleteItem();
  }
  expireItems();
}

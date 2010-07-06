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
#ifndef ITEMS_H
#define ITEMS_H

#include "SDL.h"
#include <vector>

class Item;
class Vector2D;

class Items {
  vector<Item *> items;

  int timeNextItemAppear;

  public:
  Items();
  ~Items();

  void addItem(Item *item);
  void generateItemNow( Vector2D pos, Vector2D vel );
  void generate( int dT );
  void expireItems();
  void update( int dT );
  void draw(SdlCompat_AcceleratedSurface *screen);
  void deleteAllItems();
  
  inline unsigned int getNrItems() { return items.size(); }
  Item *getItem(unsigned int idx);
};


#endif

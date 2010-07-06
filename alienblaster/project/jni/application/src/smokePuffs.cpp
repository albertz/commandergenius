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
#include "smokePuff.h"
#include "smokePuffs.h"
#include "surfaceDB.h"
#include "global.h"

SmokePuffs::SmokePuffs() {}

SmokePuffs::~SmokePuffs() {
  vector<SmokePuff *>::iterator i;
  for (i = smokePuffs.begin(); i != smokePuffs.end(); ++i) {
    delete *i;
  }
}

void SmokePuffs::addSmokePuff( SmokePuff *smokePuff ) {
  if ( smokePuff ) {
    smokePuffs.push_back( smokePuff );
  }
}

void SmokePuffs::addSmokePuff( Vector2D pos, Vector2D vel, SmokePuffTypes whichType ) {
  SmokePuff *newSmokePuff = new SmokePuff( pos, vel, whichType );
  addSmokePuff( newSmokePuff );
}

void SmokePuffs::expireSmokePuffs() {
  unsigned int i = 0;
  while ( i < smokePuffs.size() ) {
    if ( smokePuffs[i]->isExpired() ) {
      delete smokePuffs[i];
      smokePuffs.erase( smokePuffs.begin() + i );
    } else {
      i++;
    }
  }
}

void SmokePuffs::update( int dT ) {
  vector<SmokePuff *>::iterator i;
  for (i = smokePuffs.begin(); i != smokePuffs.end(); ++i) {
    (*i)->update( dT );
  }
}

void SmokePuffs::draw(SdlCompat_AcceleratedSurface *screen) {
  vector<SmokePuff *>::iterator i;
  for (i = smokePuffs.begin(); i != smokePuffs.end(); ++i) {
    (*i)->drawSmokePuff( screen );
  }
}

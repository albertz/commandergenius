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

#include "explosions.h"
#include "explosion.h"

Explosions::Explosions() {
}

Explosions::~Explosions() {
  vector<Explosion *>::iterator i;
  for (i = explosions.begin(); i != explosions.end(); ++i) {
    delete *i;
  }
}

void Explosions::addExplosion(Explosion *explosion) {
  if (explosion) {
    explosions.push_back(explosion);
  }
}

void Explosions::drawAirExplosions(SdlCompat_AcceleratedSurface *screen) {
  vector<Explosion *>::iterator i;
  for (i = explosions.begin(); i != explosions.end(); ++i) {
    (*i)->drawAirExplosion(screen);
  }
}

void Explosions::drawGroundExplosions(SdlCompat_AcceleratedSurface *screen) {
  vector<Explosion *>::iterator i;
  for (i = explosions.begin(); i != explosions.end(); ++i) {
    (*i)->drawGroundExplosion(screen);
  }
}

void Explosions::updateExplosions( int dT ) {
  vector<Explosion *>::iterator i;
  for (i = explosions.begin(); i != explosions.end(); ++i) {
    (*i)->update( dT );
  }
}

void Explosions::expireExplosions() {
  unsigned int i = 0;
  while ( i < explosions.size() ) {
    if ( explosions[i]->isExpired() ) {
      delete explosions[i];
      explosions.erase(explosions.begin() + i);
    } else {
      i++;
    }
  }
}

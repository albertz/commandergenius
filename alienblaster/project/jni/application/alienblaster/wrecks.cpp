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
#include "wrecks.h"
#include "wreck.h"

Wrecks::Wrecks() {
}

Wrecks::~Wrecks() {
  vector<Wreck *>::iterator i;
  for (i = wrecks.begin(); i != wrecks.end(); ++i) {
    delete *i;
  }
}

void Wrecks::addWreck(Wreck *wreck) {
  if (wreck) {
    wrecks.push_back(wreck);
  }
}

void Wrecks::expireWrecks() {
  unsigned int i = 0;
  while ( i < wrecks.size() ) {
    if ( wrecks[i]->isExpired() ) {
      delete wrecks[i];
      wrecks.erase(wrecks.begin() + i);
    } else {
      i++;
    }
  }
}

void Wrecks::updateWrecks( int dT ) {
  vector<Wreck *>::iterator i;
  for (i = wrecks.begin(); i != wrecks.end(); ++i) {
    (*i)->update( dT );
  }
}

void Wrecks::draw(SdlCompat_AcceleratedSurface *screen) {
  vector<Wreck *>::iterator i;
  for (i = wrecks.begin(); i != wrecks.end(); ++i) {
    (*i)->draw(screen);
  }
}

void Wrecks::deleteAllWrecks() {
  vector<Wreck *>::iterator i;
  for (i = wrecks.begin(); i != wrecks.end(); ++i) {
    (*i)->deleteWreck();
  }  
}

Wreck *Wrecks::getWreck(unsigned int idx) {
  return wrecks[idx];
}

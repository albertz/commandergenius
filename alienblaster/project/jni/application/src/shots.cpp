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

#include "shots.h"
#include "shot.h"

Shots::Shots() {}

Shots::~Shots() {
  vector<Shot *>::iterator i;
  for (i = shots.begin(); i != shots.end(); ++i) {
    delete *i;
  }
}

void Shots::addShot(Shot *shot) {
  if (shot) {
    shots.push_back(shot);
  }
}

void Shots::moveAndCollide( int dT ) {
  vector<Shot *>::iterator i;
  for (i = shots.begin(); i != shots.end(); ++i) {
    (*i)->moveAndCollide( dT );
  }
}

void Shots::deleteAllShots() {
  vector<Shot *>::iterator i;
  for (i = shots.begin(); i != shots.end(); ++i) {
    delete *i;
  }
  shots.clear();
}

void Shots::expireShots() {
  unsigned int i = 0;
  while ( i < shots.size() ) {
    if ( shots[i]->isExpired() ) {
      delete shots[i];
      // TODO: Performance-Optimierung, indem der letzte Schuß
      // nach vorne kopiert und dann das letzte Element gelöscht wird.
      // Dann wird nicht immer der Rest des Vektors kopiert.
      shots.erase(shots.begin() + i);
    } else {
      i++;
    }
  }
}

void Shots::drawShadows(SDL_Surface *screen) {
  vector<Shot *>::iterator i;
  for (i = shots.begin(); i != shots.end(); ++i) {
    (*i)->drawShadow(screen);
  }
}

void Shots::drawGroundShots(SDL_Surface *screen) {
  vector<Shot *>::iterator i;
  for (i = shots.begin(); i != shots.end(); ++i) {
    (*i)->drawGroundShot(screen);
  }
}

void Shots::drawAirShots(SDL_Surface *screen) {
  vector<Shot *>::iterator i;
  for (i = shots.begin(); i != shots.end(); ++i) {
    (*i)->drawAirShot(screen);
  }
}

void Shots::drawGroundAirShots(SDL_Surface *screen) {
  vector<Shot *>::iterator i;
  for (i = shots.begin(); i != shots.end(); ++i) {
    (*i)->drawGroundAirShot(screen);
  }
}

Shot* Shots::getNearestRocket(Vector2D position) {
  vector<Shot *>::iterator i;
  float distance = 99999.0;
  Shot* rocket = NULL;

  for (i = shots.begin(); i != shots.end(); ++i) {
    if ( (*i)->getShotType() == ENEMY_SHOT_TANK_ROCKET ) {
      if (position.distanceTo( (*i)->getPos() ) < distance ) {
        distance = position.distanceTo( (*i)->getPos() );
        rocket = (*i);
      }
    }
  }
  return rocket;
}

bool Shots::existsRocket() {
  vector<Shot *>::iterator i;
  for (i = shots.begin(); i != shots.end(); ++i) {
    if ( (*i)->getShotType() == ENEMY_SHOT_TANK_ROCKET ) {
      return true;
    }
  }
  return false;
}

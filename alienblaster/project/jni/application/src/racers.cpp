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
#include "racer.h"
#include "racers.h"
#include "shots.h"
#include "items.h"
#include "enemys.h"
#include "enemy.h"
#include "explosions.h"
#include "explosion.h"
#include "global.h"

Racers::Racers() {
  pointsInArcadeMode = 0;
}

Racers::~Racers() {
  vector<Racer *>::iterator i;
  for ( i = racers.begin(); i != racers.end(); ++i ) {
    delete *i;
  }
}

bool Racers::isShipTypeActive( int shipType ) {
  vector<Racer *>::iterator i;
  for ( i = racers.begin(); i != racers.end(); ++i ) {
    if ( (*i)->getShipType() == shipType ) {
      return true;
    }
  }
  return false;
}

Racer *Racers::getRacer( unsigned int i ) {
  return racers[i];
}

void Racers::addRacer( Racer *racer ) {
  if (racer) {
    racers.push_back(racer);
  }
}

void Racers::deleteRacers() {
  racers.clear();
}

void Racers::collideWithEnemys() {
  for ( unsigned int i = 0; i < racers.size(); ++i ) {
    for ( unsigned int j = 0; j < enemys->getNrEnemys(); ++j ) {

      // wurde noch nicht von einem anderen Racer gerammt und fliegt ebenfalls
      if ( !(enemys->getEnemy( j )->isExpired()) &&
	   ENEMY_FLYING[ enemys->getEnemy( j )->getType() ] ) {
	// kollidiert
	if ( racers[i]->collidesWithAsCircle( enemys->getEnemy( j )->getBoundingCircle() ) ) {
	  // explosion
    Explosion *newExplosion = 
   	new Explosion( FN_EXPLOSION_ENEMY, enemys->getEnemy( j )->getPos(), enemys->getEnemy( j )->getVel(), EXPLOSION_NORMAL_AIR );
    explosions->addExplosion( newExplosion );
	  enemys->getEnemy( j )->expire();
	  racers[i]->receiveDamage( ENEMY_COLLISION_DAMAGE[ enemys->getEnemy( j )->getType() ]);
	}
      }
      
    }
  }
}

void Racers::moveAndCollide( int dT ) {
  for ( unsigned int i = 0; i < racers.size(); ++i ) {
    racers[i]->move( dT );
    collideWithEnemys();
  }
}

void Racers::pickUpItems() {
  vector<Racer *>::iterator i;
  for ( i = racers.begin(); i != racers.end(); ++i ) {
    (*i)->pickUpItems();
  }
}

void Racers::shoot() {
  for ( unsigned int i = 0; i < racers.size(); i++ ) {
    racers[i]->shootPrimary();
    racers[i]->shootSecondary();
    racers[i]->shootSpecial();
  }
}

void Racers::rechargeShield( int dT ) {
  vector<Racer *>::iterator i;
  for (i = racers.begin(); i != racers.end(); ++i) {
    (*i)->rechargeShield( dT );
  }
}

void Racers::drawRacers( SdlCompat_AcceleratedSurface *screen ) {
  vector<Racer *>::iterator i;
  for (i = racers.begin(); i != racers.end(); ++i) {
    (*i)->drawRacer(screen);
  }
}

void Racers::drawShadows( SdlCompat_AcceleratedSurface *screen ) {
  vector<Racer *>::iterator i;
  for (i = racers.begin(); i != racers.end(); ++i) {
    (*i)->drawShadow(screen);
  }
}

void Racers::drawStats( SdlCompat_AcceleratedSurface *screen ) {
  vector<Racer *>::iterator i;
  for (i = racers.begin(); i != racers.end(); ++i) {
    (*i)->drawStats(screen);
  }
}

bool Racers::bothPlayersLost() {
  bool allLost = true;
  for ( unsigned int i = 0; i < racers.size(); ++i) {
    if ( !racers[i]->isDead() ) {
      allLost = false;
    }
  }
  return allLost;
}

void Racers::receivePointsArcade( float amount ) {
  for ( unsigned int i = 0; i < racers.size(); ++i) {
    racers[i]->receivePoints( amount );
  }
}

int Racers::getPointsArcadeMode() {
  if (!arcadeGame) return -1;
   if (!racers.empty()) {
    return racers[0]->getPoints();
  }
  else return pointsInArcadeMode;
}

void Racers::expireRacers() {
  unsigned int i = 0;
  while ( i < racers.size() ) {
    if ( racers[i]->isDead() ) {
      // explode
      Explosion *newExplosion = 
     	new Explosion( FN_EXPLOSION_ENEMY, racers[i]->getPos(), 
		       racers[i]->getVel(), EXPLOSION_NORMAL_AIR );
      explosions->addExplosion( newExplosion );

      if ( arcadeGame ) pointsInArcadeMode = racers[i]->getPoints();

      delete racers[i];
      racers.erase(racers.begin() + i);
    } else {
      i++;
    }
  }
}

void Racers::getKeyActionMaps() {
  playerKeys0 = settings->getPlayerKeys( 0 );
  playerKeys1 = settings->getPlayerKeys( 1 );
}

void Racers::handleEvent( const SDLKey key, const bool pressed ) {
  PlayerEvent pEvent0 = playerKeys0[ key ];
  if ( pEvent0 != PE_UNKNOWN ) {
    handlePlayerEvent( pEvent0, 0, pressed );
  }
  PlayerEvent pEvent1 = playerKeys1[ key ];
  if ( pEvent1 != PE_UNKNOWN ) {
    handlePlayerEvent( pEvent1, 1, pressed );
  }
}

void Racers::handlePlayerEvent( PlayerEvent pEvent, int playerNr, bool keyDown ) {
  for ( unsigned int i = 0; i < racers.size(); i++ )
    if ( racers[i]->getPlayerNr() == playerNr ) 
      racers[i]->handlePlayerEvent( pEvent, keyDown );
}

  

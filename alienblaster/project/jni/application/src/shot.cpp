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

#include <iostream>
#include "shot.h"
#include "surfaceDB.h"
#include "racers.h"
#include "racer.h"
#include "explosions.h"
#include "explosion.h"
#include "enemys.h"
#include "enemy.h"
#include "smokePuffs.h"
#include "boundingBox.h"

Shot::Shot( ShotTypes shotType, int playerNr, Vector2D position, float angle ) {

  this->shotType = shotType;
  fromWhichPlayer = playerNr;
  
  pos = position;
  collidedWithGround = false;
  deflectedBySonicFromPlayer1 = false;
  deflectedBySonicFromPlayer2 = false;

  generatesSmokePuffs = false;
  timeToNextSmokePuff = 100;

  switch (shotType) {
    // primary shots
  case SHOT_NORMAL:
    {
      vel = Vector2D( VEL_SHOT_NORMAL, angle, POLAR);
      timeToLive = LIFETIME_SHOT_NORMAL;
      sprite = surfaceDB.loadSurface( FN_SHOT_NORMAL );
      break;
    }
  case SHOT_NORMAL_HEAVY:
    {
      vel = Vector2D( VEL_SHOT_NORMAL_HEAVY, angle, POLAR);
      timeToLive = LIFETIME_SHOT_NORMAL_HEAVY;
      sprite = surfaceDB.loadSurface( FN_SHOT_NORMAL_HEAVY );
      break;
    }
  case SHOT_DOUBLE:
    {
      vel = Vector2D(VEL_SHOT_DOUBLE, angle, POLAR);
      timeToLive = LIFETIME_SHOT_DOUBLE;
      sprite = surfaceDB.loadSurface( FN_SHOT_DOUBLE );
      break;
    }
  case SHOT_DOUBLE_HEAVY:
    {
      vel = Vector2D(VEL_SHOT_DOUBLE_HEAVY, angle, POLAR);
      timeToLive = LIFETIME_SHOT_DOUBLE_HEAVY;
      sprite = surfaceDB.loadSurface( FN_SHOT_DOUBLE_HEAVY );
      break;
    }
  case SHOT_TRIPLE:
    {
      vel = Vector2D(VEL_SHOT_TRIPLE, angle, POLAR);
      timeToLive = LIFETIME_SHOT_TRIPLE;
      sprite = surfaceDB.loadSurface( FN_SHOT_TRIPLE );
      break;
    }

    // primary shots heavy fighter
  case SHOT_HF_NORMAL:
    {
      vel = Vector2D( VEL_SHOT_HF_NORMAL, angle, POLAR);
      timeToLive = LIFETIME_SHOT_HF_NORMAL;
      sprite = surfaceDB.loadSurface( FN_SHOT_HF_NORMAL );
      break;
    }
  case SHOT_HF_DOUBLE:
    {
      vel = Vector2D( VEL_SHOT_HF_DOUBLE, angle, POLAR);
      timeToLive = LIFETIME_SHOT_HF_DOUBLE;
      sprite = surfaceDB.loadSurface( FN_SHOT_HF_DOUBLE );
      break;
    }
  case SHOT_HF_TRIPLE:
    {
      vel = Vector2D(VEL_SHOT_HF_TRIPLE, angle, POLAR);
      timeToLive = LIFETIME_SHOT_HF_TRIPLE;
      sprite = surfaceDB.loadSurface( FN_SHOT_HF_TRIPLE );
      break;
    }
  case SHOT_HF_QUATTRO:
    {
      vel = Vector2D(VEL_SHOT_HF_QUATTRO, angle, POLAR);
      timeToLive = LIFETIME_SHOT_HF_QUATTRO;
      sprite = surfaceDB.loadSurface( FN_SHOT_HF_QUATTRO );
      break;
    }
  case SHOT_HF_QUINTO:
    {
      vel = Vector2D(VEL_SHOT_HF_QUINTO, angle, POLAR);
      timeToLive = LIFETIME_SHOT_HF_QUINTO;
      sprite = surfaceDB.loadSurface( FN_SHOT_HF_QUINTO );
      break;
    }

    // secondary shots
  case SHOT_DUMBFIRE:
    {
      vel = Vector2D(VEL_SHOT_DUMBFIRE, angle, POLAR);
      timeToLive = LIFETIME_SHOT_DUMBFIRE;
      sprite = surfaceDB.loadSurface( FN_SHOT_DUMBFIRE );
      generatesSmokePuffs = true;
      break;
    }
  case SHOT_DUMBFIRE_DOUBLE:
    {
      vel = Vector2D(VEL_SHOT_DUMBFIRE_DOUBLE, angle, POLAR);
      timeToLive = LIFETIME_SHOT_DUMBFIRE_DOUBLE;
      sprite = surfaceDB.loadSurface( FN_SHOT_DUMBFIRE_DOUBLE );
      generatesSmokePuffs = true;
      break;
    }
  case SHOT_KICK_ASS_ROCKET: 
    {
      vel = Vector2D(VEL_SHOT_KICK_ASS_ROCKET, angle, POLAR);
      timeToLive = LIFETIME_SHOT_KICK_ASS_ROCKET;
      sprite = surfaceDB.loadSurface( FN_SHOT_KICK_ASS_ROCKET );
      spriteShadow = surfaceDB.loadSurface( FN_SHOT_KICK_ASS_ROCKET_SHADOW, true );
      break;
    }
  case SHOT_HELLFIRE:
    {
      vel = Vector2D(VEL_SHOT_HELLFIRE / 2.0, angle, POLAR);
      timeToLive = LIFETIME_SHOT_HELLFIRE;
      sprite = surfaceDB.loadSurface( FN_SHOT_HELLFIRE );
      spriteShadow = surfaceDB.loadSurface( FN_SHOT_HELLFIRE_SHADOW, true );
      generatesSmokePuffs = true;
      break;
    }
  case SHOT_MACHINE_GUN:
    {
      vel = Vector2D(VEL_SHOT_MACHINE_GUN, angle, POLAR);
      timeToLive = LIFETIME_SHOT_MACHINE_GUN;
      sprite = surfaceDB.loadSurface( FN_SHOT_MACHINE_GUN );
      break;
    }
  case SHOT_ENERGY_BEAM:
    {
      vel = Vector2D(VEL_SHOT_ENERGY_BEAM, angle, POLAR);
      timeToLive = LIFETIME_SHOT_ENERY_BEAM;
      sprite = surfaceDB.loadSurface( FN_SHOT_ENERGY_BEAM, true );
      break;
    }

    // secondary shots heavy fighter
  case SHOT_HF_DUMBFIRE:
    {
      vel = Vector2D(VEL_SHOT_HF_DUMBFIRE, angle, POLAR);
      timeToLive = LIFETIME_SHOT_HF_DUMBFIRE;
      sprite = surfaceDB.loadSurface( FN_SHOT_HF_DUMBFIRE );
      generatesSmokePuffs = true;
      break;
    }
  case SHOT_HF_DUMBFIRE_DOUBLE:
    {
      vel = Vector2D(VEL_SHOT_HF_DUMBFIRE_DOUBLE, angle, POLAR);
      timeToLive = LIFETIME_SHOT_HF_DUMBFIRE_DOUBLE;
      sprite = surfaceDB.loadSurface( FN_SHOT_HF_DUMBFIRE_DOUBLE );
      generatesSmokePuffs = true;
      break;
    }
  case SHOT_HF_KICK_ASS_ROCKET: 
    {
      vel = Vector2D(VEL_SHOT_HF_KICK_ASS_ROCKET, angle, POLAR);
      timeToLive = LIFETIME_SHOT_HF_KICK_ASS_ROCKET;
      sprite = surfaceDB.loadSurface( FN_SHOT_HF_KICK_ASS_ROCKET );
      spriteShadow = surfaceDB.loadSurface( FN_SHOT_HF_KICK_ASS_ROCKET_SHADOW, true );
      break;
    }
  case SHOT_HF_LASER:
    {
      vel = Vector2D(VEL_SHOT_HF_LASER, angle, POLAR);
      timeToLive = LIFETIME_SHOT_HF_LASER;
      sprite = surfaceDB.loadSurface( FN_SHOT_HF_LASER );
      break;
    }

    // special shots
  case SPECIAL_SHOT_HEATSEEKER:
    {
      vel = Vector2D(VEL_SPECIAL_SHOT_HEATSEEKER, angle, POLAR);
      timeToLive = LIFETIME_SPECIAL_SHOT_HEATSEEKER;
      sprite = surfaceDB.loadSurface( FN_SPECIAL_SHOT_HEATSEEKER );
      generatesSmokePuffs = true;
      break;
    }
  case SPECIAL_SHOT_NUKE:
    {
      vel = Vector2D( VEL_SPECIAL_SHOT_NUKE, angle, POLAR );
      timeToLive = LIFETIME_SPECIAL_SHOT_NUKE;
      sprite = surfaceDB.loadSurface( FN_SPECIAL_SHOT_NUKE );
      spriteShadow = surfaceDB.loadSurface( FN_SPECIAL_SHOT_NUKE_SHADOW, true );
      break;
    }

    // enemy shots
  case ENEMY_SHOT_NORMAL:
    {
      vel = Vector2D(VEL_ENEMY_SHOT_NORMAL, angle, POLAR);
      timeToLive = LIFETIME_ENEMY_SHOT_NORMAL;
      sprite = surfaceDB.loadSurface( FN_ENEMY_SHOT_NORMAL );
      break;
    }
  case ENEMY_SHOT_TANK_ROCKET:
    {
      vel = Vector2D(VEL_ENEMY_SHOT_TANK_ROCKET, angle, POLAR);
      timeToLive = LIFETIME_ENEMY_SHOT_TANK_ROCKET;
      sprite = surfaceDB.loadSurface( FN_ENEMY_SHOT_TANK_ROCKET );
      spriteShadow = surfaceDB.loadSurface( FN_ENEMY_SHOT_TANK_ROCKET_SHADOW, true );
      generatesSmokePuffs = true;
      break;
    }
    
  default: 
    {
      vel = Vector2D(0,0);
      timeToLive = 0;
      sprite = surfaceDB.loadSurface( FN_SHOT_NORMAL );
      break;
    }
  }

}


void Shot::moveAndCollide( int dT ) {

  if ( fromWhichPlayer == 666 ) {
    moveAndCollideEnemyShot( dT );
  } else {
    moveAndCollidePlayerShot( dT );
  }

  generateSmokePuff( dT );

  timeToLive -= dT;
}


void Shot::moveAndCollidePlayerShot( int dT ) {

  Vector2D posOld = pos;

  // move the shot
  movePlayerShot( dT );

  if ( !isExpired() && collidePlayerShot( posOld ) ) {
    addExplosion();
  }
}


void Shot::movePlayerShot( int dT ) {
  switch (shotType) {
  case SHOT_NORMAL:
  case SHOT_NORMAL_HEAVY:
  case SHOT_DOUBLE:
  case SHOT_DOUBLE_HEAVY:
  case SHOT_TRIPLE:
  case SHOT_HF_NORMAL:
  case SHOT_HF_DOUBLE:
  case SHOT_HF_TRIPLE:
  case SHOT_HF_QUATTRO:
  case SHOT_HF_QUINTO:
  case SHOT_DUMBFIRE:
  case SHOT_DUMBFIRE_DOUBLE:
  case SHOT_KICK_ASS_ROCKET:
  case SHOT_MACHINE_GUN:
  case SHOT_ENERGY_BEAM:
  case SHOT_HF_DUMBFIRE:
  case SHOT_HF_DUMBFIRE_DOUBLE:
  case SHOT_HF_KICK_ASS_ROCKET:
  case SHOT_HF_LASER:
    {
      pos = pos + vel * dT / 1000.0;
      break;
    }
  case SHOT_HELLFIRE:
    {
      pos = pos + vel * dT / 1000.0;
      if ( timeToLive < LIFETIME_SHOT_HELLFIRE - 100 ) {
	vel = Vector2D( 0, -VEL_SHOT_HELLFIRE );
      }
      break;
    }
  case SPECIAL_SHOT_HEATSEEKER:
    {
      pos = pos + vel * dT / 1000.0;
      if ( timeToLive >= LIFETIME_SPECIAL_SHOT_HEATSEEKER - 200 ) break;
      int idxNearestEnemy = 0;
      float distNearest = 10000;
      for ( unsigned int i = 0; i < enemys->getNrEnemys(); i++ ) {
	// vector from the shotPosition to the enemy
	Vector2D v = enemys->getEnemy(i)->getPos() - pos;
	if ( getAbsAngleDifference( vel.getDirection(), v.getDirection() ) < 180 ) {
	  float dist = (enemys->getEnemy(i)->getPos() - pos).getLength();
	  if ( dist < distNearest ) {
	    distNearest = dist;
	    idxNearestEnemy = i;
	  }
	}
      }
      if ( distNearest != 10000 ) {
	float angle = 
	  getAngleDifference( (enemys->getEnemy(idxNearestEnemy)->getPos()-pos).getDirection(),
			      vel.getDirection() );
	if ( fabs(angle) < 80 * dT / 1000.0 ) {
	  vel.rotate(angle);
	} else if ( angle < 0 ) {
	  vel.rotate( -80 * dT / 1000.0 );
	} else {
	  vel.rotate( 80 * dT / 1000.0 );
	}
      } 

      break;
    }

  case SPECIAL_SHOT_NUKE:
    {
      pos = pos + vel * dT / 1000.0;
      // Nuke is in its place!
      if ( (pos - Vector2D( SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0 )).getLength()
	   <= (vel * dT / 1000.0).getLength() ) {
	nukeIsInPlace = true;
      }
      break;
    }

  default: 
    {
      cout << "movePlayerShot: unexpected shotType: " << shotType << endl;
      break;
    }
  }

  // clip at the outside of the window
  if ( !RectangleGeo( Vector2D( -SHOT_SCREEN_BORDER, -SHOT_SCREEN_BORDER ), 
		      Vector2D( SCREEN_WIDTH + SHOT_SCREEN_BORDER, 
				SCREEN_HEIGHT + SHOT_SCREEN_BORDER )).isInside(pos) ) {
    timeToLive = 0;
  }
}


bool Shot::collidePlayerShot( Vector2D posOld ) {
  switch (shotType) {
    // only against air
  case SHOT_ENERGY_BEAM:
    {
      BoundingBox box( lroundf(posOld.getX()) - sprite->w / 2,
		       lroundf(pos.getY()) - sprite->w / 2,
		       sprite->w,
		       lroundf((posOld-pos).getY()) + sprite->h );		       
      for ( unsigned int i = 0; i < enemys->getNrEnemys(); i++ ) {
	if ( ENEMY_FLYING[ enemys->getEnemy(i)->getType() ] &&
	     enemys->getEnemy(i)->collidesWith( &box ) ) {
	  enemys->getEnemy(i)->doDamage( shotType, fromWhichPlayer );
	  timeToLive = 0;
	  collidedWithGround = !ENEMY_FLYING[ enemys->getEnemy(i)->getType() ];
	  return true;
	}
      }
      break;
    }
    //only against air
  case SHOT_HF_LASER:
    {
      for ( unsigned int i = 0; i < enemys->getNrEnemys(); i++ ) {
	if ( ENEMY_FLYING[ enemys->getEnemy(i)->getType() ] &&
	     enemys->getEnemy(i)->collidesWith( posOld, pos ) ) {
	  enemys->getEnemy(i)->doDamage( shotType, fromWhichPlayer );
	  timeToLive = 0;
	  collidedWithGround = false;
	  return true;
	}
      }
      break;
    }
  // against air and ground
  case SHOT_NORMAL:
  case SHOT_NORMAL_HEAVY:
  case SHOT_DOUBLE:
  case SHOT_DOUBLE_HEAVY:
  case SHOT_TRIPLE:
  case SHOT_HF_NORMAL:
  case SHOT_HF_DOUBLE:
  case SHOT_HF_TRIPLE:
  case SHOT_HF_QUATTRO:
  case SHOT_HF_QUINTO:
    {
      for ( unsigned int i = 0; i < enemys->getNrEnemys(); i++ ) {
	if ( //ENEMY_FLYING[ enemys->getEnemy(i)->getType() ] &&
	     enemys->getEnemy(i)->collidesWith( posOld, pos ) ) {
	  enemys->getEnemy(i)->doDamage( shotType, fromWhichPlayer );
	  timeToLive = 0;
	  collidedWithGround = !ENEMY_FLYING[ enemys->getEnemy(i)->getType() ];
	  return true;
	}
      }
      break;
    }
    // against air and ground
  case SHOT_DUMBFIRE:
  case SHOT_DUMBFIRE_DOUBLE:
  case SHOT_HF_DUMBFIRE:
  case SHOT_HF_DUMBFIRE_DOUBLE:
    {
      for ( unsigned int i = 0; i < enemys->getNrEnemys(); i++ ) {
	if ( enemys->getEnemy(i)->collidesWith( Circle(pos, 15) ) ) {
	  enemys->getEnemy(i)->doDamage( shotType, fromWhichPlayer );
	  timeToLive = 0;
	  collidedWithGround = !ENEMY_FLYING[ enemys->getEnemy(i)->getType() ];
	  return true;
	}
      }
      break;
    }
    // only against ground
  case SHOT_KICK_ASS_ROCKET:
  case SHOT_HF_KICK_ASS_ROCKET:
    {
      for ( unsigned int i = 0; i < enemys->getNrEnemys(); i++ ) {
	if ( (!ENEMY_FLYING[ enemys->getEnemy(i)->getType() ]) && 
	     enemys->getEnemy(i)->collidesWith( Circle(pos, 15) ) ) {
	  enemys->getEnemy(i)->doDamage( shotType, fromWhichPlayer );
	  timeToLive = 0;
	  collidedWithGround = true;
	  return true;
	}
      }
      break;
    }
    // only against ground, but has to hit more exactly than kickAssRocket
  case SHOT_HELLFIRE:
    {
      for ( unsigned int i = 0; i < enemys->getNrEnemys(); i++ ) {
	if ( (!ENEMY_FLYING[ enemys->getEnemy(i)->getType() ]) && 
	     enemys->getEnemy(i)->collidesWith( Circle(pos, 5) ) ) {
	  enemys->getEnemy(i)->doDamage( shotType, fromWhichPlayer );
	  timeToLive = 0;
	  collidedWithGround = true;
	  return true;
	}
      }
      break;
    }
    // against air and ground
  case SHOT_MACHINE_GUN:
    {
      for ( unsigned int i = 0; i < enemys->getNrEnemys(); i++ ) {
	if ( enemys->getEnemy(i)->collidesWith( posOld, pos ) ) {
	  enemys->getEnemy(i)->doDamage( shotType, fromWhichPlayer );
	  timeToLive = 0;
	  collidedWithGround = !ENEMY_FLYING[ enemys->getEnemy(i)->getType() ];
	  return true;
	}
      }
      break;
    }

    // against air and ground
  case SPECIAL_SHOT_HEATSEEKER:
    {
      for ( unsigned int i = 0; i < enemys->getNrEnemys(); i++ ) {
	if ( enemys->getEnemy(i)->collidesWith( Circle(pos, 5) ) ) {
	  enemys->getEnemy(i)->doDamage( shotType, fromWhichPlayer );
	  timeToLive = 0;
	  collidedWithGround = !ENEMY_FLYING[ enemys->getEnemy(i)->getType() ];
	  return true;
	}
      }
      break;
    }
  case SPECIAL_SHOT_NUKE: break;
    
  default: 
    {
      cout << "collidePlayerShot: unexpected shotType: " << shotType << endl;
      return false;
    }
  }
  return false;
}

///////////////////////////

void Shot::moveAndCollideEnemyShot( int dT ) {
  Vector2D posOld = pos;

  moveEnemyShot( dT );

  if ( !isExpired() && collideEnemyShot( posOld ) ) {
    addExplosion();
  }
}


void Shot::moveEnemyShot( int dT ) {
  switch (shotType) {
  case ENEMY_SHOT_NORMAL: {
    // is this shot near the deflector of a racer?
    for ( unsigned int i = 0; i < racers->getNrRacers(); i++) {
      if ( racers->getRacer(i)->getShipType() == HEAVY_FIGHTER ) {
        Vector2D racerToShot = pos - racers->getRacer(i)->getPos();
        if ( racerToShot.getLength() < RACER_DEFLECTOR_ACTIVATION_DIST ) {
          vel += Vector2D( RACER_DEFLECTOR_POWER * (dT / 1000.0), 
			   racerToShot.getDirection(), POLAR );
        }
        if ( racers->getRacer(i)->isDeflectorSpecialActive() ) {
          if ( racerToShot.getLength() < ITEM_DEFLECTOR_ACTIVATION_DIST ) {
            vel += Vector2D( ITEM_DEFLECTOR_POWER * (dT / 1000.0), 
			     racerToShot.getDirection(), POLAR );
          }
        }
      }
    }
    pos = pos + vel * dT / 1000.0;
    break;
  }
  case ENEMY_SHOT_TANK_ROCKET: {
    if ( deflectedBySonicFromPlayer1 ) {
      Vector2D racerToShot = pos - racers->getRacer(0)->getPos();
      if ( racerToShot.getLength() < RACER_SONIC_ACTIVATION_DIST ) {
        vel += Vector2D( RACER_SONIC_POWER * (dT / 1000.0), 
			 racerToShot.getDirection(), POLAR );
        deflectedBySonicFromPlayer1 = false;
      }
    }
    if ( deflectedBySonicFromPlayer2 ) {
      Vector2D racerToShot = pos - racers->getRacer(1)->getPos();
      if ( racerToShot.getLength() < RACER_SONIC_ACTIVATION_DIST ) {
        vel += Vector2D( RACER_SONIC_POWER * (dT / 1000.0),
			 racerToShot.getDirection(), POLAR );
        deflectedBySonicFromPlayer2 = false;
      }
    }
    pos = pos + vel * dT / 1000.0;
    break;
  }
  default: {
    cout << "moveEnemyShot: unexpected shotType: " << shotType << endl;
    break;
  }
  }

  // clip at the outside of the window
  if ( !RectangleGeo(Vector2D( -SHOT_SCREEN_BORDER, -SHOT_SCREEN_BORDER ), 
		     Vector2D( SCREEN_WIDTH + SHOT_SCREEN_BORDER, 
			       SCREEN_HEIGHT + SHOT_SCREEN_BORDER )).isInside(pos) ) {
    timeToLive = 0;
  }
}


bool Shot::collideEnemyShot( Vector2D posOld ) {
  switch (shotType) {
  case ENEMY_SHOT_NORMAL:
    {
      for ( unsigned int i = 0; i < racers->getNrRacers(); i++ ) {
	if ( racers->getRacer(i)->collidesWith( posOld, pos ) ) {
	  racers->getRacer(i)->doDamage( shotType );
	  timeToLive = 0;
	  collidedWithGround = false;
	  return true;
	}
      }
      break;
    }
  case ENEMY_SHOT_TANK_ROCKET:
    {
      for ( unsigned int i = 0; i < racers->getNrRacers(); i++ ) {
	if ( racers->getRacer(i)->collidesWith( Circle(pos, 5) ) ) {
	  racers->getRacer(i)->doDamage( shotType );
	  timeToLive = 0;
	  collidedWithGround = false;
	  return true;
	}
      }
      break;
    }
  default:
    {
      cout << "collideEnemyShot: unexpected shotType: " << shotType << endl;
      return false;
    }
  }
  return false;
}

/////////////

void Shot::addExplosion() {
  Explosion *explosion;
  switch (shotType) {
  default: 
    {
      if ( collidedWithGround ) {
	explosion = new Explosion( FN_EXPLOSION_NORMAL, pos, 
				   vel / 10.0, EXPLOSION_NORMAL_GROUND );
      } else {
	if ( shotType == SHOT_HF_LASER ) {
	  // Laser is too fast...
	  explosion = new Explosion( FN_EXPLOSION_NORMAL, pos, 
				     vel / 30.0, EXPLOSION_NORMAL_AIR );
	} else {
	  explosion = new Explosion( FN_EXPLOSION_NORMAL, pos, 
				     vel / 10.0, EXPLOSION_NORMAL_AIR );
	}
      }
      break;
    }
  }
  explosions->addExplosion( explosion );
}

////////////////////

void Shot::drawShadow(SdlCompat_AcceleratedSurface *screen) {
  switch (shotType) {
  case SHOT_KICK_ASS_ROCKET:
  case SHOT_HF_KICK_ASS_ROCKET:
  case SHOT_HELLFIRE:
  case SPECIAL_SHOT_NUKE:
    {
      SDL_Rect shadowR;
      shadowR.x = lroundf(pos.getX()) - spriteShadow->w / 2 - 7;
      shadowR.y = lroundf(pos.getY()) - spriteShadow->h / 2 + 7;
      shadowR.w = spriteShadow->w;
      shadowR.h = spriteShadow->h;
      SDL_BlitSurface( spriteShadow, 0, screen, &shadowR );
      break;
    }
  case ENEMY_SHOT_TANK_ROCKET: 
    {
      SDL_Rect destR;
      SDL_Rect srcR;
      destR.x = lroundf(pos.getX()) - spriteShadow->w / 16 - 10;
      destR.y = lroundf(pos.getY()) - spriteShadow->h / 2 + 10;
      destR.w = spriteShadow->w / 8;
      destR.h = spriteShadow->h;
      float angle = vel.getDirection() + 202.5;
      int idx = lroundf(angle) % 360;
      idx = idx / 45;
      srcR.x = idx * spriteShadow->w / 8;
      srcR.y = 0;
      srcR.w = spriteShadow->w / 8;
      srcR.h = spriteShadow->h;

      SDL_BlitSurface( spriteShadow, &srcR, screen, &destR );
      break;
    }
  default: break;
  }
}

void Shot::drawGroundShot(SdlCompat_AcceleratedSurface *screen) {
  switch (shotType) {
  case SHOT_KICK_ASS_ROCKET:
  case SHOT_HF_KICK_ASS_ROCKET:
  case SHOT_HELLFIRE:
    {
      SDL_Rect destR;
      SDL_Rect srcR;
      destR.x = lroundf(pos.getX()) - sprite->w / 2;
      destR.y = lroundf(pos.getY()) - sprite->h / 2;
      destR.w = sprite->w;
      destR.h = sprite->h;
      srcR.x = 0;
      srcR.y = 0;
      srcR.w = sprite->w;
      srcR.h = sprite->h;

      SDL_BlitSurface( sprite, &srcR, screen, &destR );
      break;
    }
  default: break;
  }
}
 
void Shot::drawGroundAirShot(SdlCompat_AcceleratedSurface *screen) {
  switch (shotType) {
  case SHOT_DUMBFIRE:
  case SHOT_DUMBFIRE_DOUBLE:
  case SHOT_HF_DUMBFIRE:
  case SHOT_HF_DUMBFIRE_DOUBLE:
    {
      SDL_Rect destR;
      SDL_Rect srcR;
      destR.x = lroundf(pos.getX()) - sprite->w / 16;
      destR.y = lroundf(pos.getY()) - sprite->h / 2;
      destR.w = sprite->w / 8;
      destR.h = sprite->h;
      // TODO: eight directions are outdated for dumbfire, but existent in the image
      float angle = vel.getDirection() + 202.5;
      int idx = lroundf(angle) % 360;
      idx = idx / 45;
      srcR.x = idx * 8;
      srcR.y = 0;
      srcR.w = 8;
      srcR.h = 8;

      SDL_BlitSurface( sprite, &srcR, screen, &destR );
      break;
    }
  case SHOT_MACHINE_GUN:
  case SHOT_HF_LASER:
    {
      SDL_Rect destR;
      destR.x = lroundf(pos.getX()) - sprite->w / 2;
      destR.y = lroundf(pos.getY()) - sprite->h / 2;
      destR.w = sprite->w;
      destR.h = sprite->h;
      SDL_BlitSurface( sprite, 0, screen, &destR );
      break;
    }
  case SHOT_ENERGY_BEAM:
    {
      SDL_Rect destR;
      destR.x = lroundf(pos.getX()) - sprite->w / 2;
      destR.y = lroundf(pos.getY()) - sprite->h / 2;
      destR.w = sprite->w;
      destR.h = sprite->h;
      SDL_BlitSurface( sprite, 0, screen, &destR );
      destR.x = lroundf(pos.getX()) - sprite->w / 2;
      destR.y = lroundf(pos.getY()) - sprite->h / 2;
      SDL_BlitSurface( sprite, 0, screen, &destR );
      break;
    }
  case SPECIAL_SHOT_HEATSEEKER:
    {
      SDL_Rect destR;
      SDL_Rect srcR;
      destR.x = lroundf(pos.getX()) - sprite->w / 16;
      destR.y = lroundf(pos.getY()) - sprite->h / 2;
      destR.w = sprite->w / 8;
      destR.h = sprite->h;

      float angle = vel.getDirection() + 202.5;
      int idx = lroundf(angle) % 360;
      idx = idx / 45;
      srcR.x = idx * 8;
      srcR.y = 0;
      srcR.w = 8;
      srcR.h = 8;

      SDL_BlitSurface( sprite, &srcR, screen, &destR );
      break;
    }
  case ENEMY_SHOT_TANK_ROCKET:
    {
      SDL_Rect destR;
      SDL_Rect srcR;
      destR.x = lroundf(pos.getX()) - sprite->w / 16;
      destR.y = lroundf(pos.getY()) - sprite->h / 2;
      destR.w = sprite->w / 8;
      destR.h = sprite->h;

      float angle = vel.getDirection() + 202.5;
      int idx = lroundf(angle) % 360;
      idx = idx / 45;
      srcR.x = idx * sprite->w / 8;
      srcR.y = 0;
      srcR.w = sprite->w / 8;
      srcR.h = sprite->h;

      SDL_BlitSurface( sprite, &srcR, screen, &destR );
      break;
    }

  default: break;
  }
}

void Shot::drawAirShot(SdlCompat_AcceleratedSurface *screen) {
  switch (shotType) {
  case SHOT_NORMAL:
  case SHOT_NORMAL_HEAVY:
  case SHOT_DOUBLE:
  case SHOT_DOUBLE_HEAVY:
  case SHOT_TRIPLE:
  case SHOT_HF_NORMAL:
  case SHOT_HF_DOUBLE:
  case SHOT_HF_TRIPLE:
  case SHOT_HF_QUATTRO:
  case SHOT_HF_QUINTO:
  case SPECIAL_SHOT_NUKE:
  case ENEMY_SHOT_NORMAL:
    {
      SDL_Rect destR;
      destR.x = lroundf(pos.getX()) - sprite->w / 2;
      destR.y = lroundf(pos.getY()) - sprite->h / 2;
      destR.w = sprite->w;
      destR.h = sprite->h;
      SDL_BlitSurface( sprite, 0, screen, &destR );
      break;
    }
  default: break;
  }
}

ShotTypes Shot::getShotType() {
  return shotType;
}


void Shot::generateSmokePuff( int dT) {
  if ( ! generatesSmokePuffs ) return;

  timeToNextSmokePuff -= dT;
  if ( timeToNextSmokePuff < 0 ) {
    Vector2D relPos = -vel;
    relPos.setLength( sprite->h / 2 );
    if ( shotType == SHOT_HELLFIRE ||
	 shotType == ENEMY_SHOT_TANK_ROCKET ) {

      smokePuffs->addSmokePuff( pos + relPos, 
				vel * SMOKE_PUFF_VELOCITY_FACTOR,
				SMOKE_PUFF_MEDIUM );
      timeToNextSmokePuff += SMOKE_PUFF_DELAY_TO_NEXT_PUFF[ SMOKE_PUFF_MEDIUM ];
    } else {
      smokePuffs->addSmokePuff( pos + relPos, 
				vel * SMOKE_PUFF_VELOCITY_FACTOR,
				SMOKE_PUFF_SMALL );
      timeToNextSmokePuff += SMOKE_PUFF_DELAY_TO_NEXT_PUFF[ SMOKE_PUFF_SMALL ];
    }
  }
}

			      
  

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
#include <math.h>
#include "global.h"
#include "enemys.h"
#include "enemy.h"
#include "racers.h"
#include "racer.h"
#include "wrecks.h"
#include "wreck.h"
#include "surfaceDB.h"
#include "formation.h"
#include "banners.h"
#include "options.h"

Enemys::Enemys() {
  timeToNextEnemy = GENERATE_ENEMY_DELAY + (rand() % (GENERATE_ENEMY_RAND_DELAY+1));
  timeToNextFormation = 
    GENERATE_FORMATION_DELAY + (rand() % (GENERATE_FORMATION_RAND_DELAY + 1));
  bossActive = 0;
  enemysKilled = 0;
  enemysGenerated = 0;
}

Enemys::~Enemys() {
  vector<Enemy *>::iterator i;
  for (i = enemys.begin(); i != enemys.end(); ++i) {
    delete *i;
  }
  vector<Formation *>::iterator f;
  for (f = formations.begin(); f != formations.end(); ++f) {
    delete *f;
  }
}

void Enemys::addEnemy( Enemy *newEnemy ) {
  if ( newEnemy ) {
    enemys.push_back( newEnemy );
    enemysGenerated++;
  }
}

bool Enemys::minibossDead(){
  for ( unsigned int i = 0; i < enemys.size(); i++ ) {
    if ( enemys[ i ]->getType() == BOSS_2 ) return false;
  }
  return true;
}
  
bool Enemys::bossDead() {
  bool dead = true;
  for ( unsigned int i = 0; i < enemys.size(); i++ ) {
    if ( enemys[ i ]->getType() >= NR_ENEMY_TYPES_NORMAL ) {
      dead = false;
      break;
    }
  }
  if (dead) bossActive = 0;
  return dead;
}


void Enemys::bossTime( int bossNr ) {
  bossActive = bossNr;
  Enemy *boss;
  Wreck *wreck;
  switch (bossNr) {
  // Final boss
  case 1:
    {
      wreck = new Wreck( Vector2D( SCREEN_WIDTH / 2 , -120 ), WRECK_BOSS_1_BACKGROUND );
      wrecks->addWreck( wreck );

      boss = new Enemy( Vector2D( 321, -102 ), Vector2D( 0, SCROLL_SPEED ), BOSS_1_MAIN_GUN );
      addEnemy( boss );
      boss = new Enemy( Vector2D( 248, -73 ), Vector2D( 0, SCROLL_SPEED ), 
			  BOSS_1_SHOT_BATTERY_LEFT );
      addEnemy( boss );
      boss = new Enemy( Vector2D( 393, -73 ), Vector2D( 0, SCROLL_SPEED ), 
			  BOSS_1_SHOT_BATTERY_RIGHT );
      addEnemy( boss );
      boss = new Enemy( Vector2D( 50, -85 ), Vector2D( 0, SCROLL_SPEED ), 
			  BOSS_1_ROCKET_LAUNCHER );
      addEnemy( boss );
      boss = new Enemy( Vector2D( 590, -85 ), Vector2D( 0, SCROLL_SPEED ), 
			  BOSS_1_ROCKET_LAUNCHER );
      addEnemy( boss );
      break;
    }
  // Miniboss  
  case 2: 
    {
      boss = new Enemy( Vector2D( SCREEN_WIDTH / 2 , -100 ),
        Vector2D( 0, SCROLL_SPEED ), BOSS_2 );
      addEnemy( boss );
      break;
    }
  default: 
    {
      cout << "Enemys::bossTime: unexpected bossNr: " << bossNr << endl;
      break;
    }
  }
}


void Enemys::generateEnemys( int dT ) {
  if ( bossActive != 0 ) return;


  timeToNextFormation -= dT;
  if ( timeToNextFormation < 0 ) {
    FormationTypes whichFormation = (FormationTypes)(rand() % NR_FORMATION_TYPES);
    int flags = 0;
    if ( rand() % 2 == 0 ) flags = flags | FORMATION_CHANGE_ON_KILL;
    if ( rand() % 2 == 0 ) { 
      flags = flags | FORMATION_CHANGE_SPONTANEOUS;
      if ( rand() % 2 == 0 ) flags = flags | FORMATION_CHANGE_OFTEN;
      else flags = flags | FORMATION_CHANGE_SELDOM;
    }
    FormationShotPatterns shotPattern = 
      (FormationShotPatterns)getRandValue( FORMATION_SP_CHANCES, NR_FORMATION_SP );
    
    Formation *newV = 
      new Formation( whichFormation,
		     Vector2D( 80 + (rand() % (SCREEN_WIDTH  - 160)), -100 ),
		     Vector2D( 0, 40 ),
		     FORMATION_MAX_NR_ENEMYS[ whichFormation ] - (rand() % 3),
		     (FormationEnemySets)(rand() % NR_FORMATION_ENEMY_SETS),
		     flags, shotPattern );
    formations.push_back( newV );

    timeToNextFormation = 
      GENERATE_FORMATION_DELAY + (rand() % (GENERATE_FORMATION_RAND_DELAY + 1));
  }
  

  timeToNextEnemy -= dT;
  if ( timeToNextEnemy < 0 ) {
    int enemyType = getRandValue( ENEMY_APPEAR_CHANCES, NR_ENEMY_TYPES_NORMAL );
    Enemy *newOne = 0;
    switch ((EnemyTypes)enemyType) {
    case FIGHTER:
      {
	bool collides;
	float xpos;
	for ( int i = 0; i < 10; i++ ) {
	  collides = false;
	  xpos =  20 + rand() % (SCREEN_WIDTH - 40);
	  // check if collides with a formation
	  for ( unsigned int f = 0; f < formations.size(); f++ ) {
	    Vector2D formationCenter = formations[ f ]->getCenter();
	    collides =
	      formationCenter.getY() < 150 && 
	      fabs(xpos - formationCenter.getX()) < 150;
	    if (collides) break;
	  }
	}
	if ( !collides ) {
	  newOne = new Enemy( Vector2D( xpos, -20 ), // position
			      Vector2D( (rand() % 5) - 2, rand() % 7 + 3 ) * 10, // velocity
			      FIGHTER );
	}
	break;
      }
    case BOMBER:
      {
	bool collides;
	float xpos;
	for ( int i = 0; i < 10; i++ ) {
	  collides = false;
	  xpos =  20 + rand() % (SCREEN_WIDTH - 40);
	  // check if collides with a formation
	  for ( unsigned int f = 0; f < formations.size(); f++ ) {
	    Vector2D formationCenter = formations[ f ]->getCenter();
	    collides =
	      formationCenter.getY() < 150 && 
	      fabs(xpos - formationCenter.getX()) < 150;
	    if (collides) break;
	  }
	}
	if ( !collides ) {
	  newOne = new Enemy( Vector2D( xpos, -20 ), // position
			      Vector2D( (rand() % 5) - 2, rand() % 3 + 3 ) * 10, // velocity
			      BOMBER );
	}
	break;
      }
    case TANK:
      {
// 	SdlCompat_AcceleratedSurface *spriteTank = surfaceDB.loadSurface( FN_ENEMY_TANK );
// 	SdlCompat_AcceleratedSurface *spriteTankWreck = surfaceDB.loadSurface( FN_WRECK_TANK );
	string fn1, fn2;
	levelConf->getStr( LVL_ENEMY_TANK, fn1 );
	levelConf->getStr( LVL_WRECK_TANK, fn2 );
	SdlCompat_AcceleratedSurface *spriteTank = surfaceDB.loadSurface( fn1 );
	SdlCompat_AcceleratedSurface *spriteTankWreck = surfaceDB.loadSurface( fn2 );
	int halfWidthTank = spriteTank->w / 2;
	int halfHeightTank = spriteTank->h / 2;
	int halfWidthTankWreck = spriteTankWreck->w / 2;
	int halfHeightTankWreck = spriteTankWreck->h / 2;

	int xPos;
	
	for ( int i = 0; i < 10; i++ ) {
	  bool collides = false;
	  xPos =  halfWidthTank + rand() % (SCREEN_WIDTH - spriteTank->w);
	  // check if collides with another tank
	  for ( unsigned int t = 0; t < enemys.size(); t++ ) {
	    if ( enemys[ t ]->getType() == TANK ) {
	      Vector2D enemyTankPos = enemys[ t ]->getPos();
	      collides = 
		!(enemyTankPos.getX() - halfWidthTank > xPos + halfWidthTank) &&
		!(enemyTankPos.getX() + halfWidthTank < xPos - halfWidthTank) &&
		!(enemyTankPos.getY() - halfHeightTank > 0);
	      if (collides) break;
	    }
	  }
	  if (!collides) {
	    // check if collides with an old wreck
	    for ( unsigned int w = 0; w < wrecks->getNrWrecks(); w++ ) {
	      if ( wrecks->getWreck( w )->getType() == WRECK_TANK ) {
		Vector2D wreckPos = wrecks->getWreck(w)->getPos();
		collides = 
		  !(wreckPos.getX() - halfWidthTankWreck > xPos + halfWidthTank) &&
		  !(wreckPos.getX() + halfWidthTankWreck < xPos - halfWidthTank) &&
		  !(wreckPos.getY() - halfHeightTankWreck > 0);
		if (collides) break;
	      }
	    }
	  }
	  
	  // no collision -> generate this enemy
	  if ( !collides ) {
	    // the tank and the background have to be at the same pixel-fraction 
	    // to avoid a waggle-effect
	    float correlateToBackground = actBackgroundPos - truncf(actBackgroundPos);
	    newOne = new Enemy( Vector2D( xPos, -halfHeightTank - correlateToBackground ),
				Vector2D (0, SCROLL_SPEED), // tanks are not moving
				TANK );
	    break;
	  }
	}
	
	break;
      }
    default:
      {
	cout << "generateEnemys(): unexpected enemyType: " << enemyType << endl;
	break;
      }
    }
    addEnemy( newOne );
    // +1 for security
    timeToNextEnemy = GENERATE_ENEMY_DELAY + (rand() % (GENERATE_ENEMY_RAND_DELAY+1));
  }
}

void Enemys::updateEnemys( int dT ) {
  vector<Formation *>::iterator f;
  for (f = formations.begin(); f != formations.end(); ++f) {
    (*f)->update( dT );
  }
  vector<Enemy *>::iterator i;
  for (i = enemys.begin(); i != enemys.end(); ++i) {
    (*i)->update( dT );
  }
}

void Enemys::doNukeDamage() {
  vector<Enemy *>::iterator i;
  for (i = enemys.begin(); i != enemys.end(); ++i) {
    (*i)->doDamage( SPECIAL_SHOT_NUKE, -1 );
  }
}
  

void Enemys::deleteExpiredEnemys() {
  unsigned int i = 0;
  while ( i < enemys.size() ) {
    if ( enemys[i]->isExpired() ) {
      if ( enemys[i]->isDead() ) {
	enemysKilled++;
	if ( arcadeGame ) {
	  if ( enemysKilled % 50 == 0 ) {
	    banners->addBanner( BANNER_ENEMYS_KILLED,
				BANNER_MODE_RANDOM,
				ARCADE_BONUS_FOR_ENEMYS_KILLED );
	    racers->receivePointsArcade( ARCADE_POINTS_FOR_ENEMYS_KILLED );
	  }	  
	}
      }
      for ( unsigned int f = 0; f < formations.size(); f++ ) {
	formations[f]->enemyKilled( enemys[i] );
      }
      delete enemys[i];
      enemys.erase(enemys.begin() + i);
    } else {
      i++;
    }
  }
  unsigned int f = 0;
  while ( f < formations.size() ) {
    if ( formations[f]->isExpired() ) {
      // assert, that the formation is not deleted, because all
      // enemys in the formation flew out of the screen
      if ( arcadeGame && formations[f]->getCenter().getY() < SCREEN_HEIGHT + 400 ) {
	banners->addBanner( (BannerTexts)(rand() % NR_BANNER_TEXTS),
			    BANNER_MODE_RANDOM, 
			    ARCADE_BONUS_FOR_FORMATION_DESTRUCTION );
	racers->receivePointsArcade( ARCADE_POINTS_FOR_FORMATION_DESTRUCTION );
      }
      delete formations[f];
      formations.erase(formations.begin() + f);
    } else {
      f++;
    }
  }
}

void Enemys::drawGroundEnemys(SdlCompat_AcceleratedSurface *screen) {
  vector<Enemy *>::iterator i;
  for (i = enemys.begin(); i != enemys.end(); ++i) {
    (*i)->drawGroundEnemy(screen);
  }
}

void Enemys::drawAirEnemys(SdlCompat_AcceleratedSurface *screen) {
  vector<Enemy *>::iterator i;
  for (i = enemys.begin(); i != enemys.end(); ++i) {
    (*i)->drawAirEnemy(screen);
  }
}
  
void Enemys::drawShadows(SdlCompat_AcceleratedSurface *screen) {
  vector<Enemy *>::iterator i;
  for (i = enemys.begin(); i != enemys.end(); ++i) {
    (*i)->drawShadow(screen);
  }
}

void Enemys::drawBossStats( SdlCompat_AcceleratedSurface *screen ) {
  for ( unsigned int i = 0; i < enemys.size(); i++ ) {
    if ( enemys[ i ]->getType() >= NR_ENEMY_TYPES_NORMAL ) {
      enemys[ i ]->drawStats( screen );
    }
  }
}

void Enemys::drawAllStats( SdlCompat_AcceleratedSurface *screen ) {
  for ( unsigned int i = 0; i < enemys.size(); i++ ) {
    enemys[ i ]->drawStats( screen );
  }
}

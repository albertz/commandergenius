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
#include "enemy.h"
#include "SDL.h"
#include "surfaceDB.h"
#include "boundingBox.h"
#include "mixer.h"
#include "shots.h"
#include "shot.h"
#include "racers.h"
#include "racer.h"
#include "items.h"
#include "item.h"
#include "wrecks.h"
#include "wreck.h"
#include "global.h"
#include "explosions.h"
#include "explosion.h"
#include "options.h"

Enemy::Enemy( Vector2D pos, Vector2D vel, EnemyTypes whichEnemyType, 
	      bool isInFormation, bool fireByFormation ) {

  this->isInFormation = isInFormation;
  this->fireByFormation = fireByFormation;
  enemyType = whichEnemyType;
  hitpoints = ENEMY_HITPOINTS[ enemyType ];

  this->pos = pos;
  this->vel = vel;
  relTargetPos = Vector2D(0,0);
  
  switch ( enemyType ) {
  case FIGHTER: 
    {
      string fn = LVL_ENEMY_FIGHTER;
      levelConf->getStr( LVL_ENEMY_FIGHTER, fn );
      spriteEnemy = surfaceDB.loadSurface( fn );
      
      fn = LVL_ENEMY_FIGHTER_SHADOW;
      levelConf->getStr( LVL_ENEMY_FIGHTER_SHADOW, fn );
      spriteShadow = surfaceDB.loadSurface( fn, true );
      break;
    }
  case BOMBER:
    {
      string fn = LVL_ENEMY_BOMBER;
      levelConf->getStr( LVL_ENEMY_BOMBER, fn );
      spriteEnemy = surfaceDB.loadSurface( fn );

      fn = LVL_ENEMY_BOMBER_SHADOW;
      levelConf->getStr( LVL_ENEMY_BOMBER_SHADOW, fn );
      spriteShadow = surfaceDB.loadSurface( fn, true );
      break;
    }
  case TANK:
    {
      string fn = LVL_ENEMY_TANK;
      levelConf->getStr( LVL_ENEMY_TANK, fn );
      spriteEnemy = surfaceDB.loadSurface( fn );
      break;
    }

  case BOSS_1_MAIN_GUN:
    {
      string fn = LVL_ENEMY_BOSS_1_MAIN_GUN;
      levelConf->getStr( LVL_ENEMY_BOSS_1_MAIN_GUN, fn );
      spriteEnemy = surfaceDB.loadSurface( fn );
      break;
    }
  case BOSS_1_ROCKET_LAUNCHER:
    {
      string fn = LVL_ENEMY_BOSS_1_ROCKET_LAUNCHER;
      levelConf->getStr( LVL_ENEMY_BOSS_1_ROCKET_LAUNCHER, fn );
      spriteEnemy = surfaceDB.loadSurface( fn );
      break;
    }
  case BOSS_1_SHOT_BATTERY_LEFT:
    {
      string fn = LVL_ENEMY_BOSS_1_SHOT_BATTERY_LEFT;
      levelConf->getStr( LVL_ENEMY_BOSS_1_SHOT_BATTERY_LEFT, fn );
      spriteEnemy = surfaceDB.loadSurface( fn );
      break;
    }
  case BOSS_1_SHOT_BATTERY_RIGHT:
    {
      string fn = LVL_ENEMY_BOSS_1_SHOT_BATTERY_RIGHT;
      levelConf->getStr( LVL_ENEMY_BOSS_1_SHOT_BATTERY_RIGHT, fn );
      spriteEnemy = surfaceDB.loadSurface( fn );
      break;
    }
  case BOSS_2:
    {
      spriteEnemy = surfaceDB.loadSurface( FN_ENEMY_BOSS_2 );
      spriteShadow = surfaceDB.loadSurface( FN_ENEMY_BOSS_2_SHADOW, true );
      boss2PointReached = false;
      boss2TargetPos = Vector2D(SCREEN_WIDTH / 2, 100);
      break;
    }

  default: 
    {
      string fn = LVL_ENEMY_FIGHTER;
      levelConf->getStr( LVL_ENEMY_FIGHTER, fn );
      spriteEnemy = surfaceDB.loadSurface( fn );
      break;
    }
  }

  boundingBox = new BoundingBox( lroundf(pos.getX() - spriteEnemy->w * 0.45),
				 lroundf(pos.getY() - spriteEnemy->h * 0.45),
				 lroundf(spriteEnemy->w * 0.9),
				 lroundf(spriteEnemy->h * 0.9) );
  nextShotPrimary = rand() % (ENEMY_RAND_WAIT_PRIMARY[ enemyType ]+1);
  nextShotSecondary = rand() % (ENEMY_RAND_WAIT_SECONDARY[ enemyType ]+1);

  sndShotPrimary = mixer.loadSample( FN_SOUND_SHOT_PRIMARY );
  sndShotSecondary = mixer.loadSample( FN_SOUND_SHOT_SECONDARY );
}

Enemy::~Enemy() {
  delete boundingBox;
}


bool Enemy::isExpired() {
  return ( hitpoints <= 0 || 
	   pos.getY() < -500 ||
	   pos.getY() > SCREEN_HEIGHT + 500 ||
	   pos.getX() < -500 || 
	   pos.getX() > SCREEN_WIDTH + 500 );
}


void Enemy::expire() {
  hitpoints = -1;
}

Circle Enemy::getBoundingCircle() {
  return Circle( pos, min(spriteEnemy->w / 2, spriteEnemy->h / 2) );
}

BoundingBox *Enemy::getBoundingBox() {
  return boundingBox;
}

bool Enemy::collidesWith( const Vector2D &shotPosOld, const Vector2D &shotPosNew ) {
  return boundingBox->overlaps(shotPosOld, shotPosNew);
}

bool Enemy::collidesWith( BoundingBox *box ) {
  return boundingBox->overlaps( box );
}

bool Enemy::collidesWith( const Circle &circle ) {
  return boundingBox->overlaps( circle );
}

bool Enemy::collidesWithAsCircle( const Circle &circle ) {
  return ( circle.getRadius() + spriteEnemy->w / 2 > circle.getCenter().distanceTo( pos ) );
}

bool Enemy::collidesWithAsCircle( BoundingBox *box ) {
  return ( box->overlaps( Circle( pos, min( spriteEnemy->h / 2, spriteEnemy->w / 2 ) ) ) );
}


void Enemy::update( int dT ) {
  move( dT );
  if ( !fireByFormation ) {
    shootPrimary( dT );
    shootSecondary( dT );
  }
}

void Enemy::move( int dT ) {
  switch ( enemyType ) {
  case FIGHTER:
  case BOMBER:
    {
      if ( scrollingOn ) pos += vel * dT / 1000.0;
      else pos += (vel - SCROLL_SPEED) * dT / 1000.0;
      if ( isInFormation && relTargetPos != Vector2D(0,0) ) {
        Vector2D addMovement = 
          Vector2D( 40, relTargetPos.getDirection(), POLAR ) * dT / 1000.0;
        if ( addMovement.getLength() > relTargetPos.getLength() ) {
          addMovement.setLength( relTargetPos.getLength() );
        }
        pos += addMovement;
        relTargetPos -= addMovement;
      }
      updateBoundingBox();
      break;
    }
  case TANK: {
    if ( scrollingOn ) pos += vel * dT / 1000.0;
    updateBoundingBox();
    break;
  }
  case BOSS_1_MAIN_GUN:
  case BOSS_1_ROCKET_LAUNCHER:
  case BOSS_1_SHOT_BATTERY_LEFT:
  case BOSS_1_SHOT_BATTERY_RIGHT: {
    if ( scrollingOn ) {
      pos += Vector2D( 0, SCROLL_SPEED * dT / 1000.0 );
      updateBoundingBox();
      if ( pos.getY() >= BOSS_1_END_Y ) {
        scrollingOn = false;
        pos.setY( BOSS_1_END_Y );
      }
    }  
    break;
  }    
  case BOSS_2: {
    if ( boss2PointReached ) {
      boss2TargetPos = Vector2D( (rand() % (SCREEN_WIDTH - spriteEnemy->w)) + spriteEnemy->w / 2, rand() % 100 + spriteEnemy->h );
      boss2PointReached = false;
    } else {
      pos += (boss2TargetPos - pos) / 50;
    }
    if ( pos.distanceTo(boss2TargetPos) < 15.0 ) boss2PointReached = true;
    updateBoundingBox();
    break;
  }
  default: cout << "enemys.cc::move(): unknown enemyType" << endl; break;
  }
}

void Enemy::updateBoundingBox() {
  boundingBox->moveUpperBound( lroundf(pos.getY() - spriteEnemy->h * 0.45) );
  boundingBox->moveLeftBound( lroundf(pos.getX() - spriteEnemy->w * 0.45) );
}


void Enemy::firePrimary() {
  switch (enemyType) {
  case FIGHTER: 
    {
      Shot *shot = 
	new Shot( ENEMY_SHOT_NORMAL, 666, 
		  pos + Vector2D( 0, spriteEnemy->h / 2 ),
		  90 );
      shots->addShot( shot );
      mixer.playSample( sndShotPrimary, 0 );
      break;
    }
  case BOMBER:
    {
      Shot *shot = 
	new Shot( ENEMY_SHOT_NORMAL, 666,
		  pos + Vector2D( -7, spriteEnemy->h / 2 ),
		  100 );
      shots->addShot( shot );
      shot = 
	new Shot( ENEMY_SHOT_NORMAL, 666,
		  pos + Vector2D( +7, spriteEnemy->h / 2 ),
		  80 );
      shots->addShot( shot );
      mixer.playSample( sndShotPrimary, 0 );
      break;
    }
  case TANK:
    {
      Shot *shot = 
	new Shot( ENEMY_SHOT_NORMAL, 666,
		  pos,
		  (rand() % 360) - 180 );
      shots->addShot( shot );
      mixer.playSample( sndShotPrimary, 0 );
      break;
    }
  case BOSS_1_MAIN_GUN:
    {
      Shot *shot = 
	new Shot( ENEMY_SHOT_NORMAL, 666,
		  pos,
		  (rand() % 20) + 80 );
      shots->addShot( shot );
      mixer.playSample( sndShotPrimary, 0 );
      break;
    }
  case BOSS_1_SHOT_BATTERY_LEFT:
    {
      Shot *shot = 
	new Shot( ENEMY_SHOT_NORMAL, 666,
		  pos,
		  (rand() % 120) + 30 );
      shots->addShot( shot );
      mixer.playSample( sndShotPrimary, 0 );
      break;
    }
  case BOSS_1_SHOT_BATTERY_RIGHT:
    {
      Shot *shot = 
	new Shot( ENEMY_SHOT_NORMAL, 666,
		  pos,
		  (rand() % 120) + 30 );
      shots->addShot( shot );
      mixer.playSample( sndShotPrimary, 0 );
      break;
    }
  case BOSS_1_ROCKET_LAUNCHER:
    {
      unsigned int racerIdx = rand() % racers->getNrRacers();
      float angle = (racers->getRacer( racerIdx )->getPos() - pos).getDirection();
      Shot *shot = 
	new Shot( ENEMY_SHOT_TANK_ROCKET, 666, pos, angle );
      shots->addShot( shot );
      mixer.playSample( sndShotSecondary, 0 );
      break;
    }
  case BOSS_2:
    {
      unsigned int racerIdx = rand() % racers->getNrRacers();
      float angle = (racers->getRacer( racerIdx )->getPos() - pos).getDirection();
      Shot *shot = new Shot( ENEMY_SHOT_NORMAL, 666, pos, angle );
      shots->addShot( shot );
      mixer.playSample( sndShotPrimary, 0 );
      break;
    }
  default:
    {
      break;
    }
  }
}


void Enemy::shootPrimary( int dT ) {
  nextShotPrimary -= dT;
  if ( nextShotPrimary < 0 ) {
    firePrimary();
    nextShotPrimary = 
      (rand() % (ENEMY_RAND_WAIT_PRIMARY[ enemyType ]+1)) + ENEMY_COOLDOWN_PRIMARY[ enemyType ];
  }
}


void Enemy::fireSecondary() {
  switch (enemyType) {
  case TANK:
    {
      unsigned int racerIdx = rand() % racers->getNrRacers();
      float angle = (racers->getRacer( racerIdx )->getPos() - pos).getDirection();
      
      Shot *shot = 
	new Shot( ENEMY_SHOT_TANK_ROCKET, 666, pos, angle );
      shots->addShot( shot );
      mixer.playSample( sndShotSecondary, 0 );
      break;
    }
  case BOSS_2:
    {
      unsigned int racerIdx = rand() % racers->getNrRacers();
      float angle = (racers->getRacer( racerIdx )->getPos() - pos).getDirection();
      Shot *shot = new Shot( ENEMY_SHOT_TANK_ROCKET, 666, pos - Vector2D(-80,0), angle );
      shots->addShot( shot );
      shot = new Shot( ENEMY_SHOT_TANK_ROCKET, 666, pos - Vector2D(+80,0), angle );
      shots->addShot( shot );
      mixer.playSample( sndShotSecondary, 0 );
      break;
    }
  default:
    {
      break;
    }
  }
}

void Enemy::shootSecondary( int dT ) {
  nextShotSecondary -= dT;
  if ( nextShotSecondary < 0 ) {
    fireSecondary();
    nextShotSecondary = 
      (rand() % (ENEMY_RAND_WAIT_SECONDARY[ enemyType ]+1)) + 
      ENEMY_COOLDOWN_SECONDARY[ enemyType ];
  }
}


void Enemy::doDamage( ShotTypes shotType, int fromWhichPlayer ) {
  bool allreadyDead = isExpired();

  switch (shotType) {
  case SHOT_NORMAL: hitpoints -= DAMAGE_SHOT_NORMAL; break;
  case SHOT_NORMAL_HEAVY: hitpoints -= DAMAGE_SHOT_NORMAL_HEAVY; break;
  case SHOT_DOUBLE: hitpoints -= DAMAGE_SHOT_DOUBLE; break;
  case SHOT_DOUBLE_HEAVY: hitpoints -= DAMAGE_SHOT_DOUBLE_HEAVY; break;
  case SHOT_TRIPLE: hitpoints -= DAMAGE_SHOT_TRIPLE; break;
    
  case SHOT_HF_NORMAL: hitpoints -= DAMAGE_SHOT_HF_NORMAL; break;
  case SHOT_HF_DOUBLE: hitpoints -= DAMAGE_SHOT_HF_DOUBLE; break;
  case SHOT_HF_TRIPLE: hitpoints -= DAMAGE_SHOT_HF_TRIPLE; break;
  case SHOT_HF_QUATTRO: hitpoints -= DAMAGE_SHOT_HF_QUATTRO; break;
  case SHOT_HF_QUINTO: hitpoints -= DAMAGE_SHOT_HF_QUINTO; break;
    
  case SHOT_DUMBFIRE: hitpoints -= DAMAGE_SHOT_DUMBFIRE; break;
  case SHOT_DUMBFIRE_DOUBLE: hitpoints -= DAMAGE_SHOT_DUMBFIRE_DOUBLE; break;
  case SHOT_KICK_ASS_ROCKET: hitpoints -= DAMAGE_SHOT_KICK_ASS_ROCKET; break;
  case SHOT_HELLFIRE: hitpoints -= DAMAGE_SHOT_HELLFIRE; break;
  case SHOT_MACHINE_GUN: hitpoints -= DAMAGE_SHOT_MACHINE_GUN; break;
  case SHOT_ENERGY_BEAM: hitpoints -= DAMAGE_SHOT_ENERGY_BEAM; break;

  case SHOT_HF_DUMBFIRE: hitpoints -= DAMAGE_SHOT_HF_DUMBFIRE; break;
  case SHOT_HF_DUMBFIRE_DOUBLE: hitpoints -= DAMAGE_SHOT_HF_DUMBFIRE_DOUBLE; break;
  case SHOT_HF_KICK_ASS_ROCKET: hitpoints -= DAMAGE_SHOT_HF_KICK_ASS_ROCKET; break;
  case SHOT_HF_LASER: hitpoints -= DAMAGE_SHOT_HF_LASER; break;

  case SPECIAL_SHOT_HEATSEEKER: hitpoints -= DAMAGE_SPECIAL_SHOT_HEATSEEKER; break;
  case SPECIAL_SHOT_NUKE: hitpoints -= DAMAGE_SPECIAL_SHOT_NUKE; break;
  default: 
    {
      cout << "Enemy::doDamage: unexpected shotType: " << shotType << endl;
      break;
    }
  }

  if ( enemyType < NR_ENEMY_TYPES_NORMAL ) {
    // the enemy just died -> explode, generate item, 
    if ( (!allreadyDead) && isExpired() ) {
      // the player gets points, who did the final shot
      if ( 0 <= fromWhichPlayer && fromWhichPlayer < (int)racers->getNrRacers() ) {
        racers->getRacer( fromWhichPlayer )->receivePoints( ENEMY_POINTS_FOR_DEST[ enemyType ]);
      }
      // explode
      Explosion *newExplosion = 
        new Explosion( FN_EXPLOSION_ENEMY, pos, vel, EXPLOSION_NORMAL_AIR );
      explosions->addExplosion( newExplosion );
      // generate wreck
      Wreck *newWreck = new Wreck( pos, WRECK_FOR_ENEMYTYPE[ enemyType ]);
      wrecks->addWreck( newWreck );
      // generate item
      if ( rand() % ENEMY_DIES_ITEM_APPEAR_CHANCE[ enemyType ] == 0 ) {
        items->generateItemNow( pos, vel / 10.0 );
      }
    }
  } 
  // it's a boss!!!
  else {
    if ( (!allreadyDead) && isExpired() ) {
      // explode
      Explosion *newExplosion;
      for(int i = 0; i < 10; i++){
        newExplosion = new Explosion(
          FN_EXPLOSION_ENEMY,
          pos + Vector2D(rand() % spriteEnemy->w - spriteEnemy->w / 2, rand() % spriteEnemy->h - spriteEnemy->h / 2),
          vel + Vector2D(rand() % 100 - 50, rand() % 100 - 50) / 5.0,
          EXPLOSION_NORMAL_GROUND );
        explosions->addExplosion( newExplosion );
      }

      // generate wreck
      Wreck *newWreck = new Wreck( pos, WRECK_FOR_ENEMYTYPE[ enemyType ]);
      wrecks->addWreck( newWreck );      
    }
  }  
}


void Enemy::drawGroundEnemy( SDL_Surface *screen ) {
  if ( ENEMY_FLYING[ enemyType ] ) return;

  SDL_Rect destR;

  destR.x = lroundf(pos.getX()) - spriteEnemy->w / 2;
  destR.y = lroundf(pos.getY()) - spriteEnemy->h / 2;
  destR.w = spriteEnemy->w;
  destR.h = spriteEnemy->h;

  SDL_BlitSurface( spriteEnemy, 0, screen, &destR );
}

void Enemy::drawAirEnemy( SDL_Surface *screen ) {
  if ( !ENEMY_FLYING[ enemyType ] ) return;

  SDL_Rect destR;

  destR.x = lroundf(pos.getX()) - spriteEnemy->w / 2;
  destR.y = lroundf(pos.getY()) - spriteEnemy->h / 2;
  destR.w = spriteEnemy->w;
  destR.h = spriteEnemy->h;

  SDL_BlitSurface( spriteEnemy, 0, screen, &destR );
}

void Enemy::drawShadow( SDL_Surface *screen ) {
  if ( !ENEMY_FLYING[ enemyType ] ) return;

  SDL_Rect destR;

  destR.x = lroundf(pos.getX()) - spriteShadow->w / 2 - ENEMY_FLYING_HEIGHT[ enemyType ];
  destR.y = lroundf(pos.getY()) - spriteShadow->h / 2 + ENEMY_FLYING_HEIGHT[ enemyType ];
  destR.w = spriteShadow->w;
  destR.h = spriteShadow->h;

  SDL_BlitSurface( spriteShadow, 0, screen, &destR );
}


void Enemy::drawStats( SDL_Surface *screen ) {
  // draw status of the bosses
  float pixPerHP = spriteEnemy->w / (float)(ENEMY_HITPOINTS[ enemyType ]);
  SDL_Rect destR;
  // draw damage
  destR.x = lroundf(pos.getX()) - spriteEnemy->w / 2;
  destR.y = lroundf(pos.getY()) - spriteEnemy->h / 2 - 4;
  float damageToDraw = min( (float)ENEMY_HITPOINTS[ enemyType ] / 2, hitpoints );
  destR.w = lroundf(pixPerHP * damageToDraw);
  destR.h = 3;
  SDL_FillRect( screen, &destR, SDL_MapRGB(screen->format, 255, 0, 0) );
  // draw shields
  destR.x = lroundf(pos.getX());
  destR.y = lroundf(pos.getY()) - spriteEnemy->h / 2 - 4;
  float shieldToDraw = min( (float)ENEMY_HITPOINTS[ enemyType ] / 2, 
			    hitpoints - ENEMY_HITPOINTS[ enemyType ] / 2 );
  if ( shieldToDraw < 1 ) destR.w = 0;
  else destR.w = lroundf(pixPerHP * shieldToDraw);
  destR.h = 3;
  SDL_FillRect(screen, &destR, SDL_MapRGB(screen->format, 0, 255, 0) );
}

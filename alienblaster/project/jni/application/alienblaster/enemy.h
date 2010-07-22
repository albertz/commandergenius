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
#ifndef ENEMY_HH
#define ENEMY_HH

#include "SDL.h"
#include "geometry.h"
#include <string>
#include "shot.h"
#include "global.h"

class Shot;
class Shots;
class BoundingBox;

class Enemy {
  private:
  SdlCompat_AcceleratedSurface *spriteEnemy;
  SdlCompat_AcceleratedSurface *spriteShadow;
  // for collision with racers or shots. 
  // A rectangle with racersize * 0.9 is used.
  BoundingBox *boundingBox;

  // Movement-System
  Vector2D pos; // absolute position
  Vector2D vel; // the velocity vector

  Vector2D relTargetPos;

  // needed for cooldown
  int nextShotPrimary;
  int nextShotSecondary;

  int sndShotPrimary;
  int sndShotSecondary;

  float hitpoints;

  EnemyTypes enemyType;

  bool isInFormation;
  bool fireByFormation;
  bool boss2PointReached;
  Vector2D boss2TargetPos;

  public:
  Enemy( Vector2D pos, Vector2D vel, EnemyTypes whichEnemyType, bool isInFormation=false,
    bool fireByFormation=false );
  ~Enemy();

  bool isExpired();
  void expire();
  bool isDead() { return ( hitpoints <= 0 ); }
  
  EnemyTypes getType() { return enemyType; }

  void update( int dT );

  void firePrimary();
  void fireSecondary();

  // The enemy will make an additional movement of newRelTargetPos.
  // This is used, when the enemy is part of a formation and should move
  // to another position in the formation.
  void setNewRelTargetPos( Vector2D newRelTargetPos ) { relTargetPos = newRelTargetPos; }

  void setPos(Vector2D newPos);
  Vector2D getPos() { return pos; }
  Vector2D getVel() { return vel; }
  Vector2D setVel(Vector2D newVel); // returns old vel


  private:
  // fire the Guns!
  void shootPrimary( int dT );
  void shootSecondary( int dT );
  // moves the enemy according to his velocity
  void move( int dT );
  // move the boundingBox accordingly to the movement
  void updateBoundingBox();

  public:
  void drawGroundEnemy( SdlCompat_AcceleratedSurface *screen );
  void drawAirEnemy( SdlCompat_AcceleratedSurface *screen );
  void drawShadow( SdlCompat_AcceleratedSurface *screen );
  void drawStats( SdlCompat_AcceleratedSurface *screen );

  // collision system
  // return if the line between the two points collides with the boundingBox
  bool collidesWith( const Vector2D &shotPosOld, const Vector2D &shotPosNew );
  // return if the racers boundingBox overlaps with box
  bool collidesWith( BoundingBox *box );
  // return if the racers boundingBox overlaps with circle
  bool collidesWith( const Circle &circle );
  // returns if a inner circle around the racer overlaps with circle
  bool collidesWithAsCircle( const Circle &circle );
  // returns if a inner circle around the racer overlaps with circle
  bool collidesWithAsCircle( BoundingBox *box );
  // returns the boundingBox of the racer
  BoundingBox *getBoundingBox();
  Circle getBoundingCircle();

  // the enemy got hit -> do the damage according to the shotType
  void doDamage( ShotTypes shotType, int fromWhichPlayer );
};

#endif

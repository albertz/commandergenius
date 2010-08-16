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
#ifndef SHOT_H
#define SHOT_H

#include "SDL.h"
#include "geometry.h"
#include <string>
#include "global.h"

class Shot {
  // time (ms) the shot may fly around
  int timeToLive;
  
  Vector2D pos;
  Vector2D vel;

  // who fired the shot
  int fromWhichPlayer;
  ShotTypes shotType;

  SdlCompat_AcceleratedSurface *sprite;
  SdlCompat_AcceleratedSurface *spriteShadow;

  bool collidedWithGround; // defaultValue = false
  
  bool generatesSmokePuffs;
  int timeToNextSmokePuff;

  public:
  Shot( ShotTypes shotType, int playerNr, Vector2D position, float angle );

  // for rockets only
  bool deflectedBySonicFromPlayer1;
  bool deflectedBySonicFromPlayer2;

  // checks if the shot hit a racer. if yes it expires, does its damage 
  // and makes a neat explosion
  void moveAndCollide( int dT );
  // draws the shot
  void drawShadow(SdlCompat_AcceleratedSurface *screen);
  void drawGroundShot(SdlCompat_AcceleratedSurface *screen);
  void drawAirShot(SdlCompat_AcceleratedSurface *screen);
  void drawGroundAirShot(SdlCompat_AcceleratedSurface *screen);

  bool isExpired() { return (timeToLive <= 0); }
  Vector2D getPos() { return pos; }
  ShotTypes getShotType();

  private:

  void addExplosion();

  void moveAndCollidePlayerShot( int dT );
  void movePlayerShot( int dT );
  bool collidePlayerShot( Vector2D posOld );

  void moveAndCollideEnemyShot( int dT );
  void moveEnemyShot( int dT );
  bool collideEnemyShot( Vector2D posOld );

  void generateSmokePuff( int dT );
};

#endif

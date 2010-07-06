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
#ifndef RACER_H
#define RACER_H

#include <vector>
#include "SDL.h"
#include "settings.h"

class Racer;
class Shots;
class Items;
class Enemys;
class Explosions;

/* A class, that manages the individual racers.
   It is used for everything, that the racers are capable of doing.
 */
class Racers {
  vector<Racer *> racers;

  // points reached by racer 0 in arcade mode
  int pointsInArcadeMode;

  void collideWithEnemys();

  PlayerKeys playerKeys0;
  PlayerKeys playerKeys1;

  public:
  
  Racers();
  ~Racers();
  
  bool isShipTypeActive( int shipType );

  void addRacer( Racer *racer );
  void deleteRacers();
  Racer *getRacer(unsigned int i);
  unsigned int getNrRacers() { return racers.size(); }

  // Moves the racers. Calculates collisions between the racers and 
  // collisions between a racer and enemies.
  void moveAndCollide( int dT );

  // Checks if a racer has picked up an item by flying over it.
  void pickUpItems();
  // Checks if a racer was at his pitstop and is being repaired.
  //  void repair( PitStops *pitStops );
  // Lets the racers shoot, if they want to.
  void shoot();
  // recharge the shields
  void rechargeShield( int dT );
  // draws the racers.
  void drawRacers( SdlCompat_AcceleratedSurface *screen );
  void drawShadows( SdlCompat_AcceleratedSurface *screen );
  void drawStats( SdlCompat_AcceleratedSurface *screen );

  // returns, which racer has shot more enemys
  //int getWinner();

  void expireRacers();

  bool bothPlayersLost();

  void receivePointsArcade( float amount );
  int getPointsArcadeMode();

  void getKeyActionMaps();
  void handleEvent( const SDLKey key, const bool pressed );
  private:
  void handlePlayerEvent( PlayerEvent pEvent, int playerNr, bool keyDown );
};

#endif

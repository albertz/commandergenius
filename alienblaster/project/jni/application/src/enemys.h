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
#ifndef ENEMYS_H
#define ENEMYS_H

#include <vector>
#include "SDL.h"


class Enemy;
class Formation;

class Enemys {
  std::vector<Enemy *> enemys;

  std::vector<Formation *> formations;

  int timeToNextEnemy;
  int timeToNextFormation;

  // if != 0, only the enemys for this boss are generated
  int bossActive;

  int enemysGenerated;
  int enemysKilled;
  
  public:
  
  Enemys();
  ~Enemys();
  
  void addEnemy( Enemy *newEnemy );
  
  bool minibossDead();
  bool bossDead();
  void bossTime( int bossNr );
  void generateEnemys( int dT );
  void deleteExpiredEnemys();
  
  inline Enemy *getEnemy(unsigned int i) { return enemys[i]; }  
  inline unsigned int getNrEnemys() { return enemys.size(); }
  inline int getNrEnemysKilled() { return enemysKilled; }
  inline int getNrEnemysGenerated() { return enemysGenerated; }


  // move and shoot
  void updateEnemys( int dT );

  // a nuke exploded -> damage all enemys
  void doNukeDamage();
  
  // draws the enemys.
  void drawGroundEnemys( SdlCompat_AcceleratedSurface *screen );
  void drawAirEnemys( SdlCompat_AcceleratedSurface *screen );
  void drawShadows( SdlCompat_AcceleratedSurface *screen );
  void drawBossStats( SdlCompat_AcceleratedSurface *screen );
  void drawAllStats( SdlCompat_AcceleratedSurface *screen );
};

#endif

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
#ifndef FORMATION_H
#define FORMATION_H

#include <vector>
#include "global.h"
#include "geometry.h"

class Enemy;

class Formation {
  private:
  std::vector<Enemy *> involvedEnemys;
  int nrInvolved;

  FormationTypes formationType;

  Vector2D formationCenter;
  Vector2D formationSpeed;
  
  bool enemyWasKilled;

  bool formationFires;
  int nextFirePrimary;
  int nextFireSecondary;

  bool changeOnKill, changeSpontaneous, changeOften, changeSeldom;
  int nextFormationChange;
  FormationShotPatterns shotPattern;
  int actShootingEnemy;

  void chooseNewFormationType();
  
  void handleSpontaneousFormationChange( int dT );
  void moveEnemyInFormation();
  void fillTargetPos( vector<Vector2D> &targetPos );
  void fillTargetPosFormationV( vector<Vector2D> &targetPos );
  void fillTargetPosFormationReverseV( vector<Vector2D> &targetPos );
  void fillTargetPosFormationBlock( vector<Vector2D> &targetPos );
  void fillTargetPosFormationLine( vector<Vector2D> &targetPos );
  void getBestMapping( vector<Vector2D> &targetPos,
		       vector<Vector2D> &relPosForFighters );
  
  void calcNextPerm( int *perm );
  float calcTestMapping( int *perm, vector<Vector2D> &targetPos,
			 vector<Vector2D> *mapping );
  int factorial( int n );
  
  void shoot( int dT );

  public:
  Formation( FormationTypes whichFormation, Vector2D centerAtStart,
	     Vector2D startVel, int nrEnemys=66, 
	     FormationEnemySets enemyTypes=FORMATION_ENEMY_SET_DEFAULT,
	     int flagsFormationChangePolicy=0,
	     FormationShotPatterns shotPattern=FORMATION_SP_NONE );
  ~Formation();

  Vector2D getCenter() { return formationCenter; }

  void enemyKilled( Enemy *killedEnemy );
  void update( int dT );
  bool isExpired() { return (nrInvolved == 0); }
};


#endif

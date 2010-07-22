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
#include "formation.h"
#include "enemy.h"
#include "enemys.h"
#include <iostream>

using namespace std;

Formation::Formation( FormationTypes whichFormation, Vector2D centerAtStart,
		      Vector2D startVel, int nrEnemys,
		      FormationEnemySets enemyTypes,
		      int flagsFormationChangePolicy,
		      FormationShotPatterns shotPattern ) {

  formationType = whichFormation;
  formationCenter = centerAtStart;
  formationSpeed = startVel;

  changeOnKill = flagsFormationChangePolicy & FORMATION_CHANGE_ON_KILL;
  changeSpontaneous = flagsFormationChangePolicy & FORMATION_CHANGE_SPONTANEOUS;
  changeOften = flagsFormationChangePolicy & FORMATION_CHANGE_OFTEN;
  changeSeldom = flagsFormationChangePolicy & FORMATION_CHANGE_SELDOM;
  if ( changeSpontaneous && !changeSeldom && !changeOften ) changeSeldom = true;
  if ( changeOften ) {
    nextFormationChange = 
      FORMATION_CHANGE_OFTEN_DELAY + 
      rand() % (FORMATION_CHANGE_OFTEN_RAND_DELAY+1);
  } else if ( changeSeldom ) {
    nextFormationChange = 
      FORMATION_CHANGE_SELDOM_DELAY + 
      rand() % (FORMATION_CHANGE_SELDOM_RAND_DELAY+1);
  }

  if ( nrEnemys > FORMATION_MAX_NR_ENEMYS[ formationType ] ) 
    nrInvolved = FORMATION_MAX_NR_ENEMYS[ formationType ];
  else if ( nrEnemys < 0 ) nrInvolved = 0;
  else nrInvolved = nrEnemys;
  
  this->shotPattern = shotPattern;
  actShootingEnemy = 0;

  if ( shotPattern == FORMATION_SP_NONE ) formationFires = false;
  else formationFires = true;

  vector<Vector2D> targetPos;
  fillTargetPos( targetPos );

  for ( int i = 0; i < nrInvolved; i++ ) {
    Enemy *newOne;
    switch ( enemyTypes ) {
    case FORMATION_ENEMY_SET_DEFAULT:
    case FORMATION_ENEMY_SET_FIGHTER:
      {
	newOne = new Enemy( formationCenter + targetPos[ i ], formationSpeed, FIGHTER,
			    true, formationFires );
	break;
      }
    case FORMATION_ENEMY_SET_BOMBER:
      {
	newOne = new Enemy( formationCenter + targetPos[ i ], formationSpeed, BOMBER,
			    true, formationFires );
	break;
      }
    case FORMATION_ENEMY_SET_FIGHTER_BOMBER:
      {
	newOne = 
	  new Enemy( formationCenter + targetPos[ i ], formationSpeed, 
		     (EnemyTypes)(FIGHTER + (i % 2)),
		     true, formationFires );
	break;
      }
    }
    enemys->addEnemy( newOne );	
    involvedEnemys.push_back( newOne );
  }

  enemyWasKilled = false;
  // wait at least 100 ms before the first shot
  nextFirePrimary = 100; 
  nextFireSecondary = 100;

  // cout << "Type: " << formationType << "  SP: " << shotPattern << endl;
}

Formation::~Formation() {}


void Formation::enemyKilled( Enemy *killedEnemy ) {
  for ( int i = 0; i < nrInvolved; i++ ) {
    if ( involvedEnemys[i] == killedEnemy ) {
      enemyWasKilled = true;
      involvedEnemys.erase( involvedEnemys.begin() + i );
      nrInvolved--;
      break;
    }
  }
}

void Formation::update( int dT ) {
  if ( changeSpontaneous ) handleSpontaneousFormationChange( dT );

  if ( enemyWasKilled ) {
    // change the formation?
    if ( changeOnKill && (rand() % 100 < 70) ) chooseNewFormationType();
    moveEnemyInFormation();
    enemyWasKilled = false;
  }
  formationCenter += formationSpeed * dT / 1000.0;
  shoot( dT );
}


void Formation::handleSpontaneousFormationChange( int dT ) {
  nextFormationChange -= dT;
  if ( changeSpontaneous && nextFormationChange < 0 ) {

    chooseNewFormationType();
    moveEnemyInFormation();

    if ( changeOften ) {
      nextFormationChange = 
	FORMATION_CHANGE_OFTEN_DELAY + 
	rand() % (FORMATION_CHANGE_OFTEN_RAND_DELAY+1);
    } else if ( changeSeldom ) {
      nextFormationChange = 
	FORMATION_CHANGE_SELDOM_DELAY + 
	rand() % (FORMATION_CHANGE_SELDOM_RAND_DELAY+1);
    }
  }
}


void Formation::moveEnemyInFormation() {
  // calc the target positions in the new formation (relative to the center of the formation)
  vector<Vector2D> targetPos;
  fillTargetPos( targetPos );
  
  // choose the best mapping from enemy to targetPosition 
  // (shortest way for enemy to its position is best)
  vector<Vector2D> relPosForEnemies;
  getBestMapping( targetPos, relPosForEnemies );

  // give the enemy its order
  for ( int i = 0; i < nrInvolved; i++ ) {
    involvedEnemys[i]->setNewRelTargetPos( relPosForEnemies[ i ] );
  }
}


void Formation::chooseNewFormationType() {
  bool found = false;
  int i = 0;
  while ( i < 10 && !found ) {
    FormationTypes newFormationType = (FormationTypes)(rand() % (NR_FORMATION_TYPES-1));
    if ( formationType == newFormationType ) {
      newFormationType = (FormationTypes)(NR_FORMATION_TYPES - 1);
    } 
    if ( nrInvolved <= FORMATION_MAX_NR_ENEMYS[ newFormationType ] ) {
      formationType = newFormationType;
      found = true;
    }
    i++;
  }
}


void Formation::fillTargetPos( vector<Vector2D> &targetPos ) {
  switch ( formationType ) {
  case FORMATION_V: 
    {
      fillTargetPosFormationV( targetPos );
      break;
    }
  case FORMATION_REVERSE_V:
    {
      fillTargetPosFormationReverseV( targetPos );
      break;
    }
  case FORMATION_BLOCK:
    {
      fillTargetPosFormationBlock( targetPos );
      break;
    }
  case FORMATION_LINE:
    {
      fillTargetPosFormationLine( targetPos );
      break;
    }
  default:
    {
      for ( int i = 0; i < nrInvolved; i++ ) {
	targetPos.push_back( Vector2D(0,0) );
      }
      break;
    }
  }
}


void Formation::fillTargetPosFormationV( vector<Vector2D> &targetPos ) {
  switch ( nrInvolved ) {
  case 1: 
    {
      targetPos.push_back( Vector2D(0,0) );
      break;
    }
  case 2: 
    {
      targetPos.push_back( Vector2D(-30,0) );
      targetPos.push_back( Vector2D(30,0) );
      break;
    }
  case 3: 
    {
      targetPos.push_back( Vector2D(-50,25) );
      targetPos.push_back( Vector2D(0,-25) );
      targetPos.push_back( Vector2D(50,25) );
      break;
    }
  case 4: 
    {
      targetPos.push_back( Vector2D(-80,25) );
      targetPos.push_back( Vector2D(-30,-25) );
      targetPos.push_back( Vector2D(30,-25) );
      targetPos.push_back( Vector2D(80,25) );
      break;
    }
  case 5: 
    {
      targetPos.push_back( Vector2D(-100,50) );
      targetPos.push_back( Vector2D(-50,0) );
      targetPos.push_back( Vector2D(0,-50) );
      targetPos.push_back( Vector2D(50,0) );
      targetPos.push_back( Vector2D(100,50) );
      break;
    }
  case 6: 
    {
      targetPos.push_back( Vector2D(-130,50) );
      targetPos.push_back( Vector2D(-80,0) );
      targetPos.push_back( Vector2D(-30,-50) );
      targetPos.push_back( Vector2D(30,-50) );
      targetPos.push_back( Vector2D(80,0) );
      targetPos.push_back( Vector2D(130,50) );
      break;
    }
  case 7: 
    {
      targetPos.push_back( Vector2D(-150,75) );
      targetPos.push_back( Vector2D(-100,25) );
      targetPos.push_back( Vector2D(-50,-25) );
      targetPos.push_back( Vector2D(0,-75) );
      targetPos.push_back( Vector2D(50,-25) );
      targetPos.push_back( Vector2D(100,25) );
      targetPos.push_back( Vector2D(150,75) );
      break;
    }
  default:
    {
      cout << "fillTargetPosFormationV: too many enemys involved: " << nrInvolved << endl;
      for ( int i = 0; i < nrInvolved; i++ ) {
	targetPos.push_back( Vector2D(0,0) );
      }
      break;
    }
  }
}


void Formation::fillTargetPosFormationReverseV( vector<Vector2D> &targetPos ) {
  switch ( nrInvolved ) {
  case 1: 
    {
      targetPos.push_back( Vector2D(0,0) );
      break;
    }
  case 2: 
    {
      targetPos.push_back( Vector2D(-30,0) );
      targetPos.push_back( Vector2D(30,0) );
      break;
    }
  case 3: 
    {
      targetPos.push_back( Vector2D(-50,-25) );
      targetPos.push_back( Vector2D(0,25) );
      targetPos.push_back( Vector2D(50,-25) );
      break;
    }
  case 4: 
    {
      targetPos.push_back( Vector2D(-80,-25) );
      targetPos.push_back( Vector2D(-30,25) );
      targetPos.push_back( Vector2D(30,25) );
      targetPos.push_back( Vector2D(80,-25) );
      break;
    }
  case 5: 
    {
      targetPos.push_back( Vector2D(-100,-50) );
      targetPos.push_back( Vector2D(-50,0) );
      targetPos.push_back( Vector2D(0,50) );
      targetPos.push_back( Vector2D(50,0) );
      targetPos.push_back( Vector2D(100,-50) );
      break;
    }
  case 6: 
    {
      targetPos.push_back( Vector2D(-130,-50) );
      targetPos.push_back( Vector2D(-80,0) );
      targetPos.push_back( Vector2D(-30,50) );
      targetPos.push_back( Vector2D(30,50) );
      targetPos.push_back( Vector2D(80,0) );
      targetPos.push_back( Vector2D(130,-50) );
      break;
    }
  case 7: 
    {
      targetPos.push_back( Vector2D(-150,-75) );
      targetPos.push_back( Vector2D(-100,-25) );
      targetPos.push_back( Vector2D(-50,25) );
      targetPos.push_back( Vector2D(0,75) );
      targetPos.push_back( Vector2D(50,25) );
      targetPos.push_back( Vector2D(100,-25) );
      targetPos.push_back( Vector2D(150,-75) );
      break;
    }
  default:
    {
      cout << "fillTargetPosFormationReverseV: too many enemys involved: " 
	   << nrInvolved << endl;
      for ( int i = 0; i < nrInvolved; i++ ) {
	targetPos.push_back( Vector2D(0,0) );
      }
      break;
    }
  }
}


void Formation::fillTargetPosFormationBlock( vector<Vector2D> &targetPos ) {
  switch ( nrInvolved ) {
  case 1: 
    {
      targetPos.push_back( Vector2D(0,0) );
      break;
    }
  case 2: 
    {
      targetPos.push_back( Vector2D(-30,0) );
      targetPos.push_back( Vector2D(30,0) );
      break;
    }
  case 3: 
    {
      targetPos.push_back( Vector2D(-30,25) );
      targetPos.push_back( Vector2D(0,-25) );
      targetPos.push_back( Vector2D(30,25) );
      break;
    }
  case 4: 
    {
      targetPos.push_back( Vector2D(-30,-25) );
      targetPos.push_back( Vector2D(-30,25) );
      targetPos.push_back( Vector2D(30,-25) );
      targetPos.push_back( Vector2D(30,25) );
      break;
    }
  case 5: 
    {
      targetPos.push_back( Vector2D(-40,-30) );
      targetPos.push_back( Vector2D(-40,30) );
      targetPos.push_back( Vector2D(0,0) );
      targetPos.push_back( Vector2D(40,-30) );
      targetPos.push_back( Vector2D(40,30) );
      break;
    }
  case 6: 
    {
      targetPos.push_back( Vector2D(-60,-30) );
      targetPos.push_back( Vector2D(-60,30) );
      targetPos.push_back( Vector2D(0,-30) );
      targetPos.push_back( Vector2D(0,30) );
      targetPos.push_back( Vector2D(60,-30) );
      targetPos.push_back( Vector2D(60,30) );
      break;
    }
  case 7: 
    {
      targetPos.push_back( Vector2D(-60,-50) );
      targetPos.push_back( Vector2D(-60,0) );
      targetPos.push_back( Vector2D(0,-50) );
      targetPos.push_back( Vector2D(0,0) );
      targetPos.push_back( Vector2D(0,50) );
      targetPos.push_back( Vector2D(60,-50) );
      targetPos.push_back( Vector2D(60,0) );
      break;
    }
  default:
    {
      cout << "fillTargetPosFormationBlock: too many enemys involved: " << nrInvolved << endl;
      for ( int i = 0; i < nrInvolved; i++ ) {
	targetPos.push_back( Vector2D(0,0) );
      }
      break;
    }
  }
}


void Formation::fillTargetPosFormationLine( vector<Vector2D> &targetPos ) {
  switch ( nrInvolved ) {
  case 1: 
    {
      targetPos.push_back( Vector2D(0,0) );
      break;
    }
  case 2: 
    {
      targetPos.push_back( Vector2D(-30,0) );
      targetPos.push_back( Vector2D(30,0) );
      break;
    }
  case 3: 
    {
      targetPos.push_back( Vector2D(-60,0) );
      targetPos.push_back( Vector2D(0,0) );
      targetPos.push_back( Vector2D(60,0) );
      break;
    }
  case 4: 
    {
      targetPos.push_back( Vector2D(-90,0) );
      targetPos.push_back( Vector2D(-30,0) );
      targetPos.push_back( Vector2D(30,0) );
      targetPos.push_back( Vector2D(90,0) );
      break;
    }
  case 5: 
    {
      targetPos.push_back( Vector2D(-120,0) );
      targetPos.push_back( Vector2D(-60,0) );
      targetPos.push_back( Vector2D(0,0) );
      targetPos.push_back( Vector2D(60,0) );
      targetPos.push_back( Vector2D(120,0) );
      break;
    }
  case 6: 
    {
      targetPos.push_back( Vector2D(-150,0) );
      targetPos.push_back( Vector2D(-90,0) );
      targetPos.push_back( Vector2D(-30,0) );
      targetPos.push_back( Vector2D(30,0) );
      targetPos.push_back( Vector2D(90,0) );
      targetPos.push_back( Vector2D(150,0) );
      break;
    }
  default:
    {
      cout << "fillTargetPosFormationLine: too many enemys involved: " << nrInvolved << endl;
      for ( int i = 0; i < nrInvolved; i++ ) {
	targetPos.push_back( Vector2D(0,0) );
      }
      break;
    }
  }
}




///////////////////////////////////////////////


void Formation::getBestMapping( vector<Vector2D> &targetPos,
				vector<Vector2D> &relPosForFighters ) {
  
  int actPerm[nrInvolved];
  vector<Vector2D> *bestMapping = new vector<Vector2D>();
  vector<Vector2D> *testMapping = new vector<Vector2D>();
  for ( int i = 0; i < nrInvolved; i++ ) {
    bestMapping->push_back( Vector2D( 0,0 ) );
    testMapping->push_back( Vector2D( 0,0 ) );
    actPerm[ i ] = i;
  }
  float mapCost = 1000000;
  
  int nrPerm = factorial( nrInvolved );

  for ( int perm = 0; perm < nrPerm; perm++ ) {
    calcNextPerm( actPerm );
    float testMappingCost = calcTestMapping( actPerm, targetPos, testMapping );
    if ( mapCost > testMappingCost ) {
      vector<Vector2D> *tmpMapping = bestMapping;
      bestMapping = testMapping;
      testMapping = tmpMapping;
      mapCost = testMappingCost;
    }
  }  

  for ( int e = 0; e < nrInvolved; e++ ) {
    relPosForFighters.push_back( (*bestMapping)[e] );
  }
  delete bestMapping;
  delete testMapping;
}


void Formation::calcNextPerm( int *perm ) {
  int n = nrInvolved;
  int i = n-1;
  int j = n;
  int tmp;

  while ( i != 0 && perm[ i-1 ] >= perm[ i ] ) {
    i--;
  }
  if ( i == 0 ) {
    for ( int k = 0; k < n/2; k++ ) {
      tmp = perm[ k ];
      perm[ k ] = perm[ n - k - 1 ];
      perm[ n - k - 1 ] = tmp;
    }
    return;
  }
  while ( perm[ j-1 ] <= perm[ i-1 ] ) {
    --j;
  }
  tmp = perm[ i-1 ];
  perm[ i-1 ] = perm[ j-1 ];
  perm[ j-1 ] = tmp;
  
  i++;
  j = n;
  
  while ( i < j ) {
    tmp = perm[ i-1 ];
    perm[ i-1 ] = perm[ j-1 ];
    perm[ j-1 ] = tmp;
    i++;
    j--;
  }
}


float Formation::calcTestMapping( int *perm, vector<Vector2D> &targetPos,
				 vector<Vector2D> *mapping ) {

  float cost = 0;

  for ( int i = 0; i < nrInvolved; i++ ) {
    // enemy i shall fly to the position targetPos[perm[i]]
    // save the vector from its actual pos to its new targetpos in mapping
    (*mapping)[ i ] = 
      targetPos[ perm[i] ] + formationCenter - 
      involvedEnemys[ i ]->getPos();

    if ( cost < (*mapping)[ i ].getLength() ) {
      cost = (*mapping)[ i ].getLength();
    }
  }
  return cost;
}


int Formation::factorial( int n ) {
  int result = 1;
  for ( int i = 2; i <= n; i++ ) {
    result *= i;
  }
  return result;
}



void Formation::shoot( int dT ) {
  if ( shotPattern == FORMATION_SP_NONE ) return;
  nextFirePrimary -= dT;
  //  nextFireSecondary -= dT;

  float enemyRatio = FORMATION_MAX_NR_ENEMYS[ formationType ] / (float)nrInvolved;

  while ( nextFirePrimary < 0 ) {
    switch ( shotPattern ) {
    case FORMATION_SP_VOLLEY_FAST:
    case FORMATION_SP_VOLLEY_MEDIUM:
    case FORMATION_SP_VOLLEY_SLOW:
      {
	for ( int i = 0; i < nrInvolved; i++ ) {
	  involvedEnemys[ i ]->firePrimary();
	}
	break;
      }
    case FORMATION_SP_RAND_FAST:
    case FORMATION_SP_RAND_MEDIUM:
    case FORMATION_SP_RAND_SLOW:
      {
	involvedEnemys[ rand() % nrInvolved ]->firePrimary();
	break;
      }
    case FORMATION_SP_LEFT_RIGHT_FAST:
    case FORMATION_SP_LEFT_RIGHT_MEDIUM:
      {
	actShootingEnemy = (actShootingEnemy + 1) % nrInvolved;
	involvedEnemys[ actShootingEnemy ]->firePrimary();
	break;
      }
    case FORMATION_SP_RIGHT_LEFT_FAST:
    case FORMATION_SP_RIGHT_LEFT_MEDIUM:
      {
	actShootingEnemy--;
	if ( actShootingEnemy < 0 ) actShootingEnemy = nrInvolved-1;
	involvedEnemys[ actShootingEnemy ]->firePrimary();
	break;
      }
    default:
      {
	break;
      }
    }

    nextFirePrimary += 
      lroundf( (FORMATION_SP_PRIMARY_DELAY[ shotPattern ] +
		rand() % (FORMATION_SP_PRIMARY_RAND_DELAY[ shotPattern ] + 1)) * enemyRatio );
  }
}


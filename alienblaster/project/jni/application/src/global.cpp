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
#include "global.h"
#include "options.h"
#include "asstring.h"
#include <iostream>
#include "SDL.h"
#include "surfaceDB.h"

using namespace std;

int GAME_LENGTH = 300000; // ms
int ITEM_LIFETIME = 20000; // ms
int ITEM_APPEAR_DELAY = 10000; // ms
int ITEM_APPEAR_RAND_DELAY = 20000; // ms
int ITEM_HEALTH_REPAIR_AMOUNT = 100;

int ITEM_HEATSEEKER_AMMO = 70;
int ITEM_NUKE_AMMO = 2;
int ITEM_DEFLECTOR_AMMO = 1;
int ITEM_DEFLECTOR_ACTIVATION_DIST = 200;
int ITEM_DEFLECTOR_POWER = 100;
int ITEM_DEFLECTOR_DURATION = 15000;

int ITEM_APPEAR_CHANCES[] = { 50, 20, 30, 30, 30, 50, 20, 20, 30, 20, 20 };

int GENERATE_ENEMY_DELAY = 100;
int GENERATE_ENEMY_RAND_DELAY = 2000;

int LIGHT_FIGHTER_MAX_SHIELD = 200;
int HEAVY_FIGHTER_MAX_SHIELD = 500;
int LIGHT_FIGHTER_MAX_DAMAGE = 200;
int HEAVY_FIGHTER_MAX_DAMAGE = 500;

int LIGHT_FIGHTER_SHIELD_RECHARGE = 80;
int HEAVY_FIGHTER_SHIELD_RECHARGE = 110;

int RACER_DEFLECTOR_ACTIVATION_DIST = 150;
int RACER_DEFLECTOR_POWER = 50;
int RACER_SONIC_ACTIVATION_DIST = 200;
int RACER_SONIC_POWER = 50;

int ENEMY_HITPOINTS[] = { 80, 120, 150, 2000, 1500, 1000, 1000, 3000 };
int ENEMY_COLLISION_DAMAGE[] = {20, 40, 0, 0, 0, 0, 0, 2000};
int ENEMY_DIES_ITEM_APPEAR_CHANCE[] = {15, 12, 8, 0, 0, 0, 0, 1};
int ENEMY_APPEAR_CHANCES[] = {40, 40, 10, 0, 0, 0, 0, 0};
int ENEMY_COOLDOWN_PRIMARY[] = {400, 400, 400, 50, 500, 100, 100, 300};
int ENEMY_COOLDOWN_SECONDARY[] = {0, 0, 2000, 0, 0, 0, 0, 600};
int ENEMY_RAND_WAIT_PRIMARY[] = {1500, 1000, 300, 80, 1000, 200, 200, 2000};
int ENEMY_RAND_WAIT_SECONDARY[] = {0, 0, 10000, 0, 0, 0, 0, 400};

int GENERATE_FORMATION_DELAY = 5000;
int GENERATE_FORMATION_RAND_DELAY = 20000;

int FORMATION_MAX_NR_ENEMYS[] = {7,7,7,6};
int FORMATION_SP_CHANCES[] = { 10, 10, 5, 5, 20, 10, 5, 20, 10, 20, 10 };
int FORMATION_SP_PRIMARY_DELAY[] = {0, 80, 120, 160,
				    500, 800, 1100,
				    150, 250, 150, 250};
int FORMATION_SP_PRIMARY_RAND_DELAY[] = { 0, 80, 120, 160,
					  1000, 1500, 2000,
					  0, 0, 0, 0 };




void parseGlobalConfigValues( int difficultyLevel ) {

  Options *op = 0;
  op = new Options( FN_DIFFICULTY_CONFIG + 
		    asString(difficultyLevel) + 
		    FN_DIFFICULTY_CONFIG_SUFFIX );

  op->getInt( "GAME_LENGTH", GAME_LENGTH );

  op->getInt( "ITEM_LIFETIME", ITEM_LIFETIME );
  op->getInt( "ITEM_APPEAR_DELAY", ITEM_APPEAR_DELAY );
  op->getInt( "ITEM_APPEAR_RAND_DELAY", ITEM_APPEAR_RAND_DELAY );

  op->getInt( "ITEM_HEALTH_REPAIR_AMOUNT", ITEM_HEALTH_REPAIR_AMOUNT );

  op->getInt( "ITEM_HEATSEEKER_AMMO", ITEM_HEATSEEKER_AMMO );
  op->getInt( "ITEM_NUKE_AMMO", ITEM_NUKE_AMMO );
  op->getInt( "ITEM_DEFLECTOR_AMMO", ITEM_DEFLECTOR_AMMO );
  op->getInt( "ITEM_DEFLECTOR_DURATION", ITEM_DEFLECTOR_DURATION );
  op->getInt( "ITEM_DEFLECTOR_ACTIVATION_DIST", ITEM_DEFLECTOR_ACTIVATION_DIST );
  op->getInt( "ITEM_DEFLECTOR_POWER", ITEM_DEFLECTOR_POWER );

  op->getInt( "GENERATE_ENEMY_DELAY", GENERATE_ENEMY_DELAY );
  op->getInt( "GENERATE_ENEMY_RAND_DELAY", GENERATE_ENEMY_RAND_DELAY );

  op->getInt( "LIGHT_FIGHTER_MAX_SHIELD", LIGHT_FIGHTER_MAX_SHIELD );
  op->getInt( "HEAVY_FIGHTER_MAX_SHIELD", HEAVY_FIGHTER_MAX_SHIELD );
  op->getInt( "LIGHT_FIGHTER_MAX_DAMAGE", LIGHT_FIGHTER_MAX_DAMAGE );
  op->getInt( "HEAVY_FIGHTER_MAX_DAMAGE", HEAVY_FIGHTER_MAX_DAMAGE );
  op->getInt( "LIGHT_FIGHTER_SHIELD_RECHARGE", LIGHT_FIGHTER_SHIELD_RECHARGE );
  op->getInt( "HEAVY_FIGHTER_SHIELD_RECHARGE", HEAVY_FIGHTER_SHIELD_RECHARGE );

  op->getInt( "HEAVY_FIGHTER_DEFLECTOR_ACTIVATION_DIST", 
	      RACER_DEFLECTOR_ACTIVATION_DIST );
  op->getInt( "HEAVY_FIGHTER_DEFLECTOR_POWER",
	      RACER_DEFLECTOR_POWER );
  
  op->getInt( "FIGHTER_HITPOINTS", ENEMY_HITPOINTS[ FIGHTER ] );
  op->getInt( "BOMBER_HITPOINTS", ENEMY_HITPOINTS[ BOMBER ] );
  op->getInt( "TURRET_HITPOINTS", ENEMY_HITPOINTS[ TANK ] );
  op->getInt( "BOSS1_MAIN_GUN_HITPOINTS", ENEMY_HITPOINTS[ BOSS_1_MAIN_GUN ] );
  op->getInt( "BOSS1_ROCKET_LAUNCHER_HITPOINTS", ENEMY_HITPOINTS[ BOSS_1_ROCKET_LAUNCHER ] );
  op->getInt( "BOSS1_SHOT_BATTERY_LEFT_HITPOINTS", 
	      ENEMY_HITPOINTS[ BOSS_1_SHOT_BATTERY_LEFT ] );
  op->getInt( "BOSS1_SHOT_BATTERY_RIGHT_HITPOINTS", 
	      ENEMY_HITPOINTS[ BOSS_1_SHOT_BATTERY_RIGHT ] );
  
  op->getInt( "COLLISION_DAMAGE_FIGHTER", ENEMY_COLLISION_DAMAGE[ FIGHTER ] );
  op->getInt( "COLLISION_DAMAGE_BOMBER", ENEMY_COLLISION_DAMAGE[ BOMBER ] );
  op->getInt( "ENEMY_DIES_ITEM_APPEAR_CHANCE_FIGHTER", 
	      ENEMY_DIES_ITEM_APPEAR_CHANCE[ FIGHTER ] );
  op->getInt( "ENEMY_DIES_ITEM_APPEAR_CHANCE_BOMBER",
	      ENEMY_DIES_ITEM_APPEAR_CHANCE[ BOMBER ] );
  op->getInt( "ENEMY_DIES_ITEM_APPEAR_CHANCE_TURRET", 
	      ENEMY_DIES_ITEM_APPEAR_CHANCE[ TANK ] );

  op->getInt( "ENEMY_APPEAR_CHANCE_FIGHTER",
	      ENEMY_APPEAR_CHANCES[ FIGHTER ] );
  op->getInt( "ENEMY_APPEAR_CHANCE_BOMBER",
	      ENEMY_APPEAR_CHANCES[ BOMBER ] );
  op->getInt( "ENEMY_APPEAR_CHANCE_TURRET",
	      ENEMY_APPEAR_CHANCES[ TANK ] );

  op->getInt( "ENEMY_COOLDOWN_FIGHTER", 
	      ENEMY_COOLDOWN_PRIMARY[ FIGHTER ] );
  op->getInt( "ENEMY_COOLDOWN_BOMBER", 
	      ENEMY_COOLDOWN_PRIMARY[ BOMBER ] );
  op->getInt( "ENEMY_COOLDOWN_TURRET", 
	      ENEMY_COOLDOWN_PRIMARY[ TANK ] );
  op->getInt( "ENEMY_COOLDOWN_BOSS1_MAIN_GUN", 
	      ENEMY_COOLDOWN_PRIMARY[ BOSS_1_MAIN_GUN ] );
  op->getInt( "ENEMY_COOLDOWN_BOSS1_ROCKET_LAUNCHER", 
	      ENEMY_COOLDOWN_PRIMARY[ BOSS_1_ROCKET_LAUNCHER] );
  op->getInt( "ENEMY_COOLDOWN_BOSS1_SHOT_BATTERY_LEFT", 
	      ENEMY_COOLDOWN_PRIMARY[ BOSS_1_SHOT_BATTERY_LEFT] );
  op->getInt( "ENEMY_COOLDOWN_BOSS1_SHOT_BATTERY_RIGHT", 
	      ENEMY_COOLDOWN_PRIMARY[ BOSS_1_SHOT_BATTERY_RIGHT ] );

  op->getInt( "ENEMY_COOLDOWN_SECONDARY_TURRET", ENEMY_COOLDOWN_SECONDARY[ TANK ] );

  op->getInt( "ENEMY_RAND_WAIT_FIGHTER", 
	      ENEMY_RAND_WAIT_PRIMARY[ FIGHTER ] );
  op->getInt( "ENEMY_RAND_WAIT_BOMBER", 
	      ENEMY_RAND_WAIT_PRIMARY[ BOMBER ] );
  op->getInt( "ENEMY_RAND_WAIT_TURRET",
	      ENEMY_RAND_WAIT_PRIMARY[ TANK ] );
  op->getInt( "ENEMY_RAND_WAIT_BOSS1_MAIN_GUN",
	      ENEMY_RAND_WAIT_PRIMARY[ BOSS_1_MAIN_GUN ] );
  op->getInt( "ENEMY_RAND_WAIT_BOSS1_ROCKET_LAUNCHER",
	      ENEMY_RAND_WAIT_PRIMARY[ BOSS_1_ROCKET_LAUNCHER ] );
  op->getInt( "ENEMY_RAND_WAIT_BOSS1_SHOT_BATTERY_LEFT",
	      ENEMY_RAND_WAIT_PRIMARY[ BOSS_1_SHOT_BATTERY_LEFT ] );
  op->getInt( "ENEMY_RAND_WAIT_BOSS1_SHOT_BATTERY_RIGHT",
	      ENEMY_RAND_WAIT_PRIMARY[ BOSS_1_SHOT_BATTERY_RIGHT ] );

  op->getInt( "ENEMY_RAND_WAIT_SECONDARY_TURRET",
	      ENEMY_RAND_WAIT_SECONDARY[ TANK ] );

  op->getInt( "ITEM_APPEAR_CHANCE_PRIMARY_UPGRADE", 
	      ITEM_APPEAR_CHANCES[ ITEM_PRIMARY_UPGRADE ] );
  op->getInt( "ITEM_APPEAR_CHANCE_DUMBRIFE_DOUBLE", 
	      ITEM_APPEAR_CHANCES[ ITEM_DUMBFIRE_DOUBLE ] );
  op->getInt( "ITEM_APPEAR_CHANCE_KICK_ASS_ROCKET", 
	      ITEM_APPEAR_CHANCES[ ITEM_KICK_ASS_ROCKET ] );
  op->getInt( "ITEM_APPEAR_CHANCE_HELLFIRE", 
	      ITEM_APPEAR_CHANCES[ ITEM_HELLFIRE ] );
  op->getInt( "ITEM_APPEAR_CHANCE_MACHINE_GUN", 
	      ITEM_APPEAR_CHANCES[ ITEM_MACHINE_GUN ] );
  op->getInt( "ITEM_APPEAR_CHANCE_HEALTH", 
	      ITEM_APPEAR_CHANCES[ ITEM_HEALTH ] );
  op->getInt( "ITEM_APPEAR_CHANCE_HEATSEEKER", 
	      ITEM_APPEAR_CHANCES[ ITEM_HEATSEEKER ] );
  op->getInt( "ITEM_APPEAR_CHANCE_NUKE", 
	      ITEM_APPEAR_CHANCES[ ITEM_NUKE ] );
  op->getInt( "ITEM_APPEAR_CHANCE_DEFLECTOR", 
	      ITEM_APPEAR_CHANCES[ ITEM_DEFLECTOR ] );
  op->getInt( "ITEM_APPEAR_CHANCE_ENERGY_BEAM", 
	      ITEM_APPEAR_CHANCES[ ITEM_ENERGY_BEAM ] );
  op->getInt( "ITEM_APPEAR_CHANCE_LASER", 
	      ITEM_APPEAR_CHANCES[ ITEM_LASER ] );

  op->getInt( "GENERATE_FORMATION_DELAY", GENERATE_FORMATION_DELAY );
  op->getInt( "GENERATE_FORMATION_RAND_DELAY", GENERATE_FORMATION_RAND_DELAY );  

  op->getInt( "FORMATION_MAX_NR_ENEMYS_V",
	      FORMATION_MAX_NR_ENEMYS[ FORMATION_V ] );
  op->getInt( "FORMATION_MAX_NR_ENEMYS_REVERSE_V",
	      FORMATION_MAX_NR_ENEMYS[ FORMATION_REVERSE_V ] );
  op->getInt( "FORMATION_MAX_NR_ENEMYS_BLOCK",
	      FORMATION_MAX_NR_ENEMYS[ FORMATION_BLOCK ] );
  op->getInt( "FORMATION_MAX_NR_ENEMYS_LINE",
	      FORMATION_MAX_NR_ENEMYS[ FORMATION_LINE ] );
  for ( int i = 0; i < NR_FORMATION_TYPES; i++ ) {
    if ( FORMATION_MAX_NR_ENEMYS[ i ] > FORMATION_MAX_NR_ENEMYS_HARD_LIMIT[ i ] )
      FORMATION_MAX_NR_ENEMYS[ i ] = FORMATION_MAX_NR_ENEMYS_HARD_LIMIT[ i ];
  }
  
  op->getInt( "FORMATION_SP_CHANCE_NONE",
	      FORMATION_SP_CHANCES[ FORMATION_SP_NONE ] );
  op->getInt( "FORMATION_SP_CHANCE_RAND_FAST",
	      FORMATION_SP_CHANCES[ FORMATION_SP_RAND_FAST ] );
  op->getInt( "FORMATION_SP_CHANCE_RAND_MEDIUM",
	      FORMATION_SP_CHANCES[ FORMATION_SP_RAND_MEDIUM ] );
  op->getInt( "FORMATION_SP_CHANCE_RAND_SLOW",
	      FORMATION_SP_CHANCES[ FORMATION_SP_RAND_SLOW ] );
  op->getInt( "FORMATION_SP_CHANCE_VOLLEY_FAST",
	      FORMATION_SP_CHANCES[ FORMATION_SP_VOLLEY_FAST ] );
  op->getInt( "FORMATION_SP_CHANCE_VOLLEY_MEDIUM",
	      FORMATION_SP_CHANCES[ FORMATION_SP_VOLLEY_MEDIUM ] );
  op->getInt( "FORMATION_SP_CHANCE_VOLLEY_SLOW",
	      FORMATION_SP_CHANCES[ FORMATION_SP_VOLLEY_SLOW ] );
  op->getInt( "FORMATION_SP_CHANCE_LEFT_RIGHT_FAST",
	      FORMATION_SP_CHANCES[ FORMATION_SP_LEFT_RIGHT_FAST ] );
  op->getInt( "FORMATION_SP_CHANCE_LEFT_RIGHT_MEDIUM",
	      FORMATION_SP_CHANCES[ FORMATION_SP_LEFT_RIGHT_MEDIUM ] );
  op->getInt( "FORMATION_SP_CHANCE_RIGHT_LEFT_FAST",
	      FORMATION_SP_CHANCES[ FORMATION_SP_RIGHT_LEFT_FAST ] );
  op->getInt( "FORMATION_SP_CHANCE_RIGHT_LEFT_MEDIUM",
	      FORMATION_SP_CHANCES[ FORMATION_SP_RIGHT_LEFT_MEDIUM ] );

  op->getInt( "FORMATION_SP_PRIM_DELAY_RAND_FAST",
	      FORMATION_SP_PRIMARY_DELAY[ FORMATION_SP_RAND_FAST ] );
  op->getInt( "FORMATION_SP_PRIM_DELAY_RAND_MEDIUM",
	      FORMATION_SP_PRIMARY_DELAY[ FORMATION_SP_RAND_MEDIUM ] );
  op->getInt( "FORMATION_SP_PRIM_DELAY_RAND_SLOW",
	      FORMATION_SP_PRIMARY_DELAY[ FORMATION_SP_RAND_SLOW ] );
  op->getInt( "FORMATION_SP_PRIM_DELAY_VOLLEY_FAST",
	      FORMATION_SP_PRIMARY_DELAY[ FORMATION_SP_VOLLEY_FAST ] );
  op->getInt( "FORMATION_SP_PRIM_DELAY_VOLLEY_MEDIUM",
	      FORMATION_SP_PRIMARY_DELAY[ FORMATION_SP_VOLLEY_MEDIUM ] );
  op->getInt( "FORMATION_SP_PRIM_DELAY_VOLLEY_SLOW",
	      FORMATION_SP_PRIMARY_DELAY[ FORMATION_SP_VOLLEY_SLOW ] );
  op->getInt( "FORMATION_SP_PRIM_DELAY_LEFT_RIGHT_FAST",
	      FORMATION_SP_PRIMARY_DELAY[ FORMATION_SP_LEFT_RIGHT_FAST ] );
  op->getInt( "FORMATION_SP_PRIM_DELAY_LEFT_RIGHT_MEDIUM",
	      FORMATION_SP_PRIMARY_DELAY[ FORMATION_SP_LEFT_RIGHT_MEDIUM ] );
  op->getInt( "FORMATION_SP_PRIM_DELAY_RIGHT_LEFT_FAST",
	      FORMATION_SP_PRIMARY_DELAY[ FORMATION_SP_RIGHT_LEFT_FAST ] );
  op->getInt( "FORMATION_SP_PRIM_DELAY_RIGHT_LEFT_MEDIUM",
	      FORMATION_SP_PRIMARY_DELAY[ FORMATION_SP_RIGHT_LEFT_MEDIUM ] );

  op->getInt( "FORMATION_SP_PRIM_RAND_DELAY_RAND_FAST",
	      FORMATION_SP_PRIMARY_RAND_DELAY[ FORMATION_SP_RAND_FAST ] );
  op->getInt( "FORMATION_SP_PRIM_RAND_DELAY_RAND_MEDIUM",
	      FORMATION_SP_PRIMARY_RAND_DELAY[ FORMATION_SP_RAND_MEDIUM ] );
  op->getInt( "FORMATION_SP_PRIM_RAND_DELAY_RAND_SLOW",
	      FORMATION_SP_PRIMARY_RAND_DELAY[ FORMATION_SP_RAND_SLOW ] );
  op->getInt( "FORMATION_SP_PRIM_RAND_DELAY_VOLLEY_FAST",
	      FORMATION_SP_PRIMARY_RAND_DELAY[ FORMATION_SP_VOLLEY_FAST ] );
  op->getInt( "FORMATION_SP_PRIM_RAND_DELAY_VOLLEY_MEDIUM",
	      FORMATION_SP_PRIMARY_RAND_DELAY[ FORMATION_SP_VOLLEY_MEDIUM ] );
  op->getInt( "FORMATION_SP_PRIM_RAND_DELAY_VOLLEY_SLOW",
	      FORMATION_SP_PRIMARY_RAND_DELAY[ FORMATION_SP_VOLLEY_SLOW ] );
  op->getInt( "FORMATION_SP_PRIM_RAND_DELAY_LEFT_RIGHT_FAST",
	      FORMATION_SP_PRIMARY_RAND_DELAY[ FORMATION_SP_LEFT_RIGHT_FAST ] );
  op->getInt( "FORMATION_SP_PRIM_RAND_DELAY_LEFT_RIGHT_MEDIUM",
	      FORMATION_SP_PRIMARY_RAND_DELAY[ FORMATION_SP_LEFT_RIGHT_MEDIUM ] );
  op->getInt( "FORMATION_SP_PRIM_RAND_DELAY_RIGHT_LEFT_FAST",
	      FORMATION_SP_PRIMARY_RAND_DELAY[ FORMATION_SP_RIGHT_LEFT_FAST ] );
  op->getInt( "FORMATION_SP_PRIM_RAND_DELAY_RIGHT_LEFT_MEDIUM",
	      FORMATION_SP_PRIMARY_RAND_DELAY[ FORMATION_SP_RIGHT_LEFT_MEDIUM ] );

  delete op;
}

/////////////////////

int getRandValue( const int *choicesWeights, int nrChoices, int sumWeights ) {
  int sum = 0;
  if ( sumWeights != 0 ) sum = sumWeights;
  else for ( int i = 0; i < nrChoices; i++ ) sum += choicesWeights[ i ];
  
  if ( sum == 0 ) {
    cout << "Error in getRandValue: sum==0!" << endl;
    exit(1);
  }
  
  int val = rand() % sum;

  int idx = 0;
  int tmpSum = 0;
  while ( idx < nrChoices ) {
    tmpSum += choicesWeights[ idx ];
    if ( val < tmpSum ) {
      return idx;
    }
    idx++;
  }
  cout << "Error in getRandValue: val: " << val << endl;
  exit(1);
}


/////////////////////////////

void initAllSurfaces() {
  surfaceDB.loadSurface("./images/fighter.bmp" );
  surfaceDB.loadSurface("./images/fighterShadow.bmp", true);
  surfaceDB.loadSurface("./images/bomber.bmp");
  surfaceDB.loadSurface("./images/bomberShadow.bmp", true);
  surfaceDB.loadSurface("./images/tank.bmp");
  surfaceDB.loadSurface("./images/boss1MainGun.bmp");
  surfaceDB.loadSurface("./images/boss1RocketLauncher.bmp");
  surfaceDB.loadSurface("./images/boss1ShotBatteryLeft.bmp");
  surfaceDB.loadSurface("./images/boss1ShotBatteryRight.bmp");
  surfaceDB.loadSurface("./images/wreckFighter.bmp");
  surfaceDB.loadSurface("./images/wreckBomber.bmp");
  surfaceDB.loadSurface("./images/wreckTank.bmp");
  surfaceDB.loadSurface("./images/wreckBoss1.bmp");
  surfaceDB.loadSurface("./images/wreckBossBackground.bmp");
  surfaceDB.loadSurface("./images/boss.bmp");
  surfaceDB.loadSurface("./images/normalShot.bmp");
  surfaceDB.loadSurface("./images/heavyShot.bmp");
  surfaceDB.loadSurface("./images/normalShotHF.bmp");
  surfaceDB.loadSurface("./images/dumbfire.bmp");
  surfaceDB.loadSurface("./images/kickAssRocket.bmp");
  surfaceDB.loadSurface("./images/kickAssRocketShadow.bmp", true);
  surfaceDB.loadSurface("./images/hellfire.bmp");
  surfaceDB.loadSurface("./images/hellfireShadow.bmp", true);
  surfaceDB.loadSurface("./images/machineGun.bmp");
  surfaceDB.loadSurface("./images/energyBeam.bmp", true);
  surfaceDB.loadSurface("./images/laser.bmp");
  surfaceDB.loadSurface("./images/enemyShotNormal.bmp");
  surfaceDB.loadSurface("./images/tankRocket.bmp");
  surfaceDB.loadSurface("./images/tankRocketShadow.bmp", true);
  surfaceDB.loadSurface("./images/heatseeker.bmp");
  surfaceDB.loadSurface("./images/shotNuke.bmp");
  surfaceDB.loadSurface("./images/shotNukeShadow.bmp", true);
  surfaceDB.loadSurface("./images/nukeEffect.bmp");
  surfaceDB.loadSurface("./images/sonic.bmp");
  surfaceDB.loadSurface("./images/itemPrimaryUpgrade.bmp");
  surfaceDB.loadSurface("./images/itemDumbfireDouble.bmp");
  surfaceDB.loadSurface("./images/itemKickAssRocket.bmp");
  surfaceDB.loadSurface("./images/itemHellfire.bmp");
  surfaceDB.loadSurface("./images/itemMachineGun.bmp");
  surfaceDB.loadSurface("./images/itemHealth.bmp");
  surfaceDB.loadSurface("./images/itemHeatseeker.bmp");
  surfaceDB.loadSurface("./images/itemNuke.bmp");
  surfaceDB.loadSurface("./images/itemDeflector.bmp");
  surfaceDB.loadSurface("./images/itemEnergyBeam.bmp");
  surfaceDB.loadSurface("./images/itemLaser.bmp");
  surfaceDB.loadSurface("./images/background.bmp");
  surfaceDB.loadSurface("./images/lightFighterShieldDamaged.bmp");
  surfaceDB.loadSurface("./images/heavyFighterShieldDamaged.bmp");
  surfaceDB.loadSurface("./images/heavyFighterDeflector.bmp", true);
  surfaceDB.loadSurface("./images/font-20red.bmp");
  surfaceDB.loadSurface("./images/font-20blue.bmp");
  surfaceDB.loadSurface("./images/explosion.bmp");
  surfaceDB.loadSurface("./images/explosionEnemy.bmp");
  surfaceDB.loadSurface("./images/bannerExcellent.bmp", true);
  surfaceDB.loadSurface("./images/bannerYouRule.bmp", true);
  surfaceDB.loadSurface("./images/bannerHeiho.bmp", true);
  surfaceDB.loadSurface("./images/bannerHealth.bmp", true);
  surfaceDB.loadSurface("./images/bannerEnemysKilled.bmp", true);
  surfaceDB.loadSurface("./images/bannerBonus100.bmp", true);
  surfaceDB.loadSurface("./images/bannerBonus200.bmp", true);
}

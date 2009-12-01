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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "SDL.h"
#include "SDL_mixer.h"
#include "game.h"
#include "surfaceDB.h"
#include "racers.h"
#include "racer.h"
#include "video.h"
#include "shots.h"
#include "shot.h"
#include "items.h"
#include "explosions.h"
#include "explosion.h"
#include "enemys.h"
#include "font.h"
#include "mixer.h"
#include "input.h"
#include "wrecks.h"
#include "wreck.h"
#include "global.h"
#include "settings.h"
#include "intro.h"
#include "setDifficulty.h"
#include "menuArcadeMode.h"
#include "asstring.h"
#include "sonic.h"
#include "banners.h"
#include "banner.h"
#include "smokePuffs.h"
#include "background.h"
#include "options.h"

Racers *racers = NULL;
Enemys *enemys = NULL;
Shots *shots = NULL;
Explosions *explosions = NULL;
Items *items = NULL;
Wrecks *wrecks = NULL;
Banners *banners = NULL;
SmokePuffs *smokePuffs = NULL;
Options *levelConf = NULL;

bool scrollingOn;
bool nukeIsInPlace;
bool playMusicOn;
bool onePlayerGame;
bool arcadeGame;
int difficultyLevel;
float actBackgroundPos;

Game::Game() {
  videoserver = new Video();
  screen = 0;
  screen = videoserver->init();
  settings = new Settings();
  intro = new Intro( screen );
  setDifficulty = new SetDifficulty( screen );
  menuArcadeMode = new MenuArcadeMode( screen );
  
  pauseSprite = surfaceDB.loadSurface( FN_PAUSED );
  youLoseSprite = surfaceDB.loadSurface( FN_YOU_LOSE );
  youWinSprite = surfaceDB.loadSurface( FN_YOU_WIN );
  // for arcadeMode
  gameOverSprite = surfaceDB.loadSurface( FN_GAME_OVER );

  nukeEffectSurface = surfaceDB.loadSurface( FN_NUKE_EFFECT );

  bossAlarm = mixer.loadSample( FN_SOUND_BOSS_ALARM, 60 );

  fontTime = new Font( FN_FONT_NUMBERS_TIME );
  fontSizeTime = fontTime->getCharWidth();

  racers = 0;
  explosions = 0;
  enemys = 0;
  shots = 0;
  items = 0;
  wrecks = 0;
  banners = 0;
  smokePuffs = 0;
  // needed for calculating fps
  frameCnt = 0;
  tickCnt = 0;

  gameState = GS_INTRO;
  paused = true;
  sdlTicks = SDL_GetTicks();
  gameActRuntime = 0;
  timeNukeEnd = 0;
  timePauseOn = 0;
  timeMinibossOn = 0;

  scrollingOn = true;
  showAllShipStats = false;
  playMusicOn = true;
  onePlayerGame = false;
  arcadeGame = false;
  difficultyLevel = 1;

  sonic1 = new Sonic();
  sonic2 = new Sonic();

  background = new Background();
  loadLevel( FN_LEVEL_ONE_PLAYER );

  SDL_Surface *loadingSprite = surfaceDB.loadSurface( FN_LOADING );
  SDL_Rect dest;
  dest.x = (SCREEN_WIDTH - loadingSprite->w ) / 2;
  dest.y = (SCREEN_HEIGHT - loadingSprite->h ) / 2;
  dest.w = loadingSprite->w;
  dest.h = loadingSprite->h;
  SDL_BlitSurface( loadingSprite, 0, screen, &dest );
  SDL_Flip( screen );
  initAllSurfaces();
}

Game::~Game(){
  if (videoserver) delete videoserver;
  if (settings) delete settings;
  if (intro) delete intro;
  if (setDifficulty) delete setDifficulty;
  if (racers) delete racers;
  if (shots) delete shots;
  if (explosions) delete explosions;
  if (enemys) delete enemys;
  if (items) delete items;
  if (wrecks) delete wrecks;
  if (fontTime) delete fontTime;
  if (sonic1) delete sonic1;
  if (sonic2) delete sonic2;
  if (banners) delete banners;
  if (smokePuffs) delete smokePuffs;
}

void Game::initNewGame() {
  parseGlobalConfigValues( difficultyLevel );

  if (racers) delete racers;
  if (shots) delete shots;
  if (explosions) delete explosions;
  if (enemys) delete enemys;
  if (items) delete items;
  if (wrecks) delete wrecks;
  if (sonic1) delete sonic1;
  if (sonic2) delete sonic2;
  if (banners) delete banners;
  if (smokePuffs) delete smokePuffs;

  banners = new Banners();
  smokePuffs = new SmokePuffs();
  
  racers = new Racers();
  Racer *racer;

  if ( !arcadeGame ) {
    if ( onePlayerGame ) {
      loadLevel( FN_LEVEL_ONE_PLAYER );
      if ( setDifficulty->getPlayerOneLightFighter() ) {
	racer = new Racer( FN_LIGHT_FIGHTER_1, 0,
			   Vector2D(320,350), LIGHT_FIGHTER );
      } else {
	racer = new Racer( FN_HEAVY_FIGHTER_1, 0,
			   Vector2D(320,350), HEAVY_FIGHTER );
      }
      racers->addRacer(racer);
    } 
    // two players
    else {
      loadLevel( FN_LEVEL_TWO_PLAYER );
      if ( setDifficulty->getPlayerOneLightFighter() ) {
	racer = new Racer( FN_LIGHT_FIGHTER_1, 0,
			   Vector2D(250,350), LIGHT_FIGHTER );
      } else {
	racer = new Racer( FN_HEAVY_FIGHTER_1, 0,
			   Vector2D(250,350), HEAVY_FIGHTER );
      }
      racers->addRacer(racer);
    
      if ( setDifficulty->getPlayerTwoLightFighter() ) {
	racer = new Racer( FN_LIGHT_FIGHTER_2, 1,
			   Vector2D(390,350), LIGHT_FIGHTER );
      } else {
	racer = new Racer( FN_HEAVY_FIGHTER_2, 1,
			   Vector2D(390,350), HEAVY_FIGHTER );
      }
      racers->addRacer(racer);
    }
  }
  // arcade game
  else {
    loadLevel( FN_LEVEL_ARCADEMODE );
    if ( menuArcadeMode->getPlayerOneLightFighter() ) {
      racer = new Racer( FN_LIGHT_FIGHTER_1, 0,
			 Vector2D(320,350), LIGHT_FIGHTER );
    } else {
      racer = new Racer( FN_HEAVY_FIGHTER_1, 0,
			 Vector2D(320,350), HEAVY_FIGHTER );
    }
    racers->addRacer(racer);
  }
  
  racers->getKeyActionMaps();

  explosions = new Explosions();
  enemys = new Enemys();
  shots = new Shots();
  items = new Items();
  wrecks = new Wrecks();

  gameActRuntime = 0;
  paused = true;
  bossTime = false;
  bossNukeEffect = false;
  bossExplosion = mixer.loadSample( FN_SOUND_EXPLOSION_BOSS );

  minibossAlreadyKilled = false;
  minibossTime = false;
  timeMinibossOn = SDL_GetTicks();
  timeLastUpdate = SDL_GetTicks();
  timeNukeEnd = SDL_GetTicks();
  timePauseOn = SDL_GetTicks();
  actBackgroundPos = 0;

  scrollingOn = true;
  nukeIsInPlace = false;

  sonic1 = new Sonic();
  sonic2 = new Sonic();
  sonic1->setActive( false );
  sonic2->setActive( false );
}

void Game::run(){
  while( gameState != GS_QUIT ) {
    switch (gameState) {
    case GS_SCREENSHOTS:
      {
	intro->showScreenshots();
	gameState = GS_INTRO;
	break;
      }
    case GS_INTRO: 
      {
	intro->run( gameState );
	break;
      }
    case GS_SET_DIFFICULTY: 
      {
	setDifficulty->run( gameState, onePlayerGame);
	break;
      }
    case GS_ARCADE_MODE_SETUP: 
      {
	menuArcadeMode->run( gameState );
	break;
      }
    case GS_ARCADE_MODE_FINISHED: 
      {
	menuArcadeMode->run( gameState, racers->getPointsArcadeMode() );
	break;
      }
    case GS_OPTIONS: 
      {
	options();
	break;
      }
    case GS_ROUNDFINISHED: 
      {
	roundFinished();
	break;
      }
    case GS_BOSS_KILLED: 
      {
        bossKilled();
        break;
      }
    case GS_PLAYON: 
      {
        initNewGame();
        if ( playMusicOn ) mixer.playMusic( MUSIC_PLAYON, -1, 1000 );
        playOn();
        break;
      }
    default: break;
    }
  }
}


/*********************************/
/**** PlayOn *********************/
/*********************************/

void Game::playOn() {
  int A = SDL_GetTicks();
  frameCnt = 0;
  tickCnt = 0;
  cout << "frameCnt: " << frameCnt << "  tickCnt: " << tickCnt 
       << "  SDL_GetTicks()=" <<  A << endl;

  while ( gameState == GS_PLAYON ) {
    handleEventsPlayOn();
    if (!paused) {
      updateGameState();
    }
    drawPlayOn();
    // miniboss
    if ( minibossTime && enemys->minibossDead() ) minibossAlreadyKilled = true;
    if (!minibossAlreadyKilled && !arcadeGame && !paused && !minibossTime && 
	(unsigned int)GAME_LENGTH / 2 <= gameActRuntime) {
      generateMiniboss();
    }  
    if ( minibossTime && enemys->bossDead() ) minibossKilled();
    // endboss
    if (!arcadeGame && !paused && !bossTime && !minibossTime && 
	(unsigned int)GAME_LENGTH < gameActRuntime) {
      enemys->bossTime(1); // generates the boss
      bossTime = true;
      mixer.playSample( bossAlarm , 0, true );
      if ( playMusicOn ) mixer.playMusic( MUSIC_BOSS1, -1, 0 );
    }
    if ( bossTime && enemys->bossDead() ) gameState = GS_BOSS_KILLED;
    if ( racers->bothPlayersLost() ) gameState = GS_ROUNDFINISHED;
  }

  int B = SDL_GetTicks();
  cout << "frameCnt: " << frameCnt << "  tickCnt: " << tickCnt
       << "  SDL_GetTicks()=" <<  B << endl;
  cout << "Miliseconds: " << B-A << endl;
  cout << "Frames/sec : " << (float)frameCnt / ((float)(B-A) / 1000.0) << endl;
  cout << "ms/Frame   : " << (float)tickCnt / (float)frameCnt << endl;
}


void Game::pause() {
  if (paused) {
    Uint32 timePaused = SDL_GetTicks() - timePauseOn;
    timeLastUpdate += timePaused;
  } else {
    timePauseOn = SDL_GetTicks();
  }
  paused = !paused;
}


void Game::handleEventsPlayOn() {
  SDL_Event event;
  
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_JOYBUTTONDOWN:
      case SDL_KEYDOWN: {
	    if (paused) pause();
        racers->handleEvent(input.translate(event), input.isPressed(event));
        break;
      }
      case SDL_KEYUP: {
        switch(event.key.keysym.sym) {
          case SDLK_F10: {
            pause();
            break;
          }
          case SDLK_F5: {
            videoserver->toggleFullscreen();
            break;
          }
          case SDLK_F6: {
            showAllShipStats = !showAllShipStats;
            break;
          }
          case SDLK_ESCAPE: {
            gameState = GS_INTRO;
            break;
          }
          case SDLK_F1: {
            if (!paused) pause();
            settings->settingsDialog(screen);
            racers->getKeyActionMaps();
            pause();
            break;
          }
          case SDLK_F7: {
            if ( playMusicOn ) {
              playMusicOn = false;
              mixer.stopMusic();
            } else {
              playMusicOn = true;
              if ( bossTime ) mixer.playMusic( MUSIC_BOSS1, -1, 1000 );
              else mixer.playMusic( MUSIC_PLAYON, -1, 1000 );
            }
            break;
          }
	default: break;
        }
      }
      case SDL_JOYAXISMOTION:
      case SDL_JOYBUTTONUP:
        racers->handleEvent(input.translate(event), input.isPressed(event));
        break; 
      case SDL_QUIT: {
        gameState = GS_QUIT;
        break;
      }
      default: break;
    }
  }
}


// what to do in one tick
void Game::updateGameState() {
  int dT = SDL_GetTicks() - timeLastUpdate;
  timeLastUpdate += dT;
  if ( !minibossTime ) gameActRuntime += dT;
  
  if ( nukeIsInPlace ) handleNuke();
  
  enemys->generateEnemys( dT );
  explosions->updateExplosions( dT );

  smokePuffs->update( dT );
  shots->moveAndCollide( dT );
  wrecks->updateWrecks( dT );
  enemys->updateEnemys( dT );
  if ( !arcadeGame ) items->generate( dT );
  items->update( dT );

  racers->moveAndCollide( dT );
  racers->pickUpItems();
  racers->shoot();
  if ( !arcadeGame ) racers->rechargeShield( dT );

  enemys->deleteExpiredEnemys();
  shots->expireShots();
  items->expireItems();
  explosions->expireExplosions();
  wrecks->expireWrecks();
  racers->expireRacers();
  smokePuffs->expireSmokePuffs();
  sonicDeflectorEffect();

  banners->update( dT );
  banners->expireBanners();
  
  if ( scrollingOn ) actBackgroundPos -= SCROLL_SPEED * dT / 1000.0;
  
  if ( arcadeGame ) { 
    racers->receivePointsArcade( ARCADE_POINTS_PER_TEN_SECONDS * dT / 10000.0 );
  }
}

void Game::handleNuke() {
  sonic1->setActive( false );
  sonic2->setActive( false );

  enemys->doNukeDamage();
  shots->deleteAllShots();
  timeNukeEnd = SDL_GetTicks() + NUKE_EFFECT_DURATION;
  nukeIsInPlace = false;
}

void Game::sonicDeflectorEffect() {
  for ( unsigned int i = 0; i < racers->getNrRacers(); i++) {
    if ( racers->getRacer(i)->getShipType() == LIGHT_FIGHTER ) {
      Shot* nearestRocket = shots->getNearestRocket( racers->getRacer(i)->getPos() );
      // a rocket exists and it is in range of the sonic
      if ( nearestRocket != NULL &&
        (nearestRocket->getPos() - racers->getRacer(i)->getPos()).getLength() < 
        RACER_SONIC_ACTIVATION_DIST ) {
        // activate the correct sonic
        if (i == 0) {
          sonic1->setPos( racers->getRacer(i)->getPos(), nearestRocket->getPos() );
          nearestRocket->deflectedBySonicFromPlayer1 = true;
        } else {
          sonic2->setPos( racers->getRacer(i)->getPos(), nearestRocket->getPos() );
          nearestRocket->deflectedBySonicFromPlayer2 = true;
        }
      // no rocket is in sonic-range
      } else {
        // deactivate the sonic
        if (i == 0) {
          sonic1->setActive( false );
        } else {
          sonic2->setActive( false );
        }
      }
    }
  }
}

void Game::drawPlayOn() {
  drawBackground();
  wrecks->draw(screen);
  enemys->drawGroundEnemys(screen);
  shots->drawShadows(screen);
  racers->drawShadows(screen);
  enemys->drawShadows(screen);
  explosions->drawGroundExplosions(screen);

  smokePuffs->draw(screen);
  
  sonic1->draw(screen);
  sonic2->draw(screen);

  shots->drawGroundShots(screen);
  shots->drawGroundAirShots(screen);
  items->draw(screen);
  enemys->drawAirEnemys(screen);
  racers->drawRacers(screen);
  shots->drawAirShots(screen);
  explosions->drawAirExplosions(screen);


  if ( showAllShipStats ) enemys->drawAllStats(screen);
  else {
    if ( bossTime ) {
      enemys->drawBossStats(screen);
      fontTime->drawStr( screen, (screen->w / 2), 5, "BOSS", FONT_ALIGN_CENTERED );
    }  
  }

  if ( SDL_GetTicks() < timeNukeEnd ) {
    drawNukeEffect();
  }

  racers->drawStats(screen);

  banners->draw(screen);
  if ( !arcadeGame && !bossTime && !minibossTime ) drawTime();
  else {
    if ( bossTime || minibossTime ) {
    fontTime->drawStr( screen, (screen->w / 2), 5, "BOSS", FONT_ALIGN_CENTERED );
    }
    else {
      drawPointsArcadeMode();
    }
  }   
 
  if (paused) drawPaused();
  
  SDL_Flip( screen );
   
  frameCnt++;
}

void Game::drawBackground() {
  background->draw(screen, (int) (actBackgroundPos + 0.5) );
}


void Game::drawTime() {
  Uint32 timeToDraw;
  timeToDraw = (GAME_LENGTH - gameActRuntime) / 1000;
  if ( timeToDraw > 0 ) {
    int digitCnt = 1;
    Uint32 i=1;
    while ( timeToDraw >= i*10 ) {
      digitCnt++;
      i *= 10;
    }
    fontTime->drawInt(screen, (screen->w / 2) - (fontSizeTime * digitCnt) / 2, 5, 
		      timeToDraw, digitCnt, 0);
  }  
}


void Game::drawPointsArcadeMode() {
  int pointsToDraw = racers->getPointsArcadeMode();
  if ( pointsToDraw < 0 ) return;

  int digitCnt = 1;
  int i=1;
  while ( pointsToDraw >= i*10 ) {
    digitCnt++;
    i *= 10;
  }
  fontTime->drawInt( screen, (screen->w / 2) - (fontSizeTime * digitCnt) / 2, 10, 
		     pointsToDraw, digitCnt, 0);
}
  
    

void Game::drawPaused() {
  SDL_Rect r;
  r.x = screen->w/2 - pauseSprite->w/2;
  r.y = screen->h/2 - pauseSprite->h/2;
  r.w = pauseSprite->w;
  r.h = pauseSprite->h;
  SDL_BlitSurface( pauseSprite, 0, screen, &r );
}


void Game::drawNukeEffect() {
  // effect-process: transparent -> nearly opaque -> transparent
  int timeFromMaximum = (NUKE_EFFECT_DURATION / 2) - (timeNukeEnd - SDL_GetTicks());
  timeFromMaximum = abs(timeFromMaximum);
  SDL_SetAlpha( nukeEffectSurface, SDL_SRCALPHA | SDL_RLEACCEL,
		lroundf(((NUKE_EFFECT_DURATION / 2) - timeFromMaximum) * 128.0 / 
			(NUKE_EFFECT_DURATION / 2)) );
  SDL_BlitSurface( nukeEffectSurface, 0, screen, 0 );

  int randRange = (int)
    (( ((NUKE_EFFECT_DURATION / 2.0) - timeFromMaximum) * NUKE_QUAKE_EFFECT /
	     (NUKE_EFFECT_DURATION / 2.0 ) ) + 0.5);

  int randX, randY;
  if ( randRange == 0 ) randRange = 1;
  randX = (rand() % randRange) - randRange / 2;
  randY = (rand() % randRange) - randRange / 2;

  SDL_Rect src, dest;
  if ( randX < 0 ) {
    src.x = -randX;    
    src.w = screen->w + randX;
    dest.x = 0;
    dest.w = screen->w + randX;
  } else {
    src.x = 0;
    src.w = screen->w - randX;
    dest.x = randX;
    dest.w = screen->w - randX;
  }
  if ( randY < 0 ) {
    src.y = -randY;    
    src.h = screen->h + randY;
    dest.y = 0;
    dest.h = screen->h + randY;
  } else {
    src.y = 0;
    src.h = screen->h - randY;
    dest.y = randY;
    dest.h = screen->h - randY;
  }

  SDL_BlitSurface( screen, &src, screen, &dest );
}


// not in use - not needed anymore
void Game::timeManagement() {
  Uint32 sdlTicksNew = SDL_GetTicks();
  tickCnt += sdlTicksNew - sdlTicks;
  // 25 Frames per Second is wanted
  // we needed less than 40 ms -> wait till the 40 ms are over
  while ( sdlTicks + 40 > sdlTicksNew ) {
    sdlTicksNew = SDL_GetTicks();
  }
  sdlTicks = sdlTicksNew;
}



/***********************************/
/** Options ************************/
/***********************************/

void Game::options() {}


/************************************/
/** Round Finished ******************/
/************************************/

void Game::roundFinished() {
  drawRoundFinished();
  SDL_Flip( screen );
  while (gameState == GS_ROUNDFINISHED ) {
    handleEventsRoundFinished();
  }
}

void Game::handleEventsRoundFinished() {
  SDL_Event event;
  
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
    case SDL_KEYUP:
      {
        switch(event.key.keysym.sym) {
        case SDLK_ESCAPE:
	case SDLK_SPACE:
	case SDLK_RETURN:
          {
	    if ( arcadeGame ) gameState = GS_ARCADE_MODE_FINISHED;
	    else gameState = GS_INTRO;
            break;
          }
	case SDLK_F5: 
	  {
	    videoserver->toggleFullscreen();
	    break;
	  }
	default:
	  {
	    break;
	  }
	}
	break;
      }
    case SDL_QUIT: 
      {
	if ( arcadeGame ) gameState = GS_ARCADE_MODE_FINISHED;
	else gameState = GS_INTRO;
        break;
      }
    default: break;
    }
  }
}
  
void Game::drawRoundFinished() {
  SDL_Rect r;

  if ( arcadeGame ) {
    r.x = screen->w/2 - gameOverSprite->w / 2;
    r.y = screen->h/2 - gameOverSprite->h / 2;
    r.w = gameOverSprite->w;
    r.h = gameOverSprite->h;
    SDL_BlitSurface( gameOverSprite, 0, screen, &r );

    fontTime->drawStr( screen, screen->w/2, screen->h/2 + gameOverSprite->h/2 + 50, 
		       "SCORE: " + asString( racers->getPointsArcadeMode() ),
		       FONT_ALIGN_CENTERED );
    fontTime->drawStr( screen, screen->w/2, screen->h/2 + gameOverSprite->h/2 + 100, 
		       "Kill Rate: " + asString(enemys->getNrEnemysKilled()) 
		       + "/" + asString(enemys->getNrEnemysGenerated()),
		       FONT_ALIGN_CENTERED );
  } 
  
  // normal game
  else {
    if ( racers->bothPlayersLost() ) {
      r.x = screen->w/2 - youLoseSprite->w / 2;
      r.y = screen->h/2 - youLoseSprite->h / 2;
      r.w = youLoseSprite->w;
      r.h = youLoseSprite->h;
      SDL_BlitSurface( youLoseSprite, 0, screen, &r );
    } else {
      r.x = screen->w/2 - youWinSprite->w / 2;
      r.y = screen->h/2 - youWinSprite->h / 2;
      r.w = youWinSprite->w;
      r.h = youWinSprite->h;
      SDL_BlitSurface( youWinSprite, 0, screen, &r );
      fontTime->drawStr(screen, screen->w/2, screen->h/2 + youWinSprite->h/2 + 50, 
			"Kill Rate: " + asString(enemys->getNrEnemysKilled()) 
			+ ":" + asString(enemys->getNrEnemysGenerated()),
			FONT_ALIGN_CENTERED);
    }
  }
}


/************************************/
/** Miniboss  ***********************/
/************************************/

void Game::generateMiniboss() {
  scrollingOn = false;
  minibossTime = true;
  enemys->bossTime(2); // generates the miniboss
  mixer.playSample( bossAlarm , 0, true );
  if ( playMusicOn ) mixer.playMusic( MUSIC_BOSS1, -1, 0 );
}

void Game::minibossKilled() {
  scrollingOn = true;
  minibossTime = false;
  if ( playMusicOn ) mixer.playMusic( MUSIC_PLAYON, -1, 0 );
}


/************************************/
/** Boss Killed *********************/
/************************************/

void Game::bossKilled() {
  int BOSS_EXPLOSION_DURATION = 3000;
  int startOfBossExplosion = SDL_GetTicks();
  int actualTime;
  bool bossExplosionSound = false;
  timeNukeEnd = SDL_GetTicks() + BOSS_EXPLOSION_DURATION + NUKE_EFFECT_DURATION;

  while ( gameState == GS_BOSS_KILLED ) {
    actualTime = SDL_GetTicks();
    updateBossKilled();
    if ( (actualTime - startOfBossExplosion) < BOSS_EXPLOSION_DURATION ) {
      // explosions
      Explosion *newExplosion = 
      	new Explosion( FN_EXPLOSION_ENEMY,
          Vector2D( rand() % SCREEN_WIDTH, rand() % 150 ),
          Vector2D( 0, 0 ),
          EXPLOSION_NORMAL_GROUND );
      explosions->addExplosion( newExplosion );
    }
    else if ( (actualTime - startOfBossExplosion) < (BOSS_EXPLOSION_DURATION + NUKE_EFFECT_DURATION) ) {
      // nuke effect
      if ( !bossExplosionSound ) {
        mixer.playSample( bossExplosion, 0, true );
        bossExplosionSound = false;
        enemys->doNukeDamage();
        enemys->deleteExpiredEnemys();
        shots->deleteAllShots();
        items->deleteAllItems();
        wrecks->deleteAllWrecks();
        Wreck *newWreck = new Wreck( Vector2D( SCREEN_WIDTH / 2, 70 ), WRECK_BOSS_1_DESTROYED);
        wrecks->addWreck( newWreck );
        bossNukeEffect = true;
      }
    }
    else {
      bossNukeEffect = false;
    }
    if ( (actualTime - startOfBossExplosion) > (BOSS_EXPLOSION_DURATION + NUKE_EFFECT_DURATION + 4000) ) {
      gameState = GS_ROUNDFINISHED;
    }
    drawBossKilled();
    handleEventsBossKilled();
  }
}

void Game::handleEventsBossKilled() {
  SDL_Event event;
  
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
    case SDL_KEYUP: {
      switch(event.key.keysym.sym) {
      case SDLK_ESCAPE:
      case SDLK_SPACE:
      case SDLK_RETURN: 
	{
	  gameState = GS_INTRO;
	  break;
	}
      case SDLK_F5: 
	{
	  videoserver->toggleFullscreen();
	  break;
	}
      default: break;
      }
      break;
    }
    case SDL_QUIT: 
      {
	gameState = GS_INTRO;
	break;
      }
    default: break;
    }
  }
}
  
void Game::drawBossKilled() {
  drawBackground();
  wrecks->draw(screen);
  enemys->drawGroundEnemys(screen);
  shots->drawShadows(screen);
  racers->drawShadows(screen);
  enemys->drawShadows(screen);
  explosions->drawGroundExplosions(screen);
  shots->drawGroundShots(screen);
  shots->drawGroundAirShots(screen);
  items->draw(screen);
  enemys->drawAirEnemys(screen);
  racers->drawRacers(screen);
  shots->drawAirShots(screen);
  explosions->drawAirExplosions(screen);
  if ( bossNukeEffect ) drawNukeEffect();
  racers->drawStats(screen);
  SDL_Flip( screen );
}

void Game::updateBossKilled() {
  int dT = SDL_GetTicks() - timeLastUpdate;
  timeLastUpdate += dT;
  explosions->updateExplosions( dT );
  explosions->expireExplosions();
}


void Game::loadLevel( string fn ) {
  if (levelConf) {
    delete levelConf;
  }
  levelConf = new Options( fn );

  // load background tiles
  background->clearTileList();
  int cnt = 0;
  if (!levelConf->getInt( LVL_BACKG_TILE_CNT, cnt )) {
    cout << "ERROR: " << fn << " contains no '" << LVL_BACKG_TILE_CNT << "' keyword!" << endl;
    exit(0);
  }
  for(int i=0; i < cnt; i++) {
    string tilename;
    if (levelConf->getStr( LVL_BACKG_TILE + asString( i ), tilename ) ) {
      background->addTile( tilename );
    }
  }
  int len = SCREEN_WIDTH;
  levelConf->getInt( LVL_BACKG_LENGTH, len );
  background->generateBackground( len );
}

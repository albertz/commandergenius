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
#ifndef GAME_HH
#define GAME_HH

#include "SDL.h"
#include "SdlForwardCompat.h"
#include <string>


class Video;
class SurfaceDB;
class Racers;
class Shots;
class Explosions;
class Enemys;
class Items;
class Font;
class Intro;
class SetDifficulty;
class MenuArcadeMode;
class Sonic;
class Background;
class Options;

enum GameStates { GS_QUIT, GS_SCREENSHOTS, GS_INTRO, GS_SET_DIFFICULTY,
		  GS_OPTIONS, GS_ARCADE_MODE_SETUP, GS_ARCADE_MODE_FINISHED,
		  GS_PLAYON, GS_ROUNDFINISHED, GS_BOSS_KILLED };

/* The big class, that has to control the gamelogic and keep track of all the
   game objects and their dependencies. */
class Game {
  // Video system
  SdlCompat_AcceleratedSurface *screen;

  SdlCompat_AcceleratedSurface *pauseSprite;
  SdlCompat_AcceleratedSurface *youLoseSprite;
  SdlCompat_AcceleratedSurface *youWinSprite;
  SdlCompat_AcceleratedSurface *gameOverSprite;
  SdlCompat_AcceleratedSurface *nukeEffectSurface;
  SdlCompat_AcceleratedSurface *hud;

  // Time system
  Font *fontTime;
  int fontSizeTime;

  Uint32 frameCnt;
  Uint32 tickCnt;
  Uint32 sdlTicks;
  Uint32 gameActRuntime;
  Uint32 timePauseOn;
  Uint32 timeMinibossOn;
  Uint32 timeLastUpdate;
  Uint32 timeNukeEnd;

  bool paused;

  bool showAllShipStats;

  Background *background;

  // is the game in playon or in intro or somewhere else?
  GameStates gameState;
  Intro *intro;
  SetDifficulty *setDifficulty;
  MenuArcadeMode *menuArcadeMode;
  
  // boss specific
  bool bossTime;
  bool minibossTime;
  bool minibossAlreadyKilled;
  bool bossNukeEffect;
  int bossExplosion;
  int bossAlarm;
  
  Sonic *sonic1;
  Sonic *sonic2;

  public:
  // for access from main()
  Game();
  ~Game();
  // the outer game loop
  void run();

  private:
  // starts a new game
  void initNewGame();

  /* Methods for GS_PLAYON */
  // the inner loop when the race is going on
  void playOn();
  void handleEventsPlayOn();
  // toggles the pause state (takes into account the corresponding time problems)
  void pause();
  void updateGameState();
  void handleNuke();
  void sonicDeflectorEffect();
  void drawPlayOn();
  void drawBackground();
  void drawTime();
  void drawPointsArcadeMode();
  void drawPaused();
  void drawNukeEffect();
  void timeManagement(); // not needed and used any more


  /* Methods for GS_OPTIONS (not implemented by now) */
  void options();
  void loadLevel( std::string fn );
  
  /* Methods for GS_ROUNDFINISHED (the time is up -> display the winner) */
  void roundFinished();
  void drawRoundFinished();
  void handleEventsRoundFinished();

  void generateMiniboss();
  void minibossKilled();
  
  void bossKilled();
  void updateBossKilled();
  void drawBossKilled();
  void handleEventsBossKilled();
};

#endif //#ifndef GAME_HH


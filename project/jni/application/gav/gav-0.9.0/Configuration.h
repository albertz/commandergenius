/* -*- C++ -*- */
/*
  GAV - Gpl Arcade Volleyball
  
  Copyright (C) 2002
  GAV team (http://sourceforge.net/projects/gav/)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* Configuration options */

#include <string>
#include <sstream>
#include "aarg.h"

#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#define MAX_PLAYERS (4)
#define DEFAULT_FPS (50)
#define DEFAULT_WINNING_SCORE (15)

#define DEFAULT_BALL_AMPLIFY 5

#define DEFAULT_FRAME_SKIP 0

#define DEFAULT_THEME "classic"

#define DEFAULT_FULLSCREEN false

#define DEFAULT_SOUND true

#define DEFAULT_NPLAYERFRAMES 4
#define DEFAULT_PLAYERSTILLB  1
#define DEFAULT_PLAYERSTILLE  1
#define DEFAULT_PLAYERSTILLP  0
#define DEFAULT_PLAYERRUNB    2
#define DEFAULT_PLAYERRUNE    3
#define DEFAULT_PLAYERRUNP    250
#define DEFAULT_PLAYERJMPB    4
#define DEFAULT_PLAYERJMPE    4
#define DEFAULT_PLAYERJMPP    0

#define DEFAULT_NBALLFRAMES   4
#define DEFAULT_BALLPERIOD    1000

#define BALL_SPEED_INC        3

#define DEFAULT_CONF_FILENAME     ".gav"
#define ALTERNATIVE_CONF_FILENAME "gav.ini"

#define ENVIRONMENT_WIDTH  (640)
#define ENVIRONMENT_HEIGHT (400)

#define BIG_ENVIRONMENT_WIDTH  (1000)
#define BIG_ENVIRONMENT_HEIGHT (400)


enum { PLAYER_NONE, PLAYER_HUMAN, PLAYER_COMPUTER};
enum { MONITOR_NORMAL, MONITOR_OLD, MONITOR_VERYOLD, MONITOR_VERYVERYOLD};

typedef struct PlayerFrameConf_s {
  unsigned short nPlayerFrames;
  unsigned short playerStillB;
  unsigned short playerStillE;
  unsigned short playerStillP;
  unsigned short playerRunB;
  unsigned short playerRunE;
  unsigned short playerRunP;
  unsigned short playerJmpB;
  unsigned short playerJmpE;
  unsigned short playerJmpP;
} PlayerFrameConf_t;

typedef struct BallFrameConf_s {
  unsigned short nBallFrames;
  unsigned short ballPeriod;
} BallFrameConf_t;

typedef struct Resolution_s {
  unsigned short x;
  unsigned short y;
  float ratioX;
  float ratioY;
} Resolution_t;

typedef struct Environment_s {
  unsigned short w;
  unsigned short h;
} Environment_t;

class Configuration {
public:
  int left_nplayers;
  int right_nplayers;
  int left_players[MAX_PLAYERS/2];
  int right_players[MAX_PLAYERS/2];
  PlayerFrameConf_t playerFrameConf;
  BallFrameConf_t ballFrameConf;
  Resolution_t resolution;
  Resolution_t desiredResolution;
  Environment_t env;
  std::string currentTheme;
  
  /* Constants that depend on the screen size */
  int SCREEN_WIDTH;
  int SCREEN_HEIGHT;
  float SPEEDY;
  int FLOOR_ORD;
  int SPEED_MULTIPLIER;
  int NET_X;
  int NET_Y;
  int CEILING;
  int LEFT_WALL;
  int RIGHT_WALL;
  int DEFAULT_SPEED;

  /* To add: something meaningful to record the controls... */

  bool sound;
  int winning_score;

  int monitor_type;

  unsigned int frame_skip;     // one every frame_skip + 1 are actually drawn
  unsigned int fps;            // fps of the update (not graphical)
  unsigned int mill_per_frame; // caches the # of msecs per frame (1000/fps)
  bool bgBig;                  // if the background is big
  bool fullscreen;
  unsigned int ballAmplify;

  Configuration() : left_nplayers(1), right_nplayers(1),
		    sound(DEFAULT_SOUND),
		    winning_score(DEFAULT_WINNING_SCORE) {
    monitor_type = MONITOR_NORMAL;
    frame_skip = DEFAULT_FRAME_SKIP;
    fps = DEFAULT_FPS;
    mill_per_frame = 1000 / fps;
    left_players[0] = PLAYER_HUMAN;
    right_players[0] = PLAYER_COMPUTER;
    for ( int i = 1; i < MAX_PLAYERS/2; i++ ) {
      left_players[i] = PLAYER_NONE;
      right_players[i] = PLAYER_NONE;
    }
    bgBig = false;
    fullscreen = DEFAULT_FULLSCREEN;
    ballAmplify = DEFAULT_BALL_AMPLIFY;
    setDefaultFrameConf();
    currentTheme = "classic";
    scaleFactors(ENVIRONMENT_WIDTH, ENVIRONMENT_HEIGHT);
    env.w = ENVIRONMENT_WIDTH;
    env.h = ENVIRONMENT_HEIGHT;
    setDesiredResolution(ENVIRONMENT_WIDTH, ENVIRONMENT_HEIGHT);
    setResolution(ENVIRONMENT_WIDTH, ENVIRONMENT_HEIGHT);
  }

  void scaleFactors(int width, int height) {
    SCREEN_WIDTH = width;
    SCREEN_HEIGHT = height;
    SPEEDY = ((float) SCREEN_HEIGHT / 2.5);
    FLOOR_ORD = SCREEN_HEIGHT -(SCREEN_HEIGHT / 200);
    NET_X = width / 2 - width / 80;
    NET_Y = height / 2 + ( 3*height / 200 );
    CEILING = (int) (height / 17);
    LEFT_WALL = (int) (width / 80);
    RIGHT_WALL = (int) (width - width / 40);
    DEFAULT_SPEED = (int) (bgBig)?(width/4):(25*width/64);
  }

  inline void setResolution(int w, int h) {
    resolution.x = w;
    resolution.y = h;
    resolution.ratioX = (float) resolution.x / (float) env.w;
    resolution.ratioY = (float) resolution.y / (float) env.h;
  }

  inline void setResolutionToDesired() {
    setResolution(desiredResolution.x, desiredResolution.y);
  }

  inline void setDesiredResolution(int w, int h) {
    desiredResolution.x = w;
    desiredResolution.y = h;
    desiredResolution.ratioX = (float) desiredResolution.x / (float) env.w;
    desiredResolution.ratioY = (float) desiredResolution.y / (float) env.h;
  }

  inline void setDefaultFrameConf() {
    playerFrameConf.nPlayerFrames = DEFAULT_NPLAYERFRAMES;
    playerFrameConf.playerStillB = DEFAULT_PLAYERSTILLB;
    playerFrameConf.playerStillE = DEFAULT_PLAYERSTILLE;
    playerFrameConf.playerStillP = DEFAULT_PLAYERSTILLP;
    playerFrameConf.playerRunB = DEFAULT_PLAYERRUNB;
    playerFrameConf.playerRunE = DEFAULT_PLAYERRUNE;
    playerFrameConf.playerRunP = DEFAULT_PLAYERRUNP;
    playerFrameConf.playerJmpB = DEFAULT_PLAYERJMPB;
    playerFrameConf.playerJmpE = DEFAULT_PLAYERJMPE;
    playerFrameConf.playerJmpP = DEFAULT_PLAYERJMPP;

    ballFrameConf.nBallFrames = DEFAULT_NBALLFRAMES;
    ballFrameConf.ballPeriod = DEFAULT_BALLPERIOD;
  }

  inline void setFps(int val) {
    fps = val;
    mill_per_frame = 1000 / val;
  }

  std::string toString(int v) {
    std::ostringstream os;
    
    os << v;
    
    return os.str();
  }

  int loadConfiguration();
  int saveConfiguration(std::string fname);
  int createConfigurationFile();
  std::string confFileName();
  //void scaleFactors(int width, int height);
};

#endif

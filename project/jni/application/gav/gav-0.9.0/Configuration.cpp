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

#include <stdlib.h>
#include <string>
#include <iostream>
#include "Configuration.h"
#include "ControlsArray.h"

using namespace std;

Configuration configuration;

Aargh aargh;

int Configuration::loadConfiguration() {
  string fname = confFileName();
  
  if ( !aargh.loadConf(fname.c_str()) )
    return(-1);
  string value;
  if ( aargh.getArg("left_nplayers", value) )
    configuration.left_nplayers = atoi(value.c_str());
  if ( aargh.getArg("right_nplayers", value) )
    configuration.right_nplayers = atoi(value.c_str());
  if ( aargh.getArg("monitor_type", value) ) {
    if ( value == "normal" )
      configuration.monitor_type = MONITOR_NORMAL;
    else if ( value == "old" )
      configuration.monitor_type = MONITOR_OLD;
    else if ( value == "very_old")
      configuration.monitor_type = MONITOR_VERYOLD;
    else if ( value == "very_very_old")
      configuration.monitor_type = MONITOR_VERYVERYOLD;
    else
      cerr << "Unrecognized monitor type \"" << value << "\"\n";
  }
  if ( aargh.getArg("frame_skip", value) )
    configuration.frame_skip = atoi(value.c_str());
  if ( aargh.getArg("fps", value) )
    configuration.setFps(atoi(value.c_str()));

  for ( int i = 0; i < left_nplayers; i++ )
    if ( aargh.getArg(("left_player" + toString(i)), value) )
      if ( value ==  "human")
	configuration.left_players[i] = PLAYER_HUMAN;
      else if ( value == "computer")
	configuration.left_players[i] = PLAYER_COMPUTER;
      else
	cerr << "Unrecognized player type \"" << value << "\"\n";
  
  for ( int i = 0; i < MAX_PLAYERS/2; i++ ) {
    string arg1, arg2, arg3;

    arg1 = "left_player" + toString(i) + "left";
    arg2 = "left_player" + toString(i) + "right";
    arg3 = "left_player" + toString(i) + "jump";
    if ( aargh.getArg(arg1) && aargh.getArg(arg2) && aargh.getArg(arg3) ) {
      string lval, rval, jval;
      controls_t ctrls;
      aargh.getArg(arg1, lval);
      aargh.getArg(arg2, rval);
      aargh.getArg(arg3, jval);
      ctrls.left_key = atoi(lval.c_str());
      ctrls.right_key = atoi(rval.c_str());
      ctrls.jump_key = atoi(jval.c_str());
      controlsArray->setControls(i*2, ctrls);
    }
  }

  for ( int i = 0; i < right_nplayers; i++ )
    if ( aargh.getArg(("right_player" + toString(i)), value) )
      if ( value == "human" )
	configuration.right_players[i] = PLAYER_HUMAN;
      else if ( value == "computer")
	configuration.right_players[i] = PLAYER_COMPUTER;
      else
	cerr << "Unrecognized player type \"" << value << "\"\n";
  
  for ( int i = 0; i < MAX_PLAYERS/2; i++ ) {
    string arg1, arg2, arg3;
    
    arg1 = "right_player" + toString(i) + "left";
    arg2 = "right_player" + toString(i) + "right";
    arg3 = "right_player" + toString(i) + "jump";
    if ( aargh.getArg(arg1) && aargh.getArg(arg2) && aargh.getArg(arg3) ) {
      string lval, rval, jval;
      controls_t ctrls;
      aargh.getArg(arg1, lval);
      aargh.getArg(arg2, rval);
      aargh.getArg(arg3, jval);
      ctrls.left_key = atoi(lval.c_str());
      ctrls.right_key = atoi(rval.c_str());
      ctrls.jump_key = atoi(jval.c_str());
      controlsArray->setControls(i*2 + 1, ctrls);
    }
  }
  if ( aargh.getArg("big_background", value) )
    configuration.bgBig = (value=="yes");
  if ( aargh.getArg("fullscreen", value) )
    configuration.fullscreen = (value=="yes");
  if ( aargh.getArg("ball_speed", value) )
    if ( value == "normal" )
      configuration.ballAmplify = DEFAULT_BALL_AMPLIFY;
    else if ( value == "fast" )
      configuration.ballAmplify = DEFAULT_BALL_AMPLIFY + BALL_SPEED_INC;
    else if ( value == "very_fast" )
      configuration.ballAmplify = DEFAULT_BALL_AMPLIFY + 2*BALL_SPEED_INC;
    else
      cerr << "Unrecognized ball speed \"" << value << "\"\n";
  if ( aargh.getArg("theme", value) )
    configuration.currentTheme = value;

  if ( aargh.getArg("sound", value) )
    configuration.sound = (value == "yes");

  if ( aargh.getArg("winning_score", value) )
    configuration.winning_score = atoi(value.c_str());

  return 0;
}

/* we'll use aargh's dump feature! Yiipeee!! */
int Configuration::saveConfiguration(string fname) {
  // cerr << "saving to: " << fname << endl;

  string tosave;
  
  aargh.dump(tosave);
  
  FILE *fp;
  if ( (fp = fopen(fname.c_str(), "w")) == NULL )
    return(-1);
  
  fputs(tosave.c_str(), fp);
  fclose(fp);
  return 0;
}

/* unfortunately, I *HAVE* to go through all the settings...
   This function puts configuration parameters inside aargh, and then
   dumps it. */
int Configuration::createConfigurationFile() {
  string fname = confFileName();

  Configuration c = configuration; /* for short :) */

  aargh.reset();

  aargh.setArg("left_nplayers", c.left_nplayers);
  aargh.setArg("right_nplayers", c.right_nplayers);
  switch ( c.monitor_type ) {
  case MONITOR_NORMAL:
    aargh.setArg("monitor_type", "normal"); break;
  case MONITOR_OLD:
    aargh.setArg("monitor_type", "old"); break;
  case MONITOR_VERYOLD:
    aargh.setArg("monitor_type", "very_old"); break;
  case MONITOR_VERYVERYOLD:
    aargh.setArg("monitor_type", "very_very_old"); break;
  }
  aargh.setArg("frame_skip", c.frame_skip);
  aargh.setArg("fps", c.fps);
  for ( int i = 0; i < left_nplayers; i++ ) {
    switch ( c.left_players[i] ) {
    case PLAYER_HUMAN:
      aargh.setArg("left_player" + toString(i), "human"); break;
    case PLAYER_COMPUTER:
      aargh.setArg("left_player" + toString(i), "computer"); break;
    } 
  }
  for ( int i = 0; i < right_nplayers; i++ ) {
    switch ( c.right_players[i] ) {
    case PLAYER_HUMAN:
      aargh.setArg("right_player" + toString(i), "human"); break;
    case PLAYER_COMPUTER:
      aargh.setArg("right_player" + toString(i), "computer"); break;
    }
  }
  aargh.setArg("big_background", c.bgBig?"yes":"no");
  aargh.setArg("fullscreen", (c.fullscreen?"yes":"no"));
  if ( c.ballAmplify == DEFAULT_BALL_AMPLIFY + BALL_SPEED_INC )
    aargh.setArg("ball_speed", "fast");
  else if ( c.ballAmplify == DEFAULT_BALL_AMPLIFY + 2*BALL_SPEED_INC )
    aargh.setArg("ball_speed", "very_fast");
  else if ( c.ballAmplify == DEFAULT_BALL_AMPLIFY )
    aargh.setArg("ball_speed", "normal");
  aargh.setArg("theme", c.currentTheme);
  aargh.setArg("sound", c.sound?"yes":"no");
  aargh.setArg("winning_score", c.winning_score);

  for ( int i = 0; i < MAX_PLAYERS/2; i++ ) {
    controls_t ct = controlsArray->getControls(i*2);
    aargh.setArg("left_player" + toString(i) + "left", ct.left_key);
    aargh.setArg("left_player" + toString(i) + "right", ct.right_key);
    aargh.setArg("left_player" + toString(i) + "jump", ct.jump_key);
  } 
   
  for ( int i = 0; i < MAX_PLAYERS/2; i++ ) {
    controls_t ct = controlsArray->getControls(i*2 + 1);
    aargh.setArg("right_player" + toString(i) + "left", ct.left_key);
    aargh.setArg("right_player" + toString(i) + "right", ct.right_key);
    aargh.setArg("right_player" + toString(i) + "jump", ct.jump_key);
  }
  
  return(saveConfiguration(fname));
}

string Configuration::confFileName() {
  string ret;
  const char *home = getenv("HOME");
  
  if ( home ) {
    ret = (string(home) + "/" + DEFAULT_CONF_FILENAME);
  } else { /* gav.ini in the current directory */
    ret = string(ALTERNATIVE_CONF_FILENAME);
  }

  return ret;
}

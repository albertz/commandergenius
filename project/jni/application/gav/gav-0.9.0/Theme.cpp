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

#ifndef WIN32
#include <unistd.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif /* WIN32 */
#include <string>
#include <iostream>
#include "Theme.h"

using namespace std;

Theme *CurrentTheme;
string ThemeDir;

void errorOn(string file) {
    throw Theme:: ThemeErrorException("Error accessing file " + file);
}

bool Theme::_checkTheme() {
    bool r;

    _hasConfFile = true;
#ifndef WIN32
    cerr << "Verifying Theme `" << _name << "' [" << ThemeDir << "/" << _name << "/]:\n";
    if ( access(_CCS(_background), R_OK) ) {
      if (!_bigBackground)
	_background = TD + TH_BACKGROUND_JPG;
      else
	_background = TD + TH_BACKGROUND_BIG_JPG;
      if ( access(_CCS(_background), R_OK) ) errorOn("background.{jpg,png}");
    }
    if ( access(_CCS(_font), R_OK) )        errorOn(TH_FONT);
    if ( access(_CCS(_fontinv), R_OK) )     errorOn(TH_FONTINV);
    if ( access(_CCS(_leftmale), R_OK) )    errorOn(TH_LEFTMALE);
    if ( access(_CCS(_rightmale), R_OK) )   errorOn(TH_RIGHTMALE);
    if ( access(_CCS(_leftfemale), R_OK) )  errorOn(TH_LEFTFEMALE);
    if ( access(_CCS(_rightfemale), R_OK) ) errorOn(TH_RIGHTFEMALE);
    if ( access(_CCS(_ball), R_OK) )        errorOn(TH_BALL);
    if ( access(_CCS(_confFile), R_OK) )    _hasConfFile = false;

    r = (access(_CCS(_net), R_OK) == 0);
#else
    struct _stat sStat ;
    
    cerr << "Verifying Theme `" << _name << "' [" << ThemeDir << "\\" << _name << "\\]:\n";
    if (_stat (_background.c_str(), &sStat)) {
      if (!_bigBackground)
	_background = TD + TH_BACKGROUND_JPG;
      else
	_background = TD + TH_BACKGROUND_BIG_JPG;
      if (_stat (_background.c_str(), &sStat)) errorOn("background.{jpg,png}");
    }

    if (_stat (_font.c_str(), &sStat))				errorOn (TH_FONT) ;
    if (_stat (_fontinv.c_str(), &sStat))			errorOn (TH_FONTINV) ;
    if (_stat (_leftmale.c_str(), &sStat))			errorOn (TH_LEFTMALE) ;
    if (_stat (_rightmale.c_str(), &sStat))			errorOn (TH_RIGHTMALE) ;
    if (_stat (_leftfemale.c_str(), &sStat))		errorOn (TH_LEFTFEMALE) ;
    if (_stat (_rightfemale.c_str(), &sStat))		errorOn (TH_RIGHTFEMALE) ;
    if (_stat (_ball.c_str(), &sStat))				errorOn (TH_BALL) ;
    
    if ( _stat(_confFile.c_str(), &sStat) ) _hasConfFile = false;

    r = (_stat (_net.c_str(), &sStat) == 0) ;
#endif /* WIN32 */
    
    if ( !r ) cerr << "Warning: No net for this theme!\n";

    cerr << "OK!\n";

    configuration.setDefaultFrameConf();
    if ( _hasConfFile ) {
      cerr << "Using configuration file theme.conf\n";
      loadConf();
    } else {
      cerr << "No theme.conf\n";
    }

    return(r);
}

void Theme::loadConf()
{
  Aargh myAargh; /* I want a new one, 'cause the global one is used by
		    configuration */

  myAargh.loadConf(_confFile.c_str());
  /* now set things up in configuration */
  string value;
  if ( myAargh.getArg("NPlayerFrames", value) )
    configuration.playerFrameConf.nPlayerFrames = atoi(value.c_str());
  if ( myAargh.getArg("PlayerStillB", value) )
    configuration.playerFrameConf.playerStillB = atoi(value.c_str());
  if ( myAargh.getArg("PlayerStillE", value) )
    configuration.playerFrameConf.playerStillE = atoi(value.c_str());
  if ( myAargh.getArg("PlayerStillP", value) )
    configuration.playerFrameConf.playerStillP = atoi(value.c_str());
  if ( myAargh.getArg("PlayerRunB", value) )
    configuration.playerFrameConf.playerRunB = atoi(value.c_str());
  if ( myAargh.getArg("PlayerRunE", value) )
    configuration.playerFrameConf.playerRunE = atoi(value.c_str());
  if ( myAargh.getArg("PlayerRunP", value) )
    configuration.playerFrameConf.playerRunP = atoi(value.c_str());
  if ( myAargh.getArg("PlayerJumpB", value) )
    configuration.playerFrameConf.playerJmpB = atoi(value.c_str());
  if ( myAargh.getArg("PlayerJumpE", value) )
    configuration.playerFrameConf.playerJmpE = atoi(value.c_str());
  if ( myAargh.getArg("PlayerJumpP", value) )
    configuration.playerFrameConf.playerJmpP = atoi(value.c_str());
  if ( myAargh.getArg("NBallFrames", value) )
    configuration.ballFrameConf.nBallFrames = atoi(value.c_str());
  if ( myAargh.getArg("BallPeriod", value) )
    configuration.ballFrameConf.ballPeriod = atoi(value.c_str());
}

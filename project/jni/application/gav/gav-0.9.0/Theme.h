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

#ifndef _THEMES_H_
#define _THEMES_H_
#include <string>
#include <iostream>

#include <sys/types.h>
#ifndef WIN32
#include <dirent.h>
#else
#include <windows.h>
#endif /* WIN32 */

#include <SDL.h>
#include <SDL_image.h>

#include "SoundMgr.h"
#include "globals.h"
#include "GameRenderer.h"
#include "Menu.h"
#include "MenuItem.h"
#include "MenuItemMonitor.h"

#define TH_DIR "themes"

#define TH_NET          "net.png"
#define TH_LEFTMALE     "plml.png"
#define TH_RIGHTMALE    "plmr.png"
#define TH_LEFTFEMALE   "plfl.png"
#define TH_RIGHTFEMALE  "plfr.png"
#define TH_BACKGROUND_JPG   "background.jpg"
#define TH_BACKGROUND_PNG   "background.png"
#define TH_BACKGROUND_BIG_JPG   "background_big.jpg"
#define TH_BACKGROUND_BIG_PNG   "background_big.png"
#define TH_BALL         "ball.png"
#define TH_FONT         "Font.png"
#define TH_FONTINV      "FontInv.png"
#define TH_CONFNAME     "theme.conf"

extern std::string ThemeDir;

class Theme {
private:
  std::string _name;
  bool _hasnet; // To possibly add the image of the net (not used yet)
  std::string _background;
  std::string _net;
  std::string _font;
  std::string _fontinv;
  std::string _leftmale;
  std::string _rightmale;
  std::string _leftfemale;
  std::string _rightfemale;
  std::string _ball;
  std::string _confFile;
  std::string TD;
  bool _hasConfFile;
  bool _bigBackground;
  bool _checkTheme(); // Theme Validation
    
 public:
    Theme(std::string name) {
#ifndef WIN32
      DIR *dir;
      if ((dir = opendir(ThemeDir.c_str())) == NULL) {
	ThemeDir = "/usr/share/games/gav/" + ThemeDir;
	if ((dir = opendir(ThemeDir.c_str())) == NULL) {
	  std::cerr << "Cannot find themes directory\n";
	  exit(0);
	} else
	  closedir(dir);
      } else
	closedir(dir);

      configuration.currentTheme = name;
      TD = ThemeDir + "/" + name +  "/";

#else
      HANDLE hFindFile ;
      WIN32_FIND_DATA ffdData ;
      
      hFindFile = FindFirstFile (ThemeDir.c_str(), &ffdData) ;
      if (hFindFile == INVALID_HANDLE_VALUE)
	{
	  std::cerr << "Cannot find themes directory\n" ;
	  exit(0) ;
	}
      
      FindClose (hFindFile) ;
      
      TD = ThemeDir + "\\" + name + "\\" ;
      
#endif /* WIN32 */
      
      _name = name;

      _bigBackground = configuration.bgBig;
      
      if ( _bigBackground ) {
	printf("Big Background hack: FIX IT!\n");
	configuration.env.w = BIG_ENVIRONMENT_WIDTH;
	configuration.env.h = BIG_ENVIRONMENT_HEIGHT;
	double rat = ((double) configuration.desiredResolution.y) /
	  (double) BIG_ENVIRONMENT_HEIGHT;
	int w = (int) (rat * BIG_ENVIRONMENT_WIDTH);
	int h = configuration.desiredResolution.y;
	configuration.setResolution(w, h);
	configuration.scaleFactors(BIG_ENVIRONMENT_WIDTH,
				   BIG_ENVIRONMENT_HEIGHT);
      } else {
	configuration.env.w = ENVIRONMENT_WIDTH;
	configuration.env.h = ENVIRONMENT_HEIGHT;
	configuration.setResolutionToDesired();
	configuration.scaleFactors(ENVIRONMENT_WIDTH, ENVIRONMENT_HEIGHT);
      }

      _net = TD + TH_NET;
      
      if (!_bigBackground)
	_background = TD + TH_BACKGROUND_PNG;
      else
	_background = TD + TH_BACKGROUND_BIG_PNG;
      
      _font    = TD + TH_FONT;
      _fontinv = TD + TH_FONTINV;
      
      _leftmale    = TD + TH_LEFTMALE;
      _rightmale   = TD + TH_RIGHTMALE;
      _leftfemale  = TD + TH_LEFTFEMALE;
      _rightfemale = TD + TH_RIGHTFEMALE;
      _confFile    = TD + TH_CONFNAME;
      
      _ball = TD + TH_BALL;
      
      _hasnet = Theme::_checkTheme();
      
      // ::background = IMG_Load(background());
      // TODO: fetch the environment size (and therefore the ratios)
      //       from the background image

      //if ( CurrentTheme->hasnet() ) IMG_Load(CurrentTheme->net());
      
      //screenFlags = SDL_DOUBLEBUF|SDL_HWSURFACE;
      //screenFlags |= SDL_DOUBLEBUF;
      screen = SDL_SetVideoMode(configuration.resolution.x,
				configuration.resolution.y,
				videoinfo->vfmt->BitsPerPixel,
				screenFlags);
      
      ::background = new LogicalFrameSeq(background(), 1, false); 

      gameRenderer = new GameRenderer(configuration.resolution.x,
				      configuration.resolution.y,
				      configuration.env.w,
				      configuration.env.h);

      cga = new ScreenFont(font(), FONT_FIRST_CHAR, FONT_NUMBER);
      cgaInv = new ScreenFont(fontinv(), FONT_FIRST_CHAR, FONT_NUMBER);
      
      MenuItemMonitor().apply();

#ifdef AUDIO
      if ( soundMgr )
	delete(soundMgr);
      soundMgr = new SoundMgr((TD+"sounds").c_str(),
			      (ThemeDir+"/../sounds").c_str());
#endif // AUDIO
    }
  
  ~Theme() {
    delete(::background);
    delete(cga);
    delete(cgaInv);
    delete(gameRenderer);
  }

#define _CCS(str) ((str).c_str())

    inline const char * name()       { return( _CCS(_name)        );}
    inline bool   hasnet()           { return( _hasnet            );}
    inline bool   bigBackground()    { return( _bigBackground     );}
    inline const char * background() { return( _CCS(_background)  );}
    inline const char * net()        { return( _CCS(_net)         );}
    inline const char * font()       { return( _CCS(_font)        );}
    inline const char * fontinv()    { return( _CCS(_fontinv)     );}
    inline const char * leftmale()   { return( _CCS(_leftmale)    );}
    inline const char * rightmale()  { return( _CCS(_rightmale)   );}
    inline const char * leftfemale() { return( _CCS(_leftfemale)  );}
    inline const char * rightfemale(){ return( _CCS(_rightfemale) );}
    inline const char * ball()       { return( _CCS(_ball)        );}
  
  void loadConf();

  class ThemeErrorException {
  public:
    std::string message;
    ThemeErrorException(std::string msg) {
      message = msg;
    }
  };
};


extern Theme *CurrentTheme;


inline void setThemeDir(std::string h) { ThemeDir = h; }
#endif

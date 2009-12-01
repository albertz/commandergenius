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

#include <cassert>
#include <string>
#include <map>
#include "SDL.h"
#include "options.h"
#include "settings.h"
#include "global.h"
#include "surfaceDB.h"
#include "asstring.h"
#include "font.h"
#include "video.h"
#include "mixer.h"
#include "input.h"

Settings *settings;

Settings::Settings() {
  opfile = NULL;

  introSprite = surfaceDB.loadSurface( FN_ALIENBLASTER_INTRO );
  activeChoiceSprite = surfaceDB.loadSurface( FN_INTRO_SHOW_CHOICE );
  bluePlain = surfaceDB.loadSurface( FN_SETTINGS_BLUE, true );
  whitePlain = surfaceDB.loadSurface( FN_SETTINGS_WHITE, false );

  fontMenu = new Font ( FN_FONT_SETTINGS );
  fontMenuHighlighted = new Font ( FN_FONT_SETTINGS_HIGHLIGHTED );
  fontNormal = new Font( FN_FONT_SETTINGS_SMALL );
  fontKey = new Font ( FN_FONT_SETTINGS_SMALL_BLUE );
  fontHighlighted = new Font( FN_FONT_SETTINGS_SMALL_HIGHLIGHTED );

  playerEventNames[ PE_UNKNOWN ] = "UNKNOWN"; 
  playerEventNames[ PE_UP ]    = "UP"; 
  playerEventNames[ PE_DOWN ]  = "DOWN"; 
  playerEventNames[ PE_LEFT ]  = "LEFT"; 
  playerEventNames[ PE_RIGHT ] = "RIGHT";
  playerEventNames[ PE_FIRE_WEAPONS ]  = "FIRE"; 
  playerEventNames[ PE_FIRE_SPECIALS ] = "FIRE-SPECIAL";
  playerEventNames[ PE_CHOOSE_WEAPON_SECONDARY ] = "CHOOSE-WEAPON"; 
  playerEventNames[ PE_CHOOSE_WEAPON_SPECIALS ]  = "CHOOSE-SPECIALS"; 
  
  defaultSettings[ string("PLAYER0-") + playerEventNames[ PE_UP ] ] = SDLK_UP;
  defaultSettings[ string("PLAYER0-") + playerEventNames[ PE_DOWN ] ] = SDLK_DOWN;
  defaultSettings[ string("PLAYER0-") + playerEventNames[ PE_LEFT ] ] = SDLK_LEFT;
  defaultSettings[ string("PLAYER0-") + playerEventNames[ PE_RIGHT ] ] = SDLK_RIGHT;
  defaultSettings[ string("PLAYER0-") + playerEventNames[ PE_CHOOSE_WEAPON_SECONDARY ] ] = SDLK_RSHIFT;
  defaultSettings[ string("PLAYER0-") + playerEventNames[ PE_CHOOSE_WEAPON_SPECIALS ] ] = SDLK_PERIOD;
  defaultSettings[ string("PLAYER0-") + playerEventNames[ PE_FIRE_WEAPONS ] ] = SDLK_RCTRL;
  defaultSettings[ string("PLAYER0-") + playerEventNames[ PE_FIRE_SPECIALS ] ] = SDLK_MODE;
  
  defaultSettings[ string("PLAYER1-") + playerEventNames[ PE_UP ] ] = SDLK_e;
  defaultSettings[ string("PLAYER1-") + playerEventNames[ PE_DOWN ] ] = SDLK_d;
  defaultSettings[ string("PLAYER1-") + playerEventNames[ PE_LEFT ] ] = SDLK_s;
  defaultSettings[ string("PLAYER1-") + playerEventNames[ PE_RIGHT ] ] = SDLK_f;
  defaultSettings[ string("PLAYER1-") + playerEventNames[ PE_CHOOSE_WEAPON_SECONDARY ] ] = SDLK_LSHIFT;
  defaultSettings[ string("PLAYER1-") + playerEventNames[ PE_CHOOSE_WEAPON_SPECIALS ] ] = SDLK_y;
  defaultSettings[ string("PLAYER1-") + playerEventNames[ PE_FIRE_WEAPONS ] ] = SDLK_LCTRL;
  defaultSettings[ string("PLAYER1-") + playerEventNames[ PE_FIRE_SPECIALS ] ] = SDLK_LALT;

  setKeyNames();

  loadSettings();
}


Settings::~Settings() {
  if (fontKey) delete fontKey;
  if (fontHighlighted) delete fontHighlighted;
  if (fontNormal) delete fontNormal;
  if (fontMenu) delete fontMenu;
  if (opfile) delete opfile;
}


void Settings::loadSettings() {
  bool restoredSettings = false;
  if (opfile) {
    delete opfile;
  }
  opfile = new Options( FN_SETTINGS );
  playerKeys.clear();
  for(int i=0; i < MAX_PLAYER_CNT; ++i) {
    PlayerEventKeys pk;
    for(int t=1; t < PlayerEventCnt; ++t) {
      int key;
      string keyname = string("PLAYER") + asString(i) + "-" + playerEventNames[(PlayerEvent)t];
      if (!opfile->getInt( keyname , key)) {
	key = defaultSettings[ keyname ];
	restoredSettings = true;
      }
      pk[ (PlayerEvent)t ] = (SDLKey)key;
    }
    playerKeys.push_back(pk);
  }
  if (restoredSettings) {
    saveSettings();
  }
}

void Settings::saveSettings() {
  for(int i=0; i < MAX_PLAYER_CNT; ++i) {
    string name;
    for(int t=1; t < PlayerEventCnt; ++t) {
      opfile->setInt( (int)playerKeys[i][ (PlayerEvent)t ], 
		      string("PLAYER") + asString(i) + "-" + playerEventNames[(PlayerEvent)t] );
    }
  }
  if (opfile) {
    opfile->saveFile( FN_SETTINGS );
  }
}


void Settings::draw( SDL_Surface *screen, bool getNewKey ) {
  videoserver->clearScreen();
  SDL_Rect r;
  r.x = screen->w / 2 - introSprite->w / 2;
  r.y = 0;
  r.w = introSprite->w;
  r.h = introSprite->h;  
  SDL_BlitSurface( introSprite, 0, screen, &r );
  showSpecialKeys( screen );
  showSettings( screen, getNewKey );
  showMenu( screen );
  SDL_Flip( screen );
}


void Settings::showSpecialKeys( SDL_Surface *screen ) {
  fontNormal->drawStr(screen, screen->w/2, screen->h - 2*fontNormal->getCharHeight() - 10,
		      "F1: Configure Keys   F5: Fullscreen", FONT_ALIGN_CENTERED );
  fontNormal->drawStr(screen, screen->w/2, screen->h - fontNormal->getCharHeight() - 5,
		      "F6: Display Enemy Stats   F7: Toggle Music   F10: Pause Game",
		      FONT_ALIGN_CENTERED );
}


void Settings::showSettings( SDL_Surface *screen, bool getNewKey ) {
  int playerOfActiveItem = -1;
  if ( actChoice <= SC_FIRE_SPEC_1 ) playerOfActiveItem = 0;
  else if ( actChoice <= SC_FIRE_SPEC_2 ) playerOfActiveItem = 1;
  int activePlayerEvent = actChoice - (8*playerOfActiveItem) + 1;
  SDL_Rect r;
  SDL_Rect srcDesc;
  srcDesc.x = 0;
  srcDesc.y = 0;
  srcDesc.w = 155;
  srcDesc.h = 14;
  SDL_Rect srcKey;
  srcKey.x = 0;
  srcKey.y = 0;
  srcKey.w = 100;
  srcKey.h = 14;

  int i = 150;
  fontHighlighted->drawStr( screen, 15, i - fontHighlighted->getCharHeight() - 5, "Player 1" );
  fontHighlighted->drawStr( screen, screen->w/2 + 5, i - fontHighlighted->getCharHeight() - 5, 
		       "Player 2" );

  std::map< PlayerEvent, SDLKey >::const_iterator pkiter;
  for ( pkiter = playerKeys[0].begin(); pkiter != playerKeys[0].end(); ++pkiter ) {
    
    if ( !(playerOfActiveItem == 0 && pkiter->first == activePlayerEvent) ) {
      r.x = 15;
      r.y = i - 1;
      SDL_BlitSurface(bluePlain, &srcDesc, screen, &r );
      fontNormal->drawStr( screen,  20, i, playerEventNames[ pkiter->first ] + ":" );
      
      r.x = 175;
      r.y = i-1;
      SDL_BlitSurface(bluePlain, &srcKey, screen, &r );
      fontNormal->drawStr( screen, 270, i, keyName[ pkiter->second ], FONT_ALIGN_RIGHT );
    }
    if ( !(playerOfActiveItem == 1 && pkiter->first == activePlayerEvent) ) {
      r.x = screen->w/2 + 5;
      r.y = i - 1;
      SDL_BlitSurface(bluePlain, &srcDesc, screen, &r );
      
      fontNormal->drawStr( screen, screen->w/2 + 10,  i, 
			   playerEventNames[ pkiter->first ] + ":" );

      r.x = screen->w/2 + 165;
      r.y = i-1;
      SDL_BlitSurface(bluePlain, &srcKey, screen, &r );
      fontNormal->drawStr( screen, screen->w/2 + 260, i, 
			   keyName[ playerKeys[1][pkiter->first] ], FONT_ALIGN_RIGHT );
    }
    
    if ( playerOfActiveItem == 0 && pkiter->first == activePlayerEvent ) {
      r.x = 15;
      r.y = i - 1;
      SDL_BlitSurface(bluePlain, &srcDesc, screen, &r );
      fontNormal->drawStr( screen, 20, i, playerEventNames[ pkiter->first ] + ":" );

      r.x = 175;
      r.y = i-1;
      SDL_BlitSurface(whitePlain, &srcKey, screen, &r );
      if (!getNewKey) {
	fontHighlighted->drawStr( screen, 270, i, keyName[ pkiter->second ], FONT_ALIGN_RIGHT );
      }
    }
    if ( playerOfActiveItem == 1 && pkiter->first == activePlayerEvent ) {
      r.x = screen->w/2 + 5;
      r.y = i - 1;
      SDL_BlitSurface(bluePlain, &srcDesc, screen, &r );
      
      fontNormal->drawStr( screen, screen->w/2 + 10,  i, 
			   playerEventNames[ pkiter->first ] + ":" );
      r.x = screen->w/2 + 165;
      r.y = i-1;
      SDL_BlitSurface(whitePlain, &srcKey, screen, &r );
      if (!getNewKey) {
	fontHighlighted->drawStr( screen, screen->w/2 + 260, i, 
				  keyName[ playerKeys[1][pkiter->first] ], FONT_ALIGN_RIGHT );
      }
    }
    
    i = i + fontNormal->getCharHeight() + 5;
  }
}

void Settings::showMenu( SDL_Surface *screen ) {
  SDL_Rect r;
  r.x = 230 - activeChoiceSprite->w - 8;
  r.w = activeChoiceSprite->w;
  r.h = activeChoiceSprite->h;
  
  if ( actChoice == SC_DEFAULTS ) {
    r.y = 328;
    SDL_BlitSurface(activeChoiceSprite, 0, screen, &r );
    fontMenuHighlighted->drawStr( screen, 230, 330, "Reset Defaults" );
  } else fontMenu->drawStr( screen, 230, 330, "Reset Defaults" );

  if ( actChoice == SC_CANCEL ) {
    r.y = 363;
    SDL_BlitSurface(activeChoiceSprite, 0, screen, &r );
    fontMenuHighlighted->drawStr( screen, 230, 365, "Cancel" );
  } else fontMenu->drawStr( screen, 230, 365, "Cancel" );

  if ( actChoice == SC_FINISH ) {
    r.y = 398;
    SDL_BlitSurface(activeChoiceSprite, 0, screen, &r );
    fontMenuHighlighted->drawStr( screen, 230, 400, "Finish" );
  } else fontMenu->drawStr( screen, 230, 400, "Finish" );
}

void Settings::settingsDialog( SDL_Surface *screen ) {
  bool run = true;
  actChoice = SC_FINISH;
  wasLeftColumn = true;
  draw( screen );
  while(run) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_KEYDOWN: {
  	      switch(event.key.keysym.sym) {
            case SDLK_UP: {
              if ( actChoice == SC_DEFAULTS ) {
                if ( wasLeftColumn ) actChoice = SC_FIRE_SPEC_1;
                else actChoice = SC_FIRE_SPEC_2;
	          } else if ( actChoice == SC_UP2 || actChoice == SC_UP1 ) {
		        actChoice = SC_FINISH;
	          } else {
		        actChoice = (SettingsChoices)(actChoice - 1);
	          }
	          break;
	        }
	        case SDLK_DOWN: {
	          if ( actChoice == SC_FINISH ) {
		        if ( wasLeftColumn ) actChoice = SC_UP1;
		        else actChoice = SC_UP2;
	          } else if ( actChoice == SC_FIRE_SPEC_1 ) {
		        actChoice = SC_DEFAULTS;
	          } else {
		        actChoice = (SettingsChoices)(actChoice + 1);
	          }
	          break;
	        }
	        case SDLK_RIGHT: {
	          if (actChoice <= SC_FIRE_SPEC_1) {
		        actChoice = (SettingsChoices)(actChoice + 8);
		        wasLeftColumn = false;
	          }
	          break;
	        }
	        case SDLK_LEFT: {
	          if ( SC_UP2 <= actChoice && actChoice <= SC_FIRE_SPEC_2 ) {
		        actChoice = (SettingsChoices)(actChoice - 8);
		        wasLeftColumn = true;
	          }
	          break;
	        }
	        case SDLK_TAB: {
	          if ( SC_UP1 <= actChoice && actChoice <= SC_FIRE_SPEC_1 ) {
		        actChoice = (SettingsChoices)(actChoice + 8);
		        wasLeftColumn = false;
	          } else {
		        if ( SC_UP2 <= actChoice && actChoice <= SC_FIRE_SPEC_2 ) {
		          actChoice = (SettingsChoices)(actChoice - 8);
		          wasLeftColumn = true;
		        }
	          }
	          break;
	        }
	        case SDLK_RETURN:
	        case SDLK_SPACE: {
              if ( actChoice == SC_DEFAULTS ) {
		        loadDefaultSettings();
	          } else if ( actChoice == SC_CANCEL ) {
		        run = false;
		        loadSettings();
	          } else if ( actChoice == SC_FINISH ) {
		        run = false;
		        saveSettings();
	          } else {
		        draw( screen, true );
		        getNewKeyFromUser();
	          }
	          break;
	        }
            case SDLK_F5: {
	          videoserver->toggleFullscreen();
	          break;
	        }
            case SDLK_F7: {
              if ( playMusicOn ) {
                playMusicOn = false;
                mixer.stopMusic();
              } else {
                playMusicOn = true;
                mixer.playMusic( MUSIC_INTRO, -1, 1000 );
              }
              break;
            }
	        case SDLK_ESCAPE: {
              run = false;
              loadDefaultSettings();
              break;
            }
            default: break;
          }
        }
      }
      draw( screen );
      SDL_Delay( 50 ); // save cpu-power
    }
  }
}

void Settings::loadDefaultSettings() {
  playerKeys.clear();
  for(int i=0; i < MAX_PLAYER_CNT; ++i) {
    PlayerEventKeys pk;   
    for(int t=1; t < PlayerEventCnt; ++t) {
      int key;
      string keyname = string("PLAYER") + asString(i) + "-" + playerEventNames[(PlayerEvent)t];
      key = defaultSettings[ keyname ];
      pk[ (PlayerEvent)t ] = (SDLKey)key;
    }
    playerKeys.push_back(pk);
  }
}

void Settings::getNewKeyFromUser() {
  SDL_Event event;
  bool keypressed = false;
  while( !keypressed ) {
    while (!SDL_PollEvent(&event)) {}
    switch(event.type) {
    case SDL_KEYDOWN:
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYAXISMOTION: {
      if (input.isPressed(event)) {
        int player = (actChoice >= SC_UP2) ? 1 : 0;
        playerKeys[player][ (PlayerEvent)(actChoice - (player * 8) + 1) ] = 
	      input.translate(event);
	    keypressed = true;
      }
	  break;
    }
    }    
  }
}

const PlayerKeys Settings::getPlayerKeys(unsigned int player) const {
  assert( player < playerKeys.size() );
  PlayerKeys pk;
  for(int i=SDLK_FIRST; i <= SDLK_LAST; ++i) {
    pk[ (SDLKey)i ] = PE_UNKNOWN;
  }
  std::map< PlayerEvent, SDLKey >::const_iterator pkiter;
  for(pkiter = playerKeys[player].begin(); pkiter != playerKeys[player].end(); ++pkiter) {
    pk[ pkiter->second ] = pkiter->first;
  }
  return pk;
}

void Settings::setKeyNames() {
  keyName[ (SDLKey)0 ] = "UNKNOWN";
  keyName[ (SDLKey)8 ] = "BACKSPACE";
  keyName[ (SDLKey)9 ] = "TAB";
  keyName[ (SDLKey)12 ] = "CLEAR";
  keyName[ (SDLKey)13 ] = "RETURN";
  keyName[ (SDLKey)19 ] = "PAUSE";
  keyName[ (SDLKey)27 ] = "ESCAPE";
  keyName[ (SDLKey)32 ] = "SPACE";
  keyName[ (SDLKey)33 ] = "EXCLAIM";
  keyName[ (SDLKey)34 ] = "QUOTEDBL";
  keyName[ (SDLKey)35 ] = "HASH";
  keyName[ (SDLKey)36 ] = "DOLLAR";
  keyName[ (SDLKey)38 ] = "AMPERSAND";
  keyName[ (SDLKey)39 ] = "QUOTE";
  keyName[ (SDLKey)40 ] = "LEFTPAREN";
  keyName[ (SDLKey)41 ] = "RIGHTPAREN";
  keyName[ (SDLKey)42 ] = "ASTERISK";
  keyName[ (SDLKey)43 ] = "PLUS";
  keyName[ (SDLKey)44 ] = "COMMA";
  keyName[ (SDLKey)45 ] = "MINUS";
  keyName[ (SDLKey)46 ] = "PERIOD";
  keyName[ (SDLKey)47 ] = "SLASH";
  keyName[ (SDLKey)48 ] = "0";
  keyName[ (SDLKey)49 ] = "1";
  keyName[ (SDLKey)50 ] = "2";
  keyName[ (SDLKey)51 ] = "3";
  keyName[ (SDLKey)52 ] = "4";
  keyName[ (SDLKey)53 ] = "5";
  keyName[ (SDLKey)54 ] = "6";
  keyName[ (SDLKey)55 ] = "7";
  keyName[ (SDLKey)56 ] = "8";
  keyName[ (SDLKey)57 ] = "9";
  keyName[ (SDLKey)58 ] = "COLON";
  keyName[ (SDLKey)59 ] = "SEMICOLON";
  keyName[ (SDLKey)60 ] = "LESS";
  keyName[ (SDLKey)61 ] = "EQUALS";
  keyName[ (SDLKey)62 ] = "GREATER";
  keyName[ (SDLKey)63 ] = "QUESTION";
  keyName[ (SDLKey)64 ] = "AT";
  keyName[ (SDLKey)91 ] = "LEFTBRACKET";
  keyName[ (SDLKey)92 ] = "BACKSLASH";
  keyName[ (SDLKey)93 ] = "RIGHTBRACKET";
  keyName[ (SDLKey)94 ] = "CARET";
  keyName[ (SDLKey)95 ] = "UNDERSCORE";
  keyName[ (SDLKey)96 ] = "BACKQUOTE";
  keyName[ (SDLKey)97 ] = "a";
  keyName[ (SDLKey)98 ] = "b";
  keyName[ (SDLKey)99 ] = "c";
  keyName[ (SDLKey)100 ] = "d";
  keyName[ (SDLKey)101 ] = "e";
  keyName[ (SDLKey)102 ] = "f";
  keyName[ (SDLKey)103 ] = "g";
  keyName[ (SDLKey)104 ] = "h";
  keyName[ (SDLKey)105 ] = "i";
  keyName[ (SDLKey)106 ] = "j";
  keyName[ (SDLKey)107 ] = "k";
  keyName[ (SDLKey)108 ] = "l";
  keyName[ (SDLKey)109 ] = "m";
  keyName[ (SDLKey)110 ] = "n";
  keyName[ (SDLKey)111 ] = "o";
  keyName[ (SDLKey)112 ] = "p";
  keyName[ (SDLKey)113 ] = "q";
  keyName[ (SDLKey)114 ] = "r";
  keyName[ (SDLKey)115 ] = "s";
  keyName[ (SDLKey)116 ] = "t";
  keyName[ (SDLKey)117 ] = "u";
  keyName[ (SDLKey)118 ] = "v";
  keyName[ (SDLKey)119 ] = "w";
  keyName[ (SDLKey)120 ] = "x";
  keyName[ (SDLKey)121 ] = "y";
  keyName[ (SDLKey)122 ] = "z";
  keyName[ (SDLKey)127 ] = "DELETE";
  keyName[ (SDLKey)160 ] = "WORLD_0";
  keyName[ (SDLKey)161 ] = "WORLD_1";
  keyName[ (SDLKey)162 ] = "WORLD_2";
  keyName[ (SDLKey)163 ] = "WORLD_3";
  keyName[ (SDLKey)164 ] = "WORLD_4";
  keyName[ (SDLKey)165 ] = "WORLD_5";
  keyName[ (SDLKey)166 ] = "WORLD_6";
  keyName[ (SDLKey)167 ] = "WORLD_7";
  keyName[ (SDLKey)168 ] = "WORLD_8";
  keyName[ (SDLKey)169 ] = "WORLD_9";
  keyName[ (SDLKey)170 ] = "WORLD_10";
  keyName[ (SDLKey)171 ] = "WORLD_11";
  keyName[ (SDLKey)172 ] = "WORLD_12";
  keyName[ (SDLKey)173 ] = "WORLD_13";
  keyName[ (SDLKey)174 ] = "WORLD_14";
  keyName[ (SDLKey)175 ] = "WORLD_15";
  keyName[ (SDLKey)176 ] = "WORLD_16";
  keyName[ (SDLKey)177 ] = "WORLD_17";
  keyName[ (SDLKey)178 ] = "WORLD_18";
  keyName[ (SDLKey)179 ] = "WORLD_19";
  keyName[ (SDLKey)180 ] = "WORLD_20";
  keyName[ (SDLKey)181 ] = "WORLD_21";
  keyName[ (SDLKey)182 ] = "WORLD_22";
  keyName[ (SDLKey)183 ] = "WORLD_23";
  keyName[ (SDLKey)184 ] = "WORLD_24";
  keyName[ (SDLKey)185 ] = "WORLD_25";
  keyName[ (SDLKey)186 ] = "WORLD_26";
  keyName[ (SDLKey)187 ] = "WORLD_27";
  keyName[ (SDLKey)188 ] = "WORLD_28";
  keyName[ (SDLKey)189 ] = "WORLD_29";
  keyName[ (SDLKey)190 ] = "WORLD_30";
  keyName[ (SDLKey)191 ] = "WORLD_31";
  keyName[ (SDLKey)192 ] = "WORLD_32";
  keyName[ (SDLKey)193 ] = "WORLD_33";
  keyName[ (SDLKey)194 ] = "WORLD_34";
  keyName[ (SDLKey)195 ] = "WORLD_35";
  keyName[ (SDLKey)196 ] = "WORLD_36";
  keyName[ (SDLKey)197 ] = "WORLD_37";
  keyName[ (SDLKey)198 ] = "WORLD_38";
  keyName[ (SDLKey)199 ] = "WORLD_39";
  keyName[ (SDLKey)200 ] = "WORLD_40";
  keyName[ (SDLKey)201 ] = "WORLD_41";
  keyName[ (SDLKey)202 ] = "WORLD_42";
  keyName[ (SDLKey)203 ] = "WORLD_43";
  keyName[ (SDLKey)204 ] = "WORLD_44";
  keyName[ (SDLKey)205 ] = "WORLD_45";
  keyName[ (SDLKey)206 ] = "WORLD_46";
  keyName[ (SDLKey)207 ] = "WORLD_47";
  keyName[ (SDLKey)208 ] = "WORLD_48";
  keyName[ (SDLKey)209 ] = "WORLD_49";
  keyName[ (SDLKey)210 ] = "WORLD_50";
  keyName[ (SDLKey)211 ] = "WORLD_51";
  keyName[ (SDLKey)212 ] = "WORLD_52";
  keyName[ (SDLKey)213 ] = "WORLD_53";
  keyName[ (SDLKey)214 ] = "WORLD_54";
  keyName[ (SDLKey)215 ] = "WORLD_55";
  keyName[ (SDLKey)216 ] = "WORLD_56";
  keyName[ (SDLKey)217 ] = "WORLD_57";
  keyName[ (SDLKey)218 ] = "WORLD_58";
  keyName[ (SDLKey)219 ] = "WORLD_59";
  keyName[ (SDLKey)220 ] = "WORLD_60";
  keyName[ (SDLKey)221 ] = "WORLD_61";
  keyName[ (SDLKey)222 ] = "WORLD_62";
  keyName[ (SDLKey)223 ] = "WORLD_63";
  keyName[ (SDLKey)224 ] = "WORLD_64";
  keyName[ (SDLKey)225 ] = "WORLD_65";
  keyName[ (SDLKey)226 ] = "WORLD_66";
  keyName[ (SDLKey)227 ] = "WORLD_67";
  keyName[ (SDLKey)228 ] = "WORLD_68";
  keyName[ (SDLKey)229 ] = "WORLD_69";
  keyName[ (SDLKey)230 ] = "WORLD_70";
  keyName[ (SDLKey)231 ] = "WORLD_71";
  keyName[ (SDLKey)232 ] = "WORLD_72";
  keyName[ (SDLKey)233 ] = "WORLD_73";
  keyName[ (SDLKey)234 ] = "WORLD_74";
  keyName[ (SDLKey)235 ] = "WORLD_75";
  keyName[ (SDLKey)236 ] = "WORLD_76";
  keyName[ (SDLKey)237 ] = "WORLD_77";
  keyName[ (SDLKey)238 ] = "WORLD_78";
  keyName[ (SDLKey)239 ] = "WORLD_79";
  keyName[ (SDLKey)240 ] = "WORLD_80";
  keyName[ (SDLKey)241 ] = "WORLD_81";
  keyName[ (SDLKey)242 ] = "WORLD_82";
  keyName[ (SDLKey)243 ] = "WORLD_83";
  keyName[ (SDLKey)244 ] = "WORLD_84";
  keyName[ (SDLKey)245 ] = "WORLD_85";
  keyName[ (SDLKey)246 ] = "WORLD_86";
  keyName[ (SDLKey)247 ] = "WORLD_87";
  keyName[ (SDLKey)248 ] = "WORLD_88";
  keyName[ (SDLKey)249 ] = "WORLD_89";
  keyName[ (SDLKey)250 ] = "WORLD_90";
  keyName[ (SDLKey)251 ] = "WORLD_91";
  keyName[ (SDLKey)252 ] = "WORLD_92";
  keyName[ (SDLKey)253 ] = "WORLD_93";
  keyName[ (SDLKey)254 ] = "WORLD_94";
  keyName[ (SDLKey)255 ] = "WORLD_95";
  keyName[ (SDLKey)256 ] = "KP0";
  keyName[ (SDLKey)257 ] = "KP1";
  keyName[ (SDLKey)258 ] = "KP2";
  keyName[ (SDLKey)259 ] = "KP3";
  keyName[ (SDLKey)260 ] = "KP4";
  keyName[ (SDLKey)261 ] = "KP5";
  keyName[ (SDLKey)262 ] = "KP6";
  keyName[ (SDLKey)263 ] = "KP7";
  keyName[ (SDLKey)264 ] = "KP8";
  keyName[ (SDLKey)265 ] = "KP9";
  keyName[ (SDLKey)266 ] = "KP_PERIOD";
  keyName[ (SDLKey)267 ] = "KP_DIVIDE";
  keyName[ (SDLKey)268 ] = "KP_MULTIPLY";
  keyName[ (SDLKey)269 ] = "KP_MINUS";
  keyName[ (SDLKey)270 ] = "KP_PLUS";
  keyName[ (SDLKey)271 ] = "KP_ENTER";
  keyName[ (SDLKey)272 ] = "KP_EQUALS";
  keyName[ (SDLKey)273 ] = "UP";
  keyName[ (SDLKey)274 ] = "DOWN";
  keyName[ (SDLKey)275 ] = "RIGHT";
  keyName[ (SDLKey)276 ] = "LEFT";
  keyName[ (SDLKey)277 ] = "INSERT";
  keyName[ (SDLKey)278 ] = "HOME";
  keyName[ (SDLKey)279 ] = "END";
  keyName[ (SDLKey)280 ] = "PAGEUP";
  keyName[ (SDLKey)281 ] = "PAGEDOWN";
  keyName[ (SDLKey)282 ] = "F1";
  keyName[ (SDLKey)283 ] = "F2";
  keyName[ (SDLKey)284 ] = "F3";
  keyName[ (SDLKey)285 ] = "F4";
  keyName[ (SDLKey)286 ] = "F5";
  keyName[ (SDLKey)287 ] = "F6";
  keyName[ (SDLKey)288 ] = "F7";
  keyName[ (SDLKey)289 ] = "F8";
  keyName[ (SDLKey)290 ] = "F9";
  keyName[ (SDLKey)291 ] = "F10";
  keyName[ (SDLKey)292 ] = "F11";
  keyName[ (SDLKey)293 ] = "F12";
  keyName[ (SDLKey)294 ] = "F13";
  keyName[ (SDLKey)295 ] = "F14";
  keyName[ (SDLKey)296 ] = "F15";
  keyName[ (SDLKey)300 ] = "NUMLOCK";
  keyName[ (SDLKey)301 ] = "CAPSLOCK";
  keyName[ (SDLKey)302 ] = "SCROLLOCK";
  keyName[ (SDLKey)303 ] = "RSHIFT";
  keyName[ (SDLKey)304 ] = "LSHIFT";
  keyName[ (SDLKey)305 ] = "RCTRL";
  keyName[ (SDLKey)306 ] = "LCTRL";
  keyName[ (SDLKey)307 ] = "RALT";
  keyName[ (SDLKey)308 ] = "LALT";
  keyName[ (SDLKey)309 ] = "RMETA";
  keyName[ (SDLKey)310 ] = "LMETA";
  keyName[ (SDLKey)311 ] = "LSUPER";
  keyName[ (SDLKey)312 ] = "RSUPER";
  keyName[ (SDLKey)313 ] = "MODE";
  keyName[ (SDLKey)314 ] = "COMPOSE";
  keyName[ (SDLKey)315 ] = "HELP";
  keyName[ (SDLKey)316 ] = "PRINT";
  keyName[ (SDLKey)317 ] = "SYSREQ";
  keyName[ (SDLKey)318 ] = "BREAK";
  keyName[ (SDLKey)319 ] = "MENU";
  keyName[ (SDLKey)320 ] = "POWER";
  keyName[ (SDLKey)321 ] = "EURO";
  keyName[ (SDLKey)322 ] = "UNDO";
  // the following entries are chosen by us... just for the mapping
  keyName[ (SDLKey)1024 ] = "GPD1_L";
  keyName[ (SDLKey)1025 ] = "GPD1_R";
  keyName[ (SDLKey)1026 ] = "GPD1_U";
  keyName[ (SDLKey)1027 ] = "GPD1_D";
  keyName[ (SDLKey)1028 ] = "GPD2_L";
  keyName[ (SDLKey)1029 ] = "GPD2_R";
  keyName[ (SDLKey)1030 ] = "GPD2_U";
  keyName[ (SDLKey)1031 ] = "GPD2_D";
  keyName[ (SDLKey)1032 ] = "GPD3_L";
  keyName[ (SDLKey)1033 ] = "GPD3_R";
  keyName[ (SDLKey)1034 ] = "GPD3_U";
  keyName[ (SDLKey)1035 ] = "GPD3_D";
  keyName[ (SDLKey)1036 ] = "GPD4_L";
  keyName[ (SDLKey)1037 ] = "GPD4_R";
  keyName[ (SDLKey)1038 ] = "GPD4_U";
  keyName[ (SDLKey)1039 ] = "GPD4_D";
  keyName[ (SDLKey)1040 ] = "GPB_0";
  keyName[ (SDLKey)1041 ] = "GPB_1";
  keyName[ (SDLKey)1042 ] = "GPB_2";
  keyName[ (SDLKey)1043 ] = "GPB_3";
  keyName[ (SDLKey)1044 ] = "GPB_4";
  keyName[ (SDLKey)1045 ] = "GPB_5";
  keyName[ (SDLKey)1046 ] = "GPB_6";
  keyName[ (SDLKey)1047 ] = "GPB_7";
  keyName[ (SDLKey)1048 ] = "GPB_8";
  keyName[ (SDLKey)1049 ] = "GPB_9";
  keyName[ (SDLKey)1050 ] = "GPB_10";
  keyName[ (SDLKey)1051 ] = "GPB_11";
  keyName[ (SDLKey)1052 ] = "GPB_12";
  keyName[ (SDLKey)1053 ] = "GPB_13";
  keyName[ (SDLKey)1054 ] = "GPB_14";
  keyName[ (SDLKey)1055 ] = "GPB_15";
}

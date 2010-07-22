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
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <string>
#include <map>
#include <vector>
#include "SDL.h"
#include "SdlForwardCompat.h"


class Font;
class Options;

enum PlayerEvent { PE_UNKNOWN = 0, PE_UP, PE_DOWN, PE_LEFT, PE_RIGHT,
		   PE_CHOOSE_WEAPON_SECONDARY, PE_CHOOSE_WEAPON_SPECIALS, 
		   PE_FIRE_WEAPONS, PE_FIRE_SPECIALS };

const int PlayerEventCnt = 9;

enum SettingsChoices { SC_UP1 = 0, SC_DOWN1, SC_LEFT1, SC_RIGHT1, 
		       SC_CHOOSE_WEAP_SEC_1, SC_CHOOSE_WEAP_SPEC_1,
		       SC_FIRE_WEAP_1, SC_FIRE_SPEC_1,
		       SC_UP2, SC_DOWN2, SC_LEFT2, SC_RIGHT2, 
		       SC_CHOOSE_WEAP_SEC_2, SC_CHOOSE_WEAP_SPEC_2,
		       SC_FIRE_WEAP_2, SC_FIRE_SPEC_2,
		       SC_DEFAULTS, SC_CANCEL, SC_FINISH };

const int NR_SETTINGS_CHOICES = 19;


typedef std::map< PlayerEvent, SDLKey > PlayerEventKeys;
typedef std::map< SDLKey, PlayerEvent > PlayerKeys;
typedef std::map< PlayerEvent, std::string > PlayerEventName;
typedef std::map< std::string, SDLKey > DefaultSettings;

class Settings;

extern Settings *settings;

class Settings {
  public:

  Settings();
  ~Settings();
  
  void settingsDialog(SdlCompat_AcceleratedSurface *screen);
  const PlayerKeys getPlayerKeys(unsigned int player) const;
  const PlayerEventKeys & getPlayerEventKeys(unsigned int player) const;

  private:

  SdlCompat_AcceleratedSurface *introSprite;
  SdlCompat_AcceleratedSurface *activeChoiceSprite;
  SdlCompat_AcceleratedSurface *bluePlain;
  SdlCompat_AcceleratedSurface *whitePlain;
  SettingsChoices actChoice;
  bool wasLeftColumn;
  Options *opfile;
  Font *fontMenu;
  Font *fontMenuHighlighted;
  Font *fontNormal;
  Font *fontKey;
  Font *fontHighlighted;
  std::vector< PlayerEventKeys > playerKeys;
  //std::map< SDLKey, std::string > keyName;
  PlayerEventName playerEventNames;
  DefaultSettings defaultSettings;
  void getNewKeyFromUser();
  void loadDefaultSettings();
  void loadSettings();
  void saveSettings();
  void draw( SdlCompat_AcceleratedSurface *screen, bool getNewKey=false );
  void showSpecialKeys( SdlCompat_AcceleratedSurface *screen );
  void showSettings( SdlCompat_AcceleratedSurface *screen, bool getNewKey );
  void showMenu( SdlCompat_AcceleratedSurface *screen );
  void changeCurrentSettings(SdlCompat_AcceleratedSurface *screen, int player);
  void setKeyNames();
};

#endif

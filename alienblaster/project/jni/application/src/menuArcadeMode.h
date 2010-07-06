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
#ifndef MENU_ARCADE_MODE_H
#define MENU_ARCADE_MODE_H

#include "SDL.h"
#include "game.h"
#include <string>

class Font;
class Options;

enum MENU_ARCADE_CHOICES { ARCADE_FIGHT, ARCADE_BACK_TO_MAIN_MENU };

const std::string STRINGS_MENU_ARCADE_CHOICES[] = 
{ "FIGHT",
  "Back to Main Menu" };
const int NR_MENU_ARCADE_CHOICES = 2;

class MenuArcadeMode {
  private:
  SdlCompat_AcceleratedSurface *screen;
  SdlCompat_AcceleratedSurface *arcadeSprite;
  SdlCompat_AcceleratedSurface *activeChoiceSprite;
  SdlCompat_AcceleratedSurface *lightFighterIcon1;
  SdlCompat_AcceleratedSurface *heavyFighterIcon1;
  Font *font;
  Font *fontHighlighted;

  Options *op;
  
  int activeChoice;
  // sounds
  int choose;
  int confirm;

  bool playerOneLightFighter;

  public:
  MenuArcadeMode( SdlCompat_AcceleratedSurface *scr );
  ~MenuArcadeMode();
  void run( GameStates &gameState, int points=-1 );
  bool getPlayerOneLightFighter();

  private:
  void updateHighScore( int points );
  void readHighScoreName( int pos );
  void drawReadName( string &newName );
  bool handleEventsReadName( string &newName );

  void handleEvents( GameStates &gameState );
  void draw();
};

#endif

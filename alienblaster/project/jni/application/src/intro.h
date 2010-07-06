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
#ifndef INTRO_H
#define INTRO_H

#include <string>
#include "SDL.h"
#include "SdlForwardCompat.h"
#include "game.h"

class Font;
class Infoscreen;

enum IntroChoices { ONE_PLAYER_GAME, TWO_PLAYER_GAME, ARCADE_GAME,
		    CONFIGURE_KEYS, INFOSCREEN, QUIT_AND_DIE };
const std::string INTRO_CHOICES[] = { "1-Player Action", "2-Player Fight",
				      "Arcade",
				      "Configure Keys", "Infoscreen", "Quit and Die" };
const int NR_INTRO_CHOICES = 6;

class Intro {
  private:
  SdlCompat_AcceleratedSurface *screen;
  SdlCompat_AcceleratedSurface *introSprite;
  SdlCompat_AcceleratedSurface *activeChoiceSprite;
  Font *font;
  Font *fontHighlighted;
  Infoscreen *infoscreen;
  
  int activeChoice;

  // sound
  int choose;
  int confirm;

  public:
  Intro( SdlCompat_AcceleratedSurface *scr );
  ~Intro();
  void run( GameStates &gameState );
  void showScreenshots();
  // bool blendImages( SdlCompat_AcceleratedSurface *screen, SdlCompat_AcceleratedSurface *surf0, SDL_Rect *r1, SdlCompat_AcceleratedSurface *surf1, SDL_Rect *r2, int sps );

  private:
  void handleEvents( GameStates &gameState );
  void draw();
};

#endif

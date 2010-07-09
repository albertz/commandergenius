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

#include "menuArcadeMode.h"
#include "global.h"
#include "surfaceDB.h"
#include "mixer.h"
#include "video.h"
#include "font.h"
#include "racer.h"
#include "racers.h"
#include "asstring.h"
#include "options.h"
#include <iostream>
#include <fstream>

MenuArcadeMode::MenuArcadeMode( SdlCompat_AcceleratedSurface *scr ) {
  screen = scr;
  arcadeSprite = surfaceDB.loadSurface( FN_ARCADE_LOGO );
  activeChoiceSprite = surfaceDB.loadSurface( FN_INTRO_SHOW_CHOICE );

  font = new Font( FN_FONT_INTRO );
  fontHighlighted = new Font( FN_FONT_INTRO_HIGHLIGHTED );
  lightFighterIcon1 = surfaceDB.loadSurface( FN_LIGHT_FIGHTER_1_ICON );
  heavyFighterIcon1 = surfaceDB.loadSurface( FN_HEAVY_FIGHTER_1_ICON );
  choose = Mixer::mixer().loadSample( FN_SOUND_ARCADE_CHOOSE, 100 );
  confirm = Mixer::mixer().loadSample( FN_SOUND_ARCADE_CONFIRM, 60 );
  activeChoice = 0;
  playerOneLightFighter = true;

  op = new Options( FN_HIGHSCORE );

  // check, if the highscore is open
  int highscoreTest;
  if ( !op->getInt( "POINTS_1", highscoreTest ) ) {
    cout << "Creating the highscore..." << endl;
    op->setStr( "ALIENBLASTER", "NAME_1" );
    op->setInt( 1666, "POINTS_1" );
    op->setStr( "GNOSTICAL", "NAME_2" );
    op->setInt( 1000, "POINTS_2" );
    op->setStr( "SLOBJOK", "NAME_3" );
    op->setInt( 1000, "POINTS_3" );
    op->setStr( "SNOWBALL", "NAME_4" );
    op->setInt( 1000, "POINTS_4" );
    op->setStr( "THUNDERBIRD", "NAME_5" );
    op->setInt( 1000, "POINTS_5" );
    op->setStr( "KaoT", "NAME_6" );
    op->setInt( 666, "POINTS_6" );
    op->setStr( "SDL", "NAME_7" );
    op->setInt( 500, "POINTS_7" );
    op->setStr( "DEBIAN", "NAME_8" );
    op->setInt( 400, "POINTS_8" );
    op->setStr( "DEV-CPP", "NAME_9" );
    op->setInt( 300, "POINTS_9" );
    op->setStr( "I RULE", "NAME_10" );
    op->setInt( 200, "POINTS_10" );
    op->saveFile( FN_HIGHSCORE );
  }  
}

MenuArcadeMode::~MenuArcadeMode() {
  op->saveFile( FN_HIGHSCORE );
  delete op;
}

void MenuArcadeMode::run( GameStates &gameState, int points ) {
  activeChoice = 0;
  
  if ( GS_ARCADE_MODE_FINISHED ) {
    updateHighScore( points );
    gameState = GS_ARCADE_MODE_SETUP;
  }

  draw();
  while ( gameState == GS_ARCADE_MODE_SETUP ) {
    handleEvents( gameState );
    draw();
    SDL_Delay( 50 );
  }
}


bool MenuArcadeMode::getPlayerOneLightFighter() {
  return playerOneLightFighter;
}
 
void MenuArcadeMode::draw() {
  videoserver->clearScreen();
  SDL_Rect r;
  r.x = screen->w / 2 - arcadeSprite->w / 2;
  r.y = 0;
  r.w = arcadeSprite->w;
  r.h = arcadeSprite->h;  
  SDL_BlitSurface( arcadeSprite, 0, screen, &r );

  string name = "UNKNOWN";
  int points = 0;
  op->getStr( "NAME_1", name );
  op->getInt( "POINTS_1", points );
  fontHighlighted->drawStr( screen, 230, 60, "1. " + name + ": " + asString(points) );
  for ( int i = 2; i <= 10; i++ ) {
    op->getStr( "NAME_" + asString( i ), name );
    op->getInt( "POINTS_" + asString( i ), points );
    font->drawStr( screen, 230, 40 + i * 25, 
		   asString( i ) + ". " + name + ": " + asString(points) );
  }

  for ( int i = 0; i < NR_MENU_ARCADE_CHOICES; i++ ) {
    if ( activeChoice == i ) {
      r.x = 230 - activeChoiceSprite->w - 8;
      r.y = 338 + i * 40;
      r.w = activeChoiceSprite->w;
      r.h = activeChoiceSprite->h;
      SDL_BlitSurface( activeChoiceSprite, 0, screen, &r );
      fontHighlighted->drawStr( screen, 230, 340 + i * 40, STRINGS_MENU_ARCADE_CHOICES[ i ] );
    } else {
      font->drawStr( screen, 230, 340 + i * 40, STRINGS_MENU_ARCADE_CHOICES[ i ] );
    }
  }
  
  font->drawStr( screen, 50, 270, "Player 1" );
  if ( playerOneLightFighter ) {
    r.x = 100 - lightFighterIcon1->w / 2;
    r.y = 340 - lightFighterIcon1->h / 2;
    r.w = lightFighterIcon1->w;
    r.h = lightFighterIcon1->h;
    SDL_BlitSurface( lightFighterIcon1, 0, screen, &r );
  } else {
    r.x = 100 - heavyFighterIcon1->w / 2;
    r.y = 340 - heavyFighterIcon1->h / 2;
    r.w = heavyFighterIcon1->w;
    r.h = heavyFighterIcon1->h;
    SDL_BlitSurface( heavyFighterIcon1, 0, screen, &r );
  }
  fontHighlighted->drawStr( screen, 100, 400, "Press \"Volume Up\"", FONT_ALIGN_CENTERED );
  fontHighlighted->drawStr( screen, 100, 430, "To Change", FONT_ALIGN_CENTERED );

  SDL_Flip( screen );
}


void MenuArcadeMode::handleEvents( GameStates &gameState ) {
  SDL_Event event;
  
  while ( SDL_PollEvent(&event) ) {
    switch(event.type) {
    case SDL_KEYDOWN: {
      switch ( event.key.keysym.sym ) {
      case SDLK_PAGEUP: {
	playerOneLightFighter = !playerOneLightFighter;
	break;
      }
      case SDLK_F5: {
        videoserver->toggleFullscreen();
        break;
      }
      case SDLK_F7: {
	if ( playMusicOn ) {
	  playMusicOn = false;
	  Mixer::mixer().stopMusic();
	} else {
	  playMusicOn = true;
	  Mixer::mixer().playMusic( MUSIC_INTRO, -1, 1000 );
	}
	break;
      }
      case SDLK_UP: {
	Mixer::mixer().playSample( choose, 0 );
	activeChoice--;
	if ( activeChoice < 0 ) activeChoice = NR_MENU_ARCADE_CHOICES - 1;
	break;
      }
      case SDLK_DOWN: {
	Mixer::mixer().playSample( choose, 0 );
	activeChoice = (activeChoice + 1) % NR_MENU_ARCADE_CHOICES;
	break;
      }
      case SDLK_ESCAPE: {
    	  gameState = GS_INTRO;
	      break;      
      }
      case SDLK_RETURN: {
	switch (activeChoice) {
	case ARCADE_FIGHT: {
	  Mixer::mixer().playSample( confirm, 0 );
	  difficultyLevel = ARCADE_DIFFICULTY_LEVEL;
	  gameState = GS_PLAYON;
	  break;
	}
	case ARCADE_BACK_TO_MAIN_MENU: {
	  gameState = GS_INTRO;
	  break;
	}
	}
	break;
      }
      default: break;
      }
      break;
    }
    case SDL_QUIT: {
      gameState = GS_QUIT;
      break;
    }
    default: break;
    }
  }
}

/**************** read the new name for the highscore ***********************/


void MenuArcadeMode::updateHighScore( int points ) {
  int pointTenth = -1;
  op->getInt( "POINTS_10", pointTenth );
  // the player made it in the hall of fame!
  if ( points >= pointTenth ) {
    
    int pointsOfEntry = -1;
    int newPos = -1;
    for ( int i = 1; i <= 10; i++ ) {
      op->getInt( "POINTS_" + asString( i ), pointsOfEntry );
      if ( pointsOfEntry <= points ) {
	newPos = i;
	break;
      }
    }
    if ( newPos != -1 ) {
      // move the positions after newPos one position further down
      int pointsPrev;
      string namePrev;
      for ( int i = 10; i > newPos; i-- ) {
	op->getInt( "POINTS_" + asString( i-1 ), pointsPrev );
	op->getStr( "NAME_" + asString( i-1 ), namePrev );
	op->setInt( pointsPrev, "POINTS_" + asString ( i ) );
	op->setStr( namePrev, "NAME_" + asString( i ) );
      }
      op->setInt( points, "POINTS_" + asString( newPos ) );
      op->setStr(  "? ? ? ?", "NAME_" + asString( newPos ) );
      readHighScoreName( newPos );
    }
    op->saveFile( FN_HIGHSCORE );
  }
}


void MenuArcadeMode::readHighScoreName( int pos ) {
  videoserver->clearScreen();
  SDL_Rect r;
  r.x = screen->w / 2 - arcadeSprite->w / 2;
  r.y = 0;
  r.w = arcadeSprite->w;
  r.h = arcadeSprite->h;  
  SDL_BlitSurface( arcadeSprite, 0, screen, &r );
  
  string name = "UNKNOWN";
  int points = 0;
  for ( int i = 1; i <= 10; i++ ) {
    op->getStr( "NAME_" + asString( i ), name );
    op->getInt( "POINTS_" + asString( i ), points );
    if ( pos == i ) {
      fontHighlighted->drawStr( screen, 230, 40 + i * 25, 
				asString( i ) + ". " + name + ": " + asString(points) );
    } else {
      font->drawStr( screen, 230, 40 + i * 25, 
		     asString( i ) + ". " + name + ": " + asString(points) );
    }
  }

  fontHighlighted->drawStr( screen, 320, 340, "CONGRATULATION - Enter your worthy name!",
			    FONT_ALIGN_CENTERED );

  string newName = "";
  while ( handleEventsReadName( newName ) ) {
    drawReadName( newName );
    SDL_Flip( screen );
    SDL_Delay( 50 );
  }
  op->setStr( newName, "NAME_" + asString( pos ) );
}

bool MenuArcadeMode::handleEventsReadName( string &newName ) {
  bool moreToRead = true;
  SDL_Event event;
  
  while ( SDL_PollEvent(&event) ) {
    if ( event.type == SDL_KEYDOWN ) {
      switch ( event.key.keysym.sym ) {
      case SDLK_F5: {
        videoserver->toggleFullscreen();
        break;
      }
      case SDLK_F7: 
	{
	  if ( playMusicOn ) {
	    playMusicOn = false;
	    Mixer::mixer().stopMusic();
	  } else {
	    playMusicOn = true;
	    Mixer::mixer().playMusic( MUSIC_INTRO, -1, 1000 );
	  }
	  break;
	}
      case SDLK_RETURN: 
	{
	  moreToRead = false;
	  break;
	}
      case SDLK_BACKSPACE:
	{
	  if ( newName.length() > 0 ) {
	    newName.resize( newName.length() - 1 );
	  }
	  break;
	}
      default: 
	{
	  if ( newName.length() <= 15 ) {
	    if ( SDLK_a <= event.key.keysym.sym &&
		 event.key.keysym.sym <= SDLK_z ) {
	      if ( ((SDL_GetModState()) & KMOD_LSHIFT != 0 ) ||
		   ((SDL_GetModState()) & KMOD_RSHIFT != 0 ) ) {
		newName += asString( (char)('A' + event.key.keysym.sym - SDLK_a ) );
	      } else {
		newName += asString( (char)('a' + event.key.keysym.sym - SDLK_a ) );
	      }
	    } else if ( event.key.keysym.sym == SDLK_SPACE ) {
	      newName += " ";
	    }
	  }
	  break;
	}
      }
    }
  }
  return moreToRead;
}

void MenuArcadeMode::drawReadName( string &name ) {
  SDL_Rect r;
  r.x = 150;
  r.y = 380;
  r.w = 400;
  r.h = 40;
  SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0, 0, 0) );

  fontHighlighted->drawStr( screen, 320, 380, name, FONT_ALIGN_CENTERED );
}

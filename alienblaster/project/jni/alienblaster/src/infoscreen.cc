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

#include "infoscreen.h"
#include "global.h"
#include "surfaceDB.h"
#include "mixer.h"
#include "video.h"
#include "font.h"
#include "items.h"
#include "item.h"

Items *infoscreenItems;

Infoscreen::Infoscreen( SDL_Surface *scr ) {
  screen = scr;
  font = new Font( FN_FONT_INTRO );
  fontHighlighted = new Font( FN_FONT_INTRO_HIGHLIGHTED );
  activeChoiceSprite = surfaceDB.loadSurface( FN_INTRO_SHOW_CHOICE );
  lightFighterIcon1 = surfaceDB.loadSurface( FN_LIGHT_FIGHTER_1_ICON );
  lightFighterIcon2 = surfaceDB.loadSurface( FN_LIGHT_FIGHTER_2_ICON );
  heavyFighterIcon1 = surfaceDB.loadSurface( FN_HEAVY_FIGHTER_1_ICON );
  heavyFighterIcon2 = surfaceDB.loadSurface( FN_HEAVY_FIGHTER_2_ICON );
  choose = mixer.loadSample( FN_SOUND_INTRO_CHOOSE, 100 );
  confirm = mixer.loadSample( FN_SOUND_INTRO_CONFIRM, 100 );
  if (infoscreenItems) delete infoscreenItems;
  infoscreenItems = new Items();
  activeChoice = 0;
}

Infoscreen::~Infoscreen() {
  if (infoscreenItems) delete infoscreenItems;
}

void Infoscreen::run() {
  draw();
  quitInfoscreen = false;
  activeChoice = 0;
  while ( !quitInfoscreen ) {
    handleEvents();
    draw();
    SDL_Delay( 50 );
  }
}

void Infoscreen::putBitmapAtPosition( int x, int y, SDL_Surface* bitmap ) {
  SDL_Rect d;
  d.x = x - bitmap->w / 2;
  d.y = y - bitmap->h / 2;
  d.w = bitmap->w;
  d.h = bitmap->h;
  SDL_BlitSurface( bitmap, 0, screen, &d );
}

void Infoscreen::draw() {
  // clear the screen
  videoserver->clearScreen();
  // draw vertical green line
  SDL_Rect r;
  r.x = 250;
  r.y = 0;
  r.w = 1;
  r.h = screen->h;
  SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0, 255, 0) );

  for ( int i = 0; i < NR_INFOSCREEN_CHOICES; i++ ) {
    int ypos = 30 + i * 25;
    if ( i == INFO_CREDITS ) ypos += 15;
    if ( i == INFO_BACK_TO_MAIN_MENU ) ypos += 30;
    if( activeChoice == i ) {
      putBitmapAtPosition( 15, ypos + 8, activeChoiceSprite );
      fontHighlighted->drawStr( screen, 30, ypos, SET_INFOSCREEN_CHOICES[ i ] );
    } else {
      font->drawStr( screen, 30, ypos, SET_INFOSCREEN_CHOICES[ i ] );
    }
  }
  switch ( activeChoice ) {
  case INFO_LIGHT_FIGHTER: 
    {
      putBitmapAtPosition( 386, 50, lightFighterIcon1 );
      putBitmapAtPosition( 502, 50, lightFighterIcon2 );
      font->drawStr( screen, 270, 100, "The Light Fighter is more");
      font->drawStr( screen, 270, 130, "vulnerable but smaller and");
      font->drawStr( screen, 270, 160, "faster than the Heavy Fighter.");
      font->drawStr( screen, 270, 200, "Possible Updates:");
      newItem = new Item( Vector2D ( 270, 240 ) , Vector2D(0,0), ITEM_PRIMARY_UPGRADE );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 230, "Primary Weapon Upgrade");
      newItem = new Item( Vector2D ( 270, 270 ) , Vector2D(0,0), ITEM_MACHINE_GUN );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 260, "Machinegun");
      newItem = new Item( Vector2D ( 270, 300 ) , Vector2D(0,0), ITEM_DUMBFIRE_DOUBLE );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 290, "Double Dumbfire");
      newItem = new Item( Vector2D ( 270, 330 ) , Vector2D(0,0), ITEM_KICK_ASS_ROCKET );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 320, "Kick-Ass Rocket");
      newItem = new Item( Vector2D ( 270, 360 ) , Vector2D(0,0), ITEM_HELLFIRE );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 350, "Hellfire Rocket");
      newItem = new Item( Vector2D ( 270, 390 ) , Vector2D(0,0), ITEM_HEATSEEKER );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 380, "Heatseeker");
      newItem = new Item( Vector2D ( 270, 420 ) , Vector2D(0,0), ITEM_NUKE );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 410, "The Great Nuke");
      newItem = new Item( Vector2D ( 270, 450 ) , Vector2D(0,0), ITEM_ENERGY_BEAM );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 440, "Energy Beam");
      infoscreenItems->draw( screen );
      break;
    }
  case INFO_HEAVY_FIGHTER: 
    {
      putBitmapAtPosition( 386, 50, heavyFighterIcon1 );
      putBitmapAtPosition( 502, 50, heavyFighterIcon2 );
      font->drawStr( screen, 270, 100, "The Heavy Fighter has superior");
      font->drawStr( screen, 270, 130, "firepower and an energyweapon");
      font->drawStr( screen, 270, 160, "deflector. He lacks agility.");
      font->drawStr( screen, 270, 200, "Possible Updates:");
      newItem = new Item( Vector2D ( 270, 240 ) , Vector2D(0,0), ITEM_PRIMARY_UPGRADE );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 230, "Primary Weapon Upgrade");
      newItem = new Item( Vector2D ( 270, 270 ) , Vector2D(0,0), ITEM_KICK_ASS_ROCKET );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 260, "Kick-Ass Rocket");
      newItem = new Item( Vector2D ( 270, 300 ) , Vector2D(0,0), ITEM_HEATSEEKER );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 290, "Heatseeker");
      newItem = new Item( Vector2D ( 270, 330 ) , Vector2D(0,0), ITEM_NUKE );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 320, "The Great Nuke");
      newItem = new Item( Vector2D ( 270, 360 ) , Vector2D(0,0), ITEM_DEFLECTOR );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 350, "Energyweapon Deflector");
      newItem = new Item( Vector2D ( 270, 390 ) , Vector2D(0,0), ITEM_LASER );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 380, "Heavy Laser");
      infoscreenItems->draw( screen );
      break;
    }
  case INFO_PRIMARY_WEAPON: 
    {
      newItem = new Item( Vector2D ( 270, 50 ) , Vector2D(0,0), ITEM_PRIMARY_UPGRADE );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 40, "Primary Weapon Upgrade");
      font->drawStr( screen, 270, 100, "The Primary Weapon Upgrade can");
      font->drawStr( screen, 270, 130, "be used multiple times and each");
      font->drawStr( screen, 270, 160, "time the primary weapon will");
      font->drawStr( screen, 270, 190, "gain one extra shot or improve");
      font->drawStr( screen, 270, 220, "its damage. The maximum for the");
      font->drawStr( screen, 270, 250, "light fighter are three heavy");
      font->drawStr( screen, 270, 280, "shots and for the heavy figher");
      font->drawStr( screen, 270, 310, "five shots.");
      infoscreenItems->draw( screen );
      break;
    }    
  case INFO_MACHINE_GUN: 
    {
      newItem = new Item( Vector2D ( 270, 50 ) , Vector2D(0,0), ITEM_MACHINE_GUN );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 40, "Machine Gun");
      font->drawStr( screen, 270, 100, "The Machine Gun fires a");
      font->drawStr( screen, 270, 130, "massive amount of steel per");
      font->drawStr( screen, 270, 160, "minute at the enemy on the");
      font->drawStr( screen, 270, 190, "ground and in the air.");
      font->drawStr( screen, 270, 220, "It has the ability to aim in a");
      font->drawStr( screen, 270, 250, "small angle at nearby enemys.");
      font->drawStr( screen, 270, 280, "This makes it useful to");
      font->drawStr( screen, 270, 310, "attack formations from the");
      font->drawStr( screen, 270, 340, "side.");
      infoscreenItems->draw( screen );
      break;
    }    
  case INFO_DUMBFIRE_DOUBLE: 
    {
      newItem = new Item( Vector2D ( 270, 50 ) , Vector2D(0,0), ITEM_DUMBFIRE_DOUBLE );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 40, "Double Dumbfire Rocket");
      font->drawStr( screen, 270, 100, "With this upgrade, you get one");
      font->drawStr( screen, 270, 130, "extra dumbfire missile for the");
      font->drawStr( screen, 270, 160, "light fighter. These rockets");
      font->drawStr( screen, 270, 190, "hit targets on the ground and");
      font->drawStr( screen, 270, 220, "in the air.");
      infoscreenItems->draw( screen );
      break;
    }    
  case INFO_KICK_ASS_ROCKET: 
    {
      newItem = new Item( Vector2D ( 270, 50 ) , Vector2D(0,0), ITEM_KICK_ASS_ROCKET );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 40, "Kick-Ass Rocket");
      font->drawStr( screen, 270, 100, "This is the strongest air to");
      font->drawStr( screen, 270, 130, "ground rocket. It takes only");
      font->drawStr( screen, 270, 160, "one shot to let a turret");
      font->drawStr( screen, 270, 190, "explode but reloading takes");
      font->drawStr( screen, 270, 220, "longer.");
      infoscreenItems->draw( screen );
      break;
    }
  case INFO_HELLFIRE: 
    {
      newItem = new Item( Vector2D ( 270, 50 ) , Vector2D(0,0), ITEM_HELLFIRE );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 40, "Hellfire Rocket");
      font->drawStr( screen, 270, 100, "Medium air to ground missiles");
      font->drawStr( screen, 270, 130, "that are fired in pairs and");
      font->drawStr( screen, 270, 160, "look much better than the");
      font->drawStr( screen, 270, 190, "fat clumsy Kick-Ass-Rocket.");
      infoscreenItems->draw( screen );
      break;
    }
    // heatseaker (für Paul :-)
  case INFO_HEATSEEKER: 
    {
      newItem = new Item( Vector2D ( 270, 50 ) , Vector2D(0,0), ITEM_HEATSEEKER );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 40, "Heatseaker Rocket");
      font->drawStr( screen, 270, 100, "Just hit the trigger and this");
      font->drawStr( screen, 270, 130, "missile will find the enemy");
      font->drawStr( screen, 270, 160, "itself. Ground targets as well");
      font->drawStr( screen, 270, 190, "as air targets will be hit.");
      font->drawStr( screen, 270, 220, "Each item gives 70 of these.");
      infoscreenItems->draw( screen );
      break;
    }    
  case INFO_NUKE: 
    {
      newItem = new Item( Vector2D ( 270, 50 ) , Vector2D(0,0), ITEM_NUKE );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 40, "The Great Nuke");
      font->drawStr( screen, 270, 100, "Not much is known about this");
      font->drawStr( screen, 270, 130, "weapon because everyone who");
      font->drawStr( screen, 270, 160, "saw it exploding was dead in");
      font->drawStr( screen, 270, 190, "just that moment.");
      infoscreenItems->draw( screen );
      break;
    }    
  case INFO_DEFLECTOR: 
    {
      newItem = new Item( Vector2D ( 270, 50 ) , Vector2D(0,0), ITEM_DEFLECTOR );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 40, "Energyweapon Deflector");
      font->drawStr( screen, 270, 100, "This is a very useful");
      font->drawStr( screen, 270, 130, "equipment for the heavy");
      font->drawStr( screen, 270, 160, "fighter. It amplifies the");
      font->drawStr( screen, 270, 190, "builtin deflector, so that");
      font->drawStr( screen, 270, 220, "it is very hard to be hit");
      font->drawStr( screen, 270, 250, "by an energy weapon.");
      infoscreenItems->draw( screen );
      break;
    }    
  case INFO_ENERGY_BEAM: 
    {
      newItem = new Item( Vector2D ( 270, 50 ) , Vector2D(0,0), ITEM_ENERGY_BEAM );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 40, "Energy Beam");
      font->drawStr( screen, 270, 100, "The Energy Beam was built");
      font->drawStr( screen, 270, 130, "for the light fighter and");
      font->drawStr( screen, 270, 160, "its devastating power");
      font->drawStr( screen, 270, 190, "makes it the best choice");
      font->drawStr( screen, 270, 220, "against air targets. In");
      font->drawStr( screen, 270, 250, "addition its really hard");
      font->drawStr( screen, 270, 280, "to evade this weapon.");
      infoscreenItems->draw( screen );
      break;
    }    
  case INFO_LASER: 
    {
      newItem = new Item( Vector2D ( 270, 50 ) , Vector2D(0,0), ITEM_LASER );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 40, "Heavy Laser");
      font->drawStr( screen, 270, 100, "This is the newest type");
      font->drawStr( screen, 270, 130, "of equipment for the heavy");
      font->drawStr( screen, 270, 160, "fighter. A skilled pilot");
      font->drawStr( screen, 270, 190, "can take down two air");
      font->drawStr( screen, 270, 220, "enemies at once with this");
      font->drawStr( screen, 270, 250, "heavy laser.");
      infoscreenItems->draw( screen );
      break;
    }    
  case INFO_SHIELD_UP: 
    {
      newItem = new Item( Vector2D ( 270, 50 ) , Vector2D(0,0), ITEM_HEALTH );
      infoscreenItems->addItem( newItem );
      font->drawStr( screen, 300, 40, "Health Powerup");
      font->drawStr( screen, 270, 100, "If your shield goes down");
      font->drawStr( screen, 270, 130, "this powerup is especially");
      font->drawStr( screen, 270, 160, "useful for instant repairs.");
      font->drawStr( screen, 270, 190, "Try to get it as often as");
      font->drawStr( screen, 270, 220, "you can.");
      infoscreenItems->draw( screen );
      break;
    }    
  case INFO_CREDITS:
    {
      font->drawStr( screen, 445, 40, "Alien Blaster", FONT_ALIGN_CENTERED );
      font->drawStr( screen, 270, 100, "Programming:");
      font->drawStr( screen, 280, 130, "Arne Hormann, Daniel Kühn,");
      font->drawStr( screen, 280, 160, "Paul Grathwohl, Sönke Schwardt");
      font->drawStr( screen, 270, 210, "Art Work: Arne Hormann", FONT_MONOSPACE);
      font->drawStr( screen, 270, 240, "Music:    Paul Grathwohl", FONT_MONOSPACE);
      font->drawStr( screen, 620, 440, "Contact: AlienBlaster@gmx.de", FONT_ALIGN_RIGHT);
      break;
    }
  case INFO_BACK_TO_MAIN_MENU: 
    {
      font->drawStr( screen, 445, 220, "GO AND FIGHT !", FONT_ALIGN_CENTERED );
      break;
    }
  }
  SDL_Flip( screen );
}

void Infoscreen::handleEvents() {
  SDL_Event event;
  
  while ( SDL_PollEvent(&event) ) {
    switch(event.type) {
      case SDL_KEYDOWN: {
        switch ( event.key.keysym.sym ) {
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
          case SDLK_UP: {
            mixer.playSample( choose, 0 );
            infoscreenItems->deleteAllItems();
            activeChoice--;
            if ( activeChoice < 0 ) activeChoice = NR_INFOSCREEN_CHOICES - 1;
            break;
          }
          case SDLK_DOWN: {
            mixer.playSample( choose, 0 );
            infoscreenItems->deleteAllItems();
            activeChoice = (activeChoice + 1) % NR_INFOSCREEN_CHOICES;
            break;
          }
          case SDLK_ESCAPE: {
            quitInfoscreen = true;
	          break;      
          }
          case SDLK_RETURN: {
            mixer.playSample( confirm, 0 );
            switch (activeChoice) {
              case INFO_BACK_TO_MAIN_MENU: {
                quitInfoscreen = true;
                break;
              }
   	        }
            break;
  	      }
          default: break;
  	    }
        break;
      }
      default: break;
    }
  }
}


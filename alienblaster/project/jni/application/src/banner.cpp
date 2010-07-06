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
#include "banner.h"
#include "surfaceDB.h"
#include <iostream>

Banner::Banner( BannerTexts text, BannerModes mode, BannerBoni bonus ) {
  sprite = surfaceDB.loadSurface( FN_BANNER_TEXTS[ text ], true );
  this->mode = mode;
  if ( this->mode == BANNER_MODE_RANDOM ) {
    this->mode = (BannerModes)(rand() % NR_BANNER_MODES);
  }
  this->bonus = bonus;
  if ( this->bonus != BANNER_BONUS_NONE ) {
    spriteBonus = surfaceDB.loadSurface( FN_BANNER_BONUS[ bonus ], true );
  }
  
  pos = Vector2D( -1000, -1000 );

  if ( mode == BANNER_MODE_ITEM_COLLECTED_SINGLE_PLAYER ) {
    pos = Vector2D( 10, SCREEN_HEIGHT - 20 - sprite->h );
  } else if ( mode == BANNER_MODE_ITEM_COLLECTED_PLAYER_ONE ) {
    pos = Vector2D( 10, SCREEN_HEIGHT - 20 - sprite->h );
  } else if ( mode == BANNER_MODE_ITEM_COLLECTED_PLAYER_TWO ) {
    pos = Vector2D( SCREEN_WIDTH - sprite->w - 10,
		    SCREEN_HEIGHT - 20 - sprite->h );
  }
  timeLived = 0;
}


Banner::~Banner() {}

bool Banner::isExpired() {
  return timeLived > BANNER_MODE_LIFETIME[ mode ];
}


void Banner::update( int dT ) {
  timeLived += dT;
  
  switch ( mode ) {
  case BANNER_MODE_FLY_FROM_LEFT:
    {
      if ( timeLived < 1000 ) {
	pos = Vector2D( -(sprite->w) + ((SCREEN_WIDTH + sprite->w)/ 2) * (timeLived / 1000.0), 
			200 - sprite->h / 2 );
      } else if ( 1000 < timeLived && timeLived < 3000 ) {
	pos = Vector2D( ( SCREEN_WIDTH - sprite->w ) / 2, 
			200 - sprite->h / 2 );
      } else {
	pos = Vector2D( ((SCREEN_WIDTH - sprite->w)/2) + 
			((SCREEN_WIDTH + sprite->w)/2)*((timeLived-3000)/2000.0), 
			200 - sprite->h / 2 );
      }
      break;
    }
  case BANNER_MODE_FROM_TOP:
    {
      if ( timeLived < 1000 ) {
	pos = Vector2D( (SCREEN_WIDTH - sprite->w)/2,
			-(sprite->h) + (200 + sprite->h/2) * (timeLived / 1000.0) );
      } else if ( 1000 < timeLived && timeLived < 3000 ) {
	pos = Vector2D( (SCREEN_WIDTH - sprite->w)/2,
			200 - (sprite->h / 2 ) );
      } else {
	pos = Vector2D( (SCREEN_WIDTH - sprite->w)/2,
			200 - (sprite->h / 2) + 
			(200 + (sprite->h / 2))*((timeLived-3000)/2000.0) );
      }
      break;
    }
  case BANNER_MODE_ITEM_COLLECTED_SINGLE_PLAYER:
  case BANNER_MODE_ITEM_COLLECTED_PLAYER_ONE:
  case BANNER_MODE_ITEM_COLLECTED_PLAYER_TWO:
    {
      break;
    }
  default:
    {
      break;
    }
  }
}
    
bool Banner::movingAway() {
  return ( 3000 <= timeLived );
}

void Banner::draw(SdlCompat_AcceleratedSurface *screen) {
  SDL_Rect r;
  r.x = lroundf(pos.getX());
  r.y = lroundf(pos.getY());
  r.w = sprite->w;
  r.h = sprite->h;
  SDL_BlitSurface( sprite, 0, screen, &r );
  if ( bonus != BANNER_BONUS_NONE &&
       1000 < timeLived && timeLived < 3000 ) {
    r.x = SCREEN_WIDTH / 2 - spriteBonus->w / 2;
    r.y = 250;
    r.w = spriteBonus->w;
    r.h = spriteBonus->h;
    SDL_BlitSurface( spriteBonus, 0, screen, &r );
  }
}

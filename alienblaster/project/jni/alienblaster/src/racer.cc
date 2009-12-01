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

#include "racer.h"
#include "surfaceDB.h"
#include "shot.h"
#include "shots.h"
#include "items.h"
#include "item.h"
#include "font.h"
#include "boundingBox.h"
#include "enemys.h"
#include "enemy.h"
#include "mixer.h"
#include "global.h"
#include "banners.h"
#include "shieldGlow.h"

Racer::Racer( string fnSprite, int whichPlayer, Vector2D startpos , int newShipType) {

  spriteRacerBase = surfaceDB.loadSurface( fnSprite );
  drawRectBase.w = spriteRacerBase->w;
  drawRectBase.h = spriteRacerBase->h;
  playerNr = whichPlayer;
  pos = startpos;
  shipType = newShipType;

  // init weaponary and specials
  timeLastShotPrimary   = SDL_GetTicks();
  timeLastShotSecondary = SDL_GetTicks();
  lastDumbfireWasLeft = false;
  timeLastHeatseekerUsed = SDL_GetTicks();
  timeLastNukeUsed = SDL_GetTicks();
  lastHeatseekerWasLeft = false;
  deflectorActive = false;
  deflectorTimeLeft = -1;

  for ( int i = 0; i < NR_SECONDARY_WEAPONS; i++ ) {
    secondaryWeaponsAvailability[ i ] = false;
  }
  for ( int i = 0; i < NR_SPECIALS; i++ ) {
    specialsAvailability[ i ] = 0;
  }
  specialsAvailability[ SPECIAL_NONE ] = 1; // this one is always present

  // assign values dependant on shiptype
  switch (shipType) {
    case LIGHT_FIGHTER:
    {
      maxShield = LIGHT_FIGHTER_MAX_SHIELD;
      maxDamage = LIGHT_FIGHTER_MAX_DAMAGE;
      shield = maxShield;
      damage = maxDamage;
      maxVel = LIGHT_FIGHTER_VEL_MAX;
      spriteShadow = surfaceDB.loadSurface( FN_LIGHT_FIGHTER_SHADOW, true );
      shieldGlow = new ShieldGlow( LIGHT_FIGHTER );
      if ( playerNr == 0 ) {
	spriteFighterIcon = surfaceDB.loadSurface( FN_LIGHT_FIGHTER_1_SMALL );
      } else {
	spriteFighterIcon = surfaceDB.loadSurface( FN_LIGHT_FIGHTER_2_SMALL );
      }
      if ( arcadeGame ) {
	secondaryWeaponsAvailability[ SHOT_DUMBFIRE_DOUBLE-100 ] = true;
	secondaryWeaponsAvailability[ SHOT_KICK_ASS_ROCKET-100 ] = true;
	secondaryWeaponsAvailability[ SHOT_HELLFIRE-100 ] = true;
	secondaryWeaponsAvailability[ SHOT_MACHINE_GUN-100 ] = true;
	secondaryWeaponsAvailability[ SHOT_ENERGY_BEAM-100 ] = true;
	primaryShotType = SHOT_TRIPLE;
	activeSecondary = SHOT_HELLFIRE;
	activeSpecial = SPECIAL_NUKE; switchSpecials(); // init specials
      } else {
	secondaryWeaponsAvailability[ 0 ] = true; // Dumbfire is always available
	primaryShotType = SHOT_NORMAL;
	activeSecondary = SHOT_DUMBFIRE;
	activeSpecial = SPECIAL_NUKE; switchSpecials(); // init specials
      }
      break;
    }  
    case HEAVY_FIGHTER:
    {  
      maxShield = HEAVY_FIGHTER_MAX_SHIELD;
      maxDamage = HEAVY_FIGHTER_MAX_DAMAGE;
      shield = maxShield;
      damage = maxDamage;
      maxVel = HEAVY_FIGHTER_VEL_MAX;
      spriteShadow = surfaceDB.loadSurface( FN_HEAVY_FIGHTER_SHADOW, true );
      shieldGlow = new ShieldGlow( HEAVY_FIGHTER );
      if ( playerNr == 0 ) {
	spriteFighterIcon = surfaceDB.loadSurface( FN_HEAVY_FIGHTER_1_SMALL );
      } else {
	spriteFighterIcon = surfaceDB.loadSurface( FN_HEAVY_FIGHTER_2_SMALL );
      }
      if ( arcadeGame ) {
	secondaryWeaponsAvailability[ SHOT_HF_DUMBFIRE_DOUBLE-100 ] = true;
	secondaryWeaponsAvailability[ SHOT_HF_KICK_ASS_ROCKET-100 ] = true;
	secondaryWeaponsAvailability[ SHOT_HF_LASER-100 ] = true;
	primaryShotType = SHOT_HF_QUINTO;
	activeSecondary = SHOT_HF_KICK_ASS_ROCKET;
	activeSpecial = SPECIAL_NUKE; switchSpecials(); // init specials
      } else {
	// Dumbfire is always available
	secondaryWeaponsAvailability[ SHOT_HF_DUMBFIRE-100 ] = true; 
	specialsAvailability[ SPECIAL_DEFLECTOR ] = 1;
	primaryShotType = SHOT_HF_NORMAL;
	activeSecondary = SHOT_HF_DUMBFIRE;
	specialsAvailability[ SPECIAL_NUKE ] = 1;
	activeSpecial = SPECIAL_NUKE; switchSpecials(); // init specials
      }
      break;
    }  
  }

  spriteHPStat = surfaceDB.loadSurface( FN_HITPOINTS_STAT );
  pixPerHP = spriteHPStat->w / (maxDamage + maxShield);

  shieldDamageEndTime = 0;
  shieldDamageActive = false;

  if ( whichPlayer == 0 ) {
    font = new Font( FN_FONT_NUMBERS_LEFT );
  } else {
    font = new Font( FN_FONT_NUMBERS_RIGHT );
  } 
  fontSize = font->getCharWidth();

  sndShotPrimary = mixer.loadSample( FN_SOUND_SHOT_PRIMARY );
  sndShotSecondary = mixer.loadSample( FN_SOUND_SHOT_SECONDARY );

  spriteSpecials = surfaceDB.loadSurface( FN_ICONS_SPECIALS );
  spriteSecondaryWeapons = surfaceDB.loadSurface( FN_ICONS_SECONDARY_WEAPONS );

  spriteDeflector = surfaceDB.loadSurface( FN_HEAVY_FIGHTER_DEFLECTOR, true );
  drawRectDeflector.w = spriteDeflector->w;
  drawRectDeflector.h = spriteDeflector->h;

  boundaryRect = 
    new RectangleGeo(Vector2D(spriteRacerBase->w/(2*RACER_IMAGE_CNT), spriteRacerBase->h/2),
		     Vector2D(320-spriteRacerBase->w/(2*RACER_IMAGE_CNT), 200-spriteRacerBase->h/2));

  boundingBox = new BoundingBox( lroundf(pos.getX() - (spriteRacerBase->w / RACER_IMAGE_CNT) * 0.45),
				 lroundf(pos.getY() - spriteRacerBase->h * 0.45),
				 lroundf((spriteRacerBase->w / RACER_IMAGE_CNT) * 0.9),
				 lroundf(spriteRacerBase->h * 0.9) );
  
  shipAngle = -90;
  vel = Vector2D(0,0);
  thrust = false;
  backwards = false;
  left = false;
  right = false;
  fireWeapons = false;
  useSpecial = false;
 
  points = 0;

  timeLastMove = SDL_GetTicks();
}


Racer::~Racer() {
  delete boundaryRect;
  delete boundingBox;
  if (font) delete font;
  delete shieldGlow;
}


Vector2D Racer::move( int dT ) {
  // TODO: move this to a correct place
  if ( deflectorActive ) {
    deflectorTimeLeft -= dT;
    if (deflectorTimeLeft < 0) deflectorActive = false;
  }

  Vector2D oldPos = pos;

  if ( thrust ) {
    pos += Vector2D( 0, -maxVel ) * dT / 1000.0;
  }
  if ( backwards ) {
    pos += Vector2D( 0, maxVel ) * dT / 1000.0;
  }
  if ( left ) {
    pos += Vector2D( -maxVel, 0 ) * dT / 1000.0;
  } 
  if ( right ) {
    pos += Vector2D( maxVel, 0 ) * dT / 1000.0;
  }
  
  clipWorld();
  
  updateBoundingBox();

  return oldPos;
}


void Racer::clipWorld() {
  int left = lroundf(pos.getX()) - (spriteRacerBase->w / (2*RACER_IMAGE_CNT));
  int top = lroundf(pos.getY()) - (spriteRacerBase->h / 2);
  int right = lroundf(pos.getX()) + (spriteRacerBase->w / (2*RACER_IMAGE_CNT));
  int bottom = lroundf(pos.getY()) + (spriteRacerBase->h / 2);
  if ( left <= 1 ) {
    pos.setX( 1 + spriteRacerBase->w / (2*RACER_IMAGE_CNT) );
  } else if ( right >= SCREEN_WIDTH - 1 ) {
    pos.setX( SCREEN_WIDTH - 2 - spriteRacerBase->w / (2*RACER_IMAGE_CNT));
  }
  if ( top <= 1 ) {
    pos.setY( 1 + spriteRacerBase->h / 2 );
  } else if ( bottom >= SCREEN_HEIGHT - 1 ) {
    pos.setY( SCREEN_HEIGHT - 2 - spriteRacerBase->h / 2 );
  }
}
    

void Racer::drawStats( SDL_Surface *screen ) {
  SDL_Rect srcR, destR;

  int indent = 5;

  if ( playerNr == 1 ) {
    indent = screen->w - 
      ( 7 + 3 * fontSize + (spriteSpecials->w / (NR_SPECIALS-1)) ) -
      ( 5 + spriteSecondaryWeapons->w / NR_SECONDARY_WEAPONS );
  }

  int x = indent;

  // draw Secondary Weapon
  srcR.x = (activeSecondary - SHOT_DUMBFIRE) * spriteSecondaryWeapons->w / NR_SECONDARY_WEAPONS;
  srcR.y = 0;
  srcR.w = spriteSecondaryWeapons->w / NR_SECONDARY_WEAPONS;
  srcR.h = spriteSecondaryWeapons->h;
  destR.x = x;
  destR.y = 25;
  destR.w = spriteSecondaryWeapons->w / NR_SECONDARY_WEAPONS;
  x += destR.w + 5;
  destR.h = spriteSecondaryWeapons->h;
  SDL_BlitSurface( spriteSecondaryWeapons, &srcR, screen, &destR );
  
  // draw active special and amount of it
  if ( activeSpecial != SPECIAL_NONE ) {
    int pixPerSpecial = spriteSpecials->w / (NR_SPECIALS - 1);
    srcR.y = 0;
    srcR.w = pixPerSpecial;
    srcR.h = spriteSpecials->h;
    srcR.x = (activeSpecial - SPECIAL_NUKE) * pixPerSpecial;
    destR.x = x;
    destR.y = 25;
    destR.w = pixPerSpecial;
    x += pixPerSpecial + 2;
    destR.h = spriteSpecials->h;
    SDL_BlitSurface( spriteSpecials, &srcR, screen, &destR );
    int digitCnt = 1;
    int i=1;
    while ( specialsAvailability[ activeSpecial ] >= i*10 ) {
      digitCnt++;
      i *= 10;
    }
    font->drawInt( screen, x, 25,
		   specialsAvailability[ activeSpecial ], digitCnt, 0);
  }

  if ( !onePlayerGame ) {
    // draw Icon
    switch (playerNr) {
    case 0: indent = 10 + spriteHPStat->w; break;
    case 1: indent = screen->w - (20 + spriteHPStat->w + spriteFighterIcon->w); break;
    default: break;
    }
    destR.x = indent;
    destR.w = spriteFighterIcon->w;
    destR.y = 2;
    destR.h = spriteFighterIcon->h;
    SDL_BlitSurface( spriteFighterIcon, 0, screen, &destR );
  }

  // draw shields and damage
  switch (playerNr) {
  case 0: indent = 5; break;
  case 1: indent = screen->w - (15 + spriteHPStat->w); break;
  default: break;
  }

  // draw damage
  srcR.x = 0;
  srcR.y = 0;
  srcR.w = lroundf(damage * pixPerHP);
  srcR.h = spriteHPStat->h;
  destR.x = indent;
  destR.w = srcR.w;
  destR.y = 5;
  destR.h = spriteHPStat->h;
  SDL_BlitSurface( spriteHPStat, &srcR, screen, &destR );
  // draw shield
  destR.x = indent + srcR.w;
  srcR.x = spriteHPStat->w / 2;
  srcR.w = lroundf(shield * pixPerHP);
  destR.w = srcR.w;
  destR.h = spriteHPStat->h;
  SDL_BlitSurface( spriteHPStat, &srcR, screen, &destR );
}


void Racer::drawShadow( SDL_Surface *screen ) {
  SDL_Rect destR;
  destR.x = lroundf(pos.getX()) - (spriteShadow->w / 2) - 10;
  destR.y = lroundf(pos.getY()) - (spriteShadow->h / 2) + 10;
  destR.w = spriteShadow->w;
  destR.h = spriteShadow->h;
  SDL_BlitSurface( spriteShadow, 0, screen, &destR );
}


void Racer::drawRacer( SDL_Surface *screen ) {

  SDL_Rect srcR;
  SDL_Rect destR;

  destR.x = lroundf(pos.getX()) - (spriteRacerBase->w / (2*RACER_IMAGE_CNT));
  destR.y = lroundf(pos.getY()) - (spriteRacerBase->h / 2);
  destR.w = spriteRacerBase->w / RACER_IMAGE_CNT;
  destR.h = spriteRacerBase->h;
  
  int idx = 0; // normal pos
  if ( !left && !right && !thrust && !backwards ) idx = 0; //stop
  else if ( left && !right && !thrust && !backwards ) idx = 1; // left
  else if ( !left && right && !thrust && !backwards ) idx = 2; // right
  else if ( !left && !right && thrust && !backwards ) idx = 3; // forward
  else if ( !left && !right && !thrust && backwards ) idx = 4; // backward
  else if ( left && !right && thrust && !backwards ) idx = 5; // left-forward
  else if ( !left && right && thrust && !backwards ) idx = 6; // right-forward
  else if ( left && !right && !thrust && backwards ) idx = 7; // left-backward
  else if ( !left && right && !thrust && backwards ) idx = 8; // right-backward
  srcR.x = idx * (spriteRacerBase->w / RACER_IMAGE_CNT);
  srcR.y = 0;
  srcR.w = (spriteRacerBase->w / RACER_IMAGE_CNT);
  srcR.h = spriteRacerBase->h;
  
  SDL_BlitSurface( spriteRacerBase, &srcR, screen, &destR );

  if ( shieldDamageActive ) {
    int shieldGlowTimeLeft = shieldDamageEndTime - SDL_GetTicks();
    if ( shieldGlowTimeLeft < 0 ) {
      shieldDamageActive = false;
    } else {
      shieldGlow->draw( screen, pos, RACER_SHIELD_DAMAGE_LIFETIME - shieldGlowTimeLeft );
    }
  }

  if ( deflectorActive ) {
    drawRectDeflector.x = lroundf(pos.getX()) - spriteDeflector->w / 2;
    drawRectDeflector.y = lroundf(pos.getY()) - spriteDeflector->h / 2;
    SDL_BlitSurface( spriteDeflector, 0, screen, &drawRectDeflector );
  }
}


void Racer::switchSecondary() {
  int newActive = (activeSecondary-100 + 1) % NR_SECONDARY_WEAPONS;
  while ( newActive != activeSecondary-100 &&
	  secondaryWeaponsAvailability[ newActive ] == false ) {
    newActive = (newActive + 1) % NR_SECONDARY_WEAPONS;
  }
  activeSecondary = (ShotTypes)(newActive+100);
  
  if ( activeSecondary == SHOT_DUMBFIRE && 
       secondaryWeaponsAvailability[ SHOT_DUMBFIRE_DOUBLE - 100 ] ) {
    activeSecondary = SHOT_DUMBFIRE_DOUBLE;
  }
  // same for heavy-fighter
  if ( activeSecondary == SHOT_HF_DUMBFIRE && 
       secondaryWeaponsAvailability[ SHOT_HF_DUMBFIRE_DOUBLE - 100 ] ) {
    activeSecondary = SHOT_HF_DUMBFIRE_DOUBLE;
  }

  // TODO: ugly workaround...
  if ( activeSecondary == SHOT_KICK_ASS_ROCKET ||
       activeSecondary == SHOT_HF_KICK_ASS_ROCKET ) {
    timeLastShotSecondary -= RACER_COOLDOWN_KICK_ASS_ROCKET / 2;
  }
}

void Racer::switchSpecials() {
  int newActive = activeSpecial + 1;
  if ( newActive == NR_SPECIALS ) newActive = 1;
  for(int i=0; i < NR_SPECIALS-1; ++i) {
    // Only Sönke knows what the following does ;-)
    if (specialsAvailability[ ((activeSpecial + i) % (NR_SPECIALS-1))+1 ] != 0) {
      activeSpecial = (SpecialTypes)(((activeSpecial + i) % (NR_SPECIALS-1))+1);
      return;
    }
  }
  activeSpecial = SPECIAL_NONE;
}


void Racer::shootPrimary() {
  if ( fireWeapons ) {

    switch ( primaryShotType ) {
    case SHOT_NORMAL:
      {
	if ( (SDL_GetTicks() - RACER_COOLDOWN_SHOT_NORMAL) > timeLastShotPrimary ) {
	  Shot *shot = 
	    new Shot( SHOT_NORMAL, playerNr,
		      pos + Vector2D(0, -spriteRacerBase->h / 2),
		      -90 + (rand() % SPREAD_ANGLE_SHOT_NORMAL) -
		      SPREAD_ANGLE_SHOT_NORMAL / 2 );
	  shots->addShot( shot );
	  mixer.playSample( sndShotPrimary, 0 );
	  timeLastShotPrimary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_NORMAL_HEAVY:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_SHOT_NORMAL_HEAVY > timeLastShotPrimary ) {
	  Shot *shot = 
	    new Shot( SHOT_NORMAL_HEAVY, playerNr,
		      pos + Vector2D(0, -spriteRacerBase->h / 2), -90 );
	  shots->addShot( shot );
	  mixer.playSample( sndShotPrimary, 0 );
	  timeLastShotPrimary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_DOUBLE:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_SHOT_DOUBLE > timeLastShotPrimary ) {
	  Shot *shot1 = 
	    new Shot( SHOT_DOUBLE, playerNr, pos + Vector2D(-8,-2), -95 );
	  Shot *shot2 = 
	    new Shot( SHOT_DOUBLE, playerNr, pos + Vector2D(8,-2), -85 );
	  shots->addShot(shot1);
	  shots->addShot(shot2);
	  mixer.playSample( sndShotPrimary, 0 );
	  timeLastShotPrimary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_DOUBLE_HEAVY:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_SHOT_DOUBLE_HEAVY > timeLastShotPrimary ) {
	  Shot *shot1 = 
	    new Shot( SHOT_DOUBLE_HEAVY, playerNr, pos + Vector2D(-8,-2), -95 );
	  Shot *shot2 = 
	    new Shot( SHOT_DOUBLE_HEAVY, playerNr, pos + Vector2D(8,-2), -85 );
	  shots->addShot(shot1);
	  shots->addShot(shot2);
	  mixer.playSample( sndShotPrimary, 0 );
	  timeLastShotPrimary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_TRIPLE:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_SHOT_TRIPLE > timeLastShotPrimary ) {
	  Shot *shot1 = 
	    new Shot( SHOT_TRIPLE, playerNr, pos + Vector2D(-8, -2), -100 );
	  Shot *shot2 = 
	    new Shot( SHOT_TRIPLE, playerNr, pos + Vector2D(0, -spriteRacerBase->h / 2), -90 );
	  Shot *shot3 = 
	    new Shot( SHOT_TRIPLE, playerNr, pos + Vector2D(8, -2), -80 );
	  shots->addShot(shot1);
	  shots->addShot(shot2);
	  shots->addShot(shot3);
	  mixer.playSample( sndShotPrimary, 0 );
	  timeLastShotPrimary = SDL_GetTicks();
	}
	break;
      }
      
      // HEAVY FIGHTER SHOTS
    case SHOT_HF_NORMAL:
      {
	if ( (SDL_GetTicks() - RACER_COOLDOWN_SHOT_HF_NORMAL) > timeLastShotPrimary ) {
	  Shot *shot = 
	    new Shot( SHOT_HF_NORMAL, playerNr,
		      pos + Vector2D(0, -spriteRacerBase->h / 2), -90 );
	  shots->addShot( shot );
	  mixer.playSample( sndShotPrimary, 0 );
	  timeLastShotPrimary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_HF_DOUBLE:
      {
	if ( (SDL_GetTicks() - RACER_COOLDOWN_SHOT_HF_DOUBLE) > timeLastShotPrimary ) {
	  Shot *shot = new Shot( SHOT_HF_DOUBLE, playerNr, pos + Vector2D(-12, -12), -90 );
	  shots->addShot( shot );
	  shot = new Shot( SHOT_HF_DOUBLE, playerNr, pos + Vector2D(12, -12), -90 );
	  shots->addShot( shot );
	  mixer.playSample( sndShotPrimary, 0 );
	  timeLastShotPrimary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_HF_TRIPLE:
      {
	if ( (SDL_GetTicks() - RACER_COOLDOWN_SHOT_HF_TRIPLE) > timeLastShotPrimary ) {
	  Shot *shot = new Shot( SHOT_HF_TRIPLE, playerNr, pos + Vector2D(-12, -12), -103 );
	  shots->addShot( shot );
	  shot = new Shot( SHOT_HF_TRIPLE, playerNr, 
			   pos + Vector2D(0, -spriteRacerBase->h / 2), -90 );
	  shots->addShot( shot );
	  shot = new Shot( SHOT_HF_TRIPLE, playerNr, pos + Vector2D(12, -12), -77 );
	  shots->addShot( shot );
	  mixer.playSample( sndShotPrimary, 0 );
	  timeLastShotPrimary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_HF_QUATTRO:
      {
	if ( (SDL_GetTicks() - RACER_COOLDOWN_SHOT_HF_QUATTRO) > timeLastShotPrimary ) {
	  Shot *shot = new Shot( SHOT_HF_QUATTRO, playerNr, pos + Vector2D(-30, -15), -100 );
	  shots->addShot( shot );
	  shot = new Shot( SHOT_HF_QUATTRO, playerNr, pos + Vector2D(-12, -12), -90 );
	  shots->addShot( shot );
	  shot = new Shot( SHOT_HF_QUATTRO, playerNr, pos + Vector2D(12, -12), -90 );
	  shots->addShot( shot );
	  shot = new Shot( SHOT_HF_QUATTRO, playerNr, pos + Vector2D(30, -15), -80 );
	  shots->addShot( shot );
	  mixer.playSample( sndShotPrimary, 0 );
	  timeLastShotPrimary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_HF_QUINTO:
      {
	if ( (SDL_GetTicks() - RACER_COOLDOWN_SHOT_HF_QUINTO) > timeLastShotPrimary ) {
	  Shot *shot = new Shot( SHOT_HF_QUINTO, playerNr, pos + Vector2D(-30, -15), -110 );
	  shots->addShot( shot );
	  shot = new Shot( SHOT_HF_QUINTO, playerNr, pos + Vector2D(-12, -12), -100 );
	  shots->addShot( shot );
	  shot = new Shot( SHOT_HF_QUINTO, playerNr, 
			   pos + Vector2D(0, -spriteRacerBase->h / 2), -90 );
	  shots->addShot( shot );
	  shot = new Shot( SHOT_HF_QUINTO, playerNr, pos + Vector2D(12, -12), -80 );
	  shots->addShot( shot );
	  shot = new Shot( SHOT_HF_QUINTO, playerNr, pos + Vector2D(30, -15), -70 );
	  shots->addShot( shot );
	  mixer.playSample( sndShotPrimary, 0 );
	  timeLastShotPrimary = SDL_GetTicks();
	}
	break;
      }

    default: 
      {
	cout << "Racer::shootPrimary: unexpected primaryShotType: " 
	     << primaryShotType << endl;
	break;
      }
    }
  }
}


void Racer::shootSecondary() {
  if ( fireWeapons ) {

    switch ( activeSecondary ) {

    case SHOT_DUMBFIRE:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_DUMBFIRE > timeLastShotSecondary ) {
	  Shot *shot = 
	    new Shot( SHOT_DUMBFIRE, playerNr, pos + Vector2D(0, -5), -90 );
	  shots->addShot(shot);
	  mixer.playSample( sndShotSecondary, 0 );
	  timeLastShotSecondary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_DUMBFIRE_DOUBLE:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_DUMBFIRE_DOUBLE > timeLastShotSecondary ) {
	  Shot *shot;
	  if ( lastDumbfireWasLeft ) {
	    shot = new Shot( SHOT_DUMBFIRE_DOUBLE, playerNr, pos + Vector2D(15,-3), -90 );
	  } else {
	    shot = new Shot( SHOT_DUMBFIRE_DOUBLE, playerNr, pos + Vector2D(-15,-3), -90 );
	  }
	  lastDumbfireWasLeft = !lastDumbfireWasLeft;
	  shots->addShot(shot);
	  mixer.playSample( sndShotSecondary, 0 );
	  timeLastShotSecondary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_KICK_ASS_ROCKET:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_KICK_ASS_ROCKET > timeLastShotSecondary ) {
	  Shot *shot = 
	    new Shot( SHOT_KICK_ASS_ROCKET, playerNr, pos, -90 );
	  shots->addShot(shot);
	  mixer.playSample( sndShotSecondary, 0 );
	  timeLastShotSecondary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_HELLFIRE:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_HELLFIRE > timeLastShotSecondary ) {
	  Shot *shot1 = 
	    new Shot( SHOT_HELLFIRE, playerNr, pos + Vector2D(-15, -3), -180 );
	  Shot *shot2 = 
	    new Shot( SHOT_HELLFIRE, playerNr, pos + Vector2D(15, -3), 0 );
	  shots->addShot(shot1);
	  shots->addShot(shot2);
	  mixer.playSample( sndShotSecondary, 0 );
	  timeLastShotSecondary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_MACHINE_GUN:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_MACHINE_GUN > timeLastShotSecondary ) {

	  int idxNearestEnemy1 = 0;
	  float distFarestY1 = -1;
	  int idxNearestEnemy2 = 0;
	  float distFarestY2 = -1;
	  float angle1 = -90;
	  float angle2 = -90;
	  float maxDistY = pos.getY() + 40; // only enemys in or already in the screen
	  for ( unsigned int i = 0; i < enemys->getNrEnemys(); i++ ) {
	    // angle of the vector from the racer position to the enemy
	    float angleEnemy = (enemys->getEnemy(i)->getPos() - pos).getDirection();
	    if ( -90 <= angleEnemy && angleEnemy < -60 ) {
	      float distY = pos.getY() - enemys->getEnemy(i)->getPos().getY();
	      if ( distFarestY1 < distY && distY < maxDistY ) {
		distFarestY1 = distY;
		idxNearestEnemy1 = i;
		angle2 = angleEnemy + 5;
	      }
	    } else {
	      if ( -120 < angleEnemy && angleEnemy < -90 ) {
		float distY = pos.getY() - enemys->getEnemy(i)->getPos().getY();
		if ( distFarestY2 < distY && distY < maxDistY ) {
		  distFarestY2 = distY;
		  idxNearestEnemy2 = i;
		  angle1 = angleEnemy - 5;
		}
	      }
	    }
	  }
	  
	  Shot *shot1 = 
	    new Shot( SHOT_MACHINE_GUN, playerNr, 
		      pos + Vector2D(-3, -spriteRacerBase->h/2), angle1 );
	  Shot *shot2 = 
	    new Shot( SHOT_MACHINE_GUN, playerNr, 
		      pos + Vector2D(+3, -spriteRacerBase->h/2), angle2 );
	  shots->addShot(shot1);
	  shots->addShot(shot2);
	  mixer.playSample( sndShotSecondary, 0 );
	  timeLastShotSecondary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_ENERGY_BEAM:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_ENERGY_BEAM > timeLastShotSecondary ) {
	  Shot *shot = 
	    new Shot( SHOT_ENERGY_BEAM, playerNr, pos + Vector2D(-1, -10), -90 );
	  shots->addShot(shot);
	  mixer.playSample( sndShotSecondary, 0 );
	  timeLastShotSecondary = SDL_GetTicks();
	}
	break;
      }
      
      // HEAVY FIGHTER SHOTS
    case SHOT_HF_DUMBFIRE:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_HF_DUMBFIRE > timeLastShotSecondary ) {
	  Shot *shot = 
	    new Shot( SHOT_HF_DUMBFIRE, playerNr, pos + Vector2D(-30, -5), -90 );
	  shots->addShot(shot);
	  shot = 
	    new Shot( SHOT_HF_DUMBFIRE, playerNr, pos + Vector2D(30, -5), -90 );
	  shots->addShot(shot);
	  mixer.playSample( sndShotSecondary, 0 );
	  timeLastShotSecondary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_HF_DUMBFIRE_DOUBLE:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_HF_DUMBFIRE_DOUBLE > timeLastShotSecondary ) {
	  Shot *shot = 
	    new Shot( SHOT_HF_DUMBFIRE_DOUBLE, playerNr, pos + Vector2D(-30, -5), -90 );
	  shots->addShot(shot);
	  shot = 
	    new Shot( SHOT_HF_DUMBFIRE_DOUBLE, playerNr, pos + Vector2D(30, -5), -90 );
	  shots->addShot(shot);
	  mixer.playSample( sndShotSecondary, 0 );
	  timeLastShotSecondary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_HF_KICK_ASS_ROCKET:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_HF_KICK_ASS_ROCKET > timeLastShotSecondary ) {
	  Shot *shot1 = 
	    new Shot( SHOT_HF_KICK_ASS_ROCKET, playerNr, pos + Vector2D(-30,0), -90 );
	  shots->addShot(shot1);
	  Shot *shot2 = 
	    new Shot( SHOT_HF_KICK_ASS_ROCKET, playerNr, pos + Vector2D(30,0), -90 );
	  shots->addShot(shot2);
	  mixer.playSample( sndShotSecondary, 0 );
	  timeLastShotSecondary = SDL_GetTicks();
	}
	break;
      }
    case SHOT_HF_LASER:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_HF_LASER > timeLastShotSecondary ) {
	  Shot *shot1 = 
	    new Shot( SHOT_HF_LASER, playerNr, pos + Vector2D(-30,-10), -90 );
	  shots->addShot(shot1);
	  Shot *shot2 = 
	    new Shot( SHOT_HF_LASER, playerNr, pos + Vector2D(30,-10), -90 );
	  shots->addShot(shot2);
	  mixer.playSample( sndShotSecondary, 0 );
	  timeLastShotSecondary = SDL_GetTicks();
	}
	break;
      }

    default: 
      {
	cout << "Racer::shootSecondary: unexpected activeSecondary: " 
	     << activeSecondary << endl;
	break;
      }      
    }

  }
}


void Racer::shootSpecial() {
  if ( useSpecial ) {

    switch ( activeSpecial ) {

    case SPECIAL_HEATSEEKER:
      {
	if ( SDL_GetTicks() - RACER_COOLDOWN_SPECIAL_HEATSEEKER > timeLastHeatseekerUsed &&
	     specialsAvailability[ SPECIAL_HEATSEEKER ] > 0 ) {
	  Shot *shot;
	  if ( lastHeatseekerWasLeft ) {
	    shot = new Shot( SPECIAL_SHOT_HEATSEEKER, playerNr, pos + Vector2D(15,-3), -90 );
	  } else {
	    shot = new Shot( SPECIAL_SHOT_HEATSEEKER, playerNr, pos + Vector2D(-15,-3), -90 );
	  }
	  lastHeatseekerWasLeft = !lastHeatseekerWasLeft;
	  shots->addShot(shot);
	  mixer.playSample( sndShotSecondary, 0 );
	  timeLastHeatseekerUsed = SDL_GetTicks();
	  specialsAvailability[ SPECIAL_HEATSEEKER ]--;
	}
	break;
      }
    default: break;
    }
  }
}


bool Racer::collidesWith( const Vector2D &shotPosOld, const Vector2D &shotPosNew ) {
  return boundingBox->overlaps(shotPosOld, shotPosNew);
}

bool Racer::collidesWith( BoundingBox *box ) {
  return boundingBox->overlaps( box );
}

bool Racer::collidesWith( const Circle &circle ) {
  return boundingBox->overlaps( circle );
}

bool Racer::collidesWithAsCircle( const Circle &circle ) {
  return ( (circle.getRadius() + (spriteRacerBase->w / RACER_IMAGE_CNT)/2) > circle.getCenter().distanceTo( pos ) );
}

bool Racer::collidesWithAsCircle( BoundingBox *box ) {
  return ( box->overlaps( Circle( pos, spriteRacerBase->h >> 1 ) ) );
}

BoundingBox *Racer::getBoundingBox() {
  return boundingBox;
}


bool Racer::isDead() {
  return (damage < 0);
}


void Racer::receiveDamage( float amount ) {
  if ( lroundf(shield) >= amount ) {
    shield -= amount;
    shieldDamageActive = true;
    shieldDamageEndTime = SDL_GetTicks() + RACER_SHIELD_DAMAGE_LIFETIME;
    return;
  }
  amount -= shield;
  shield = 0;
  damage -= amount;
}

void Racer::doDamage( ShotTypes shotType ) {

  switch (shotType) {
  case ENEMY_SHOT_NORMAL:
    {
      receiveDamage( DAMAGE_ENEMY_SHOT_NORMAL );
      break;
    }
  case ENEMY_SHOT_TANK_ROCKET:
    {
      receiveDamage( DAMAGE_ENEMY_SHOT_TANK_ROCKET );
      break;
    }
  default: 
    {
      cout << "Racer::doDamage: unexpected shotType: " << shotType << endl;
      break;
    }
  }
}

void Racer::rechargeShield ( int dT ) {
  float amount;
  if ( shipType == LIGHT_FIGHTER ) {
    amount = LIGHT_FIGHTER_SHIELD_RECHARGE * dT / 100000.0;
  } else {
    amount = HEAVY_FIGHTER_SHIELD_RECHARGE * dT / 100000.0;
  }
  if (shield + amount < maxShield) shield += amount;
  else shield = maxShield;
}

void Racer::receivePoints( float amount ) {
  float oldPoints = points;
  points += amount;

  if ( arcadeGame ) {
    // time for a health item?
    for ( int i = 0; i < NR_ARCACE_POINTS_FOR_HEALTH_ITEM; i++ ) {
      if ( oldPoints < ARCADE_POINTS_FOR_HEALTH_ITEM[ i ] &&
	   ARCADE_POINTS_FOR_HEALTH_ITEM[ i ] <= points ) {
	Item *health = new Item( Vector2D( SCREEN_WIDTH / 2, -30 ),
				 Vector2D( 0, 30 ),
				 ITEM_HEALTH );
	items->addItem( health );
	banners->addBanner( BANNER_HEALTH, BANNER_MODE_FLY_FROM_LEFT, BANNER_BONUS_NONE );
	break;
      }
    }
  }

}


Vector2D Racer::setVel( const Vector2D &newVel ) {
  Vector2D temp = vel;
  vel = newVel;
  if ( vel.getLength() >= getActVelMax() ) {
    vel.setLength(getActVelMax());
  }
  return temp;
}


void Racer::setPos( const Vector2D &newPos ) {
  pos = newPos;
  updateBoundingBox();
}


void Racer::updateBoundingBox() {
  boundingBox->moveUpperBound( lroundf(pos.getY() - spriteRacerBase->h * 0.45) );
  boundingBox->moveLeftBound( lroundf(pos.getX() - (spriteRacerBase->w / RACER_IMAGE_CNT) * 0.45) );
}


void Racer::pickUpItems() {
  for ( unsigned int i = 0; i < items->getNrItems(); i++ ) {
    Item *item = items->getItem(i);
    if ( !item->isExpired() && boundingBox->overlaps( item->getBoundingBox() ) ) {
      switch (item->getType()) {
      case ITEM_PRIMARY_UPGRADE: 
	{
	  int newPrimaryShotType = primaryShotType + 1;
	  if ( shipType == LIGHT_FIGHTER && newPrimaryShotType <= SHOT_TRIPLE ) 
	    primaryShotType = (ShotTypes)newPrimaryShotType;
	  else if ( shipType == HEAVY_FIGHTER && newPrimaryShotType <= SHOT_HF_QUINTO )
	    primaryShotType = (ShotTypes)newPrimaryShotType;
	  item->pickedUp();
	  addBannerItemCollected( BANNER_ITEM_PRIMARY_UPGRADE_COLLECTED );
	  break;
	}
      case ITEM_DUMBFIRE_DOUBLE:
	{
	  if ( shipType == LIGHT_FIGHTER ) {
	    secondaryWeaponsAvailability[ SHOT_DUMBFIRE_DOUBLE - 100 ] = true;
	    if (activeSecondary == SHOT_DUMBFIRE ) activeSecondary = SHOT_DUMBFIRE_DOUBLE;
	  } else {
	    secondaryWeaponsAvailability[ SHOT_HF_DUMBFIRE_DOUBLE - 100 ] = true;
	    if (activeSecondary == SHOT_HF_DUMBFIRE ) activeSecondary = SHOT_HF_DUMBFIRE_DOUBLE;
	  }
	  item->pickedUp();
	  addBannerItemCollected( BANNER_ITEM_DUMBFIRE_DOUBLE_COLLECTED );
	  break;
	}
      case ITEM_KICK_ASS_ROCKET:
	{
	  if ( shipType == LIGHT_FIGHTER )
	    secondaryWeaponsAvailability[ SHOT_KICK_ASS_ROCKET - 100 ] = true;
	  else
	    secondaryWeaponsAvailability[ SHOT_HF_KICK_ASS_ROCKET - 100 ] = true;
	  item->pickedUp();
	  addBannerItemCollected( BANNER_ITEM_KICK_ASS_ROCKET_COLLECTED );
	  break;
	}
      case ITEM_HELLFIRE:
	{
	  if ( shipType == LIGHT_FIGHTER ) {
	    secondaryWeaponsAvailability[ SHOT_HELLFIRE - 100 ] = true;
	    item->pickedUp();
	    addBannerItemCollected( BANNER_ITEM_HELLFIRE_COLLECTED );
	  }
	  break;
	}
      case ITEM_MACHINE_GUN:
	{
	  if ( shipType == LIGHT_FIGHTER ) {
	    secondaryWeaponsAvailability[ SHOT_MACHINE_GUN - 100 ] = true;
	    item->pickedUp();
	    addBannerItemCollected( BANNER_ITEM_MACHINE_GUN_COLLECTED );
	  }
	  break;
	}
      case ITEM_HEALTH:
	{
	  if ( arcadeGame ) {
	    repair( ITEM_HEALTH_REPAIR_AMOUNT );
	  } else {
	    if ( shipType == LIGHT_FIGHTER )
	      repair( ITEM_HEALTH_REPAIR_AMOUNT );
	    else repair( ITEM_HEALTH_REPAIR_AMOUNT * ITEM_HEALTH_REPAIR_FACTOR_HEAVY_FIGHTER );
	  }
	  item->pickedUp();
	  addBannerItemCollected( BANNER_ITEM_HEALTH_COLLECTED );
	  break;
	}
      case ITEM_HEATSEEKER:
	{
	  specialsAvailability[ SPECIAL_HEATSEEKER ] += ITEM_HEATSEEKER_AMMO;
	  activeSpecial = SPECIAL_HEATSEEKER;
	  item->pickedUp();
	  addBannerItemCollected( BANNER_ITEM_HEATSEEKER_COLLECTED );
	  break;
	}
      case ITEM_NUKE:
	{
	  specialsAvailability[ SPECIAL_NUKE ] += ITEM_NUKE_AMMO;
	  if ( activeSpecial == SPECIAL_NONE ) {
	    activeSpecial = SPECIAL_NUKE;
	  }
	  item->pickedUp();
	  addBannerItemCollected( BANNER_ITEM_NUKE_COLLECTED );
	  break;
	}
      case ITEM_DEFLECTOR:
	{
	  if (shipType == HEAVY_FIGHTER ) {
	    specialsAvailability[ SPECIAL_DEFLECTOR ] += ITEM_DEFLECTOR_AMMO;
	    if ( activeSpecial == SPECIAL_NONE ) {
	      activeSpecial = SPECIAL_DEFLECTOR;
	    }
	    item->pickedUp();
	    addBannerItemCollected( BANNER_ITEM_DEFLECTOR_COLLECTED );
	  }
	  break;
	}
      case ITEM_ENERGY_BEAM:
	{
	  if ( shipType == LIGHT_FIGHTER ) {
	    secondaryWeaponsAvailability[ SHOT_ENERGY_BEAM - 100 ] = true;
	    item->pickedUp();
	    addBannerItemCollected( BANNER_ITEM_ENERGY_BEAM_COLLECTED );
	  }
	  break;
	}
      case ITEM_LASER:
	{
	  if ( shipType == HEAVY_FIGHTER ) {
	    secondaryWeaponsAvailability[ SHOT_HF_LASER - 100 ] = true;
	    addBannerItemCollected( BANNER_ITEM_LASER_COLLECTED );
	    item->pickedUp();
	  }
	  break;
	}
	  
      default:
	{
	  cout << "pickUpItems: unexpected itemType: " << item->getType() << endl;
	  break;
	}
      }
    }
  }
}


float Racer::getActVelMax() {
  return maxVel;
}


void Racer::repair( float amount ) {
  float possibleDamageAdd = maxDamage - damage;
  if ( possibleDamageAdd >= amount ) {
    damage += amount;
  } else {
    damage += possibleDamageAdd;
    amount -= possibleDamageAdd;
    shield += amount;
  }
  if ( shield > maxShield ) shield = maxShield;
}


void Racer::specialKeyDown() {
  switch ( activeSpecial ) {
  case SPECIAL_NUKE: 
    {
      if ( timeLastNukeUsed + RACER_COOLDOWN_SPECIAL_NUKE < SDL_GetTicks() ) {
	shots->addShot( new Shot( SPECIAL_SHOT_NUKE,
				  playerNr, pos, 
				  (Vector2D( SCREEN_WIDTH / 2.0, 
					     SCREEN_HEIGHT / 2.0 ) - pos).getDirection() ) );
	specialsAvailability[ SPECIAL_NUKE ]--;
	timeLastNukeUsed = SDL_GetTicks();
      }
      break;
    }
  case SPECIAL_DEFLECTOR:
    {
      if ( !deflectorActive ) {
	deflectorActive = true;
	deflectorTimeLeft = ITEM_DEFLECTOR_DURATION;
	specialsAvailability[ SPECIAL_DEFLECTOR ]--;
      }
      break;
    }
  default:
    {
      break;
    }
  }
  if ( specialsAvailability[ activeSpecial ] == 0 ) {
    switchSpecials();
  }
}

void Racer::specialKeyUp() {
  if ( specialsAvailability[ activeSpecial ] == 0 ) {
    switchSpecials();
  }
}

void Racer::handlePlayerEvent( PlayerEvent pEvent, bool keyDown ) {
  switch (pEvent) {
  case PE_UP: if ( keyDown ) thrust = true; else thrust = false; break;
  case PE_DOWN: if ( keyDown ) backwards = true; else backwards = false; break;
  case PE_LEFT: if ( keyDown ) left = true; else left = false; break;
  case PE_RIGHT: if ( keyDown ) right = true; else right = false; break;
  case PE_FIRE_WEAPONS: if ( keyDown ) fireWeapons = true; else fireWeapons = false; break;
  case PE_CHOOSE_WEAPON_SECONDARY: 
    {
      if ( !keyDown ) { 
	switchSecondary();
      }
      break;
    }
  case PE_CHOOSE_WEAPON_SPECIALS: 
    {
      if ( !keyDown ) {
	switchSpecials();
      }
      break;
    }
  case PE_FIRE_SPECIALS: 
    {
      if ( keyDown ) {
	useSpecial = true; 
	specialKeyDown();
      } else {
	useSpecial = false; 
	specialKeyUp();
	break;
      }
    }
  default: break;
  }
}


void Racer::addBannerItemCollected( BannerTexts text ) {
  if ( onePlayerGame ) {
    banners->addBanner( text,
			BANNER_MODE_ITEM_COLLECTED_SINGLE_PLAYER,
			BANNER_BONUS_NONE );
  } else {
    if ( playerNr == 0 ) {
      banners->addBanner( text,
			  BANNER_MODE_ITEM_COLLECTED_PLAYER_ONE,
			  BANNER_BONUS_NONE );
    } else {
      banners->addBanner( text,
			  BANNER_MODE_ITEM_COLLECTED_PLAYER_TWO,
			  BANNER_BONUS_NONE );
    }
  }
}

      
      

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
#ifndef RACER_HH
#define RACER_HH

#include "SDL.h"
#include "geometry.h"
#include <string>
#include "global.h"
#include "settings.h"

class SurfaceDB;
class Gate;
class Track;
class Shot;
class Shots;
class Items;
class Font;
class BoundingBox;
class ShieldGlow;


/* The Racer is the vehicle, that the player can steer. */
class Racer {

  SdlCompat_AcceleratedSurface *spriteRacerBase;
  SDL_Rect drawRectBase;
  SdlCompat_AcceleratedSurface *spriteShadow;
  SdlCompat_AcceleratedSurface *spriteDeflector;
  SDL_Rect drawRectDeflector;
  SdlCompat_AcceleratedSurface *spriteHPStat;
  SdlCompat_AcceleratedSurface *spriteFighterIcon;

  // for collision with other racers or shots. 
  // A rectangle with racersize * 0.9 is used.
  BoundingBox *boundingBox;

  int playerNr;
  int shipType;

  Font *font; // font used for displaying ammo and lapcnt
  int fontSize;
  SdlCompat_AcceleratedSurface *spriteSecondaryWeapons;
  SdlCompat_AcceleratedSurface *spriteSpecials;

  int sndShotPrimary;
  int sndShotSecondary;
  
  float points;

  // Movement-System
  Vector2D pos; // absolute position
  float shipAngle; // the orientation
  Vector2D vel; // the velocity vector
  // the boundary of the world (i.e. the reachable section of the screen)
  RectangleGeo *boundaryRect;

  // Weapon-System
  ShotTypes primaryShotType;
  Uint32 timeLastShotPrimary;

  ShotTypes activeSecondary;
  Uint32 timeLastShotSecondary;
  bool secondaryWeaponsAvailability[ NR_SECONDARY_WEAPONS ];
  bool lastDumbfireWasLeft;

  SpecialTypes activeSpecial;
  int specialsAvailability[ NR_SPECIALS ];
  Uint32 timeLastNukeUsed;
  Uint32 timeLastHeatseekerUsed;
  bool lastHeatseekerWasLeft;

  bool deflectorActive;
  int deflectorTimeLeft;
  
  // Damage-System
  float shield;
  float damage;
  float pixPerHP;
  float maxShield;
  float maxDamage;
  // not used at the moment:
  // returns the maximum reachable velocity. 
  // if damage < 100 the reachable velocity will be reduced linearly
  float getActVelMax();
  float maxVel;
  // needed for displaying the shield glow, when hit
  Uint32 shieldDamageEndTime;
  bool shieldDamageActive;
  ShieldGlow *shieldGlow;

  Uint32 timeLastMove;

  void repair( float amount );
  
  bool thrust;
  bool backwards;
  bool left;
  bool right;
  bool fireWeapons;
  bool useSpecial;

  void addBannerItemCollected( BannerTexts text );

  public:
  // Input-System - which keys are pressed at the moment
  void handlePlayerEvent( PlayerEvent pEvent, bool keyDown );

  Racer( string fnSprite, int whichPlayer, Vector2D startpos, int newShipType );
  ~Racer();

  bool isDeflectorSpecialActive() { return deflectorActive; }
  inline int getPlayerNr() { return playerNr; }
  inline int getShipType() { return shipType; }
  void setPos( const Vector2D &newPos );
  inline Vector2D getPos() { return pos; }
  inline Vector2D getVel() { return vel; }
  Vector2D setVel( const Vector2D &newVel ); // returns old vel

  // moves the racer according to his velocity and the pressed keys. 
  // Already collides with the boundaryRect (the border of the world)
  Vector2D move( int dT );
  void clipWorld();
  // move the boundingBox accordingly to the movement
  void updateBoundingBox();

  // fire the Guns!
  void shootPrimary();
  void shootSecondary();
  void shootSpecial();
  // switch to the next available weapon
  //  void switchPrimary();
  void switchSecondary();
  void switchSpecials();
  
  // use the active special, if it should be used _once_ when keyDown-Event occurs
  void specialKeyDown();
  // switch special, if the activespecial is out of ammo
  void specialKeyUp();
  
  void drawRacer( SdlCompat_AcceleratedSurface *screen );
  void drawShadow( SdlCompat_AcceleratedSurface *screen );
  void drawStats( SdlCompat_AcceleratedSurface *screen );

  // collision system
  // return if the line between the two points collides with the boundingBox
  bool collidesWith( const Vector2D &shotPosOld, const Vector2D &shotPosNew );
  // return if the racers boundingBox overlaps with box
  bool collidesWith( BoundingBox *box );
  // return if the racers boundingBox overlaps with circle
  bool collidesWith( const Circle &circle );
  // returns if a inner circle around the racer overlaps with circle
  bool collidesWithAsCircle( const Circle &circle );
  // returns if a inner circle around the racer overlaps with circle
  bool collidesWithAsCircle( BoundingBox *box );
  // returns the boundingBox of the racer
  BoundingBox *getBoundingBox();

  // the racer got hit -> do the damage according to the shotType
  void doDamage( ShotTypes shotType );
  // recharge the shield
  void rechargeShield( int dT );
  // the racer was hit
  void receiveDamage( float amount );

  bool isDead();
  

  void receivePoints( float amount );
  
  // picks up items, if flown over
  void pickUpItems();

  // returns the number of laps completed by the racer
  int getPoints() { return (int)(points + 0.5); }
};

#endif

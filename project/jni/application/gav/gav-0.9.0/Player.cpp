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

#include "Player.h"
#include "ControlsArray.h"
#include <math.h>
#include "Team.h"

#define SPEED_FACT_CONST (5)

int Player::maxX() { return _team->xmax(); }
int Player::minX() { return _team->xmin(); }
int Player::minY() { return _team->ymin(); }

int Player::speedX()  {
  if ( (_speedX > 0) &&
       (_x < (_team->xmax() - _frames->width())) )
    return(_speedX);
  if ( (_speedX < 0) &&
       (_x > _team->xmin()) )
    return(_speedX);
  return(0);
}

bool Player::setState(pl_state_t st, bool force) {

  if ((_state == st) && !force) return false;
  
  _state = st;
  
  switch (st) {
  case  PL_STATE_JUMP:
    _currFrameB = configuration.playerFrameConf.playerJmpB - 1;
    _currFrameE = configuration.playerFrameConf.playerJmpE - 1;
    if ( (_currFrameB == _currFrameE) ||
	 (configuration.playerFrameConf.playerJmpP == 0) )
      _currStateFrameDelay = 0;
    else
      _currStateFrameDelay = configuration.playerFrameConf.playerJmpP/
	( _currFrameE - _currFrameB);
    break;
  case PL_STATE_STILL:
    _currFrameB = configuration.playerFrameConf.playerStillB - 1;
    _currFrameE = configuration.playerFrameConf.playerStillE - 1;
    if ( (_currFrameB == _currFrameE) ||
	 (configuration.playerFrameConf.playerStillP == 0) )
      _currStateFrameDelay = 0;
    else
      _currStateFrameDelay = configuration.playerFrameConf.playerStillP/
	( _currFrameE - _currFrameB);
    break;
  case PL_STATE_WALK:
    _currFrameB = configuration.playerFrameConf.playerRunB - 1;
    _currFrameE = configuration.playerFrameConf.playerRunE - 1;
    if ( (_currFrameB == _currFrameE) ||
	 (configuration.playerFrameConf.playerRunP == 0) )
      _currStateFrameDelay = 0;
    else
      _currStateFrameDelay = configuration.playerFrameConf.playerRunP/
	( _currFrameE - _currFrameB);
    break;
  }

  return true;
}

void Player::updateFrame(int ticks, bool changed) {
  _overallPassed += ticks;

  if ( changed ) {
    _overallPassed = 0;
    _frameIdx = _currFrameB;
  } else if ( _currStateFrameDelay &&
	      (_overallPassed > _currStateFrameDelay) ) {
    // update _frameIdx
    if ( ++_frameIdx > _currFrameE )
      _frameIdx = _currFrameB;
    _overallPassed = 0;
  }

}

/* Invoked by the network client in order to draw the proper
   animation frame */
void Player::updateClient(int ticks, pl_state_t st) {
  updateFrame(ticks, setState(st));
}

void Player::update(int ticks, ControlsArray *ca) {
  triple_t input = ca->getCommands(_plId);
  
  int dx = 0;
  bool firstTime = (_overallPassed == 0);

  if ( input.left ) 
    dx--;

  if ( input.right )
    dx++;

  _speedX = dx?(dx * _speed):0;

  _displx += (float) dx * (_speed * ticks / 1000.0); 

  if ( fabs(_displx) >= 1.0 ) {
    _x += (int) _displx;
    _displx -= (int) _displx;
  }

  if ( _x > (_team->xmax() - _frames->width()) )
    _x = (_team->xmax() - _frames->width());
  else if ( _x < _team->xmin() )
    _x = _team->xmin();

  if ( _y == GROUND_LEVEL() && input.jump ) {
    _speedY = -(configuration.SPEEDY);
  }

  if ( _y > GROUND_LEVEL() ) {
    _y = GROUND_LEVEL();
    _speedY = 0;
  }

  _disply = (float) (_speedY * SPEED_FACT_CONST * ((float) ticks / 1000.0));
  
  if ( fabs(_disply) >= 1.0 ) {
    _y += (int) _disply;
    _disply -= (int) _disply;
  }

  if ( _y < GROUND_LEVEL() )
    _speedY +=
      (float) (configuration.SPEEDY * SPEED_FACT_CONST * ticks) / 1000.0;
  
  int _oldState = _state;
  /* detect state changes */
  if ( _y < GROUND_LEVEL() ) {
    setState(PL_STATE_JUMP); // jumping
  } else if (firstTime || (!dx)) { // player still
    setState(PL_STATE_STILL, firstTime);
  } else if ( dx ) { // player running
    setState(PL_STATE_WALK);
  }

  updateFrame(ticks, (_oldState != _state));
}

void Player::draw(SDL_Surface * screen) {
  SDL_Rect rect;
  rect.x = _x;
  rect.y = _y;

  _frames->blit(_frameIdx, screen, &rect);
}

bool
Player::collidesWith(FrameSeq *fs, int idx, SDL_Rect *rect)
{
  SDL_Rect myRect;
  myRect.x = _x;
  myRect.y = _y;

  return(_frames->collidesWith(fs, _state, idx, &myRect, rect));
}

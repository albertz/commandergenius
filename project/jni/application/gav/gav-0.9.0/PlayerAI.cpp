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

#include "PlayerAI.h"
#include "Ball.h"

#define JUMP_LIMIT       ( (int) (configuration.SCREEN_HEIGHT / 1.82) )
#define JUMPING_TIME     ( 150 )

#define NEWPL_SIDE (-1)

triple_t PlayerAI::planAction() {
  int jmp = 0;
  //int i;
  int side = (team())->side();
  int nplrs = (team())->nplayers();
  int px, bx, fs, mp, bsx, net, wall, xt, xs; /* Normalized values */
  int mypos, minslot, maxslot;
  float slotsize;
  int move, pw;
  triple_t ret;
  int ttime;

  net = fs = abs(maxX() - minX());
  wall = 0;
  mp = fs/2;

  /* Normalize player x position, ball x position and ball x speed:
     Reasoning as it plays on the left, net=fs, wall=0 */
  px = (side>0)?(maxX()-x()-width()/2):
      (x()+width()/2-minX()); 
  pw = width();
  bx = (side>0)?(maxX()-(_b->radius()+_b->x())):
      (_b->radius()+_b->x()-minX()); 
  bsx = -side*_b->spdx();
  xs = bx;

  
/*  if ( (_y > _highestpoint) ){
      printf("_y: %d, hp: %d\n",_y, _highestpoint);
      _highestpoint = _y;
      }*/
  if ( !_b->gravity() ) {
	  xt = (side>0)?(maxX()-(_b->radius()/2+_b->x())):
	      (_b->radius()/2+_b->x()-minX()); 
  } else { 
  //xt = bx; /* The old one 
      int exp_y = _b->y();
      int exp_x = _b->x();
      int sx    = _b->spdx();
      int sy    = _b->spdy();
//	  printf("I(%d,%d) -> (%d,%d)\n",exp_x,exp_y,sx,sy);
      
      ttime=0;
      while (exp_y < 240) {
	  int passed=20;
	  int px = exp_x;
	  int py = exp_y; 
	  
	  ttime += passed;
	  exp_x += (int)(sx * ((float) passed/1000.0));
	  exp_y -= (int)(sy * ((float) passed/1000.0));
	  sy = (int)(sy-((float) passed*_b->gravity()/1000.0));
	  if ( exp_y < configuration.CEILING ) {
	      exp_y = configuration.CEILING;
	      sy = - (int ) (sy*ELASTIC_SMOOTH);
	  }
	  if ( exp_x < configuration.LEFT_WALL ) {
	      exp_x = configuration.LEFT_WALL;
	      sx = - (int ) (sx*ELASTIC_SMOOTH);
	  }
	  if ( exp_x > configuration.RIGHT_WALL - (_b->radius()*2) ) {
	      exp_x = configuration.RIGHT_WALL - (_b->radius()*2);
	      sx = - (int ) (sx*ELASTIC_SMOOTH);
	  }
	  int minx = (exp_x < px)?exp_x:px;
	  
	  if ( (minx > (configuration.NET_X-_b->radius()*2)) && 
	       ( minx <  configuration.NET_X ) ) {
	      /* The ball crossed the net region */
	      if ( ( exp_y > configuration.NET_Y ) ||
		   ( py > configuration.NET_Y ) ) {
		  /* Probably the ball will hit the net,
		     consider a full collision */
		  
		  exp_x =  (minx == exp_x)?configuration.NET_X-_b->radius()*2:configuration.NET_X;
		  sx = - (int ) (sx*ELASTIC_SMOOTH);		      
		  if (!sx) sx = side*20;/*SMALL_SPD_X*/
	      }
	  }
//	      printf("+(%d,%d) -> (%d,%d)\n", exp_x, exp_y, sx, sy);
      }
      xt = exp_x+_b->radius();
      xt = (side>0)?(maxX()-xt):(xt-minX()); 
//	  printf("Expected X: %d (bsx: %d, bsy: %d, bx: %d, by: %d (radius: %d, rw:%d, hp: %d)\n",xt, _b->spdx(), _b->spdy(), _b->x(), _b->y(), _b->radius(), RIGHT_WALL(2*_b->radius()),_highestpoint);
//	  printf("HP: %d\n"u, _highestpoint);
  }
//Expected X: 80061 (bsx: -624, bsy: 679, bx: 455, by: 241 (radius: 25)
  slotsize = net/nplrs;
  
  if (nplrs > 1) {
      std::vector<Player *> plv = (team())->players();
      /* Look for my id */
      std::vector<Player *>::iterator it; 
      int myidx = orderInField(), i;

      /* If nobody set the Order In Field I will do for all the team */
      if (orderInField() < 0) {
	  for ( it = plv.begin(), i=0;
		it != plv.end();
		it++,i++ ) 
	      (*it)->setOIF(i);	      
	  
	  myidx = orderInField();

      }
      mypos = (int)(slotsize*(myidx+0.5));
      Player *closerp;
      int minxsearching = net, minopx, opx, closer;
      for ( it = plv.begin();
	    it != plv.end(); 
	    it++ ) {
	  opx = (side>0)?((*it)->maxX()-(*it)->x()-(*it)->width()/2):
	      ((*it)->x()+(*it)->width()/2-(*it)->minX()); 	  
	  if ( ( (*it)->id() != id() ) &&
	       ( abs(opx-mypos) < minxsearching ) ){
	      closer=(*it)->id();
	      closerp=(*it);
	      minxsearching=abs(opx-mypos);
	      minopx=opx;
	  }
			     
      }
      
      /* Someone is inside my slot over the 15% the slot size */
      if (minxsearching < (slotsize*0.35)) {
	  myidx = closerp->orderInField();
	  closerp->setOIF(orderInField());
	  setOIF(myidx);
      }
      mypos = (int )(slotsize*(myidx+0.5));
      minslot = (int )(slotsize*myidx);
      maxslot = (int )(slotsize*(myidx+1));
  } else {
      minslot = wall;
      mypos = mp-50; // behind the middle point
      maxslot = net;
  }

  /* My rest position has been chosen, and the slot determined */
  
  //  int hd = 3*_b->radius()/4;
  int hd = (_b->radius()+width())/2;
  int minhd = 0;
  if ( !_b->gravity() ) { 
      if (hd/2 >= 12) 
	  minhd = rand()%(hd/2-10)+10; 
      hd = rand()%(hd-minhd)+minhd+1;
  }
  /* When I consider closer, is the closer to the falling point */
  /* I care whether I'm the closer only when it's "service time" */
  int closest = 1;
  if ( !_b->gravity() ) {
      int opx;
      std::vector<Player *> plv = (team())->players();
      /* Look for my id */
      std::vector<Player *>::iterator it; 
      for ( it = plv.begin();
	    it != plv.end(); 
	    it++ ) {
	  opx = (side>0)?((*it)->maxX()-(*it)->x()-(*it)->width()/2):
	      ((*it)->x()+(*it)->width()/2-(*it)->minX()); 	  
	  if ( abs(px-xt) > abs(opx-xt) )
	      closest = 0;
      }      
  } 

//      printf("%d\n", _b->spdy());
//      if (abs(bsx) < 160) hd += 7;
  if ( _b->gravity() ) { 
      if ( ( (abs(px - bx)) <= hd ) && // is the ball close?
	   //(abs(_b->spdy()) > 10) && // is the ball going down too slow?
	   //( abs(px-bx) > 5 ) &&     // am I to close to the ball center?
	   // Can I reach the ball jumping, or am I missing the ball? 
	   (_b->y() > ((_b->spdy() > 20)?JUMP_LIMIT:(JUMP_LIMIT+20)))
	  ) {
	  jmp = 1;
      }
  } else { // I serve only forward
      jmp = ( (minhd < (bx - px) && ((bx - px) <= hd) ) );
  }
//  printf("hd: %d - %d (%d - %d)\n",hd, abs(bx-px), _b->spdy(), _b->y()); 

  // if I'm the closest player I serve (when it's "service time").
  // if the ball is (too) outside my slot I do not go for it.
  double concern = (nplrs>1)?(slotsize*1.05):slotsize;
  if ( (abs(xt-mypos) < concern) && closest) { 
      if ( _b->gravity() ) { 
	  move = xt-hd-px;
      } else { // I serve only forward
	  move = bx-hd-px;
      }
      if (  ( ttime < JUMPING_TIME ) ){
	  jmp = 1;
	  move = (bx-px);
      }
  } else {
      move = mypos-px;
  }
  
  move=-side*(move);

  ret.left = ret.right = ret.jump = 0;
  if ( abs(move) > 2) {
      if ( move < 0 )
	  ret.left = 1;
      if ( move > 0 )
	  ret.right = 1;
  }
  if ( jmp )
      ret.jump = 1;

  return ret;
}


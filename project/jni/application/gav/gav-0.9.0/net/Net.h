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

#ifndef __NET_H__
#define __NET_H__

#ifndef NONET

#include <SDL_net.h>
#include <vector>
#include "Configuration.h"
#include "Ball.h"
#include "Team.h"
#include "automa/StateWithInput.h"

#define PLAYER_FOR_TEAM_IN_NET_GAME 2
#define SERVER_PORT 7145

/* The ID of a client is a char with the higher order 2 bits indicating
   the team, the left 6 bits say the player number. When a client wants to
   register itself, it sends a register request with id equal to 
   NET_TEAM_LEFT or NET_TEAM_RIGHT. The server fills the others 6 bits 
   and sends the id back to the client.
*/
#define NET_TEAM_LEFT  0x80    // 10xxxxxx
#define NET_TEAM_RIGHT 0x40    // 01xxxxxx

typedef struct {
  Uint16 x;
  Uint16 y;
  Uint16 frame;
} net_object_snapshot_t;

typedef struct {
  //unsigned int timestamp;
  net_object_snapshot_t teaml[PLAYER_FOR_TEAM_IN_NET_GAME];
  net_object_snapshot_t teamr[PLAYER_FOR_TEAM_IN_NET_GAME];
  net_object_snapshot_t ball;
  unsigned char scorel;
  unsigned char scorer;
} net_game_snapshot_t;

typedef struct {
  //unsigned int timestamp;
  unsigned char id;       // the client ID
  unsigned char command;
} net_command_t;

typedef struct {
  unsigned char id;
  unsigned char nplayers_l;
  unsigned char nplayers_r;
  unsigned char bgBig;    // 0 or 1
  unsigned char winning_score;
} net_register_t;

class Net : public StateWithInput{
protected:
  UDPsocket mySock;
  UDPpacket * packetCmd;
  UDPpacket * packetSnap;
  UDPpacket * packetRegister;

public:
  Net() {
    packetSnap = SDLNet_AllocPacket(sizeof(net_game_snapshot_t));
    packetSnap->len = packetSnap->maxlen;
    packetCmd = SDLNet_AllocPacket(sizeof(net_command_t));
    packetCmd->len = packetCmd->maxlen;
    packetRegister = SDLNet_AllocPacket(sizeof(net_register_t));
    packetRegister->len = packetRegister->maxlen;
  }

  ~Net() {
    SDLNet_FreePacket(packetSnap);
    SDLNet_FreePacket(packetCmd);
  }

};

#endif // NONET

#endif

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

#ifndef NONET

#include "NetServer.h"
#include "Player.h"

using namespace std;

int NetServer::StartServer(int port) {
  /* open the socket */
  mySock = SDLNet_UDP_Open((Uint16)port);
  if(!mySock) {
    fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
    return -1;
  }

  return 0;
}

int NetServer::ComputePlayerID(char id) {
  unsigned char tm ,pl;
  tm = (id & NET_TEAM_LEFT)?0:1;
  pl = id & 0x3F;
  return  ((pl << 1) | tm);
}

int NetServer::WaitClients(InputState * is, int nclients) {
  IPaddress * ipa;
  //char nleft = 0;
  //char nright = 0;
  unsigned char * id;
  int i;
  bool inserted = false;
  char pl;

  _nclients = nclients;

  while ( !inserted ) { // (nright + nleft) != nclients) {
    inserted = false;
    if (SDLNet_UDP_Recv(mySock, packetRegister) != 0) {
      ipa = (IPaddress*)malloc(sizeof(IPaddress));
      memcpy(ipa, &(packetRegister->address), sizeof(IPaddress));
      id = &(((net_register_t*)(packetRegister->data))->id);
      if (*id & NET_TEAM_LEFT) {
	for (i = 0; (i < configuration.left_nplayers) && !inserted; i++)
	  if ((configuration.left_players[i] == PLAYER_COMPUTER) &&
	      !_players[2*i]) {
	    inserted = true;
	    pl = (char)i;
	  }
	if (inserted) {
	  *id |= pl;
	  //nleft++;
	} else
	  continue;
      } else if (*id & NET_TEAM_RIGHT) {
	for (i = 0; (i < configuration.right_nplayers) && !inserted; i++)
	  if ((configuration.right_players[i] == PLAYER_COMPUTER) &&
	      !_players[2*i+1]) {
	    inserted = true;
	    pl = (char)i;
	  }
	if (inserted) {
	  *id |= pl;
	  //nright++;
	} else
	  continue;
      }
      _players[ComputePlayerID(*id)] = 1;
      clientIP.push_back(ipa);
      /* send the ID back to client */
      ((net_register_t*)(packetRegister->data))->nplayers_l = 
	configuration.left_nplayers;
      ((net_register_t*)(packetRegister->data))->nplayers_r = 
	configuration.right_nplayers;
      ((net_register_t*)(packetRegister->data))->bgBig =
	configuration.bgBig;
      ((net_register_t*)(packetRegister->data))->winning_score =
	configuration.winning_score;
		     
      SDLNet_UDP_Send(mySock, -1, packetRegister);
    } else {
      if (getKeyPressed(is, false) == SDLK_ESCAPE) {
	return -1;
      }
      SDL_Delay(500);
    }
  }

  return (nclients-1);
}

//Uint16 NetServer::NormalizeCurrentFrame(

int NetServer::SendSnapshot(Team *tleft, Team *tright, Ball * ball) {
  unsigned int i;
  net_game_snapshot_t * snap = (net_game_snapshot_t *)(packetSnap->data);
  std::vector<Player *> plv;

  snap->scorel = tleft->getScore();
  snap->scorer = tright->getScore();
  /* fill the left team informations */
  plv = tleft->players();
  for (i = 0; i < plv.size(); i++) {
    SDLNet_Write16(plv[i]->x(), &((snap->teaml)[i].x));
    SDLNet_Write16(plv[i]->y(), &((snap->teaml)[i].y));
    SDLNet_Write16(plv[i]->state(), &((snap->teaml)[i].frame));
  }
  /* fill the right team informations */
  plv = tright->players();
  for (i = 0; i < plv.size(); i++) {
    SDLNet_Write16(plv[i]->x(), &((snap->teamr)[i].x));
    SDLNet_Write16(plv[i]->y(), &((snap->teamr)[i].y));
    SDLNet_Write16(plv[i]->state(), &((snap->teamr)[i].frame));
  }
  /* fill the ball informations */
  SDLNet_Write16(ball->x(), &((snap->ball).x));
  SDLNet_Write16(ball->y(), &((snap->ball).y));
  // ball has just one state
  //SDLNet_Write16(ball->frame(), &((snap->ball).frame));

  /* send the snapshot to all clients */
  for (i = 0; i < clientIP.size(); i++) {
    packetSnap->address = *(clientIP[i]);
    SDLNet_UDP_Send(mySock, -1, packetSnap);
  }
  return 0;
}

int NetServer::ReceiveCommand(int * player, char * cmd) {
  if (SDLNet_UDP_Recv(mySock, packetCmd) != 0) {
    net_command_t * c = (net_command_t*)(packetCmd->data);
    *player = ComputePlayerID(c->id);
    *cmd = c->command;
    return 0;
  }

  return -1;
}

int NetServer::isRemote(int pl) {
  return _players[pl];
}


#endif


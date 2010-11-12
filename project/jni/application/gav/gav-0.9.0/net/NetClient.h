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

#ifndef __NETCLIENT_H__
#define __NETCLIENT_H__

#ifndef NONET

#include "Net.h"

class NetClient: public Net {
  IPaddress ipaddress;
  int channel;   // the channel assigned to client
  char _id;      // if I'm a client I've an id
  char _nplayers_l;
  char _nplayers_r;

private:
  int receiveData(Uint16 *data);

public:
  NetClient() {
  }

  ~NetClient() {
    SDLNet_UDP_Unbind(mySock, channel);
    SDLNet_UDP_Close(mySock);
  }

  /* client methods */
  int ConnectToServer(InputState * is, int * pl, int * pr, char team, 
		      const char * hostname, int port = SERVER_PORT);
  int WaitGameStart();
  int ReceiveSnapshot(Team *tleft, Team *tright, Ball * ball, int passed);
  int SendCommand(char cmd);
  inline char id() { return _id; }

};

#endif // NONET

#endif

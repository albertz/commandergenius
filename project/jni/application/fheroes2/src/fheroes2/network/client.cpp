/***************************************************************************
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   Part of the Free Heroes2 Engine:                                      *
 *   http://sourceforge.net/projects/fheroes2                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifdef WITH_NET

#include "settings.h"
#include "localevent.h"
#include "client.h"

FH2Client::FH2Client()
{
}

bool FH2Client::IsConnected(void) const
{
    return Modes(ST_CONNECT) && sd;
}

bool FH2Client::Wait(QueueMessage & packet, u16 id)
{
    while(LocalEvent::Get().HandleEvents())
    {
        if(Ready())
        {
            if(!Network::RecvMessage(*this, packet))
            {
                Close();
		DEBUG(DBG_NETWORK , DBG_TRACE, "FH2Client::Wait: error");
                return false;
            }
            if(id == packet.GetID()) break;
        }
	DELAY(10);
    }
    return true;
}

bool FH2Client::Send(QueueMessage & packet)
{
    if(!Network::SendMessage(*this, packet))
    {
	packet.Dump();
        Close();
	DEBUG(DBG_NETWORK , DBG_TRACE, "FH2Client::Send: error");
        return false;
    }
    return true;
}

bool FH2Client::Recv(QueueMessage & packet)
{
    if(!Network::RecvMessage(*this, packet))
    {
	packet.Dump();
        Close();
	DEBUG(DBG_NETWORK , DBG_TRACE, "FH2Client::Recv: error");
        return false;
    }
    return true;
}

#endif

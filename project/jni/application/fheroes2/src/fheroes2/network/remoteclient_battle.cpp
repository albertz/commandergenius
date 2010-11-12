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

#include <algorithm>
#include <functional>
#include "world.h"
#include "settings.h"
#include "server.h"
#include "client.h"
#include "kingdom.h"
#include "battle2.h"
#include "battle_stats.h"
#include "battle_cell.h"
#include "remoteclient.h"

bool FH2RemoteClient::SendBattleAction(u8 color, QueueMessage & msg)
{
    FH2RemoteClient* remote = FH2Server::Get().GetRemoteClient(color);

    if(remote)
    {
	DEBUG(DBG_NETWORK, DBG_INFO, "FH2RemoteClient::SendBattleAction: " << Network::GetMsgString(msg.GetID()));
	return remote->Send(msg);
    }
    else
	DEBUG(DBG_NETWORK, DBG_WARN, "FH2RemoteClient::SendBattleAction: " << "incorrect param: " << static_cast<int>(color));
    return false;
}

bool FH2RemoteClient::SendBattleEarthQuakeSpell(u8 color, const std::vector<u8> & targets)
{
    FH2RemoteClient* remote = FH2Server::Get().GetRemoteClient(color);

    if(remote)
    {
	remote->packet.Reset();
	remote->packet.SetID(MSG_BATTLE_CAST);
	remote->packet.Push(static_cast<u8>(Spell::EARTHQUAKE));
	remote->packet.Push(static_cast<u32>(targets.size()));

	std::vector<u8>::const_iterator it1 = targets.begin();
	std::vector<u8>::const_iterator it2 = targets.end();

	for(; it1 != it2; ++it1)
	    remote->packet.Push(*it1);

	DEBUG(DBG_NETWORK, DBG_INFO, "FH2RemoteClient::SendBattleEarthQquakeSpell: ");
	return remote->Send(remote->packet);
    }
    else
	DEBUG(DBG_NETWORK, DBG_WARN, "FH2RemoteClient::SendBattleEarthQquakeSpell: " << "incorrect param");
    return false;
}

bool FH2RemoteClient::SendBattleSpell(u8 color, u8 spell, u8 hero_color, const std::vector<Battle2::TargetInfo> & targets)
{
    FH2RemoteClient* remote = FH2Server::Get().GetRemoteClient(color);

    if(remote)
    {
	remote->packet.Reset();
	remote->packet.SetID(MSG_BATTLE_CAST);
	remote->packet.Push(spell);
	remote->packet.Push(hero_color);
	remote->packet.Push(static_cast<u32>(targets.size()));

	std::vector<Battle2::TargetInfo>::const_iterator it1 = targets.begin();
	std::vector<Battle2::TargetInfo>::const_iterator it2 = targets.end();

	for(; it1 != it2; ++it1)
	{
	    const u16 id = (*it1).defender ? (*it1).defender->GetID() : 0;
	    remote->packet.Push(id);
	    remote->packet.Push((*it1).damage);
	    remote->packet.Push((*it1).killed);
	}
	DEBUG(DBG_NETWORK, DBG_INFO, "FH2RemoteClient::SendBattleSpell: ");
	return remote->Send(remote->packet);
    }
    else
	DEBUG(DBG_NETWORK, DBG_WARN, "FH2RemoteClient::SendBattleSpell: " << "incorrect param");
    return false;
}

bool FH2RemoteClient::SendBattleTeleportSpell(u8 color, u16 src, u16 dst)
{
    FH2RemoteClient* remote = FH2Server::Get().GetRemoteClient(color);

    if(remote)
    {
	remote->packet.Reset();
	remote->packet.SetID(MSG_BATTLE_CAST);
	remote->packet.Push(static_cast<u8>(Spell::TELEPORT));
	remote->packet.Push(src);
	remote->packet.Push(dst);
	DEBUG(DBG_NETWORK, DBG_INFO, "FH2RemoteClient::SendBattleTeleportSpell: ");
	return remote->Send(remote->packet);
    }
    else
	DEBUG(DBG_NETWORK, DBG_WARN, "FH2RemoteClient::SendBattleTeleportSpell: " << "incorrect param");
    return false;
}

bool FH2RemoteClient::SendBattleResult(u8 color, const Battle2::Result & result)
{
    FH2RemoteClient* remote = FH2Server::Get().GetRemoteClient(color);

    if(remote)
    {
	remote->packet.Reset();
	remote->packet.SetID(MSG_BATTLE_RESULT);
	remote->packet.Push(result.army1);
	remote->packet.Push(result.army2);
	remote->packet.Push(result.exp1);
	remote->packet.Push(result.exp2);
	DEBUG(DBG_NETWORK, DBG_INFO, "FH2RemoteClient::SendBattleResult: ");
	return remote->Send(remote->packet);
    }
    else
	DEBUG(DBG_NETWORK, DBG_WARN, "FH2RemoteClient::SendBattleResult: " << "incorrect param");
    return false;
}

bool FH2RemoteClient::SendBattleBoard(u8 color, const Battle2::Arena & arena)
{
    FH2RemoteClient* remote = FH2Server::Get().GetRemoteClient(color);

    if(remote)
    {
	remote->packet.Reset();
	remote->packet.SetID(MSG_BATTLE_BOARD);

	arena.PackBoard(remote->packet);

	DEBUG(DBG_NETWORK, DBG_INFO, "FH2RemoteClient::SendBattleBoard: ");
	return remote->Send(remote->packet);
    }
    else
	DEBUG(DBG_NETWORK, DBG_WARN, "FH2RemoteClient::SendBattleBoard: " << "incorrect param");
    return false;
}

bool FH2RemoteClient::SendBattleAttack(u8 color, u16 id, const std::vector<Battle2::TargetInfo> & targets)
{
    FH2RemoteClient* remote = FH2Server::Get().GetRemoteClient(color);

    if(remote)
    {
	remote->packet.Reset();
	remote->packet.SetID(MSG_BATTLE_ATTACK);
	remote->packet.Push(id);
	remote->packet.Push(static_cast<u32>(targets.size()));

	std::vector<Battle2::TargetInfo>::const_iterator it1 = targets.begin();
	std::vector<Battle2::TargetInfo>::const_iterator it2 = targets.end();

	for(; it1 != it2; ++it1)
	{
	    const u16 id = (*it1).defender ? (*it1).defender->GetID() : 0;
	    remote->packet.Push(id);
	    remote->packet.Push((*it1).damage);
	    remote->packet.Push((*it1).killed);
	}
	DEBUG(DBG_NETWORK, DBG_INFO, "FH2RemoteClient::SendBattleAttack: ");
	return remote->Send(remote->packet);
    }
    else
	DEBUG(DBG_NETWORK, DBG_WARN, "FH2RemoteClient::SendBattleAttack: " << "incorrect param");
    return false;
}

bool FH2RemoteClient::RecvBattleHumanTurn(const Battle2::Stats & b, const Battle2::Arena & arena, Battle2::Actions & a)
{
    bool exit = false;

    packet.Reset();
    packet.SetID(MSG_BATTLE_TURN);
    packet.Push(b.GetID());

    // send battle turn
    DEBUG(DBG_NETWORK, DBG_INFO, "FH2RemoteClient::BattleHumanTurn: id: 0x" << b.GetID() << ", send battle turn");
    if(!Send(packet)) return false;

    DEBUG(DBG_NETWORK, DBG_INFO, "FH2RemoteClient::BattleHumanTurn: id: 0x" << b.GetID() << ", send battle board");
    if(!SendBattleBoard(player_color, arena)) return false;

    while(!exit)
    {
        if(Modes(ST_SHUTDOWN)) return false;

	if(Ready())
	{
	    if(!Recv(packet)) return false;
            DEBUG(DBG_NETWORK , DBG_INFO, "FH2RemoteClient::RecvHumanTurn: recv: " << Network::GetMsgString(packet.GetID()));

	    // check broadcast
	    MsgBroadcast();

    	    // msg processing
    	    switch(Network::GetMsg(packet.GetID()))
    	    {
		case MSG_BATTLE_CAST:
		case MSG_BATTLE_SKIP:
		case MSG_BATTLE_END_TURN:
		    exit = 1;
		    a.push_back(packet);
		    break;

		case MSG_BATTLE_MOVE:
		case MSG_BATTLE_ATTACK:
		case MSG_BATTLE_DEFENSE:
		case MSG_BATTLE_DAMAGE:
		case MSG_BATTLE_MORALE:
		case MSG_BATTLE_LUCK:
		case MSG_BATTLE_CATAPULT:
		case MSG_BATTLE_TOWER:
		case MSG_BATTLE_RETREAT:
		case MSG_BATTLE_SURRENDER:
		    a.push_back(packet);
		    break;

    		default: break;
    	    }
	}

        DELAY(100);
    }
    return true;
}

#endif

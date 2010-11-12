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

#include <sstream>
#include <algorithm>
#include <functional>
#include "world.h"
#include "settings.h"
#include "server.h"
#include "client.h"
#include "kingdom.h"
#include "battle2.h"
#include "battle_stats.h"
#include "remoteclient.h"
#include "zzlib.h"

int FH2RemoteClient::callbackCreateThread(void *data)
{
    return data ? reinterpret_cast<FH2RemoteClient *>(data)->Main() : -1;
}

FH2RemoteClient::FH2RemoteClient()
{
}

void FH2RemoteClient::RunThread(void)
{
    if(thread.IsRun()) thread.Wait();
    thread.Create(callbackCreateThread, this);
}

void FH2RemoteClient::ShutdownThread(void)
{
    SetModes(ST_SHUTDOWN);
    DELAY(100);

    if(thread.IsRun()) thread.Kill();

    if(sd)
    {
        packet.Reset();
	packet.SetID(MSG_SHUTDOWN);
	Network::SendMessage(*this, packet);
        CloseConnection();
    }
    SetModes(0);
}

void FH2RemoteClient::CloseConnection(void)
{
    Close();

    modes = 0;
    player_id = 0;
    player_color = 0;
}

int FH2RemoteClient::Main(void)
{
    if(ConnectionChat())
    {
	FH2Server & server = FH2Server::Get();

	server.Lock();
	server.SetStartGame();
	server.Unlock();

	StartGame();
    }

    CloseConnection();

    return 1;
}

bool FH2RemoteClient::ConnectionChat(void)
{
    Settings & conf = Settings::Get();

    player_color = 0;
    player_race = Race::RAND;
    player_name.clear();

    // wait thread id
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2RemoteClient::" << "ConnectionChat: " << "wait start thread");
    while(0 == thread.GetID()){ DELAY(10); };
    player_id = thread.GetID();

    SetModes(ST_CONNECT);

    // send banner
    std::ostringstream banner;
    banner << "Free Heroes II Server, version: " << static_cast<int>(conf.MajorVersion()) << "." << static_cast<int>(conf.MinorVersion()) << std::endl;

    packet.Reset();
    packet.SetID(MSG_READY);
    packet.Push(banner.str());

    // send ready
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2RemoteClient::" << "ConnectionChat: " << "id: 0x" << std::hex << player_id << ", send ready");
    if(!Send(packet)) return false;

    // recv hello
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2RemoteClient::" << "ConnectionChat: " << "id: 0x" << std::hex << player_id << ", wait hello");
    if(!Wait(packet, MSG_HELLO)) return false;

    packet.Pop(player_name);
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2RemoteClient::" << "ConnectionChat: " << "id: 0x" << std::hex << player_id << ", connected " << " player: " << player_name << ", host 0x" << std::hex << Host() << ":0x" << Port());

    FH2Server & server = FH2Server::Get();

    // check color
    server.Lock();
    player_color = Color::GetFirst(conf.CurrentFileInfo().human_colors & (~conf.PlayersColors()));
    server.Unlock();
    if(0 == player_color)
    {
	DEBUG(DBG_NETWORK , DBG_INFO, "FH2RemoteClient::" << "ConnectionChat: " << "id: 0x" << std::hex << player_id << ", player_color = 0, logout");
	return false;
    }

    // send hello, modes, id, color, cur maps
    packet.Reset();
    packet.SetID(MSG_HELLO);
    packet.Push(modes);
    packet.Push(player_id);
    packet.Push(player_color);
    // added cur maps info
    Network::PacketPushMapsFileInfo(packet, conf.CurrentFileInfo());
    // send
    DEBUG(DBG_NETWORK, DBG_INFO, "FH2RemoteClient::" << "ConnectionChat: " << "id: 0x" << std::hex << player_id << ", send hello");
    if(!Send(packet)) return false;

    // update colors
    server.Lock();
    conf.SetPlayersColors(server.GetPlayersColors());
    server.Unlock();

    // send to all: update players
    SendUpdatePlayers(0);

    DEBUG(DBG_NETWORK, DBG_INFO, "FH2RemoteClient::" << "ConnectionChat: " << (Modes(ST_ADMIN) ? "admin" : "client") << " mode");
    if(Modes(ST_ADMIN)) SetModes(ST_ALLOWPLAYERS);
    
    DEBUG(DBG_NETWORK, DBG_INFO, "FH2RemoteClient::" << "ConnectionChat: " << "start queue");

    while(1)
    {
        if(Modes(ST_SHUTDOWN)) return false;

	if(Ready())
	{
	    if(!Recv(packet)) return false;
            DEBUG(DBG_NETWORK, DBG_INFO, "FH2RemoteClient::" << "ConnectionChat: " << "recv: " << Network::GetMsgString(packet.GetID()));

	    // check broadcast
	    MsgBroadcast();

    	    // msg processing
    	    switch(Network::GetMsg(packet.GetID()))
    	    {
    		case MSG_PING:
		    MsgPing();
		    break;

        	case MSG_LOGOUT:
		    MsgLogout();
	    	    return false;

    		case MSG_CHANGE_COLORS:
		    if(Modes(ST_ADMIN)) MsgChangeColors();
		    break;

    		case MSG_CHANGE_RACE:
		    if(Modes(ST_ADMIN)) MsgChangeRace();
		    break;

    		case MSG_SET_CURRENT_MAP:
		    {
			std::string str;
			packet.Pop(str);

			if(Modes(ST_ADMIN) &&
			    Settings::Get().LoadFileMapsMP2(str) && !SendSetCurrentMap()) return false;
		    }
		    break;

    		case MSG_GET_MAPS_LIST:
		    if(Modes(ST_ADMIN))
		    {
			if(!SendMapsInfoList()) return false;
		    }
		    else
		    {
			if(!SendAccessDenied()) return false;
		    }
		    break;

    		case MSG_START_GAME:
		    if(Modes(ST_ADMIN)) return true;
    		    break;

    		default:
    		    break;
    	    }
	}

        DELAY(100);
    }

    return false;
}

bool FH2RemoteClient::StartGame(void)
{
    //Settings & conf = Settings::Get();
    //FH2Server & server = FH2Server::Get();
    bool exit = false;

    while(!exit)
    {
        if(Modes(ST_SHUTDOWN)) return false;

	if(Ready())
	{
	    if(!Recv(packet)) return false;
            DEBUG(DBG_NETWORK , DBG_INFO, "FH2RemoteClient::" << "StartGame: " << "recv: " << Network::GetMsgString(packet.GetID()));

	    // check broadcast
	    MsgBroadcast();

    	    // msg processing
    	    switch(Network::GetMsg(packet.GetID()))
    	    {
    		case MSG_PING:
		    MsgPing();
		    break;

        	case MSG_LOGOUT:
		    MsgLogout();
	    	    return false;

		case MSG_TILES:
		    Network::UnpackTile(packet);
		    break;

		case MSG_CASTLE:
		    Network::UnpackCastle(packet);
		    break;

		case MSG_HEROES:
		    Network::UnpackHero(packet);
		    break;

		case MSG_KINGDOM:
		    Network::UnpackKingdom(packet);
		    break;

		case MSG_END_TURN:
		    // FIX FIX: check heroes and castles for current kingdom
		    ResetModes(ST_TURN);
		    break;

    		default:
    		    break;
    	    }
	}

        DELAY(100);
    }

    return true;
}

void FH2RemoteClient::MsgBroadcast(void)
{
    if(MSG_UNKNOWN != Network::GetMsg(packet.GetID()) && Network::MsgIsBroadcast(packet.GetID()))
    {
	FH2Server & server = FH2Server::Get();

	server.Lock();
	server.SendToAllClients(packet, player_id);
	server.Unlock();
    }
}

void FH2RemoteClient::MsgPing(void)
{
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2RemoteClient::" << "MsgPing:");
    packet.Reset();
    packet.SetID(MSG_PING);
    Network::SendMessage(*this, packet);
}

void FH2RemoteClient::MsgLogout(void)
{
    // send message
    packet.Reset();
    packet.SetID(MSG_MESSAGE);
    std::string str = "logout player: " + player_name;
    packet.Push(str);

    FH2Server & server = FH2Server::Get();
    Settings & conf = Settings::Get();

    server.Lock();
    server.SendToAllClients(packet, player_id);
    if(Modes(ST_ADMIN)) server.SetNewAdmin(player_id);
    server.Unlock();
    //
    conf.SetPlayersColors(server.GetPlayersColors() & (~player_color));
    if(Modes(ST_INGAME)) world.GetKingdom(player_color).SetControl(Game::AI); // FIXME: MSGLOGOUT: INGAME AND CURRENT TURN?
    // send players
    SendUpdatePlayers(player_id);
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2RemoteClient::" << "MsgLogout: " << str);
}

void FH2RemoteClient::MsgChangeRace(void)
{
    FH2Server & server = FH2Server::Get();
    Settings & conf = Settings::Get();
    u8 color, race;

    packet.Pop(color);
    packet.Pop(race);

    if(conf.AllowChangeRace(color))
    {
	conf.SetKingdomRace(color, race);
	packet.Reset();
	packet.SetID(MSG_CHANGE_RACE);
	server.Lock();
	server.ChangeClientRace(color, race);
	Network::PackRaceColors(packet);
	server.SendToAllClients(packet);
	server.Unlock();
    }
}

void FH2RemoteClient::MsgChangeColors(void)
{
    FH2Server & server = FH2Server::Get();
    Settings & conf = Settings::Get();
    u8 from, to;

    packet.Pop(from);
    packet.Pop(to);

    if(conf.AllowColors(from) && conf.AllowColors(to))
    {
	server.Lock();
	server.ChangeClientColors(from, to);
	server.Unlock();
	SendUpdatePlayers(0);
    }
}

void FH2RemoteClient::SendUpdatePlayers(u32 exclude)
{
    FH2Server & server = FH2Server::Get();
    packet.Reset();
    packet.SetID(MSG_UPDATE_PLAYERS);
    server.Lock();
    server.PushPlayersInfo(packet, exclude);
    server.SendToAllClients(packet, exclude);
    server.Unlock();
}

bool FH2RemoteClient::SendSetCurrentMap(void)
{
    FH2Server & server = FH2Server::Get();
    Settings & conf = Settings::Get();

    packet.Reset();
    packet.SetID(MSG_SET_CURRENT_MAP);
    server.Lock();
    Network::PacketPushMapsFileInfo(packet, conf.CurrentFileInfo());
    server.SendToAllClients(packet, 0);
    server.Unlock();
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2RemoteClient::" << "SendSetCurrentMap:");
    if(!Send(packet)) return false;

    // reset players
    server.Lock();
    server.ResetPlayers();
    server.Unlock();

    // send players
    SendUpdatePlayers(0);

    return true;
}

bool FH2RemoteClient::SendAccessDenied(void)
{
    packet.Reset();
    packet.SetID(MSG_ACCESS_DENIED);

    DEBUG(DBG_NETWORK , DBG_INFO, "FH2RemoteClient::" << "SendAccessDenied: ");
    return Send(packet);
}

bool FH2RemoteClient::SendMapsInfoList(void)
{
    FH2Server & server = FH2Server::Get();

    packet.Reset();
    packet.SetID(MSG_GET_MAPS_LIST);

    server.Lock();
    server.PushMapsFileInfoList(packet);
    server.Unlock();

    DEBUG(DBG_NETWORK , DBG_INFO, "FH2RemoteClient::" << "SendMapsInfoList: ");
    return Send(packet);
}

#endif

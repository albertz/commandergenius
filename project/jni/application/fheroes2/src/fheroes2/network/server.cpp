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

#include <algorithm>

#include "game.h"
#include "dialog.h"
#include "agg.h"
#include "settings.h"
#include "cursor.h"
#include "localclient.h"
#include "world.h"
#include "kingdom.h"
#include "battle2.h"
#include "server.h"

#ifdef WITH_NET

u8 SelectCountPlayers(void);

void SendPacketToAllClients(std::vector<FH2RemoteClient> & clients, const QueueMessage & msg, u32 owner)
{
    static const QueueMessage ready(MSG_READY);

    std::vector<FH2RemoteClient>::iterator it = clients.begin();
    for(; it != clients.end(); ++it) if((*it).IsConnected())
	(*it).player_id != owner ? Network::SendMessage(*it, msg) : Network::SendMessage(*it, ready);
}

FH2Server::FH2Server()
{
    AGG::Cache & cache = AGG::Cache::Get();
    if(! cache.ReadDataDir()) Error::Except("FH2Server::", " AGG data files not found.");

    if(!PrepareMapsFileInfoList(finfo_list, true) ||
       !Settings::Get().LoadFileMapsMP2(finfo_list.front().file)) DEBUG(DBG_NETWORK , DBG_WARN, "FH2Server::" << " No maps available!");
}

FH2Server::~FH2Server()
{
}

FH2Server & FH2Server::Get(void)
{
    static FH2Server fh2server;

    return fh2server;
}

int FH2Server::callbackCreateThread(void *ptr)
{
    return ptr ? reinterpret_cast<FH2Server *>(ptr)->Main() : -1;
}

bool FH2Server::Bind(u16 port)
{
    IPaddress ip;

    return Network::ResolveHost(ip, NULL, port) && Open(ip);
}

void FH2Server::Lock(void)
{
    mutex.Lock();
}

void FH2Server::Unlock(void)
{
    mutex.Unlock();
}

void FH2Server::SetExit(void)
{
    exit = true;
}

void FH2Server::SetStartGame(void)
{
    start_game = true;
}

void FH2Server::SendToAllClients(const QueueMessage & msg, u32 id)
{
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2Server::" << "SendToAllClients: " << Network::GetMsgString(msg.GetID()));
    SendPacketToAllClients(clients, msg, id);
}

void FH2Server::PushMapsFileInfoList(QueueMessage & msg) const
{
    msg.Push(static_cast<u16>(finfo_list.size()));
    MapsFileInfoList::const_iterator it1 = finfo_list.begin();
    MapsFileInfoList::const_iterator it2 = finfo_list.end();

    for(; it1 != it2; ++it1) Network::PacketPushMapsFileInfo(msg, *it1);
}

void FH2Server::ChangeClientColors(u8 from, u8 to)
{
    std::vector<FH2RemoteClient>::iterator it1, it2;

    it1 = std::find_if(clients.begin(), clients.end(), std::bind2nd(std::mem_fun_ref(&FH2RemoteClient::isColor), from));
    it2 = std::find_if(clients.begin(), clients.end(), std::bind2nd(std::mem_fun_ref(&FH2RemoteClient::isColor), to));

    if(it2 == clients.end())
    {
	(*it1).player_color = to;
	Settings::Get().SetPlayersColors(GetPlayersColors());
    }
    else
    if(it1 != clients.end())
    {
	std::swap((*it1).player_color, (*it2).player_color);
    }
}

void FH2Server::ChangeClientRace(u8 color, u8 race)
{
    std::vector<FH2RemoteClient>::iterator it;

    it = std::find_if(clients.begin(), clients.end(), std::bind2nd(std::mem_fun_ref(&FH2RemoteClient::isColor), color));
    if(it != clients.end())
	(*it).player_race = race;
}

void FH2Server::SetNewAdmin(u32 old_admin)
{
    std::vector<FH2RemoteClient>::iterator it;

    it = std::find_if(clients.begin(), clients.end(), std::bind2nd(std::mem_fun_ref(&FH2RemoteClient::isID), old_admin));
    if(it != clients.end()) (*it).ResetModes(ST_ADMIN);

    it = std::find_if(clients.begin(), clients.end(), std::not1(std::bind2nd(std::mem_fun_ref(&FH2RemoteClient::isID), old_admin)));
    if(it != clients.end()) (*it).SetModes(ST_ADMIN);
}

void FH2Server::PushPlayersInfo(QueueMessage & msg, u32 exclude) const
{
    Network::PacketPushPlayersInfo(msg, clients, exclude);
}

void FH2Server::PopMapsFileInfoList(QueueMessage & msg)
{
    Network::PacketPopMapsFileInfoList(msg, finfo_list);
}

u8 FH2Server::GetPlayersColors(void) const
{
    return Network::GetPlayersColors(clients);
}

void FH2Server::ResetPlayers(void)
{
    Settings & conf = Settings::Get();

    // reset all
    std::vector<FH2RemoteClient>::iterator it1 = clients.begin();
    std::vector<FH2RemoteClient>::const_iterator it2 = clients.end();
    for(; it1 != it2; ++it1)
    {
	(*it1).player_color = 0;
	(*it1).player_race = 0;
    }

    conf.SetPlayersColors(0);
    u8 colors = 0;

    // set first admin
    it1 = std::find_if(clients.begin(), clients.end(), std::bind2nd(std::mem_fun_ref(&FH2RemoteClient::Modes), ST_ADMIN));
    if(it1 != clients.end())
    {
        (*it1).player_color = conf.FirstAllowColor();
        colors |= (*it1).player_color;
    }

    // set other
    it1 = clients.begin();
    it2 = clients.end();
    for(; it1 != it2; ++it1) if(0 == (*it1).player_color)
    {
        const u8 color = Color::GetFirst(conf.AllowColors() & (~colors));
        if(color)
        {
            (*it1).player_color = color;
            colors |= color;
        }
        else
        // no free colors, shutdown client
        {
            (*it1).SetModes(ST_SHUTDOWN);
        }
    }

    conf.SetPlayersColors(colors);
}

void FH2Server::CloseClients(void)
{
    std::for_each(clients.begin(), clients.end(), std::mem_fun_ref(&FH2RemoteClient::ShutdownThread));
}

int FH2Server::Main(void)
{
    WaitClients();
    StartGame();

    // stop scan queue
    //Timer::Remove(timer);

    CloseClients();

    Close();

    return 1;
}

bool FH2Server::IsRun(void) const
{
    return !exit;
}

void FH2Server::WaitClients(void)
{
    Settings & conf = Settings::Get();
    std::vector<FH2RemoteClient>::iterator it;
    QueueMessage packet(MSG_UNKNOWN);

    clients.reserve(8);
    exit = false;
    start_game = false;

    // wait players
    while(!exit && !start_game)
    {
    	if(TCPsocket csd = Accept())
    	{
	    const u8 players = std::count_if(clients.begin(), clients.end(), std::mem_fun_ref(&FH2RemoteClient::IsConnected));

    	    // request admin
	    it = std::find_if(clients.begin(), clients.end(), std::bind2nd(std::mem_fun_ref(&FH2RemoteClient::Modes), ST_ADMIN));

	    // check count players
    	    if((conf.PreferablyCountPlayers() <= players) ||
    	    // check admin allow connect
    		((clients.end() != it && (*it).Modes(ST_CONNECT)) && !(*it).Modes(ST_ALLOWPLAYERS)))
	    {
		DEBUG(DBG_NETWORK, DBG_INFO, "FH2Server::" << "WaitClients: " << "max players, current: " << static_cast<int>(conf.PreferablyCountPlayers()));
		Socket sct(csd);
		// send message
		sct.Close();
	    }
	    else
	    // connect
    	    {
    		// find free socket
    		it = std::find_if(clients.begin(), clients.end(), std::not1(std::mem_fun_ref(&FH2RemoteClient::IsConnected)));
    		if(it == clients.end())
    		{
            	    clients.push_back(FH2RemoteClient());
            	    it = clients.end();
            	    --it;
        	}

		// first player: set admin mode
		if(0 == players)
		    (*it).SetModes(ST_ADMIN);

        	(*it).Assign(csd);
    		(*it).RunThread();
	    }
	}

	DELAY(100);
    }
}

void FH2Server::StartGame(void)
{
    Settings & conf = Settings::Get();

    conf.FixKingdomRandomRace();
    std::for_each(clients.begin(), clients.end(), Player::FixRandomRace);
    conf.SetPlayersColors(Network::GetPlayersColors(clients));

    QueueMessage packet;

    world.LoadMaps(conf.MapsFile());

    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color) if(color & conf.PlayersColors())
	world.GetKingdom(color).SetControl(Game::REMOTE);

    std::for_each(clients.begin(), clients.end(), std::bind2nd(std::mem_fun_ref(&FH2RemoteClient::SetModes), ST_INGAME));

    GameOver::Result::Get().Reset();
    std::vector<FH2RemoteClient>::iterator it;

/*
    while(!exit && conf.PlayersColors())
    {
	world.NewDay();

	// sync world
	mutex.Lock();
	packet.Reset();
	Game::IO::SaveBIN(packet);
	packet.SetID(MSG_LOAD_KINGDOM);
	SendPacketToAllClients(clients, packet, 0);
	mutex.Unlock();
*/

    // TEST BATTLE NETWORK
    it = std::find_if(clients.begin(), clients.end(), std::bind2nd(std::mem_fun_ref(&FH2RemoteClient::Modes), ST_ADMIN));
    Kingdom & kingdom = world.GetKingdom((*it).player_color);

    Heroes & hero1 = *world.GetHeroes(Heroes::SANDYSANDY);
    Heroes & hero2 = *world.GetHeroes(Heroes::RIALDO);

    hero1.SetSpellPoints(150);
    hero1.Recruit(kingdom.GetColor(), Point(20, 20));
    hero2.Recruit(Color::GRAY, Point(20, 21));

    Army::army_t & army1 = hero1.GetArmy();
    Army::army_t & army2 = hero2.GetArmy();

    army1.Clear();
    army1.JoinTroop(Monster::PHOENIX, 10);
    army1.JoinTroop(Monster::RANGER, 80);

    army2.Clear();
    army2.At(0) = Army::Troop(Monster::SKELETON, 400);
    army2.At(2) = Army::Troop(Monster::SKELETON, 400);
    army2.At(4) = Army::Troop(Monster::SKELETON, 400);

    // sync world
    mutex.Lock();
    packet.Reset();
    Game::IO::SaveBIN(packet);
    packet.SetID(MSG_MAPS_LOAD);
    SendPacketToAllClients(clients, packet, 0);
    mutex.Unlock();

    const u16 index = 33;

    packet.Reset();
    packet.SetID(MSG_BATTLE);
    // attacker monster oor hero
    packet.Push(static_cast<u8>(1));
    packet.Push(hero1.GetIndex());
    // defender monster, hero or castle
    packet.Push(static_cast<u8>(1));
    packet.Push(hero2.GetIndex());
    packet.Push(index);

    Lock();
    SendToAllClients(packet);
    Unlock();

    Battle2::Loader(army1, army2, index);

/*
	// send all tiles
	// send action new day
	
	for(Color::color_t color = Color::BLUE; color != Color::GRAY && !exit; ++color)
	{
	    Kingdom & kingdom = world.GetKingdom(color);

	    if(!kingdom.isPlay()) continue;

	    conf.SetCurrentColor(color);
	    world.ClearFog(color);

	    // send turn
	    mutex.Lock();
	    packet.Reset();
	    packet.SetID(MSG_YOUR_TURN);
    	    packet.Push(static_cast<u8>(color));
	    packet.Push(static_cast<u8>(0));
	    SendPacketToAllClients(clients, packet, 0);
	    mutex.Unlock();

	    switch(kingdom.Control())
	    {
		default:
		    conf.SetMyColor(color);

		    it = std::find_if(clients.begin(), clients.end(), std::bind2nd(std::mem_fun_ref(&FH2RemoteClient::isColor), color));
		    if(it == clients.end()) break;

		    (*it).SetModes(ST_TURN);
		    DEBUG(DBG_NETWORK, DBG_INFO, "Server: Player turn: " << Color::String(color));
		    // wait turn
		    while(!exit && (*it).Modes(ST_TURN)) DELAY(100);
		    // lost connection
		    if(!(*it).Modes(ST_CONNECT))
		    {
    			mutex.Lock();
			world.GetKingdom(color).SetControl(Game::AI);
			conf.SetPlayersColors(conf.PlayersColors() & (~color));
			mutex.Unlock();
		    }
		    break;

		case Game::AI:
		    DEBUG(DBG_NETWORK, DBG_INFO, "Server: AI turn: " << Color::String(color));
        	    kingdom.AITurns();

        	    //packet.Reset();
        	    //packet.SetID(MSG_KINGDOM);
        	    //packet.Push(static_cast<u8>(kingdom.GetColor()));
        	    //Game::IO::PackKingdom(packet, kingdom);
        	    //Lock();
        	    //SendToAllClients(packet);
        	    //Unlock();
		    break;
	    }
	}

	DELAY(100);
    }
*/
}


FH2RemoteClient* FH2Server::GetRemoteClient(u8 color)
{
    std::vector<FH2RemoteClient>::iterator it;
    it = std::find_if(clients.begin(), clients.end(), std::bind2nd(std::mem_fun_ref(&FH2RemoteClient::isColor), color));

    return it != clients.end() ? &(*it) : NULL;
}

Game::menu_t Game::NetworkHost(void)
{
    Settings & conf = Settings::Get();
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();

    // select count players
    const u8 max_players = SelectCountPlayers();

    if(2 > max_players) return Game::MAINMENU;
    conf.SetPreferablyCountPlayers(max_players);
    conf.SetGameType(Game::NETWORK);

    // clear background
    const Sprite &back = AGG::GetICN(ICN::HEROES, 0);
    cursor.Hide();
    display.Blit(back);
    cursor.Show();
    display.Flip();

    // create local server
    FH2Server & server = FH2Server::Get();

    if(! server.Bind(conf.GetPort()))
    {
	Dialog::Message(_("Error"), Network::GetError(), Font::BIG, Dialog::OK);
	return Game::MAINMENU;
    }

    SDL::Thread thread;
    thread.Create(FH2Server::callbackCreateThread, &server);

    // create local client
    const std::string localhost("127.0.0.1");
    FH2LocalClient & client = FH2LocalClient::Get();

    // connect to server
    if(client.Connect(localhost, conf.GetPort()))
    {
	client.SetModes(ST_LOCALSERVER);
	client.Main();
    }
    else
        Dialog::Message(_("Error"), Network::GetError(), Font::BIG, Dialog::OK);

    server.Lock();
    server.SetExit();
    server.Unlock();

    if(0 > thread.Wait())
    {
	Dialog::Message(_("Error"), Network::GetError(), Font::BIG, Dialog::OK);
	return Game::MAINMENU;
    }
    server.Close();

    return QUITGAME;
}

#else
Game::menu_t Game::NetworkHost(void)
{
    Dialog::Message(_("Error"), _("This release is compiled without network support."), Font::BIG, Dialog::OK);
    return MAINMENU;
}
#endif

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

#include <cstdlib>
#include <algorithm>
#include <functional>

#include "sdlnet.h"
#include "settings.h"
#include "server.h"
#include "error.h"
#include "remoteclient.h"
#include "localclient.h"
#include "kingdom.h"
#include "castle.h"
#include "heroes.h"
#include "game_io.h"
#include "maps_tiles.h"
#include "world.h"
#include "network.h"

#ifdef WITH_NET

const char* Network::GetMsgString(u16 msg)
{
    switch(msg)
    {
        case MSG_RAW:           	return "MSG_RAW";

        case MSG_PING:          	return "MSG_PING";
        case MSG_READY:         	return "MSG_READY";
        case MSG_MESSAGE:       	return "MSG_MESSAGE";

        case MSG_HELLO:         	return "MSG_HELLO";
        case MSG_LOGOUT:        	return "MSG_LOGOUT";
        case MSG_SHUTDOWN:      	return "MSG_SHUTDOWN";
        case MSG_ACCESS_DENIED:		return "MSG_ACCESS_DENIED";

        case MSG_UPDATE_PLAYERS:	return "MSG_UPDATE_PLAYERS";
        case MSG_GET_MAPS_LIST:		return "MSG_GET_MAPS_LIST";
        case MSG_SET_CURRENT_MAP:	return "MSG_SET_CURRENT_MAP";
        case MSG_CHANGE_COLORS:		return "MSG_CHANGE_COLORS";
        case MSG_CHANGE_RACE:		return "MSG_CHANGE_RACE";


        case MSG_START_GAME:		return "MSG_START_GAME";
        case MSG_MAPS_LOAD:		return "MSG_MAPS_LOAD";
        case MSG_MAPS_LOAD_ERR:		return "MSG_MAPS_LOAD_ERR";


        case MSG_CASTLE_BUILD:		return "MSG_CASTLE_BUILD";
        case MSG_CASTLE_RECRUIT_HERO:	return "MSG_CASTLE_RECRUIT_HERO";
        case MSG_CASTLE_BUY_BOAT:	return "MSG_CASTLE_BUY_BOAT";
        case MSG_CASTLE_RECRUIT_MONSTER:return "MSG_CASTLE_RECRUIT_MONSTER";

        case MSG_MARKET_SELL_RESOURCE:	return "MSG_MARKET_SELL_RESOURCE";
        case MSG_MARKET_BUY_RESOURCE:	return "MSG_MARKET_BUY_RESOURCE";

        case MSG_HEROES_BUY_MAGICBOOK:	return "MSG_HEROES_BUY_MAGICBOOK";
        case MSG_HEROES_SWAP_ARTIFACTS:	return "MSG_HEROES_SWAP_ARTIFACTS";

	case MSG_ARMY_UPGRADE_TROOP:	return "MSG_ARMY_UPGRADE_TROOP";
	case MSG_ARMY_DISMISS_TROOP:	return "MSG_ARMY_DISMISS_TROOP";
	case MSG_ARMY_SWAP_TROOPS:	return "MSG_ARMY_SWAP_TROOPS";
	case MSG_ARMY_SPLIT_TROOP:	return "MSG_ARMY_SPLIT_TROOP";
	case MSG_ARMY_JOIN_TROOP:	return "MSG_ARMY_JOIN_TROOP";
        case MSG_ARMY_COMBAT_FORMATION: return "MSG_ARMY_COMBAT_FORMATION";

        case MSG_BATTLE_BOARD:		return "MSG_BATTLE_BOARD";
        case MSG_BATTLE_MOVE:		return "MSG_BATTLE_MOVE";
        case MSG_BATTLE_ATTACK:		return "MSG_BATTLE_ATTACK";
        case MSG_BATTLE_DEFENSE:	return "MSG_BATTLE_DEFENSE";
        case MSG_BATTLE_DAMAGE:		return "MSG_BATTLE_DAMAGE";
        case MSG_BATTLE_CAST:		return "MSG_BATTLE_CAST";
        case MSG_BATTLE_SKIP:		return "MSG_BATTLE_SKIP";
        case MSG_BATTLE_MORALE:		return "MSG_BATTLE_MORALE";
        case MSG_BATTLE_LUCK:		return "MSG_BATTLE_LUCK";
        case MSG_BATTLE_CATAPULT:	return "MSG_BATTLE_CATAPULT";
        case MSG_BATTLE_TOWER:		return "MSG_BATTLE_TOWER";
        case MSG_BATTLE_END_TURN:	return "MSG_BATTLE_END_TURN";
        case MSG_BATTLE_RETREAT:	return "MSG_BATTLE_RETREAT";
        case MSG_BATTLE_SURRENDER:	return "MSG_BATTLE_SURRENDER";
        case MSG_BATTLE_TURN:		return "MSG_BATTLE_TURN";
        case MSG_BATTLE_RESULT:		return "MSG_BATTLE_RESULT";

        case MSG_YOUR_TURN:     return "MSG_YOUR_TURN";
        case MSG_END_TURN:      return "MSG_END_TURN";
        case MSG_TILES:         return "MSG_TILES";
        case MSG_HEROES:        return "MSG_HEROES";
        case MSG_BATTLE:        return "MSG_BATTLE";
        case MSG_CASTLE:        return "MSG_CASTLE";
        case MSG_SPELL:         return "MSG_SPELL";
        case MSG_MAPS:          return "MSG_MAPS";
        case MSG_KINGDOM:       return "MSG_KINGDOM";
        case MSG_WORLD:         return "MSG_WORLD";

        default: break;
    }

    return "MSG_UNKNOWN";
}

msg_t Network::GetMsg(u16 msg)
{
    return msg < MSG_UNKNOWN ? static_cast<msg_t>(msg) : MSG_UNKNOWN;
}

bool Network::isLocalClient(void)
{
    Settings & conf = Settings::Get();
    return conf.GameType() == Game::NETWORK && conf.NetworkLocalClient() && !conf.NetworkDedicatedServer();
}

bool Network::isRemoteClient(void)
{
    Settings & conf = Settings::Get();
    return conf.GameType() == Game::NETWORK && !isLocalClient();
}

bool Network::MsgIsBroadcast(u16 msg)
{
    switch(msg)
    {
	case MSG_SHUTDOWN:
	case MSG_MESSAGE:
	    return true;
    }
    return false;
}

int Network::RunDedicatedServer(void)
{
    Settings & conf = Settings::Get();

    Network::SetProtocolVersion(static_cast<u16>((conf.MajorVersion() << 8)) | conf.MinorVersion());
    
    if(SDL::Init(INIT_TIMER))
    try
    {
        std::atexit(SDL::Quit);

        FH2Server & server = FH2Server::Get();

        if(! server.Bind(conf.GetPort()))
        {
            DEBUG(DBG_NETWORK , DBG_WARN, "Network::RunDedicatedServer: " << Network::GetError());
            return -1;
        }

	conf.SetNetworkDedicatedServer(true);
	conf.SetGameType(Game::NETWORK);

        return FH2Server::callbackCreateThread(&server);
    }
    catch(std::bad_alloc)
    {
    }
    catch(Error::Exception)
    {
        conf.Dump();
    }

    return 0;
}

void Network::PacketPopMapsFileInfoList(QueueMessage & packet, MapsFileInfoList & flist)
{
    flist.clear();
    u16 count;
    packet.Pop(count);
    for(u16 ii = 0; ii < count; ++ii)
    {
	Maps::FileInfo fi;
	PacketPopMapsFileInfo(packet, fi);
	flist.push_back(fi);
    }
}

void Network::PacketPushMapsFileInfo(QueueMessage & packet, const Maps::FileInfo & fi)
{
    packet.Push(fi.file);
    packet.Push(fi.name);
    packet.Push(fi.description);
    packet.Push(fi.size_w);
    packet.Push(fi.size_h);
    packet.Push(fi.difficulty);

    packet.Push(static_cast<u8>(KINGDOMMAX));
    for(u8 ii = 0; ii < KINGDOMMAX; ++ii) packet.Push(fi.races[ii]);

    packet.Push(fi.kingdom_colors);
    packet.Push(fi.human_colors);
    packet.Push(fi.computer_colors);
    packet.Push(fi.rnd_races);
    packet.Push(fi.conditions_wins);
    packet.Push(fi.wins1);
    packet.Push(fi.wins2);
    packet.Push(fi.wins3);
    packet.Push(fi.wins4);
    packet.Push(fi.conditions_loss);
    packet.Push(fi.loss1);
    packet.Push(fi.loss2);
    packet.Push(static_cast<u32>(fi.localtime));
    packet.Push(static_cast<u8>(fi.with_heroes));
}

void Network::PacketPopMapsFileInfo(QueueMessage & packet, Maps::FileInfo & fi)
{
    u8 byte8, race;
    u32 byte32;

    packet.Pop(fi.file);
    packet.Pop(fi.name);
    packet.Pop(fi.description);
    packet.Pop(fi.size_w);
    packet.Pop(fi.size_h);
    packet.Pop(fi.difficulty);

    packet.Pop(byte8);
    for(u8 ii = 0; ii < byte8; ++ii){ packet.Pop(race); fi.races[ii] = race; }

    packet.Pop(fi.kingdom_colors);
    packet.Pop(fi.human_colors);
    packet.Pop(fi.computer_colors);
    packet.Pop(fi.rnd_races);
    packet.Pop(fi.conditions_wins);
    packet.Pop(fi.wins1);
    packet.Pop(fi.wins2);
    packet.Pop(fi.wins3);
    packet.Pop(fi.wins4);
    packet.Pop(fi.conditions_loss);
    packet.Pop(fi.loss1);
    packet.Pop(fi.loss2);
    packet.Pop(byte32);
    fi.localtime = byte32;
    packet.Pop(byte8);
    fi.with_heroes = byte8;
}

void Network::PacketPushPlayersInfo(QueueMessage & m, const std::vector<FH2RemoteClient> & v, u32 exclude)
{
    u8 count = std::count_if(v.begin(), v.end(), std::not1(std::bind2nd(std::mem_fun_ref(&Player::isID), 0)));
    m.Push(count);
    
    if(count)
    {
	std::vector<FH2RemoteClient>::const_iterator itc1 = v.begin();
	std::vector<FH2RemoteClient>::const_iterator itc2 = v.end();
	for(; itc1 != itc2; ++itc1) if((*itc1).player_id && (*itc1).player_id != exclude)
	{
	    m.Push((*itc1).player_color);
	    m.Push((*itc1).player_race);
    	    m.Push((*itc1).player_name);
	    m.Push((*itc1).player_id);
	    m.Push(static_cast<u8>((*itc1).Modes(ST_ADMIN)));
	}
    }
}

void Network::PackRaceColors(QueueMessage & m)
{
    m.Push(static_cast<u8>(KINGDOMMAX));

    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color)
    {
	m.Push(static_cast<u8>(color));
	m.Push(static_cast<u8>(Settings::Get().KingdomRace(color)));
    }
}

void Network::UnpackRaceColors(QueueMessage & m)
{
    u8 max, color, race;

    m.Pop(max);
    if(max == KINGDOMMAX)
    {
	for(u8 ii = 0; ii < max; ++ii)
	{
	    m.Pop(color);
	    m.Pop(race);
	    Settings::Get().SetKingdomRace(color, race);
	}
    }
}

u8 Network::GetPlayersColors(const std::vector<FH2RemoteClient> & v)
{
    u8 res = 0;
    std::vector<FH2RemoteClient>::const_iterator it1 = v.begin();
    std::vector<FH2RemoteClient>::const_iterator it2 = v.end();
    for(; it1 != it2; ++it1) if((*it1).player_id && (*it1).player_color) res |= (*it1).player_color;
                
    return res;
}

void Network::PackKingdom(QueueMessage & msg, const Kingdom & kingdom)
{
    msg.Reset();
    msg.SetID(MSG_KINGDOM);
    msg.Push(static_cast<u8>(kingdom.GetColor()));
    Game::IO::PackKingdom(msg, kingdom);
}

void Network::UnpackKingdom(QueueMessage & msg)
{
    u8 kingdom_color;
    msg.Pop(kingdom_color);
    Kingdom & kingdom = world.GetKingdom(kingdom_color);
    Game::IO::UnpackKingdom(msg, kingdom);
}

void Network::PackHero(QueueMessage & msg, const Heroes & hero)
{
    msg.Reset();
    msg.SetID(MSG_HEROES);
    msg.Push(static_cast<u8>(hero.GetID()));
    Game::IO::PackHeroes(msg, hero);
}

void Network::UnpackHero(QueueMessage & msg)
{
    u8 hero_id;
    msg.Pop(hero_id);
    Heroes *hero = world.GetHeroes(Heroes::ConvertID(hero_id));
    if(hero)
	Game::IO::UnpackHeroes(msg, *hero);
    else
	DEBUG(DBG_NETWORK, DBG_WARN, "Network::UnpackHero: unknown hero id");
}

void Network::PackTile(QueueMessage & msg, const Maps::Tiles & tile)
{
    msg.Reset();
    msg.SetID(MSG_TILES);
    msg.Push(tile.GetIndex());
    Game::IO::PackTile(msg, tile);
}

void Network::UnpackTile(QueueMessage & msg)
{
    s32 tile_index;
    msg.Pop(tile_index);
    Maps::Tiles & tile = world.GetTiles(tile_index);
    Game::IO::UnpackTile(msg, tile);
}

void Network::PackCastle(QueueMessage & msg, const Castle & castle)
{
    msg.Reset();
    msg.SetID(MSG_CASTLE);
    msg.Push(castle.GetIndex());
    Game::IO::PackCastle(msg, castle);
}

void Network::UnpackCastle(QueueMessage & msg)
{
    s32 castle_index;
    msg.Pop(castle_index);
    Castle *castle = world.GetCastle(castle_index);
    if(castle)
	Game::IO::UnpackCastle(msg, *castle);
    else
	DEBUG(DBG_NETWORK, DBG_WARN, "Network::UnpackCastle: unknown index id");
}

#endif

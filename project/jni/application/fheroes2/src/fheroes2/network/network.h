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

#ifndef H2NETWORK_H
#define H2NETWORK_H

#include "sdlnet.h"
#include "gamedefs.h"

enum msg_t
{
    MSG_RAW,

    MSG_PING,
    MSG_READY,
    MSG_MESSAGE,

    MSG_HELLO,
    MSG_LOGOUT,
    MSG_SHUTDOWN,
    MSG_ACCESS_DENIED,

    MSG_UPDATE_PLAYERS,
    MSG_GET_MAPS_LIST,
    MSG_SET_CURRENT_MAP,
    MSG_CHANGE_COLORS,
    MSG_CHANGE_RACE,

    MSG_START_GAME,
    MSG_MAPS_LOAD,
    MSG_MAPS_LOAD_ERR,


    MSG_YOUR_TURN,
    MSG_END_TURN,

    MSG_CASTLE_BUILD,
    MSG_CASTLE_RECRUIT_HERO,
    MSG_CASTLE_BUY_BOAT,
    MSG_CASTLE_RECRUIT_MONSTER,

    MSG_MARKET_SELL_RESOURCE,
    MSG_MARKET_BUY_RESOURCE,

    MSG_HEROES_BUY_MAGICBOOK,
    MSG_HEROES_SWAP_ARTIFACTS,

    MSG_ARMY_UPGRADE_TROOP,
    MSG_ARMY_DISMISS_TROOP,
    MSG_ARMY_SWAP_TROOPS,
    MSG_ARMY_JOIN_TROOP,
    MSG_ARMY_SPLIT_TROOP,
    MSG_ARMY_COMBAT_FORMATION,

    MSG_BATTLE_BOARD,
    MSG_BATTLE_MOVE, 
    MSG_BATTLE_ATTACK,
    MSG_BATTLE_DEFENSE,
    MSG_BATTLE_DAMAGE,
    MSG_BATTLE_CAST,
    MSG_BATTLE_MORALE,
    MSG_BATTLE_LUCK,
    MSG_BATTLE_CATAPULT,
    MSG_BATTLE_TOWER,
    MSG_BATTLE_RETREAT,
    MSG_BATTLE_SURRENDER,
    MSG_BATTLE_SKIP,
    MSG_BATTLE_END_TURN,
    MSG_BATTLE_TURN,
    MSG_BATTLE_RESULT,

    MSG_TILES,
    MSG_HEROES,
    MSG_BATTLE,
    MSG_CASTLE,
    MSG_SPELL,
    MSG_MAPS,
    MSG_KINGDOM,
    MSG_WORLD,

    MSG_UNKNOWN
};

#ifdef WITH_NET

#include "client.h"
#include "maps_fileinfo.h"

typedef std::pair<QueueMessage, u32> MessageID;
class FH2RemoteClient;
class Kingdom;
class Heroes;
class Castle;
namespace Maps { class Tiles; }


namespace Network
{
    int			RunDedicatedServer(void);
    const char*         GetMsgString(u16);
    msg_t		GetMsg(u16);
    bool		MsgIsBroadcast(u16);

    bool		isLocalClient(void);
    bool		isRemoteClient(void);

    void		PacketPopMapsFileInfoList(QueueMessage &, MapsFileInfoList &);
    void		PacketPushMapsFileInfo(QueueMessage &, const Maps::FileInfo &);
    void		PacketPopMapsFileInfo(QueueMessage &, Maps::FileInfo &);
    void		PacketPushPlayersInfo(QueueMessage &, const std::vector<FH2RemoteClient> &, u32 exclude = 0);

    void		PackRaceColors(QueueMessage &);
    void		UnpackRaceColors(QueueMessage &);

    u8			GetPlayersColors(const std::vector<FH2RemoteClient> &);
    
    void		PackKingdom(QueueMessage &, const Kingdom &);
    void		UnpackKingdom(QueueMessage &);

    void		PackTile(QueueMessage &, const Maps::Tiles &);
    void		UnpackTile(QueueMessage &);

    void		PackHero(QueueMessage &, const Heroes &);
    void		UnpackHero(QueueMessage &);

    void		PackCastle(QueueMessage &, const Castle &);
    void		UnpackCastle(QueueMessage &);
}

#endif

#endif

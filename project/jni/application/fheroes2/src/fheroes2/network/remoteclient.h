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

#ifndef H2REMOTECLIENT_H
#define H2REMOTECLIENT_H

#ifdef WITH_NET

#include "gamedefs.h"
#include "thread.h"
#include "network.h"

class FH2Server;
namespace Battle2
{
    class Actions;
    class Arena;
    class Stats;
    struct Result;
    struct TargetInfo;
}

class FH2RemoteClient : public FH2Client
{
public:
    FH2RemoteClient();

    int Main(void);
    bool ConnectionChat(void);
    bool StartGame(void);
    void CloseConnection(void);

    void RunThread(void);
    void ShutdownThread(void);

    void MsgBroadcast(void);
    void MsgPing(void);
    void MsgLogout(void);
    void MsgChangeColors(void);
    void MsgChangeRace(void);

    bool RecvBattleHumanTurn(const Battle2::Stats &, const Battle2::Arena & arena, Battle2::Actions &);

    bool SendSetCurrentMap(void);
    bool SendMapsInfoList(void);
    bool SendAccessDenied(void);
    void SendUpdatePlayers(u32 exclude);

    static bool SendBattleAction(u8, QueueMessage &);
    static bool SendBattleResult(u8, const Battle2::Result &);
    static bool SendBattleAttack(u8, u16, const std::vector<Battle2::TargetInfo> &);
    static bool SendBattleBoard(u8, const Battle2::Arena &);
    static bool SendBattleSpell(u8, u8, u8, const std::vector<Battle2::TargetInfo> &);
    static bool SendBattleTeleportSpell(u8, u16, u16);
    static bool SendBattleEarthQuakeSpell(u8, const std::vector<u8> &);

    static int callbackCreateThread(void *);

    SDL::Thread thread;
};

#endif
#endif

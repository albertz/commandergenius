/***************************************************************************
 *   Copyright (C) 2010 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#ifndef H2BATTLE2_H
#define H2BATTLE2_H

#include <vector>
#include <utility>
#include "network.h"
#include "icn.h"
#include "m82.h"
#include "gamedefs.h"

#ifdef WITH_NET
#include "localclient.h"
#endif

namespace Army { class army_t; }
namespace Battle2
{
    struct Stats;

    enum { RESULT_LOSS = 0x01, RESULT_RETREAT = 0x02, RESULT_SURRENDER = 0x04, RESULT_WINS = 0x80 };

    struct Result
    {
	Result() : army1(0), army2(0), exp1(0), exp2(0) {}

	bool AttackerWins(void) const;
	bool DefenderWins(void) const;
	u8   AttackerResult(void) const;
	u8   DefenderResult(void) const;
	u32  GetExperienceAttacker(void) const;
	u32  GetExperienceDefender(void) const;

	u8 army1;
	u8 army2;
	u32  exp1;
	u32  exp2;
    };

    Result Loader(Army::army_t &, Army::army_t &, s32);
    void UpdateMonsterInfoAnimation(const std::string &);
    void UpdateMonsterAttributes(const std::string &);

    enum direction_t
    {
        UNKNOWN         = 0xFF,
        TOP_LEFT        = 0x01,
        TOP_RIGHT       = 0x02,
        RIGHT           = 0x04,
        BOTTOM_RIGHT    = 0x08,
        BOTTOM_LEFT     = 0x10,
        LEFT            = 0x20,
        CENTER          = 0x40
    };

    inline direction_t & operator++ (direction_t & d){ return d = ( CENTER == d ? TOP_LEFT : direction_t(d << 1)); }
    inline direction_t & operator-- (direction_t & d){ return d = ( TOP_LEFT == d ? CENTER : direction_t(d >> 1)); }

    enum tower_t { TWR_LEFT, TWR_CENTER, TWR_RIGHT };

    enum catapult_t { CAT_WALL1 = 1, CAT_WALL2 = 2, CAT_WALL3 = 3, CAT_WALL4 = 4, CAT_TOWER1 = 5, CAT_TOWER2 = 6, CAT_BRIDGE = 7, CAT_TOWER3 = 8, CAT_MISS = 9 };

    enum animstate_t
    {
        AS_NONE,
        AS_IDLE,
        AS_MOVE,
        AS_FLY1,
        AS_FLY2,
        AS_FLY3,
        AS_SHOT0,
        AS_SHOT1,
        AS_SHOT2,
        AS_SHOT3,
        AS_ATTK0,
        AS_ATTK1,
        AS_ATTK2,
        AS_ATTK3,
        AS_WNCE,
        AS_KILL
    };

    struct animframe_t
    {
        u8 start;
        u8 count;
    };

    struct MonsterInfo
    {
        ICN::icn_t icn_file;
        animframe_t frm_idle;
        animframe_t frm_move;
        animframe_t frm_fly1;
        animframe_t frm_fly2;
        animframe_t frm_fly3;
        animframe_t frm_shot0;
        animframe_t frm_shot1;
        animframe_t frm_shot2;
        animframe_t frm_shot3;
        animframe_t frm_attk0;
        animframe_t frm_attk1;
        animframe_t frm_attk2;
        animframe_t frm_attk3;
        animframe_t frm_wnce;
        animframe_t frm_kill;
        M82::m82_t m82_attk;
        M82::m82_t m82_kill;
        M82::m82_t m82_move;
        M82::m82_t m82_wnce;
    };

    struct TargetInfo
    {
	TargetInfo() : defender(NULL), damage(0), killed(0), resist(false) {};

        Stats* defender;
	u32 damage;
	u32 killed;
	bool resist;

        bool isFinishAnimFrame(void) const;
    };

    typedef QueueMessage Action;
}

#endif

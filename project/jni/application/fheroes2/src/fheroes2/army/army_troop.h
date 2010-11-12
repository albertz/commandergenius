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

#ifndef H2ARMYTROOP_H
#define H2ARMYTROOP_H

#include <string>
#include "bitmodes.h"
#include "monster.h"

namespace Battle2
{
    class Arena;
    class Stats;
}

namespace Maps { class Tiles; }

namespace Army
{
    class army_t;
    class Troop;

    bool isValidTroop(const Troop & troop);
    bool WeakestTroop(const Troop & t1, const Troop & t2);
    bool StrongestTroop(const Troop & t1, const Troop & t2);
    bool SlowestTroop(const Troop & t1, const Troop & t2);
    void SwapTroops(Troop & t1, Troop & t2);

    class Troop : public Monster
    {
      public:
        Troop(monster_t m = Monster::UNKNOWN, u32 c = 0);
        Troop(const Troop &);
	Troop(const Maps::Tiles &);
	~Troop();
        
        Troop & operator= (const Troop &);

        void	Set(const Monster &, u32);
        void	Set(monster_t, u32);
        void	SetMonster(const Monster &);
        void	SetMonster(monster_t);
        void	SetCount(u32);
        void	Reset(void);
        
        const Skill::Primary* MasterSkill(void) const;
        army_t*         GetArmy(void);
        const army_t*   GetArmy(void) const;
	const char*     GetName(u32 amount = 0) const;

        u32 		Count(void) const { return GetCount(); }
        u32 		GetCount(void) const;
        
	u8		GetAttack(void) const;
	u8		GetDefense(void) const;
	u32		GetHitPoints(void) const;

	Color::color_t	GetColor(void) const;
        u8		GetSpeed(void) const;

	const Battle2::Stats*	GetBattleStats(void) const;
	Battle2::Stats*	GetBattleStats(void);
        u32		GetDamageMin(void) const;
	u32		GetDamageMax(void) const;

        s8		GetMorale(void) const;
	s8		GetLuck(void) const;

        bool		isValid(void) const;
        bool    	isAffectedByMorale(void) const;
	bool		HasMonster(monster_t) const;

	bool		BattleInit(void);
	void		BattleQuit(void);
	void		BattleNewTurn(void);
	void		BattleSetModes(u32);
	void		BattleResetModes(u32);
	bool		BattleFindModes(u32) const;
	bool		BattleIsDragons(void) const;
	bool		BattleIsArchers(void) const;
	u32		BattleKilled(void) const;
	s8		GetArmyIndex(void) const;

      protected:
        friend class army_t;
        friend class Battle2::Stats;
        friend class Battle2::Arena;

        u32		count;
	army_t*		army;
	Battle2::Stats*	battle;
    };
}

#endif

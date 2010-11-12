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

#ifndef H2ARMY_H
#define H2ARMY_H

#include <string>
#include <vector>
#include "bitmodes.h"
#include "heroes_base.h"
#include "army_troop.h"

class Castle;
class Heroes;
namespace Maps { class Tiles; }
namespace Battle2 { class Stats; }
namespace Resource { struct funds_t; }

namespace Army
{
    class BattleTroop;

    enum format_t
    {
	FORMAT_GROUPED = 0,
	FORMAT_SPREAD  = 1
    };

    enum armysize_t
    {
	FEW	= 1,
	SEVERAL	= 5,
	PACK	= 10,
	LOTS	= 20,
	HORDE	= 50,
	THRONG	= 100,
	SWARM	= 250,
	ZOUNDS	= 500,
	LEGION	= 1000
    };

    const char* String(u32);
    armysize_t GetSize(u32);

    // 0: fight, 1: free join, 2: join with gold, 3: flee
    u8 GetJoinSolution(const Heroes &, const Maps::Tiles &, u32 &, s32 &);

    class army_t
    {
	public:
	    army_t(HeroBase* s = NULL);
	    army_t(const army_t &);
	    army_t & operator= (const army_t &);

	    void	FromGuardian(const Maps::Tiles &);
	    void	UpgradeMonsters(const Monster &);
	    void	UpgradeMonsters(const Monster::monster_t);
	    void	Clear(void);
	    void	Reset(bool = false);	// reset: soft or hard

	    void	BattleExportKilled(army_t &) const;
	    void	BattleSetModes(u32);
	    void	BattleResetModes(u32);
	    Troop*      BattleFindModes(u32);
	    const Troop*BattleFindModes(u32) const;
	    void	BattleInit(void);
	    void	BattleQuit(void);
	    void	BattleNewTurn(void);
	    Troop &     BattleNewTroop(Monster::monster_t, u32);
	    bool	BattleArchersPresent(void) const;
	    bool	BattleDragonsPresent(void) const;
	    u32		BattleKilled(void) const;
	    u8		BattleUndeadTroopCount(void) const;
	    u8		BattleLifeTroopCount(void) const;
	    const Battle2::Stats* BattleRandomTroop(void) const;
	    Battle2::Stats* BattleSlowestTroop(bool skipmove);
	    Battle2::Stats* BattleFastestTroop(bool skipmove);

	    void	DrawMons32Line(s16, s16, u16, u8 = 0, u8 = 0, bool = false) const;

	    Troop &	FirstValid(void);
	    Troop &	At(u8);
	    Troop &	GetWeakestTroop(void);

	    const Troop &	At(u8) const;
	    const Troop &	GetSlowestTroop(void) const;

	    Race::race_t   GetRace(void) const;
	    Color::color_t GetColor(void) const;
	    u8             GetControl(void) const;

	    void        SetColor(Color::color_t);

	    u8		Size(void) const;
	    u8		GetCount(void) const;
	    u8		GetUniqCount(void) const;
	    u32		GetCountMonsters(const Monster &) const;
	    u32		GetCountMonsters(const Monster::monster_t) const;
	    s8		GetMorale(void) const;
	    s8		GetLuck(void) const;
	    s8		GetMoraleModificator(std::string *strs) const;
	    s8		GetLuckModificator(std::string *strs) const;
	    u32		CalculateExperience(void) const;
	    u32		ActionToSirens(void);
	    u32		GetSurrenderCost(void) const;

	    u16		GetAttack(void) const;
	    u16		GetDefense(void) const;
	    u32		GetHitPoints(void) const;
	    u32		GetDamageMin(void) const;
	    u32		GetDamageMax(void) const;
	    double	GetStrength(void) const;

	    bool	isValid(void) const;
	    bool	HasMonster(const Monster &) const;
	    bool	HasMonster(const Monster::monster_t) const;
	    bool	JoinTroop(const Troop & troop);
	    bool	JoinTroop(const Monster::monster_t mon, const u32 count);
	    bool	JoinTroop(const Monster & mon, const u32 count);
	    bool	StrongerEnemyArmy(const army_t &) const;
	    bool	AllTroopsIsRace(u8) const;

	    void	JoinStrongestFromArmy(army_t &);
            void	KeepOnlyWeakestTroops(army_t &);
	    void	UpgradeTroops(const Castle &);
	    void	ArrangeForBattle(void);

	    void	Dump(void) const;

	    const HeroBase* GetCommander(void) const;
	    HeroBase* GetCommander(void);
	    void        SetCommander(HeroBase*);

	    void        SetCombatFormat(format_t);
	    u8          GetCombatFormat(void) const;

	protected:
	    friend class Troop;
	    void	Import(army_t &);
	    void	Import(const std::vector<Troop> &);
	    s8		GetTroopIndex(const Troop &) const;

	    std::vector<Troop> army;
	    HeroBase*    commander;
	    u8 combat_format;
	    Color::color_t color;
    };
}

#endif

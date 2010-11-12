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
#include <functional>
#include "agg.h"
#include "speed.h"
#include "spell.h"
#include "settings.h"
#include "luck.h"
#include "morale.h"
#include "army.h"
#include "maps_tiles.h"
#include "army_troop.h"
#include "battle_stats.h"

Army::Troop::Troop(monster_t m, u32 c) : Monster(m), count(c), army(NULL), battle(NULL)
{
}

Army::Troop::Troop(const Troop & t) : Monster(t.id), count(t.count), army(t.army), battle(NULL)
{
}

Army::Troop::Troop(const Maps::Tiles & t) : army(NULL), battle(NULL)
{
    id = Monster::FromInt(t.GetQuantity3());
    count = t.GetCountMonster();
}

Army::Troop::~Troop()
{
    if(battle) BattleQuit();
}

Army::Troop & Army::Troop::operator= (const Troop & t)
{
    id = t.id;
    count = t.count;
    if(!army && t.army) army = t.army;
    if(battle) BattleQuit();

    return *this;
}

bool Army::Troop::HasMonster(monster_t m) const
{
    return id == m;
}

void Army::Troop::Set(const Monster & m, u32 c)
{
    Monster::Set(m);
    count = c;
}

void Army::Troop::Set(monster_t m, u32 c)
{
    Monster::Set(m);
    count = c;
}

void Army::Troop::SetMonster(const Monster & m)
{
    Monster::Set(m);
}

void Army::Troop::SetMonster(monster_t m)
{
    Monster::Set(m);
}

void Army::Troop::SetCount(u32 c)
{
    count = c;
}

void Army::Troop::Reset(void)
{
    Monster::Set(Monster::UNKNOWN);
    count = 0;
}

const Battle2::Stats* Army::Troop::GetBattleStats(void) const
{
    if(!battle) DEBUG(DBG_GAME, DBG_INFO, "Army::Troop::GetBattleStats: return NULL");
    return battle;
}

Battle2::Stats* Army::Troop::GetBattleStats(void)
{
    return battle;
}

bool Army::Troop::BattleInit(void)
{
    if(isValid())
    {
	if(battle) delete battle;
	battle = new Battle2::Stats(*this);
	return battle;
    }
    DEBUG(DBG_BATTLE, DBG_WARN, "Army::Troop::BattleInit: invalid troop");
    return false;
}

void Army::Troop::BattleQuit(void)
{
    if(battle)
    {
	if(battle->dead > count)
	{
	    VERBOSE("Army::Troop::BattleQuit: " << GetName() << ", dead(" << battle->dead << ") > current(" << count << ") incorrect!! FIXME!");
	    count = 0;
	}
	else
	    count -= battle->dead;

	delete battle;
	battle = NULL;
    }
}

void Army::Troop::BattleNewTurn(void)
{
    if(isValid() && battle) battle->NewTurn();
}

u32 Army::Troop::BattleKilled(void) const
{
    return battle ? battle->dead : 0;
}

void Army::Troop::BattleSetModes(u32 f)
{
    if(battle) battle->SetModes(f);
}

bool Army::Troop::BattleFindModes(u32 f) const
{
    return battle && battle->isValid() && battle->Modes(f);
}

void Army::Troop::BattleResetModes(u32 f)
{
    if(battle) battle->ResetModes(f);
}

bool Army::Troop::BattleIsDragons(void) const
{
    return battle && battle->isValid() && isDragons();
}

bool Army::Troop::BattleIsArchers(void) const
{
    return battle && battle->isValid() && battle->isArchers();
}

const Skill::Primary* Army::Troop::MasterSkill(void) const
{
    return army ? army->commander : NULL;
}

Army::army_t* Army::Troop::GetArmy(void)
{
    return army;
}

const Army::army_t* Army::Troop::GetArmy(void) const
{
    return army;
}

const char* Army::Troop::GetName(u32 amount /* = 0 */) const
{
    u32 cmp = count;
    if(amount)
        cmp = amount;
    return 1 < cmp ? Monster::GetMultiName() : Monster::GetName();
}

u32 Army::Troop::GetCount(void) const
{
    return count;
}

u8 Army::Troop::GetAttack(void) const
{
    return battle ? battle->GetAttack() : Monster::GetAttack() +
            (army && army->commander ? army->commander->GetAttack() : 0);
}

u8 Army::Troop::GetDefense(void) const
{
    return battle ? battle->GetDefense() : Monster::GetDefense() +
            (army && army->commander ? army->commander->GetDefense() : 0);
}

Color::color_t Army::Troop::GetColor(void) const
{
    return army ? army->GetColor() : Color::GRAY;
}

u32 Army::Troop::GetHitPoints(void) const
{
    return battle ? battle->GetHitPoints() : Monster::GetHitPoints() * count;
}

u32 Army::Troop::GetDamageMin(void) const
{
    return Monster::GetDamageMin() * count;
}

u32 Army::Troop::GetDamageMax(void) const
{
    return Monster::GetDamageMax() * count;
}

u8 Army::Troop::GetSpeed(void) const
{
    return battle ? battle->GetSpeed() : Monster::GetSpeed();
}

s8 Army::Troop::GetMorale(void) const
{
    return isAffectedByMorale() && army ? army->GetMorale() : Morale::NORMAL;
}

s8 Army::Troop::GetLuck(void) const
{
    return army ? army->GetLuck() : Luck::NORMAL;
}

bool Army::Troop::isValid(void) const
{
    return battle ? battle->isValid() : Monster::UNKNOWN < id && count;
}

bool Army::Troop::isAffectedByMorale(void) const
{
    return !(isUndead() || isElemental());
}

s8 Army::Troop::GetArmyIndex(void) const
{
    return army ? army->GetTroopIndex(*this) : -1;
}

bool Army::isValidTroop(const Troop & troop)
{
    return troop.isValid();
}

bool Army::WeakestTroop(const Troop & t1, const Troop & t2)
{
    return t1.GetDamageMax() < t2.GetDamageMax();
}

bool Army::StrongestTroop(const Troop & t1, const Troop & t2)
{
    return t1.GetDamageMin() > t2.GetDamageMin();
}

bool Army::SlowestTroop(const Troop & t1, const Troop & t2)
{
    return t1.GetSpeed() < t2.GetSpeed();
}

void Army::SwapTroops(Troop & t1, Troop & t2)
{
    std::swap(t1, t2);
}

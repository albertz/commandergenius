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

#include <algorithm>
#include <functional>
#include "settings.h"
#include "heroes.h"
#include "castle.h"
#include "battle_arena.h"
#include "battle_cell.h"
#include "battle_stats.h"
#include "battle_interface.h"

namespace Battle2
{
    bool isApplySpell(const Spell::spell_t, const Stats*, const HeroBase &, Actions &);
}

void Battle2::Arena::AITurn(const Stats & b, Actions & a)
{
    DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::AITurn: " << \
        b.GetName() << "(color: " << Color::String(b.GetColor()) << \
        ", id: " << b.id << ", pos: " << b.position << ")");

    // reset quality param for board
    ResetBoard();

    // set quality for enemy troop
    board.SetEnemyQuality(b);

    const Stats* enemy = NULL;
    bool attack = false;

    if(b.isArchers() && !b.isHandFighting())
    {
	enemy = GetEnemyMaxQuality(b.GetColor());
	AIMagicAction(b, a, enemy);
	attack = true;
    }
    else
    if(b.Modes(SP_BERSERKER))
    {
	std::vector<u16> pos;
	GetNearestTroops(b.GetPosition(), pos, NULL);
	const u16* move = Rand::Get(pos);
    
	if(move)
	{
	    a.AddedMoveAction(b, *move);
	    enemy = GetEnemyAbroadMaxQuality(*move, 0);
	    attack = true;
	}
    }
    else
    if(b.isHandFighting())
    {
	enemy = GetEnemyAbroadMaxQuality(b.GetPosition(), b.GetColor());
	AIMagicAction(b, a, enemy);
	attack = true;
    }
    else
    {
	// set quality position from enemy
	board.SetPositionQuality(b);

	// get passable quality positions
	std::vector<u16> positions;
	positions.reserve(30);
	GetPassableQualityPositions(b, positions);
	attack = true;

	if(positions.size())
	{
	    const u16 move = b.AIGetAttackPosition(positions);

	    if(b.isFly())
	    {
		enemy = GetEnemyAbroadMaxQuality(move, b.GetColor());
		AIMagicAction(b, a, enemy);
	    	a.AddedMoveAction(b, move);
		attack = true;
	    }
	    else
	    {
		std::vector<u16> path;
		GetPath(b, move, path);
		if(path.size())
		{
		    enemy = GetEnemyAbroadMaxQuality(path.back(), b.GetColor());
		    AIMagicAction(b, a, enemy);
	    	    a.AddedMoveAction(b, path);

		    // archers move and short attack only
		    attack = b.isArchers() ? false : true;
		}
	    }
	}
	else
	    enemy = GetEnemyAbroadMaxQuality(b);
    }

    if(enemy)
    {
	if(attack) a.AddedAttackAction(b, *enemy);
    }
    else
    if(IS_DEBUG(DBG_BATTLE, DBG_TRACE))
    {
	VERBOSE("Battle2::Arena::AITurn: " << "enemy is NULL");
	DumpBoard();
    }

    // end action
    a.AddedEndAction(b);
}

void Battle2::Arena::AIMagicAction(const Stats & b, Actions & a, const Stats* enemy)
{
    const HeroBase* hero = b.GetCommander();

    if(b.Modes(SP_BERSERKER) || !hero || hero->Modes(Heroes::SPELLCASTED) || !hero->HaveSpellBook()) return;

    const Army::army_t* my_army = b.GetArmy();
    const Army::army_t* enemy_army = GetArmy(GetOppositeColor(b.GetColor()));

    const Army::Troop* troop = NULL;
    const Stats* stats = NULL;

    // troop bad spell - clean
    if(b.Modes(IS_BAD_MAGIC))
    {
	if(isApplySpell(Spell::DISPEL, &b, *hero, a)) return;
	if(isApplySpell(Spell::CURE, &b, *hero, a)) return;
    }

    if(enemy)
    {
	// curse
	if(!enemy->Modes(SP_CURSE) && isApplySpell(Spell::CURSE, enemy, *hero, a)) return;
	// enemy good spell - clean
	if(enemy->Modes(IS_GOOD_MAGIC) && isApplySpell(Spell::DISPEL, enemy, *hero, a)) return;

	// up defense
	if(!b.Modes(SP_STEELSKIN) && !b.Modes(SP_STONESKIN) && isApplySpell(Spell::STEELSKIN, &b, *hero, a)) return;
	if(!b.Modes(SP_STONESKIN) && !b.Modes(SP_STEELSKIN) && isApplySpell(Spell::STONESKIN, &b, *hero, a)) return;
    }

    // my army blessing
    if(b.isArchers() || enemy)
    {
	if(enemy->troop.isDragons() && !b.Modes(SP_DRAGONSLAYER) && isApplySpell(Spell::DRAGONSLAYER, &b, *hero, a)) return;
	if(!b.Modes(SP_BLESS) && isApplySpell(Spell::BLESS, &b, *hero, a)) return;
	if(!b.Modes(SP_BLOODLUST) && isApplySpell(Spell::BLOODLUST, &b, *hero, a)) return;
	if(!b.Modes(SP_HASTE) && isApplySpell(Spell::HASTE, &b, *hero, a)) return;
    }

    // shield spell conditions
    if(NULL != enemy_army)
    {
	// find archers
	if(enemy_army->BattleArchersPresent() ||
	// or archers tower
	    (castle && castle->GetColor() != b.GetColor() && castle->isCastle()))
	{
	    if(!b.Modes(SP_SHIELD) && isApplySpell(Spell::SHIELD, &b, *hero, a)) return;
	}
    }

    // my army scan - clean
    if(NULL != my_army && NULL != (troop = my_army->BattleFindModes(IS_BAD_MAGIC)))
    {
	if(NULL != (stats = troop->GetBattleStats()))
	{
	    if(isApplySpell(Spell::DISPEL, stats, *hero, a)) return;
	    if(isApplySpell(Spell::CURE, stats, *hero, a)) return;
	}
    }

    // enemy army spell
    if(NULL != enemy_army)
    {
	// find mirror image or summon elem
	if(NULL != (troop = enemy_army->BattleFindModes(CAP_MIRRORIMAGE | CAP_SUMMONELEM)) && NULL != (stats = troop->GetBattleStats()))
	{
	    if(isApplySpell(Spell::ARROW, stats, *hero, a)) return;
	    if(isApplySpell(Spell::COLDRAY, stats, *hero, a)) return;
	    if(isApplySpell(Spell::FIREBALL, stats, *hero, a)) return;
	    if(isApplySpell(Spell::LIGHTNINGBOLT, stats, *hero, a)) return;
	}

	// find good magic
	if(NULL != (troop = enemy_army->BattleFindModes(IS_GOOD_MAGIC)) && NULL != (stats = troop->GetBattleStats()))
	{
	    // slow
	    if(stats->Modes(SP_HASTE) && isApplySpell(Spell::SLOW, stats, *hero, a)) return;
	    // curse
	    if(!stats->Modes(SP_CURSE) && isApplySpell(Spell::CURSE, stats, *hero, a)) return;
	    //
	    if(isApplySpell(Spell::DISPEL, stats, *hero, a)) return;
	}

	// check undead
	if(my_army->BattleUndeadTroopCount() < enemy_army->BattleUndeadTroopCount())
	{
	    if(isApplySpell(Spell::HOLYSHOUT, NULL, *hero, a)) return;
	    if(isApplySpell(Spell::HOLYWORD, NULL, *hero, a)) return;
	}

	// check alife
	if(my_army->BattleLifeTroopCount() < enemy_army->BattleLifeTroopCount())
	{
	    if(isApplySpell(Spell::DEATHRIPPLE, NULL, *hero, a)) return;
	    if(isApplySpell(Spell::DEATHWAVE, NULL, *hero, a)) return;
	}

	stats = enemy_army->BattleRandomTroop();

	if(isApplySpell(Spell::LIGHTNINGBOLT, stats, *hero, a)) return;
	if(isApplySpell(Spell::FIREBALL, stats, *hero, a)) return;
	if(isApplySpell(Spell::COLDRAY, stats, *hero, a)) return;
	if(isApplySpell(Spell::ARROW, stats, *hero, a)) return;
    }

/* FIX: Battle2::Arena: AIMagicAction:Damage Spell:
    Spell::FIREBLAST
    Spell::COLDRING
    Spell::CHAINLIGHTNING
    Spell::METEORSHOWER
*/

    if(isApplySpell(Spell::ARMAGEDDON, NULL, *hero, a)) return;
    if(isApplySpell(Spell::ELEMENTALSTORM, NULL, *hero, a)) return;
}

bool Battle2::isApplySpell(const Spell::spell_t spell, const Stats* b, const HeroBase & hero, Actions & a)
{
    switch(spell)
    {
        case Spell::CURE:	if(isApplySpell(Spell::MASSCURE, b, hero, a)) return true; break;
        case Spell::HASTE:	if(isApplySpell(Spell::MASSHASTE, b, hero, a)) return true; break;
        case Spell::SLOW:	if(isApplySpell(Spell::MASSSLOW, b, hero, a)) return true; break;
        case Spell::BLESS:	if(isApplySpell(Spell::MASSBLESS, b, hero, a)) return true; break;
        case Spell::CURSE:	if(isApplySpell(Spell::MASSCURSE, b, hero, a)) return true; break;
        case Spell::DISPEL:	if(isApplySpell(Spell::MASSDISPEL, b, hero, a)) return true; break;
        case Spell::SHIELD:	if(isApplySpell(Spell::MASSSHIELD, b, hero, a)) return true; break;

	default: break;
    }

    if(hero.HaveSpell(spell) && hero.HaveSpellPoints(Spell::CostManaPoints(spell, &hero)) && (!b || b->AllowApplySpell(spell, &hero)))
    {
	a.AddedCastAction(spell, (b ? b->GetPosition() : MAXU16));
	return true;
    }

    return false;
}

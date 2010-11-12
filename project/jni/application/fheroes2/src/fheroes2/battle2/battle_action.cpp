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
#include "settings.h"
#include "world.h"
#include "kingdom.h"
#include "battle_cell.h"
#include "battle_stats.h"
#include "battle_arena.h"
#include "battle_tower.h"
#include "battle_bridge.h"
#include "battle_catapult.h"
#include "battle_interface.h"
#include "remoteclient.h"

void Battle2::Actions::AddedSurrenderAction(void)
{
    Action action;
    action.SetID(MSG_BATTLE_SURRENDER);

    push_back(action);
}

void Battle2::Actions::AddedRetreatAction(void)
{
    Action action;
    action.SetID(MSG_BATTLE_RETREAT);

    push_back(action);
}

void Battle2::Actions::AddedCastAction(u8 spell, u16 dst)
{
    Action action;
    action.SetID(MSG_BATTLE_CAST);
    action.Push(spell);
    action.Push(dst);

    push_back(action);
}

void Battle2::Actions::AddedCastTeleportAction(u16 src, u16 dst)
{
    Action action;
    action.SetID(MSG_BATTLE_CAST);
    action.Push(static_cast<u8>(Spell::TELEPORT));
    action.Push(src);
    action.Push(dst);

    push_back(action);
}

void Battle2::Actions::AddedEndAction(const Stats & b)
{
    Action action;
    action.SetID(MSG_BATTLE_END_TURN);
    action.Push(b.id);

    push_back(action);
}

void Battle2::Actions::AddedSkipAction(const Stats & b, bool hard)
{
    Action action;
    action.SetID(MSG_BATTLE_SKIP);
    action.Push(b.id);
    action.Push(static_cast<u8>(hard));

    push_back(action);
}

void Battle2::Actions::AddedMoveAction(const Stats & b, u16 dst)
{
    Action action;
    action.SetID(MSG_BATTLE_MOVE);
    action.Push(b.id);
    action.Push(dst);
    action.Push(static_cast<u16>(0));

    push_back(action);
}

void Battle2::Actions::AddedMoveAction(const Stats & b, const std::vector<u16> & path)
{
    Action action;
    action.SetID(MSG_BATTLE_MOVE);
    action.Push(b.id);
    action.Push(path.back());
    action.Push(static_cast<u16>(path.size()));
    for(u16 ii = 0; ii < path.size(); ++ii) action.Push(path[ii]);

    push_back(action);
}

void Battle2::Actions::AddedAttackAction(const Stats & a, const Stats & d)
{
    Action action;
    action.SetID(MSG_BATTLE_ATTACK);
    action.Push(a.id);
    action.Push(d.id);

    push_back(action);
}

void Battle2::Actions::AddedMoraleAction(const Stats & b, u8 state)
{
    Action action;
    action.SetID(MSG_BATTLE_MORALE);
    action.Push(b.id);
    action.Push(state);

    push_back(action);
}

void Battle2::Arena::BattleProcess(Stats & attacker, Stats & defender)
{
    std::vector<TargetInfo> targets;

    GetTargetsForDamage(attacker, defender, targets);
    if(interface) interface->RedrawActionAttackPart1(attacker, defender, targets);

    TargetsApplyDamage(attacker, defender, targets);
    if(interface) interface->RedrawActionAttackPart2(attacker, targets);

    const u8 spell = attacker.GetSpellMagic();

    // magic attack
    if(defender.isValid() && Spell::NONE != spell)
    {
	const std::string name(attacker.GetName());

	GetTargetsForSpells(attacker.GetCommander(), spell, defender.GetPosition(), targets);
	if(interface) interface->RedrawActionSpellCastPart1(spell, defender.GetPosition(), name, targets);

	// magic attack not depends from hero
	TargetsApplySpell(NULL, spell, targets);
	if(interface) interface->RedrawActionSpellCastPart2(spell, targets);
	if(interface) interface->RedrawActionMonsterSpellCastStatus(attacker, targets.front());

#ifdef WITH_NET
	if(Game::REMOTE == army1.GetControl()) FH2RemoteClient::SendBattleSpell(army1.GetColor(), spell, attacker.GetColor(), targets);
	if(Game::REMOTE == army2.GetControl()) FH2RemoteClient::SendBattleSpell(army2.GetColor(), spell, attacker.GetColor(), targets);
#endif
    }

    attacker.PostAttackAction(defender);
}

void Battle2::Arena::ApplyAction(Action & action)
{
    switch(action.GetID())
    {
	case MSG_BATTLE_CAST:		ApplyActionSpellCast(action); break;
	case MSG_BATTLE_ATTACK:		ApplyActionAttack(action); break;
	case MSG_BATTLE_MOVE:		ApplyActionMove(action);   break;
	case MSG_BATTLE_SKIP:		ApplyActionSkip(action);   break;
	case MSG_BATTLE_END_TURN:	ApplyActionEnd(action);    break;
	case MSG_BATTLE_MORALE:		ApplyActionMorale(action); break;

	case MSG_BATTLE_TOWER:		ApplyActionTower(action); break;
	case MSG_BATTLE_CATAPULT:	ApplyActionCatapult(action); break;

	case MSG_BATTLE_RETREAT:	ApplyActionRetreat(action); break;
	case MSG_BATTLE_SURRENDER:	ApplyActionSurrender(action); break;

	default: break;
    }

#ifdef WITH_NET
    switch(action.GetID())
    {
	case MSG_BATTLE_MOVE:
	case MSG_BATTLE_SKIP:
	case MSG_BATTLE_END_TURN:
	case MSG_BATTLE_MORALE:
	case MSG_BATTLE_TOWER:
	case MSG_BATTLE_CATAPULT:
	    if(Network::isRemoteClient())
    	    {
		if(Game::REMOTE == army1.GetControl()) FH2RemoteClient::SendBattleAction(army1.GetColor(), action);
    		if(Game::REMOTE == army2.GetControl()) FH2RemoteClient::SendBattleAction(army2.GetColor(), action);
	    }
	    break;

	default: break;
    }
#endif
}

void Battle2::Arena::ApplyActionSpellCast(Action & action)
{
    u8 byte8;
    u16 dst;

    action.Pop(byte8);
    action.Pop(dst);

    const Spell::spell_t spell = Spell::FromInt(byte8);
    if(current_commander && !current_commander->Modes(Heroes::SPELLCASTED) &&
	current_commander->HaveSpell(spell) &&
	Spell::isCombat(spell) && current_commander->HaveSpellPoints(Spell::CostManaPoints(spell, current_commander)))
    {
	const u8 color = current_commander->GetColor();

	DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::ApplyActionSpellCast: " << \
	    current_commander->GetName() << ", color: " << Color::String(color) << ", spell: " << Spell::GetName(spell) << ", dst: " << dst);

	// uniq spells action
	switch(spell)
	{
	    case Spell::TELEPORT:
	    {
		u16 src = dst;
		action.Pop(dst);
		SpellActionTeleport(src, dst);
#ifdef WITH_NET
		if(Game::REMOTE == army1.GetControl()) FH2RemoteClient::SendBattleTeleportSpell(army1.GetColor(), src, dst);
		if(Game::REMOTE == army2.GetControl()) FH2RemoteClient::SendBattleTeleportSpell(army2.GetColor(), src, dst);
#endif
		break;
	    }

	    case Spell::EARTHQUAKE:
		SpellActionEarthQuake();
		break;

	    default:
	    {
		std::vector<TargetInfo> targets;

		GetTargetsForSpells(current_commander, spell, dst, targets);
		if(interface) interface->RedrawActionSpellCastPart1(spell, dst, current_commander->GetName(), targets);

		TargetsApplySpell(current_commander, spell, targets);
		if(interface) interface->RedrawActionSpellCastPart2(spell, targets);
#ifdef WITH_NET
		if(Game::REMOTE == army1.GetControl()) FH2RemoteClient::SendBattleSpell(army1.GetColor(), spell, current_commander->GetColor(), targets);
		if(Game::REMOTE == army2.GetControl()) FH2RemoteClient::SendBattleSpell(army2.GetColor(), spell, current_commander->GetColor(), targets);
#endif
	    }
	    break;
	}

	current_commander->SetModes(Heroes::SPELLCASTED);
	current_commander->TakeSpellPoints(Spell::CostManaPoints(spell, current_commander));
    }
    else
	DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::ApplyActionSpellCast: " << "incorrect param: ");
}

void Battle2::Arena::ApplyActionAttack(Action & action)
{
    u16 id1, id2;

    action.Pop(id1);
    action.Pop(id2);

    Battle2::Stats* b1 = GetTroopID(id1);
    Battle2::Stats* b2 = GetTroopID(id2);

    if(b1 && b1->isValid() &&
	b2 && b2->isValid() &&
	(b1->GetColor() != b2->GetColor() || b2->Modes(SP_HYPNOTIZE)))
    {
	DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::ApplyActionAttack: " << \
	    b1->GetName() << "(color: " << Color::String(b1->GetColor()) << ", pos: " << b1->position << ") to " << \
	    b2->GetName() << "(color: " << Color::String(b2->GetColor()) << ", pos: " << b2->position << ")");

	const bool handfighting = Stats::isHandFighting(*b1, *b2);
	// check position
	if(b1->isArchers() || handfighting)
	{
	    b1->UpdateDirection(*b2);
	    b2->UpdateDirection(*b1);

	    // attack
	    BattleProcess(*b1, *b2);

	    if(b2->isValid())
	    {
		// defense answer
		if(handfighting && !b1->isHideAttack() && b2->AllowResponse())
		{
		    BattleProcess(*b2, *b1);
		    b2->SetResponse();
		}

		if(b2->Modes(SP_BLIND)) b2->ResetBlind();

		// twice attack
		if(b1->isValid() && b1->isTwiceAttack() && !b1->Modes(IS_PARALYZE_MAGIC))
		    BattleProcess(*b1, *b2);
	    }
	}
	else
	{
	    DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::ApplyActionAttack: " << "incorrect position: " << \
		b1->GetName() << "(color: " << Color::String(b1->GetColor()) << ", pos: " << b1->position << ") to " << \
		b2->GetName() << "(color: " << Color::String(b2->GetColor()) << ", pos: " << b2->position << ")");
	}

	if(!Settings::Get().ExtBattleTroopDirection())
	{
	    b1->UpdateDirection();
	    b2->UpdateDirection();
	}
    }
    else
    	DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::ApplyActionAttack: " << "incorrect param: " << "id1: " << id1 << ", id2: " << id2);
}

void Battle2::Arena::ApplyActionMove(Action & action)
{
    u16 id, dst, size;

    action.Pop(id);
    action.Pop(dst);
    action.Pop(size);

    Battle2::Stats* b = GetTroopID(id);
    Cell* cell = GetCell(dst);

    if(b && b->isValid() &&
	cell && cell->isPassable(*b, false))
    {
	b->UpdateDirection(*cell);

	// change pos to tail
	if(b->isWide() && !cell->isPassable(*b, true))
	{
	    dst = Board::GetIndexDirection(dst, b->isReflect() ? LEFT : RIGHT);
	    cell = GetCell(dst);
	}

	// force check fly
	if(b->troop.isFly())
	{
	    if(interface) interface->RedrawActionFly(*b, dst);
	}
	else
	{
	    std::vector<u16> path;

	    // check path
	    if(b->GetControl() != Game::AI || 0 == size)
	    {
		GetPath(*b, dst, path);
	    }
	    else
	    {
		path.resize(size, 0);
		for(u16 ii = 0; ii < size; ++ii) action.Pop(path[ii]);
	    }

	    if(path.empty())
	    {
		DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::ApplyActionMove: " << "ERROR empty path, " << \
		    b->GetName() << "(color: " << Color::String(b->GetColor()) << ", pos: " << b->GetPosition() << ") to " << "dst: " << dst);
		return;
	    }

	    if(interface) interface->RedrawActionMove(*b, path);
	    else
    	    if(bridge)
    	    {
        	if(!bridge->isDown() && bridge->NeedDown(*b, dst))
        	    bridge->SetDown(true);
        	else
        	if(bridge->isDown() && bridge->AllowUp())
    		    bridge->SetDown(false);
	    }
	}

	b->position = dst;
	b->UpdateDirection();

	DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::ApplyActionMove: " << \
	    b->GetName() << "(color: " << Color::String(b->GetColor()) << ", pos: " << b->GetPosition() << ") to " << "dst: " << dst);
    }
    else
    {
    	DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::ApplyActionMove: " << "incorrect param: " << "id: " << id << ", dst: " << dst);
    }
}

void Battle2::Arena::ApplyActionSkip(Action & action)
{
    u16 id;
    u8 hard;
    action.Pop(id);
    action.Pop(hard);

    Battle2::Stats* battle = GetTroopID(id);
    if(battle && battle->isValid() && !battle->Modes(TR_MOVED))
    {
	if(hard)
	{
	    battle->SetModes(TR_HARDSKIP);
	    battle->SetModes(TR_SKIPMOVE);
	    battle->SetModes(TR_MOVED);
	}
	else
	    battle->SetModes(battle->Modes(TR_SKIPMOVE) ? TR_MOVED : TR_SKIPMOVE);

	if(interface) interface->RedrawActionSkipStatus(*battle);

	DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::ApplyActionSkip: " << \
	    battle->GetName() << "(color: " << Color::String(battle->GetColor()) << ", pos: " << battle->position << ")");
    }
    else
	DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::ApplyActionSkip: " << "incorrect param: " << "id: " << id);
}

void Battle2::Arena::ApplyActionEnd(Action & action)
{
    u16 id;

    action.Pop(id);

    Battle2::Stats* battle = GetTroopID(id);

    if(battle && !battle->Modes(TR_MOVED))
    {
	battle->SetModes(TR_MOVED);

	if(battle->Modes(TR_SKIPMOVE) && interface) interface->RedrawActionSkipStatus(*battle);

	DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::ApplyActionEnd: " << \
	    battle->GetName() << "(color: " << Color::String(battle->GetColor()) << ", pos: " << battle->position << ")");
    }
    else
	DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::ApplyActionEnd: " << "incorrect param: " << "id: " << id);
}

void Battle2::Arena::ApplyActionMorale(Action & action)
{
    u16 id;
    u8  morale;

    action.Pop(id);
    action.Pop(morale);

    Battle2::Stats* b = GetTroopID(id);

    if(b && b->isValid())
    {
	// good morale
	if(morale && b->Modes(TR_MOVED) && b->Modes(MORALE_GOOD))
	{
	    b->ResetModes(TR_MOVED);
    	    b->ResetModes(MORALE_GOOD);
        }
	// bad morale
        else
	if(!morale && !b->Modes(TR_MOVED) && b->Modes(MORALE_BAD))
        {
	    b->SetModes(TR_MOVED);
	    b->ResetModes(MORALE_BAD);
	}

	if(interface) interface->RedrawActionMorale(*b, morale);

	DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::ApplyActionMorale: " << (morale ? "good" : "bad"));
    }
    else
	DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::ApplyActionMorale: " << "incorrect param: " << "id: " << id);
}

void Battle2::Arena::ApplyActionRetreat(Action & action)
{
    if(!result_game || !current_commander) return;

    const u8 color = current_commander->GetColor();

    if(CanRetreatOpponent(color))
    {
	if(army1.GetColor() == color && result_game)
    	{
    	    result_game->army1 = RESULT_RETREAT;
    	}
    	else
    	if(army2.GetColor() == color && result_game)
    	{
    	    result_game->army2 = RESULT_RETREAT;
    	}
	DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::ApplyActionRetreat: " << "color: " << Color::String(color));
    }
    else
	DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::ApplyActionRetreat: " << "incorrect param: ");
}

void Battle2::Arena::ApplyActionSurrender(Action & action)
{
    if(!result_game || !current_commander) return;

    const u8 color = current_commander->GetColor();

    if(CanSurrenderOpponent(color))
    {
	Resource::funds_t cost;

    	if(army1.GetColor() == color)
		cost.gold = army1.GetSurrenderCost();
    	else
    	if(army2.GetColor() == color)
		cost.gold = army2.GetSurrenderCost();
        
    	if(world.GetKingdom(color).AllowPayment(cost))
    	{
	    if(army1.GetColor() == color)
    	    {
		    result_game->army1 = RESULT_SURRENDER;
		    world.GetKingdom(color).OddFundsResource(cost);
		    world.GetKingdom(army2.GetColor()).AddFundsResource(cost);
	    }
	    else
	    if(army2.GetColor() == color)
	    {
		    result_game->army2 = RESULT_SURRENDER;
		    world.GetKingdom(color).OddFundsResource(cost);
		    world.GetKingdom(army1.GetColor()).AddFundsResource(cost);
	    }
	    DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::ApplyActionSurrender: " << "color: " << Color::String(color));
    	}
    }
    else
	DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::ApplyActionSurrender: " << "incorrect param: ");
}

void Battle2::Arena::TargetsApplyDamage(Stats & attacker, Stats & defender, std::vector<TargetInfo> & targets)
{
    std::vector<TargetInfo>::iterator it = targets.begin();

    for(; it != targets.end(); ++it)
    {
	TargetInfo & target = *it;
	if(target.defender) target.killed = target.defender->ApplyDamage(attacker, target.damage);
    }
}

void Battle2::Arena::GetTargetsForDamage(Stats & attacker, Stats & defender, std::vector<TargetInfo> & targets)
{
    if(targets.size()) targets.clear();
    targets.reserve(8);

    Stats* enemy = NULL;
    Cell* cell = NULL;
    TargetInfo res;

    // first target
    res.defender = &defender;
    res.damage = attacker.GetDamage(defender);
    targets.push_back(res);

    // long distance attack
    if(attacker.isDoubleCellAttack())
    {
        const direction_t dir = Board::GetDirection(attacker.position, defender.position);
        if((!defender.isWide() || 0 == ((RIGHT | LEFT) & dir)))
	{
	    if(NULL != (cell = GetCell(defender.position, dir)) &&
		NULL != (enemy = GetTroopBoard(cell->index)) && enemy != &defender)
    	    {
		res.defender = enemy;
		res.damage = attacker.GetDamage(*enemy);
		targets.push_back(res);
	    }
        }
    }
    else
    // around hydra
    if(attacker.troop() == Monster::HYDRA)
    {
	std::vector<Stats*> v;
	v.reserve(8);
	for(direction_t dir = TOP_LEFT; dir < CENTER; ++dir)
	{
	    if(NULL != (cell = GetCell(attacker.position, dir)) &&
		NULL != (enemy = GetTroopBoard(cell->index)) && enemy != &defender && enemy->GetColor() != attacker.GetColor())
        	    v.push_back(enemy);

	    if(NULL != (cell = GetCell(attacker.GetTailIndex(), dir)) &&
		NULL != (enemy = GetTroopBoard(cell->index)) && enemy != &defender && enemy->GetColor() != attacker.GetColor())
        	    v.push_back(enemy);
	}
	std::unique(v.begin(), v.end());
	for(u8 ii = 0; ii < v.size(); ++ii)
    	{
	    enemy = v[ii];
	    res.defender = enemy;
	    res.damage = attacker.GetDamage(*enemy);
	    targets.push_back(res);
	}
    }
    else
    // lich cloud damages
    if((attacker.troop() == Monster::LICH ||
	attacker.troop() == Monster::POWER_LICH) && !attacker.isHandFighting())
    {
	for(direction_t dir = TOP_LEFT; dir < CENTER; ++dir)
	{
	    if(NULL != (cell = GetCell(defender.position, dir)) &&
		NULL != (enemy = GetTroopBoard(cell->index)) && enemy != &defender)
    	    {
		res.defender = enemy;
		res.damage = attacker.GetDamage(*enemy);
		targets.push_back(res);
	    }
	}
    }

#ifdef WITH_NET
    if(Game::REMOTE == army1.GetControl()) FH2RemoteClient::SendBattleAttack(army1.GetColor(), attacker.GetID(), targets);
    if(Game::REMOTE == army2.GetControl()) FH2RemoteClient::SendBattleAttack(army2.GetColor(), attacker.GetID(), targets);
#endif
}

void Battle2::Arena::TargetsApplySpell(const HeroBase* hero, const u8 spell, std::vector<TargetInfo> & targets)
{
    DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::TargetsApplySpell: " << "targets: " << targets.size());

    if(Spell::isSummon(spell))
    {
        SpellActionSummonElemental(hero, spell);
    }
    else
    {
	std::vector<TargetInfo>::iterator it = targets.begin();

	for(; it != targets.end(); ++it)
	{
	    TargetInfo & target = *it;
	    if(target.defender) target.defender->ApplySpell(spell, hero, target);
	}
    }
}

void Battle2::Arena::GetTargetsForSpells(const HeroBase* hero, const u8 spell, const u16 dst, std::vector<TargetInfo> & targets)
{
    if(targets.size()) targets.clear();
    targets.reserve(8);

    TargetInfo res;
    Stats* target = GetTroopBoard(dst);

    // from spells
    switch(spell)
    {
	case Spell::CHAINLIGHTNING:
	case Spell::COLDRING:
	    // skip center
	    target = NULL;
	    break;

	default: break;
    }

    // first target
    if(target && target->AllowApplySpell(spell, hero))
    {
	res.defender = target;
	targets.push_back(res);
    }

    // resurrect spell? get target from graveyard
    if(NULL == target && isAllowResurrectFromGraveyard(spell, dst))
    {
        target = GetTroopID(graveyard.GetLastTroopIDFromCell(dst));

	if(target && target->AllowApplySpell(spell, hero))
	{
	    res.defender = target;
	    targets.push_back(res);
	}
    }
    else
    // check other spells
    switch(spell)
    {
	case Spell::CHAINLIGHTNING:
        {
	    std::vector<u16> trgts;
	    std::vector<u16> reslt;
	    std::vector<u16>::iterator it1, it2;
	    u16 index = dst;
	    trgts.push_back(index);

	    // find targets
	    for(u8 ii = 0; ii < 3; ++ii)
	    {
		GetNearestTroops(index, reslt, &trgts);
		if(reslt.empty()) break;
		index = reslt.size() > 1 ? *Rand::Get(reslt) : reslt.front();
		trgts.push_back(index);
		reslt.clear();
	    }

	    // save targets
	    it1 = trgts.begin();
	    it2 = trgts.end();
	    for(; it1 != it2; ++it1)
	    {
		Stats* target = GetTroopBoard(*it1);
		if(target)
		{
		    res.defender = target;
		    // store temp priority for calculate damage
		    res.damage = std::distance(trgts.begin(), it1);
		    targets.push_back(res);
		}
	    }
	}
	break;

	// check abroads
	case Spell::FIREBALL:
	case Spell::METEORSHOWER:
	case Spell::COLDRING:
	case Spell::FIREBLAST:
	{
	    std::vector<u16> positions;
	    u8 radius = (Spell::FIREBLAST == spell ? 2 : 1);
	    board.GetAbroadPositions(dst, radius, positions);
	    std::vector<u16>::const_iterator it1 = positions.begin();
	    std::vector<u16>::const_iterator it2 = positions.end();
            for(; it1 != it2; ++it1)
            {
		Stats* target = GetTroopBoard(*it1);
		if(target && target->AllowApplySpell(spell, hero))
		{
		    res.defender = target;
		    targets.push_back(res);
		}
	    }
	}
	break;

	// check all troops
	case Spell::DEATHRIPPLE:
	case Spell::DEATHWAVE:
	case Spell::ELEMENTALSTORM:
	case Spell::HOLYWORD:
	case Spell::HOLYSHOUT:
	case Spell::ARMAGEDDON:
	case Spell::MASSBLESS:
	case Spell::MASSCURE:
	case Spell::MASSCURSE:
	case Spell::MASSDISPEL:
	case Spell::MASSHASTE:
	case Spell::MASSSHIELD:
	case Spell::MASSSLOW:
	{
	    Board::const_iterator it1 = board.begin();
	    Board::const_iterator it2 = board.end();
            for(; it1 != it2; ++it1)
            {
		Stats* target = GetTroopBoard((*it1).index);
		if(target && target->GetPosition() != dst && target->AllowApplySpell(spell, hero))
		{
		    res.defender = target;
		    targets.push_back(res);
		}
	    }
	}
        break;

	default: break;
    }

    // remove resistent magic troop
    std::vector<TargetInfo>::iterator it = targets.begin();
    while(it != targets.end())
    {
	const u8 resist = (*it).defender->GetMagicResist(spell, hero ? hero->GetPower() : 0);

	if(0 < resist && 100 > resist && resist >= Rand::Get(1, 100))
	{
	    if(interface) interface->RedrawActionResistSpell(*(*it).defender);
	
	    // erase(it)
	    if(it + 1 != targets.end()) std::swap(*it, targets.back());
	    targets.pop_back();
	}
	else ++it;
    }
}

void Battle2::Arena::ApplyActionTower(Action & action)
{
    u8 type;
    u16 id;

    action.Pop(type);
    action.Pop(id);

    Battle2::Stats* b2 = GetTroopID(id);
    Tower* tower = GetTower(type);

    if(b2 && b2->isValid() && tower)
    {
	DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::ApplyActionTower: " << "tower: " << static_cast<int>(type) << \
		", attack pos: " << b2->position << ", troop: " << b2->GetName());

	Stats* b1 = tower->GetBattleStats();
	TargetInfo target;
	target.defender = b2;
	target.damage = b1->GetDamage(*b2);

	if(interface) interface->RedrawActionTowerPart1(*tower, *b2);
	target.killed = b2->ApplyDamage(*b1, target.damage);
	if(interface) interface->RedrawActionTowerPart2(*tower, target);
    }
    else
	DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::ApplyActionTower: " << "incorrect param: " << "tower: " << static_cast<int>(type) << ", id: " << id);
}

void Battle2::Arena::ApplyActionCatapult(Action & action)
{
    if(catapult)
    {
	u8 shots, target, damage;

	action.Pop(shots);

	while(shots--)
	{
	    action.Pop(target);
	    action.Pop(damage);

	    if(target)
	    {
		if(interface) interface->RedrawActionCatapult(target);
		SetCastleTargetValue(target, GetCastleTargetValue(target) - damage);
		DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::ApplyActionCatapult: " << "target: " << static_cast<int>(target));
	    }
	}
    }
    else
	DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::ApplyActionCatapult: " << "incorrect param: ");
}

void Battle2::Arena::SpellActionSummonElemental(const HeroBase* hero, u8 type)
{
    const u16 pos = GetFreePositionNearHero(hero->GetColor());
    Army::army_t* army = GetArmy(hero->GetColor());

    if(army && 0 != pos)
    {
        const Army::Troop* other = army->BattleFindModes(CAP_SUMMONELEM);
        bool affect = true;
        if(other) switch(type)
        {
            case Spell::SUMMONEELEMENT: if(other->GetID() != Monster::EARTH_ELEMENT) affect = false; break;
            case Spell::SUMMONAELEMENT: if(other->GetID() != Monster::AIR_ELEMENT) affect = false; break;
            case Spell::SUMMONFELEMENT: if(other->GetID() != Monster::FIRE_ELEMENT) affect = false; break;
            case Spell::SUMMONWELEMENT: if(other->GetID() != Monster::WATER_ELEMENT) affect = false; break;
            default: break;
        }

	if(!affect)
	{
	    DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::SpellActionSummonElemental: " << "incorrect elemental");
	    return;
	}

	Monster::monster_t mons = Monster::UNKNOWN;
        switch(type)
        {
            case Spell::SUMMONEELEMENT: mons = Monster::EARTH_ELEMENT; break;
            case Spell::SUMMONAELEMENT: mons = Monster::AIR_ELEMENT; break;
            case Spell::SUMMONFELEMENT: mons = Monster::FIRE_ELEMENT; break;
            case Spell::SUMMONWELEMENT: mons = Monster::WATER_ELEMENT; break;
            default: break;
        }

	DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::SpellActionSummonElemental: " << Monster::GetName(mons) << ", position: " << pos);
	u16 count = Spell::GetExtraValue(Spell::FromInt(type)) * hero->GetPower();
	if(hero->HasArtifact(Artifact::BOOK_ELEMENTS)) count *= 2;

        Stats* elem = army->BattleNewTroop(mons, count).GetBattleStats();
        elem->position = pos;
        elem->arena = this;
        elem->SetReflection(hero == army2.GetCommander());
        elem->SetModes(CAP_SUMMONELEM);
        if(interface)
	{
	    elem->InitContours();
	    interface->RedrawActionSummonElementalSpell(*elem);
	}
    }
    else
    {
    	if(interface) interface->SetStatus(_("Summon Elemental spell failed!"), true);
	DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::SpellActionSummonElemental: " << "incorrect param");
    }
}

void Battle2::Arena::SpellActionTeleport(u16 src, u16 dst)
{
    Stats* b = GetTroopBoard(src);
    Cell* cell = GetCell(dst);

    if(b)
    {
        if(b->isWide() && !cell->isPassable(*b, true))
	{
	    dst = Board::GetIndexDirection(dst, b->isReflect() ? LEFT : RIGHT);
	}
	if(interface) interface->RedrawActionTeleportSpell(*b, dst);
	b->position = dst;
	b->UpdateDirection();
    }
}

void Battle2::Arena::SpellActionEarthQuake(void)
{
    std::vector<u8> targets;

    // check walls
    if(0 != board[8].object)  targets.push_back(CAT_WALL1);
    if(0 != board[29].object) targets.push_back(CAT_WALL2);
    if(0 != board[73].object) targets.push_back(CAT_WALL3);
    if(0 != board[96].object) targets.push_back(CAT_WALL4);

    // check right/left towers
    if(towers[0] && towers[0]->isValid()) targets.push_back(CAT_TOWER1);
    if(towers[2] && towers[2]->isValid()) targets.push_back(CAT_TOWER2);

    if(interface) interface->RedrawActionEarthQuakeSpell(targets);

    // FIXME: Arena::SpellActionEarthQuake: check hero spell power

    // apply random damage
    if(0 != board[8].object)  board[8].object = Rand::Get(board[8].object);
    if(0 != board[29].object) board[29].object = Rand::Get(board[29].object);
    if(0 != board[73].object) board[73].object = Rand::Get(board[73].object);
    if(0 != board[96].object) board[96].object = Rand::Get(board[96].object);

    if(towers[0] && towers[0]->isValid() && Rand::Get(1)) towers[0]->SetDestroy();
    if(towers[2] && towers[2]->isValid() && Rand::Get(1)) towers[2]->SetDestroy();

#ifdef WITH_NET
    if(Game::REMOTE == army1.GetControl()) FH2RemoteClient::SendBattleEarthQuakeSpell(army1.GetColor(), targets);
    if(Game::REMOTE == army2.GetControl()) FH2RemoteClient::SendBattleEarthQuakeSpell(army2.GetColor(), targets);

    if(Game::REMOTE == army1.GetControl()) FH2RemoteClient::SendBattleBoard(army1.GetColor(), *this);
    if(Game::REMOTE == army2.GetControl()) FH2RemoteClient::SendBattleBoard(army2.GetColor(), *this);
#endif
}

void Battle2::Arena::SpellActionMirrorImage(Stats & b)
{
    std::vector<u16> v;
    board.GetAbroadPositions(b.position, 4, v);

    std::vector<u16>::const_iterator it1 = v.begin();
    std::vector<u16>::const_iterator it2 = v.end();

    for(; it1 != it2; ++it1)
    {
    	const Cell* cell = GetCell(*it1);
    	if(cell && cell->isPassable(b, true))
	{
	    if(b.isWide() && (b.position + 1 == *it1))
		continue;
	    else
		break;
	}
    }

    if(it1 != v.end())
    {
    	DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::SpellCreateMirrorImage: " << "set position: " << *it1);
	if(interface) interface->RedrawActionMirrorImageSpell(b, *it1);

    	Stats* image = b.GetArmy()->BattleNewTroop(b.troop(), b.count).GetBattleStats();

    	b.mirror = image;
    	image->position = *it1;
    	image->arena = this;
    	image->mirror = &b;
    	image->SetReflection(b.reflect);
    	image->SetModes(CAP_MIRRORIMAGE);
    	if(interface) image->InitContours();
    	b.SetModes(CAP_MIRROROWNER);
    }
    else
    {
    	if(interface) interface->SetStatus(_("Mirror Image spell failed!"), true);
        DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::SpellCreateMirrorImage: " << "new position not found!");
    }
}


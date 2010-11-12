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
#include "settings.h"
#include "payment.h"
#include "world.h"
#include "kingdom.h"
#include "maps_tiles.h"
#include "text.h"
#include "luck.h"
#include "morale.h"
#include "speed.h"
#include "castle.h"
#include "heroes.h"
#include "battle_stats.h"
#include "tools.h"
#include "army.h"

std::vector<Army::Troop>::iterator MaxElement(std::vector<Army::Troop>::iterator first, std::vector<Army::Troop>::iterator last, bool (*pf)(const Army::Troop &, const Army::Troop &))
{
    while(first != last) if(Army::isValidTroop(*first)) break; else ++first;
    std::vector<Army::Troop>::iterator largest = first;
    if(first == last) return last;
    while(++first != last) if(Army::isValidTroop(*first) && pf(*first, *largest)) largest = first;
    return largest;
}

std::vector<Army::Troop>::const_iterator MaxElement(std::vector<Army::Troop>::const_iterator first, std::vector<Army::Troop>::const_iterator last, bool (*pf)(const Army::Troop &, const Army::Troop &))
{
    while(first != last) if(Army::isValidTroop(*first)) break; else ++first;
    std::vector<Army::Troop>::const_iterator largest = first;
    if(first == last) return last;
    while(++first != last) if(Army::isValidTroop(*first) && pf(*first, *largest)) largest = first;
    return largest;
}

std::vector<Army::Troop>::iterator MinElement(std::vector<Army::Troop>::iterator first, std::vector<Army::Troop>::iterator last, bool (*pf)(const Army::Troop &, const Army::Troop &))
{
    while(first != last) if(Army::isValidTroop(*first)) break; else ++first;
    std::vector<Army::Troop>::iterator lowest = first;
    if(first == last) return last;
    while(++first != last) if(Army::isValidTroop(*first) && pf(*first, *lowest)) lowest = first;
    return lowest;
}

std::vector<Army::Troop>::const_iterator MinElement(std::vector<Army::Troop>::const_iterator first, std::vector<Army::Troop>::const_iterator last, bool (*pf)(const Army::Troop &, const Army::Troop &))
{
    while(first != last) if(Army::isValidTroop(*first)) break; else ++first;
    std::vector<Army::Troop>::const_iterator lowest = first;
    if(first == last) return last;
    while(++first != last) if(Army::isValidTroop(*first) && pf(*first, *lowest)) lowest = first;
    return lowest;
}

const char* Army::String(u32 size)
{
    const char* str_size[] = { _("army|Few"), _("army|Several"), _("army|Pack"), _("army|Lots"), _("army|Horde"), _("army|Throng"), _("army|Swarm"), _("army|Zounds"), _("army|Legion") };

    switch(GetSize(size))
    {
	case FEW:	return str_size[0];
        case SEVERAL:	return str_size[1];
        case PACK:	return str_size[2];
        case LOTS:	return str_size[3];
        case HORDE:	return str_size[4];
        case THRONG:	return str_size[5];
        case SWARM:	return str_size[6];
        case ZOUNDS:	return str_size[7];
        case LEGION:	return str_size[8];
    }

    return str_size[0];
}

Army::armysize_t Army::GetSize(u32 count)
{
    if(LEGION <= count)		return LEGION;
    else
    if(ZOUNDS <= count)		return ZOUNDS;
    else
    if(SWARM <= count)		return SWARM;
    else
    if(THRONG <= count)		return THRONG;
    else
    if(HORDE <= count)		return HORDE;
    else
    if(LOTS <= count)		return LOTS;
    else
    if(PACK <= count)		return PACK;
    else
    if(SEVERAL <= count)	return SEVERAL;

    return FEW;
}

Army::army_t::army_t(HeroBase* s) : army(ARMYMAXTROOPS), commander(s), combat_format(FORMAT_SPREAD), color(Color::GRAY)
{
    std::vector<Troop>::iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    for(; it1 != it2; ++it1) (*it1).army = this;
}

Army::army_t::army_t(const army_t & a) : army(ARMYMAXTROOPS), commander(NULL), combat_format(FORMAT_SPREAD), color(Color::GRAY)
{
    std::vector<Troop>::iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    for(; it1 != it2; ++it1) (*it1).army = this;

    Import(a.army);
}

void Army::army_t::FromGuardian(const Maps::Tiles & t)
{
    Reset();

    if(MP2::isCaptureObject(t.GetObject()))
	color = world.ColorCapturedObject(t.GetIndex());

    u8 obj = t.GetObject();

    if(MP2::OBJ_HEROES == obj)
    {
	const Heroes* hero = world.GetHeroes(t.GetIndex());
	if(hero) obj = hero->GetUnderObject();
    }

    switch(obj)
    {
	case MP2::OBJ_PYRAMID:
            army[0].Set(Monster::ROYAL_MUMMY, 10);
            army[1].Set(Monster::VAMPIRE_LORD, 10);
            army[2].Set(Monster::ROYAL_MUMMY, 10);
            army[3].Set(Monster::VAMPIRE_LORD, 10);
            army[4].Set(Monster::ROYAL_MUMMY, 10);
	    break;

	case MP2::OBJ_GRAVEYARD:
	    army[0].Set(Monster::MUTANT_ZOMBIE, 100);
	    ArrangeForBattle();
	    break;

	case MP2::OBJ_SHIPWRECK:
	    army[0].Set(Monster::GHOST, t.GetQuantity2());
	    ArrangeForBattle();
	    break;

	case MP2::OBJ_DERELICTSHIP:
	    army[0].Set(Monster::SKELETON, 200);
	    ArrangeForBattle();
	    break;

	case MP2::OBJ_ARTIFACT:
	    switch(t.GetQuantity2())
	    {
		case 6:	army[0].Set(Monster::ROGUE, 50); break;	
		case 7:	army[0].Set(Monster::GENIE, 1); break;	
		case 8:	army[0].Set(Monster::PALADIN, 1); break;	
		case 9:	army[0].Set(Monster::CYCLOPS, 1); break;	
		case 10:army[0].Set(Monster::PHOENIX, 1); break;	
		case 11:army[0].Set(Monster::GREEN_DRAGON, 1); break;	
		case 12:army[0].Set(Monster::TITAN, 1); break;	
		case 13:army[0].Set(Monster::BONE_DRAGON, 1); break;
		default: break;	
	    }
	    ArrangeForBattle();
	    break;

	//case MP2::OBJ_ABANDONEDMINE:
	//    army[0] = Troop(t);
	//    ArrangeForBattle();
	//    break;

	case MP2::OBJ_CITYDEAD:
            army[0].Set(Monster::ZOMBIE, 20);
            army[1].Set(Monster::VAMPIRE_LORD, 5);
            army[2].Set(Monster::POWER_LICH, 5);
            army[3].Set(Monster::VAMPIRE_LORD, 5);
            army[4].Set(Monster::ZOMBIE, 20);
	    break;

	case MP2::OBJ_TROLLBRIDGE:
            army[0].Set(Monster::TROLL, 4);
            army[1].Set(Monster::WAR_TROLL, 4);
            army[2].Set(Monster::TROLL, 4);
            army[3].Set(Monster::WAR_TROLL, 4);
            army[4].Set(Monster::TROLL, 4);
	    break;

	case MP2::OBJ_DRAGONCITY:
            army[0].Set(Monster::GREEN_DRAGON, 3);
            army[1].Set(Monster::RED_DRAGON, 2);
            army[2].Set(Monster::BLACK_DRAGON, 1);
	    break;

	case MP2::OBJ_DAEMONCAVE:
            army[0].Set(Monster::EARTH_ELEMENT, 2);
            army[1].Set(Monster::EARTH_ELEMENT, 2);
            army[2].Set(Monster::EARTH_ELEMENT, 2);
            army[3].Set(Monster::EARTH_ELEMENT, 2);
	    break;

	default:
	    army[0] = Troop(t);
	    ArrangeForBattle();
	    break;
    }
}

Army::army_t & Army::army_t::operator= (const army_t & a)
{
    commander = NULL;

    Import(a.army);
    combat_format = a.combat_format;
    color = a.color;

    return *this;
}

void Army::army_t::SetCombatFormat(format_t f)
{
    combat_format = f;
}

u8 Army::army_t::GetCombatFormat(void) const
{
    return combat_format;
}

void Army::army_t::Import(const std::vector<Troop> & v)
{
    for(u8 ii = 0; ii < Size(); ++ii)
    {
	if(ii < v.size())
	    army[ii] = v[ii];
	else
	    army[ii].Reset();
    }
}

void Army::army_t::UpgradeMonsters(const Monster & m)
{
    UpgradeMonsters(m());
}

void Army::army_t::UpgradeMonsters(const Monster::monster_t m)
{
    for(u8 ii = 0; ii < Size(); ++ii) if(army[ii].isValid() && army[ii] == m) army[ii].Upgrade();
}

u8 Army::army_t::Size(void) const
{
    return army.size() > ARMYMAXTROOPS ? army.size() : ARMYMAXTROOPS;
}

Army::Troop & Army::army_t::FirstValid(void)
{
    std::vector<Troop>::iterator it = std::find_if(army.begin(), army.end(), Army::isValidTroop);

    if(it == army.end())
    {
	DEBUG(DBG_GAME , DBG_WARN, "Army::FirstValid: not found, return first..");
	it = army.begin();
    }

    return *it;
}

s8 Army::army_t::GetTroopIndex(const Troop & t) const
{
    for(u8 ii = 0; ii < Size(); ++ii) if(&army[ii] == &t) return ii;
    return -1;
}

Army::Troop & Army::army_t::At(u8 index)
{
    return index < Size() ? army[index] : army[Size() - 1];
}

const Army::Troop & Army::army_t::At(u8 index) const
{
    return index < Size() ? army[index] : army[Size() - 1];
}

u8 Army::army_t::GetCount(void) const
{
    return std::count_if(army.begin(), army.end(), Army::isValidTroop);
}

u8 Army::army_t::GetUniqCount(void) const
{
    std::vector<Monster::monster_t> troops;
    troops.reserve(ARMYMAXTROOPS);

    std::vector<Troop>::const_iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    for(; it1 != it2; ++it1) if((*it1).isValid()) troops.push_back((*it1).GetID());
    troops.resize(std::unique(troops.begin(), troops.end()) - troops.begin());

    return troops.size();
}

u32 Army::army_t::GetCountMonsters(const Monster & m) const
{
    return GetCountMonsters(m());
}

u32 Army::army_t::GetCountMonsters(const Monster::monster_t m) const
{
    u32 c = 0;

    std::vector<Troop>::const_iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    for(; it1 != it2; ++it1) if((*it1).isValid() && (*it1) == m) c += (*it1).GetCount();

    return c;
}

bool Army::army_t::JoinTroop(const Monster & mon, const u32 count)
{
    return JoinTroop(Troop(mon.GetID(), count));
}

bool Army::army_t::JoinTroop(const Monster::monster_t mon, const u32 count)
{
    return JoinTroop(Troop(mon, count));
}

bool Army::army_t::JoinTroop(const Troop & troop)
{
    if(!troop.isValid()) return false;

    std::vector<Troop>::iterator it;
    it = std::find_if(army.begin(), army.end(), std::bind2nd(std::mem_fun_ref(&Troop::HasMonster), troop()));

    if(it == army.end()) it = std::find_if(army.begin(), army.end(), std::not1(std::mem_fun_ref(&Troop::isValid)));

    if(it != army.end())
    {
	if((*it).isValid())
	    (*it).SetCount((*it).GetCount() + troop.GetCount());
	else
	    (*it).Set(troop, troop.GetCount());
	DEBUG(DBG_GAME , DBG_INFO, "Army::JoinTroop: monster: " << troop.GetName() << ", count: " << std::dec << troop.GetCount() << ", commander: " << (commander ? commander->GetName() : "unknown"));
	return true;
    }

    return false;
}

bool Army::army_t::isValid(void) const
{
    return army.end() != std::find_if(army.begin(),army.end(), Army::isValidTroop);
}

bool Army::army_t::HasMonster(const Monster & mons) const
{
    return HasMonster(mons());
}

bool Army::army_t::HasMonster(const Monster::monster_t mons) const
{
    return army.end() != std::find_if(army.begin(), army.end(), std::bind2nd(std::mem_fun_ref(&Troop::HasMonster), mons));
}

Color::color_t Army::army_t::GetColor(void) const
{
    return commander ? commander->GetColor() : color;
}

void Army::army_t::SetColor(Color::color_t cl)
{
    color = cl;
}

Race::race_t Army::army_t::GetRace(void) const
{
    std::vector<Troop>::const_iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    std::vector<Race::race_t> races;
    races.reserve(ARMYMAXTROOPS);

    for(; it1 != it2; ++it1) if((*it1).isValid()) races.push_back((*it1).GetRace());
    races.resize(std::unique(races.begin(), races.end()) - races.begin());

    if(races.empty())
    {
        DEBUG(DBG_GAME , DBG_WARN, "Army::GetRaceArmy: empty");
        return Race::MULT;
    }

    return 1 < races.size() ? Race::MULT : races.at(0);
}

s8 Army::army_t::GetLuck(void) const
{
    return commander ? commander->GetLuck() : GetLuckModificator(NULL);
}

s8 Army::army_t::GetLuckModificator(std::string *strs) const
{
    return Luck::NORMAL;
}

bool Army::army_t::AllTroopsIsRace(u8 race) const
{
    std::vector<Troop>::const_iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    for(; it1 != it2; ++it1) if((*it1).isValid() && (*it1).GetRace() != race) return false;

    return true;
}

s8 Army::army_t::GetMorale(void) const
{
    return commander ? commander->GetMorale() : GetMoraleModificator(NULL);
}

// TODO:: need optimize
s8 Army::army_t::GetMoraleModificator(std::string *strs) const
{
    s8 result(Morale::NORMAL);

    // different race penalty
    std::vector<Troop>::const_iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    u8 count = 0;
    u8 count_kngt = 0;
    u8 count_barb = 0;
    u8 count_sorc = 0;
    u8 count_wrlk = 0;
    u8 count_wzrd = 0;
    u8 count_necr = 0;
    u8 count_bomg = 0;
    bool ghost_present = false;
    for(; it1 != it2; ++it1) if((*it1).isValid())
    {
        switch((*it1).GetRace())
	{
            case Race::KNGT: ++count_kngt; break;
            case Race::BARB: ++count_barb; break;
            case Race::SORC: ++count_sorc; break;
            case Race::WRLK: ++count_wrlk; break;
            case Race::WZRD: ++count_wzrd; break;
            case Race::NECR: ++count_necr; break;
            case Race::BOMG: ++count_bomg; break;
            default: break;
	}
        if(*it1 == Monster::GHOST) ghost_present = true;
    }

    Race::race_t r = Race::MULT;
    if(count_kngt){ ++count; r = Race::KNGT; }
    if(count_barb){ ++count; r = Race::BARB; }
    if(count_sorc){ ++count; r = Race::SORC; }
    if(count_wrlk){ ++count; r = Race::WRLK; }
    if(count_wzrd){ ++count; r = Race::WZRD; }
    if(count_necr){ ++count; r = Race::NECR; }
    if(count_bomg){ ++count; r = Race::BOMG; }
    const u8 uniq_count = GetUniqCount();

    switch(count)
    {
        case 2:
        case 0: break;
        case 1:
    	    if(0 == count_necr && !ghost_present)
            {
		if(1 < uniq_count)
                {
		    ++result;
            	    if(strs)
            	    {
            		std::string str = _("All %{race} troops +1");
            		String::Replace(str, "%{race}", Race::String(r));
            		strs->append(str);
            		strs->append("\n");
            	    }
		}
            }
	    else
            {
	        if(strs)
                {
            	    strs->append(_("Entire unit is undead, so morale does not apply."));
            	    strs->append("\n");
            	}
		return 0;
	    }
            break;
        case 3:
            result -= 1;
            if(strs)
            {
        	strs->append(_("Troops of 3 alignments -1"));
        	strs->append("\n");
    	    }
            break;
        case 4:
    	    result -= 2;
            if(strs)
            {
        	strs->append(_("Troops of 4 alignments -2"));
        	strs->append("\n");
    	    }
            break;
        default:
            result -= 3;
            if(strs)
            {
        	strs->append(_("Troops of 5 alignments -3"));
        	strs->append("\n");
    	    }
            break;
    }

    // undead in life group
    if((1 < uniq_count && (count_necr || ghost_present) && (count_kngt || count_barb || count_sorc || count_wrlk || count_wzrd || count_bomg)) ||
    // or artifact Arm Martyr
	(commander && commander->HasArtifact(Artifact::ARM_MARTYR)))
    {
        result -= 1;
        if(strs)
        {
    	    strs->append(_("Some undead in groups -1"));
    	    strs->append("\n");
    	}
    }

    return result;
}

Army::Troop & Army::army_t::GetWeakestTroop(void)
{
    return *MinElement(army.begin(), army.end(), WeakestTroop);
}

const Army::Troop & Army::army_t::GetSlowestTroop(void) const
{
    return *MinElement(army.begin(), army.end(), SlowestTroop);
}

/* draw MONS32 sprite in line, first valid = 0, count = 0 */
void Army::army_t::DrawMons32Line(s16 cx, s16 cy, u16 width, u8 first, u8 count, bool hide) const
{
    if(!isValid()) return;

    if(0 == count) count = GetCount();
    else
    if(Size() < count) count = Size();

    const u16 chunk = width / count;
    cx += chunk / 2;

    std::string str;
    Text text;
    text.Set(Font::SMALL);

    for(u8 ii = 0; ii < Size(); ++ii)
    {
	const Army::Troop & troop = army[ii];

    	if(troop.isValid())
	{
	    if(0 == first && count)
    	    {
		const Sprite & monster = AGG::GetICN(ICN::MONS32, Monster::GetSpriteIndex(troop()));

    		Display::Get().Blit(monster, cx - monster.w() / 2, cy + 30 - monster.h());

    		if(hide)
		{
		    text.Set(Army::String(troop.GetCount()));
		}
		else
		{
    		    str.clear();
		    String::AddInt(str, troop.GetCount());
		    text.Set(str);
		}
		text.Blit(cx - text.w() / 2, cy + 28);

		cx += chunk;
		--count;
	    }
	    else
		--first;
	}
    }
}

void Army::army_t::ArrangeForBattle(void)
{
    if(GetControl() != Game::AI) return;

    std::vector<Troop> priority;
    priority.reserve(ARMYMAXTROOPS);

    std::vector<Troop>::iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    
    for(; it1 != it2; ++it1)
    {
	Troop & troop = *it1;
	if(!troop.isValid()) continue;

	std::vector<Troop>::iterator it = std::find_if(priority.begin(), priority.end(), std::bind2nd(std::mem_fun_ref(&Troop::HasMonster), troop()));

	if(it == priority.end())
	    priority.push_back(troop);
	else
	    (*it).SetCount((*it).GetCount() + troop.GetCount());

	troop.Reset();
    }

    switch(priority.size())
    {
	case 1:
	{
	    const Monster & m = priority.back();
	    const u32 count = priority.back().GetCount();
	    if(49 < count)
	    {
		const u32 c = count / 5;
		army[0].Set(m, c);
		army[1].Set(m, c);
		army[2].Set(m, c + count - (c * 5));
		army[3].Set(m, c);
		army[4].Set(m, c);
	    }
	    else
	    if(20 < count)
	    {
		const u32 c = count / 3;
		army[1].Set(m, c);
		army[2].Set(m, c + count - (c * 3));
		army[3].Set(m, c);
	    }
	    else
		army[2].Set(m, count);
	    break;
	}
	case 2:
	{
	    // TODO: need modify army for 2 troops
	    Import(priority);
	    break;
	}
	case 3:
	{
	    // TODO: need modify army for 3 troops
	    Import(priority);
	    break;
	}
	case 4:
	{
	    // TODO: need modify army for 4 troops
	    Import(priority);
	    break;
	}
	case 5:
	{    // possible change orders monster
	    // store
	    Import(priority);
	    break;
	}
	default: break;
    }
}

u32 Army::army_t::CalculateExperience(void) const
{
    u32 res = 0;

    for(u8 ii = 0; ii < Size(); ++ii)
	res += army[ii].GetHitPoints();

    if(commander) res += 500;

    return res;

}

void Army::army_t::BattleInit(void)
{
    // reserve ARMYMAXTROOPS for summons and mirrors spell
    if(commander)
    {
	army.reserve(ARMYMAXTROOPS * 2);
	commander->PreBattleAction();
    }
}

void Army::army_t::BattleQuit(void)
{
    std::for_each(army.begin(), army.end(), std::mem_fun_ref(&Troop::BattleQuit));
    if(army.size() > ARMYMAXTROOPS) army.resize(ARMYMAXTROOPS);
}

void Army::army_t::BattleNewTurn(void)
{
    std::for_each(army.begin(), army.end(), std::mem_fun_ref(&Troop::BattleNewTurn));
    if(commander) commander->ResetModes(Heroes::SPELLCASTED);
}

void Army::army_t::BattleSetModes(u32 f)
{
    //std::for_each(army.begin(), army.end(), std::bind2nd(std::mem_fun_ref(&Troop::BattleSetModes), f)); // Cannot call non-const function here
    for( std::vector<Troop> ::iterator it = army.begin(); it != army.end(); it++ )
        it->BattleSetModes(f);
}

void Army::army_t::BattleResetModes(u32 f)
{
    //std::for_each(army.begin(), army.end(), std::bind2nd(std::mem_fun_ref(&Troop::BattleResetModes), f)); // Cannot call non-const function here
    for( std::vector<Troop> ::iterator it = army.begin(); it != army.end(); it++ )
        it->BattleResetModes(f);
}

Army::Troop* Army::army_t::BattleFindModes(u32 f)
{
    std::vector<Troop>::iterator it = std::find_if(army.begin(), army.end(), std::bind2nd(std::mem_fun_ref(&Troop::BattleFindModes), f));
    return it != army.end() ? &(*it) : NULL;
}

const Army::Troop* Army::army_t::BattleFindModes(u32 f) const
{
    std::vector<Troop>::const_iterator it = std::find_if(army.begin(), army.end(), std::bind2nd(std::mem_fun_ref(&Troop::BattleFindModes), f));
    return it != army.end() ? &(*it) : NULL;
}

Army::Troop & Army::army_t::BattleNewTroop(Monster::monster_t id, u32 count)
{
    // find free invalid
    std::vector<Troop>::iterator it = army.begin();
    while(it != army.end() && NULL != (*it).GetBattleStats()) ++it;

    if(army.end() == it)
    {
	army.resize(army.size() + 1);
	it = army.end() - 1;
    }

    (*it).id = id;
    (*it).count = count;
    (*it).army = this;

    (*it).BattleInit();

    return *it;
}

u32 Army::army_t::BattleKilled(void) const
{
    u32 res = 0;
    std::vector<Troop>::const_iterator it = army.begin();
    for(; it != army.end(); ++it) res += (*it).BattleKilled();

    return res;
}

u8 Army::army_t::BattleUndeadTroopCount(void) const
{
    return std::count_if(army.begin(), army.end(), std::mem_fun_ref(&Troop::isUndead));
}

u8 Army::army_t::BattleLifeTroopCount(void) const
{
    return GetCount() - std::count_if(army.begin(), army.end(), std::mem_fun_ref(&Troop::isUndead)) -
	std::count_if(army.begin(), army.end(), std::mem_fun_ref(&Troop::isElemental));
}

const Battle2::Stats* Army::army_t::BattleRandomTroop(void) const
{
    std::vector<const Battle2::Stats*> v;

    std::vector<Troop>::const_iterator it = army.begin();
    for(; it != army.end(); ++it) if((*it).isValid())
	v.push_back((*it).GetBattleStats());

    return v.size() ? *Rand::Get(v) : NULL;
}

bool Army::army_t::BattleArchersPresent(void) const
{
    return army.end() != std::find_if(army.begin(), army.end(), std::mem_fun_ref(&Troop::BattleIsArchers));
}

bool Army::army_t::BattleDragonsPresent(void) const
{
    return army.end() != std::find_if(army.begin(), army.end(), std::mem_fun_ref(&Troop::BattleIsDragons));
}

Battle2::Stats* Army::army_t::BattleFastestTroop(bool skipmove)
{
    std::vector<Troop>::iterator it = army.begin();
    Battle2::Stats* cur = NULL;

    for(; it != army.end(); ++it) if((*it).isValid())
    {
	Battle2::Stats* b = (*it).GetBattleStats();

	if(b && !b->Modes(Battle2::TR_MOVED) &&
	   ((skipmove && b->Modes(Battle2::TR_SKIPMOVE)) || (!skipmove && !b->Modes(Battle2::TR_SKIPMOVE))) &&
	   Speed::STANDING < b->GetSpeed() &&
	   (NULL == cur || b->GetSpeed() > cur->GetSpeed())) cur = b;
    }

    return cur;
}

Battle2::Stats* Army::army_t::BattleSlowestTroop(bool skipmove)
{
    std::vector<Troop>::iterator it = army.begin();
    Battle2::Stats* cur = NULL;

    for(; it != army.end(); ++it) if((*it).isValid())
    {
	Battle2::Stats* b = (*it).GetBattleStats();

	if(b && !b->Modes(Battle2::TR_MOVED) &&
	   ((skipmove && b->Modes(Battle2::TR_SKIPMOVE)) ||(!skipmove && !b->Modes(Battle2::TR_SKIPMOVE))) &&
	   Speed::STANDING < b->GetSpeed() &&
	   (NULL == cur || b->GetSpeed() < cur->GetSpeed())) cur = b;
    }

    return cur;
}

void Army::army_t::Clear(void)
{
    std::for_each(army.begin(), army.end(), std::mem_fun_ref(&Troop::Reset));
}

void Army::army_t::BattleExportKilled(army_t & a) const
{
    a.army.resize(Size());

    for(u8 ii = 0; ii < Size(); ++ii)
    {
	a.army[ii].SetMonster(army[ii].GetID());
	a.army[ii].SetCount(army[ii].BattleKilled());
    }

    a.commander = commander;
}

void Army::army_t::Reset(bool soft)
{
    Clear();

    if(commander)
    {
    	const Monster mons1(commander->GetRace(), DWELLING_MONSTER1);

	if(soft)
	{
    	    const Monster mons2(commander->GetRace(), DWELLING_MONSTER2);

	    switch(Rand::Get(1, 3))
	    {
		case 1:
		    JoinTroop(mons1, 3 * mons1.GetGrown());
		    break;
		case 2:
		    JoinTroop(mons2, static_cast<u8>(1.5 * mons2.GetGrown()));
		    break;
		default:
		    JoinTroop(mons1, 2 * mons1.GetGrown());
		    JoinTroop(mons2, mons2.GetGrown());
		    break;
	    }
	}
	else
	{
	    JoinTroop(mons1, 1);
	}
    }
}

void Army::army_t::JoinStrongestFromArmy(army_t & army2)
{
    std::vector<Troop> priority;
    priority.reserve(ARMYMAXTROOPS * 2);

    std::vector<Troop>::iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    
    for(; it1 != it2; ++it1)
    {
	Troop & troop = *it1;
	if(!troop.isValid()) continue;

	std::vector<Troop>::iterator it = std::find_if(priority.begin(), priority.end(), std::bind2nd(std::mem_fun_ref(&Troop::HasMonster), troop()));

	if(it == priority.end())
	    priority.push_back(troop);
	else
	    (*it).SetCount((*it).GetCount() + troop.GetCount());

	troop.Reset();
    }

    it1 = army2.army.begin();
    it2 = army2.army.end();
    
    for(; it1 != it2; ++it1)
    {
	Troop & troop = *it1;
	if(!troop.isValid()) continue;

	std::vector<Troop>::iterator it = std::find_if(priority.begin(), priority.end(), std::bind2nd(std::mem_fun_ref(&Troop::HasMonster), troop()));

	if(it == priority.end())
	    priority.push_back(troop);
	else
	    (*it).SetCount((*it).GetCount() + troop.GetCount());

	troop.Reset();
    }

    // sort: strongest
    std::sort(priority.begin(), priority.end(), StrongestTroop);

    // weakest to army2
    while(Size() < priority.size())
    {
	army2.JoinTroop(priority.back());
	priority.pop_back();
    }

    // save half weak of strongest to army2
    if(Size() > army2.army.size())
    {
	Troop & last = priority.back();

	army2.JoinTroop(last, last.GetCount() - last.GetCount() / 2);
	JoinTroop(last, last.GetCount() / 2);
	priority.pop_back();
    }

    // strongest to army
    while(priority.size())
    {
	JoinTroop(priority.back());
	priority.pop_back();
    }
}

void Army::army_t::KeepOnlyWeakestTroops(army_t & army2)
{
    std::vector<Troop> priority;
    priority.reserve(ARMYMAXTROOPS * 2);

    std::vector<Troop>::iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    
    for(; it1 != it2; ++it1)
    {
	Troop & troop = *it1;
	if(!troop.isValid()) continue;

	std::vector<Troop>::iterator it = std::find_if(priority.begin(), priority.end(), std::bind2nd(std::mem_fun_ref(&Troop::HasMonster), troop()));

	if(it == priority.end())
	    priority.push_back(troop);
	else
	    (*it).SetCount((*it).GetCount() + troop.GetCount());

	troop.Reset();
    }

    it1 = army2.army.begin();
    it2 = army2.army.end();
    
    for(; it1 != it2; ++it1)
    {
	Troop & troop = *it1;
	if(!troop.isValid()) continue;

	std::vector<Troop>::iterator it = std::find_if(priority.begin(), priority.end(), std::bind2nd(std::mem_fun_ref(&Troop::HasMonster), troop()));

	if(it == priority.end())
	    priority.push_back(troop);
	else
	    (*it).SetCount((*it).GetCount() + troop.GetCount());

	troop.Reset();
    }

    // sort: weakest
    std::sort(priority.begin(), priority.end(), WeakestTroop);

    // strongest to army2
    while(1 < priority.size() && Size() > army2.GetCount())
    {
	army2.JoinTroop(priority.back());
	priority.pop_back();
    }

    // weakest to army
    while(priority.size())
    {
	JoinTroop(priority.back());
	priority.pop_back();
    }
}

void Army::army_t::UpgradeTroops(const Castle & castle)
{
    std::vector<Troop>::iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    
    for(; it1 != it2; ++it1) if((*it1).isValid())
    {
	Troop & troop = *it1;
        PaymentConditions::UpgradeMonster payment(troop());
        payment *= troop.GetCount();

	if(castle.GetRace() == troop.GetRace() &&
	   castle.isBuild(Monster::GetDwelling(Monster::Upgrade(troop))) &&
	   payment <= world.GetKingdom(castle.GetColor()).GetFundsResource())
	{
    	    world.GetKingdom(castle.GetColor()).OddFundsResource(payment);
            troop.Upgrade();
	}
    }
}

void Army::army_t::Dump(void) const
{
    std::cout << "Army::Dump: " << Color::String(commander ? commander->GetColor() : color) << ": ";

    std::vector<Troop>::const_iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    
    for(; it1 != it2; ++it1) if((*it1).isValid()) std::cout << (*it1).GetName() << "(" << std::dec << (*it1).GetCount() << "), ";

    std::cout << std::endl;
}

u16 Army::army_t::GetAttack(void) const
{
    std::vector<Troop>::const_iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    u16 res = 0;
    u8 count = 0;

    for(; it1 != it2; ++it1) if((*it1).isValid()){ res += (*it1).GetAttack(); ++count; }

    return count ? res / count : 0;
}

u16 Army::army_t::GetDefense(void) const
{
    std::vector<Troop>::const_iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    u16 res = 0;
    u8 count = 0;

    for(; it1 != it2; ++it1) if((*it1).isValid()){ res += (*it1).GetDefense(); ++count; }

    return count ? res / count : 0;
}

u32 Army::army_t::GetHitPoints(void) const
{
    std::vector<Troop>::const_iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    u32 res = 0;

    for(; it1 != it2; ++it1) if((*it1).isValid()) res += (*it1).GetHitPoints();

    return res;
}

u32 Army::army_t::GetDamageMin(void) const
{
    std::vector<Troop>::const_iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    u32 res = 0;
    u8 count = 0;

    for(; it1 != it2; ++it1) if((*it1).isValid()){ res += (*it1).GetDamageMin(); ++count; }

    return count ? res / count : 0;
}

u32 Army::army_t::GetDamageMax(void) const
{
    std::vector<Troop>::const_iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    u32 res = 0;
    u8 count = 0;

    for(; it1 != it2; ++it1) if((*it1).isValid()){ res += (*it1).GetDamageMax(); ++count; }

    return count ? res / count : 0;
}

double Army::army_t::GetStrength(void) const
{
    return (GetDamageMin() + GetDamageMax()) / static_cast<double>(2 * GetHitPoints());
}

bool Army::army_t::StrongerEnemyArmy(const army_t & army2) const
{
    const u16 a1 = GetAttack();
    const u16 d1 = GetDefense();
    double r1 = 0;

    const u16 a2 = army2.GetAttack();
    const u16 d2 = army2.GetDefense();
    double r2 = 0;

    if(a1 > d2)
        r1 = 1 + 0.1 * static_cast<double>(std::min(a1 - d2, 20));
    else
        r1 = 1 + 0.05 * static_cast<double>(std::min(d2 - a1, 14));

    if(a2 > d1)
        r2 = 1 + 0.1 * static_cast<double>(std::min(a2 - d1, 20));
    else
        r2 = 1 + 0.05 * static_cast<double>(std::min(d1 - a2, 14));

    r1 *= GetStrength();
    r2 *= army2.GetStrength();

    if(IS_DEBUG(DBG_GAME, DBG_INFO))
    {
	Dump();
	army2.Dump();
	VERBOSE("Army::StrongerEnemyArmy: " << "army1: " << r1 << ", army2: " << r2);
    }

    return 0 == r2 || 1 <= (r1 / r2);
}

void Army::army_t::SetCommander(HeroBase* c)
{
    commander = c;
}

HeroBase* Army::army_t::GetCommander(void)
{
    return (!commander || (Skill::Primary::CAPTAIN == commander->GetType() && !commander->isValid()) ? NULL : commander);
}

const HeroBase* Army::army_t::GetCommander(void) const
{
    return (!commander || (Skill::Primary::CAPTAIN == commander->GetType() && !commander->isValid()) ? NULL : commander);
}

u32 Army::army_t::ActionToSirens(void)
{
    std::vector<Troop>::iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();
    u32 res = 0;

    for(; it1 != it2; ++it1) if((*it1).isValid())
    {
	const u32 kill = (*it1).GetCount() * 30 / 100;

	if(kill)
	{
	    (*it1).SetCount((*it1).GetCount() - kill);
	    res += kill * static_cast<Monster>(*it1).GetHitPoints();
	}
    }

    return res;
}

u8 Army::army_t::GetControl(void) const
{
    return commander ? commander->GetControl() : (color == Color::GRAY ? Game::AI : world.GetKingdom(color).Control());
}

u32 Army::army_t::GetSurrenderCost(void) const
{
    std::vector<Troop>::const_iterator it1 = army.begin();
    std::vector<Troop>::const_iterator it2 = army.end();

    u32 res = 0;

    for(; it1 != it2; ++it1) if((*it1).isValid())
    {
	// FIXME: orig: 3 titan = 7500
	PaymentConditions::BuyMonster payment((*it1).GetID());
	res += (payment.gold * (*it1).GetCount());
    }

    if(commander)
    {
	switch(commander->GetLevelSkill(Skill::Secondary::DIPLOMACY))
	{
	    // 40%
	    case Skill::Level::BASIC: res = res * 40 / 100; break;
	    // 30%
	    case Skill::Level::ADVANCED: res = res * 30 / 100; break;
	    // 20%
	    case Skill::Level::EXPERT: res = res * 20 / 100; break;
	    // 50%
	    default: res = res * 50 / 100; break;
	}

	if(commander->HasArtifact(Artifact::STATESMAN_QUILL))
	    res /= 5;
    }

    // limit
    if(res < 100) res = 100;

    return res;
}

u8 Army::GetJoinSolution(const Heroes & hero, const Maps::Tiles & tile, u32 & join, s32 & cost)
{
    const Army::Troop troop(tile);

    if(! troop.isValid()) return 0xFF;

    const float ratios = troop.isValid() ? hero.GetArmy().GetHitPoints() / troop.GetHitPoints() : 0;
    const bool check_free_stack = (hero.GetArmy().GetCount() < hero.GetArmy().Size() || hero.GetArmy().HasMonster(troop()));
    const bool check_extra_condition = (!hero.HasArtifact(Artifact::HIDEOUS_MASK) && Morale::NORMAL <= hero.GetMorale());

    // force join for campain and others...
    const bool force_join = (5 == tile.GetQuantity4());

    if(tile.GetQuantity4() && check_free_stack && ((check_extra_condition && ratios >= 2) || force_join))
    {
        if(2 == tile.GetQuantity4() || force_join)
        {
	    join = troop.GetCount();
	    return 1;
	}
	else
        if(hero.HasSecondarySkill(Skill::Secondary::DIPLOMACY))
        {
            const Kingdom & kingdom = world.GetKingdom(hero.GetColor());
            Resource::funds_t payment = PaymentConditions::BuyMonster(troop());
            cost = payment.gold * troop.GetCount();
	    payment.Reset();
	    payment.gold = cost;

            // skill diplomacy
            const u32 to_join = Monster::GetCountFromHitPoints(troop(), troop.GetHitPoints() * hero.GetSecondaryValues(Skill::Secondary::DIPLOMACY) / 100);

            if(to_join && kingdom.AllowPayment(payment))
            {
		join = to_join;
		return 2;
	    }
	}
    }
    else
    if(ratios >= 5)
    {
	// ... surely flee before us

	if(hero.GetControl() == Game::AI) return Rand::Get(0, 10) < 5 ? 0 : 3;

	return 3;
    }

    return 0;
}

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

#include "castle.h"
#include "luck.h"
#include "morale.h"
#include "captain.h"
#include "settings.h"

Captain::Captain(const Castle & c) : home(c)
{
}

void Captain::LoadDefaults(void)
{
    if(Race::ALL & home.GetRace())
    {
	u8 book, spell;
	Skill::Primary::LoadDefaults(home.GetRace(), *this, book, spell);
	SpellBookActivate();
	if(home.GetLevelMageGuild()) home.GetMageGuild().EducateHero(*this);
    }
}

bool Captain::isValid(void) const
{
    return home.isBuild(BUILD_CAPTAIN);
}

u8 Captain::GetAttack(void) const
{
    return attack + GetAttackModificator(NULL);
}

u8 Captain::GetDefense(void) const
{
    return defense + GetDefenseModificator(NULL);
}

u8 Captain::GetPower(void) const
{
   return power + GetPowerModificator(NULL);
}

u8 Captain::GetKnowledge(void) const
{
    return knowledge + GetKnowledgeModificator(NULL);
}

s8 Captain::GetMorale(void) const
{
    s8 result = Morale::NORMAL;

    // global modificator
    result += GetMoraleModificator(false, NULL);

    // result
    if(result < Morale::AWFUL)  return Morale::TREASON;
    else
    if(result < Morale::POOR)   return Morale::AWFUL;
    else
    if(result < Morale::NORMAL) return Morale::POOR;
    else
    if(result < Morale::GOOD)   return Morale::NORMAL;
    else
    if(result < Morale::GREAT)  return Morale::GOOD;
    else
    if(result < Morale::BLOOD)  return Morale::GREAT;

    return Morale::BLOOD;
}

s8 Captain::GetLuck(void) const
{
    s8 result = Luck::NORMAL;

    // global modificator
    result += GetLuckModificator(false, NULL);

    // result
    if(result < Luck::AWFUL)    return Luck::CURSED;
    else
    if(result < Luck::BAD)      return Luck::AWFUL;
    else
    if(result < Luck::NORMAL)   return Luck::BAD;
    else
    if(result < Luck::GOOD)     return Luck::NORMAL;
    else
    if(result < Luck::GREAT)    return Luck::GOOD;
    else
    if(result < Luck::IRISH)    return Luck::GREAT;

    return Luck::IRISH;
}

Race::race_t Captain::GetRace(void) const
{
    return home.GetRace();
}

Color::color_t Captain::GetColor(void) const
{
    return home.GetColor();
}

const std::string & Captain::GetName(void) const
{
    return home.GetName();
}

u8 Captain::GetType(void) const
{
    return Skill::Primary::CAPTAIN;
}

u8 Captain::GetLevelSkill(const Skill::Secondary::skill_t skill) const
{
    return 0;
}

u8 Captain::GetSecondaryValues(const Skill::Secondary::skill_t skill) const
{
    return 0;
}

const Army::army_t & Captain::GetArmy(void) const
{
    return home.GetArmy();
}

Army::army_t & Captain::GetArmy(void)
{
    return const_cast<Army::army_t &>(home.GetArmy());
}

u16 Captain::GetMaxSpellPoints(void) const
{
    return knowledge * 10;
}

u8 Captain::GetControl(void) const
{
    return home.GetControl();
}

u16 Captain::GetIndex(void) const
{
    return home.GetIndex();
}

void Captain::PreBattleAction(void)
{
    SetSpellPoints(GetMaxSpellPoints());
}

const Castle* Captain::inCastle(void) const
{
    return &home;
}

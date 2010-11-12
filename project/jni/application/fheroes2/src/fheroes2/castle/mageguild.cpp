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
#include "castle.h"
#include "heroes_base.h"
#include "settings.h"
#include "mageguild.h"

Spell::spell_t GetUniqueCombatSpellCompatibility(const std::vector<Spell::spell_t> &, u8 race, u8 level);
Spell::spell_t GetCombatSpellCompatibility(u8 race, u8 level);

MageGuild::MageGuild(const Castle & cstl) : home(cstl)
{
}

void MageGuild::Builds(void)
{
    general.spells.clear();
    library.spells.clear();

    // level 5
    general.Append(7 > Rand::Get(1, 10) ? Spell::RandCombat(5) : Spell::RandAdventure(5));

    // level 4
    general.Append(GetCombatSpellCompatibility(home.GetRace(), 4));
    general.Append(Spell::RandAdventure(4));

    // level 3
    general.Append(GetCombatSpellCompatibility(home.GetRace(), 3));
    general.Append(Spell::RandAdventure(3));

    // level 2
    general.Append(GetCombatSpellCompatibility(home.GetRace(), 2));
    general.Append(GetUniqueCombatSpellCompatibility(general.spells, home.GetRace(), 2));
    general.Append(Spell::RandAdventure(2));

    // level 1
    general.Append(GetCombatSpellCompatibility(home.GetRace(), 1));
    general.Append(GetUniqueCombatSpellCompatibility(general.spells, home.GetRace(), 1));
    general.Append(Spell::RandAdventure(1));

    if(HaveLibraryCapability())
    {
	library.Append(GetUniqueCombatSpellCompatibility(general.spells, home.GetRace(), 1));
	library.Append(GetUniqueCombatSpellCompatibility(general.spells, home.GetRace(), 2));
	library.Append(GetUniqueCombatSpellCompatibility(general.spells, home.GetRace(), 3));
	library.Append(GetUniqueCombatSpellCompatibility(general.spells, home.GetRace(), 4));
	library.Append(GetUniqueCombatSpellCompatibility(general.spells, home.GetRace(), 5));
    }
}

bool MageGuild::isLibraryBuild(void) const
{
    return home.GetRace() == Race::WZRD && home.isBuild(BUILD_SPEC);
}

bool MageGuild::HaveLibraryCapability(void) const
{
    return home.GetRace() == Race::WZRD;
}

u8 MageGuild::GetLevel(void) const
{
    return home.GetLevelMageGuild();
}

void MageGuild::GetSpells(std::vector<Spell::spell_t> & spells, u8 level) const
{
    if(home.GetLevelMageGuild() >= level)
    {
	general.GetSpells(spells, level);
	if(isLibraryBuild()) library.GetSpells(spells, level);
    }
}

void MageGuild::EducateHero(HeroBase & hero) const
{
    if(hero.HaveSpellBook() && GetLevel())
    {
	std::vector<Spell::spell_t> spells;

	for(u8 level = 1; level <= 5; ++level) if(level <= GetLevel())
	{
	    general.GetSpells(spells, level);
	    if(isLibraryBuild()) library.GetSpells(spells, level);
	}

	std::vector<Spell::spell_t>::const_iterator it1 = spells.begin();
	std::vector<Spell::spell_t>::const_iterator it2 = spells.end();
	for(; it1 != it2; ++it1) hero.AppendSpellToBook(*it1);
    }
}

Spell::spell_t GetUniqueCombatSpellCompatibility(const std::vector<Spell::spell_t> & spells, u8 race, u8 lvl)
{
    Spell::spell_t spell = GetCombatSpellCompatibility(race, lvl);
    while(spells.end() != std::find(spells.begin(), spells.end(), spell)) spell = GetCombatSpellCompatibility(race, lvl);
    return spell;
}

Spell::spell_t GetCombatSpellCompatibility(u8 race, u8 lvl)
{
    Spell::spell_t spell = Spell::RandCombat(lvl);
    while(!Spell::isRaceCompatible(spell, race)) spell = Spell::RandCombat(lvl);
    return spell;
}

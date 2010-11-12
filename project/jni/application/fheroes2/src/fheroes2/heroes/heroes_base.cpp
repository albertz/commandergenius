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
#include <utility>
#include "artifact.h"
#include "army.h"
#include "castle.h"
#include "heroes_base.h"

typedef std::vector< std::pair<Artifact::artifact_t, s8> > ArtifactsModifiers;

s8 GetResultModifiers(const ArtifactsModifiers & modifiers, const HeroBase & base, std::string* strs)
{
    s8 result = 0;

    for(size_t ii = 0; ii < modifiers.size(); ++ii)
    {
	const std::pair<Artifact::artifact_t, s8> & pair = modifiers[ii];

        if(base.HasArtifact(pair.first))
        {
    	    result += pair.second;

    	    if(strs)
    	    {
        	strs->append(Artifact::GetName(pair.first));
		StringAppendModifiers(*strs, pair.second);
        	strs->append("\n");
	    }
	}
    }

    return result;
}

HeroBase::HeroBase() : magic_point(0), spell_book()
{
}

u16 HeroBase::GetSpellPoints(void) const
{
    return magic_point;
}

void HeroBase::SetSpellPoints(u16 points)
{
    magic_point = points;
}

bool HeroBase::HaveSpellPoints(u16 points) const
{
    return magic_point >= points;
}

void HeroBase::TakeSpellPoints(u16 points)
{
    magic_point -= (points < magic_point ? points : magic_point);
}

Spell::spell_t HeroBase::OpenSpellBook(SpellBook::filter_t flt, bool canselect) const
{
    return spell_book.Open(*this, flt, canselect);
}

bool HeroBase::HaveSpellBook(void) const
{
    return spell_book.isActive();
}

bool HeroBase::HaveSpell(Spell::spell_t spell) const
{
    BagArtifacts::const_iterator it1 = bag_artifacts.begin();
    BagArtifacts::const_iterator it2 = bag_artifacts.end();
    for(; it1 != it2; ++it1) if(*it1 == Artifact::SPELL_SCROLL)
    {
        if(spell == Spell::FromInt((*it1).GetExt())) return true;
    }

    return spell_book.isActive() && spell_book.isPresentSpell(spell);
}

void HeroBase::AppendSpellToBook(Spell::spell_t spell, bool without_wisdom)
{
    if(without_wisdom)
	spell_book.Append(spell);
    else
	spell_book.Append(spell, GetLevelSkill(Skill::Secondary::WISDOM));
}

void HeroBase::SpellBookActivate(void)
{
    spell_book.Activate();
}

const BagArtifacts & HeroBase::GetBagArtifacts(void) const
{
    return bag_artifacts;
}

BagArtifacts & HeroBase::GetBagArtifacts(void)
{
    return bag_artifacts;
}

bool HeroBase::HasArtifact(Artifact::artifact_t art) const
{
    return bag_artifacts.size() && bag_artifacts.end() != std::find(bag_artifacts.begin(), bag_artifacts.end(), art);
}

s8 HeroBase::GetAttackModificator(std::string* strs) const
{
    static ArtifactsModifiers modifiers;

    if(modifiers.empty())
    {
	modifiers.reserve(14);

	modifiers.push_back(std::make_pair(Artifact::SPIKED_HELM, 1));
	modifiers.push_back(std::make_pair(Artifact::THUNDER_MACE, 1));
	modifiers.push_back(std::make_pair(Artifact::GIANT_FLAIL, 1));
	modifiers.push_back(std::make_pair(Artifact::SWORD_BREAKER, 1));
	modifiers.push_back(std::make_pair(Artifact::SPIKED_SHIELD, 2));
	modifiers.push_back(std::make_pair(Artifact::POWER_AXE, 2));
	modifiers.push_back(std::make_pair(Artifact::LEGENDARY_SCEPTER, 2));
	modifiers.push_back(std::make_pair(Artifact::DRAGON_SWORD, 3));
	modifiers.push_back(std::make_pair(Artifact::ULTIMATE_CROWN, 4));
	modifiers.push_back(std::make_pair(Artifact::BATTLE_GARB, 5));
	modifiers.push_back(std::make_pair(Artifact::SWORD_ANDURAN, 5));
	modifiers.push_back(std::make_pair(Artifact::HOLY_HAMMER, 5));
	modifiers.push_back(std::make_pair(Artifact::ULTIMATE_SHIELD, 6));
	modifiers.push_back(std::make_pair(Artifact::ULTIMATE_SWORD, 12));
    }

    s8 result = GetResultModifiers(modifiers, *this, strs);

    // check castle modificator
    const Castle* castle = inCastle();

    if(castle)
	result += castle->GetAttackModificator(strs);

    return result;
}

s8 HeroBase::GetDefenseModificator(std::string* strs) const
{
    static ArtifactsModifiers modifiers;

    if(modifiers.empty())
    {
	modifiers.reserve(13);

	modifiers.push_back(std::make_pair(Artifact::SPIKED_HELM, 1));
	modifiers.push_back(std::make_pair(Artifact::ARMORED_GAUNTLETS, 1));
	modifiers.push_back(std::make_pair(Artifact::DEFENDER_HELM, 1));
	modifiers.push_back(std::make_pair(Artifact::SPIKED_SHIELD, 2));
	modifiers.push_back(std::make_pair(Artifact::STEALTH_SHIELD, 2));
	modifiers.push_back(std::make_pair(Artifact::LEGENDARY_SCEPTER, 2));
	modifiers.push_back(std::make_pair(Artifact::DIVINE_BREASTPLATE, 3));
	modifiers.push_back(std::make_pair(Artifact::ULTIMATE_CROWN, 4));
	modifiers.push_back(std::make_pair(Artifact::SWORD_BREAKER, 4));
	modifiers.push_back(std::make_pair(Artifact::BREASTPLATE_ANDURAN, 5));
	modifiers.push_back(std::make_pair(Artifact::BATTLE_GARB, 5));
	modifiers.push_back(std::make_pair(Artifact::ULTIMATE_SHIELD, 6));
	modifiers.push_back(std::make_pair(Artifact::ULTIMATE_CLOAK, 12));
    }

    s8 result = GetResultModifiers(modifiers, *this, strs);

    // check castle modificator
    const Castle* castle = inCastle();

    if(castle)
	result += castle->GetDefenseModificator(strs);

    return result;
}

s8 HeroBase::GetPowerModificator(std::string* strs) const
{
    static ArtifactsModifiers modifiers;

    if(modifiers.empty())
    {
	modifiers.reserve(15);

	modifiers.push_back(std::make_pair(Artifact::BROACH_SHIELDING, -1));
	modifiers.push_back(std::make_pair(Artifact::WHITE_PEARL, 1));
	modifiers.push_back(std::make_pair(Artifact::BLACK_PEARL, 2));
	modifiers.push_back(std::make_pair(Artifact::CASTER_BRACELET, 2));
	modifiers.push_back(std::make_pair(Artifact::MAGE_RING, 2));
	modifiers.push_back(std::make_pair(Artifact::LEGENDARY_SCEPTER, 2));
	modifiers.push_back(std::make_pair(Artifact::WITCHES_BROACH, 3));
	modifiers.push_back(std::make_pair(Artifact::ARM_MARTYR, 3));
	modifiers.push_back(std::make_pair(Artifact::ULTIMATE_CROWN, 4));
	modifiers.push_back(std::make_pair(Artifact::ARCANE_NECKLACE, 4));
        modifiers.push_back(std::make_pair(Artifact::BATTLE_GARB, 5));
	modifiers.push_back(std::make_pair(Artifact::STAFF_WIZARDRY, 5));
	modifiers.push_back(std::make_pair(Artifact::HELMET_ANDURAN, 5));
	modifiers.push_back(std::make_pair(Artifact::ULTIMATE_STAFF, 6));
	modifiers.push_back(std::make_pair(Artifact::ULTIMATE_WAND, 12));
    }

    s8 result = GetResultModifiers(modifiers, *this, strs);

    // check castle modificator
    const Castle* castle = inCastle();

    if(castle)
	result += castle->GetPowerModificator(strs);

    return result;
}

s8 HeroBase::GetKnowledgeModificator(std::string* strs) const
{
    static ArtifactsModifiers modifiers;

    if(modifiers.empty())
    {
	modifiers.reserve(10);

	modifiers.push_back(std::make_pair(Artifact::WHITE_PEARL, 1));
	modifiers.push_back(std::make_pair(Artifact::BLACK_PEARL, 2));
	modifiers.push_back(std::make_pair(Artifact::MINOR_SCROLL, 2));
	modifiers.push_back(std::make_pair(Artifact::LEGENDARY_SCEPTER, 2));
	modifiers.push_back(std::make_pair(Artifact::MAJOR_SCROLL, 3));
	modifiers.push_back(std::make_pair(Artifact::ULTIMATE_CROWN, 4));
	modifiers.push_back(std::make_pair(Artifact::SUPERIOR_SCROLL, 4));
	modifiers.push_back(std::make_pair(Artifact::FOREMOST_SCROLL, 5));
	modifiers.push_back(std::make_pair(Artifact::ULTIMATE_STAFF, 6));
	modifiers.push_back(std::make_pair(Artifact::ULTIMATE_BOOK, 12));
    }

    s8 result = GetResultModifiers(modifiers, *this, strs);

    // check castle modificator
    const Castle* castle = inCastle();

    if(castle)
	result += castle->GetKnowledgeModificator(strs);

    return result;
}

s8 HeroBase::GetMoraleModificator(bool shipmaster, std::string* strs) const
{
    ArtifactsModifiers modifiers;

    modifiers.reserve(7);

    modifiers.push_back(std::make_pair(Artifact::MEDAL_VALOR, 1));
    modifiers.push_back(std::make_pair(Artifact::MEDAL_COURAGE, 1));
    modifiers.push_back(std::make_pair(Artifact::MEDAL_HONOR, 1));
    modifiers.push_back(std::make_pair(Artifact::MEDAL_DISTINCTION, 1));
    modifiers.push_back(std::make_pair(Artifact::FIZBIN_MISFORTUNE, -2));
    modifiers.push_back(std::make_pair(Artifact::BATTLE_GARB, 10));
    if(shipmaster) modifiers.push_back(std::make_pair(Artifact::MASTHEAD, 1));

    s8 result = GetResultModifiers(modifiers, *this, strs);

    // check castle modificator
    const Castle* castle = inCastle();

    if(castle)
	result += castle->GetMoraleModificator(strs);

    // army modificator
    if(GetArmy().AllTroopsIsRace(Race::NECR))
    {
	if(strs) strs->clear();
	result = 0;
    }

    result += GetArmy().GetMoraleModificator(strs);

    return result;
}

s8 HeroBase::GetLuckModificator(bool shipmaster, std::string* strs) const
{
    ArtifactsModifiers modifiers;

    modifiers.reserve(6);

    modifiers.push_back(std::make_pair(Artifact::RABBIT_FOOT, 1));
    modifiers.push_back(std::make_pair(Artifact::GOLDEN_HORSESHOE, 1));
    modifiers.push_back(std::make_pair(Artifact::GAMBLER_LUCKY_COIN, 1));
    modifiers.push_back(std::make_pair(Artifact::FOUR_LEAF_CLOVER, 1));
    modifiers.push_back(std::make_pair(Artifact::BATTLE_GARB, 10));
    if(shipmaster) modifiers.push_back(std::make_pair(Artifact::MASTHEAD, 1));

    s8 result = GetResultModifiers(modifiers, *this, strs);

    // check castle modificator
    const Castle* castle = inCastle();

    if(castle)
	result += castle->GetLuckModificator(strs);

    // army modificator
    result += GetArmy().GetLuckModificator(strs);

    return result;
}

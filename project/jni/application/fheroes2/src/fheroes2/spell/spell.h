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
#ifndef H2SPELL_H
#define H2SPELL_H

#include <string>
#include "gamedefs.h"

class HeroBase;

class Spell
{
public:
    enum spell_t
    {
	NONE		= 0,
	FIREBALL,
	FIREBLAST,
	LIGHTNINGBOLT,
	CHAINLIGHTNING,
	TELEPORT,
	CURE,
	MASSCURE,
	RESURRECT,
	RESURRECTTRUE,
	HASTE,
	MASSHASTE,
	SLOW,
	MASSSLOW,
	BLIND,
	BLESS,
	MASSBLESS,
	STONESKIN,
	STEELSKIN,
	CURSE,
	MASSCURSE,
	HOLYWORD,
	HOLYSHOUT,
	ANTIMAGIC,
	DISPEL,
	MASSDISPEL,
	ARROW,
	BERSERKER,
	ARMAGEDDON,
	ELEMENTALSTORM,
	METEORSHOWER,
	PARALYZE,
	HYPNOTIZE,
	COLDRAY,
	COLDRING,
	DISRUPTINGRAY,
	DEATHRIPPLE,
	DEATHWAVE,
	DRAGONSLAYER,
	BLOODLUST,
	ANIMATEDEAD,
	MIRRORIMAGE,
	SHIELD,
	MASSSHIELD,
	SUMMONEELEMENT,
	SUMMONAELEMENT,
	SUMMONFELEMENT,
	SUMMONWELEMENT,
	EARTHQUAKE,
	VIEWMINES,
	VIEWRESOURCES,
	VIEWARTIFACTS,
	VIEWTOWNS,
	VIEWHEROES,
	VIEWALL,
	IDENTIFYHERO,
	SUMMONBOAT,
	DIMENSIONDOOR,
	TOWNGATE,
	TOWNPORTAL,
	VISIONS,
	HAUNT,
	SETEGUARDIAN,
	SETAGUARDIAN,
	SETFGUARDIAN,
	SETWGUARDIAN,

	STONE
    };

    Spell();
    Spell(spell_t);
    Spell(u8);

    bool operator== (u8) const;
    bool operator!= (u8) const;
    spell_t operator() (void) const;
    spell_t GetID(void) const;

    void Set(spell_t);

    const char* GetName(void) const;
    const char* GetDescription(void) const;

    u8 GetCostManaPoints(void) const;
    u8 GetLevel(void) const;
    u8 GetDamage(void) const;
    bool isCombat(void) const;
    bool isAdventure(void) const;
    bool isDamage(void) const;
    bool isMindInfluence(void) const;

    /* return index sprite spells.icn */
    u8 GetIndexSprite(void) const;
    /* return index in spellinl.icn */
    u8 GetInlIndexSprite(void) const;

    static spell_t FromInt(u8);
    static u8 Level(u8);
    static bool isCombat(u8);
    static u8 Damage(u8);
    static u8 Restore(u8);
    static u8 Resurrect(u8);
    static bool isDamage(u8);
    static bool isRestore(u8);
    static bool isResurrect(u8);
    static bool isMindInfluence(u8);
    static bool isUndeadOnly(u8);
    static bool isALiveOnly(u8);
    static bool isSummon(u8);

    static bool isApplyWithoutFocusObject(u8);
    static bool isApplyToAnyTroops(u8);
    static bool isApplyToFriends(u8);
    static bool isApplyToEnemies(u8);
    static bool isMassActions(u8);
    static bool isRaceCompatible(u8 spell, u8 race);

    static bool AllowWithWisdom(u8 spell, u8 wisdom);

    /* return index sprite spells.icn */
    static u8 IndexSprite(spell_t);
    /* return index in spellinl.icn */
    static u8 InlIndexSprite(u8);

    static u8 CostManaPoints(spell_t, const HeroBase* hero = NULL);
    static u8 GetExtraValue(spell_t);
    static const char* GetName(spell_t);
    static const char* GetDescription(spell_t);

    static spell_t RandCombat(u8);
    static spell_t RandAdventure(u8);

    static void UpdateStats(const std::string &);
    static u8 CalculateDimensionDoorDistance(u8 current_sp, u32 total_hp);

private:
    spell_t id;
    
    static u16 dd_distance;
    static u16 dd_sp;
    static u16 dd_hp;
};

#endif


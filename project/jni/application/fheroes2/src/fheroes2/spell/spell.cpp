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

#include "heroes_base.h"
#include "artifact.h"
#include "settings.h"
#include "spell.h"
#include "army.h"

#ifdef WITH_XML
#include "xmlccwrap.h"
#endif

struct spellstats_t
{
        const char* name;
        u8 mana;
        u8 sprite;
        u8 extra;
        const char* description;
};

u16 Spell::dd_distance = 0;
u16 Spell::dd_sp = 0;
u16 Spell::dd_hp = 0;

static spellstats_t spells[] = {
	//  name                      mana spr value description
	{ "Unknown",                    0,  0,     0, "Unknown spell." },
	{ _("Fireball"),                9,  8,    10, _("Causes a giant fireball to strike the selected area, damaging all nearby creatures.") },
	{ _("Fireblast"),              15,  9,    10, _("An improved version of fireball, fireblast affects two hexes around the center point of the spell, rather than one.") }, 
	{ _("Lightning Bolt"),          7,  4,    25, _("Causes a bolt of electrical energy to strike the selected creature.") }, 
	{ _("Chain Lightning"),        15,  5,    40, _("Causes a bolt of electrical energy to strike a selected creature, then strike the nearest creature with half damage, then strike the NEXT nearest creature with half again damage, and so on, until it becomes too weak to be harmful.  Warning:  This spell can hit your own creatures!") }, 
	{ _("Teleport"),                9, 10,     0, _("Teleports the creature you select to any open position on the battlefield.") }, 
	{ _("Cure"),                    6,  6,     5, _("Removes all negative spells cast upon one of your units, and restores up to %{count} HP per level of spell power.") }, 
	{ _("Mass Cure"),              15,  2,     5, _("Removes all negative spells cast upon your forces, and restores up to %{count} HP per level of spell power, per creature.") }, 
	{ _("Resurrect"),              12, 13,    50, _("Resurrects creatures from a damaged or dead unit until end of combat.") }, 
	{ _("Resurrect True"),         15, 12,    50, _("Resurrects creatures from a damaged or dead unit permanently.") }, 
	// extra: 0, because also used Speed:GetOriginal (see Battle2::Stats::GetSpeed)
	{ _("Haste"),                   3, 14,     0, _("Increases the speed of any creature by %{count}.") }, 
	{ _("Mass Haste"),             10, 14,     0, _("Increases the speed of all of your creatures by %{count}.") }, 
	{ _("spell|Slow"),              3,  1,     0, _("Slows target to half movement rate.") }, 
	{ _("Mass Slow"),              15,  1,     0, _("Slows all enemies to half movement rate.") }, 
	//
	{ _("Blind "),                  6, 21,     0, _("Clouds the affected creatures' eyes, preventing them from moving.") }, 
	{ _("Bless"),                   3,  7,     0, _("Causes the selected creatures to inflict maximum damage.") }, 
	{ _("Mass Bless"),             12,  7,     0, _("Causes all of your units to inflict maximum damage.") },
	{ _("Stoneskin"),               3, 31,     3, _("Magically increases the defense skill of the selected creatures.") },
	{ _("Steelskin"),               6, 30,     5, _("Increases the defense skill of the targeted creatures.  This is an improved version of Stoneskin.") },
	{ _("Curse"),                   3,  3,     0, _("Causes the selected creatures to inflict minimum damage.") },
	{ _("Mass Curse"),             12,  3,     0, _("Causes all enemy troops to inflict minimum damage.") },
	{ _("Holy Word"),               9, 22,    10, _("Damages all undead in the battle.") },
	{ _("Holy Shout"),             12, 23,    20, _("Damages all undead in the battle.  This is an improved version of Holy Word.") },
	{ _("Anti-Magic"),              7, 17,     0, _("Prevents harmful magic against the selected creatures.") },
	{ _("Dispel Magic"),            5, 18,     0, _("Removes all magic spells from a single target.") },
	{ _("Mass Dispel"),            12, 18,     0, _("Removes all magic spells from all creatures.") },
	{ _("Magic Arrow"),             3, 38,    10, _("Causes a magic arrow to strike the selected target.") },
	{ _("Berserker"),              12, 19,     0, _("Causes a creature to attack its nearest neighbor.") },
	{ _("Armageddon"),             20, 16,    50, _("Holy terror strikes the battlefield, causing severe damage to all creatures.") },
	{ _("Elemental Storm"),        15, 11,    25, _("Magical elements pour down on the battlefield, damaging all creatures.") },
	{ _("Meteor Shower"),          15, 24,    25, _("A rain of rocks strikes an area of the battlefield, damaging all nearby creatures.") },
	{ _("Paralyze"),                9, 20,     0, _("The targeted creatures are paralyzed, unable to move or retaliate.") },
	{ _("Hypnotize"),              15, 37,    25, _("Brings a single enemy unit under your control for one combat round if its hits are less than %{count} times the caster's spell power.") },
	{ _("Cold Ray"),                6, 36,    20, _("Drains body heat from a single enemy unit.") },
	{ _("Cold Ring"),               9, 35,    10, _("Drains body heat from all units surrounding the center point, but not including the center point.") },
	{ _("Disrupting Ray"),          7, 34,     3, _("Reduces the defense rating of an enemy unit by three.") },
	{ _("Death Ripple"),            6, 28,     5, _("Damages all living (non-undead) units in the battle.") },
	{ _("Death Wave"),             10, 29,    10, _("Damages all living (non-undead) units in the battle.  This spell is an improved version of Death Ripple.") },
	{ _("Dragon Slayer"),           6, 32,     5, _("Greatly increases a unit's attack skill vs. Dragons.") },
	{ _("Blood Lust"),              3, 27,     3, _("Increases a unit's attack skill.") },
	{ _("Animate Dead"),           10, 25,    50, _("Resurrects creatures from a damaged or dead undead unit permanently.") },
	{ _("Mirror Image"),           25, 26,     0, _("Creates an illusionary unit that duplicates one of your existing units.  This illusionary unit does the same damages as the original, but will vanish if it takes any damage.") },
	{ _("Shield"),                  3, 15,     2, _("Halves damage received from ranged attacks for a single unit.") },
	{ _("Mass Shield"),             7, 15,     0, _("Halves damage received from ranged attacks for all of your units.") },
	{ _("Summon Earth Elemental"), 30, 56,     3, _("Summons Earth Elementals to fight for your army.") },
	{ _("Summon Air Elemental"),   30, 57,     3, _("Summons Air Elementals to fight for your army.") },
	{ _("Summon Fire Elemental"),  30, 58,     3, _("Summons Fire Elementals to fight for your army.") },
	{ _("Summon Water Elemental"), 30, 59,     3, _("Summons Water Elementals to fight for your army.") },
	{ _("Earthquake"),             15, 33,     0, _("Damages castle walls.") },
	{ _("View Mines"),              1, 39,     0, _("Causes all mines across the land to become visible.") },
	{ _("View Resources"),          1, 40,     0, _("Causes all resources across the land to become visible.") },
	{ _("View Artifacts"),          2, 41,     0, _("Causes all artifacts across the land to become visible.") },
	{ _("View Towns"),              2, 42,     0, _("Causes all towns and castles across the land to become visible.") },
	{ _("View Heroes"),             2, 43,     0, _("Causes all Heroes across the land to become visible.") },
	{ _("View All"),                3, 44,     0, _("Causes the entire land to become visible.") },
	{ _("Identify Hero"),           3, 45,     0, _("Allows the caster to view detailed information on enemy Heroes.") },
	{ _("Summon Boat"),             5, 46,     0, _("Summons the nearest unoccupied, friendly boat to an adjacent shore location.  A friendly boat is one which you just built or were the most recent player to occupy.") },
	{ _("Dimension Door"),         10, 47,     0, _("Allows the caster to magically transport to a nearby location.") },
	{ _("Town Gate"),              10, 48,     0, _("Returns the caster to any town or castle currently owned.") },
	{ _("Town Portal"),            20, 49,     0, _("Returns the hero to the town or castle of choice, provided it is controlled by you.") },
	{ _("Visions"),                 6, 50,     3, _("Visions predicts the likely outcome of an encounter with a neutral army camp.") },
	{ _("Haunt"),                   8, 51,     4, _("Haunts a mine you control with Ghosts.  This mine stops producing resources.  (If I can't keep it, nobody will!)") },
	{ _("Set Earth Guardian"),     15, 52,     4, _("Sets Earth Elementals to guard a mine against enemy armies.") },
	{ _("Set Air Guardian"),       15, 53,     4, _("Sets Air Elementals to guard a mine against enemy armies.") },
	{ _("Set Fire Guardian"),      15, 54,     4, _("Sets Fire Elementals to guard a mine against enemy armies.") },
	{ _("Set Water Guardian"),     15, 55,     4, _("Sets Water Elementals to guard a mine against enemy armies.") },
	{ _("Stone"),  		        0,  0,     0, _("Stone spell from Medusa.") },
};

void Spell::UpdateStats(const std::string & spec)
{
#ifdef WITH_XML
    // parse spells.xml
    TiXmlDocument doc;
    const TiXmlElement* xml_spells = NULL;
    spellstats_t* ptr = &spells[0];

    if(doc.LoadFile(spec.c_str()) &&
        NULL != (xml_spells = doc.FirstChildElement("spells")))
    {
        const TiXmlElement* xml_spell = xml_spells->FirstChildElement("spell");
        for(; xml_spell; xml_spell = xml_spell->NextSiblingElement("spell"))
        {
            int value;

            xml_spell->Attribute("skip", &value);
            if(0 == value)
            {
		xml_spell->Attribute("cost", &value); if(value) ptr->mana = value;
		xml_spell->Attribute("extra", &value); if(value) ptr->extra = value;
	    }

            // load dimension door params
            if((ptr - &spells[0]) == DIMENSIONDOOR)
            {
		xml_spell->Attribute("distance", &value); dd_distance = value;
		xml_spell->Attribute("sp", &value); dd_sp = value;
		xml_spell->Attribute("hp", &value); dd_hp = value;
            }

	    ++ptr;

            // out of range
            if((ptr - &spells[0]) >= STONE) break;
	}
    }
    else
    VERBOSE(spec << ": " << doc.ErrorDesc());
#endif
}

Spell::Spell() : id(NONE)
{
}

Spell::Spell(spell_t s) : id(s)
{
}

Spell::Spell(u8 s) : id(FromInt(s))
{
}

bool Spell::operator== (u8 s) const
{
    return s == id;
}

bool Spell::operator!= (u8 s) const
{
    return id != s;
}

Spell::spell_t Spell::operator() (void) const
{
    return id;
}

Spell::spell_t Spell::GetID(void) const
{
    return id;
}

void Spell::Set(spell_t s)
{
    id = s;
}

const char* Spell::GetName(void) const
{
    return GetName(id);
}

const char* Spell::GetDescription(void) const
{
    return GetDescription(id);
}

u8 Spell::GetCostManaPoints(void) const
{
    return CostManaPoints(id);
}

u8 Spell::GetLevel(void) const
{
    return Level(id);
}

bool Spell::isCombat(void) const
{
    return isCombat(id);
}

bool Spell::isAdventure(void) const
{
    return !isCombat(id);
}

bool Spell::isDamage(void) const
{
    return isDamage(id);
}

u8 Spell::GetDamage(void) const
{
    return Damage(id);
}

bool Spell::isMindInfluence(void) const
{
    return isMindInfluence(id);
}


/* ---------- */


u8 Spell::GetIndexSprite(void) const
{
    return IndexSprite(id);
}

u8 Spell::GetInlIndexSprite(void) const
{
    return InlIndexSprite(id);
}

Spell::spell_t Spell::FromInt(u8 index)
{
    return index > STONE ? NONE : static_cast<spell_t>(index);
}

u8 Spell::CostManaPoints(spell_t spell, const HeroBase* hero)
{
    if(hero)
    {
	switch(spell)
	{
	    case BLESS:
	    case MASSBLESS:
	    if(hero->HasArtifact(Artifact::SNAKE_RING)) return spells[spell].mana / 2;
	    break;

	    case SUMMONEELEMENT:
	    case SUMMONAELEMENT:
	    case SUMMONFELEMENT:
	    case SUMMONWELEMENT:
	    if(hero->HasArtifact(Artifact::ELEMENTAL_RING)) return spells[spell].mana / 2;
	    break;

	    case CURSE:
	    case MASSCURSE:
	    if(hero->HasArtifact(Artifact::EVIL_EYE)) return spells[spell].mana / 2;
	    break;

	    default: break;
	}

	if(isMindInfluence(spell) && hero->HasArtifact(Artifact::SKULLCAP)) return spells[spell].mana / 2;
    }

    return spells[spell].mana;
}

u8 Spell::Level(u8 spell)
{
    switch(spell)
    {
	case BLESS:
	case BLOODLUST:
	case CURE:
	case CURSE:
	case DISPEL:
	case HASTE:
	case ARROW:
	case SHIELD:
	case SLOW:
	case STONESKIN:

	case VIEWMINES:
	case VIEWRESOURCES:
		return 1;

	case BLIND:
	case COLDRAY:
	case DEATHRIPPLE:
	case DISRUPTINGRAY:
	case DRAGONSLAYER:
	case LIGHTNINGBOLT:
	case STEELSKIN:

	case HAUNT:
	case SUMMONBOAT:
	case VIEWARTIFACTS:
	case VISIONS:
		return 2;

	case ANIMATEDEAD:
	case ANTIMAGIC:
	case COLDRING:
	case DEATHWAVE:
	case EARTHQUAKE:
	case FIREBALL:
	case HOLYWORD:
	case MASSBLESS:
	case MASSCURSE:
	case MASSDISPEL:
	case MASSHASTE:
	case PARALYZE:
	case TELEPORT:

	case IDENTIFYHERO:
	case VIEWHEROES:
	case VIEWTOWNS:
		return 3;

	case BERSERKER:
	case CHAINLIGHTNING:
	case ELEMENTALSTORM:
	case FIREBLAST:
	case HOLYSHOUT:
	case MASSCURE:
	case MASSSHIELD:
	case MASSSLOW:
	case METEORSHOWER:
	case RESURRECT:

	case SETEGUARDIAN:
	case SETAGUARDIAN:
	case SETFGUARDIAN:
	case SETWGUARDIAN:
	case TOWNGATE:
	case VIEWALL:
		return 4;

	case ARMAGEDDON:
	case HYPNOTIZE:
	case MIRRORIMAGE:
	case RESURRECTTRUE:
	case SUMMONEELEMENT:
	case SUMMONAELEMENT:
	case SUMMONFELEMENT:
	case SUMMONWELEMENT:

	case DIMENSIONDOOR:
	case TOWNPORTAL:
		return 5;

	default: break;
    }

    return 0;
}

bool Spell::isCombat(u8 spell)
{
    switch(spell)
    {
	case NONE:
	case VIEWMINES:
	case VIEWRESOURCES:
	case VIEWARTIFACTS:
	case VIEWTOWNS:
	case VIEWHEROES:
	case VIEWALL:
	case IDENTIFYHERO:
	case SUMMONBOAT:
	case DIMENSIONDOOR:
	case TOWNGATE:
	case TOWNPORTAL:
	case VISIONS:
	case HAUNT:
	case SETEGUARDIAN:
	case SETAGUARDIAN:
	case SETFGUARDIAN:
	case SETWGUARDIAN:	return false;
	default: break;
    }
    return true;
}

u8 Spell::Damage(u8 spell)
{
    switch(spell)
    {
	case ARROW:
	case FIREBALL:
	case FIREBLAST:
	case LIGHTNINGBOLT:
	case COLDRING:
	case DEATHWAVE:
	case HOLYWORD:
	case CHAINLIGHTNING:
	case ARMAGEDDON:
	case ELEMENTALSTORM:
	case METEORSHOWER:
	case COLDRAY:
	case HOLYSHOUT:
	case DEATHRIPPLE:
	    return spells[spell].extra;

	default: break;
    }

    return 0;
}

bool Spell::isDamage(u8 spell)
{
    return Damage(spell);
}

u8 Spell::Restore(u8 spell)
{
    switch(spell)
    {
        case Spell::CURE:
        case Spell::MASSCURE:
	    return spells[spell].extra;

	default: break;
    }

    return Resurrect(spell);
}

u8 Spell::Resurrect(u8 spell)
{
    switch(spell)
    {
	case Spell::ANIMATEDEAD:
        case Spell::RESURRECT:
        case Spell::RESURRECTTRUE:
	    return spells[spell].extra;

	default: break;
    }

    return 0;
}

bool Spell::isRestore(u8 spell)
{
    return Restore(spell);
}

bool Spell::isResurrect(u8 spell)
{
    return Resurrect(spell);
}

u8 Spell::IndexSprite(spell_t spell)
{
    return spells[spell].sprite;
}

u8 Spell::InlIndexSprite(u8 spell)
{
    switch(spell)
    {
	case HASTE:
	case MASSHASTE:		return 0;
	case SLOW:
	case MASSSLOW:		return 1;
	case BLIND:		return 2;
	case BLESS:
	case MASSBLESS:		return 3;
	case CURSE:
	case MASSCURSE:		return 4;
	case BERSERKER:		return 5;
	case PARALYZE:		return 6;
	case HYPNOTIZE:		return 7;
	case DRAGONSLAYER:	return 8;
	case BLOODLUST:		return 9;
	case SHIELD:
	case MASSSHIELD:	return 10;
	case STONE:		return 11;
	case ANTIMAGIC:		return 12;
	case STONESKIN:		return 13;
	case STEELSKIN:		return 14;
	default: break;
    }

    return 0;
}

u8 Spell::GetExtraValue(spell_t spell)
{ 
    return spells[spell].extra;
}

const char* Spell::GetName(spell_t spell)
{ 
    return _(spells[spell].name);
}

const char* Spell::GetDescription(spell_t spell)
{
    return _(spells[spell].description);
}

Spell::spell_t Spell::RandCombat(u8 lvl)
{
    if(1 == lvl)
    switch(Rand::Get(1, 10))
    {
	case 1:  return BLESS;
	case 2:  return BLOODLUST;
	case 3:  return CURE;
	case 4:  return CURSE;
	case 5:  return DISPEL;
	case 6:  return HASTE;
	case 7:  return ARROW;
	case 8:  return SHIELD;
	case 9:  return SLOW;
	case 10: return STONESKIN;
	default: break;
    }
    else
    if(2 == lvl)
    switch(Rand::Get(1, 7))
    {
	case 1: return BLIND;
	case 2: return COLDRAY;
	case 3: return DEATHRIPPLE;
	case 4: return DISRUPTINGRAY;
	case 5: return DRAGONSLAYER;
	case 6: return LIGHTNINGBOLT;
	case 7: return STEELSKIN;
	default: break;
    }
    else
    if(3 == lvl)
    switch(Rand::Get(1, 13))
    {
	case 1: return ANIMATEDEAD;
	case 2: return ANTIMAGIC;
	case 3: return COLDRING;
	case 4: return DEATHWAVE;
	case 5: return EARTHQUAKE;
	case 6: return FIREBALL;
	case 7: return HOLYWORD;
	case 8: return MASSBLESS;
	case 9: return MASSCURSE;
	case 10: return MASSDISPEL;
	case 11: return MASSHASTE;
	case 12: return PARALYZE;
	case 13: return TELEPORT;
	default: break;
    }
    else
    if(4 == lvl)
    switch(Rand::Get(1, 10))
    {
	case 1: return BERSERKER;
	case 2: return CHAINLIGHTNING;
	case 3: return ELEMENTALSTORM;
	case 4: return FIREBLAST;
	case 5: return HOLYSHOUT;
	case 6: return MASSCURE;
	case 7: return MASSSHIELD;
	case 8: return MASSSLOW;
	case 9: return METEORSHOWER;
	case 10: return RESURRECT;
	default: break;
    }
    else
    if(5 == lvl)
    switch(Rand::Get(1, 8))
    {
	case 1: return ARMAGEDDON;
	case 2: return HYPNOTIZE;
	case 3: return MIRRORIMAGE;
	case 4: return RESURRECTTRUE;
	case 5: return SUMMONEELEMENT;
	case 6: return SUMMONAELEMENT;
	case 7: return SUMMONFELEMENT;
	case 8: return SUMMONWELEMENT;
	default: break;
    }

    return Spell::NONE;    
}

Spell::spell_t Spell::RandAdventure(u8 lvl)
{
    if(1 == lvl)
    switch(Rand::Get(1, 2))
    {
	case 1: return VIEWMINES;
	case 2: return VIEWRESOURCES;
	default: break;
    }
    else
    if(2 == lvl)
    switch(Rand::Get(1, 4))
    {
	case 1: return HAUNT;
	case 2: return SUMMONBOAT;
	case 3: return VIEWARTIFACTS;
	case 4: return VISIONS;
	default: break;
    }
    else
    if(3 == lvl)
    switch(Rand::Get(1, 3))
    {
	case 1: return IDENTIFYHERO;
	case 2: return VIEWHEROES;
	case 3: return VIEWTOWNS;
	default: break;
    }
    else
    if(4 == lvl)
    switch(Rand::Get(1, 6))
    {
	case 1: return SETEGUARDIAN;
	case 2: return SETAGUARDIAN;
	case 3: return SETFGUARDIAN;
	case 4: return SETWGUARDIAN;
	case 5: return TOWNGATE;
	case 6: return VIEWALL;

	default: break;
    }
    else
    if(5 == lvl)
    switch(Rand::Get(1, 2))
    {
	case 1: return DIMENSIONDOOR;
	case 2: return TOWNPORTAL;
	default: break;
    }

    return Spell::NONE;
}

bool Spell::isMindInfluence(u8 spell)
{
    switch(spell)
    {
	case BLIND:
	case PARALYZE:
	case BERSERKER:
	case HYPNOTIZE: return true;

	default: break;
    }

    return false;
}

bool Spell::isUndeadOnly(u8 spell)
{
    switch(spell)
    {
	case ANIMATEDEAD:
	case HOLYWORD:
	case HOLYSHOUT:
	    return true;

	default: break;
    }

    return false;
}

bool Spell::isALiveOnly(u8 spell)
{
    switch(spell)
    {
	case BLESS:
	case MASSBLESS:
	case CURSE:
	case MASSCURSE:
	case DEATHRIPPLE:
	case DEATHWAVE:
	case RESURRECT:
	case RESURRECTTRUE:
	    return true;

	default: break;
    }

    return false;
}

bool Spell::isApplyWithoutFocusObject(u8 spell)
{
    if(isMassActions(spell) || isSummon(spell))
	return true;
    else
    switch(spell)
    {
	case DEATHRIPPLE:
	case DEATHWAVE:
	case EARTHQUAKE:
	case HOLYWORD:
	case HOLYSHOUT:
        case ARMAGEDDON:
        case ELEMENTALSTORM:
	    return true;

	default: break;
    }

    return false;
}

bool Spell::isSummon(u8 spell)
{
    switch(spell)
    {
	case SUMMONEELEMENT:
        case SUMMONAELEMENT:
        case SUMMONFELEMENT:
        case SUMMONWELEMENT:
	    return true;

	default: break;
    }

    return false;
}

bool Spell::isApplyToAnyTroops(u8 spell)
{
    switch(spell)
    {
	case DISPEL:
	case MASSDISPEL:
	    return true;

	default: break;
    }

    return false;
}

bool Spell::isApplyToFriends(u8 spell)
{
    switch(spell)
    {
	case BLESS:
	case BLOODLUST:
	case CURE:
	case HASTE:
	case SHIELD:
	case STONESKIN:
	case DRAGONSLAYER:
	case STEELSKIN:
	case ANIMATEDEAD:
	case ANTIMAGIC:
	case TELEPORT:
	case RESURRECT:
	case MIRRORIMAGE:
	case RESURRECTTRUE:

	case MASSBLESS:
	case MASSCURE:
	case MASSHASTE:
	case MASSSHIELD:
	    return true;

	default: break;
    }

    return false;
}

bool Spell::isMassActions(u8 spell)
{
    switch(spell)
    {
	case MASSCURE:
	case MASSHASTE:
	case MASSSLOW:
	case MASSBLESS:
	case MASSCURSE:
	case MASSDISPEL:
	case MASSSHIELD:
		return true;

	default: break;
    }

    return false;
}

bool Spell::isApplyToEnemies(u8 spell)
{
    switch(spell)
    {
	case MASSSLOW:
	case MASSCURSE:

	case CURSE:
	case ARROW:
	case SLOW:
	case BLIND:
	case COLDRAY:
	case DISRUPTINGRAY:
	case LIGHTNINGBOLT:
	case CHAINLIGHTNING:
	case PARALYZE:
	case BERSERKER:
	case HYPNOTIZE:
	    return true;

	default: break;
    }

    return false;
}

bool Spell::isRaceCompatible(u8 spell, u8 race)
{
    switch(spell)
    {
	case MASSCURE:
	case MASSBLESS:
	case HOLYSHOUT:
	case HOLYWORD:
	case BLESS:
	case CURE:
	    if(Race::NECR == race) return false;
	    break;

	case DEATHWAVE:
	case DEATHRIPPLE:
	case ANIMATEDEAD:
	    if(Race::NECR != race) return false;
	    break;

	default:
	    break;
    }

    return true;
}

u8 Spell::CalculateDimensionDoorDistance(u8 current_sp, u32 total_hp)
{
    if(dd_distance && dd_hp && dd_sp && total_hp)
    {
	const u16 res = (dd_distance * current_sp * dd_hp) / (dd_sp * total_hp);
	return res ? (res < 255 ? res : 255) : 1;
    }
    // original h2 variant
    return 14;
}

bool Spell::AllowWithWisdom(u8 sp, u8 wisdom)
{
    return ((4 < Spell::Level(sp) && Skill::Level::EXPERT > wisdom) ||
	    (3 < Spell::Level(sp) && Skill::Level::ADVANCED > wisdom) ||
	    (2 < Spell::Level(sp) && Skill::Level::BASIC > wisdom) ? false : true);
}

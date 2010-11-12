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
#ifndef H2ARTIFACT_H
#define H2ARTIFACT_H

#include "gamedefs.h"

class Artifact
{
public:
    enum artifact_t
    {
	ULTIMATE_BOOK,
	ULTIMATE_SWORD,
	ULTIMATE_CLOAK,
	ULTIMATE_WAND,
	ULTIMATE_SHIELD,
	ULTIMATE_STAFF,
	ULTIMATE_CROWN,
	GOLDEN_GOOSE,
	ARCANE_NECKLACE,
	CASTER_BRACELET,
	MAGE_RING,
	WITCHES_BROACH,
	MEDAL_VALOR,
	MEDAL_COURAGE,
	MEDAL_HONOR,
	MEDAL_DISTINCTION,
	FIZBIN_MISFORTUNE,
	THUNDER_MACE,
	ARMORED_GAUNTLETS,
	DEFENDER_HELM,
	GIANT_FLAIL,
	BALLISTA,
	STEALTH_SHIELD,
	DRAGON_SWORD,
	POWER_AXE,
	DIVINE_BREASTPLATE,
	MINOR_SCROLL,
	MAJOR_SCROLL,
	SUPERIOR_SCROLL,
	FOREMOST_SCROLL,
	ENDLESS_SACK_GOLD,
	ENDLESS_BAG_GOLD,
	ENDLESS_PURSE_GOLD,
	NOMAD_BOOTS_MOBILITY,
	TRAVELER_BOOTS_MOBILITY,
	RABBIT_FOOT,
	GOLDEN_HORSESHOE,
	GAMBLER_LUCKY_COIN,
	FOUR_LEAF_CLOVER,
	TRUE_COMPASS_MOBILITY,
	SAILORS_ASTROLABE_MOBILITY,
	EVIL_EYE,
	ENCHANTED_HOURGLASS,
	GOLD_WATCH,
	SKULLCAP,
	ICE_CLOAK,
	FIRE_CLOAK,
	LIGHTNING_HELM,
	EVERCOLD_ICICLE,
	EVERHOT_LAVA_ROCK,
	LIGHTNING_ROD,
	SNAKE_RING,
	ANKH,
	BOOK_ELEMENTS,
	ELEMENTAL_RING,
	HOLY_PENDANT,
	PENDANT_FREE_WILL,
	PENDANT_LIFE,
	SERENITY_PENDANT,
	SEEING_EYE_PENDANT,
	KINETIC_PENDANT,
	PENDANT_DEATH,
	WAND_NEGATION,
	GOLDEN_BOW,
	TELESCOPE,
	STATESMAN_QUILL,
	WIZARD_HAT,
	POWER_RING,
	AMMO_CART,
	TAX_LIEN,
	HIDEOUS_MASK,
	ENDLESS_POUCH_SULFUR,
	ENDLESS_VIAL_MERCURY,
	ENDLESS_POUCH_GEMS,
	ENDLESS_CORD_WOOD,
	ENDLESS_CART_ORE,
	ENDLESS_POUCH_CRYSTAL,
	SPIKED_HELM,
	SPIKED_SHIELD,
	WHITE_PEARL,
	BLACK_PEARL,

	MAGIC_BOOK,

	DUMMY1,
	DUMMY2,
	DUMMY3,
	DUMMY4,

	SPELL_SCROLL,
	ARM_MARTYR,
	BREASTPLATE_ANDURAN,
	BROACH_SHIELDING,
	BATTLE_GARB,
	CRYSTAL_BALL,
	HEART_FIRE,
	HEART_ICE,
	HELMET_ANDURAN,
	HOLY_HAMMER,
	LEGENDARY_SCEPTER,
	MASTHEAD,
	SPHERE_NEGATION,
	STAFF_WIZARDRY,
	SWORD_BREAKER,
	SWORD_ANDURAN,
	SPADE_NECROMANCY,

	UNKNOWN
    };

    Artifact();
    Artifact(artifact_t);

    bool operator== (artifact_t) const;
    bool operator!= (artifact_t) const;
    artifact_t operator() (void) const;
    artifact_t GetID(void) const;

    const char* GetName(void) const;
    const char* GetDescription(void) const;

    void Set(artifact_t);

    bool isUltimate(void) const;
    bool isValid(void) const;

    /* objnarti.icn */
    u8 GetIndexSprite(void) const;
    /* artfx.icn */
    u8 GetIndexSprite32(void) const;
    /* artifact.icn */
    u8 GetIndexSprite64(void) const;

    void SetExt(u8);
    u8 GetExt(void) const;

    static artifact_t RandUltimate(void);
    static artifact_t Rand(bool uniq = true);
    static artifact_t Rand1(bool uniq = true);
    static artifact_t Rand2(bool uniq = true);
    static artifact_t Rand3(bool uniq = true);

    static const char* GetName(artifact_t);
    static const char* GetDescription(artifact_t);

    static artifact_t FromInt(u16);
    static artifact_t FromIndexSprite(u8);

    /* objnarti.icn */
    static u8 IndexSprite(artifact_t);
    /* artfx.icn */
    static u8 IndexSprite32(artifact_t);
    /* artifact.icn */
    static u8 IndexSprite64(artifact_t);

    static const char* GetScenario(artifact_t);

private:
    artifact_t id;
    u8 ext; /* spell for scrolls, and other */
};

#endif

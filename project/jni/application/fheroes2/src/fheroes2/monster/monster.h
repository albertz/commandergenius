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

#ifndef H2MONSTER_H
#define H2MONSTER_H

#include <string>
#include "skill.h"
#include "icn.h"
#include "m82.h"
#include "payment.h"
#include "gamedefs.h"

class Monster : public Skill::Primary
{
public:
    enum level_t
    {
	LEVEL0,
	LEVEL1,
	LEVEL2,
	LEVEL3,
	LEVEL4
    };

    enum monster_t
    {
	UNKNOWN,

	PEASANT,
	ARCHER,
	RANGER,
	PIKEMAN,
	VETERAN_PIKEMAN,
	SWORDSMAN,
	MASTER_SWORDSMAN,
	CAVALRY,
	CHAMPION,
	PALADIN,
	CRUSADER,
	GOBLIN,
	ORC,
	ORC_CHIEF,
	WOLF,
	OGRE,
	OGRE_LORD,
	TROLL,
	WAR_TROLL,
	CYCLOPS,
	SPRITE,
	DWARF,
	BATTLE_DWARF,
	ELF,
	GRAND_ELF,
	DRUID,
	GREATER_DRUID,
	UNICORN,
	PHOENIX,
	CENTAUR,
	GARGOYLE,
	GRIFFIN,
	MINOTAUR,
	MINOTAUR_KING,
	HYDRA,
	GREEN_DRAGON,
	RED_DRAGON,
	BLACK_DRAGON,
	HALFLING,
	BOAR,
	IRON_GOLEM,
	STEEL_GOLEM,
	ROC,
	MAGE,
	ARCHMAGE,
	GIANT,
	TITAN,
	SKELETON,
	ZOMBIE,
	MUTANT_ZOMBIE,
	MUMMY,
	ROYAL_MUMMY,
	VAMPIRE,
	VAMPIRE_LORD,
	LICH,
	POWER_LICH,
	BONE_DRAGON,

	ROGUE,
	NOMAD,
	GHOST,
	GENIE,
	MEDUSA,
	EARTH_ELEMENT,
	AIR_ELEMENT,
	FIRE_ELEMENT,
	WATER_ELEMENT,

	MONSTER_RND1,
	MONSTER_RND2,
	MONSTER_RND3,
	MONSTER_RND4,
	MONSTER_RND
    };

    Monster();
    Monster(monster_t);
    Monster(u8, u32);

    bool operator== (monster_t) const;
    bool operator!= (monster_t) const;
    monster_t operator() (void) const;

    monster_t GetID(void) const;

    void Set(monster_t);
    void Set(const Monster &);
    void Upgrade(void);

    u8 GetAttack(void) const;
    u8 GetDefense(void) const;
    u8 GetPower(void) const;
    u8 GetKnowledge(void) const;
    s8 GetMorale(void) const;
    s8 GetLuck(void) const;
    Race::race_t GetRace(void) const;
    u8 GetType(void) const;

    u8  GetDamageMin(void) const;
    u8  GetDamageMax(void) const;
    u8  GetShots(void) const;
    u16 GetHitPoints(void) const;
    u8  GetSpeed(void) const;
    u8  GetGrown(void) const;
    u8  GetLevel(void) const;
    u16 GetRNDSize(bool skip) const;

    const char* GetName(void) const;
    const char* GetMultiName(void) const;
    const char* GetPluralName(u32) const;

    bool isElemental(void) const;
    bool isUndead(void) const;
    bool isFly(void) const;
    bool isWide(void) const;
    bool isArchers(void) const;
    bool isAllowUpgrade(void) const;
    bool isTwiceAttack(void) const;
    bool isDragons(void) const;

    ICN::icn_t ICNMonh(void) const;

    static monster_t Upgrade(Monster &);
    static u8  GetLevel(Monster &);
    static u32 GetDwelling(Monster &);

    static u8 GetSpriteIndex(u8);
    static const char* GetName(monster_t);
    static const char* GetMultiName(monster_t);
    static const char* GetPluralName(monster_t, u32);
    static monster_t Upgrade(monster_t);
    static u8  GetLevel(monster_t);
    static u32 GetDwelling(monster_t);
    static u32 GetCountFromHitPoints(monster_t, u32);

    static void GetCost(u8, payment_t &);
    static void GetUpgradeCost(u8, payment_t &);

    static monster_t FromInt(u8);
    static monster_t FromDwelling(u8, u32);
    static monster_t FromObject(u8);
    static monster_t Rand(level_t = LEVEL0);

    static void UpdateStats(const std::string &);

protected:
    monster_t id;
};

#endif

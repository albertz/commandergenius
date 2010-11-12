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
#ifndef H2CASTLE_H
#define H2CASTLE_H

#include <vector>
#include <bitset>
#include <string>
#include "mageguild.h"
#include "color.h"
#include "captain.h"
#include "dialog.h"
#include "race.h"
#include "army.h"
#include "bitmodes.h"
#include "heroes.h"
#include "game_io.h"
#include "position.h"

class Heroes;
namespace Maps { class Tiles; }

enum building_t
{
    BUILD_NOTHING           = 0x00000000,
    BUILD_THIEVESGUILD      = 0x00000001,
    BUILD_TAVERN            = 0x00000002,
    BUILD_SHIPYARD          = 0x00000004,
    BUILD_WELL              = 0x00000008,
    BUILD_STATUE            = 0x00000010,
    BUILD_LEFTTURRET        = 0x00000020,
    BUILD_RIGHTTURRET       = 0x00000040,
    BUILD_MARKETPLACE       = 0x00000080,
    BUILD_WEL2              = 0x00000100,   // Farm, Garbage He, Crystal Gar, Waterfall, Orchard, Skull Pile
    BUILD_MOAT              = 0x00000200,
    BUILD_SPEC              = 0x00000400,   // Fortification, Coliseum, Rainbow, Dungeon, Library, Storm
    BUILD_CASTLE            = 0x00000800,
    BUILD_CAPTAIN           = 0x00001000,
    BUILD_SHRINE            = 0x00002000,
    BUILD_MAGEGUILD1        = 0x00004000,
    BUILD_MAGEGUILD2        = 0x00008000,
    BUILD_MAGEGUILD3        = 0x00010000,
    BUILD_MAGEGUILD4        = 0x00020000,
    BUILD_MAGEGUILD5        = 0x00040000,
    BUILD_TENT              = 0x00080000,	// deprecated
    DWELLING_MONSTER1       = 0x00100000,
    DWELLING_MONSTER2       = 0x00200000,
    DWELLING_MONSTER3       = 0x00400000,
    DWELLING_MONSTER4       = 0x00800000,
    DWELLING_MONSTER5       = 0x01000000,
    DWELLING_MONSTER6       = 0x02000000,
    DWELLING_UPGRADE2       = 0x04000000,
    DWELLING_UPGRADE3       = 0x08000000,
    DWELLING_UPGRADE4       = 0x10000000,
    DWELLING_UPGRADE5       = 0x20000000,
    DWELLING_UPGRADE6       = 0x40000000,
    DWELLING_UPGRADE7       = 0x80000000        // black dragon
};

class Castle : public Maps::Position, public BitModes
{
public:
    enum flags_t
    {
	ALLOWCASTLE		= 0x0002,
	//			= 0x0004,
	ALLOWBUILD		= 0x0008,
	//			- 0x0010,
	CAPITAL			= 0x0020
    };

    Castle();
    Castle(s16 cx, s16 cy, const Race::race_t rs);
    void LoadFromMP2(const void *ptr);

    Captain &GetCaptain() { return captain; }

    bool isCastle(void) const{ return building & BUILD_CASTLE; }
    bool isCapital(void) const{ return Modes(CAPITAL); }
    bool HaveNearlySea(void) const;
    bool PresentBoat(void) const;
    bool RecruitMonster(u32 dw, u16 count);
    bool AllowBuyHero(const Heroes &);
    bool ContainCoord(const u16 ax, const u16 ay) const;
    bool isNecromancyShrineBuild(void) const;

    u32 CountBuildings(void) const;

    bool RecruitHero(Heroes*);
    const Heroes* GetHeroes(void) const;
    Heroes* GetHeroes(void);

    Race::race_t GetRace(void) const{ return race; }
    Color::color_t GetColor(void) const{ return color; }
    const std::string & GetName(void) const{ return name; }
    u8 GetControl(void) const;
    u8 GetLevelMageGuild(void) const;
    const MageGuild & GetMageGuild(void) const;

    const Army::army_t & GetArmy(void) const;
    Army::army_t & GetArmy(void);
    const Army::army_t & GetActualArmy(void) const;
    Army::army_t & GetActualArmy(void);
    void MergeArmies(void);
    u16 GetDwellingLivedCount(u32) const;
    u32 GetActualDwelling(u32) const;
    void RecruitAllMonster(void);

    void ChangeColor(Color::color_t cl);

    void ActionNewDay(void);
    void ActionNewWeek(void);
    void ActionNewMonth(void);

    void DrawImageCastle(const Point & pt);

    Dialog::answer_t OpenDialog(bool readonly = false, bool fade = false);

    s8 GetAttackModificator(std::string *strs) const;
    s8 GetDefenseModificator(std::string *strs) const;
    s8 GetPowerModificator(std::string *strs) const;
    s8 GetKnowledgeModificator(std::string *strs) const;
    s8 GetMoraleModificator(std::string *strs) const;
    s8 GetLuckModificator(std::string *strs) const;

    bool AllowBuild(void) const{ return Modes(ALLOWBUILD); }
    bool AllowBuyBuilding(u32) const;
    bool isBuild(u32 bd) const{ return building & bd; }
    bool BuyBuilding(u32);
    bool AllowBuyBoat(void) const;
    bool BuyBoat(void);
    u32 GetBuildingRequires(u32) const;

    void Scoute(void) const;
    
    static const char* GetStringBuilding(u32, Race::race_t = Race::BOMG);
    static const char* GetDescriptionBuilding(u32, Race::race_t = Race::BOMG);
    static ICN::icn_t GetICNBuilding(u32, Race::race_t);
    static ICN::icn_t GetICNBoat(const Race::race_t & race);
    u32 GetUpgradeBuilding(u32) const;
    
    static bool PredicateIsCastle(const Castle *castle);
    static bool PredicateIsTown(const Castle *castle);
    static bool PredicateIsBuildMarketplace(const Castle *castle);
    static bool PredicateIsCapital(const Castle *castle);

    static u8 GetGrownWell(void);
    static u8 GetGrownWel2(void);
#ifdef WITH_XML
    static void UpdateExtraGrowth(const TiXmlElement*);
#endif

    void Dump(void) const;

    void AIDefense(void);
    void AIDevelopment(void);
    void AIJoinRNDArmy(void);

    Dialog::answer_t DialogBuyHero(const Heroes*);
    Dialog::answer_t DialogBuyCaptain(bool fixed = true) const;
    Dialog::answer_t DialogBuyCastle(bool fixed = true) const;

private:
    void RedrawResourcePanel(const Point &);
    Rect GetCoordBuilding(building_t building, const Point & pt);
    u32  OpenTown(void);
    void OpenTavern(void);
    void OpenThievesGuild(void);
    void OpenWell(void);
    void OpenMageGuild(void);
    void WellRedrawInfoArea(const Point & cur_pt);
    static void RedrawAllBuilding(const Castle &, const Point &, const std::vector<building_t> &);
    static void RedrawAnimationBuilding(const Castle &, const Point &, const std::vector<building_t> &, u32 build);

private:
    friend class Game::IO;

    Race::race_t	race;
    Captain		captain;

    Color::color_t	color;
    std::string		name;
    u32			building;

    MageGuild		mageguild;
    u16			dwelling[CASTLEMAXMONSTER];
    Army::army_t        army;
    Heroes * 		castle_heroes;
    
    static u8 grown_well;
    static u8 grown_wel2;
};

#endif

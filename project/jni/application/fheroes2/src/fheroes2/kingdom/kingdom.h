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
#ifndef H2KINGDOM_H
#define H2KINGDOM_H

#include <vector>
#include <map>
#include "race.h"
#include "color.h"
#include "payment.h"
#include "puzzle.h"
#include "game.h"
#include "mp2.h"
#include "pairs.h"
#include "game_io.h"
#include "heroes_recruits.h"

class Castle;
class Heroes;

class Kingdom
{
public:
    enum flags_t
    {
	PLAY	     = 0x0001,
	IDENTIFYHERO = 0x0002
    };

    Kingdom();
    Kingdom(const Color::color_t cl);

    void SetModes(flags_t);
    void ResetModes(flags_t);
    bool Modes(flags_t) const;

    Game::control_t Control(void) const{ return control; }
    void SetControl(const Game::control_t con) { control = con; }
    void UpdateStartingResource(void);
    bool isPlay(void) const{ return Modes(PLAY); }
    bool isLoss(void) const;
    bool AllowPayment(const Resource::funds_t & funds) const;
    bool AllowRecruitHero(bool check_payment, u8 level) const;

    Color::color_t GetColor(void) const{ return color; }
    Race::race_t GetRace(void) const;

    const Resource::funds_t & GetFundsResource(void) const{ return resource; }
    u16 GetFundsWood(void) const{ return resource.wood; }
    u16 GetFundsMercury(void) const{ return resource.mercury; }
    u16 GetFundsOre(void) const{ return resource.ore; }
    u16 GetFundsSulfur(void) const{ return resource.sulfur; }
    u16 GetFundsCrystal(void) const{ return resource.crystal; }
    u16 GetFundsGems(void) const{ return resource.gems; }
    u32 GetFundsGold(void) const{ return resource.gold; }

    u32 GetIncome(void);
    const Heroes* GetBestHero(void) const;
    u32 GetArmiesStrength(void) const;

    void AddFundsResource(const Resource::funds_t & funds);
    void OddFundsResource(const Resource::funds_t & funds);

    u8 GetCountCastle(void) const;
    u8 GetCountTown(void) const;
    u8 GetCountMarketplace(void) const;
    u8 GetCountCapital(void) const;
    u8 GetLostTownDays(void) const;
    u8 GetCountNecromancyShrineBuild(void) const;
    u8 GetCountBuilding(u32) const;

    Recruits & GetRecruits(void);

    const std::vector<Heroes *> & GetHeroes(void) const{ return heroes; }
    const std::vector<Castle *> & GetCastles(void) const{ return castles; }

    std::vector<Heroes *> & GetHeroes(void) { return heroes; }
    std::vector<Castle *> & GetCastles(void) { return castles; }

    void AddHeroes(const Heroes *hero);
    void RemoveHeroes(const Heroes *hero);
    void ApplyPlayWithStartingHero(void);
    void HeroesActionNewPosition(void);

    void AddCastle(const Castle *castle);
    void RemoveCastle(const Castle *castle);
    
    void AITurns(void);
    void ActionBeforeTurn(void);
    void ActionNewDay(void);
    void ActionNewWeek(void);
    void ActionNewMonth(void);

    void SetVisited(const s32 index, const MP2::object_t object = MP2::OBJ_ZERO);
    u16  CountVisitedObjects(const MP2::object_t) const;
    bool isVisited(const u8 object) const;
    bool isVisited(const Maps::Tiles &) const;
    bool isVisited(s32, u8) const;

    bool HeroesMayStillMove(void) const;

    const Puzzle & PuzzleMaps(void) const;
    Puzzle & PuzzleMaps(void);

    void SetVisitTravelersTent(u8);
    bool IsVisitTravelersTent(u8) const;

    void Dump(void) const;

#ifdef WITH_XML
    static void UpdateStartingResource(const TiXmlElement*);
#endif

    static void SetMaxHeroes(u8);
    static u8 GetMaxHeroes(void);

private:
    friend class Game::IO;

    void UpdateRecruits(void);
    void LossPostActions(void);

    void AICastlesTurns(void);
    void AIHeroesTurns(Heroes &);
    void AIHeroesNoGUITurns(Heroes &);
    void AIHeroesGetTask(Heroes &);
    void AIHeroesPrepareTask(Heroes &);
    void AIDumpCacheObjects(const IndexDistance &) const;

    Color::color_t color;
    Game::control_t control;
    Resource::funds_t resource;

    u16 flags;
    u8  lost_town_days;

    std::vector<Castle *> castles;
    std::vector<Heroes *> heroes;

    Recruits recruits;

    Castle *ai_capital;
    std::map<s32, MP2::object_t> ai_objects;

    std::list<IndexObject> visit_object;

    Puzzle puzzle_maps;
    u8 visited_tents_colors;

    static cost_t starting_resource[];

    static u8 max_heroes;
};

#endif

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

#include <cstring>
#include <algorithm>

#include "settings.h"
#include "castle.h"
#include "heroes.h"
#include "difficulty.h"
#include "gameevent.h"
#include "profit.h"
#include "world.h"
#include "visit.h"
#include "battle2.h"
#include "kingdom.h"

u8 Kingdom::max_heroes = 8;

cost_t Kingdom::starting_resource[] = {
    { 10000, 30, 10, 30, 10, 10, 10 },
    { 7500, 20, 5, 20, 5, 5, 5 },
    { 5000, 10, 2, 10, 2, 2, 2 },
    { 2500, 5, 0, 5, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    // ai resource
    { 10000, 30, 10, 30, 10, 10, 10 },
};

bool HeroesStrongestArmy(const Heroes* h1, const Heroes* h2)
{
    return h1 && h2 && h2->GetArmy().StrongerEnemyArmy(h1->GetArmy());
}

#ifdef WITH_XML
#include "xmlccwrap.h"

void Kingdom::UpdateStartingResource(const TiXmlElement* xml_resource)
{
    const TiXmlElement* xml_difficult;
    const char* ai_always = xml_resource->Attribute("ai_always");
    const char* level;

    level = "easy";
    if(NULL != (xml_difficult = xml_resource->FirstChildElement(level)))
    {
        LoadCostFromXMLElement(Kingdom::starting_resource[0], *xml_difficult);
        if(ai_always && 0 == std::strcmp(ai_always, level)) LoadCostFromXMLElement(Kingdom::starting_resource[5], *xml_difficult);
    }

    level = "normal";
    if(NULL != (xml_difficult = xml_resource->FirstChildElement(level)))
    {
	LoadCostFromXMLElement(Kingdom::starting_resource[1], *xml_difficult);
        if(ai_always && 0 == std::strcmp(ai_always, level)) LoadCostFromXMLElement(Kingdom::starting_resource[5], *xml_difficult);
    }
    
    level = "hard";
    if(NULL != (xml_difficult = xml_resource->FirstChildElement(level)))
    {
	LoadCostFromXMLElement(Kingdom::starting_resource[2], *xml_difficult);
        if(ai_always && 0 == std::strcmp(ai_always, level)) LoadCostFromXMLElement(Kingdom::starting_resource[5], *xml_difficult);
    }

    level = "expert";
    if(NULL != (xml_difficult = xml_resource->FirstChildElement(level)))
    {
	LoadCostFromXMLElement(Kingdom::starting_resource[3], *xml_difficult);
        if(ai_always && 0 == std::strcmp(ai_always, level)) LoadCostFromXMLElement(Kingdom::starting_resource[5], *xml_difficult);
    }

    level = "impossible";
    if(NULL != (xml_difficult = xml_resource->FirstChildElement(level)))
    {
	LoadCostFromXMLElement(Kingdom::starting_resource[4], *xml_difficult);
        if(ai_always && 0 == std::strcmp(ai_always, level)) LoadCostFromXMLElement(Kingdom::starting_resource[5], *xml_difficult);
    }
}
#endif

Kingdom::Kingdom()
{
}

Kingdom::Kingdom(const Color::color_t cl) : color(cl), control(Game::AI), flags(0), lost_town_days(0), ai_capital(NULL), visited_tents_colors(0)
{
    const Settings & conf = Settings::Get();

    lost_town_days = Game::GetLostTownDays() + 1;

    // set play
    if(conf.KingdomColors(color)) SetModes(PLAY);
    
    heroes.reserve(GetMaxHeroes());
    castles.reserve(15);

    // set control
    if(color & conf.PlayersColors())
    switch(Settings::Get().GameType())
    {
        default: control = Game::LOCAL; break;
#ifdef WITH_NET
        case Game::NETWORK: control = (color == conf.MyColor() ? Game::LOCAL : Game::REMOTE); break;
#endif
    }

    UpdateStartingResource();
}

void Kingdom::UpdateStartingResource(void)
{
    cost_t* sres = NULL;

    switch(Settings::Get().GameDifficulty())
    {
	case Difficulty::EASY:       sres = &starting_resource[0]; break;
	case Difficulty::NORMAL:     sres = &starting_resource[1]; break;
	case Difficulty::HARD:       sres = &starting_resource[2]; break;
	case Difficulty::EXPERT:     sres = &starting_resource[3]; break;
	case Difficulty::IMPOSSIBLE: sres = &starting_resource[4]; break;
	default: break;
    }

    if(Game::AI == control) sres = &starting_resource[5];
    if(sres) PaymentLoadCost(resource, *sres);
}

void Kingdom::SetModes(flags_t f)
{
    flags |= f;
}

void Kingdom::ResetModes(flags_t f)
{
    flags &= ~f;
}

bool Kingdom::Modes(flags_t f) const
{
    return flags & f;
}

bool Kingdom::isLoss(void) const
{
    return castles.empty() && heroes.empty();
}

void Kingdom::LossPostActions(void)
{
    if(Modes(PLAY))
    {
	ResetModes(PLAY);
	if(heroes.size())
	{
	    std::for_each(heroes.begin(), heroes.end(), std::bind2nd(std::mem_fun(&Heroes::SetFreeman), static_cast<u8>(Battle2::RESULT_LOSS)));
	    heroes.clear();
	}
	if(castles.size()) castles.clear();
	world.KingdomLoss(color);
    }
}

void Kingdom::ActionBeforeTurn(void)
{
    // rescan heroes path
    std::for_each(heroes.begin(), heroes.end(), std::mem_fun(&Heroes::RescanPath));
}

void Kingdom::ActionNewDay(void)
{
    Settings::Get().SetCurrentColor(color);

    if(isLoss() || 0 == lost_town_days)
    {
	LossPostActions();
	return;
    }

    // modes
    ResetModes(IDENTIFYHERO);

    // check lost town
    if(castles.empty()) --lost_town_days;

    // skip incomes for first day
    if(1 < world.CountDay())
    {
        // castle New Day
	std::for_each(castles.begin(), castles.end(), std::mem_fun(&Castle::ActionNewDay));

	// heroes New Day
	std::for_each(heroes.begin(), heroes.end(), std::mem_fun(&Heroes::ActionNewDay));

	// captured object
	resource += ProfitConditions::FromMine(Resource::WOOD) * world.CountCapturedObject(MP2::OBJ_SAWMILL, color);
	resource += ProfitConditions::FromMine(Resource::ORE) * world.CountCapturedMines(Resource::ORE, color);
	resource += ProfitConditions::FromMine(Resource::MERCURY) * world.CountCapturedObject(MP2::OBJ_ALCHEMYLAB, color);
	resource += ProfitConditions::FromMine(Resource::SULFUR) * world.CountCapturedMines(Resource::SULFUR, color);
	resource += ProfitConditions::FromMine(Resource::CRYSTAL) * world.CountCapturedMines(Resource::CRYSTAL, color);
	resource += ProfitConditions::FromMine(Resource::GEMS) * world.CountCapturedMines(Resource::GEMS, color);
	resource += ProfitConditions::FromMine(Resource::GOLD) * world.CountCapturedMines(Resource::GOLD, color);

	// funds
	std::vector<Castle*>::const_iterator itc = castles.begin();
	for(; itc != castles.end(); ++itc) if(*itc)
	{
	    const Castle & castle = **itc;

	    // castle or town profit
	    resource += ProfitConditions::FromBuilding((castle.isCastle() ? BUILD_CASTLE : BUILD_TENT), 0);

	    // statue
	    if(castle.isBuild(BUILD_STATUE))
		resource += ProfitConditions::FromBuilding(BUILD_STATUE, 0);

	    // dungeon for warlock
	    if(castle.isBuild(BUILD_SPEC) && Race::WRLK == castle.GetRace())
		resource += ProfitConditions::FromBuilding(BUILD_SPEC, Race::WRLK);
	}
    }

    // check event day
    std::vector<GameEvent::Day *> events;
    events.reserve(5);
    world.GetEventDay(color, events);
    std::vector<GameEvent::Day *>::const_iterator it1 = events.begin();
    std::vector<GameEvent::Day *>::const_iterator it2 = events.end();

    for(; it1 != it2; ++it1) if(*it1) AddFundsResource((*it1)->GetResource());

    // remove day visit object
    visit_object.remove_if(Visit::isDayLife);
}

void Kingdom::ActionNewWeek(void)
{
    // skip first day
    if(1 < world.CountDay())
    {
	// castle New Week
	std::for_each(castles.begin(), castles.end(), std::mem_fun(&Castle::ActionNewWeek));

	// heroes New Week
	std::for_each(heroes.begin(), heroes.end(), std::mem_fun(&Heroes::ActionNewWeek));

	// debug an gift
	if(IS_DEVEL() && Game::LOCAL == Control())
	{
	    DEBUG(DBG_GAME , DBG_INFO, "Kingdom::ActionNewWeek: for the best debugging, God has sent you a gift.");

	    resource.wood += 20;
	    resource.ore += 20;
	    resource.sulfur += 10;
	    resource.crystal += 10;
	    resource.gems += 10;
	    resource.mercury += 10;
	    resource.gold += 5000;
	}
    }

    // remove week visit object
    visit_object.remove_if(Visit::isWeekLife);

    UpdateRecruits();
}

void Kingdom::ActionNewMonth(void)
{
    // skip first day
    if(1 < world.CountDay())
    {
	// castle New Month
	std::for_each(castles.begin(), castles.end(), std::mem_fun(&Castle::ActionNewMonth));

	// heroes New Month
	std::for_each(heroes.begin(), heroes.end(), std::mem_fun(&Heroes::ActionNewMonth));
    }

    // remove week visit object
    visit_object.remove_if(Visit::isMonthLife);
}

void Kingdom::AddHeroes(const Heroes *hero)
{
    if(hero && heroes.end() == std::find(heroes.begin(), heroes.end(), hero))
	heroes.push_back(const_cast<Heroes *>(hero));
}

void Kingdom::RemoveHeroes(const Heroes *hero)
{
    if(hero && heroes.size())
	heroes.erase(std::find(heroes.begin(), heroes.end(), hero));

    if(isLoss()) LossPostActions();
}

void Kingdom::AddCastle(const Castle *castle)
{
    if(castle && castles.end() == std::find(castles.begin(), castles.end(), castle))
	castles.push_back(const_cast<Castle *>(castle));

    lost_town_days = Game::GetLostTownDays() + 1;
}

void Kingdom::RemoveCastle(const Castle *castle)
{
    if(castle && castles.size())
	castles.erase(std::find(castles.begin(), castles.end(), castle));

    if(ai_capital == castle)
    {
	const_cast<Castle *>(castle)->ResetModes(Castle::CAPITAL);
	ai_capital = NULL;
    }

    if(isLoss()) LossPostActions();
}

u8 Kingdom::GetCountCastle(void) const
{
    return std::count_if(castles.begin(), castles.end(), Castle::PredicateIsCastle);
}

u8 Kingdom::GetCountTown(void) const
{
    return std::count_if(castles.begin(), castles.end(), Castle::PredicateIsTown);
}

u8 Kingdom::GetCountMarketplace(void) const
{
    return std::count_if(castles.begin(), castles.end(), Castle::PredicateIsBuildMarketplace);
}

u8 Kingdom::GetCountNecromancyShrineBuild(void) const
{
    return std::count_if(castles.begin(), castles.end(), std::mem_fun(&Castle::isNecromancyShrineBuild));
}

u8 Kingdom::GetCountBuilding(u32 build) const
{
    return std::count_if(castles.begin(), castles.end(), std::bind2nd(std::mem_fun(&Castle::isBuild), build));
}

Race::race_t Kingdom::GetRace(void) const
{
    return Settings::Get().KingdomRace(color);
}

bool Kingdom::AllowPayment(const Resource::funds_t & funds) const
{
    return funds > resource ? false : true;
}

/* is visited cell */
bool Kingdom::isVisited(const Maps::Tiles & tile) const
{
    return isVisited(tile.GetIndex(), tile.GetObject());
}

bool Kingdom::isVisited(s32 index, u8 object) const
{
    std::list<IndexObject>::const_iterator it = std::find_if(visit_object.begin(), visit_object.end(), std::bind2nd(std::mem_fun_ref(&IndexObject::isIndex), index));
    return visit_object.end() != it && (*it).isObject(object);
}

/* return true if object visited */
bool Kingdom::isVisited(const u8 object) const
{
    return visit_object.end() != std::find_if(visit_object.begin(), visit_object.end(), std::bind2nd(std::mem_fun_ref(&IndexObject::isObject), object));
}

u16 Kingdom::CountVisitedObjects(const MP2::object_t object) const
{
    return std::count_if(visit_object.begin(), visit_object.end(), std::bind2nd(std::mem_fun_ref(&IndexObject::isObject), object));
}

/* set visited cell */
void Kingdom::SetVisited(const s32 index, const MP2::object_t object)
{
    if(!isVisited(index, object) && object != MP2::OBJ_ZERO) visit_object.push_front(IndexObject(index, object));
}

bool Kingdom::HeroesMayStillMove(void) const
{
    return heroes.end() != std::find_if(heroes.begin(), heroes.end(), std::mem_fun(&Heroes::MayStillMove));
}

void Kingdom::Dump(void) const
{
    world.DateDump();
    std::cout << "Kingdom::Dump: " << "color: " << Color::String(color) <<
    ", resource: " << 
    "ore(" << resource.ore << ")," <<
    "wood(" << resource.wood << ")," <<
    "mercury(" << resource.mercury << ")," <<
    "sulfur(" << resource.sulfur << ")," <<
    "crystal(" << resource.crystal << ")," <<
    "gems(" << resource.gems << ")," <<
    "gold(" << resource.gold << ")" << std::endl;
}

u8 Kingdom::GetCountCapital(void) const
{
    return std::count_if(castles.begin(), castles.end(), Castle::PredicateIsCapital);
}

void Kingdom::AddFundsResource(const Resource::funds_t & funds)
{
    resource = resource + funds;

    if(0 > resource.wood) resource.wood = 0;
    if(0 > resource.mercury) resource.mercury = 0;
    if(0 > resource.ore) resource.ore = 0;
    if(0 > resource.sulfur) resource.sulfur = 0;
    if(0 > resource.crystal) resource.crystal = 0;
    if(0 > resource.gems) resource.gems = 0;
    if(0 > resource.gold) resource.gold = 0;
}

void Kingdom::OddFundsResource(const Resource::funds_t & funds)
{
    resource = resource - funds;

    if(0 > resource.wood) resource.wood = 0;
    if(0 > resource.mercury) resource.mercury = 0;
    if(0 > resource.ore) resource.ore = 0;
    if(0 > resource.sulfur) resource.sulfur = 0;
    if(0 > resource.crystal) resource.crystal = 0;
    if(0 > resource.gems) resource.gems = 0;
    if(0 > resource.gold) resource.gold = 0;
}

u8 Kingdom::GetLostTownDays(void) const
{
    return lost_town_days;
}

Recruits & Kingdom::GetRecruits(void)
{
    // update hero1
    if(Heroes::UNKNOWN == recruits.GetID1() || (recruits.GetHero1() && !recruits.GetHero1()->isFreeman()))
	recruits.SetHero1(world.GetFreemanHeroes(GetRace()));

    // update hero2
    if(Heroes::UNKNOWN == recruits.GetID2() || (recruits.GetHero2() && !recruits.GetHero2()->isFreeman()))
	recruits.SetHero2(world.GetFreemanHeroes());

    if(recruits.GetID1() == recruits.GetID2()) world.UpdateRecruits(recruits);

    return recruits;
}

void Kingdom::UpdateRecruits(void)
{
    recruits.SetHero1(world.GetFreemanHeroes(GetRace()));
    recruits.SetHero2(world.GetFreemanHeroes());

    if(recruits.GetID1() == recruits.GetID2()) world.UpdateRecruits(recruits);
}

const Puzzle & Kingdom::PuzzleMaps(void) const
{
    return puzzle_maps;
}

Puzzle & Kingdom::PuzzleMaps(void)
{
    return puzzle_maps;
}

void Kingdom::SetVisitTravelersTent(u8 col)
{
    visited_tents_colors |= col;
}

bool Kingdom::IsVisitTravelersTent(u8 col) const
{
    return visited_tents_colors & col;
}

bool Kingdom::AllowRecruitHero(bool check_payment, u8 level) const
{
    return (heroes.size() < GetMaxHeroes()) && (!check_payment || AllowPayment(PaymentConditions::RecruitHero(level)));
}

void Kingdom::ApplyPlayWithStartingHero(void)
{
    if(isPlay() && castles.size())
    {
	// get first castle
	std::vector<Castle*>::const_iterator it = std::find_if(castles.begin(), castles.end(), std::mem_fun(&Castle::isCastle));
	if(it == castles.end()) it = castles.begin();

	// check manual set hero (castle position + point(0, 1))?
	const Point & cp = (*it)->GetCenter();
	if(world.GetTiles(cp.x, cp.y + 1).GetObject() == MP2::OBJ_HEROES)
	{
    	    Heroes *hero = world.GetHeroes((cp.y + 1) * world.w() + cp.x);
    	    // and move manual set hero to castle
    	    if(hero && hero->GetColor() == GetColor())
    	    {
    		hero->SetFreeman(0);
    		hero->Recruit(**it);
    	    }
	}
	else
	if(Settings::Get().GameStartWithHeroes())
	{
    	    Heroes *hero = world.GetFreemanHeroes((*it)->GetRace());
	    if(hero && AllowRecruitHero(false, 0)) hero->Recruit(**it);
	}
    }
}

void Kingdom::SetMaxHeroes(u8 max)
{
    max_heroes = max;
}

u8 Kingdom::GetMaxHeroes(void)
{
    return max_heroes;
}

void Kingdom::HeroesActionNewPosition(void)
{
    // Heroes::ActionNewPosition: can ramove elements from heroes vector.
    size_t size = heroes.size();
    std::vector<Heroes *>::iterator it = heroes.begin();

    while(it != heroes.end())
    {
	(**it).ActionNewPosition();

	if(size != heroes.size())
	    size = heroes.size();
	else
	    ++it;
    }
}

u32 Kingdom::GetIncome(void)
{
    Resource::funds_t resource;

    // captured object
    //resource += ProfitConditions::FromMine(Resource::WOOD) * world.CountCapturedObject(MP2::OBJ_SAWMILL, color);
    //resource += ProfitConditions::FromMine(Resource::ORE) * world.CountCapturedMines(Resource::ORE, color);
    //resource += ProfitConditions::FromMine(Resource::MERCURY) * world.CountCapturedObject(MP2::OBJ_ALCHEMYLAB, color);
    //resource += ProfitConditions::FromMine(Resource::SULFUR) * world.CountCapturedMines(Resource::SULFUR, color);
    //resource += ProfitConditions::FromMine(Resource::CRYSTAL) * world.CountCapturedMines(Resource::CRYSTAL, color);
    //resource += ProfitConditions::FromMine(Resource::GEMS) * world.CountCapturedMines(Resource::GEMS, color);
    resource += ProfitConditions::FromMine(Resource::GOLD) * world.CountCapturedMines(Resource::GOLD, color);

    std::vector<Castle*>::const_iterator itc = castles.begin();
    for(; itc != castles.end(); ++itc) if(*itc)
    {
        const Castle & castle = **itc;

        // castle or town profit
        resource += ProfitConditions::FromBuilding((castle.isCastle() ? BUILD_CASTLE : BUILD_TENT), 0);

        // statue
        if(castle.isBuild(BUILD_STATUE))
                resource += ProfitConditions::FromBuilding(BUILD_STATUE, 0);

        // dungeon for warlock
	if(castle.isBuild(BUILD_SPEC) && Race::WRLK == castle.GetRace())
                resource += ProfitConditions::FromBuilding(BUILD_SPEC, Race::WRLK);
    }

    std::vector<Heroes*>::const_iterator ith = heroes.begin();
    for(; ith != heroes.end(); ++ith) if(*ith)
    {
	if((**ith).HasArtifact(Artifact::GOLDEN_GOOSE))           resource += ProfitConditions::FromArtifact(Artifact::GOLDEN_GOOSE);
        if((**ith).HasArtifact(Artifact::ENDLESS_SACK_GOLD))      resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_SACK_GOLD);
	if((**ith).HasArtifact(Artifact::ENDLESS_BAG_GOLD))       resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_BAG_GOLD);
        if((**ith).HasArtifact(Artifact::ENDLESS_PURSE_GOLD))     resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_PURSE_GOLD);
	//if((**ith).HasArtifact(Artifact::ENDLESS_POUCH_SULFUR))   resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_POUCH_SULFUR);
	//if((**ith).HasArtifact(Artifact::ENDLESS_VIAL_MERCURY))   resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_VIAL_MERCURY);
	//if((**ith).HasArtifact(Artifact::ENDLESS_POUCH_GEMS))     resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_POUCH_GEMS);
	//if((**ith).HasArtifact(Artifact::ENDLESS_CORD_WOOD))      resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_CORD_WOOD);
	//if((**ith).HasArtifact(Artifact::ENDLESS_CART_ORE))       resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_CART_ORE);
	//if((**ith).HasArtifact(Artifact::ENDLESS_POUCH_CRYSTAL))  resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_POUCH_CRYSTAL);

	// estates skill bonus
	resource.gold += (**ith).GetSecondaryValues(Skill::Secondary::ESTATES);

	if((**ith).HasArtifact(Artifact::TAX_LIEN)) resource.gold -= 250;
    }

    return resource.gold;
}

const Heroes* Kingdom::GetBestHero(void) const
{
    return heroes.size() ? *std::max_element(heroes.begin(), heroes.end(), HeroesStrongestArmy) : NULL;
}

u32 Kingdom::GetArmiesStrength(void) const
{
    double res = 0;

    std::vector<Heroes*>::const_iterator ith = heroes.begin();
    for(; ith != heroes.end(); ++ith) if(*ith) res += (**ith).GetArmy().GetStrength();

    std::vector<Castle*>::const_iterator itc = castles.begin();
    for(; itc != castles.end(); ++itc) if(*itc) res += (**itc).GetArmy().GetStrength();

    return static_cast<u32>(res);
}

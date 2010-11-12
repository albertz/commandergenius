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

#include <bitset>
#include "agg.h"
#include "monster.h"
#include "settings.h"
#include "cursor.h"
#include "button.h"
#include "world.h"
#include "kingdom.h"
#include "payment.h"
#include "profit.h"
#include "statusbar.h"
#include "buildinginfo.h"

#ifdef WITH_XML
#include "xmlccwrap.h"
#endif

struct buildstats_t
{
    const char* id1;
    const u32   id2;
    const u8    race;
    cost_t      cost;
};

static buildstats_t _builds[] = {
    // id                                             gold wood mercury ore sulfur crystal gems
    { "thieves_guild", BUILD_THIEVESGUILD, Race::ALL, { 750, 5, 0, 0, 0, 0, 0 } },
    { "tavern",        BUILD_TAVERN,       Race::ALL, { 500, 5, 0, 0, 0, 0, 0 } },
    { "shipyard",      BUILD_SHIPYARD,     Race::ALL, {2000,20, 0, 0, 0, 0, 0 } },
    { "well",          BUILD_WELL,         Race::ALL, { 500, 0, 0, 0, 0, 0, 0 } },
    { "statue",        BUILD_STATUE,       Race::ALL, {1250, 0, 0, 5, 0, 0, 0 } },
    { "left_turret",   BUILD_LEFTTURRET,   Race::ALL, {1500, 0, 0, 5, 0, 0, 0 } },
    { "right_turret",  BUILD_RIGHTTURRET,  Race::ALL, {1500, 0, 0, 5, 0, 0, 0 } },
    { "marketplace",   BUILD_MARKETPLACE,  Race::ALL, { 500, 5, 0, 0, 0, 0, 0 } },
    { "moat",          BUILD_MOAT,         Race::ALL, { 750, 0, 0, 0, 0, 0, 0 } },
    { "castle",        BUILD_CASTLE,       Race::ALL, {5000,20, 0,20, 0, 0, 0 } },
    { "captain",       BUILD_CAPTAIN,      Race::ALL, { 500, 0, 0, 0, 0, 0, 0 } },
    { "mage_guild1",   BUILD_MAGEGUILD1,   Race::ALL, {2000, 5, 0, 5, 0, 0, 0 } },
    { "mage_guild2",   BUILD_MAGEGUILD2,   Race::ALL, {1000, 5, 4, 5, 4, 4, 4 } },
    { "mage_guild3",   BUILD_MAGEGUILD3,   Race::ALL, {1000, 5, 6, 5, 6, 6, 6 } },
    { "mage_guild4",   BUILD_MAGEGUILD4,   Race::ALL, {1000, 5, 8, 5, 8, 8, 8 } },
    { "mage_guild5",   BUILD_MAGEGUILD5,   Race::ALL, {1000, 5,10, 5,10,10,10 } },

    { "farm",        BUILD_WEL2, Race::KNGT, {1000, 0, 0, 0, 0, 0, 0 } },
    { "garbage_he",  BUILD_WEL2, Race::BARB, {1000, 0, 0, 0, 0, 0, 0 } },
    { "crystal_gar", BUILD_WEL2, Race::SORC, {1000, 0, 0, 0, 0, 0, 0 } },
    { "waterfall",   BUILD_WEL2, Race::WRLK, {1000, 0, 0, 0, 0, 0, 0 } },
    { "orchard",     BUILD_WEL2, Race::WZRD, {1000, 0, 0, 0, 0, 0, 0 } },
    { "skull_pile",  BUILD_WEL2, Race::NECR, {1000, 0, 0, 0, 0, 0, 0 } },

    { "fortification", BUILD_SPEC, Race::KNGT, {1500, 5, 0,15, 0, 0, 0 } },
    { "coliseum",      BUILD_SPEC, Race::BARB, {2000,10, 0,10, 0, 0, 0 } },
    { "rainbow",       BUILD_SPEC, Race::SORC, {1500, 0, 0, 0, 0,10, 0 } },
    { "dungeon",       BUILD_SPEC, Race::WRLK, {3000, 5, 0,10, 0, 0, 0 } },
    { "library",       BUILD_SPEC, Race::WZRD, {1500, 5, 5, 5, 5, 5, 5 } },
    { "storm",         BUILD_SPEC, Race::NECR, {1000, 0,10, 0,10, 0, 0 } },

    { "shrine", BUILD_SHRINE, Race::NECR, {4000,10, 0, 0, 0,10, 0 } },

    { "thatched_hut",       DWELLING_MONSTER1, Race::KNGT, { 200, 0, 0, 0, 0, 0, 0 } },
    { "archery_range",      DWELLING_MONSTER2, Race::KNGT, {1000, 0, 0, 0, 0, 0, 0 } },
    { "upg_archery_range",  DWELLING_UPGRADE2, Race::KNGT, {1500, 5, 0, 0, 0, 0, 0 } },
    { "blacksmith",         DWELLING_MONSTER3, Race::KNGT, {1000, 0, 0, 5, 0, 0, 0 } },
    { "upg_blacksmith",     DWELLING_UPGRADE3, Race::KNGT, {1500, 0, 0, 5, 0, 0, 0 } },
    { "armory",             DWELLING_MONSTER4, Race::KNGT, {2000,10, 0,10, 0, 0, 0 } },
    { "upg_armory",         DWELLING_UPGRADE4, Race::KNGT, {2000, 5, 0, 5, 0, 0, 0 } },
    { "jousting_arena",     DWELLING_MONSTER5, Race::KNGT, {3000,20, 0, 0, 0, 0, 0 } },
    { "upg_jousting_arena", DWELLING_UPGRADE5, Race::KNGT, {3000,10, 0, 0, 0, 0, 0 } },
    { "cathedral",          DWELLING_MONSTER6, Race::KNGT, {5000,20, 0, 0, 0,20, 0 } },
    { "upg_cathedral",      DWELLING_UPGRADE6, Race::KNGT, {5000,10, 0, 0, 0,10, 0 } },

    { "hut",            DWELLING_MONSTER1, Race::BARB, { 300, 0, 0, 0, 0, 0, 0 } },
    { "stick_hut",      DWELLING_MONSTER2, Race::BARB, { 800, 5, 0, 0, 0, 0, 0 } },
    { "upg_stick_hut",  DWELLING_UPGRADE2, Race::BARB, {1200, 5, 0, 0, 0, 0, 0 } },
    { "den",            DWELLING_MONSTER3, Race::BARB, {1000, 0, 0, 0, 0, 0, 0 } },
    { "adobe",          DWELLING_MONSTER4, Race::BARB, {2000,10, 0,10, 0, 0, 0 } },
    { "upg_adobe",      DWELLING_UPGRADE4, Race::BARB, {3000, 5, 0, 5, 0, 0, 0 } },
    { "bridge",         DWELLING_MONSTER5, Race::BARB, {4000, 0, 0,20, 0, 0, 0 } },
    { "upg_bridge",     DWELLING_UPGRADE5, Race::BARB, {2000, 0, 0,10, 0, 0, 0 } },
    { "cyclop_pyramid", DWELLING_MONSTER6, Race::BARB, {6000, 0, 0,20, 0,20, 0 } },

    { "treehouse",               DWELLING_MONSTER1, Race::SORC, { 500, 5, 0, 0, 0, 0, 0 } },
    { "cottage",                 DWELLING_MONSTER2, Race::SORC, {1000, 5, 0, 0, 0, 0, 0 } },
    { "upg_cottage",             DWELLING_UPGRADE2, Race::SORC, {1500, 5, 0, 0, 0, 0, 0 } },
    { "elves_arhery_range",      DWELLING_MONSTER3, Race::SORC, {1500, 0, 0, 0, 0, 0, 0 } },
    { "upg_elves_archery_range", DWELLING_UPGRADE3, Race::SORC, {1500, 5, 0, 0, 0, 0, 0 } },
    { "stonehenge",              DWELLING_MONSTER4, Race::SORC, {1500, 0, 0,10, 0, 0, 0 } },
    { "upd_stonehenge",          DWELLING_UPGRADE4, Race::SORC, {1500, 0, 5, 0, 0, 0, 0 } },
    { "fenced_meadow",           DWELLING_MONSTER5, Race::SORC, {3000,10, 0, 0, 0, 0,10 } },
    { "phoenix_red_tower",       DWELLING_MONSTER6, Race::SORC, {10000, 0,20,30, 0, 0, 0 } },

    { "cave",        DWELLING_MONSTER1, Race::WRLK, { 500, 0, 0, 0, 0, 0, 0 } },
    { "crypt",       DWELLING_MONSTER2, Race::WRLK, {1000, 0, 0,10, 0, 0, 0 } },
    { "nest",        DWELLING_MONSTER3, Race::WRLK, {2000, 0, 0, 0, 0, 0, 0 } },
    { "maze",        DWELLING_MONSTER4, Race::WRLK, {3000, 0, 0, 0, 0, 0,10 } },
    { "upg_maze",    DWELLING_UPGRADE4, Race::WRLK, {2000, 0, 0, 0, 0, 0, 5 } },
    { "swamp",       DWELLING_MONSTER5, Race::WRLK, {4000, 0, 0, 0,10, 0, 0 } },
    { "green_tower", DWELLING_MONSTER6, Race::WRLK, {15000,0, 0,30,20, 0, 0 } },
    { "red_tower",   DWELLING_UPGRADE6, Race::WRLK, {5000, 0, 0, 5,10, 0, 0 } },
    { "black_tower", DWELLING_UPGRADE7, Race::WRLK, {5000, 0, 0, 5,10, 0, 0 } },

    { "habitat",          DWELLING_MONSTER1, Race::WZRD, { 400, 0, 0, 0, 0, 0, 0 } },
    { "pen",              DWELLING_MONSTER2, Race::WZRD, { 800, 0, 0, 0, 0, 0, 0 } },
    { "foundry",          DWELLING_MONSTER3, Race::WZRD, {1500, 5, 0, 5, 0, 0, 0 } },
    { "upg_foundry",      DWELLING_UPGRADE3, Race::WZRD, {1500, 0, 5, 0, 0, 0, 0 } },
    { "cliff_nest",       DWELLING_MONSTER4, Race::WZRD, {3000, 5, 0, 0, 0, 0, 0 } },
    { "ivory_tower",      DWELLING_MONSTER5, Race::WZRD, {3500, 5, 5, 5, 5, 5, 5 } },
    { "upg_ivory_tower",  DWELLING_UPGRADE5, Race::WZRD, {4000, 5, 0, 5, 0, 0, 0 } },
    { "cloud_castle",     DWELLING_MONSTER6, Race::WZRD, {12500,5, 0, 5, 0, 0,20 } },
    { "upg_cloud_castle", DWELLING_UPGRADE6, Race::WZRD, {12500,5, 0, 5, 0, 0,20 } },

    { "excavation",    DWELLING_MONSTER1, Race::NECR, { 400, 0, 0, 0, 0, 0, 0 } },
    { "graveyard",     DWELLING_MONSTER2, Race::NECR, {1000, 0, 0, 0, 0, 0, 0 } },
    { "upg_graveyard", DWELLING_UPGRADE2, Race::NECR, {1000, 0, 0, 0, 0, 0, 0 } },
    { "pyramid",       DWELLING_MONSTER3, Race::NECR, {1500, 0, 0,10, 0, 0, 0 } },
    { "upg_pyramid",   DWELLING_UPGRADE3, Race::NECR, {1500, 0, 0, 5, 0, 0, 0 } },
    { "mansion",       DWELLING_MONSTER4, Race::NECR, {3000,10, 0, 0, 0, 0, 0 } },
    { "upg_mansion",   DWELLING_UPGRADE4, Race::NECR, {4000, 5, 0, 0, 0,10,10 } },
    { "mausoleum",     DWELLING_MONSTER5, Race::NECR, {4000,10, 0, 0,10, 0, 0 } },
    { "upg_mausoleum", DWELLING_UPGRADE5, Race::NECR, {3000, 0, 0, 5, 0, 5, 0 } },
    { "laboratory",    DWELLING_MONSTER6, Race::NECR, {10000,10,5,10, 5, 5, 5 } },

    // end
    { NULL, BUILD_NOTHING, Race::BOMG, { 0, 0, 0, 0, 0, 0, 0 } },
};

void BuildingInfo::UpdateCosts(const std::string & spec)
{
#ifdef WITH_XML
    // parse buildings.xml
    TiXmlDocument doc;
    const TiXmlElement* xml_buildings = NULL;
    buildstats_t* ptr = &_builds[0];

    if(doc.LoadFile(spec.c_str()) &&
        NULL != (xml_buildings = doc.FirstChildElement("buildings")))
    {
	while(ptr->id1)
	{
	    const TiXmlElement* xml_building = xml_buildings->FirstChildElement(ptr->id1);

	    if(xml_building)
	    {
    		cost_t & cost = ptr->cost;
		int value;

                xml_building->Attribute("gold", &value); cost.gold = value;
                xml_building->Attribute("wood", &value); cost.wood = value;
                xml_building->Attribute("mercury", &value); cost.mercury = value;
                xml_building->Attribute("ore", &value); cost.ore = value;
                xml_building->Attribute("sulfur", &value); cost.sulfur = value;
                xml_building->Attribute("crystal", &value); cost.crystal = value;
                xml_building->Attribute("gems", &value); cost.gems = value;
	    }

	    ++ptr;
	}
    }
    else
    VERBOSE(spec << ": " << doc.ErrorDesc());
#endif
}

void BuildingInfo::GetCost(u32 build, u8 race, payment_t & payment)
{
    const buildstats_t* ptr = &_builds[0];

    while(ptr->id1 && !(ptr->id2 == build && (!race || (race & ptr->race)))) ++ptr;

    payment.Reset();

    if(ptr)
    {
	payment.gold = ptr->cost.gold;
	payment.wood = ptr->cost.wood;
	payment.mercury = ptr->cost.mercury;
	payment.ore = ptr->cost.ore;
	payment.sulfur = ptr->cost.sulfur;
	payment.crystal = ptr->cost.crystal;
	payment.gems = ptr->cost.gems;
    }
}

u8 GetIndexBuildingSprite(u32 build)
{
    switch(build)
    {
	case DWELLING_MONSTER1:	return 19;
	case DWELLING_MONSTER2: return 20;
	case DWELLING_MONSTER3: return 21;
	case DWELLING_MONSTER4: return 22;
	case DWELLING_MONSTER5: return 23; 
	case DWELLING_MONSTER6: return 24;
	case DWELLING_UPGRADE2: return 25;
	case DWELLING_UPGRADE3: return 26;
	case DWELLING_UPGRADE4: return 27;
	case DWELLING_UPGRADE5: return 28;
	case DWELLING_UPGRADE6: return 29;
	case DWELLING_UPGRADE7: return 30;
	case BUILD_MAGEGUILD1:
	case BUILD_MAGEGUILD2:
	case BUILD_MAGEGUILD3:
	case BUILD_MAGEGUILD4:
	case BUILD_MAGEGUILD5:	return 0;
	case BUILD_THIEVESGUILD:return 1;
	case BUILD_TAVERN:	return 2;
	case BUILD_SHIPYARD:	return 3;
	case BUILD_WELL:	return 4;
	case BUILD_CASTLE:	return 6;
	case BUILD_STATUE:	return 7;
	case BUILD_LEFTTURRET:	return 8;
	case BUILD_RIGHTTURRET:	return 9;
	case BUILD_MARKETPLACE:	return 10;
	case BUILD_WEL2:	return 11;
	case BUILD_MOAT:	return 12;
	case BUILD_SPEC:	return 13;
	case BUILD_CAPTAIN:	return 15;
	default: break;
    }

    return 0;
}

BuildingInfo::BuildingInfo(const Castle & c, building_t b) : castle(c), building(b), area(0, 0, 135, 57), disable(false)
{
    if(IsDwelling()) building = castle.GetActualDwelling(b);

    building = castle.isBuild(b) ? castle.GetUpgradeBuilding(b) : b;
    // check upgrade 7
    building = castle.isBuild(building) ? castle.GetUpgradeBuilding(building) : building;

    if(IsDwelling())
    {
	description = _("The %{building} produces %{monster}.");
        String::Replace(description, "%{building}", Castle::GetStringBuilding(building, castle.GetRace()));
        std::string name = Monster(castle.GetRace(), building).GetMultiName();
        String::Lower(name);
        String::Replace(description, "%{monster}", name);
    }
    else
	description = Castle::GetDescriptionBuilding(building, castle.GetRace());

    switch(building)
    {
	case BUILD_WELL:
    	    String::Replace(description, "%{count}", Castle::GetGrownWell());
	    break;

	case BUILD_WEL2:
    	    String::Replace(description, "%{count}", Castle::GetGrownWel2());
	    break;

	case BUILD_CASTLE:
	case BUILD_STATUE:
	case BUILD_SPEC:
	{
	    const payment_t profit = ProfitConditions::FromBuilding(building, castle.GetRace());
	    String::Replace(description, "%{count}", profit.gold);
	    break;
	}

	default: break;
    }

    // necr and tavern check
    if(Race::NECR == castle.GetRace() && BUILD_TAVERN == building && !Settings::Get().PriceLoyaltyVersion())
	disable = true;

    if(b == BUILD_CAPTAIN)
    {
	ICN::icn_t icn = ICN::UNKNOWN;
	switch(castle.GetRace())
	{
    	    case Race::BARB: icn = ICN::CSTLCAPB; break;
    	    case Race::KNGT: icn = ICN::CSTLCAPK; break;
    	    case Race::NECR: icn = ICN::CSTLCAPN; break;
    	    case Race::SORC: icn = ICN::CSTLCAPS; break;
    	    case Race::WRLK: icn = ICN::CSTLCAPW; break;
    	    case Race::WZRD: icn = ICN::CSTLCAPZ; break;
    	    default: break;
	}
	const Sprite & sprite = AGG::GetICN(icn, (building & BUILD_CAPTAIN ? 1 : 0));
	area.w = sprite.w();
	area.h = sprite.h();
    }
}

u32 BuildingInfo::operator() (void) const
{
    return building;
}

void BuildingInfo::SetPos(s16 x, s16 y)
{
    area.x = x;
    area.y = y;
}

const Rect & BuildingInfo::GetArea(void) const
{
    return area;
}

bool BuildingInfo::AllowBuy(void) const
{
    return castle.AllowBuyBuilding(building);
}

bool BuildingInfo::IsDisable(void) const
{
    return disable;
}

bool BuildingInfo::IsDwelling(void) const
{
    switch(building)
    {
	case DWELLING_MONSTER1:
	case DWELLING_MONSTER2:
	case DWELLING_MONSTER3:
	case DWELLING_MONSTER4:
	case DWELLING_MONSTER5:
	case DWELLING_MONSTER6:
	case DWELLING_UPGRADE2:
	case DWELLING_UPGRADE3:
	case DWELLING_UPGRADE4:
	case DWELLING_UPGRADE5:
	case DWELLING_UPGRADE6:
	case DWELLING_UPGRADE7:
	    return true;
	default: break;
    }
    return false;
}

void BuildingInfo::RedrawCaptain(void)
{
    Display & display = Display::Get();

    switch(castle.GetRace())
    {
        case Race::BARB: display.Blit(AGG::GetICN(ICN::CSTLCAPB, (building & BUILD_CAPTAIN ? 1 : 0)), area.x, area.y); break;
        case Race::KNGT: display.Blit(AGG::GetICN(ICN::CSTLCAPK, (building & BUILD_CAPTAIN ? 1 : 0)), area.x, area.y); break;
        case Race::NECR: display.Blit(AGG::GetICN(ICN::CSTLCAPN, (building & BUILD_CAPTAIN ? 1 : 0)), area.x, area.y); break;
        case Race::SORC: display.Blit(AGG::GetICN(ICN::CSTLCAPS, (building & BUILD_CAPTAIN ? 1 : 0)), area.x, area.y); break;
        case Race::WRLK: display.Blit(AGG::GetICN(ICN::CSTLCAPW, (building & BUILD_CAPTAIN ? 1 : 0)), area.x, area.y); break;
        case Race::WZRD: display.Blit(AGG::GetICN(ICN::CSTLCAPZ, (building & BUILD_CAPTAIN ? 1 : 0)), area.x, area.y); break;
        default: break;
    }

    const Sprite & sprite_allow = AGG::GetICN(ICN::TOWNWIND, 11);
    const Sprite & sprite_deny  = AGG::GetICN(ICN::TOWNWIND, 12);
    const Sprite & sprite_money = AGG::GetICN(ICN::TOWNWIND, 13);
    Point dst_pt;

    bool allow_buy = AllowBuy();

    // indicator
    dst_pt.x = area.x + 65;
    dst_pt.y = area.y + 60;
    if(castle.isBuild(building)) display.Blit(sprite_allow, dst_pt);
    else
    if(! allow_buy)
    {
	payment_t payment;
	GetCost(building, castle.GetRace(), payment);
	(1 == payment.GetValidItems() && payment.gold && castle.AllowBuild()) ? display.Blit(sprite_money, dst_pt) : display.Blit(sprite_deny, dst_pt);
    }
}

void BuildingInfo::Redraw(void)
{
    if(BUILD_CAPTAIN == building)
    {
	RedrawCaptain();
	return;
    }

    u8 index = GetIndexBuildingSprite(building);
    Display & display = Display::Get();

    display.Blit(AGG::GetICN(ICN::BLDGXTRA, 0), area.x, area.y);

    if(disable)
    {
	display.FillRect(0, 0, 0, Rect(area.x + 1, area.y + 1, area.w, area.h));
	return;
    }

    switch(castle.GetRace())
    {
        case Race::BARB: display.Blit(AGG::GetICN(ICN::CSTLBARB, index), area.x + 1, area.y + 1); break;
        case Race::KNGT: display.Blit(AGG::GetICN(ICN::CSTLKNGT, index), area.x + 1, area.y + 1); break;
        case Race::NECR: display.Blit(AGG::GetICN(ICN::CSTLNECR, index), area.x + 1, area.y + 1); break;
        case Race::SORC: display.Blit(AGG::GetICN(ICN::CSTLSORC, index), area.x + 1, area.y + 1); break;
        case Race::WRLK: display.Blit(AGG::GetICN(ICN::CSTLWRLK, index), area.x + 1, area.y + 1); break;
        case Race::WZRD: display.Blit(AGG::GetICN(ICN::CSTLWZRD, index), area.x + 1, area.y + 1); break;
        default: break;
    }

    const Sprite & sprite_allow = AGG::GetICN(ICN::TOWNWIND, 11);
    const Sprite & sprite_deny  = AGG::GetICN(ICN::TOWNWIND, 12);
    const Sprite & sprite_money = AGG::GetICN(ICN::TOWNWIND, 13);
    Point dst_pt;

    bool allow_buy = AllowBuy();

    // indicator
    dst_pt.x = area.x + 115;
    dst_pt.y = area.y + 40;
    if(castle.isBuild(building)) display.Blit(sprite_allow, dst_pt);
    else
    if(! allow_buy)
    {
	payment_t payment;
	GetCost(building, castle.GetRace(), payment);
	(1 == payment.GetValidItems() && payment.gold && castle.AllowBuild()) ? display.Blit(sprite_money, dst_pt) : display.Blit(sprite_deny, dst_pt);
    }

    // status bar
    if(!castle.isBuild(building))
    {
	dst_pt.x = area.x;
	dst_pt.y = area.y + 58;
	display.Blit(AGG::GetICN(ICN::CASLXTRA, allow_buy ? 1 : 2), dst_pt);
    }

    // name
    Text text(Castle::GetStringBuilding(building, castle.GetRace()), Font::SMALL);
    dst_pt.x = area.x + 68 - text.w() / 2;
    dst_pt.y = area.y + 59;
    text.Blit(dst_pt);
}


const char* BuildingInfo::GetName(void) const
{
    return Castle::GetStringBuilding(building, castle.GetRace());
}

const std::string & BuildingInfo::GetDescription(void) const
{
    return description;
}

bool BuildingInfo::QueueEventProcessing(void)
{
    if(disable) return false;

    LocalEvent & le = LocalEvent::Get();
    if(le.MouseClickLeft(area))
    {
	if(!castle.isBuild(BUILD_CASTLE))
	    Dialog::Message("", _("For this action it is necessary first to build a castle."), Font::BIG, Dialog::OK);
	else
	if(castle.isBuild(building))
	    Dialog::Message(GetName(), GetDescription(), Font::BIG, Dialog::OK);
	else
	    return DialogBuyBuilding(true);
    }
    else
    if(le.MousePressRight(area))
    {
	if(castle.isBuild(building))
	    Dialog::Message(GetName(), GetDescription(), Font::BIG);
	else
	    DialogBuyBuilding(false);
    }
    return false;
}

bool BuildingInfo::DialogBuyBuilding(bool buttons) const
{
    Display & display = Display::Get();

    const ICN::icn_t system = (Settings::Get().EvilInterface() ? ICN::SYSTEME : ICN::SYSTEM);

    Cursor & cursor = Cursor::Get();
    cursor.Hide();

    TextBox box1(description, Font::BIG, BOXAREA_WIDTH);

    // prepare requires build string
    std::string str;
    std::bitset<32> requires(castle.GetBuildingRequires(building));
    if(requires.any())
    {
	u8 count = 0;
	for(u8 pos = 0; pos < requires.size(); ++pos)
	{
	    if(requires.test(pos))
	    {
		u32 value = 1;
		value <<= pos;
		    
		++count;

		if(! castle.isBuild(value))
		{
		    str += Castle::GetStringBuilding(static_cast<building_t>(value), castle.GetRace());
		    if(count < requires.count()) str += ", ";
		}
	    }
	}
    }

    bool requires_true = str.size();
    Text requires_text(_("Requires:"), Font::BIG);
    TextBox box2(str, Font::BIG, BOXAREA_WIDTH);

    Resource::BoxSprite rbs(PaymentConditions::BuyBuilding(castle.GetRace(), building), BOXAREA_WIDTH);

    const Sprite & window_icons = AGG::GetICN(ICN::BLDGXTRA, 0);
    const u8 space = Settings::Get().QVGA() ? 5 : 10;
    Dialog::Box box(space + window_icons.h() + space + box1.h() + space + (requires_true ? requires_text.h() + box2.h() + space : 0) + rbs.GetArea().h, buttons);
    const Rect & box_rt = box.GetArea();
    ICN::icn_t cstl_icn = ICN::UNKNOWN;

    switch(castle.GetRace())
    {
        case Race::KNGT: cstl_icn = ICN::CSTLKNGT; break;
        case Race::BARB: cstl_icn = ICN::CSTLBARB; break;
        case Race::SORC: cstl_icn = ICN::CSTLSORC; break;
        case Race::WRLK: cstl_icn = ICN::CSTLWRLK; break;
        case Race::WZRD: cstl_icn = ICN::CSTLWZRD; break;
        case Race::NECR: cstl_icn = ICN::CSTLNECR; break;
        default: return Dialog::CANCEL;
    }

    LocalEvent & le = LocalEvent::Get();

    Point dst_pt;

    dst_pt.x = box_rt.x;
    dst_pt.y = box_rt.y + box_rt.h - AGG::GetICN(system, 1).h();
    Button button1(dst_pt, system, 1, 2);

    dst_pt.x = box_rt.x + box_rt.w - AGG::GetICN(system, 3).w();
    dst_pt.y = box_rt.y + box_rt.h - AGG::GetICN(system, 3).h();
    Button button2(dst_pt, system, 3, 4);

    dst_pt.x = box_rt.x + (box_rt.w - window_icons.w()) / 2;
    dst_pt.y = box_rt.y + space;
    display.Blit(window_icons, dst_pt);

    const Sprite & building_icons = AGG::GetICN(cstl_icn, GetIndexBuildingSprite(building));
    dst_pt.x = box_rt.x + (box_rt.w - building_icons.w()) / 2;
    dst_pt.y += 1;
    display.Blit(building_icons, dst_pt);

    Text text(GetName(), Font::SMALL);
    dst_pt.x = box_rt.x + (box_rt.w - text.w()) / 2;
    dst_pt.y += 57;
    text.Blit(dst_pt);

    dst_pt.x = box_rt.x;
    dst_pt.y = box_rt.y + space + window_icons.h() + space;
    box1.Blit(dst_pt);

    dst_pt.y += box1.h() + space;
    if(requires_true)
    {
	dst_pt.x = box_rt.x + (box_rt.w - requires_text.w()) / 2;
	requires_text.Blit(dst_pt);

	dst_pt.x = box_rt.x;
	dst_pt.y += requires_text.h();
	box2.Blit(dst_pt);

	dst_pt.y += box2.h() + space;
    }

    rbs.SetPos(dst_pt.x, dst_pt.y);
    rbs.Redraw();

    if(buttons)
    {
	if(!AllowBuy()) button1.SetDisable(true);

	button1.Draw();
	button2.Draw();
    }

    cursor.Show();
    display.Flip();

    // message loop
    while(le.HandleEvents())
    {
        if(!buttons && !le.MousePressRight()) break;

        le.MousePressLeft(button1) ? button1.PressDraw() : button1.ReleaseDraw();
        le.MousePressLeft(button2) ? button2.PressDraw() : button2.ReleaseDraw();

        if(button1.isEnable() &&
	    (Game::HotKeyPress(Game::EVENT_DEFAULT_READY) ||
    	    le.MouseClickLeft(button1))) return true;

        if(Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT) ||
    	    le.MouseClickLeft(button2)) break;
    }

    return false;
}

void BuildingInfo::SetStatusMessage(StatusBar & bar) const
{
    std::string str;
    const char* name = GetName();

    if(castle.isBuild(building))
    {
        str = _("%{name} is already built");
        String::Replace(str, "%{name}", name);
    }
    else
    {
        const PaymentConditions::BuyBuilding paymentBuild(castle.GetRace(), building);

        if(!castle.AllowBuild())
        {
            str = _("Cannot build. Already built here this turn.");
        }
        else
        if(castle.AllowBuild() && ! world.GetMyKingdom().AllowPayment(paymentBuild))
        {
            str = _("Cannot afford %{name}");
            String::Replace(str, "%{name}", name);
        }
        else
        if(BUILD_SHIPYARD == building && !castle.HaveNearlySea())
        {
            str = _("Cannot build %{name} because castle is too far from water.");
            String::Replace(str, "%{name}", name);
        }
        else
        if(!castle.AllowBuyBuilding(building))
        {
            str = _("Cannot build %{name}");
            String::Replace(str, "%{name}", name);
        }
        else
        {
            str = _("Build %{name}");
            String::Replace(str, "%{name}", name);
        }
    }

    bar.ShowMessage(str);
}

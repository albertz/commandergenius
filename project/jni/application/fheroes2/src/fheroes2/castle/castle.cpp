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

#include "agg.h"
#include "difficulty.h"
#include "settings.h"
#include "payment.h"
#include "world.h"
#include "luck.h"
#include "morale.h"
#include "kingdom.h"
#include "maps_tiles.h"
#include "castle.h"
#include "localclient.h"

u8 Castle::grown_well(2);
u8 Castle::grown_wel2(8);

Castle::Castle() : captain(*this), mageguild(*this), army(&captain), castle_heroes(NULL)
{
}

Castle::Castle(s16 cx, s16 cy, const Race::race_t rc) : Position(Point(cx, cy)), race(rc), captain(*this),
    color(Color::GRAY), building(0), mageguild(*this),
    army(NULL), castle_heroes(NULL)
{
    std::fill(dwelling, dwelling + CASTLEMAXMONSTER, 0);
    SetModes(ALLOWBUILD);

    mageguild.Builds();
}

void Castle::LoadFromMP2(const void *ptr)
{
    const u8  *ptr8  = static_cast<const u8 *>(ptr);
    u16 byte16 = 0;

    switch(*ptr8)
    {
	case 0x00: color = Color::BLUE;   break;
	case 0x01: color = Color::GREEN;  break;
        case 0x02: color = Color::RED;    break;
        case 0x03: color = Color::YELLOW; break;
        case 0x04: color = Color::ORANGE; break;
        case 0x05: color = Color::PURPLE; break;
        default:   color = Color::GRAY;   break;
    }
    ++ptr8;
    
    // custom building
    if(*ptr8)
    {
	++ptr8;
	
	// building
	byte16 = ReadLE16(ptr8);
        if(0x0002 & byte16) building |= BUILD_THIEVESGUILD;
        if(0x0004 & byte16) building |= BUILD_TAVERN;
        if(0x0008 & byte16) building |= BUILD_SHIPYARD;
        if(0x0010 & byte16) building |= BUILD_WELL;
        if(0x0080 & byte16) building |= BUILD_STATUE;
        if(0x0100 & byte16) building |= BUILD_LEFTTURRET;
        if(0x0200 & byte16) building |= BUILD_RIGHTTURRET;
        if(0x0400 & byte16) building |= BUILD_MARKETPLACE;
        if(0x1000 & byte16) building |= BUILD_MOAT;
        if(0x0800 & byte16) building |= BUILD_WEL2;
        if(0x2000 & byte16) building |= BUILD_SPEC;
	++ptr8;
	++ptr8;

	// dwelling
	byte16 = ReadLE16(ptr8);
        if(0x0008 & byte16) building |= DWELLING_MONSTER1;
        if(0x0010 & byte16) building |= DWELLING_MONSTER2;
        if(0x0020 & byte16) building |= DWELLING_MONSTER3;
        if(0x0040 & byte16) building |= DWELLING_MONSTER4;
        if(0x0080 & byte16) building |= DWELLING_MONSTER5;
        if(0x0100 & byte16) building |= DWELLING_MONSTER6;
        if(0x0200 & byte16) building |= DWELLING_UPGRADE2;
        if(0x0400 & byte16) building |= DWELLING_UPGRADE3;
        if(0x0800 & byte16) building |= DWELLING_UPGRADE4;
        if(0x1000 & byte16) building |= DWELLING_UPGRADE5;
        if(0x2000 & byte16) building |= DWELLING_UPGRADE6;
	++ptr8;
	++ptr8;

	
	// magic tower
	if(0 < *ptr8) building |= BUILD_MAGEGUILD1;
	if(1 < *ptr8) building |= BUILD_MAGEGUILD2;
	if(2 < *ptr8) building |= BUILD_MAGEGUILD3;
	if(3 < *ptr8) building |= BUILD_MAGEGUILD4;
	if(4 < *ptr8) building |= BUILD_MAGEGUILD5;
	++ptr8;
    }
    else
    {
	ptr8 += 6;

        // default building
        building |= DWELLING_MONSTER1;
        u8 dwelling2 = 0;
        switch(Settings::Get().GameDifficulty())
	{
	    case Difficulty::EASY:	dwelling2 = 75; break;
	    case Difficulty::NORMAL:	dwelling2 = 50; break;
	    case Difficulty::HARD:	dwelling2 = 25; break;
	    case Difficulty::EXPERT:	dwelling2 = 10; break;
	    default: break;
	}
	if(dwelling2 && dwelling2 >= Rand::Get(1, 100)) building |= DWELLING_MONSTER2;
    }

    // custom troops
    bool custom_troops = *ptr8;
    if(custom_troops)
    {
	++ptr8;
	
	// monster1
	army.At(0).SetMonster(Monster::FromInt(*ptr8 + 1));
	++ptr8;

	// monster2
	army.At(1).SetMonster(Monster::FromInt(*ptr8 + 1));
	++ptr8;

	// monster3
	army.At(2).SetMonster(Monster::FromInt(*ptr8 + 1));
	++ptr8;

	// monster4
	army.At(3).SetMonster(Monster::FromInt(*ptr8 + 1));
	++ptr8;

	// monster5
	army.At(4).SetMonster(Monster::FromInt(*ptr8 + 1));
	++ptr8;

	// count1
	byte16 = ReadLE16(ptr8);
	army.At(0).SetCount(byte16);
	++ptr8;
	++ptr8;

	// count2
	byte16 = ReadLE16(ptr8);
	army.At(1).SetCount(byte16);
	++ptr8;
	++ptr8;

	// count3
	byte16 = ReadLE16(ptr8);
	army.At(2).SetCount(byte16);
	++ptr8;
	++ptr8;

	// count4
	byte16 = ReadLE16(ptr8);
	army.At(3).SetCount(byte16);
	++ptr8;
	++ptr8;

	// count5
	byte16 = ReadLE16(ptr8);
	army.At(4).SetCount(byte16);
	++ptr8;
	++ptr8;
    }
    else
    {
	ptr8 += 16;
    }
    
    // captain
    if(*ptr8) building |= BUILD_CAPTAIN;
    ++ptr8;
    
    // custom name
    ++ptr8;
    name = std::string(_(reinterpret_cast<const char *>(ptr8)));
    ptr8 += 13;

    // race
    const Race::race_t kingdom_race = Settings::Get().KingdomRace(color);
    switch(*ptr8)
    { 	 
	case 0x00: race = Race::KNGT; break; 	 
	case 0x01: race = Race::BARB; break; 	 
        case 0x02: race = Race::SORC; break; 	 
	case 0x03: race = Race::WRLK; break; 	 
	case 0x04: race = Race::WZRD; break; 	 
        case 0x05: race = Race::NECR; break; 	 
        default: race = (Color::GRAY != color && (Race::ALL & kingdom_race) ? kingdom_race : Race::Rand()); break;
    }
    ++ptr8;

    // castle
    if(*ptr8) building |= BUILD_CASTLE;
    ++ptr8;

    // allow upgrade to castle (0 - true, 1 - false)
    (*ptr8 ? ResetModes(ALLOWCASTLE) : SetModes(ALLOWCASTLE));
    ++ptr8;

    // unknown 29 byte
    //

    // dwelling pack
    if(building & DWELLING_MONSTER1) dwelling[0]  = Monster(race, DWELLING_MONSTER1).GetGrown();
    if(building & DWELLING_MONSTER2) dwelling[1]  = Monster(race, DWELLING_MONSTER2).GetGrown();
    if(building & DWELLING_UPGRADE2) dwelling[1]  = Monster(race, DWELLING_UPGRADE2).GetGrown();
    if(building & DWELLING_MONSTER3) dwelling[2]  = Monster(race, DWELLING_MONSTER3).GetGrown();
    if(building & DWELLING_UPGRADE3) dwelling[2]  = Monster(race, DWELLING_UPGRADE3).GetGrown();
    if(building & DWELLING_MONSTER4) dwelling[3]  = Monster(race, DWELLING_MONSTER4).GetGrown();
    if(building & DWELLING_UPGRADE4) dwelling[3]  = Monster(race, DWELLING_UPGRADE4).GetGrown();
    if(building & DWELLING_MONSTER5) dwelling[4]  = Monster(race, DWELLING_MONSTER5).GetGrown();
    if(building & DWELLING_UPGRADE5) dwelling[4]  = Monster(race, DWELLING_UPGRADE5).GetGrown();
    if(building & DWELLING_MONSTER6) dwelling[5]  = Monster(race, DWELLING_MONSTER6).GetGrown();
    if(building & DWELLING_UPGRADE6) dwelling[5]  = Monster(race, DWELLING_UPGRADE6).GetGrown();
    if(building & DWELLING_UPGRADE7) dwelling[5]  = Monster(race, DWELLING_UPGRADE7).GetGrown();

    // MageGuild
    mageguild.Builds();
    if(captain.isValid()) mageguild.EducateHero(captain);

    // fix upgrade dwelling dependend from race
    switch(race)
    {
	case Race::BARB: building &= ~(DWELLING_UPGRADE3 | DWELLING_UPGRADE6); break;
        case Race::SORC: building &= ~(DWELLING_UPGRADE5 | DWELLING_UPGRADE6); break;
	case Race::WRLK: building &= ~(DWELLING_UPGRADE2 | DWELLING_UPGRADE3 | DWELLING_UPGRADE5); break;
	case Race::WZRD: building &= ~(DWELLING_UPGRADE2 | DWELLING_UPGRADE4); break;
        case Race::NECR: building &= ~(DWELLING_UPGRADE6); break;
        default: break;
    }

    // fix captain
    if(building & BUILD_CAPTAIN)
    {
	captain.LoadDefaults();
	army.SetCommander(&captain);
    }
    else
	army.SetColor(color);

    // AI troops auto pack
    if(!custom_troops && Game::AI == GetControl())
	AIJoinRNDArmy();

    // fix shipyard
    if(!HaveNearlySea()) building &= ~(BUILD_SHIPYARD);

    // remove tavern from necromancer castle
    if(Race::NECR == race && !Settings::Get().PriceLoyaltyVersion())
    	building &= ~BUILD_TAVERN;

    // end
    DEBUG(DBG_GAME , DBG_INFO, "Castle::LoadFromMP2: " << (building & BUILD_CASTLE ? "castle" : "town") << ": " << name << ", color: " << Color::String(color) << ", race: " << Race::String(race));
}

u32 Castle::CountBuildings(void) const
{
    std::bitset<32> requires(building & (BUILD_THIEVESGUILD | BUILD_TAVERN | BUILD_SHIPYARD | BUILD_WELL |
                    BUILD_STATUE | BUILD_LEFTTURRET | BUILD_RIGHTTURRET |
                    BUILD_MARKETPLACE | BUILD_WEL2 | BUILD_MOAT | BUILD_SPEC |
                    BUILD_CAPTAIN | BUILD_CASTLE | BUILD_MAGEGUILD1 | DWELLING_MONSTER1 |
                    DWELLING_MONSTER2 | DWELLING_MONSTER3 | DWELLING_MONSTER4 |
                    DWELLING_MONSTER5 | DWELLING_MONSTER6));

    return requires.count();
}

bool Castle::ContainCoord(const u16 ax, const u16 ay) const
{
    const Point & mp = GetCenter();
    return ((mp.x == ax && mp.y - 3 == ay) || (ax >= mp.x - 2 && ax <= mp.x + 2 && ay >= mp.y - 2 && ay <= mp.y + 1));
}

void Castle::ActionNewDay(void)
{
    castle_heroes = GetHeroes();

    // for learns new spells need 1 day
    if(castle_heroes &&
	GetLevelMageGuild()) mageguild.EducateHero(*castle_heroes);

    SetModes(ALLOWBUILD);
}

void Castle::ActionNewWeek(void)
{
    u8 well = building & BUILD_WELL ? grown_well : 0;
    u8 wel2 = building & BUILD_WEL2 ? grown_wel2 : 0;

    // dw 1
    if(building & DWELLING_MONSTER1) dwelling[0]  += Monster(race, DWELLING_MONSTER1).GetGrown() + well + wel2;
    // dw 2
    if(building & DWELLING_UPGRADE2) dwelling[1]  += Monster(race, DWELLING_UPGRADE2).GetGrown() + well;
    else
    if(building & DWELLING_MONSTER2) dwelling[1]  += Monster(race, DWELLING_MONSTER2).GetGrown() + well;
    // dw 3
    if(building & DWELLING_UPGRADE3) dwelling[2]  += Monster(race, DWELLING_UPGRADE3).GetGrown() + well;
    else
    if(building & DWELLING_MONSTER3) dwelling[2]  += Monster(race, DWELLING_MONSTER3).GetGrown() + well;
    // dw 4
    if(building & DWELLING_UPGRADE4) dwelling[3]  += Monster(race, DWELLING_UPGRADE4).GetGrown() + well;
    else
    if(building & DWELLING_MONSTER4) dwelling[3]  += Monster(race, DWELLING_MONSTER4).GetGrown() + well;
    // dw 5
    if(building & DWELLING_UPGRADE5) dwelling[4]  += Monster(race, DWELLING_UPGRADE5).GetGrown() + well;
    else
    if(building & DWELLING_MONSTER5) dwelling[4]  += Monster(race, DWELLING_MONSTER5).GetGrown() + well;
    // dw 6
    if(building & DWELLING_UPGRADE7) dwelling[5]  += Monster(race, DWELLING_UPGRADE7).GetGrown() + well;
    else
    if(building & DWELLING_UPGRADE6) dwelling[5]  += Monster(race, DWELLING_UPGRADE6).GetGrown() + well;
    else
    if(building & DWELLING_MONSTER6) dwelling[5]  += Monster(race, DWELLING_MONSTER6).GetGrown() + well;
}

void Castle::ActionNewMonth(void)
{
    if(world.GetWeekType() == Week::PLAGUE)
    {
	for(u8 ii = 0; ii < CASTLEMAXMONSTER; ++ii) if(dwelling[ii]) dwelling[ii] /= 2;
    }
}

// change castle color
void Castle::ChangeColor(Color::color_t cl)
{
    color = cl;

    // fix army
    if(!isBuild(BUILD_CAPTAIN))
	army.SetColor(color);
}

// return mage guild level
u8 Castle::GetLevelMageGuild(void) const
{
    if(building & BUILD_MAGEGUILD5) return 5;
    else
    if(building & BUILD_MAGEGUILD4) return 4;
    else
    if(building & BUILD_MAGEGUILD3) return 3;
    else
    if(building & BUILD_MAGEGUILD2) return 2;
    else
    if(building & BUILD_MAGEGUILD1) return 1;

    return 0;
}

const MageGuild & Castle::GetMageGuild(void) const
{
    return mageguild;
}

const char* Castle::GetStringBuilding(u32 build, Race::race_t race)
{
    const char* str_build[] = { _("Thieves' Guild"), _("Tavern"), _("Shipyard"), _("Well"), _("Statue"), _("Left Turret"),
	_("Right Turret"), _("Marketplace"), _("Moat"), _("Castle"), _("Tent"), _("Captain's Quarters"), _("Mage Guild, Level 1"), 
	 _("Mage Guild, Level 2"), _("Mage Guild, Level 3"), _("Mage Guild, Level 4"), _("Mage Guild, Level 5"), "Unknown" };

    const char* str_wel2[] = { _("Farm"), _("Garbage Heap"), _("Crystal Garden"), _("Waterfall"), _("Orchard"), _("Skull Pile") };

    const char* str_spec[] = { _("Fortifications"), _("Coliseum"), _("Rainbow"), _("Dungeon"), _("Library"), _("Storm") };

    const char* str_dwelling[] = {
	_("Thatched Hut"), _("Hut"), _("Treehouse"), _("Cave"), _("Habitat"), _("Excavation"),
	_("Archery Range"), _("Stick Hut"), _("Cottage"), _("Crypt"), _("Pen"), _("Graveyard"),
	_("Blacksmith"), _("Den"), _("Archery Range"), _("Nest"), _("Foundry"), _("Pyramid"),
	_("Armory"), _("Adobe"), _("Stonehenge"), _("Maze"), _("Cliff Nest"), _("Mansion"),
	_("Jousting Arena"), _("Bridge"), _("Fenced Meadow"), _("Swamp"), _("Ivory Tower"), _("Mausoleum"),
	_("Cathedral"), _("Pyramid"), _("Red Tower"), _("Green Tower"), _("Cloud Castle"), _("Laboratory") };

    const char* str_upgrade[] = {
	_("Upg. Archery Range"), _("Upg. Stick Hut"), _("Upg. Cottage"), _("Crypt"), _("Pen"), _("Upg. Graveyard"),
	_("Upg. Blacksmith"), _("Den"), _("Upg. Archery Range"), _("Nest"), _("Upg. Foundry"), _("Upg. Pyramid"),
	_("Upg. Armory"), _("Upg. Adobe"), _("Upg. Stonehenge"), _("Upg. Maze"), _("Cliff Nest"), _("Upg. Mansion"),
	_("Upg. Jousting Arena"), _("Upg. Bridge"), _("Fenced Meadow"), _("Swamp"), _("Upg. Ivory Tower"), _("Upg. Mausoleum"),
	_("Upg. Cathedral"), _("Pyramid"), _("Red Tower"), _("Red Tower"), _("Upg. Cloud Castle"), _("Laboratory"),
	"", "", "", _("Black Tower"), "", "" };

    const char* shrine = _("Shrine");

    u8 offset = 0;

    switch(race)
    {
	case Race::KNGT: offset = 0; break;
	case Race::BARB: offset = 1; break;
	case Race::SORC: offset = 2; break;
	case Race::WRLK: offset = 3; break;
	case Race::WZRD: offset = 4; break;
	case Race::NECR: offset = 5; break;
	default: break;
    }

    if(Settings::Get().PriceLoyaltyVersion() && race == Race::NECR && build == BUILD_TAVERN) return shrine;

    switch(build)
    {
        case BUILD_THIEVESGUILD:return str_build[0];
        case BUILD_TAVERN:	return str_build[1];
        case BUILD_SHIPYARD:	return str_build[2];
        case BUILD_WELL:	return str_build[3];
        case BUILD_STATUE:	return str_build[4];
        case BUILD_LEFTTURRET:	return str_build[5];
        case BUILD_RIGHTTURRET:	return str_build[6];
        case BUILD_MARKETPLACE:	return str_build[7];
        case BUILD_MOAT:	return str_build[8];
        case BUILD_CASTLE:	return str_build[9];
        case BUILD_TENT:	return str_build[10];
        case BUILD_CAPTAIN:	return str_build[11];
        case BUILD_MAGEGUILD1:	return str_build[12];
        case BUILD_MAGEGUILD2:	return str_build[13];
        case BUILD_MAGEGUILD3:	return str_build[14];
        case BUILD_MAGEGUILD4:	return str_build[15];
        case BUILD_MAGEGUILD5:	return str_build[16];

        case BUILD_SPEC:	return str_spec[offset];
        case BUILD_WEL2:	return str_wel2[offset];

        case DWELLING_MONSTER1:	return str_dwelling[offset];
        case DWELLING_MONSTER2:	return str_dwelling[6 + offset];
        case DWELLING_MONSTER3:	return str_dwelling[12 + offset];
        case DWELLING_MONSTER4:	return str_dwelling[18 + offset];
        case DWELLING_MONSTER5:	return str_dwelling[24 + offset];
        case DWELLING_MONSTER6:	return str_dwelling[30 + offset];

        case DWELLING_UPGRADE2: return str_upgrade[offset];
        case DWELLING_UPGRADE3: return str_upgrade[6 + offset];
        case DWELLING_UPGRADE4: return str_upgrade[12 + offset];
        case DWELLING_UPGRADE5: return str_upgrade[18 + offset];
        case DWELLING_UPGRADE6: return str_upgrade[24 + offset];
        case DWELLING_UPGRADE7: return str_upgrade[30 + offset];

	default: break;
    }
    
    return str_build[17];
}

const char* Castle::GetDescriptionBuilding(u32 build, Race::race_t race)
{
    const char* desc_build[] = {
	_("The Thieves' Guild provides information on enemy players. Thieves' Guilds can also provide scouting information on enemy towns."),
	_("The Tavern increases morale for troops defending the castle."),
	_("The Shipyard allows ships to be built."),
	_("The Well increases the growth rate of all dwellings by %{count} creatures per week."),
	_("The Statue increases your town's income by %{count} per day."),
	_("The Left Turret provides extra firepower during castle combat."),
	_("The Right Turret provides extra firepower during castle combat."),
	_("The Marketplace can be used to convert one type of resource into another. The more marketplaces you control, the better the exchange rate."),
	_("The Moat slows attacking units. Any unit entering the moat must end its turn there and becomes more vulnerable to attack."),
	_("The Castle improves town defense and increases income to %{count} gold per day."),
	_("The Tent provides workers to build a castle, provided the materials and the gold are available."),
	_("The Captain's Quarters provides a captain to assist in the castle's defense when no hero is present."),
	_("The Mage Guild allows heroes to learn spells and replenish their spell points."), "Unknown" };

    const char* desc_wel2[] = {
	_("The Farm increases production of Peasants by %{count} per week."),
	_("The Garbage Heap increases production of Goblins by %{count} per week."),
	_("The Crystal Garden increases production of Sprites by %{count} per week."),
	_("The Waterfall increases production of Centaurs by %{count} per week."),
	_("The Orchard increases production of Halflings by %{count} per week."),
	_("The Skull Pile increases production of Skeletons by %{count} per week.") };

    const char* desc_spec[] = {
	_("The Fortifications increase the toughness of the walls, increasing the number of turns it takes to knock them down."),
	_("The Coliseum provides inspiring spectacles to defending troops, raising their morale by two during combat."),
	_("The Rainbow increases the luck of the defending units by two."),
	_("The Dungeon increases the income of the town by %{count} / day."),
	_("The Library increases the number of spells in the Guild by one for each level of the guild."),
	_("The Storm adds +2 to the power of spells of a defending spell caster.") };

    const char* shrine_descr = _("The Shrine increases the necromancy skill of all your necromancers by 10 percent.");

    u8 offset = 0;

    switch(race)
    {
	case Race::KNGT: offset = 0; break;
	case Race::BARB: offset = 1; break;
	case Race::SORC: offset = 2; break;
	case Race::WRLK: offset = 3; break;
	case Race::WZRD: offset = 4; break;
	case Race::NECR: offset = 5; break;
	default: break;
    }

    if(Settings::Get().PriceLoyaltyVersion() && race == Race::NECR && build == BUILD_TAVERN) return shrine_descr;
    
    switch(build)
    {
        case BUILD_THIEVESGUILD:return desc_build[0];
        case BUILD_TAVERN:	return desc_build[1];
        case BUILD_SHIPYARD:	return desc_build[2];
        case BUILD_WELL:	return desc_build[3];
        case BUILD_STATUE:	return desc_build[4];
        case BUILD_LEFTTURRET:	return desc_build[5];
        case BUILD_RIGHTTURRET:	return desc_build[6];
        case BUILD_MARKETPLACE:	return desc_build[7];
        case BUILD_MOAT:	return desc_build[8];
        case BUILD_CASTLE:	return desc_build[9];
        case BUILD_TENT:	return desc_build[10];
        case BUILD_CAPTAIN:	return desc_build[11];
        case BUILD_MAGEGUILD1:
        case BUILD_MAGEGUILD2:
        case BUILD_MAGEGUILD3:
        case BUILD_MAGEGUILD4:
        case BUILD_MAGEGUILD5:	return desc_build[12];

        case BUILD_SPEC:	return desc_spec[offset];
        case BUILD_WEL2:	return desc_wel2[offset];

	default: break;
    }

    return desc_build[13];
}

bool Castle::AllowBuyHero(const Heroes & hero)
{
    return !GetHeroes() && world.GetKingdom(color).AllowRecruitHero(true, hero.GetLevel());
}

bool Castle::RecruitHero(Heroes* hero)
{
    if(!hero || !AllowBuyHero(*hero) || !hero->Recruit(*this)) return false;

    world.GetKingdom(color).OddFundsResource(PaymentConditions::RecruitHero(hero->GetLevel()));
    castle_heroes = hero;

    // update spell book
    if(GetLevelMageGuild()) mageguild.EducateHero(*castle_heroes);

    DEBUG(DBG_GAME , DBG_INFO, "Castle::RecruitHero: " << name << ", recruit: " << castle_heroes->GetName());

#ifdef WITH_NET
    FH2LocalClient::SendCastleRecruitHero(*this, *castle_heroes);
#endif
    return true;
}

/* recruit monster from building to castle army */
bool Castle::RecruitMonster(u32 dw, u16 count)
{
    u8 dw_index = 0;

    switch(dw)
    {
	case DWELLING_MONSTER1: dw_index = 0; break;
	case DWELLING_MONSTER2: dw_index = 1; break;
	case DWELLING_MONSTER3: dw_index = 2; break;
	case DWELLING_MONSTER4: dw_index = 3; break;
	case DWELLING_MONSTER5: dw_index = 4; break;
	case DWELLING_MONSTER6: dw_index = 5; break;	
	default: return false;
    }

    Monster ms = Monster(race, GetActualDwelling(dw));

    // fix count
    if(dwelling[dw_index] < count) count = dwelling[dw_index];

    // buy
    const Resource::funds_t paymentCosts(PaymentConditions::BuyMonster(ms()) * count);
    Kingdom & kingdom = world.GetKingdom(color);

    if(! kingdom.AllowPayment(paymentCosts) || !army.JoinTroop(ms, count)) return false;

    kingdom.OddFundsResource(paymentCosts);
    dwelling[dw_index] -= count;

    DEBUG(DBG_GAME , DBG_INFO, "Castle::RecruitMonster: " << name);

#ifdef WITH_NET
    FH2LocalClient::SendCastleRecruitMonster(*this, dw, count);
#endif

    return true;
}

/* return current count monster in dwelling */
u16 Castle::GetDwellingLivedCount(u32 dw) const
{
   switch(dw)
   {
      case DWELLING_MONSTER1: return dwelling[0];
      case DWELLING_MONSTER2:
      case DWELLING_UPGRADE2: return dwelling[1];
      case DWELLING_MONSTER3:
      case DWELLING_UPGRADE3: return dwelling[2];
      case DWELLING_MONSTER4:
      case DWELLING_UPGRADE4: return dwelling[3];
      case DWELLING_MONSTER5:
      case DWELLING_UPGRADE5: return dwelling[4];
      case DWELLING_MONSTER6:
      case DWELLING_UPGRADE6:
      case DWELLING_UPGRADE7: return dwelling[5];
      
      default: break;
    }

    return 0;                                                                 
}

/* return requires for building */
u32 Castle::GetBuildingRequires(u32 build) const
{
    u32 requires = 0;

    switch(build)
    {
	case BUILD_SPEC:
	    switch(race)
	    {
		case Race::WZRD:
		    requires |= BUILD_MAGEGUILD1;
		    break;

		default:
	        break;
	    }
	    break;

	case DWELLING_MONSTER2:
	    switch(race)
	    {
		case Race::KNGT:
		case Race::BARB:
		case Race::WZRD:
		case Race::WRLK:
		case Race::NECR:
		    requires |= DWELLING_MONSTER1;
		    break;

		case Race::SORC:
		    requires |= DWELLING_MONSTER1;
		    requires |= BUILD_TAVERN;
		    break;

		default:
		    break;
	    }
	    break;

	case DWELLING_MONSTER3:
	    switch(race)
	    {
		case Race::KNGT:
		    requires |= DWELLING_MONSTER1;
		    requires |= BUILD_WELL;
		    break;

		case Race::BARB:
		case Race::SORC:
		case Race::WZRD:
		case Race::WRLK:
		case Race::NECR:
		    requires |= DWELLING_MONSTER1;
		    break;

		default:
		    break;
	    }
	    break;

	case DWELLING_MONSTER4:
	    switch(race)
	    {
		case Race::KNGT:
		    requires |= DWELLING_MONSTER1;
		    requires |= BUILD_TAVERN;
		    break;

		case Race::BARB:
		    requires |= DWELLING_MONSTER1;
			break;

		case Race::SORC:
		    requires |= DWELLING_MONSTER2;
		    requires |= BUILD_MAGEGUILD1;
		    break;

		case Race::WZRD:
		case Race::WRLK:
		    requires |= DWELLING_MONSTER2;
		    break;

		case Race::NECR:
		    requires |= DWELLING_MONSTER3;
		    requires |= BUILD_THIEVESGUILD;
		    break;

		default:
		    break;
	    }
	    break;

	case DWELLING_MONSTER5:
	    switch(race)
	    {
		case Race::KNGT:
		case Race::BARB:
		    requires |= DWELLING_MONSTER2;
		    requires |= DWELLING_MONSTER3;
		    requires |= DWELLING_MONSTER4;
		    break;

		case Race::SORC:
		    requires |= DWELLING_MONSTER4;
		    break;

		case Race::WRLK:
		    requires |= DWELLING_MONSTER3;
		    break;

		case Race::WZRD:
		    requires |= DWELLING_MONSTER3;
		    requires |= BUILD_MAGEGUILD1;
		    break;

		case Race::NECR:
		    requires |= DWELLING_MONSTER2;
		    requires |= BUILD_MAGEGUILD1;
		    break;

		default:
		    break;
	    }
	    break;

	case DWELLING_MONSTER6:
	    switch(race)
	    {
		case Race::KNGT:
		    requires |= DWELLING_MONSTER2;
		    requires |= DWELLING_MONSTER3;
		    requires |= DWELLING_MONSTER4;
		    break;

		case Race::BARB:
		case Race::SORC:
		case Race::NECR:
		    requires |= DWELLING_MONSTER5;
		    break;

		case Race::WRLK:
		case Race::WZRD:
		    requires |= DWELLING_MONSTER4;
		    requires |= DWELLING_MONSTER5;
		    break;

		default:
		    break;
	    }
	    break;

	case DWELLING_UPGRADE2:
	    switch(race)
	    {
		case Race::KNGT:
		case Race::BARB:
		    requires |= DWELLING_MONSTER2;
		    requires |= DWELLING_MONSTER3;
		    requires |= DWELLING_MONSTER4;
		    break;

		case Race::SORC:
		    requires |= DWELLING_MONSTER2;
		    requires |= BUILD_WELL;
		    break;

		case Race::NECR:
		    requires |= DWELLING_MONSTER2;
		    break;
		
		default:
		    break;
	    }
	    break;

	case DWELLING_UPGRADE3:
	    switch(race)
	    {
		case Race::KNGT:
		    requires |= DWELLING_MONSTER2;
		    requires |= DWELLING_MONSTER3;
		    requires |= DWELLING_MONSTER4;
		    break;

		case Race::SORC:
		    requires |= DWELLING_MONSTER3;
		    requires |= DWELLING_MONSTER4;
		    break;

		case Race::WZRD:
		    requires |= DWELLING_MONSTER3;
		    requires |= BUILD_WELL;
		    break;

		case Race::NECR:
		    requires |= DWELLING_MONSTER3;
		    break;

		default:
		    break;
	    }
	    break;

	case DWELLING_UPGRADE4:
	    switch(race)
	    {
		case Race::KNGT:
		case Race::BARB:
		    requires |= DWELLING_MONSTER2;
		    requires |= DWELLING_MONSTER3;
		    requires |= DWELLING_MONSTER4;
		    break;

		case Race::SORC:
		case Race::WRLK:
		case Race::NECR:
		    requires |= DWELLING_MONSTER4;
		    break;

		default:
		    break;
	    }
	    break;

	case DWELLING_UPGRADE5:
	    switch(race)
	    {
		case Race::KNGT:
		    requires |= DWELLING_MONSTER2;
		    requires |= DWELLING_MONSTER3;
		    requires |= DWELLING_MONSTER4;
		    requires |= DWELLING_MONSTER5;
		    break;
		
		case Race::BARB:
		    requires |= DWELLING_MONSTER5;
		    break;

		case Race::WZRD:
		    requires |= BUILD_SPEC;
		    requires |= DWELLING_MONSTER5;
		    break;

		case Race::NECR:
		    requires |= BUILD_MAGEGUILD2;
		    requires |= DWELLING_MONSTER5;
		    break;

		default:
		    break;
	    }
	    break;

	case DWELLING_UPGRADE6:
	    switch(race)
	    {
		case Race::KNGT:
		    requires |= DWELLING_MONSTER2;
		    requires |= DWELLING_MONSTER3;
		    requires |= DWELLING_MONSTER4;
		    requires |= DWELLING_MONSTER6;
		    break;

		case Race::WRLK:
		case Race::WZRD:
		    requires |= DWELLING_MONSTER6;
		    break;
		
		default:
		    break;
	    }
	    break;

	default: break;
    }
    
    return requires;
}

/* check allow buy building */
bool Castle::AllowBuyBuilding(u32 build) const
{
    // check allow building
    if(!Modes(ALLOWBUILD) || build & building) return false;

    switch(build)
    {
	// allow build castle
	case BUILD_CASTLE: if(! Modes(ALLOWCASTLE)) return false; break;

	// buid shipyard only nearly sea
	case BUILD_SHIPYARD: if(! HaveNearlySea()) return false; break;

	// check upgrade dwelling
        case DWELLING_UPGRADE2: if((Race::WRLK | Race::WZRD) & race) return false; break;
        case DWELLING_UPGRADE3: if((Race::BARB | Race::WRLK) & race) return false; break;
        case DWELLING_UPGRADE4: if((Race::WZRD) & race) return false; break;
        case DWELLING_UPGRADE5: if((Race::SORC | Race::WRLK) & race) return false; break;
        case DWELLING_UPGRADE6: if((Race::BARB | Race::SORC | Race::NECR) & race) return false; break;
        case DWELLING_UPGRADE7: if(Race::WRLK != race) return false; break;

	default: break;
    }

    // check valid payment
    if(! world.GetKingdom(color).AllowPayment(PaymentConditions::BuyBuilding(race, build))) return false;

    // check build requirements
    std::bitset<32> requires(Castle::GetBuildingRequires(build));
    
    if(requires.any())
    {
        for(u8 pos = 0; pos < requires.size(); ++pos)
        {
            if(requires.test(pos))
            {
                u32 value = 1;
                value <<= pos;

                if(! (building & value)) return false;
	    }
	}
    }

    return true;
}

/* buy building */
bool Castle::BuyBuilding(u32 build)
{
    if(! AllowBuyBuilding(build)) return false;

#ifdef WITH_NET
    FH2LocalClient::SendCastleBuyBuilding(*this, build);
#endif

    world.GetKingdom(color).OddFundsResource(PaymentConditions::BuyBuilding(race, build));

    // add build
    building |= build;

    switch(build)
    {
	    case BUILD_CASTLE:
		Maps::UpdateSpritesFromTownToCastle(GetCenter());
		Maps::ClearFog(GetIndex(), Game::GetViewDistance(Game::VIEW_CASTLE), GetColor());
		break;

	    case BUILD_MAGEGUILD1:
	    case BUILD_MAGEGUILD2:
	    case BUILD_MAGEGUILD3:
	    case BUILD_MAGEGUILD4:
	    case BUILD_MAGEGUILD5:
		if(captain.isValid()) mageguild.EducateHero(captain);
		if(castle_heroes) mageguild.EducateHero(*castle_heroes);
		break;

	    case BUILD_CAPTAIN:
		captain.LoadDefaults();
		army.SetCommander(&captain);
		break;

            case BUILD_SPEC:
        	// build library
		if(mageguild.HaveLibraryCapability())
		{
		    if(captain.isValid()) mageguild.EducateHero(captain);
		    if(castle_heroes) mageguild.EducateHero(*castle_heroes);
		}
		break;

	    case DWELLING_MONSTER1: dwelling[0] = Monster(race, DWELLING_MONSTER1).GetGrown(); break;
	    case DWELLING_MONSTER2: dwelling[1] = Monster(race, DWELLING_MONSTER2).GetGrown(); break;
	    case DWELLING_MONSTER3: dwelling[2] = Monster(race, DWELLING_MONSTER3).GetGrown(); break;
	    case DWELLING_MONSTER4: dwelling[3] = Monster(race, DWELLING_MONSTER4).GetGrown(); break;
	    case DWELLING_MONSTER5: dwelling[4] = Monster(race, DWELLING_MONSTER5).GetGrown(); break;
	    case DWELLING_MONSTER6: dwelling[5] = Monster(race, DWELLING_MONSTER6).GetGrown(); break;
	    default: break;
    }

    // disable day build
    ResetModes(ALLOWBUILD);

    DEBUG(DBG_GAME , DBG_INFO, "Castle::BuyBuilding: " << name << " build " << GetStringBuilding(build, race));
    return true;
}

/* draw image castle to position */
void Castle::DrawImageCastle(const Point & pt)
{
    const Maps::Tiles & tile = world.GetTiles(GetCenter());
    Display & display = Display::Get();

    u8 index = 0;
    Point dst_pt;

    // draw ground
    switch(tile.GetGround())
    {
	case Maps::Ground::GRASS:	index =  0; break;
	case Maps::Ground::SNOW:	index = 10; break;
	case Maps::Ground::SWAMP:	index = 20; break;
	case Maps::Ground::LAVA:	index = 30; break;
	case Maps::Ground::DESERT:	index = 40; break;
	case Maps::Ground::DIRT:	index = 50; break;
	case Maps::Ground::WASTELAND:	index = 60; break;
	case Maps::Ground::BEACH:	index = 70; break;

	default: return;
    }

    for(int ii = 0; ii < 5; ++ii)
    {
	const Sprite & sprite = AGG::GetICN(ICN::OBJNTWBA, index + ii);
        dst_pt.x = pt.x + ii * 32 + sprite.x();
	dst_pt.y = pt.y + 3 * 32 + sprite.y();
	display.Blit(sprite, dst_pt);
    }

    for(int ii = 0; ii < 5; ++ii)
    {
	const Sprite & sprite = AGG::GetICN(ICN::OBJNTWBA, index + 5 + ii);
        dst_pt.x = pt.x + ii * 32 + sprite.x();
	dst_pt.y = pt.y + 4 * 32 + sprite.y();
	display.Blit(sprite, dst_pt);
    }
    
    // draw castle
    switch(race)
    {
        case Race::KNGT: index = 0; break;
        case Race::BARB: index = 32; break;
	case Race::SORC: index = 64; break;
	case Race::WRLK: index = 96; break;
	case Race::WZRD: index = 128; break;
	case Race::NECR: index = 160; break;
	default: break;
    }
    if(! (BUILD_CASTLE & building)) index += 16;
    const Sprite & sprite2 = AGG::GetICN(ICN::OBJNTOWN, index);
    dst_pt.x = pt.x + 2 * 32 + sprite2.x();
    dst_pt.y = pt.y + sprite2.y();
    display.Blit(sprite2, dst_pt);
    for(int ii = 0; ii < 5; ++ii)
    {
	const Sprite & sprite = AGG::GetICN(ICN::OBJNTOWN, index + 1 + ii);
        dst_pt.x = pt.x + ii * 32 + sprite.x();
	dst_pt.y = pt.y + 32 + sprite.y();
	display.Blit(sprite, dst_pt);
    }
    for(int ii = 0; ii < 5; ++ii)
    {
	const Sprite & sprite = AGG::GetICN(ICN::OBJNTOWN, index + 6 + ii);
        dst_pt.x = pt.x + ii * 32 + sprite.x();
	dst_pt.y = pt.y + 2 * 32 + sprite.y();
	display.Blit(sprite, dst_pt);
    }
    for(int ii = 0; ii < 5; ++ii)
    {
	const Sprite & sprite = AGG::GetICN(ICN::OBJNTOWN, index + 11 + ii);
        dst_pt.x = pt.x + ii * 32 + sprite.x();
	dst_pt.y = pt.y + 3 * 32 + sprite.y();
	display.Blit(sprite, dst_pt);
    }
}

ICN::icn_t Castle::GetICNBoat(const Race::race_t & race)
{
    switch(race)
    {
	case Race::BARB:	return ICN::TWNBBOAT;
	case Race::KNGT:	return ICN::TWNKBOAT;
	case Race::NECR:	return ICN::TWNNBOAT;
	case Race::SORC:	return ICN::TWNSBOAT;
	case Race::WRLK:	return ICN::TWNWBOAT;
	case Race::WZRD:	return ICN::TWNZBOAT;
	default: break;
    }

    DEBUG(DBG_GAME , DBG_WARN, "Castle::GetICNBoat: return unknown");
    return ICN::UNKNOWN;
}

/* get building name ICN */
ICN::icn_t Castle::GetICNBuilding(u32 build, Race::race_t race)
{
    if(Race::BARB == race)
    {
	switch(build)
	{
	case BUILD_CASTLE:	return ICN::TWNBCSTL;
	case BUILD_TENT:	return ICN::TWNBTENT;
	case BUILD_SPEC:	return ICN::TWNBSPEC;
	case BUILD_CAPTAIN:	return ICN::TWNBCAPT;
	case BUILD_WEL2:	return ICN::TWNBWEL2;
	case BUILD_LEFTTURRET:	return ICN::TWNBLTUR;
	case BUILD_RIGHTTURRET:	return ICN::TWNBRTUR;
	case BUILD_MOAT:	return ICN::TWNBMOAT;
	case BUILD_MARKETPLACE:	return ICN::TWNBMARK;
	case BUILD_THIEVESGUILD:return ICN::TWNBTHIE;
	case BUILD_TAVERN:	return ICN::TWNBTVRN;
	case BUILD_WELL:	return ICN::TWNBWELL;
	case BUILD_STATUE:	return ICN::TWNBSTAT;
	case BUILD_SHIPYARD:	return ICN::TWNBDOCK;
	case BUILD_MAGEGUILD1:
	case BUILD_MAGEGUILD2:
	case BUILD_MAGEGUILD3:
	case BUILD_MAGEGUILD4:
	case BUILD_MAGEGUILD5:	return ICN::TWNBMAGE;
	case DWELLING_MONSTER1:	return ICN::TWNBDW_0;
	case DWELLING_MONSTER2:	return ICN::TWNBDW_1;
	case DWELLING_UPGRADE2: return ICN::TWNBUP_1;
	case DWELLING_MONSTER3:	return ICN::TWNBDW_2;
	case DWELLING_MONSTER4:	return ICN::TWNBDW_3;
	case DWELLING_UPGRADE4: return ICN::TWNBUP_3;
	case DWELLING_MONSTER5:	return ICN::TWNBDW_4;
	case DWELLING_UPGRADE5: return ICN::TWNBUP_4;
	case DWELLING_MONSTER6:	return ICN::TWNBDW_5;
	default: break;
	}
    }
    else
    if(Race::KNGT == race)
    {
	switch(build)
	{
	case BUILD_CASTLE:	return ICN::TWNKCSTL;
	case BUILD_TENT:	return ICN::TWNKTENT;
	case BUILD_SPEC:	return ICN::TWNKSPEC;
	case BUILD_CAPTAIN:	return ICN::TWNKCAPT;
	case BUILD_WEL2:	return ICN::TWNKWEL2;
	case BUILD_LEFTTURRET:	return ICN::TWNKLTUR;
	case BUILD_RIGHTTURRET:	return ICN::TWNKRTUR;
	case BUILD_MOAT:	return ICN::TWNKMOAT;
	case BUILD_MARKETPLACE:	return ICN::TWNKMARK;
	case BUILD_THIEVESGUILD:return ICN::TWNKTHIE;
	case BUILD_TAVERN:	return ICN::TWNKTVRN;
	case BUILD_WELL:	return ICN::TWNKWELL;
	case BUILD_STATUE:	return ICN::TWNKSTAT;
	case BUILD_SHIPYARD:	return ICN::TWNKDOCK;
	case BUILD_MAGEGUILD1:
	case BUILD_MAGEGUILD2:
	case BUILD_MAGEGUILD3:
	case BUILD_MAGEGUILD4:
	case BUILD_MAGEGUILD5:	return ICN::TWNKMAGE;
	case DWELLING_MONSTER1:	return ICN::TWNKDW_0;
	case DWELLING_MONSTER2:	return ICN::TWNKDW_1;
	case DWELLING_UPGRADE2: return ICN::TWNKUP_1;
	case DWELLING_MONSTER3:	return ICN::TWNKDW_2;
	case DWELLING_UPGRADE3: return ICN::TWNKUP_2;
	case DWELLING_MONSTER4:	return ICN::TWNKDW_3;
	case DWELLING_UPGRADE4: return ICN::TWNKUP_3;
	case DWELLING_MONSTER5:	return ICN::TWNKDW_4;
	case DWELLING_UPGRADE5: return ICN::TWNKUP_4;
	case DWELLING_MONSTER6:	return ICN::TWNKDW_5;
	case DWELLING_UPGRADE6: return ICN::TWNKUP_5;
	default: break;
	}
    }
    else
    if(Race::NECR == race)
    {
	switch(build)
	{
	case BUILD_CASTLE:	return ICN::TWNNCSTL;
	case BUILD_TENT:	return ICN::TWNNTENT;
	case BUILD_SPEC:	return ICN::TWNNSPEC;
	case BUILD_CAPTAIN:	return ICN::TWNNCAPT;
	case BUILD_WEL2:	return ICN::TWNNWEL2;
	case BUILD_LEFTTURRET:	return ICN::TWNNLTUR;
	case BUILD_RIGHTTURRET:	return ICN::TWNNRTUR;
	case BUILD_MOAT:	return ICN::TWNNMOAT;
	case BUILD_MARKETPLACE:	return ICN::TWNNMARK;
	case BUILD_THIEVESGUILD:return ICN::TWNNTHIE;
	// shrine
	case BUILD_TAVERN:	if(Settings::Get().PriceLoyaltyVersion()) return ICN::TWNNTVRN;
				break;
	case BUILD_WELL:	return ICN::TWNNWELL;
	case BUILD_STATUE:	return ICN::TWNNSTAT;
	case BUILD_SHIPYARD:	return ICN::TWNNDOCK;
	case BUILD_MAGEGUILD1:
	case BUILD_MAGEGUILD2:
	case BUILD_MAGEGUILD3:
	case BUILD_MAGEGUILD4:
	case BUILD_MAGEGUILD5:	return ICN::TWNNMAGE;
	case DWELLING_MONSTER1:	return ICN::TWNNDW_0;
	case DWELLING_MONSTER2:	return ICN::TWNNDW_1;
	case DWELLING_UPGRADE2: return ICN::TWNNUP_1;
	case DWELLING_MONSTER3:	return ICN::TWNNDW_2;
	case DWELLING_UPGRADE3: return ICN::TWNNUP_2;
	case DWELLING_MONSTER4:	return ICN::TWNNDW_3;
	case DWELLING_UPGRADE4: return ICN::TWNNUP_3;
	case DWELLING_MONSTER5:	return ICN::TWNNDW_4;
	case DWELLING_UPGRADE5: return ICN::TWNNUP_4;
	case DWELLING_MONSTER6:	return ICN::TWNNDW_5;
	default: break;
	}
    }
    else
    if(Race::SORC == race)
    {
	switch(build)
	{
	case BUILD_CASTLE:	return ICN::TWNSCSTL;
	case BUILD_TENT:	return ICN::TWNSTENT;
	case BUILD_SPEC:	return ICN::TWNSSPEC;
	case BUILD_CAPTAIN:	return ICN::TWNSCAPT;
	case BUILD_WEL2:	return ICN::TWNSWEL2;
	case BUILD_LEFTTURRET:	return ICN::TWNSLTUR;
	case BUILD_RIGHTTURRET:	return ICN::TWNSRTUR;
	case BUILD_MOAT:	return ICN::TWNSMOAT;
	case BUILD_MARKETPLACE:	return ICN::TWNSMARK;
	case BUILD_THIEVESGUILD:return ICN::TWNSTHIE;
	case BUILD_TAVERN:	return ICN::TWNSTVRN;
	case BUILD_WELL:	return ICN::TWNSWELL;
	case BUILD_STATUE:	return ICN::TWNSSTAT;
	case BUILD_SHIPYARD:	return ICN::TWNSDOCK;
	case BUILD_MAGEGUILD1:
	case BUILD_MAGEGUILD2:
	case BUILD_MAGEGUILD3:
	case BUILD_MAGEGUILD4:
	case BUILD_MAGEGUILD5:	return ICN::TWNSMAGE;
	case DWELLING_MONSTER1:	return ICN::TWNSDW_0;
	case DWELLING_MONSTER2:	return ICN::TWNSDW_1;
	case DWELLING_UPGRADE2: return ICN::TWNSUP_1;
	case DWELLING_MONSTER3:	return ICN::TWNSDW_2;
	case DWELLING_UPGRADE3: return ICN::TWNSUP_2;
	case DWELLING_MONSTER4:	return ICN::TWNSDW_3;
	case DWELLING_UPGRADE4: return ICN::TWNSUP_3;
	case DWELLING_MONSTER5:	return ICN::TWNSDW_4;
	case DWELLING_MONSTER6:	return ICN::TWNSDW_5;
	default: break;
	}
    }
    else
    if(Race::WRLK == race)
    {
	switch(build)
	{
	case BUILD_CASTLE:	return ICN::TWNWCSTL;
	case BUILD_TENT:	return ICN::TWNWTENT;
	case BUILD_SPEC:	return ICN::TWNWSPEC;
	case BUILD_CAPTAIN:	return ICN::TWNWCAPT;
	case BUILD_WEL2:	return ICN::TWNWWEL2;
	case BUILD_LEFTTURRET:	return ICN::TWNWLTUR;
	case BUILD_RIGHTTURRET:	return ICN::TWNWRTUR;
	case BUILD_MOAT:	return ICN::TWNWMOAT;
	case BUILD_MARKETPLACE:	return ICN::TWNWMARK;
	case BUILD_THIEVESGUILD:return ICN::TWNWTHIE;
	case BUILD_TAVERN:	return ICN::TWNWTVRN;
	case BUILD_WELL:	return ICN::TWNWWELL;
	case BUILD_STATUE:	return ICN::TWNWSTAT;
	case BUILD_SHIPYARD:	return ICN::TWNWDOCK;
	case BUILD_MAGEGUILD1:
	case BUILD_MAGEGUILD2:
	case BUILD_MAGEGUILD3:
	case BUILD_MAGEGUILD4:
	case BUILD_MAGEGUILD5:	return ICN::TWNWMAGE;
	case DWELLING_MONSTER1:	return ICN::TWNWDW_0;
	case DWELLING_MONSTER2:	return ICN::TWNWDW_1;
	case DWELLING_MONSTER3:	return ICN::TWNWDW_2;
	case DWELLING_MONSTER4:	return ICN::TWNWDW_3;
	case DWELLING_UPGRADE4: return ICN::TWNWUP_3;
	case DWELLING_MONSTER5:	return ICN::TWNWDW_4;
	case DWELLING_MONSTER6:	return ICN::TWNWDW_5;
	case DWELLING_UPGRADE6: return ICN::TWNWUP_5;
	case DWELLING_UPGRADE7: return ICN::TWNWUP5B;
	default: break;
	}
    }
    else
    if(Race::WZRD == race)
    {
	switch(build)
	{
	case BUILD_CASTLE:	return ICN::TWNZCSTL;
	case BUILD_TENT:	return ICN::TWNZTENT;
	case BUILD_SPEC:	return ICN::TWNZSPEC;
	case BUILD_CAPTAIN:	return ICN::TWNZCAPT;
	case BUILD_WEL2:	return ICN::TWNZWEL2;
	case BUILD_LEFTTURRET:	return ICN::TWNZLTUR;
	case BUILD_RIGHTTURRET:	return ICN::TWNZRTUR;
	case BUILD_MOAT:	return ICN::TWNZMOAT;
	case BUILD_MARKETPLACE:	return ICN::TWNZMARK;
	case BUILD_THIEVESGUILD:return ICN::TWNZTHIE;
	case BUILD_TAVERN:	return ICN::TWNZTVRN;
	case BUILD_WELL:	return ICN::TWNZWELL;
	case BUILD_STATUE:	return ICN::TWNZSTAT;
	case BUILD_SHIPYARD:	return ICN::TWNZDOCK;
	case BUILD_MAGEGUILD1:
	case BUILD_MAGEGUILD2:
	case BUILD_MAGEGUILD3:
	case BUILD_MAGEGUILD4:
	case BUILD_MAGEGUILD5:	return ICN::TWNZMAGE;
	case DWELLING_MONSTER1:	return ICN::TWNZDW_0;
	case DWELLING_MONSTER2:	return ICN::TWNZDW_1;
	case DWELLING_MONSTER3:	return ICN::TWNZDW_2;
	case DWELLING_UPGRADE3: return ICN::TWNZUP_2;
	case DWELLING_MONSTER4:	return ICN::TWNZDW_3;
	case DWELLING_MONSTER5:	return ICN::TWNZDW_4;
	case DWELLING_UPGRADE5: return ICN::TWNZUP_4;
	case DWELLING_MONSTER6:	return ICN::TWNZDW_5;
	case DWELLING_UPGRADE6: return ICN::TWNZUP_5;
	default: break;
	}
    }

    std::cout << "Castle::GetICNBuilding: return unknown, race: " << Race::String(race) << ", build: " << Castle::GetStringBuilding(build, race) << ", " << build << std::endl;

    return ICN::UNKNOWN;
}

const Heroes* Castle::GetHeroes(void) const
{
    const Heroes* hero = world.GetHeroes(GetIndex());
    return hero && Color::GRAY != hero->GetColor() ? hero : NULL;
}

Heroes* Castle::GetHeroes(void)
{
    Heroes* hero = world.GetHeroes(GetIndex());
    return hero && Color::GRAY != hero->GetColor() ? hero : NULL;
}

bool Castle::HaveNearlySea(void) const
{
    // check nearest ocean
    const s32 index = GetIndex() + world.w() * 2;
    const Maps::Tiles & left = world.GetTiles(index - 1);
    const Maps::Tiles & right = world.GetTiles(index + 1);
    const Maps::Tiles & center = world.GetTiles(index);

    return Maps::Ground::WATER == left.GetGround() || Maps::Ground::WATER == right.GetGround() || Maps::Ground::WATER == center.GetGround();
}

bool TilePresentBoat(const Maps::Tiles & tile)
{
    return (Maps::Ground::WATER == tile.GetGround() &&
	(tile.GetObject() == MP2::OBJ_BOAT || tile.GetObject() == MP2::OBJ_HEROES));
}

bool Castle::PresentBoat(void) const
{
    // 2 cell down
    const s32 index = GetIndex() + world.w() * 2;
    const u16 max = world.w() * world.h();

    if(index + 1 < max)
    {
	const Maps::Tiles & left = world.GetTiles(index - 1);
	const Maps::Tiles & right = world.GetTiles(index + 1);
	const Maps::Tiles & center = world.GetTiles(index);

	if(TilePresentBoat(left) || TilePresentBoat(right) || TilePresentBoat(center)) return true;
    }
    return false;
}

u32 Castle::GetActualDwelling(u32 build) const
{
    switch(build)
    {
	case DWELLING_MONSTER2: return building & DWELLING_UPGRADE2 ? DWELLING_UPGRADE2 : build;
	case DWELLING_MONSTER3: return building & DWELLING_UPGRADE3 ? DWELLING_UPGRADE3 : build;
	case DWELLING_MONSTER4: return building & DWELLING_UPGRADE4 ? DWELLING_UPGRADE4 : build;
	case DWELLING_MONSTER5: return building & DWELLING_UPGRADE5 ? DWELLING_UPGRADE5 : build;
	case DWELLING_UPGRADE6: return building & DWELLING_UPGRADE7 ? DWELLING_UPGRADE7 : build;
	case DWELLING_MONSTER6: return building & DWELLING_UPGRADE7 ? DWELLING_UPGRADE7 : (building & DWELLING_UPGRADE6 ? DWELLING_UPGRADE6 : build);
	default: break;
    }
    return build;
}

u32 Castle::GetUpgradeBuilding(u32 build) const
{
    switch(build)
    {
	case BUILD_TENT:	return BUILD_CASTLE;
	case BUILD_MAGEGUILD1:	return BUILD_MAGEGUILD2;
	case BUILD_MAGEGUILD2:	return BUILD_MAGEGUILD3;
	case BUILD_MAGEGUILD3:	return BUILD_MAGEGUILD4;
	case BUILD_MAGEGUILD4:	return BUILD_MAGEGUILD5;
	default: break;
    }

    if(Race::BARB == race)
    {
	switch(build)
	{
	    case DWELLING_MONSTER2:	return DWELLING_UPGRADE2;
	    case DWELLING_MONSTER4:	return DWELLING_UPGRADE4;
	    case DWELLING_MONSTER5:	return DWELLING_UPGRADE5;
	    default: break;
	}
    }
    else
    if(Race::KNGT == race)
    {
	switch(build)
	{
	    case DWELLING_MONSTER2:	return DWELLING_UPGRADE2;
	    case DWELLING_MONSTER3:	return DWELLING_UPGRADE3;
	    case DWELLING_MONSTER4:	return DWELLING_UPGRADE4;
	    case DWELLING_MONSTER5:	return DWELLING_UPGRADE5;
	    case DWELLING_MONSTER6:	return DWELLING_UPGRADE6;
	    default: break;
	}
    }
    else
    if(Race::NECR == race)
    {
	switch(build)
	{
	    case DWELLING_MONSTER2:	return DWELLING_UPGRADE2;
	    case DWELLING_MONSTER3:	return DWELLING_UPGRADE3;
	    case DWELLING_MONSTER4:	return DWELLING_UPGRADE4;
	    case DWELLING_MONSTER5:	return DWELLING_UPGRADE5;
	    default: break;
	}
    }
    else
    if(Race::SORC == race)
    {
	switch(build)
	{
	    case DWELLING_MONSTER2:	return DWELLING_UPGRADE2;
	    case DWELLING_MONSTER3:	return DWELLING_UPGRADE3;
	    case DWELLING_MONSTER4:	return DWELLING_UPGRADE4;
	    default: break;
	}
    }
    else
    if(Race::WRLK == race)
    {
	switch(build)
	{
	    case DWELLING_MONSTER4:	return DWELLING_UPGRADE4;
	    case DWELLING_MONSTER6:	return DWELLING_UPGRADE6;
	    case DWELLING_UPGRADE6: return DWELLING_UPGRADE7;
	    default: break;
	}
    }
    else
    if(Race::WZRD == race)
    {
	switch(build)
	{
	    case DWELLING_MONSTER3:	return DWELLING_UPGRADE3;
	    case DWELLING_MONSTER5:	return DWELLING_UPGRADE5;
	    case DWELLING_MONSTER6:	return DWELLING_UPGRADE6;
	    default: break;
	}
    }

    return build;
}

bool Castle::PredicateIsCapital(const Castle *castle)
{
    return castle && castle->Modes(CAPITAL);
}

bool Castle::PredicateIsCastle(const Castle* castle)
{
    return castle && castle->isCastle();
}

bool Castle::PredicateIsTown(const Castle* castle)
{
    return castle && !castle->isCastle();
}

bool Castle::PredicateIsBuildMarketplace(const Castle* castle)
{
    return castle && castle->isBuild(BUILD_MARKETPLACE);
}

void Castle::Dump(void) const
{
    std::cout << "name            : " << name << std::endl;
    std::cout << "race            : " << Race::String(race) << std::endl;
    std::cout << "color           : " << Color::String(color) << std::endl;
    std::cout << "build           : " << "0x" << std::hex << building << std::dec << std::endl;
    std::cout << "present heroes  : " << (GetHeroes() ? "yes" : "no") << std::endl;
    std::cout << "present boat    : " << (PresentBoat() ? "yes" : "no") << std::endl;
    std::cout << "nearly sea      : " << (HaveNearlySea() ? "yes" : "no") << std::endl;
    std::cout << "is castle       : " << (isCastle() ? "yes" : "no") << std::endl;
}

s8 Castle::GetAttackModificator(std::string *strs) const
{
    return 0;
}

s8 Castle::GetDefenseModificator(std::string *strs) const
{
    return 0;
}

s8 Castle::GetPowerModificator(std::string *strs) const
{
    s8 result = 0;

    if(Race::NECR == race && isBuild(BUILD_SPEC))
    {
	const u8 mod = 2;
        result += mod;
        if(strs)
        {
            strs->append(GetStringBuilding(BUILD_SPEC, race));
            StringAppendModifiers(*strs, mod);
        }
    }

    return result;
}

s8 Castle::GetKnowledgeModificator(std::string *strs) const
{
    return 0;
}

s8 Castle::GetMoraleModificator(std::string *strs) const
{
    s8 result(Morale::NORMAL);

    // and tavern
    if(Race::NECR != race && isBuild(BUILD_TAVERN))
    {
	const u8 mod = 1;
        result += mod;
        if(strs)
    	{
	    strs->append(GetStringBuilding(BUILD_TAVERN, race));
            StringAppendModifiers(*strs, mod);
	}
    }

    // and barbarian coliseum
    if(Race::BARB == race && isBuild(BUILD_SPEC))
    {
	const u8 mod = 2;
        result += mod;
        if(strs)
	{
	    strs->append(GetStringBuilding(BUILD_SPEC, race));
            StringAppendModifiers(*strs, mod);
	}
    }

    return result;
}

s8 Castle::GetLuckModificator(std::string *strs) const
{
    s8 result(Luck::NORMAL);

    if(Race::SORC == race && isBuild(BUILD_SPEC))
    {
	const u8 mod = 2;
        result += mod;
        if(strs)
	{
	    strs->append(Castle::GetStringBuilding(BUILD_SPEC, race));
            StringAppendModifiers(*strs, mod);
	}
    }

    return result;
}

void Castle::RecruitAllMonster(void)
{
    if(isBuild(DWELLING_MONSTER6)) RecruitMonster(DWELLING_MONSTER6, MAXU16);
    if(isBuild(DWELLING_MONSTER5)) RecruitMonster(DWELLING_MONSTER5, MAXU16);
    if(isBuild(DWELLING_MONSTER4)) RecruitMonster(DWELLING_MONSTER4, MAXU16);
    if(isBuild(DWELLING_MONSTER3)) RecruitMonster(DWELLING_MONSTER3, MAXU16);
    if(isBuild(DWELLING_MONSTER2)) RecruitMonster(DWELLING_MONSTER2, MAXU16);
    if(isBuild(DWELLING_MONSTER1)) RecruitMonster(DWELLING_MONSTER1, MAXU16);
}

const Army::army_t & Castle::GetArmy(void) const
{
    return army;
}

Army::army_t & Castle::GetArmy(void)
{
    return army;
}

void Castle::MergeArmies(void)
{
    Heroes *hero = GetHeroes();

    if(hero && army.isValid() && (GetControl() == Game::AI || Settings::Get().ExtBattleMergeArmies()))
	hero->GetArmy().JoinStrongestFromArmy(army);
}

const Army::army_t & Castle::GetActualArmy(void) const
{
    const Heroes *heroes = GetHeroes();
    return heroes ? heroes->GetArmy() : army;
}

Army::army_t & Castle::GetActualArmy(void)
{
    Heroes *heroes = GetHeroes();
    return heroes ? heroes->GetArmy() : army;
}

bool Castle::AllowBuyBoat(void) const
{
    // check payment and present other boat
    return (HaveNearlySea() && world.GetMyKingdom().AllowPayment(PaymentConditions::BuyBoat()) && !PresentBoat());
}

bool Castle::BuyBoat(void)
{
    if(!AllowBuyBoat()) return false;
    if(Game::LOCAL == world.GetKingdom(color).Control()) AGG::PlaySound(M82::BUILDTWN);

    const s32 index = GetIndex() + world.w() * 2;
    Maps::Tiles & left = world.GetTiles(index - 1);
    Maps::Tiles & right = world.GetTiles(index + 1);
    Maps::Tiles & center = world.GetTiles(index);

    if(MP2::OBJ_ZERO == left.GetObject() && Maps::Ground::WATER == left.GetGround())
    {
	world.GetMyKingdom().OddFundsResource(PaymentConditions::BuyBoat());

    	left.SetObject(MP2::OBJ_BOAT);
#ifdef WITH_NET
	FH2LocalClient::SendCastleBuyBoat(*this, left.GetIndex());
#endif
    }
    else
    if(MP2::OBJ_ZERO == right.GetObject() && Maps::Ground::WATER == right.GetGround())
    {
	world.GetMyKingdom().OddFundsResource(PaymentConditions::BuyBoat());

    	right.SetObject(MP2::OBJ_BOAT);
#ifdef WITH_NET
	FH2LocalClient::SendCastleBuyBoat(*this, right.GetIndex());
#endif
    }
    else
    if(MP2::OBJ_ZERO == center.GetObject() && Maps::Ground::WATER == center.GetGround())
    {
	world.GetMyKingdom().OddFundsResource(PaymentConditions::BuyBoat());

    	center.SetObject(MP2::OBJ_BOAT);
#ifdef WITH_NET
	FH2LocalClient::SendCastleBuyBoat(*this, center.GetIndex());
#endif
    }
    
    return true;
}

u8 Castle::GetControl(void) const
{
    return world.GetKingdom(color).Control();
}

bool Castle::isNecromancyShrineBuild(void) const
{
    return Settings::Get().PriceLoyaltyVersion() &&
	race == Race::NECR && (BUILD_TAVERN & building);
}

u8 Castle::GetGrownWell(void)
{
    return grown_well;
}

u8 Castle::GetGrownWel2(void)
{
    return grown_wel2;
}

void Castle::Scoute(void) const
{
    Maps::ClearFog(GetIndex(), Game::GetViewDistance(isCastle() ? Game::VIEW_CASTLE : Game::VIEW_TOWN), color);
}

#ifdef WITH_XML
#include "xmlccwrap.h"

void Castle::UpdateExtraGrowth(const TiXmlElement* xml)
{
    int value;
    xml->Attribute("well", &value);
    grown_well = value;

    xml->Attribute("wel", &value);
    grown_wel2 = value;
}
#endif

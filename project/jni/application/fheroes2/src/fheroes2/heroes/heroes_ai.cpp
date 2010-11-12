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

#include "settings.h"
#include "kingdom.h"
#include "castle.h"
#include "army.h"
#include "battle2.h"
#include "luck.h"
#include "morale.h"
#include "world.h"
#include "payment.h"
#include "gameevent.h"
#include "heroes.h"

void AIToMonster(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToPickupResource(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToTreasureChest(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToArtifact(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToResource(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToWagon(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToSkeleton(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToCaptureObject(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToFlotSam(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToObservationTower(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToMagellanMaps(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToTeleports(Heroes &hero, const s32 dst_index);
void AIToWhirlpools(Heroes &hero, const s32 dst_index);
void AIToPrimarySkillObject(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToExperienceObject(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToWitchsHut(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToShrine(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToGoodLuckObject(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToGoodMoraleObject(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToMagicWell(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToArtesianSpring(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToXanadu(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToEvent(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToUpgradeArmyObject(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToPoorMoraleObject(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToPoorLuckObject(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToObelisk(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToTreeKnowledge(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToDaemonCave(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToCastle(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToSign(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToDwellingJoinMonster(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToHeroes(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToDwellingRecruitMonster(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToStables(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToAbandoneMine(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToBarrier(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToTravellersTent(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToShipwreckSurvivor(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToBoat(Heroes &hero, const u8 obj, const s32 dst_index);
void AIToCoast(Heroes &hero, const u8 obj, const s32 dst_index);

Skill::Primary::skill_t AISelectPrimarySkill(Heroes &hero)
{
    switch(hero.GetRace())
    {
	case Race::KNGT:
	{
	    if(5 > hero.GetDefense())	return Skill::Primary::DEFENSE;
	    if(5 > hero.GetAttack())	return Skill::Primary::ATTACK;
	    if(3 > hero.GetKnowledge())	return Skill::Primary::KNOWLEDGE;
	    if(3 > hero.GetPower())	return Skill::Primary::POWER;
    	    break;
	}

	case Race::BARB:
	{
	    if(5 > hero.GetAttack())	return Skill::Primary::ATTACK;
	    if(5 > hero.GetDefense())	return Skill::Primary::DEFENSE;
	    if(3 > hero.GetPower())	return Skill::Primary::POWER;
	    if(3 > hero.GetKnowledge())	return Skill::Primary::KNOWLEDGE;
	    break;
	}

	case Race::SORC:
	case Race::WZRD:
	{
	    if(5 > hero.GetKnowledge())	return Skill::Primary::KNOWLEDGE;
	    if(5 > hero.GetPower())	return Skill::Primary::POWER;
	    if(3 > hero.GetDefense())	return Skill::Primary::DEFENSE;
	    if(3 > hero.GetAttack())	return Skill::Primary::ATTACK;
	    break;
	}

	case Race::WRLK:
	case Race::NECR:
	{
	    if(5 > hero.GetPower())	return Skill::Primary::POWER;
	    if(5 > hero.GetKnowledge())	return Skill::Primary::KNOWLEDGE;
	    if(3 > hero.GetAttack())	return Skill::Primary::ATTACK;
	    if(3 > hero.GetDefense())	return Skill::Primary::DEFENSE;
	    break;
	}
	
	default: break;
    }

    switch(Rand::Get(1,4))
    {
	case 1:	return Skill::Primary::ATTACK;
	case 2:	return Skill::Primary::DEFENSE;
	case 3:	return Skill::Primary::POWER;
	case 4:	return Skill::Primary::KNOWLEDGE;
	default: break;
    }

    return Skill::Primary::UNKNOWN;
}

void AIBattleLose(Heroes &hero, const Battle2::Result & res, bool attacker, Color::color_t color = Color::GRAY)
{
    u8 reason = attacker ? res.AttackerResult() : res.DefenderResult();

    if(Settings::Get().ExtHeroSurrenderingGiveExp() &&
	Battle2::RESULT_SURRENDER == reason)
    {
        const u32 & exp = attacker ? res.GetExperienceAttacker() : res.GetExperienceDefender();

        if(Settings::Get().MyColor() == hero.GetColor())
        {
            std::string msg = _("Hero %{name} also got a %{count} experience.");
            String::Replace(msg, "%{name}", hero.GetName());
            String::Replace(msg, "%{count}", exp);
            Dialog::Message("", msg, Font::BIG, Dialog::OK);
        }
        hero.IncreaseExperience(exp);
    }

    hero.SetKillerColor(color);
    hero.SetFreeman(reason);
}

void Heroes::AIAction(const s32 dst_index)
{
    const MP2::object_t object = (dst_index == GetIndex() ?
				    GetUnderObject() : world.GetTiles(dst_index).GetObject());

    if(MP2::isActionObject(object, isShipMaster())) SetModes(ACTION);

    switch(object)
    {
        case MP2::OBJ_BOAT:		AIToBoat(*this, object, dst_index); break;
        case MP2::OBJ_COAST:		AIToCoast(*this, object, dst_index); break;

    	case MP2::OBJ_MONSTER:		AIToMonster(*this, object, dst_index); break;
	case MP2::OBJ_HEROES:		AIToHeroes(*this, object, dst_index); break;
	case MP2::OBJ_CASTLE:		AIToCastle(*this, object, dst_index); break;

        // pickup object
        case MP2::OBJ_RESOURCE:
        case MP2::OBJ_BOTTLE:
        case MP2::OBJ_CAMPFIRE:         AIToPickupResource(*this, object, dst_index); break;

        case MP2::OBJ_WATERCHEST:
        case MP2::OBJ_TREASURECHEST:	AIToTreasureChest(*this, object, dst_index); break;
	case MP2::OBJ_ARTIFACT:		AIToArtifact(*this, object, dst_index); break;

        case MP2::OBJ_MAGICGARDEN:
        case MP2::OBJ_LEANTO:
        case MP2::OBJ_WINDMILL:
        case MP2::OBJ_WATERWHEEL:	AIToResource(*this, object, dst_index); break;

        case MP2::OBJ_WAGON:		AIToWagon(*this, object, dst_index); break;
        case MP2::OBJ_SKELETON:		AIToSkeleton(*this, object, dst_index); break;
        case MP2::OBJ_FLOTSAM:		AIToFlotSam(*this, object, dst_index); break;

	case MP2::OBJ_ALCHEMYLAB:
        case MP2::OBJ_MINES:
	case MP2::OBJ_SAWMILL:
        case MP2::OBJ_LIGHTHOUSE:	AIToCaptureObject(*this, object, dst_index); break;
        case MP2::OBJ_ABANDONEDMINE:    AIToAbandoneMine(*this, object, dst_index); break;

	case MP2::OBJ_SHIPWRECKSURVIROR:AIToShipwreckSurvivor(*this, object, dst_index); break;

	// event
	case MP2::OBJ_EVENT:		AIToEvent(*this, object, dst_index); break;

	case MP2::OBJ_SIGN:		AIToSign(*this, object, dst_index); break;

	// increase view
	case MP2::OBJ_OBSERVATIONTOWER:	AIToObservationTower(*this, object, dst_index); break;
        case MP2::OBJ_MAGELLANMAPS:	AIToMagellanMaps(*this, object, dst_index); break;

        // teleports
	case MP2::OBJ_STONELIGHTS:	AIToTeleports(*this, dst_index); break;
	case MP2::OBJ_WHIRLPOOL:	AIToWhirlpools(*this, dst_index); break;

        // primary skill modification
        case MP2::OBJ_FORT:
        case MP2::OBJ_MERCENARYCAMP:
        case MP2::OBJ_DOCTORHUT:
        case MP2::OBJ_STANDINGSTONES:	AIToPrimarySkillObject(*this, object, dst_index); break;

	// experience modification
        case MP2::OBJ_GAZEBO:           AIToExperienceObject(*this, object, dst_index); break;

        // witchs hut
        case MP2::OBJ_WITCHSHUT: 	AIToWitchsHut(*this, object, dst_index); break;

        // shrine circle
	case MP2::OBJ_SHRINE1:
	case MP2::OBJ_SHRINE2:
        case MP2::OBJ_SHRINE3:		AIToShrine(*this, object, dst_index); break;

        // luck modification
        case MP2::OBJ_FOUNTAIN:
        case MP2::OBJ_FAERIERING:
        case MP2::OBJ_IDOL:		AIToGoodLuckObject(*this, object, dst_index); break;

        // morale modification
        case MP2::OBJ_OASIS:
        case MP2::OBJ_TEMPLE:
        case MP2::OBJ_WATERINGHOLE:
        case MP2::OBJ_BUOY:		AIToGoodMoraleObject(*this, object, dst_index); break;

	case MP2::OBJ_OBELISK:		AIToObelisk(*this, object, dst_index); break;

        // magic point
	case MP2::OBJ_ARTESIANSPRING:	AIToArtesianSpring(*this, object, dst_index); break;
        case MP2::OBJ_MAGICWELL: 	AIToMagicWell(*this, object, dst_index); break;

        // increase skill
	case MP2::OBJ_XANADU:		AIToXanadu(*this, object, dst_index); break;

        case MP2::OBJ_HILLFORT:
        case MP2::OBJ_FREEMANFOUNDRY:	AIToUpgradeArmyObject(*this, object, dst_index); break;

	case MP2::OBJ_SHIPWRECK:
        case MP2::OBJ_GRAVEYARD:
	case MP2::OBJ_DERELICTSHIP:	AIToPoorMoraleObject(*this, object, dst_index); break;

	case MP2::OBJ_PYRAMID:		AIToPoorLuckObject(*this, object, dst_index); break;
	case MP2::OBJ_DAEMONCAVE:	AIToDaemonCave(*this, object, dst_index); break;

        case MP2::OBJ_TREEKNOWLEDGE:	AIToTreeKnowledge(*this, object, dst_index); break;

        // accept army
        case MP2::OBJ_WATCHTOWER:
        case MP2::OBJ_EXCAVATION:
        case MP2::OBJ_CAVE:
        case MP2::OBJ_TREEHOUSE:
        case MP2::OBJ_ARCHERHOUSE:
        case MP2::OBJ_GOBLINHUT:
        case MP2::OBJ_DWARFCOTT:
	case MP2::OBJ_HALFLINGHOLE:
        case MP2::OBJ_PEASANTHUT:
        case MP2::OBJ_THATCHEDHUT:      AIToDwellingJoinMonster(*this, object, dst_index); break;

        // recruit army
        case MP2::OBJ_RUINS:
        case MP2::OBJ_TREECITY:
        case MP2::OBJ_WAGONCAMP:
	case MP2::OBJ_DESERTTENT:
        // loyalty ver
        case MP2::OBJ_WATERALTAR:
        case MP2::OBJ_AIRALTAR:
        case MP2::OBJ_FIREALTAR:
        case MP2::OBJ_EARTHALTAR:
        case MP2::OBJ_BARROWMOUNDS:     AIToDwellingRecruitMonster(*this, object, dst_index); break;

        // recruit army (battle)
        case MP2::OBJ_DRAGONCITY:
        case MP2::OBJ_CITYDEAD:
        case MP2::OBJ_TROLLBRIDGE:	AIToDwellingRecruitMonster(*this, object, dst_index); break;

	// recruit genie
	case MP2::OBJ_ANCIENTLAMP:	AIToDwellingRecruitMonster(*this, object, dst_index); break;

	case MP2::OBJ_STABLES:		AIToStables(*this, object, dst_index); break;
	case MP2::OBJ_ARENA:		AIToPrimarySkillObject(*this, object, dst_index); break;

        case MP2::OBJ_BARRIER:          AIToBarrier(*this, object, dst_index); break;
        case MP2::OBJ_TRAVELLERTENT:    AIToTravellersTent(*this, object, dst_index); break;


    	default: break;
    }
}

void AIToHeroes(Heroes &hero, const u8 obj, const s32 dst_index)
{
    const Settings & conf = Settings::Get();
    Heroes *other_hero = world.GetHeroes(dst_index);
    if(! other_hero) return;

    if(hero.GetColor() == other_hero->GetColor() ||
        (conf.ExtUnionsAllowHeroesMeetings() && conf.IsUnions(hero.GetColor(), other_hero->GetColor())))
    {
        DEBUG(DBG_AI, DBG_INFO, "AIToHeroes: " << hero.GetName() << " meeting " << other_hero->GetName());
        hero.AIMeeting(*other_hero);
    }
    else
    if(! hero.AllowBattle())
    {
        DEBUG(DBG_AI, DBG_INFO, "AIToHeroes: " << hero.GetName() << " currently can not allow battle");
    }
    else
    if(! other_hero->AllowBattle())
    {
        DEBUG(DBG_AI, DBG_INFO, "AIToHeroes: " << other_hero->GetName() << " currently can not allow battle");
    }
    else
    {
        if(other_hero->GetUnderObject() == MP2::OBJ_CASTLE)
    	{
    	    AIToCastle(hero, MP2::OBJ_CASTLE, dst_index);
    	    return;
    	}

        DEBUG(DBG_AI , DBG_INFO, "AIToHeroes: " << hero.GetName() << " attack enemy hero " << other_hero->GetName());

            // new battle2
            Battle2::Result res = Battle2::Loader(hero.GetArmy(), other_hero->GetArmy(), dst_index);

            // loss defender
            if(!res.DefenderWins())
               AIBattleLose(*other_hero, res, false, hero.GetColor());

            // loss attacker
            if(!res.AttackerWins())
                AIBattleLose(hero, res, true, other_hero->GetColor());

            // wins attacker
            if(res.AttackerWins())
            {
                hero.IncreaseExperience(res.GetExperienceAttacker());
                hero.ActionAfterBattle();
            }
            else
            // wins defender
            if(res.DefenderWins())
            {
                other_hero->IncreaseExperience(res.GetExperienceDefender());
                other_hero->ActionAfterBattle();
            }
    }
}

void AIToCastle(Heroes &hero, const u8 obj, const s32 dst_index)
{
    const Settings & conf = Settings::Get();
    Castle *castle = world.GetCastle(dst_index);

    if(! castle) return;

    if(hero.GetColor() == castle->GetColor() ||
	(conf.ExtUnionsAllowCastleVisiting() && conf.IsUnions(hero.GetColor(), castle->GetColor())))
    {
        DEBUG(DBG_AI , DBG_INFO, "AIToCastle: " << hero.GetName() << " goto castle " << castle->GetName());
	castle->GetMageGuild().EducateHero(hero);
    }
    else
    {
        DEBUG(DBG_AI , DBG_INFO, "AIToCastle: " << hero.GetName() << " attack enemy castle " << castle->GetName());

    	castle->MergeArmies();
        Army::army_t & army = castle->GetActualArmy();

	if(army.isValid())
	{
            // new battle2
            Battle2::Result res = Battle2::Loader(hero.GetArmy(), army, dst_index);
            Heroes *other_hero = world.GetHeroes(dst_index);

            // loss defender
            if(!res.DefenderWins() && other_hero)
               AIBattleLose(*other_hero, res, false, hero.GetColor());

            // loss attacker
            if(!res.AttackerWins())
                AIBattleLose(hero, res, true, castle->GetColor());

            // wins attacker
            if(res.AttackerWins())
            {
		castle->GetArmy().Clear();

                world.GetKingdom(castle->GetColor()).RemoveCastle(castle);
                world.GetKingdom(hero.GetColor()).AddCastle(castle);
                world.CaptureObject(dst_index, hero.GetColor());
    		castle->Scoute();

                hero.IncreaseExperience(res.GetExperienceAttacker());
                hero.ActionAfterBattle();
            }
            else
            // wins defender
            if(res.DefenderWins() && other_hero)
            {
                other_hero->IncreaseExperience(res.GetExperienceDefender());
                other_hero->ActionAfterBattle();
            }
	}
	else
	{
    	    world.GetKingdom(castle->GetColor()).RemoveCastle(castle);
	    world.GetKingdom(hero.GetColor()).AddCastle(castle);
    	    world.CaptureObject(dst_index, hero.GetColor());
    	    castle->Scoute();
	}
    }
}

void AIToMonster(Heroes &hero, const u8 obj, const s32 dst_index)
{
    bool destroy = false;
    Maps::Tiles & tile = world.GetTiles(dst_index);
    const Army::Troop troop(tile);

    u32 join = 0;
    Resource::funds_t cost;

    u8 reason = Army::GetJoinSolution(hero, tile, join, cost.gold);

    // free join
    if(1 == reason)
    {
    	// join if ranged or flying monsters present
    	if(hero.GetArmy().HasMonster(troop()) || troop.isArchers() || troop.isFly())
    	{
    	    DEBUG(DBG_AI , DBG_INFO, "AIToMonster: " << hero.GetName() << " join monster " << troop.GetName());
    	    hero.GetArmy().JoinTroop(troop);
	    destroy = true;
    	}
	else
	    reason = 0;
    }
    else
    // join with cost
    if(2 == reason)
    {
    	// join if archers or fly or present
    	if(hero.GetArmy().HasMonster(troop()) || troop.isArchers() || troop.isFly())
    	{
    	    DEBUG(DBG_AI , DBG_INFO, "AIToMonster: " << hero.GetName() << " join monster " << troop.GetName() << ", count: " << join << ", cost: " << cost.gold);
    	    hero.GetArmy().JoinTroop(troop(), join);
	    world.GetKingdom(hero.GetColor()).OddFundsResource(cost);
	    destroy = true;
    	}
	else
	    reason = 0;
    }

    // fight
    if(0 == reason)
    {
	DEBUG(DBG_AI , DBG_INFO, "AIToMonster: " << hero.GetName() << " attack monster " << troop.GetName());

	Army::army_t army;
	army.JoinTroop(troop);
	army.ArrangeForBattle();

    	Battle2::Result res = Battle2::Loader(hero.GetArmy(), army, dst_index);

    	if(res.AttackerWins())
    	{
    	    hero.IncreaseExperience(res.GetExperienceAttacker());
    	    destroy = true;
    	    hero.ActionAfterBattle();
    	}
    	else
    	{
    	    AIBattleLose(hero, res, true);
    	    if(Settings::Get().ExtSaveMonsterBattle())
    	    {
            	tile.SetCountMonster(army.GetCountMonsters(troop()));
            	if(2 == tile.GetQuantity4()) tile.SetQuantity4(1);
    	    }
    	}
    }
    // unknown
    else
	destroy = true;


    if(destroy)
    {
        Maps::TilesAddon *addon = tile.FindMonster();
        if(addon)
        {
            const u32 uniq = addon->uniq;
            tile.Remove(uniq);
            tile.SetObject(MP2::OBJ_ZERO);
    	    tile.ResetQuantity();

            // remove shadow from left cell
            if(Maps::isValidDirection(dst_index, Direction::LEFT))
                world.GetTiles(Maps::GetDirectionIndex(dst_index, Direction::LEFT)).Remove(uniq);
        }
    }
}

void AIToPickupResource(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    Resource::funds_t resource;
    const u8 count = tile.GetQuantity2();

    switch(tile.GetQuantity1())
    {
        case Resource::WOOD: resource.wood += count; break;
        case Resource::MERCURY: resource.mercury += count; break;
        case Resource::ORE: resource.ore += count; break;
        case Resource::SULFUR: resource.sulfur += count; break;
        case Resource::CRYSTAL: resource.crystal += count; break;
        case Resource::GEMS: resource.gems += count; break;
        case Resource::GOLD: resource.gold += 100 * count; break;

        default: break;
    }

    switch(obj)
    {
        case MP2::OBJ_CAMPFIRE:
	    resource.gold += 100 * count;
    	    break;

        default: break;
    }

    world.GetKingdom(hero.GetColor()).AddFundsResource(resource);
    tile.RemoveObjectSprite();

    hero.GetPath().Reset();

    tile.SetObject(MP2::OBJ_ZERO);
    DEBUG(DBG_AI , DBG_INFO, "AIToPickupResource: " << hero.GetName() << " pickup small resource");
}

void AIToTreasureChest(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    Resource::funds_t resource;
    resource.gold = tile.GetQuantity2() * 100;

    if(Maps::Ground::WATER == tile.GetGround())
    {
	if(tile.GetQuantity1())
	{
	    const Artifact art(Artifact::FromInt(tile.GetQuantity1()));
	    if(!hero.PickupArtifact(art()))
		    resource.gold = 1500;	// it is from FAQ
	}
	world.GetKingdom(hero.GetColor()).AddFundsResource(resource);
    }
    else
    {
	if(tile.GetQuantity1())
	{
	    const Artifact art(Artifact::FromInt(tile.GetQuantity1()));
	
	    if(!hero.PickupArtifact(art()))
		    resource.gold = 1000;	// it is from FAQ
	    world.GetKingdom(hero.GetColor()).AddFundsResource(resource);
	}
	else
	if(resource.gold >= 1000)
	{
	    const u16 expr = resource.gold - 500;

	    // select gold or exp
	    if(Rand::Get(1))
		world.GetKingdom(hero.GetColor()).AddFundsResource(resource);
	    else
		hero.IncreaseExperience(expr);
	}
    }

    tile.RemoveObjectSprite();
    tile.SetObject(MP2::OBJ_ZERO);

    DEBUG(DBG_AI , DBG_INFO, "AIToTreasureChest: " << hero.GetName());
}

void AIToResource(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    const u8 count = tile.GetQuantity2();
    Resource::funds_t resource;

    switch(tile.GetQuantity1())
    {
	case Resource::WOOD: resource.wood += count; break;
    	case Resource::MERCURY: resource.mercury += count; break;
    	case Resource::ORE: resource.ore += count; break;
    	case Resource::SULFUR: resource.sulfur += count; break;
    	case Resource::CRYSTAL: resource.crystal += count; break;
    	case Resource::GEMS: resource.gems += count; break;
    	case Resource::GOLD: resource.gold += 100 * count; break;

	default: break;
    }

    if(resource.GetValidItems())
	world.GetKingdom(hero.GetColor()).AddFundsResource(resource);

    tile.SetQuantity1(0);
    tile.SetQuantity2(0);

    DEBUG(DBG_AI , DBG_INFO, "AIToResource: " << hero.GetName());
}

void AIToSkeleton(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);

    // artifact
    if(tile.GetQuantity1() && 0 == tile.GetQuantity2())
    {
	const Artifact art(Artifact::FromInt(tile.GetQuantity1()));

	if(hero.PickupArtifact(art()))
	{
	    tile.SetQuantity1(0);
	    tile.SetQuantity2(0);
	}
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToSkeleton: " << hero.GetName());
}

void AIToWagon(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);

    // artifact
    if(tile.GetQuantity1() && 0 == tile.GetQuantity2())
    {
	const Artifact art(Artifact::FromInt(tile.GetQuantity1()));
	if(hero.PickupArtifact(art()))
	    tile.SetQuantity1(0);
    }
    else
    if(tile.GetQuantity1() && tile.GetQuantity2())
    {
	const u8 count = tile.GetQuantity2();
	Resource::funds_t resource;

	switch(tile.GetQuantity1())
	{
	    case Resource::WOOD:	resource.wood += count; break;
    	    case Resource::MERCURY:	resource.mercury += count; break;
    	    case Resource::ORE:		resource.ore += count; break;
    	    case Resource::SULFUR:	resource.sulfur += count; break;
    	    case Resource::CRYSTAL:	resource.crystal += count; break;
    	    case Resource::GEMS:	resource.gems += count; break;
    	    case Resource::GOLD:	resource.gold += 100 * count; break;

	    default: break;
	}

	world.GetKingdom(hero.GetColor()).AddFundsResource(resource);

	tile.SetQuantity1(0);
	tile.SetQuantity2(0);
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToWagon: " << hero.GetName());
}

void AIToCaptureObject(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    Resource::resource_t res = Resource::UNKNOWN;

    switch(obj)
    {
	case MP2::OBJ_ALCHEMYLAB:	res = Resource::MERCURY; break;
	case MP2::OBJ_SAWMILL:		res = Resource::WOOD; break;
        case MP2::OBJ_MINES:		res = static_cast<Resource::resource_t>(tile.GetMinesType()); break;
        default: break;
    }

    // capture object
    if(hero.GetColor() != world.ColorCapturedObject(dst_index))
    {
	bool capture = true;

	if(tile.CheckEnemyGuardians(hero.GetColor()))
	{
	    const Army::Troop troop(tile);
	    Army::army_t army;
	    army.JoinTroop(troop);
	    army.SetColor(world.ColorCapturedObject(dst_index));

	    Battle2::Result result = Battle2::Loader(hero.GetArmy(), army, dst_index);

	    if(result.AttackerWins())
	    {
		hero.IncreaseExperience(result.GetExperienceAttacker());
	        hero.ActionAfterBattle();

		tile.ResetQuantity();
	    }
	    else
	    {
		capture = false;
	        AIBattleLose(hero, result, true);
		if(Settings::Get().ExtSaveMonsterBattle())
		    tile.SetCountMonster(army.GetCountMonsters(troop()));
	    }
	}

	if(capture) world.CaptureObject(dst_index, hero.GetColor());
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToCaptureObject: " << hero.GetName() << " captured: " << MP2::StringObject(obj));
}

void AIToFlotSam(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    Resource::funds_t resource;

    resource.gold += 100 * tile.GetQuantity1();
    resource.wood += tile.GetQuantity2();

    if(resource.GetValidItems()) world.GetKingdom(hero.GetColor()).AddFundsResource(resource);

    tile.RemoveObjectSprite();
    tile.SetObject(MP2::OBJ_ZERO);

    DEBUG(DBG_AI , DBG_INFO, "AIToFlotSam: " << hero.GetName());
}

void AIToSign(Heroes &hero, const u8 obj, const s32 dst_index)
{
    hero.SetVisited(dst_index, Visit::LOCAL);
    DEBUG(DBG_AI , DBG_INFO, "AIToSign: " << hero.GetName());
}

void AIToObservationTower(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::ClearFog(dst_index, Game::GetViewDistance(Game::VIEW_OBSERVATION_TOWER), hero.GetColor());
    hero.SetVisited(dst_index, Visit::GLOBAL);
    DEBUG(DBG_AI , DBG_INFO, "AIToObservationTower: " << hero.GetName());
}

void AIToMagellanMaps(Heroes &hero, const u8 obj, const s32 dst_index)
{
    if(1000 <= world.GetKingdom(hero.GetColor()).GetFundsGold())
    {
	hero.SetVisited(dst_index, Visit::GLOBAL);
	world.ActionForMagellanMaps(hero.GetColor());
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToMagellanMaps: " << hero.GetName());
}

void AIToTeleports(Heroes &hero, const s32 index_from)
{
    u16 index_to = world.NextTeleport(index_from);
    hero.ApplyPenaltyMovement();

    if(index_from == index_to)
    {
	DEBUG(DBG_AI , DBG_WARN, "AIToTeleports: action unsuccessfully...");
	return;
    }

    hero.SetIndex(index_to);
    hero.Scoute();

    world.GetTiles(index_from).SetObject(MP2::OBJ_STONELIGHTS);
    world.GetTiles(index_to).SetObject(MP2::OBJ_HEROES);

    hero.ActionNewPosition();

    DEBUG(DBG_AI , DBG_INFO, "AIToStoneLights: " << hero.GetName());
}

void AIToWhirlpools(Heroes &hero, const s32 index_from)
{
    const u16 index_to = world.NextWhirlpool(index_from);
    hero.ApplyPenaltyMovement();

    if(index_from == index_to)
    {
	DEBUG(DBG_AI , DBG_WARN, "AIToWhirlpools: action unsuccessfully...");
	return;
    }

    hero.SetIndex(index_to);
    hero.Scoute();

    world.GetTiles(index_from).SetObject(MP2::OBJ_WHIRLPOOL);
    world.GetTiles(index_to).SetObject(MP2::OBJ_HEROES);

    Army::Troop & troops = hero.GetArmy().GetWeakestTroop();

    if(Rand::Get(1) && 1 < troops.GetCount())
	troops.SetCount(Monster::GetCountFromHitPoints(troops(), troops.GetHitPoints() - troops.GetHitPoints() * Game::GetWhirlpoolPercent() / 100));

    DEBUG(DBG_AI , DBG_INFO, "AIToWhirlpools: " << hero.GetName());
}

void AIToPrimarySkillObject(Heroes &hero, const u8 obj, const s32 dst_index)
{
    const Maps::Tiles & tile = world.GetTiles(dst_index);

    Skill::Primary::skill_t skill = Skill::Primary::UNKNOWN;

    switch(obj)
    {
        case MP2::OBJ_FORT:		skill = Skill::Primary::DEFENSE; break;
        case MP2::OBJ_MERCENARYCAMP:	skill = Skill::Primary::ATTACK; break;
        case MP2::OBJ_DOCTORHUT:	skill = Skill::Primary::KNOWLEDGE; break;
        case MP2::OBJ_STANDINGSTONES:	skill = Skill::Primary::POWER; break;
        case MP2::OBJ_ARENA:		skill = AISelectPrimarySkill(hero); break;

    	default: break;
    }

    if(!hero.isVisited(tile))
    {
	// increase skill
	hero.IncreasePrimarySkill(skill);
	hero.SetVisited(dst_index);

        // fix double action tile
        if(obj == MP2::OBJ_STANDINGSTONES)
        {
            const Maps::TilesAddon* addon = tile.FindStandingStones();

            if(addon && Maps::isValidDirection(tile.GetIndex(), Direction::LEFT) &&
        	world.GetTiles(Maps::GetDirectionIndex(tile.GetIndex(), Direction::LEFT)).FindAddonLevel1(addon->uniq)) hero.SetVisited(Maps::GetDirectionIndex(tile.GetIndex(), Direction::LEFT));

            if(addon && Maps::isValidDirection(tile.GetIndex(), Direction::RIGHT) &&
                world.GetTiles(Maps::GetDirectionIndex(tile.GetIndex(), Direction::RIGHT)).FindAddonLevel1(addon->uniq)) hero.SetVisited(Maps::GetDirectionIndex(tile.GetIndex(), Direction::RIGHT));
        }
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToPrimarySkillObject: " << hero.GetName());
}

void AIToExperienceObject(Heroes &hero, const u8 obj, const s32 dst_index)
{
    const Maps::Tiles & tile = world.GetTiles(dst_index);

    u32 exp = 0;

    switch(obj)
    {
        case MP2::OBJ_GAZEBO: exp = 1000; break;
    	default: break;
    }

    // check already visited
    if(! hero.isVisited(tile) && exp)
    {
	hero.SetVisited(dst_index);
	hero.IncreaseExperience(exp);
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToExperienceObject: " << hero.GetName());
}

void AIToWitchsHut(Heroes &hero, const u8 obj, const s32 dst_index)
{
    const Skill::Secondary::skill_t skill = Skill::Secondary::Skill(world.GetTiles(dst_index).GetQuantity1());

    // check full
    if(!hero.HasMaxSecondarySkill() && !hero.HasSecondarySkill(skill))
	hero.LearnBasicSkill(skill);

    hero.SetVisited(dst_index);
    DEBUG(DBG_AI , DBG_INFO, "AIToWitchsHut: " << hero.GetName());
}

void AIToShrine(Heroes &hero, const u8 obj, const s32 dst_index)
{
    const Spell::spell_t spell = Spell::FromInt(world.GetTiles(dst_index).GetQuantity1());
    const u8 spell_level = Spell::Level(spell);

    // check spell book
    if(hero.HasArtifact(Artifact::MAGIC_BOOK) &&
      !hero.HaveSpell(spell) &&
    // check valid level spell and wisdom skill
      !(3 == spell_level && Skill::Level::NONE == hero.GetLevelSkill(Skill::Secondary::WISDOM)))
    {
	hero.AppendSpellToBook(spell);
	hero.SetVisited(dst_index);
    }
    DEBUG(DBG_AI , DBG_INFO, "AIToShrine: " << hero.GetName());
}

void AIToGoodLuckObject(Heroes &hero, const u8 obj, const s32 dst_index)
{
    // check already visited
    if(!hero.isVisited(obj)) hero.SetVisited(dst_index);
    DEBUG(DBG_AI , DBG_INFO, "AIToGoodLuckObject: " << hero.GetName());
}

void AIToGoodMoraleObject(Heroes &hero, const u8 obj, const s32 dst_index)
{
    const Maps::Tiles & tile = world.GetTiles(dst_index);

    u16 move = 0;

    switch(obj)
    {
        case MP2::OBJ_OASIS:		move = 800; break;
        case MP2::OBJ_WATERINGHOLE:	move = 400; break;
    	default: break;
    }

    // check already visited
    if(!hero.isVisited(obj))
    {
	// modify morale
	hero.SetVisited(dst_index);
        hero.IncreaseMovePoints(move);

        // fix double action tile
        if(obj == MP2::OBJ_OASIS)
        {
    	    const Maps::TilesAddon* addon = tile.FindOasis();

            if(addon && Maps::isValidDirection(tile.GetIndex(), Direction::LEFT) &&
                world.GetTiles(Maps::GetDirectionIndex(tile.GetIndex(), Direction::LEFT)).FindAddonLevel1(addon->uniq)) hero.SetVisited(Maps::GetDirectionIndex(tile.GetIndex(), Direction::LEFT));
            if(addon && Maps::isValidDirection(tile.GetIndex(), Direction::RIGHT) &&
                world.GetTiles(Maps::GetDirectionIndex(tile.GetIndex(), Direction::RIGHT)).FindAddonLevel1(addon->uniq)) hero.SetVisited(Maps::GetDirectionIndex(tile.GetIndex(), Direction::RIGHT));
        }
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToGoodMoraleObject: " << hero.GetName());
}

void AIToMagicWell(Heroes &hero, const u8 obj, const s32 dst_index)
{
    const u16 max = hero.GetMaxSpellPoints();

    if(hero.GetSpellPoints() != max &&
	// check already visited
	!hero.isVisited(MP2::OBJ_MAGICWELL))
    {
	hero.SetVisited(dst_index);
	hero.SetSpellPoints(max);
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToMagicWell: " << hero.GetName());
}

void AIToArtesianSpring(Heroes &hero, const u8 obj, const s32 dst_index)
{
    const u16 max = hero.GetMaxSpellPoints();

    if(!hero.isVisited(MP2::OBJ_ARTESIANSPRING) &&
       hero.GetSpellPoints() < max * 2)
    {
	hero.SetVisited(dst_index);
	hero.SetSpellPoints(max * 2);

        // fix double action tile
        {
	    const Maps::Tiles & tile = world.GetTiles(dst_index);
	    const Maps::TilesAddon* addon = tile.FindArtesianSpring();

            if(addon && Maps::isValidDirection(tile.GetIndex(), Direction::LEFT) &&
                world.GetTiles(Maps::GetDirectionIndex(tile.GetIndex(), Direction::LEFT)).FindAddonLevel1(addon->uniq)) hero.SetVisited(Maps::GetDirectionIndex(tile.GetIndex(), Direction::LEFT));
            if(addon && Maps::isValidDirection(tile.GetIndex(), Direction::RIGHT) &&
                world.GetTiles(Maps::GetDirectionIndex(tile.GetIndex(), Direction::RIGHT)).FindAddonLevel1(addon->uniq)) hero.SetVisited(Maps::GetDirectionIndex(tile.GetIndex(), Direction::RIGHT));
        }
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToArtesianSpring: " << hero.GetName());
}

void AIToXanadu(Heroes &hero, const u8 obj, const s32 dst_index)
{
    const Maps::Tiles & tile = world.GetTiles(dst_index);
    const u8 level1 = hero.GetLevelSkill(Skill::Secondary::DIPLOMACY);
    const u8 level2 = hero.GetLevel();

    if(!hero.isVisited(tile) &&
      ((level1 == Skill::Level::BASIC && 7 < level2) ||
       (level1 == Skill::Level::ADVANCED && 5 < level2) ||
       (level1 == Skill::Level::EXPERT && 3 < level2) ||
       (9 < level2)))
    {
	hero.IncreasePrimarySkill(Skill::Primary::ATTACK);
	hero.IncreasePrimarySkill(Skill::Primary::DEFENSE);
	hero.IncreasePrimarySkill(Skill::Primary::KNOWLEDGE);
	hero.IncreasePrimarySkill(Skill::Primary::POWER);
	hero.SetVisited(dst_index);
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToXanadu: " << hero.GetName());
}

void AIToEvent(Heroes &hero, const u8 obj, const s32 dst_index)
{
    // check event maps
    const GameEvent::Coord* event_maps = world.GetEventMaps(hero.GetColor(), dst_index);
    if(event_maps)
    {
        if(event_maps->GetResource().GetValidItems())
    	    world.GetKingdom(hero.GetColor()).AddFundsResource(event_maps->GetResource());
	if(Artifact::UNKNOWN != event_maps->GetArtifact())
	    hero.PickupArtifact(event_maps->GetArtifact());
    }

    hero.SaveUnderObject(MP2::OBJ_ZERO);

    DEBUG(DBG_AI , DBG_INFO, "AIToEvent: " << hero.GetName());
}

void AIToUpgradeArmyObject(Heroes &hero, const u8 obj, const s32 dst_index)
{
    switch(obj)
    {
	case MP2::OBJ_HILLFORT:
	    if(hero.GetArmy().HasMonster(Monster::DWARF))
		hero.GetArmy().UpgradeMonsters(Monster::DWARF);
	    if(hero.GetArmy().HasMonster(Monster::ORC))
		hero.GetArmy().UpgradeMonsters(Monster::ORC);
	    if(hero.GetArmy().HasMonster(Monster::OGRE))
		hero.GetArmy().UpgradeMonsters(Monster::OGRE);
	    break;

	case MP2::OBJ_FREEMANFOUNDRY:
	    if(hero.GetArmy().HasMonster(Monster::PIKEMAN))
		hero.GetArmy().UpgradeMonsters(Monster::PIKEMAN);
	    if(hero.GetArmy().HasMonster(Monster::SWORDSMAN))
		hero.GetArmy().UpgradeMonsters(Monster::SWORDSMAN);
	    if(hero.GetArmy().HasMonster(Monster::IRON_GOLEM))
		hero.GetArmy().UpgradeMonsters(Monster::IRON_GOLEM);
	    break;

	default: break;
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToUpgradeArmyObject: " << hero.GetName());
}

void AIToPoorMoraleObject(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);

    const bool battle = (tile.GetQuantity1() || tile.GetQuantity2());
    bool complete = false;
    
    switch(obj)
    {
        case MP2::OBJ_GRAVEYARD:
	if(battle)
	{
	    Army::army_t army;
	    army.FromGuardian(tile);

    		// new battle2
    		Battle2::Result res = Battle2::Loader(hero.GetArmy(), army, dst_index);

    		if(res.AttackerWins())
		{
        	    hero.IncreaseExperience(res.GetExperienceAttacker());
	    	    complete = true;
	    	    const Artifact art(Artifact::FromInt(tile.GetQuantity1()));
	    	    Resource::funds_t resource;
	    	    resource.gold = tile.GetQuantity2() * 100;
	    	    hero.PickupArtifact(art());
	    	    world.GetKingdom(hero.GetColor()).AddFundsResource(resource);
	    	    hero.ActionAfterBattle();
		}
		else
		{
	    	    AIBattleLose(hero, res, true);
		}
	}
	break;

        case MP2::OBJ_SHIPWRECK:
	if(battle)
	{
		Army::army_t army;
		army.FromGuardian(tile);
		Resource::funds_t resource;
		Artifact::artifact_t art = Artifact::UNKNOWN;
                switch(tile.GetQuantity2())
                {
            	    case 10: resource.gold = 1000; break;
            	    case 15: resource.gold = 2000; break;
            	    case 25: resource.gold = 5000; break;
            	    case 50: resource.gold = 2000; art = Artifact::FromInt(tile.GetQuantity1()); break;
            	    default: DEBUG(DBG_AI , DBG_WARN, "ActionToPoorMoraleObject: unknown variant for ShipWreck, index: " << dst_index); break;
                }

    		    // new battle2
    		    Battle2::Result res = Battle2::Loader(hero.GetArmy(), army, dst_index);

    		    if(res.AttackerWins())
		    {
        		hero.IncreaseExperience(res.GetExperienceAttacker());
	    		complete = true;
			hero.PickupArtifact(art);
	    		world.GetKingdom(hero.GetColor()).AddFundsResource(resource);
	    		hero.ActionAfterBattle();
		    }
		    else
		    {
	    		AIBattleLose(hero, res, true);
		    }
	    }
	    break;

        case MP2::OBJ_DERELICTSHIP:
	if(battle)
	{
	    Army::army_t army;
	    army.FromGuardian(tile);

    		// new battle2
    		Battle2::Result res = Battle2::Loader(hero.GetArmy(), army, dst_index);

    		if(res.AttackerWins())
		{
        	    hero.IncreaseExperience(res.GetExperienceAttacker());
	    	    complete = true;
	    	    Resource::funds_t resource;
	    	    resource.gold = tile.GetQuantity2() * 100;
	    	    world.GetKingdom(hero.GetColor()).AddFundsResource(resource);
	    	    hero.ActionAfterBattle();
		}
		else
		{
	    	    AIBattleLose(hero, res, true);
		}
	}
	break;

	default: break;
    }

    if(complete)
    {
	tile.SetQuantity1(0);
	tile.SetQuantity2(0);
    }
    else
    if(!battle && !hero.isVisited(obj))
    {
	// modify morale
	hero.SetVisited(dst_index);
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToPoorMoraleObject: " << hero.GetName());
}

void AIToPoorLuckObject(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    const bool battle = tile.GetQuantity1();
    bool complete = false;

    switch(obj)
    {
        case MP2::OBJ_PYRAMID:
    	if(battle)
    	{
            // battle
            Army::army_t army;
            army.FromGuardian(tile);

    		// new battle2
    		Battle2::Result res = Battle2::Loader(hero.GetArmy(), army, dst_index);

    		if(res.AttackerWins())
		{
        	    hero.IncreaseExperience(res.GetExperienceAttacker());
		    complete = true;
		    const Spell::spell_t spell(static_cast<Spell::spell_t>(tile.GetQuantity1()));
		    // check magick book
		    if(hero.HasArtifact(Artifact::MAGIC_BOOK) &&
		    // check skill level for wisdom
			Skill::Level::EXPERT == hero.GetLevelSkill(Skill::Secondary::WISDOM))
		    {
			hero.AppendSpellToBook(spell);
		    }
		    hero.ActionAfterBattle();
		}
		else
		{
		    AIBattleLose(hero, res, true);
		}
	}
    	break;

    	default: break;
    }

    if(complete)
    {
	tile.SetQuantity1(0);
	tile.SetQuantity2(0);
    }
    else
    if(!battle && !hero.isVisited(obj))
    {
	// modify luck
        hero.SetVisited(dst_index);
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToPoorLuckObject: " << hero.GetName());
}

void AIToObelisk(Heroes &hero, const u8 obj, const s32 dst_index)
{
    if(!hero.isVisited(obj, Visit::GLOBAL))
    {
        hero.SetVisited(dst_index, Visit::GLOBAL);
        Kingdom & kingdom = world.GetKingdom(hero.GetColor());
        kingdom.PuzzleMaps().Update(kingdom.CountVisitedObjects(MP2::OBJ_OBELISK), world.CountObeliskOnMaps());
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToObelisk: " << hero.GetName());
}

void AIToTreeKnowledge(Heroes &hero, const u8 obj, const s32 dst_index)
{
    const Maps::Tiles & tile = world.GetTiles(dst_index);

    if(!hero.isVisited(tile))
    {
	Resource::funds_t payment;
	switch(tile.GetQuantity2())
	{
	    case 10:	payment.gems = 10; break;
	    case 20:	payment.gold = 2000; break;
	    default:	break;
	}

    	if(!payment.GetValidItems() || world.GetKingdom(hero.GetColor()).AllowPayment(payment))
    	{
	    if(payment.GetValidItems()) world.GetKingdom(hero.GetColor()).OddFundsResource(payment);
	    hero.SetVisited(dst_index);
	    hero.IncreaseExperience(hero.GetExperienceFromLevel(hero.GetLevel()) - hero.GetExperience());
	}
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToTreeKnowledge: " << hero.GetName());
}


void AIToDaemonCave(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);

    if(tile.GetQuantity2())
    {
	Resource::funds_t resource;

	// check variants
	switch(tile.GetQuantity2())
	{
	    case 1:
    		hero.IncreaseExperience(1000);
		tile.SetQuantity2(0);
		break;
	    case 2:
	    {
		const Artifact::artifact_t art = Artifact::FromInt(tile.GetQuantity1());
		if(Artifact::UNKNOWN != art) hero.PickupArtifact(art);
    		hero.IncreaseExperience(1000);
		tile.SetQuantity1(Artifact::UNKNOWN);
		tile.SetQuantity2(0);
		break;
	    }
	    case 3:
		resource.gold = 2500;
    		hero.IncreaseExperience(1000);
		world.GetKingdom(hero.GetColor()).AddFundsResource(resource);
		tile.SetQuantity2(0);
		break;
	    default:
		break;
	}
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToDaemonCave: " << hero.GetName());
}

void AIToDwellingJoinMonster(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    const u32 count = tile.GetCountMonster();

    if(count && hero.GetArmy().JoinTroop(Monster(Monster::FromObject(obj)), count)) tile.SetCountMonster(0);

    DEBUG(DBG_AI , DBG_INFO, "AIToDwellingJoinMonster: " << hero.GetName());
}

void AIToDwellingRecruitMonster(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    const u32 count = tile.GetCountMonster();

    if(count)
    {
        Kingdom & kingdom = world.GetKingdom(hero.GetColor());
        const Monster monster(Monster::FromObject(obj));
	const payment_t paymentCosts(PaymentConditions::BuyMonster(monster()) * count);
	const Resource::funds_t & kingdomResource = kingdom.GetFundsResource();

        if(paymentCosts <= kingdomResource && hero.GetArmy().JoinTroop(monster, count))
        {
    	    tile.SetCountMonster(0);
	    kingdom.OddFundsResource(paymentCosts);

	    // remove ancient lamp sprite
	    if(MP2::OBJ_ANCIENTLAMP == obj)
	    {
	        tile.RemoveObjectSprite();
	        tile.SetObject(MP2::OBJ_ZERO);
	    }
	}
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToDwellingJoinMonster: " << hero.GetName());
}

void AIToStables(Heroes &hero, const u8 obj, const s32 dst_index)
{
    // check already visited
    if(!hero.isVisited(obj))
    {
        hero.SetVisited(dst_index);
        hero.IncreaseMovePoints(400);
    }

    if(hero.GetArmy().HasMonster(Monster::CAVALRY)) hero.GetArmy().UpgradeMonsters(Monster::CAVALRY);
                                                                
    DEBUG(DBG_AI , DBG_INFO, "AIToStables: " << hero.GetName());
}

void AIToAbandoneMine(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);

    Army::army_t army;
    army.FromGuardian(tile);

    	// new battle2
    	Battle2::Result res = Battle2::Loader(hero.GetArmy(), army, dst_index);

    	if(res.AttackerWins())
	{
            hero.IncreaseExperience(res.GetExperienceAttacker());
            tile.SetQuantity1(0);
            tile.UpdateAbandoneMineSprite();
            world.CaptureObject(dst_index, hero.GetColor());
            hero.SaveUnderObject(MP2::OBJ_MINES);
            hero.ActionAfterBattle();
        }
	else
	{
	    AIBattleLose(hero, res, true);
	}

    DEBUG(DBG_AI , DBG_INFO, "AIToAbandoneMine: " << hero.GetName());
}

void AIToBarrier(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    Kingdom & kingdom = world.GetKingdom(hero.GetColor());

    if(kingdom.IsVisitTravelersTent(tile.GetQuantity1()))
    {
        tile.RemoveObjectSprite();
        tile.SetObject(MP2::OBJ_ZERO);
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToBarrier: " << hero.GetName());
}

void AIToTravellersTent(Heroes &hero, const u8 obj, const s32 dst_index)
{
    const Maps::Tiles & tile = world.GetTiles(dst_index);
    Kingdom & kingdom = world.GetKingdom(hero.GetColor());

    kingdom.SetVisitTravelersTent(tile.GetQuantity1());

    DEBUG(DBG_AI , DBG_INFO, "AIToTravellersTent: " << hero.GetName());
}

void AIToShipwreckSurvivor(Heroes &hero, const u8 obj, const s32 dst_index)
{
    Maps::Tiles & tile = world.GetTiles(dst_index);
    const Artifact art(Artifact::FromInt(tile.GetQuantity1()));

    if(!hero.PickupArtifact(art()))
    {
        Resource::funds_t prize(Resource::GOLD, 1500);
	world.GetKingdom(hero.GetColor()).OddFundsResource(prize);
    }

    tile.RemoveObjectSprite();
    tile.SetQuantity1(0);
    tile.SetObject(MP2::OBJ_ZERO);

    DEBUG(DBG_AI , DBG_INFO, "AIToShipwreckSurvivor: " << hero.GetName());
}

void AIToArtifact(Heroes &hero, const u8 obj, const s32 dst_index)
{
    if(hero.IsFullBagArtifacts()) return;

    Maps::Tiles & tile = world.GetTiles(dst_index);
    Artifact art(Artifact::FromInt(tile.GetQuantity1()));

    // update scroll artifact
    if(art.GetID() == Artifact::SPELL_SCROLL)
	art.SetExt(tile.GetQuantity3());

    bool conditions = false;

    switch(tile.GetQuantity2())
    {
	// 1,2,3 - 2000g, 2500g+3res,3000g+5res
	case 1:
	case 2:
	case 3:
	{
	    Resource::funds_t payment;

	    if(1 == tile.GetQuantity2())
		payment += Resource::funds_t(Resource::GOLD, 2000);
	    else
	    if(2 == tile.GetQuantity2())
	    {
		payment += Resource::funds_t(Resource::GOLD, 2500);
		payment += Resource::funds_t(tile.GetQuantity4(), 3);
	    }
	    else
	    {
		payment += Resource::funds_t(Resource::GOLD, 3000);
		payment += Resource::funds_t(tile.GetQuantity4(), 5);
	    }

	    if(world.GetKingdom(hero.GetColor()).AllowPayment(payment))
	    {
		conditions = true;
		world.GetKingdom(hero.GetColor()).OddFundsResource(payment);
	    }
	    break;
	}

	// 4,5 - need have skill wisard or leadership,
	case 4:
	case 5:
	{
	    conditions = hero.HasSecondarySkill(4 == tile.GetQuantity2() ? Skill::Secondary::WISDOM : Skill::Secondary::LEADERSHIP);
	    break;
	}

	// 6 - 50 rogues, 7 - 1 gin, 8,9,10,11,12,13 - 1 monster level4
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	{
	    Army::army_t army;
	    army.FromGuardian(tile);

	    // new battle2
	    Battle2::Result res = Battle2::Loader(hero.GetArmy(), army, dst_index);
    	    if(res.AttackerWins())
    	    {
		hero.IncreaseExperience(res.GetExperienceAttacker());
		conditions = true;
		hero.ActionAfterBattle();
	    }
	    else
	    {
		AIBattleLose(hero, res, true);
	    }
	    break;
	}

    	default:
	    conditions = true;
	    break;
    }

    if(conditions && hero.PickupArtifact(art))
    {
	tile.RemoveObjectSprite();
	tile.SetObject(MP2::OBJ_ZERO);
	tile.ResetQuantity();
    }

    DEBUG(DBG_AI , DBG_INFO, "AIToArtifact: " << hero.GetName());
}

void AIToBoat(Heroes &hero, const u8 obj, const s32 dst_index)
{
    if(hero.isShipMaster()) return;

    const s32 from_index = hero.GetIndex();

    Maps::Tiles & tiles_from = world.GetTiles(from_index);
    Maps::Tiles & tiles_to = world.GetTiles(dst_index);

    // disabled nearest coasts (on week MP2::isWeekLife)
    std::vector<s32> coasts;
    Maps::ScanDistanceObject(from_index, MP2::OBJ_COAST, 4, coasts);
    coasts.push_back(from_index);
    for(std::vector<s32>::const_iterator
	it = coasts.begin(); it != coasts.end(); ++it) hero.SetVisited(*it);

    hero.ResetMovePoints();
    tiles_from.SetObject(MP2::OBJ_COAST);
    hero.SetIndex(dst_index);
    hero.SetShipMaster(true);
    tiles_to.SetObject(MP2::OBJ_HEROES);
    hero.SaveUnderObject(MP2::OBJ_ZERO);
    hero.ClearAITasks();

    DEBUG(DBG_AI, DBG_INFO, "AIToBoat: " << hero.GetName());
}

void AIToCoast(Heroes &hero, const u8 obj, const s32 dst_index)
{
    if(! hero.isShipMaster()) return;

    s32 from_index = hero.GetIndex();

    Maps::Tiles & tiles_from = world.GetTiles(from_index);
    Maps::Tiles & tiles_to = world.GetTiles(dst_index);

    hero.ResetMovePoints();
    tiles_from.SetObject(MP2::OBJ_BOAT);
    hero.SetIndex(dst_index);
    hero.SetShipMaster(false);
    tiles_to.SetObject(MP2::OBJ_HEROES);
    hero.SaveUnderObject(MP2::OBJ_ZERO);
    hero.ClearAITasks();

    hero.ActionNewPosition();

    DEBUG(DBG_AI, DBG_INFO, "AIToCoast: " << hero.GetName());
}


                                                        










/* get priority object for AI independent of distance (1 day) */
bool Heroes::AIPriorityObject(s32 index)
{
    const Settings & conf = Settings::Get();
    Maps::Tiles & tile = world.GetTiles(index);

    if(Modes(HUNTER))
	switch(tile.GetObject())
	{
	    // capture enemy castle
	    case MP2::OBJ_CASTLE:
	    {
		const Castle *castle = world.GetCastle(index);
		return castle &&
		    !conf.IsUnions(GetColor(), castle->GetColor()) &&
		    AIValidObject(index);
	    }
	    break;

	    // kill enemy hero
	    case MP2::OBJ_HEROES:
	    {
		const Heroes *hero = world.GetHeroes(index);
		return hero &&
		    !conf.IsUnions(GetColor(), hero->GetColor()) &&
		    AIValidObject(index);
	    }
	    break;

	    case MP2::OBJ_MONSTER:
		return AIValidObject(index);

	    default: break;
	}

    if(Modes(SCOUTER))
	switch(tile.GetObject())
	{
	    case MP2::OBJ_ARTIFACT:
	    // resource
	    case MP2::OBJ_RESOURCE:
	    case MP2::OBJ_CAMPFIRE:
	    case MP2::OBJ_TREASURECHEST:
	    // free mines
	    case MP2::OBJ_SAWMILL:
	    case MP2::OBJ_MINES:
	    case MP2::OBJ_ALCHEMYLAB:
		return AIValidObject(index);

	    default: break;
	}

    return false;
}

bool Heroes::AIValidObject(s32 index)
{
    Maps::Tiles & tile = world.GetTiles(index);
    const u8 obj = tile.GetObject();

    // check other
    switch(obj)
    {
	// water object
	case MP2::OBJ_SHIPWRECKSURVIROR:
	case MP2::OBJ_WATERCHEST:
	case MP2::OBJ_FLOTSAM:
	case MP2::OBJ_BOTTLE:
	case MP2::OBJ_BUOY:

	case MP2::OBJ_MAGELLANMAPS:
	case MP2::OBJ_MERMAID:
	case MP2::OBJ_SIRENS:
	case MP2::OBJ_WHIRLPOOL:
	case MP2::OBJ_COAST:
	    if(isShipMaster()) return true;
            break;

	// capture objects
	case MP2::OBJ_SAWMILL:
	case MP2::OBJ_MINES:
	case MP2::OBJ_ALCHEMYLAB:
	    if(GetColor() != world.ColorCapturedObject(tile.GetIndex()))
	    {
		if(tile.CheckEnemyGuardians(GetColor()))
		{
		    Army::army_t enemy;
		    enemy.FromGuardian(tile);
		    return !enemy.isValid() || GetArmy().StrongerEnemyArmy(enemy);
		}
		else return true;
	    }
	    break;

	// pickup object
	case MP2::OBJ_WAGON:
	case MP2::OBJ_WATERWHEEL:
	case MP2::OBJ_WINDMILL:
	case MP2::OBJ_LEANTO:
	case MP2::OBJ_MAGICGARDEN:
	case MP2::OBJ_SKELETON:
	    if(tile.ValidQuantity()) return true;
	    break;

	// pickup resource
	case MP2::OBJ_RESOURCE:
	case MP2::OBJ_CAMPFIRE:
	case MP2::OBJ_TREASURECHEST:
	    return true;

	case MP2::OBJ_ARTIFACT:
	{
	    if(IsFullBagArtifacts()) return false;

	    // 1,2,3 - 2000g, 2500g+3res, 3000g+5res
	    if(1 <= tile.GetQuantity2() && 3 >= tile.GetQuantity2())
	    {
		Resource::funds_t payment;
		if(1 == tile.GetQuantity2())
		    payment += Resource::funds_t(Resource::GOLD, 2000);
		else
		if(2 == tile.GetQuantity2())
		{
		    payment += Resource::funds_t(Resource::GOLD, 2500);
		    payment += Resource::funds_t(tile.GetQuantity4(), 3);
		}
		else
		{
		    payment += Resource::funds_t(Resource::GOLD, 3000);
		    payment += Resource::funds_t(tile.GetQuantity4(), 5);
		}
		return world.GetKingdom(GetColor()).AllowPayment(payment);
	    }
	    else
	    // 4,5 - need have skill wisard or leadership,
	    if(3 < tile.GetQuantity2() && 6 > tile.GetQuantity2())
	    {
		return HasSecondarySkill(4 == tile.GetQuantity2() ? Skill::Secondary::WISDOM : Skill::Secondary::LEADERSHIP);
	    }
	    else
	    // 6 - 50 rogues, 7 - 1 gin, 8,9,10,11,12,13 - 1 monster level4
	    if(5 < tile.GetQuantity2() && 14 > tile.GetQuantity2())
	    {
		Army::army_t enemy;
		enemy.FromGuardian(tile);
		return !enemy.isValid() || GetArmy().StrongerEnemyArmy(enemy);
	    }
	}
	break;

	// increase view
	case MP2::OBJ_OBSERVATIONTOWER:
	// obelisk
	case MP2::OBJ_OBELISK:
	    if(! isVisited(tile, Visit::GLOBAL)) return true;
	    break;

        case MP2::OBJ_BARRIER:
	    if(world.GetKingdom(GetColor()).IsVisitTravelersTent(tile.GetQuantity1())) return true;
	    break;

        case MP2::OBJ_TRAVELLERTENT:
	    if(!world.GetKingdom(GetColor()).IsVisitTravelersTent(tile.GetQuantity1())) return true;
	    break;

	// new spell
        case MP2::OBJ_SHRINE1:
	case MP2::OBJ_SHRINE2:
	case MP2::OBJ_SHRINE3:
	    if( // check spell book
		HasArtifact(Artifact::MAGIC_BOOK) &&
		!HaveSpell(Spell::FromInt(tile.GetQuantity1())) &&
                // check valid level spell and wisdom skill
                !(3 == Spell::Level(Spell::FromInt(tile.GetQuantity1())) &&
                Skill::Level::NONE == GetLevelSkill(Skill::Secondary::WISDOM))) return true;
	    break;

    	// primary skill
	case MP2::OBJ_FORT:
    	case MP2::OBJ_MERCENARYCAMP:
    	case MP2::OBJ_DOCTORHUT:
    	case MP2::OBJ_STANDINGSTONES:
	// sec skill
	case MP2::OBJ_WITCHSHUT:
	// exp
	case MP2::OBJ_GAZEBO:
	    if(! isVisited(tile)) return true;
	    break;

	case MP2::OBJ_TREEKNOWLEDGE:
	    if(! isVisited(tile))
	    {
		Resource::funds_t payment;
		switch(tile.GetQuantity2())
		{
		    case 10:	payment.gems = 10; break;
		    case 20:	payment.gold = 2000; break;
		    default:	break;
		}

    		if(!payment.GetValidItems() || world.GetKingdom(GetColor()).AllowPayment(payment))
		    return true;
	    }
	    break;

    	// good luck
        case MP2::OBJ_FOUNTAIN:
    	case MP2::OBJ_FAERIERING:
    	case MP2::OBJ_IDOL:
	    if(! isVisited(obj) &&
		Luck::IRISH > GetLuck()) return true;
	    break;

	// good morale
	case MP2::OBJ_OASIS:
    	case MP2::OBJ_TEMPLE:
    	case MP2::OBJ_WATERINGHOLE:
	    if(! isVisited(obj) &&
		Morale::BLOOD > GetMorale()) return true;
	    break;

	case MP2::OBJ_MAGICWELL:
	    if(! isVisited(tile) &&
		GetMaxSpellPoints() != GetSpellPoints()) return true;
	    break;

	case MP2::OBJ_ARTESIANSPRING:
	    if(! isVisited(tile) &&
		2 * GetMaxSpellPoints() > GetSpellPoints()) return true;
	    break;

	case MP2::OBJ_XANADU:
	{
	    const u8 level1 = GetLevelSkill(Skill::Secondary::DIPLOMACY);
	    const u8 level2 = GetLevel();

	    if(!isVisited(tile) &&
		((level1 == Skill::Level::BASIC && 7 < level2) ||
		(level1 == Skill::Level::ADVANCED && 5 < level2) ||
		(level1 == Skill::Level::EXPERT && 3 < level2) || (9 < level2))) return true;
	    break;
	}

        // accept army
        case MP2::OBJ_WATCHTOWER:
        case MP2::OBJ_EXCAVATION:
        case MP2::OBJ_CAVE:
        case MP2::OBJ_TREEHOUSE:
        case MP2::OBJ_ARCHERHOUSE:
        case MP2::OBJ_GOBLINHUT:
        case MP2::OBJ_DWARFCOTT:
	case MP2::OBJ_HALFLINGHOLE:
        case MP2::OBJ_PEASANTHUT:
        case MP2::OBJ_THATCHEDHUT:
        {
    	    Monster mons = Monster::FromObject(obj);
	    if(tile.GetCountMonster() &&
		(army.HasMonster(mons) ||
		(army.GetCount() < army.Size() && (mons.isArchers() || mons.isFly())))) return true;
	    break;
	}

        // recruit army
        case MP2::OBJ_RUINS:
        case MP2::OBJ_TREECITY:
        case MP2::OBJ_WAGONCAMP:
	case MP2::OBJ_DESERTTENT:
        case MP2::OBJ_WATERALTAR:
        case MP2::OBJ_AIRALTAR:
        case MP2::OBJ_FIREALTAR:
        case MP2::OBJ_EARTHALTAR:
        case MP2::OBJ_BARROWMOUNDS:
	{
	    const u32 count = tile.GetCountMonster();
    	    const Monster monster(Monster::FromObject(obj));
	    const payment_t paymentCosts(PaymentConditions::BuyMonster(monster()) * count);
	    const Resource::funds_t & kingdomResource = world.GetKingdom(GetColor()).GetFundsResource();

	    if(count && paymentCosts <= kingdomResource &&
		(army.HasMonster(monster) ||
		(army.GetCount() < army.Size() && (monster.isArchers() || monster.isFly())))) return true;
	    break;
	}

        // recruit army (battle)
        case MP2::OBJ_DRAGONCITY:
        case MP2::OBJ_CITYDEAD:
        case MP2::OBJ_TROLLBRIDGE:
        {
    	    const bool battle = (Color::GRAY == world.ColorCapturedObject(index));
	    const u32 count = tile.GetCountMonster();
    	    const Monster monster(Monster::FromObject(obj));
	    const payment_t paymentCosts(PaymentConditions::BuyMonster(monster()) * count);
	    const Resource::funds_t & kingdomResource = world.GetKingdom(GetColor()).GetFundsResource();

	    if(!battle && count && paymentCosts <= kingdomResource &&
		(army.HasMonster(monster) ||
		(army.GetCount() < army.Size()))) return true;
	    break;
        }

	// recruit genie
	case MP2::OBJ_ANCIENTLAMP:
	{
	    const u32 count = tile.GetCountMonster();
	    const payment_t paymentCosts(PaymentConditions::BuyMonster(Monster::GENIE) * count);
	    const Resource::funds_t & kingdomResource = world.GetKingdom(GetColor()).GetFundsResource();

	    if(count && paymentCosts <= kingdomResource &&
		(army.HasMonster(Monster::GENIE) ||
		(army.GetCount() < army.Size()))) return true;
	    break;
	}

	// upgrade army
	case MP2::OBJ_HILLFORT:
            if(army.HasMonster(Monster::DWARF) ||
               army.HasMonster(Monster::ORC) ||
               army.HasMonster(Monster::OGRE)) return true;
            break;

	// upgrade army
        case MP2::OBJ_FREEMANFOUNDRY:
            if(army.HasMonster(Monster::PIKEMAN) ||
               army.HasMonster(Monster::SWORDSMAN) ||
               army.HasMonster(Monster::IRON_GOLEM)) return true;
            break;

	// loyalty obj
	case MP2::OBJ_STABLES:
	    if(army.HasMonster(Monster::CAVALRY) ||
		! isVisited(tile)) return true;
	    break;

	case MP2::OBJ_ARENA:
	    if(! isVisited(tile)) return true;
	    break;

	// poor morale obj
	case MP2::OBJ_SHIPWRECK:
        case MP2::OBJ_GRAVEYARD:
	case MP2::OBJ_DERELICTSHIP:
	    if(! isVisited(tile, Visit::GLOBAL))
	    {
		if(tile.CheckEnemyGuardians(GetColor()))
		{
		    Army::army_t enemy;
		    enemy.FromGuardian(tile);
		    return enemy.isValid() && GetArmy().StrongerEnemyArmy(enemy);
		}
	    }
	    break;

	//case MP2::OBJ_PYRAMID:

	case MP2::OBJ_DAEMONCAVE:
	    if(tile.GetQuantity2() && 4 != tile.GetQuantity2()) return true;
	    break;

	case MP2::OBJ_MONSTER:
	{
	    Army::army_t enemy;
	    enemy.FromGuardian(tile);
	    return !enemy.isValid() || GetArmy().StrongerEnemyArmy(enemy);
	}
	break;

	// sign
	case MP2::OBJ_SIGN:
	    if(!isVisited(tile)) return true;
	    break;

	case MP2::OBJ_CASTLE:
	{
	    const Settings & conf = Settings::Get();
	    const Castle *castle = world.GetCastle(index);
	    if(castle)
	    {
		if(GetColor() == castle->GetColor())
		    return NULL == castle->GetHeroes() && ! isVisited(tile);
		else
		// FIXME: AI skip visiting alliance
		if(conf.IsUnions(GetColor(), castle->GetColor())) return false;
		else
		if(GetArmy().StrongerEnemyArmy(castle->GetActualArmy())) return true;
	    }
	    break;
	}

	case MP2::OBJ_HEROES:
	{
	    const Settings & conf = Settings::Get();
	    const Heroes *hero = world.GetHeroes(index);
	    if(hero)
	    {
		if(GetColor() == hero->GetColor()) return true;
		// FIXME: AI skip visiting alliance
		else
		if(conf.IsUnions(GetColor(), hero->GetColor())) return false;
		else
		if(hero->AllowBattle() &&
		    GetArmy().StrongerEnemyArmy(hero->GetArmy())) return true;
	    }
	    break;
	}

	case MP2::OBJ_BOAT:
	case MP2::OBJ_STONELIGHTS:
	    // check later
	    return true;

	/*
	    or add later
	*/

	default: break;
    }

    return false;
}

void Heroes::AIRescueWhereMove(void)
{
    u8 scoute = GetScoute();

    switch(Settings::Get().GameDifficulty())
    {
        case Difficulty::NORMAL:    scoute += 2; break;
        case Difficulty::HARD:      scoute += 3; break;
        case Difficulty::EXPERT:    scoute += 4; break;
        case Difficulty::IMPOSSIBLE:scoute += 6; break;
        default: break;
    }

    scoute += 1;
    const Point & mp = GetCenter();

    // find unchartered territory
    for(u8 ii = 1; ii <= scoute; ++ii)
    {
        const s32 tx = mp.x - ii;
        const s32 ty = mp.y - ii;

        const s32 mx = tx + 2 * ii;
        const s32 my = ty + 2 * ii;

        for(s32 iy = ty; iy <= my; ++iy)
            for(s32 ix = tx; ix <= mx; ++ix)
        {
            if(ty < iy && iy < my && tx < ix && ix < mx) continue;

            const s32 res = Maps::GetIndexFromAbsPoint(ix, iy);

            if(Maps::isValidAbsIndex(res) &&
                world.GetTiles(res).isFog(color) &&
                world.GetTiles(res).isPassable(this, true) &&
		GetPath().Calculate(res))
            {
                ai_sheduled_visit.push_back(res);

		DEBUG(DBG_AI , DBG_INFO, "Heroes::AIRescueMove: " << Color::String(GetColor()) <<
                	", hero: " << GetName() << ", added task: " << res);

                return;
            }
        }
    }

    if(MP2::OBJ_STONELIGHTS == save_maps_object ||
	MP2::OBJ_WHIRLPOOL == save_maps_object)
    {
	AIAction(GetIndex());
    }
}

void Heroes::AIMeeting(Heroes & heroes2)
{
    if(Settings::Get().ExtEyeEagleAsScholar())
        Heroes::ScholarAction(*this, heroes2);

    if(Modes(HUNTER))
	GetArmy().JoinStrongestFromArmy(heroes2.GetArmy());
    else
    if(heroes2.Modes(HUNTER))
	heroes2.GetArmy().JoinStrongestFromArmy(GetArmy());
}

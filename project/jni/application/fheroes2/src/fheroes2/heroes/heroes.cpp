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

#include <cmath>
#include <algorithm>
#include "artifact.h"
#include "world.h"
#include "castle.h"
#include "settings.h"
#include "agg.h"
#include "speed.h"
#include "luck.h"
#include "morale.h"
#include "monster.h"
#include "payment.h"
#include "profit.h"
#include "cursor.h"
#include "kingdom.h"
#include "visit.h"
#include "battle2.h"
#include "heroes.h"
#include "localclient.h"
#include "game_focus.h"
#include "game_interface.h"

// heroes_action.cpp
u16 DialogWithArtifact(const std::string & hdr, const std::string & msg, const Artifact::artifact_t art, const u16 buttons = Dialog::OK);
void PlayPickupSound(void);

const char* HeroesName(Heroes::heroes_t id)
{
    const char* names[] = {
	// knight
	_("Lord Kilburn"), _("Sir Gallanth"), _("Ector"), _("Gwenneth"), _("Tyro"), _("Ambrose"), _("Ruby"), _("Maximus"), _("Dimitry"),
	// barbarian
	_("Thundax"), _("Fineous"), _("Jojosh"), _("Crag Hack"), _("Jezebel"), _("Jaclyn"), _("Ergon"), _("Tsabu"), _("Atlas"),
	// sorceress
	_("Astra"), _("Natasha"), _("Troyan"), _("Vatawna"), _("Rebecca"), _("Gem"), _("Ariel"), _("Carlawn"), _("Luna"),
	// warlock
	_("Arie"), _("Alamar"), _("Vesper"), _("Crodo"), _("Barok"), _("Kastore"), _("Agar"), _("Falagar"), _("Wrathmont"),
	// wizard
	_("Myra"), _("Flint"), _("Dawn"), _("Halon"), _("Myrini"), _("Wilfrey"), _("Sarakin"), _("Kalindra"), _("Mandigal"),
	// necromant
	_("Zom"), _("Darlana"), _("Zam"), _("Ranloo"), _("Charity"), _("Rialdo"), _("Roxana"), _("Sandro"), _("Celia"),
	// campains
	_("Roland"), _("Lord Corlagon"), _("Sister Eliza"), _("Archibald"), _("Lord Halton"), _("Brother Bax"),
	// loyalty version
	_("Solmyr"), _("Dainwin"), _("Mog"), _("Uncle Ivan"), _("Joseph"), _("Gallavant"), _("Elderian"), _("Ceallach"), _("Drakonia"), _("Martine"), _("Jarkonas"),
	// debug
	"SandySandy", "Unknown" };

    return names[id];
}

typedef std::vector< std::pair<MP2::object_t, s8> > ObjectVisitedModifiers;

s8 GetResultModifiers(const ObjectVisitedModifiers & modifiers, const Heroes & hero, std::string* strs)
{
    s8 result = 0;

    for(size_t ii = 0; ii < modifiers.size(); ++ii)
    {
        const std::pair<MP2::object_t, s8> & pair = modifiers[ii];

        if(hero.isVisited(pair.first))
        {
            result += pair.second;

    	    if(strs)
    	    {
		strs->append(MP2::StringObject(pair.first));
		StringAppendModifiers(*strs, pair.second);
		strs->append("\n");
	    }
        }
    }

    return result;
}

Heroes::heroes_t Heroes::ConvertID(u8 index)
{
    return index > UNKNOWN ? UNKNOWN : static_cast<heroes_t>(index);
}

Heroes::Heroes() : move_point_scale(-1), army(this), path(*this), 
    direction(Direction::RIGHT), sprite_index(18), patrol_square(0),
    ai_primary_target(-1)
{
    bag_artifacts.assign(HEROESMAXARTIFACT, Artifact::UNKNOWN);
}

Heroes::Heroes(heroes_t ht, Race::race_t rc) : killer_color(Color::GRAY), experience(0), move_point(0),
    move_point_scale(-1), army(this), portrait(ht), race(rc),
    save_maps_object(MP2::OBJ_ZERO), path(*this), direction(Direction::RIGHT), sprite_index(18), patrol_square(0),
    ai_primary_target(-1)
{
    name = _(HeroesName(ht));

    bag_artifacts.assign(HEROESMAXARTIFACT, Artifact::UNKNOWN);

    u8 book, spell;
    Skill::Primary::LoadDefaults(race, *this, book, spell);

    secondary_skills.reserve(HEROESMAXSKILL);
    Skill::Secondary::LoadDefaults(race, secondary_skills);

    if(book)
    {
        SpellBookActivate();
        AppendSpellToBook(Spell::FromInt(spell));
        if(!HasArtifact(Artifact::MAGIC_BOOK)) PickupArtifact(Artifact::MAGIC_BOOK);
    }

    // hero is freeman
    color = Color::GRAY;

    // set default army
    army.Reset(true);

    // extra hero
    switch(portrait)
    {
        case ROLAND:
            attack    = 0;
            defense   = 1;
            power     = 4;
            knowledge = 5;

	    secondary_skills.clear();
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::WISDOM, Skill::Level::ADVANCED));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::LEADERSHIP, Skill::Level::EXPERT));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::ARCHERY, Skill::Level::BASIC));
    	    break;

        case CORLAGON:
            attack    = 5;
            defense   = 3;
            power     = 1;
            knowledge = 1;

	    secondary_skills.clear();
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::NECROMANCY, Skill::Level::EXPERT));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::BALLISTICS, Skill::Level::BASIC));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::WISDOM, Skill::Level::BASIC));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::LEADERSHIP, Skill::Level::BASIC));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::PATHFINDING, Skill::Level::BASIC));
    	    break;
        
        case ELIZA:
            attack    = 0;
            defense   = 1;
            power     = 2;
            knowledge = 6;

	    secondary_skills.clear();
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::NAVIGATION, Skill::Level::ADVANCED));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::WISDOM, Skill::Level::EXPERT));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::ARCHERY, Skill::Level::BASIC));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::LUCK, Skill::Level::BASIC));
    	    break;

    	case ARCHIBALD:
            attack    = 1;
            defense   = 1;
            power     = 4;
            knowledge = 4;

	    secondary_skills.clear();
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::SCOUTING, Skill::Level::EXPERT));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::LEADERSHIP, Skill::Level::EXPERT));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::WISDOM, Skill::Level::ADVANCED));
    	    break;

    	case HALTON:
            attack    = 3;
            defense   = 3;
            power     = 3;
            knowledge = 2;

	    secondary_skills.clear();
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::BALLISTICS, Skill::Level::BASIC));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::LEADERSHIP, Skill::Level::ADVANCED));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::DIPLOMACY, Skill::Level::BASIC));
    	    break;
    	
    	case BAX:
            attack    = 1;
            defense   = 1;
            power     = 4;
            knowledge = 3;

	    secondary_skills.clear();
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::WISDOM, Skill::Level::EXPERT));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::NECROMANCY, Skill::Level::BASIC));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::NAVIGATION, Skill::Level::BASIC));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::PATHFINDING, Skill::Level::BASIC));
    	    break;

	case SOLMYR:
	case DRAKONIA:
	    secondary_skills.clear();
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::WISDOM, Skill::Level::ADVANCED));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::LEADERSHIP, Skill::Level::BASIC));
	    break;

	case DAINWIN:
	case ELDERIAN:
	    secondary_skills.clear();
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::WISDOM, Skill::Level::ADVANCED));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::SCOUTING, Skill::Level::BASIC));
	    break;

	case MOG:
	    secondary_skills.clear();
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::WISDOM, Skill::Level::BASIC));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::NECROMANCY, Skill::Level::ADVANCED));
	    break;
	
	case UNCLEIVAN:
	    secondary_skills.clear();
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::PATHFINDING, Skill::Level::ADVANCED));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::LEADERSHIP, Skill::Level::BASIC));
	    break;
	
	case JOSEPH:
	    secondary_skills.clear();
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::LEADERSHIP, Skill::Level::BASIC));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::SCOUTING, Skill::Level::BASIC));
	    break;
	
	case GALLAVANT:
	    break;
	
	case CEALLACH:
	    break;
	
	case MARTINE:
	    break;

	case JARKONAS:
	    break;

    	case SANDYSANDY:
	    army.Clear();
	    army.JoinTroop(Monster::BLACK_DRAGON, 2);
	    army.JoinTroop(Monster::RED_DRAGON, 3);

	    secondary_skills.clear();
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::PATHFINDING, Skill::Level::ADVANCED));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::LOGISTICS, Skill::Level::ADVANCED));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::SCOUTING, Skill::Level::BASIC));
	    secondary_skills.push_back(Skill::Secondary(Skill::Secondary::MYSTICISM, Skill::Level::BASIC));

	    PickupArtifact(Artifact::STEALTH_SHIELD);
	    PickupArtifact(Artifact::DRAGON_SWORD);
	    PickupArtifact(Artifact::NOMAD_BOOTS_MOBILITY);
	    PickupArtifact(Artifact::TRAVELER_BOOTS_MOBILITY);
	    PickupArtifact(Artifact::TRUE_COMPASS_MOBILITY);

	    experience = 777;

	    // all spell in magic book
	    for(u8 spell = Spell::FIREBALL; spell < Spell::STONE; ++spell) AppendSpellToBook(Spell::FromInt(spell), true);
	    break;

	default: break;
    }

    SetSpellPoints(GetMaxSpellPoints());
    move_point = GetMaxMovePoints();
}

void Heroes::LoadFromMP2(s32 map_index, const void *ptr, const Color::color_t cl, const Race::race_t rc)
{
    // reset modes
    modes = 0;

    SetIndex(map_index);

    color = cl;
    killer_color = Color::GRAY;

    const u8  *ptr8  = static_cast<const u8 *>(ptr);
    u16 byte16 = 0;
    u32 byte32 = 0;

    // unknown
    ++ptr8;

    // custom troops
    bool custom_troop = false;
    if(*ptr8)
    {
        ++ptr8;
	custom_troop = true;

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

    // custom portrate
    bool custom_portrait = false;
    if(*ptr8)
    {
	custom_portrait = true;
	++ptr8;
	// index sprite portrait
	if(portrait != ConvertID(*ptr8)) DEBUG(DBG_GAME, DBG_WARN, "Heroes::LoadFromMP2: " << "custom portrait incorrect");
	++ptr8;
    }
    else
	ptr8 += 2;

    // artifacts
    Artifact::artifact_t artifact = Artifact::UNKNOWN;

    // artifact 1
    if(Artifact::UNKNOWN != (artifact = Artifact::FromInt(*ptr8))) PickupArtifact(artifact);
    ++ptr8;

    // artifact 2
    if(Artifact::UNKNOWN != (artifact = Artifact::FromInt(*ptr8))) PickupArtifact(artifact);
    ++ptr8;

    // artifact 3
    if(Artifact::UNKNOWN != (artifact = Artifact::FromInt(*ptr8))) PickupArtifact(artifact);
    ++ptr8;

    // unknown byte
    ++ptr8;

    // experience
    byte32 = ReadLE32(ptr8);
    experience = byte32;
    ptr8 += 4;

    bool custom_secskill = false;
    // custom skill
    if(*ptr8)
    {
	custom_secskill = true;
	++ptr8;

	secondary_skills.clear();

	// skills
	for(u8 ii = 0; ii < 8; ++ii)
	{
	    const Skill::Secondary::skill_t skill = Skill::Secondary::FromMP2(*(ptr8 + ii));
	    const Skill::Level::type_t lvl = Skill::Level::FromMP2(*(ptr8 + ii + 8));

	    if(Skill::Secondary::UNKNOWN != skill && Skill::Level::NONE != lvl) secondary_skills.push_back(Skill::Secondary(skill, lvl));
	}
	ptr8 += 16;
    }
    else
    {
	++ptr8;

        ptr8 += 16;
    }
    
    // unknown
    ++ptr8;

    // custom name
    if(*ptr8) name = std::string(_(reinterpret_cast<const char *>(ptr8 + 1)));
    ptr8 += 14;

    // fixed race for custom portrait
    if(custom_portrait && Settings::Get().ExtForceSelectRaceFromType())
    {
	if(Race::ALL & rc) race = rc;

	// fixed default primary skills
	u8 book, spell;
	Skill::Primary::LoadDefaults(race, *this, book, spell);

	// fixed default troop
	if(!custom_troop)
	    army.Reset(true);

	// fixed default sec skills
	if(!custom_secskill)
	    Skill::Secondary::LoadDefaults(race, secondary_skills);

	// fixed default spell
	if(book)
	{
    	    SpellBookActivate();
    	    AppendSpellToBook(Spell::FromInt(spell));
    	    if(!HasArtifact(Artifact::MAGIC_BOOK)) PickupArtifact(Artifact::MAGIC_BOOK);
	}
    }

    // patrol
    if(*ptr8)
    {
	SetModes(PATROL);
	patrol_center = GetCenter();
    }
    ++ptr8;

    // count square
    patrol_square = *ptr8;
    ++ptr8;

    // end

    // save general object
    save_maps_object = MP2::OBJ_ZERO;

    // fix zero army
    if(!army.isValid()) army.Reset(true);

    // level up
    u8 level = GetLevel();
    while(1 < level--)
    {
	Skill::Primary::skill_t primary = LevelUpPrimarySkill();
	if(!custom_secskill) LevelUpSecondarySkill(primary, true);
    }

    // other param
    SetSpellPoints(GetMaxSpellPoints());
    move_point = GetMaxMovePoints();

    DEBUG(DBG_GAME , DBG_INFO, "Heroes::LoadFromMP2: " << name << ", color: " << Color::String(color) << ", race: " << Race::String(race));
}

Heroes::heroes_t Heroes::GetID(void) const
{
    return portrait;
}

u8 Heroes::GetMobilityIndexSprite(void) const
{
    // valid range (0 - 25)
    const u8 index = !CanMove() ? 0 : move_point / 100;
    return 25 >= index ? index : 25;
}

u8 Heroes::GetManaIndexSprite(void) const
{
    // valid range (0 - 25)
    u8 r = GetSpellPoints() / 5;
    return 25 >= r ? r : 25;
}

u8 Heroes::GetAttack(void) const
{
    return GetAttack(NULL);
}

u8 Heroes::GetAttack(std::string* strs) const
{
    s16 result = attack + GetAttackModificator(strs);

    return result < 0 ? 0 : (result > 255 ? 255 : result);
}

u8 Heroes::GetDefense(void) const
{
    return GetDefense(NULL);
}

u8 Heroes::GetDefense(std::string* strs) const
{
    s16 result = defense + GetDefenseModificator(strs);

    return result < 0 ? 0 : (result > 255 ? 255 : result);
}

u8 Heroes::GetPower(void) const
{
    return GetPower(NULL);
}

u8 Heroes::GetPower(std::string* strs) const
{
    s16 result = power + GetPowerModificator(strs);

    return result < 0 ? 0 : (result > 255 ? 255 : result);
}

u8 Heroes::GetKnowledge(void) const
{
    return GetKnowledge(NULL);
}

u8 Heroes::GetKnowledge(std::string* strs) const
{
    s16 result = knowledge + GetKnowledgeModificator(strs);

    return result < 0 ? 0 : (result > 255 ? 255 : result);
}

void Heroes::IncreasePrimarySkill(const Skill::Primary::skill_t skill)
{
    switch(skill)
    {
	case Skill::Primary::ATTACK:	++attack; break;
	case Skill::Primary::DEFENSE:	++defense; break;
	case Skill::Primary::POWER:	++power; break;
	case Skill::Primary::KNOWLEDGE:	++knowledge; break;
	default: break;
    }
}

u32 Heroes::GetExperience(void) const
{
    return experience;
}

void Heroes::IncreaseMovePoints(const u16 point)
{
    move_point += point;
}

u16 Heroes::GetMovePoints(void) const
{
    return move_point;
}

u16 Heroes::GetMaxSpellPoints(void) const
{
    return 10 * GetKnowledge();
}

u16 Heroes::GetMaxMovePoints(void) const
{
    u16 point = 0;

    // start point
    if(isShipMaster())
    {
	point = 1500;

	// skill navigation
	point += point * GetSecondaryValues(Skill::Secondary::NAVIGATION) / 100;

	// artifact bonus
        if(HasArtifact(Artifact::SAILORS_ASTROLABE_MOBILITY)) point += 1000;

        // visited object
        if(isVisited(MP2::OBJ_LIGHTHOUSE)) point += 500;
    }
    else
    {
    	switch(army.GetSlowestTroop().GetSpeed())
	{
	    default: break;
	    case Speed::CRAWLING:
	    case Speed::VERYSLOW:	point = 1000; break;
	    case Speed::SLOW:		point = 1100; break;
	    case Speed::AVERAGE:	point = 1200; break;
	    case Speed::FAST:		point = 1300; break;
	    case Speed::VERYFAST:	point = 1400; break;
	    case Speed::ULTRAFAST:
	    case Speed::BLAZING:
	    case Speed::INSTANT:	point = 1500; break;
	}

	// skill logistics
	point += point * GetSecondaryValues(Skill::Secondary::LOGISTICS) / 100;

	// artifact bonus
	if(HasArtifact(Artifact::NOMAD_BOOTS_MOBILITY)) point += 600;
	if(HasArtifact(Artifact::TRAVELER_BOOTS_MOBILITY)) point += 300;

        // visited object
        if(isVisited(MP2::OBJ_STABLES)) point += 500;
    }

    if(HasArtifact(Artifact::TRUE_COMPASS_MOBILITY)) point += 500;

    return point;
}

s8 Heroes::GetMorale(void) const
{
    return GetMoraleWithModificators(NULL);
}

s8 Heroes::GetMoraleWithModificators(std::string *strs) const
{
    s8 result = Morale::NORMAL;

    // bonus artifact
    result += GetMoraleModificator(isShipMaster(), strs);

    if(army.AllTroopsIsRace(Race::NECR)) return Morale::NORMAL;

    // bonus leadership
    result += Skill::GetLeadershipModifiers(GetLevelSkill(Skill::Secondary::LEADERSHIP), strs);

    // object visited
    ObjectVisitedModifiers modifiers;
    modifiers.reserve(7);

    modifiers.push_back(std::make_pair(MP2::OBJ_BUOY, 1));
    modifiers.push_back(std::make_pair(MP2::OBJ_OASIS, 1));
    modifiers.push_back(std::make_pair(MP2::OBJ_WATERINGHOLE, 1));
    modifiers.push_back(std::make_pair(MP2::OBJ_TEMPLE, 2));
    modifiers.push_back(std::make_pair(MP2::OBJ_GRAVEYARD, -1));
    modifiers.push_back(std::make_pair(MP2::OBJ_DERELICTSHIP, -1));
    modifiers.push_back(std::make_pair(MP2::OBJ_SHIPWRECK, -1));

    // global modificator
    result += GetResultModifiers(modifiers, *this, strs);

    // result
    if(result < Morale::AWFUL)	return Morale::TREASON;
    else
    if(result < Morale::POOR)	return Morale::AWFUL;
    else
    if(result < Morale::NORMAL)	return Morale::POOR;
    else
    if(result < Morale::GOOD)	return Morale::NORMAL;
    else
    if(result < Morale::GREAT)	return Morale::GOOD;
    else
    if(result < Morale::BLOOD)	return Morale::GREAT;

    return Morale::BLOOD;
}

s8 Heroes::GetLuck(void) const
{
    return GetLuckWithModificators(NULL);
}

s8 Heroes::GetLuckWithModificators(std::string *strs) const
{
    s8 result = Luck::NORMAL;

    // bonus artifact
    result += GetLuckModificator(isShipMaster(), strs);

    // bonus luck
    result += Skill::GetLuckModifiers(GetLevelSkill(Skill::Secondary::LUCK), strs);

    // object visited
    ObjectVisitedModifiers modifiers;
    modifiers.reserve(5);

    modifiers.push_back(std::make_pair(MP2::OBJ_MERMAID, 1));
    modifiers.push_back(std::make_pair(MP2::OBJ_FAERIERING, 1));
    modifiers.push_back(std::make_pair(MP2::OBJ_FOUNTAIN, 1));
    modifiers.push_back(std::make_pair(MP2::OBJ_IDOL, 1));
    modifiers.push_back(std::make_pair(MP2::OBJ_PYRAMID, -2));

    // global modificator
    result += GetResultModifiers(modifiers, *this, strs);

    if(result < Luck::AWFUL)	return Luck::CURSED;
    else
    if(result < Luck::BAD)	return Luck::AWFUL;
    else
    if(result < Luck::NORMAL)	return Luck::BAD;
    else
    if(result < Luck::GOOD)	return Luck::NORMAL;
    else
    if(result < Luck::GREAT)	return Luck::GOOD;
    else
    if(result < Luck::IRISH)	return Luck::GREAT;

    return Luck::IRISH;
}

/* recrut hero */
bool Heroes::Recruit(const Color::color_t cl, const Point & pt)
{
    if(color != Color::GRAY)
    {
	DEBUG(DBG_GAME , DBG_WARN, "Heroes::Recrut: hero not freeman!");
	return false;
    }

    Kingdom & kingdom = world.GetKingdom(cl);

    if(kingdom.AllowRecruitHero(false, 0))
    {
	Maps::Tiles & tiles = world.GetTiles(pt);
	color = cl;
	killer_color = Color::GRAY;
	SetCenter(pt);
	if(!Modes(SAVEPOINTS)) move_point = GetMaxMovePoints();
	MovePointsScaleFixed();

	if(!army.isValid()) army.Reset(false);

	// save general object
	save_maps_object = tiles.GetObject();
	tiles.SetObject(MP2::OBJ_HEROES);

	kingdom.AddHeroes(this);
	return true;
    }

    return false;
}

bool Heroes::Recruit(const Castle & castle)
{
    if(NULL == castle.GetHeroes() &&
	Recruit(castle.GetColor(), castle.GetCenter()))
    {
	if(castle.GetLevelMageGuild())
	{
	    // magic point
	    if(!Modes(SAVEPOINTS)) SetSpellPoints(GetMaxSpellPoints());
	    // learn spell
	    castle.GetMageGuild().EducateHero(*this);
	}
	return true;
    }

    return false;
}

void Heroes::ActionNewDay(void)
{
    // increase resource
    Resource::funds_t resource;

    // skip incomes for first day
    if(HasArtifact(Artifact::GOLDEN_GOOSE))           resource += ProfitConditions::FromArtifact(Artifact::GOLDEN_GOOSE);
    if(HasArtifact(Artifact::ENDLESS_SACK_GOLD))      resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_SACK_GOLD);
    if(HasArtifact(Artifact::ENDLESS_BAG_GOLD))       resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_BAG_GOLD);
    if(HasArtifact(Artifact::ENDLESS_PURSE_GOLD))     resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_PURSE_GOLD);
    if(HasArtifact(Artifact::ENDLESS_POUCH_SULFUR))   resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_POUCH_SULFUR);
    if(HasArtifact(Artifact::ENDLESS_VIAL_MERCURY))   resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_VIAL_MERCURY);
    if(HasArtifact(Artifact::ENDLESS_POUCH_GEMS))     resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_POUCH_GEMS);
    if(HasArtifact(Artifact::ENDLESS_CORD_WOOD))      resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_CORD_WOOD);
    if(HasArtifact(Artifact::ENDLESS_CART_ORE))       resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_CART_ORE);
    if(HasArtifact(Artifact::ENDLESS_POUCH_CRYSTAL))  resource += ProfitConditions::FromArtifact(Artifact::ENDLESS_POUCH_CRYSTAL);

    // estates skill bonus
    resource.gold += GetSecondaryValues(Skill::Secondary::ESTATES);

    // added
    if(resource.GetValidItems()) world.GetKingdom(GetColor()).AddFundsResource(resource);

    if(HasArtifact(Artifact::TAX_LIEN))
    {
	resource.Reset();
	resource.gold = 250;
	world.GetKingdom(GetColor()).OddFundsResource(resource);
    }

    // recovery move points
    move_point = GetMaxMovePoints();
    // stables visited?
    if(isVisited(MP2::OBJ_STABLES)) move_point += 400;

    // recovery spell points
    if(HaveSpellBook())
    {
	// possible visit arteian spring 2 * max
	const u16 prev = GetSpellPoints();
	const Castle* castle = inCastle();

	// in castle?
	if(castle && castle->GetLevelMageGuild())
	{
	    //restore from mage guild
	    if(prev < GetMaxSpellPoints()) SetSpellPoints(GetMaxSpellPoints());
	}
	else
	{
	    u16 curr = GetSpellPoints();

	    // everyday
	    curr += Game::GetHeroRestoreSpellPointsPerDay();

	    if(HasArtifact(Artifact::POWER_RING)) curr += 2;

	    // secondary skill
	    curr += GetSecondaryValues(Skill::Secondary::MYSTICISM);

	    if((curr > GetMaxSpellPoints()) && (curr > prev)) curr = prev;

	    SetSpellPoints(curr);
	}
    }

    // remove day visit object
    visit_object.remove_if(Visit::isDayLife);
    

    // new day, new capacities
    ResetModes(STUPID);
    ResetModes(SAVEPOINTS);
}

void Heroes::ActionNewWeek(void)
{
    // remove week visit object
    visit_object.remove_if(Visit::isWeekLife);
    
    // fix artesian spring effect
    if(GetSpellPoints() > GetMaxSpellPoints()) SetSpellPoints(GetMaxSpellPoints());
}

void Heroes::ActionNewMonth(void)
{
    // remove month visit object
    visit_object.remove_if(Visit::isMonthLife);
}

void Heroes::ActionAfterBattle(void)
{
    // remove month visit object
    visit_object.remove_if(Visit::isBattleLife);
    //
    SetModes(ACTION);
}

s32 Heroes::FindPath(s32 dst_index) const
{
    Route::Path route(*this);

    return route.Calculate(dst_index);
}

void Heroes::RescanPath(void)
{
    if(path.isValid())
    {
	if(Game::AI == GetControl())
	{
	    if(path.hasObstacle()) path.Reset();
	}
	else
	{
	    path.ScanObstacleAndReduce();
	}
    }
}

/* if hero in castle */
const Castle* Heroes::inCastle(void) const
{
    if(Color::GRAY == color) return false;

    const std::vector<Castle *> & castles = world.GetKingdom(color).GetCastles();
    
    std::vector<Castle *>::const_iterator it1 = castles.begin();
    std::vector<Castle *>::const_iterator it2 = castles.end();

    for(; it1 != it2; ++it1) if((**it1).GetCenter() == GetCenter()) return *it1;

    return NULL;
}

Castle* Heroes::inCastle(void)
{
    if(Color::GRAY == color) return false;

    const std::vector<Castle *> & castles = world.GetKingdom(color).GetCastles();
    
    std::vector<Castle *>::const_iterator it1 = castles.begin();
    std::vector<Castle *>::const_iterator it2 = castles.end();

    for(; it1 != it2; ++it1) if((**it1).GetCenter() == GetCenter()) return *it1;

    return NULL;
}

/* is visited cell */
bool Heroes::isVisited(const Maps::Tiles & tile, const Visit::type_t type) const
{
    const u16 & index = tile.GetIndex();
    const MP2::object_t object = (tile.GetObject() == MP2::OBJ_HEROES ? GetUnderObject() : tile.GetObject());

    if(Visit::GLOBAL == type) return world.GetKingdom(color).isVisited(index, object);

    return visit_object.end() != std::find(visit_object.begin(), visit_object.end(), IndexObject(index, object));
}

/* return true if object visited */
bool Heroes::isVisited(const u8 object, const Visit::type_t type) const
{
    if(Visit::GLOBAL == type) return world.GetKingdom(color).isVisited(object);

    return visit_object.end() != std::find_if(visit_object.begin(), visit_object.end(), std::bind2nd(std::mem_fun_ref(&IndexObject::isObject), object));
}

/* set visited cell */
void Heroes::SetVisited(const s32 index, const Visit::type_t type)
{
    const Maps::Tiles & tile = world.GetTiles(index);

    const MP2::object_t object = (tile.GetObject() == MP2::OBJ_HEROES ? GetUnderObject() : tile.GetObject());

    if(Visit::GLOBAL == type)
	world.GetKingdom(color).SetVisited(index, object);
    else
    if(isVisited(tile))
	return;
    else
    if(MP2::OBJ_ZERO != object) visit_object.push_front(IndexObject(index, object));
}

u8 Heroes::GetCountArtifacts(void) const
{
    return std::count_if(bag_artifacts.begin(), bag_artifacts.end(), std::mem_fun_ref(&Artifact::isValid));
}

bool Heroes::HasUltimateArtifact(void) const
{
    return bag_artifacts.end() != std::find_if(bag_artifacts.begin(), bag_artifacts.end(), std::mem_fun_ref(&Artifact::isUltimate));
}

bool Heroes::IsFullBagArtifacts(void) const
{
    return bag_artifacts.end() == std::find(bag_artifacts.begin(), bag_artifacts.end(), Artifact::UNKNOWN);
}

bool Heroes::PickupArtifact(const Artifact::artifact_t art)
{
    return PickupArtifact(Artifact(art));
}

bool Heroes::PickupArtifact(const Artifact & art)
{
    BagArtifacts::iterator it = std::find(bag_artifacts.begin(), bag_artifacts.end(), Artifact::UNKNOWN);

    if(bag_artifacts.end() == it)
    {
	if(Settings::Get().MyColor() == color)
	{
	    art == Artifact::MAGIC_BOOK ?
	    Dialog::Message("", _("You must purchase a spell book to use the mage guild, but you currently have no room for a spell book. Try giving one of your artifacts to another hero."), Font::BIG, Dialog::OK) :
	    Dialog::Message(art.GetName(), _("You have no room to carry another artifact!"), Font::BIG, Dialog::OK);
	}
	return false;
    }

    *it = art;

    // book insert first
    if(art == Artifact::MAGIC_BOOK &&
	it != bag_artifacts.begin()) std::swap(*it, bag_artifacts.front());

    // check: anduran garb
    if(HasArtifact(Artifact::BREASTPLATE_ANDURAN) &&
	HasArtifact(Artifact::HELMET_ANDURAN) &&
	HasArtifact(Artifact::SWORD_ANDURAN))
    {
	it = std::find(bag_artifacts.begin(), bag_artifacts.end(), Artifact::BREASTPLATE_ANDURAN);
	*it = Artifact::UNKNOWN;
	it = std::find(bag_artifacts.begin(), bag_artifacts.end(), Artifact::HELMET_ANDURAN);
	*it = Artifact::UNKNOWN;
	it = std::find(bag_artifacts.begin(), bag_artifacts.end(), Artifact::SWORD_ANDURAN);
	*it = Artifact::UNKNOWN;
	it = std::find(bag_artifacts.begin(), bag_artifacts.end(), Artifact::UNKNOWN);
	*it = Artifact::BATTLE_GARB;

	if(Settings::Get().MyColor() == color)
	    DialogWithArtifact("", _("The three Anduran artifacts magically combine into one."), Artifact::BATTLE_GARB);
    }

    return true;
}

/* return level hero */
u8 Heroes::GetLevel(void) const
{
    return GetLevelFromExperience(experience);
}

void Heroes::IncreaseExperience(const u32 exp)
{
    const u8 level_old = GetLevelFromExperience(experience);
    const u8 level_new = GetLevelFromExperience(experience + exp);

    for(u8 ii = 0; ii < level_new - level_old; ++ii) LevelUp();

    experience += exp;
}

/* calc level from exp */
u8 Heroes::GetLevelFromExperience(u32 exp)
{
    for(u8 lvl = 1; lvl < 255; ++ lvl) if(exp < GetExperienceFromLevel(lvl)) return lvl;

    return 0;
}

/* calc exp from level */
u32 Heroes::GetExperienceFromLevel(u8 lvl)
{
    switch(lvl)
    {
	case 0:		return 0;
	case 1:		return 1000;
	case 2:		return 2000;
	case 3:		return 3200;
	case 4:		return 4500;
	case 5:		return 6000;
	case 6:		return 7700;
	case 7:		return 9000;
	case 8: 	return 11000;
	case 9:		return 13200;
	case 10:	return 15500;
	case 11:	return 18500;
	case 12:	return 22100;
	case 13:	return 26400;
	case 14:	return 31600;
	case 15:	return 37800;
	case 16:	return 45300;
	case 17:	return 54200;
	case 18:	return 65000;
	case 19:	return 78000;
	case 20:	return 93600;
	case 21:	return 112300;
	case 22:	return 134700;
	case 23:	return 161600;
	case 24:	return 193900;
	case 25:	return 232700;
	case 26:	return 279300;
	case 27:	return 335200;
	case 28:	return 402300;
	case 29:	return 482800;
	case 30:	return 579400;
	case 31:	return 695300;
	case 32:	return 834400;
	case 33:	return 1001300;
	case 34:	return 1201600;
	case 35:	return 1442000;
	case 36:	return 1730500;
	case 37:	return 2076700;
	case 38:	return 2492100;
	case 39:	return 2990600;

	default:        break;
    }

    const u32 l1 = GetExperienceFromLevel(lvl - 1);
    return (l1 + static_cast<u32>(round((l1 - GetExperienceFromLevel(lvl - 2)) * 1.2 / 100) * 100));
}

/* buy book */
bool Heroes::BuySpellBook(const MageGuild* mageguild, u8 shrine)
{
    if(HaveSpellBook() || Color::GRAY == color) return false;

    const payment_t payment = PaymentConditions::BuySpellBook(shrine);
    Kingdom & kingdom = world.GetKingdom(color);

    std::string header = _("To cast spells, you must first buy a spell book for %{gold} gold.");
    String::Replace(header, "%{gold}", payment.gold);

    if( ! kingdom.AllowPayment(payment))
    {
	if(Settings::Get().MyColor() == color)
	{
	    header.append(". ");
	    header.append(_("Unfortunately, you seem to be a little short of cash at the moment."));
	    Dialog::Message("", header, Font::BIG, Dialog::OK);
	}
	return false;
    }

    if(Settings::Get().MyColor() == color)
    {
	const Sprite & border = AGG::GetICN(ICN::RESOURCE, 7);
	Surface sprite(border.w(), border.h());

	sprite.Blit(border);
	sprite.Blit(AGG::GetICN(ICN::ARTIFACT, Artifact::IndexSprite64(Artifact::MAGIC_BOOK)), 5, 5);

	header.append(". ");
	header.append(_("Do you wish to buy one?"));

	if(Dialog::NO == Dialog::SpriteInfo("", header, sprite, Dialog::YES | Dialog::NO)) return false;
    }

    if(!HasArtifact(Artifact::MAGIC_BOOK) && PickupArtifact(Artifact::MAGIC_BOOK))
    {
	kingdom.OddFundsResource(payment);
	SpellBookActivate();

	// add all spell to book
	if(mageguild) mageguild->EducateHero(*this);
#ifdef WITH_NET
	FH2LocalClient::SendHeroesBuyMagicBook(*this);
#endif
	return true;
    }

    return false;
}

/* return true is move enable */
bool Heroes::isEnableMove(void) const
{
    return Modes(ENABLEMOVE) && path.isValid() && path.GetFrontPenalty() <= move_point;
}

bool Heroes::CanMove(void) const
{
    return move_point >= Maps::Ground::GetPenalty(GetIndex(), Direction::CENTER, GetLevelSkill(Skill::Secondary::PATHFINDING));
}

/* set enable move */
void Heroes::SetMove(bool f)
{
    if(f)
	SetModes(ENABLEMOVE);
    else
    {
	ResetModes(ENABLEMOVE);

	// reset sprite position
	switch(direction)
        {
            case Direction::TOP:            sprite_index = 0; break;
            case Direction::BOTTOM:         sprite_index = 36; break;
            case Direction::TOP_RIGHT:
            case Direction::TOP_LEFT:       sprite_index = 9; break;
            case Direction::BOTTOM_RIGHT:
            case Direction::BOTTOM_LEFT:    sprite_index = 27; break;
            case Direction::RIGHT:
            case Direction::LEFT:           sprite_index = 18; break;
            default: break;
	}
    }
}

void Heroes::SaveUnderObject(MP2::object_t obj)
{
    save_maps_object = obj;
}

MP2::object_t Heroes::GetUnderObject(void) const
{
    return save_maps_object;
}

bool Heroes::isShipMaster(void) const
{
    return Modes(SHIPMASTER);
}

bool Heroes::CanPassToShipMaster(const Heroes & hero) const
{
    if(hero.isShipMaster())
    {
	Route::Path route(*this);

	const s16 & cx = hero.GetCenter().x;
	const s16 & cy = hero.GetCenter().y;
	const bool full = false;
	s32 coast = 0;

	for(s8 y = -1; y <= 1; ++y)
    	    for(s8 x = -1; x <= 1; ++x)
	{
            if((!y && !x) || (y && x && !full)) continue;

            coast = Maps::GetIndexFromAbsPoint(cx + x, cy + y);

            if(Maps::isValidAbsIndex(coast) &&
                MP2::OBJ_COAST == world.GetTiles(coast).GetObject() && FindPath(coast)) return true;
	}
    }
    return false;
}

void Heroes::SetShipMaster(bool f)
{
    f ? SetModes(SHIPMASTER) : ResetModes(SHIPMASTER);
}

const std::vector<Skill::Secondary> & Heroes::GetSecondarySkills(void) const
{
    return secondary_skills;
}

bool Heroes::HasSecondarySkill(const Skill::Secondary::skill_t skill) const
{
    return Skill::Level::NONE != GetLevelSkill(skill);
}

u8 Heroes::GetSecondaryValues(const Skill::Secondary::skill_t skill) const
{
    return Skill::Secondary::GetValues(skill, GetLevelSkill(skill));
}

bool Heroes::HasMaxSecondarySkill(void) const
{
    return HEROESMAXSKILL <= secondary_skills.size();
}

u8 Heroes::GetLevelSkill(const Skill::Secondary::skill_t skill) const
{
    std::vector<Skill::Secondary>::const_iterator it;

    it = std::find_if(secondary_skills.begin(), secondary_skills.end(), 
			std::bind2nd(std::mem_fun_ref(&Skill::Secondary::isSkill), skill));

    return it == secondary_skills.end() ? Skill::Level::NONE : (*it).Level();
}

void Heroes::LearnBasicSkill(const Skill::Secondary::skill_t skill)
{
    std::vector<Skill::Secondary>::iterator it;

    it = std::find_if(secondary_skills.begin(), secondary_skills.end(), 
			std::bind2nd(std::mem_fun_ref(&Skill::Secondary::isSkill), skill));

    if(it != secondary_skills.end())
	(*it).SetLevel(Skill::Level::BASIC);
    else
	secondary_skills.push_back(Skill::Secondary(skill, Skill::Level::BASIC));
}

void Heroes::LevelUpSkill(const Skill::Secondary::skill_t skill)
{
    std::vector<Skill::Secondary>::iterator it;

    it = std::find_if(secondary_skills.begin(), secondary_skills.end(), 
			std::bind2nd(std::mem_fun_ref(&Skill::Secondary::isSkill), skill));

    if(it != secondary_skills.end())
	(*it).NextLevel();
    else
	secondary_skills.push_back(Skill::Secondary(skill, Skill::Level::BASIC));

    if(skill == Skill::Secondary::SCOUTING)
	Scoute();
}

void Heroes::Scoute(void)
{
    Maps::ClearFog(GetIndex(), GetScoute(), color);
}

u8 Heroes::GetScoute(void) const
{
    return (HasArtifact(Artifact::TELESCOPE) ? Game::GetViewDistance(Game::VIEW_TELESCOPE) : 0) +
	Game::GetViewDistance(Game::VIEW_HEROES) + GetSecondaryValues(Skill::Secondary::SCOUTING);
}

u8 Heroes::GetVisionsDistance(void) const
{
    u8 dist = Spell::GetExtraValue(Spell::VISIONS);

    if(HasArtifact(Artifact::CRYSTAL_BALL))
        dist = Settings::Get().UseAltResource() ? dist * 2 + 2 : 8;

    return dist;
}

/* return route range in days */
u8 Heroes::GetRangeRouteDays(const s32 dst) const
{
    const u32 max = GetMaxMovePoints();
    const u16 limit = max * 5 / 100; // limit ~5 day

    // approximate distance, this restriction calculation
    if((4 * max / 100) < Maps::GetApproximateDistance(GetIndex(), dst))
    {
	DEBUG(DBG_GAME , DBG_INFO, "Heroes::GetRangeRouteDays: distance limit");
	return 0;
    }

    Route::Path test(*this);
    // approximate limit, this restriction path finding algorithm
    if(test.Calculate(dst, limit))
    {
	u32 total = test.TotalPenalty();
	if(move_point >= total) return 1;

	total -= move_point;
	if(max >= total) return 2;

	total -= move_point;
	if(max >= total) return 3;

	return 4;
    }
    else
    DEBUG(DBG_GAME , DBG_INFO, "Heroes::GetRangeRouteDays: iteration limit: " << limit);

    return 0;
}

/* select secondary skills for level up */
void Heroes::FindSkillsForLevelUp(Skill::Secondary & sec1, Skill::Secondary & sec2) const
{
    std::vector<Skill::Secondary::skill_t> exclude_skills;
    exclude_skills.reserve(MAXSECONDARYSKILL + HEROESMAXSKILL);

    // exclude for expert
    {
	std::vector<Skill::Secondary>::const_iterator it1 = secondary_skills.begin();
	std::vector<Skill::Secondary>::const_iterator it2 = secondary_skills.end();
	for(; it1 != it2; ++it1) if((*it1).Level() == Skill::Level::EXPERT) exclude_skills.push_back((*it1).Skill());
    }

    // exclude is full, add other.
    if(HEROESMAXSKILL <= secondary_skills.size())
    {
	std::vector<Skill::Secondary::skill_t> skills;
	Skill::Secondary::FillStandard(skills);

	std::vector<Skill::Secondary::skill_t>::const_iterator it1 = skills.begin();
	std::vector<Skill::Secondary::skill_t>::const_iterator it2 = skills.end();

	for(; it1 != it2; ++it1)
	    if(Skill::Level::NONE == GetLevelSkill(*it1)) exclude_skills.push_back(*it1);
    }

    sec1.SetSkill(Skill::Secondary::PriorityFromRace(GetRace(), exclude_skills));
    exclude_skills.push_back(sec1.Skill());
    sec2.SetSkill(Skill::Secondary::PriorityFromRace(GetRace(), exclude_skills));

    sec1.SetLevel(GetLevelSkill(sec1.Skill()));
    sec2.SetLevel(GetLevelSkill(sec2.Skill()));

    sec1.NextLevel();
    sec2.NextLevel();
}

/* up level */
void Heroes::LevelUp(bool autoselect)
{
    LevelUpSecondarySkill(LevelUpPrimarySkill(), autoselect);
}

Skill::Primary::skill_t Heroes::LevelUpPrimarySkill(void)
{
    const Skill::Primary::skill_t primary1 = Skill::Primary::FromLevelUp(race, GetLevel());

    // upgrade primary
    switch(primary1)
    {
	case Skill::Primary::ATTACK:	++attack; break;
	case Skill::Primary::DEFENSE:	++defense; break;
	case Skill::Primary::POWER:	++power; break;
	case Skill::Primary::KNOWLEDGE:	++knowledge; break;
	default: break;
    }
    DEBUG(DBG_GAME , DBG_INFO, "Heroes::LevelUpPrimarySkill: for " << GetName() << ", up " << Skill::Primary::String(primary1));

    return primary1;
}

void Heroes::LevelUpSecondarySkill(const Skill::Primary::skill_t primary1, bool autoselect)
{
    Skill::Secondary sec1;
    Skill::Secondary sec2;

    FindSkillsForLevelUp(sec1, sec2);
    DEBUG(DBG_GAME, DBG_INFO, "Heroes::LevelUpSecondarySkill: " << GetName() << " select " << Skill::Secondary::String(sec1.Skill()) << " or " << Skill::Secondary::String(sec2.Skill()));

    std::string header;
    std::string message;

    if(Skill::Secondary::UNKNOWN == sec1.Skill() && Skill::Secondary::UNKNOWN == sec2.Skill())
    {
	if(!autoselect && GetColor() == Settings::Get().MyColor())
	{
	    AGG::PlaySound(M82::NWHEROLV);
	    header = _("%{name} has gained a level.");
	    String::Replace(header, "%{name}", name);
	    message = _("%{skill} Skill +1");
	    String::Replace(message, "%{skill}", Skill::Primary::String(primary1));
	    Dialog::Message(header, message, Font::BIG, Dialog::OK);
	}
    }
    else
    if(Skill::Secondary::UNKNOWN == sec1.Skill() || Skill::Secondary::UNKNOWN == sec2.Skill())
    {
	Skill::Secondary* sec = Skill::Secondary::UNKNOWN == sec2.Skill() ? &sec1 : &sec2;

	if(!autoselect && GetColor() == Settings::Get().MyColor())
	{
	    AGG::PlaySound(M82::NWHEROLV);
	    header = _("%{name} has gained a level. %{skill} Skill +1");
	    String::Replace(header, "%{name}", name);
	    String::Replace(header, "%{skill}", Skill::Primary::String(primary1));
    	    message = _("You have learned %{level} %{skill}.");
	    String::Replace(message, "%{level}", Skill::Level::String(sec->Level()));
	    String::Replace(message, "%{skill}", Skill::Secondary::String(sec->Skill()));

	    const Sprite & sprite_frame = AGG::GetICN(ICN::SECSKILL, 15);
    	    Surface sf(sprite_frame.w(), sprite_frame.h());
    	    sf.Blit(sprite_frame);
	    // sprite
	    const Sprite & sprite_skill = AGG::GetICN(ICN::SECSKILL, Skill::Secondary::GetIndexSprite1(sec->Skill()));
	    sf.Blit(sprite_skill, 3, 3);
	    // text
	    const std::string &name_skill = Skill::Secondary::String(sec->Skill());
	    Text text_skill(name_skill, Font::SMALL);
	    text_skill.Blit(3 + (sprite_skill.w() - text_skill.w()) / 2, 6, sf);
	    const std::string &name_level = Skill::Level::String(sec->Level());
	    Text text_level(name_level, Font::SMALL);
	    text_level.Blit(3 + (sprite_skill.w() - text_level.w()) / 2, sprite_skill.h() - 12, sf);

	    Dialog::SpriteInfo(header, message, sf);
	}
	LevelUpSkill(sec->Skill());
    }
    else
    {
	Skill::Secondary::skill_t skill_select(Skill::Secondary::UNKNOWN);

	if(!autoselect && GetColor() == Settings::Get().MyColor())
	{
	    AGG::PlaySound(M82::NWHEROLV);
	    header = _("%{name} has gained a level. %{skill} Skill +1");
	    String::Replace(header, "%{name}", name);
	    String::Replace(header, "%{skill}", Skill::Primary::String(primary1));
    	    skill_select = Dialog::LevelUpSelectSkill(header, sec1, sec2);
	}
	// AI select
	else
	{
    	    skill_select = (Rand::Get(0, 1) ? sec1.Skill() : sec2.Skill());
	}

	LevelUpSkill(skill_select);
    }
}

/* apply penalty */
bool Heroes::ApplyPenaltyMovement(void)
{
    const u16 penalty = path.isValid() ?
	    path.GetFrontPenalty() :
	    Maps::Ground::GetPenalty(GetIndex(), Direction::CENTER, GetLevelSkill(Skill::Secondary::PATHFINDING));

    if(move_point >= penalty) move_point -= penalty;
    else return false;

    return true;
}

bool Heroes::MayStillMove(void) const
{
    if(Modes(STUPID) || isFreeman()) return false;
    return path.isValid() ? (move_point >= path.GetFrontPenalty()) : CanMove();
}

bool Heroes::isValid(void) const
{
    return true;
}

bool Heroes::isFreeman(void) const
{
    return Color::GRAY == color && !Modes(JAIL);
}

void Heroes::SetFreeman(const u8 reason)
{
    if(isFreeman()) return;

    bool savepoints = false;
    if((Battle2::RESULT_RETREAT | Battle2::RESULT_SURRENDER) & reason)
    {
	if(Settings::Get().ExtRememberPointsForHeroRetreating()) savepoints = true;
	world.GetKingdom(color).GetRecruits().SetHero2(this);
    }

    if(!army.isValid() || (Battle2::RESULT_RETREAT & reason)) army.Reset(false);
    else
    if((Battle2::RESULT_LOSS & reason) && !(Battle2::RESULT_SURRENDER & reason)) army.Reset(true);

    if(color != Color::GRAY) world.GetKingdom(color).RemoveHeroes(this);

    color = Color::GRAY;
    world.GetTiles(GetIndex()).SetObject(save_maps_object);
    modes = 0;
    SetIndex(-1);
    move_point_scale = -1;
    path.Reset();
    SetMove(false);
    SetModes(ACTION);
    if(savepoints) SetModes(SAVEPOINTS);
}

bool Heroes::isShow(u8 color)
{
    const s32 index_from = GetIndex();
    const Maps::Tiles & tile_from = world.GetTiles(index_from);

    if(path.isValid())
    {
        const s32 index_to = Maps::GetDirectionIndex(index_from, path.GetFrontDirection());
        const Maps::Tiles & tile_to = world.GetTiles(index_to);

        return !tile_from.isFog(color) && !tile_to.isFog(color);
    }

    return !tile_from.isFog(color);
}

const Surface & Heroes::GetPortrait30x22(void) const
{
    if(Heroes::SANDYSANDY > portrait) return AGG::GetICN(ICN::MINIPORT, portrait);
    else
    if(Heroes::SANDYSANDY == portrait) return AGG::GetICN(ICN::MINIPORT, BAX);

    return AGG::GetICN(ICN::MINIPORT, 0);
}

const Surface & Heroes::GetPortrait50x46(void) const
{
    if(Heroes::SANDYSANDY > portrait) return AGG::GetICN(ICN::PORTMEDI, portrait + 1);
    else
    if(Heroes::SANDYSANDY == portrait) return AGG::GetICN(ICN::PORTMEDI, BAX + 1);

    return AGG::GetICN(ICN::PORTMEDI, 0);
}

const Surface & Heroes::GetPortrait101x93(void) const
{
    ICN::icn_t icn = ICN::PORTxxxx(portrait);

    return AGG::GetICN(ICN::UNKNOWN != icn ? icn : ICN::PORT0000, 0);
}

void Heroes::SetKillerColor(Color::color_t c)
{
    killer_color = c;
}

Color::color_t Heroes::GetKillerColor(void) const
{
    return killer_color;
}

u8 Heroes::GetControl(void) const
{
    return world.GetKingdom(color).Control();
}

bool Heroes::AllowBattle(void) const
{
    switch(save_maps_object)
    {
	case MP2::OBJ_TEMPLE: return false;
	default: break;
    }

    return true;
}

void Heroes::PreBattleAction(void)
{
}

void Heroes::ActionNewPosition(void)
{
    const Settings & conf = Settings::Get();
    // check around monster
    const u16 dst_around = Maps::TileUnderProtection(GetIndex());

    if(dst_around)
    {
	SetMove(false);

	for(Direction::vector_t dir = Direction::TOP_LEFT; dir < Direction::CENTER && !isFreeman(); ++dir)
	    if(dst_around & dir)
	{
	    const s32 mons = Maps::GetDirectionIndex(GetIndex(), dir);

    	    // redraw gamearea for monster action sprite
	    if(conf.MyColor() == GetColor())
	    {
		Interface::Basic & I = Interface::Basic::Get();
		Game::Focus & F = Game::Focus::Get();
        	Cursor::Get().Hide();
		I.gameArea.Center(F.Center());
		F.SetRedraw();
		I.Redraw();
        	Cursor::Get().Show();
		// force flip, for monster attack show sprite
        	Display::Get().Flip();
	    }
	    Action(mons);
	    if(conf.ExtOnlyFirstMonsterAttack()) break;
	}
    }

    ResetModes(VISIONS);
}

const Point & Heroes::GetCenterPatrol(void) const
{
    return patrol_center;
}

u8 Heroes::GetSquarePatrol(void) const
{
    return patrol_square;
}

bool Heroes::CanScouteTile(s32 index) const
{
    if(Settings::Get().ExtScouteExtended())
    {
	const Maps::Tiles & tile = world.GetTiles(index);

	u8 dist = GetSecondaryValues(Skill::Secondary::SCOUTING) ? GetScoute() : 0;
	if(Modes(VISIONS) && dist < GetVisionsDistance()) dist = GetVisionsDistance();

	return (dist > Maps::GetApproximateDistance(GetIndex(), tile.GetIndex()));
    }
    return false;
}

void Heroes::MovePointsScaleFixed(void)
{
    move_point_scale = move_point * 1000 / GetMaxMovePoints();
}

void Heroes::RecalculateMovePoints(void)
{
    if(0 <= move_point_scale) move_point = GetMaxMovePoints() * move_point_scale / 1000;
}

void Heroes::Dump(void) const
{
    std::cout << "name            : " << name << std::endl;
    std::cout << "race            : " << Race::String(race) << std::endl;
    std::cout << "color           : " << Color::String(color) << std::endl;
    std::cout << "experience      : " << experience << std::endl;
    std::cout << "magic point     : " << GetSpellPoints() << std::endl;
    std::cout << "position x      : " << GetCenter().x << std::endl;
    std::cout << "position y      : " << GetCenter().y << std::endl;
    std::cout << "move point      : " << move_point << std::endl;
    std::cout << "max magic point : " << GetMaxSpellPoints() << std::endl;
    std::cout << "max move point  : " << GetMaxMovePoints() << std::endl;
    std::cout << "direction       : " << Direction::String(direction) << std::endl;
    std::cout << "index sprite    : " << static_cast<u16>(sprite_index) << std::endl;
    std::cout << "flags           : " << (Modes(SHIPMASTER) ? "SHIPMASTER," : ",") <<
                                         (Modes(SCOUTER) ? "SCOUTER," : ",") <<
                                         (Modes(HUNTER) ? "HUNTER," : ",") <<
                                         (Modes(PATROL) ? "PATROL," : ",") <<
                                         (Modes(STUPID) ? "STUPID," : ",") << std::endl;

    std::cout << "ai primary target: " << ai_primary_target << std::endl;
    std::cout << "ai sheduled visit: ";
    std::deque<s32>::const_iterator it1 = ai_sheduled_visit.begin();
    std::deque<s32>::const_iterator it2 = ai_sheduled_visit.end();
    for(; it1 != it2; ++it1) std::cout << *it1 << "(" << MP2::StringObject(world.GetTiles(*it1).GetObject()) << "), ";
    std::cout << std::endl;
}

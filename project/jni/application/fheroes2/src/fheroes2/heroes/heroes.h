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

#ifndef H2HEROES_H
#define H2HEROES_H

#include <string>
#include <vector>
#include <deque>
#include <list>
#include "spell.h"
#include "mp2.h"
#include "dialog.h"
#include "route.h"
#include "pairs.h"
#include "visit.h"
#include "direction.h"
#include "game_io.h"
#include "heroes_base.h"
#include "army.h"
#include "gamedefs.h"

class Recruits;
class Surface;
class MageGuild;

class Heroes : public HeroBase
{
public:
    enum heroes_t
    {
	// knight
	LORDKILBURN, SIRGALLANTH, ECTOR, GVENNETH, TYRO, AMBROSE, RUBY, MAXIMUS, DIMITRY,
	// barbarian
	THUNDAX, FINEOUS, JOJOSH, CRAGHACK, JEZEBEL, JACLYN, ERGON, TSABU, ATLAS,
	// sorceress
	ASTRA, NATASHA, TROYAN, VATAWNA, REBECCA, GEM, ARIEL, CARLAWN, LUNA,
	// warlock
	ARIE, ALAMAR, VESPER, CRODO, BAROK, KASTORE, AGAR, FALAGAR, WRATHMONT,
	// wizard
	MYRA, FLINT, DAWN, HALON, MYRINI, WILFREY, SARAKIN, KALINDRA, MANDIGAL,
	// necromancer
	ZOM, DARLANA, ZAM, RANLOO, CHARITY, RIALDO, ROXANA, SANDRO, CELIA,
	// from campain
	ROLAND, CORLAGON, ELIZA, ARCHIBALD, HALTON, BAX,
	// from extended
	SOLMYR, DAINWIN, MOG, UNCLEIVAN, JOSEPH, GALLAVANT, ELDERIAN, CEALLACH, DRAKONIA, MARTINE, JARKONAS,
	// debugger
	SANDYSANDY, UNKNOWN
    };

    static heroes_t ConvertID(u8);

    enum flags_t
    {
	SHIPMASTER	= 0x0001,
	//		= 0x0002,
	SPELLCASTED	= 0x0004,
	ENABLEMOVE	= 0x0008,
	HUNTER		= 0x0010,
	SCOUTER		= 0x0020,
	STUPID		= 0x0040,
	JAIL		= 0x0080,
	ACTION		= 0x0100,
	SAVEPOINTS	= 0x0200,
	VISIONS		= 0x4000,
	PATROL		= 0x8000
    };

    Heroes();
    Heroes(heroes_t ht, Race::race_t rc);

    bool isValid(void) const;
    bool isFreeman(void) const;
    void SetFreeman(const u8 reason);

    const Castle* inCastle(void) const;
    Castle* inCastle(void);

    void LoadFromMP2(s32 map_index, const void *ptr,  const Color::color_t cl, const Race::race_t rc);

    void ResetStupidFlag(void) { ResetModes(STUPID); }

    Color::color_t GetColor(void) const{ return color; }
    Race::race_t GetRace(void) const{ return race; }
    const std::string & GetName(void) const{ return name; }
    u8 GetType(void) const { return Skill::Primary::HEROES; }
    u8 GetControl(void) const;

    Color::color_t GetKillerColor(void) const;
    void SetKillerColor(Color::color_t);

    const Surface & GetPortrait30x22(void) const;
    const Surface & GetPortrait50x46(void) const;
    const Surface & GetPortrait101x93(void) const;

    const Army::army_t & GetArmy(void) const{ return army; }
    Army::army_t & GetArmy(void) { return army; }

    heroes_t GetID(void) const;

    u8 GetAttack(void) const;
    u8 GetDefense(void) const;
    u8 GetPower(void) const;
    u8 GetKnowledge(void) const;

    u8 GetAttack(std::string*) const;
    u8 GetDefense(std::string*) const;
    u8 GetPower(std::string*) const;
    u8 GetKnowledge(std::string*) const;

    void IncreasePrimarySkill(const Skill::Primary::skill_t skill);

    s8 GetMorale(void) const;
    s8 GetLuck(void) const;
    s8 GetMoraleWithModificators(std::string *str = NULL) const;
    s8 GetLuckWithModificators(std::string *str = NULL) const;
    u8 GetLevel(void) const;

    const Point & GetCenterPatrol(void) const;
    u8 GetSquarePatrol(void) const;

    u16 GetMaxSpellPoints(void) const;

    u16 GetMaxMovePoints(void) const;
    u16 GetMovePoints(void) const;
    void IncreaseMovePoints(const u16 point);
    bool MayStillMove(void) const;
    void ResetMovePoints(void) { move_point = 0; };
    void MovePointsScaleFixed(void);
    void RecalculateMovePoints(void);

    bool HasSecondarySkill(const Skill::Secondary::skill_t skill) const;
    bool HasMaxSecondarySkill(void) const;
    u8   GetLevelSkill(const Skill::Secondary::skill_t skill) const;
    void LearnBasicSkill(const Skill::Secondary::skill_t skill);
    void LevelUpSkill(const Skill::Secondary::skill_t skill);
    void FindSkillsForLevelUp(Skill::Secondary & sec1, Skill::Secondary & sec2) const;
    const std::vector<Skill::Secondary>	& GetSecondarySkills(void) const;
    u8   GetSecondaryValues(const Skill::Secondary::skill_t) const;

    bool PickupArtifact(const Artifact::artifact_t);
    bool PickupArtifact(const Artifact &);
    bool HasUltimateArtifact(void) const;
    u8 GetCountArtifacts(void) const;
    bool IsFullBagArtifacts(void) const;

    u8 GetMobilityIndexSprite(void) const;
    u8 GetManaIndexSprite(void) const;

    Dialog::answer_t OpenDialog(bool readonly = false, bool fade = false);
    void MeetingDialog(Heroes &);
    void AIMeeting(Heroes &);

    bool Recruit(const Color::color_t cl, const Point & pt);
    bool Recruit(const Castle & castle);

    void ActionNewDay(void);
    void ActionNewWeek(void);
    void ActionNewMonth(void);
    void ActionAfterBattle(void);

    bool BuySpellBook(const MageGuild*, u8 shrine = 0);

    const Route::Path & GetPath(void) const{ return path; }
    Route::Path & GetPath(void) { return path; }
    s32 FindPath(s32 dst_index) const;
    u8 GetRangeRouteDays(const s32 dst) const;
    void ShowPath(bool f){ f ? path.Show() : path.Hide(); }
    void RescanPath(void);

    Direction::vector_t GetDirection(void) const{ return direction; }

    void SetVisited(const s32 index, const Visit::type_t type = Visit::LOCAL);
    bool isVisited(const u8 object, const Visit::type_t type = Visit::LOCAL) const;
    bool isVisited(const Maps::Tiles & tile, const Visit::type_t type = Visit::LOCAL) const;


    bool Move(bool fast = false);
    bool isShow(u8 color);
    bool isEnableMove(void) const;
    bool CanMove(void) const;
    void SetMove(bool f);
    bool isAction(void) const { return Modes(ACTION); }
    void ResetAction(void) { ResetModes(ACTION); }
    void Action(const s32 dst_index);
    void ActionNewPosition(void);
    bool ApplyPenaltyMovement(void);
    bool ActionSpellCast(Spell::spell_t);

    void AIAction(const s32 dst_index);
    bool AIValidObject(s32);
    void AIRescueWhereMove(void);
    bool AIPriorityObject(s32);

    std::deque<s32> & GetAITasks(void) { return ai_sheduled_visit; }
    void ClearAITasks(void) { ai_sheduled_visit.clear(); }
    s32 GetPrimaryTarget(void) const { return ai_primary_target; }
    void SetPrimaryTarget(s32 target) { ai_primary_target = target; }

    void Redraw(Surface &, bool) const;
    void Redraw(Surface &, const s16, const s16, bool) const;
    u8   GetSpriteIndex(void) const{ return sprite_index; }
    void FadeOut(void) const;
    void FadeIn(void) const;
    void Scoute(void);
    u8   GetScoute(void) const;
    bool CanScouteTile(s32) const;
    u8   GetVisionsDistance(void) const;

    bool isShipMaster(void) const;
    bool CanPassToShipMaster(const Heroes &) const;
    void SetShipMaster(bool f);

    void SaveUnderObject(MP2::object_t obj);
    MP2::object_t GetUnderObject(void) const;

    u32 GetExperience(void) const;
    void IncreaseExperience(const u32 exp);

    bool AllowBattle(void) const;
    void PreBattleAction(void);

    void Dump(void) const;

    static u8 GetLevelFromExperience(u32 exp);
    static u32 GetExperienceFromLevel(u8 lvl);

    static void ScholarAction(Heroes &, Heroes &);

private:
    friend class Recruits;
    friend class Game::IO;

    void LevelUp(bool autoselect = false);
    Skill::Primary::skill_t LevelUpPrimarySkill(void);
    void LevelUpSecondarySkill(const Skill::Primary::skill_t, bool autoselect = false);
    void AngleStep(const Direction::vector_t to_direct);
    bool MoveStep(bool fast = false);

    std::string		name;
    Color::color_t	color;
    Color::color_t	killer_color;
    u32			experience;
    u16			move_point;
    s32			move_point_scale;

    std::vector<Skill::Secondary>	secondary_skills;

    Army::army_t        army;

    heroes_t		portrait;
    Race::race_t	race;

    MP2::object_t	save_maps_object;

    Route::Path		path;

    Direction::vector_t direction;
    u8			sprite_index;

    Point patrol_center;
    u8 patrol_square;

    std::list<IndexObject> visit_object;
    std::deque<s32> ai_sheduled_visit;
    s32 ai_primary_target;
};

#endif

/***************************************************************************
 *   Copyright (C) 2010 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#ifndef H2BATTLE2_ARENA_H
#define H2BATTLE2_ARENA_H

#include <list>
#include <map>

#include "battle2.h"
#include "gamedefs.h"

#define ARENAW 11
#define ARENAH 9
#define ARENASIZE ARENAW * ARENAH

namespace Army { class Troop; }
class Castle;
class HeroBase;

namespace Battle2
{
    class Arena;
    class Cell;
    class Stats;
    class Tower;
    class Catapult;
    class Bridge;
    class Interface;

    struct Actions : public std::list<Action>
    {
        void AddedRetreatAction(void);
        void AddedSurrenderAction(void);
        void AddedCastAction(u8, u16);
        void AddedCastTeleportAction(u16, u16);
        void AddedEndAction(const Stats &);
        void AddedSkipAction(const Stats &, bool);
        void AddedMoveAction(const Stats &, u16);
        void AddedMoveAction(const Stats &, const std::vector<u16> &);
        void AddedAttackAction(const Stats &, const Stats &);
        void AddedMoraleAction(const Stats &, u8);
    };

    struct Board : public std::vector<Cell>
    {
	Board();

	s16 GetIndexAbsPosition(const Point &) const;
	Rect GetArea(void) const;
	void SetEnemyQuality(const Stats &);
	void SetPositionQuality(const Stats &);
	void SetCobjObjects(s32);
	void SetCobjObject(u16, u16);
	void SetCovrObjects(u16);

	void GetAbroadPositions(u16, u8, std::vector<u16> &) const;
	void GetIndexesFromAbsPoints(std::vector<u16> &, const std::vector<Point> &) const;

	static bool inCastle(u16);
	static bool isMoatIndex(u16);
	static direction_t GetReflectDirection(u8);
	static direction_t GetDirection(u16, u16);
	static u16 GetDistance(u16, u16);
	static bool isValidDirection(u16, u8);
	static u16 GetIndexDirection(u16, u8);
    };

    class GraveyardTroop
    {
    public:
	GraveyardTroop(const Arena &);
	
	void GetClosedCells(std::vector<u16> &) const;
	u16 GetLastTroopIDFromCell(u16) const;
	void AddTroopID(u16);
	void RemoveTroopID(u16);

    private:
	std::map<u16, std::vector<u16> > map;
	
	const Arena & arena;
    };

    class Arena
    {
    public:
	Arena(Army::army_t &, Army::army_t &, s32, bool);
	~Arena();

	void Turns(u16, Result &);
	void RemoteTurn(const Stats &, Actions &);
	void HumanTurn(const Stats &, Actions &);

	const Cell* GetCell(u16, direction_t = CENTER) const;
	Cell* GetCell(u16, direction_t = CENTER);

	Stats* GetTroopBoard(u16);
	const Stats* GetTroopBoard(u16) const;

	Stats* GetTroopID(u16);
	const Stats* GetTroopID(u16) const;

	Stats* GetEnemyAbroadMaxQuality(u16, u8);
	const Stats* GetEnemyAbroadMaxQuality(u16, u8) const;

	Stats* GetEnemyAbroadMaxQuality(const Stats &);
	const Stats* GetEnemyAbroadMaxQuality(const Stats &) const;

	Stats* GetEnemyMaxQuality(u8);
	const Stats* GetEnemyMaxQuality(u8) const;

	Army::army_t* GetArmy(u8);
	const Army::army_t* GetArmy(u8) const;
	
	void GetArmyPositions(u8, std::vector<u16> &) const;
	u16 GetMaxQualityPosition(const std::vector<u16> &) const;
	u16 GetNearestTroops(u16, std::vector<u16> &, const std::vector<u16>* black = NULL) const;

	void DialogBattleSummary(const Result &) const;
	u8   DialogBattleHero(const HeroBase &) const;

	void FadeArena(void) const;

	const std::vector<u8> & GetUsageSpells(void) const;
	void AddSpell(u8);

	u16  GetPath(const Stats &, u16, std::vector<u16> &);
	void DumpBoard(void) const;

	Interface* GetInterface(void);
	Tower* GetTower(u8);

	void ApplyAction(Action &);

	void GetTargetsForDamage(Stats &, Stats &, std::vector<TargetInfo> &);
	void TargetsApplyDamage(Stats &, Stats &, std::vector<TargetInfo> &);

	void GetTargetsForSpells(const HeroBase*, const u8, const u16, std::vector<TargetInfo> &);
	void TargetsApplySpell(const HeroBase*, const u8, std::vector<TargetInfo> &);

	void UnpackBoard(Action &);
	void PackBoard(Action &) const;

        u8 GetCastleTargetValue(u8) const;
        void SetCastleTargetValue(u8, u8);

	bool isDisableCastSpell(u8 spell, std::string *msg) const;
	bool isAllowResurrectFromGraveyard(u8, u16) const;

	u8 GetOppositeColor(u8) const;

	void TowerAction(void);
	void CatapultAction(void);

	bool CanSurrenderOpponent(u8 color) const;
	bool CanRetreatOpponent(u8 color) const;

	void ResetBoard(void);
	void ScanPassabilityBoard(const Stats &, bool skip_speed = false);
	static u16 GetShortDistance(u16, const std::vector<u16> &);
	void GetPassableQualityPositions(const Stats &, std::vector<u16> &);
	u16 GetFreePositionNearHero(u8) const;

	// uniq spells
	void SpellActionSummonElemental(const HeroBase*, u8);
	void SpellActionTeleport(u16, u16);
	void SpellActionEarthQuake(void);
	void SpellActionMirrorImage(Stats &);

	// battle_ai
	void AITurn(const Stats &, Actions &);
	void AIMagicAction(const Stats &, Actions &, const Stats*);

	// battle_action
	void ApplyActionRetreat(Action &);
	void ApplyActionSurrender(Action &);
	void ApplyActionAttack(Action &);
	void ApplyActionMove(Action &);
	void ApplyActionEnd(Action &);
	void ApplyActionSkip(Action &);
	void ApplyActionMorale(Action &);
	void ApplyActionLuck(Action &);
	void ApplyActionSpellCast(Action &);
	void ApplyActionTower(Action &);
	void ApplyActionCatapult(Action &);
	void BattleProcess(Stats &, Stats &);

    protected:
	friend class Interface;
	friend class Cell;
	friend class Stats;
	friend class Tower;
	friend class Bridge;
	friend class Catapult;

	Army::army_t & army1;
        Army::army_t & army2;

	const Castle* castle;
	HeroBase* current_commander;

	Tower* towers[3];
	Catapult* catapult;
	Bridge* bridge;

	Interface* interface;
	Result *result_game;

	GraveyardTroop graveyard;
	std::vector<u8> usage_spells;

	Board board;
	ICN::icn_t icn_covr;
    };
}

#endif

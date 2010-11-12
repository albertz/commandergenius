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

#ifndef H2BATTLE2_STATS_H
#define H2BATTLE2_STATS_H

#include <vector>
#include <utility>
#include "army.h"
#include "bitmodes.h"
#include "battle_arena.h"

class Sprite;
class HeroBase;

namespace Battle2
{
    enum flags_t
    {
        TR_RESPONSED	= 0x00000001,
        TR_MOVED        = 0x00000002,
        TR_HARDSKIP     = 0x00000004,
        TR_SKIPMOVE     = 0x00000008,

        LUCK_GOOD       = 0x00000100,
        LUCK_BAD        = 0x00000200,
        MORALE_GOOD     = 0x00000400,
        MORALE_BAD      = 0x00000800,

	CAP_TOWER	= 0x00001000,
	CAP_SUMMONELEM	= 0x00002000,
	CAP_MIRROROWNER	= 0x00004000,
	CAP_MIRRORIMAGE	= 0x00008000,

	SP_BLOODLUST    = 0x00020000,
        SP_BLESS        = 0x00040000,
        SP_HASTE        = 0x00080000,
        SP_SHIELD       = 0x00100000,
        SP_STONESKIN    = 0x00200000,
	SP_DRAGONSLAYER = 0x00400000,
        SP_STEELSKIN    = 0x00800000,

        SP_ANTIMAGIC    = 0x01000000,

        SP_CURSE        = 0x02000000,
        SP_SLOW         = 0x04000000,
        SP_BERSERKER    = 0x08000000,
        SP_HYPNOTIZE    = 0x10000000,
        SP_BLIND        = 0x20000000,
        SP_PARALYZE     = 0x40000000,
        SP_STONE        = 0x80000000,

	IS_GOOD_MAGIC	= 0x00FE0000,
	IS_PARALYZE_MAGIC=0xC0000000,
	IS_MIND_MAGIC	= 0x78000000,
	IS_BAD_MAGIC	= 0xFE000000,
        IS_MAGIC        = 0xFFFE0000,

	IS_RED_STATUS	= IS_BAD_MAGIC,
	IS_GREEN_STATUS = SP_SHIELD | SP_STEELSKIN | SP_STONESKIN | SP_DRAGONSLAYER | SP_BLOODLUST | SP_BLESS | SP_HASTE | SP_ANTIMAGIC
    };

    u8 GetCellTargets(const Stats &, const Stats &, std::vector<u16> &);

    struct ModeDuration : public std::pair<u32, u16>
    {
	ModeDuration();
	ModeDuration(u32, u16);

	bool isMode(u32) const;
	bool isZeroDuration(void) const;
	void DecreaseDuration(void);
    };

    struct ModesAffected : public std::vector<ModeDuration>
    {
	ModesAffected();

	void AddMode(u32, u16);
	void RemoveMode(u32);
	void DecreaseDuration(void);

	u32  FindZeroDuration(void) const;
    };

    // battle troop stats
    struct Stats : public BitModes
    {
	Stats(Army::Troop &);
	~Stats();

	void	InitContours(void);
	void	SetPosition(u16);
	void	SetArena(Arena*);
	void	SetReflection(bool);
	void	SetMorale(s8);
	void	SetLuck(s8);
	void	NewTurn(void);

	bool	isArchers(void) const;
	bool	isFly(void) const;
	bool	isWide(void) const;
	bool	isValid(void) const;
	bool	isHandFighting(void) const;
	bool	isReflect(void) const;
	bool	AllowResponse(void) const;

	void	Dump(void) const;


	u16	GetID(void) const;

	u32	GetCount(void) const;
	s32	GetExtraQuality(s32) const;
	u16	GetTailIndex(void) const;
	u16	GetAttack(void) const;
	u16	GetDefense(void) const;
	u8	GetColor(void) const;
	u8	GetSpeed(bool skip_standing_check = false) const;
	const char* GetName(void) const;
	const char* GetPluralName(u32) const;
	u8	GetControl(void) const;
	u32	GetDamage(const Stats &) const;
	u32	GetDamageMin(void) const;
	u32	GetDamageMax(void) const;
	u16	GetScoreQuality(const Stats &) const;
	u32	GetHitPoints(void) const;
	u8	GetShots(void) const;
	u16	GetPosition(void) const;
	u32	ApplyDamage(Stats &, u32);
	u32	ApplyDamage(u32);
	bool	ApplySpell(u8, const HeroBase* hero, TargetInfo &);
	bool	AllowApplySpell(u8, const HeroBase* hero, std::string* msg = NULL) const;
	void	PostAttackAction(Stats &);
	void	ResetBlind(void);
	void	SpellModesAction(u8, u8, const HeroBase*);
	void	SpellApplyDamage(u8, u8, const HeroBase*, TargetInfo &);
	void	SpellRestoreAction(u8, u8, const HeroBase*);
	u32	Resurrect(u32, bool, bool);

	Monster GetMonster(void) const;
	const MonsterInfo & GetMonsterInfo(void) const;

	const animframe_t & GetFrameState(void) const;
	const animframe_t & GetFrameState(u8) const;
	void	IncreaseAnimFrame(bool loop = false);
	bool    isStartAnimFrame(void) const;
	bool    isFinishAnimFrame(void) const;
	u8      GetFrameOffset(void) const;
	u8      GetFrameStart(void) const;
	u8      GetFrameCount(void) const;

	s8 GetStartMissileOffset(u8) const;

	M82::m82_t M82Attk(void) const;
	M82::m82_t M82Kill(void) const;
	M82::m82_t M82Move(void) const;
	M82::m82_t M82Wnce(void) const;
	M82::m82_t M82Expl(void) const;

	ICN::icn_t ICNFile(void) const;
	ICN::icn_t ICNMiss(void) const;

	s32	     GetCellQuality(void) const;
	const Rect & GetCellPosition(void) const;

	u32	HowMuchWillKilled(u32) const;
	u16	AIGetAttackPosition(const std::vector<u16> &) const;

	void	SetResponse(void);
	void	ResetAnimFrame(u8);
	void	UpdateDirection(void);
	void	UpdateDirection(const Stats &);
	bool	UpdateDirection(const Cell &);
	void	PostKilledAction(void);

	bool	AppertainsArmy1(void) const;

	bool	isTwiceAttack(void) const;
	bool	isResurectLife(void) const;
	bool	isDoubleCellAttack(void) const;
	bool	isMultiCellAttack(void) const;
	bool	isHideAttack(void) const;
	bool	isAlwayResponse(void) const;
	bool 	isMagicResist(u8, u8) const;
	u8	GetMagicResist(u8, u8) const;
	bool	isMagicAttack(void) const;
	u8	GetSpellMagic(bool force = false) const;
	bool	isHaveDamage(void) const;
	u8	GetObstaclesPenalty(const Stats &) const;

	const Army::army_t* GetArmy(void) const;
	Army::army_t*       GetArmy(void);

	const HeroBase* GetCommander(void) const;
	HeroBase*       GetCommander(void);

	static bool isHandFighting(const Stats &, const Stats &);

	Army::Troop & troop;
	Arena* arena;

	u16	id;
	u16	position;
	u32	hp;
	u32	count;
	u32	dead;
	u8	shots;
	u8	disruptingray;
	bool	reflect;

	Stats*	mirror;

	u8	animstate;
	u8	animframe;
	s8	animstep;

	ModesAffected affected;
	Sprite* contours[4];
    };
}

#endif

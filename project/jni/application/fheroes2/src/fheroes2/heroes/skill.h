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

#ifndef H2SKILL_H
#define H2SKILL_H

#include <string>
#include <vector>
#include <utility>
#include "race.h"
#include "color.h"
#include "gamedefs.h"

void StringAppendModifiers(std::string &, s8);

namespace Skill
{
    s8 GetLeadershipModifiers(u8 level, std::string* strs);
    s8 GetLuckModifiers(u8 level, std::string* strs);

    void UpdateStats(const std::string &);

    namespace Level
    {
	enum type_t { NONE=0, BASIC=1, ADVANCED=2, EXPERT=3 };

	type_t FromMP2(const u8 byte);

	const char* String(u8 level);
    }

    class Secondary : private std::pair<u8, u8>
    {
	public:

	enum skill_t
	{
	    UNKNOWN	= 0,
	    PATHFINDING	= 1,
	    ARCHERY	= 2,
	    LOGISTICS	= 3,
	    SCOUTING	= 4,
	    DIPLOMACY	= 5,
	    NAVIGATION	= 6,
	    LEADERSHIP	= 7,
	    WISDOM	= 8,
	    MYSTICISM	= 9,
	    LUCK	= 10,
	    BALLISTICS	= 11,
	    EAGLEEYE	= 12,
	    NECROMANCY	= 13,
	    ESTATES	= 14
	};

	Secondary();
	Secondary(const skill_t s, const Level::type_t t);

	void		SetSkill(const skill_t s);
	void		SetLevel(const u8 level);
	void		NextLevel(void);

	Level::type_t	Level(void) const;
	skill_t		Skill(void) const;

	bool		isLevel(u8) const;
	bool		isSkill(u8) const;

	static skill_t 	Skill(const u8);
	static skill_t 	FromMP2(const u8 byte);
	static skill_t 	RandForWitchsHut(void);
	static const char* String(const skill_t skill);
	static const char* Description(const skill_t skill, const Level::type_t level);
	static skill_t PriorityFromRace(const u8 race, const std::vector<skill_t>& exclude);
	static void FillStandard(std::vector<skill_t> &);
	static u8 GetWeightSkillFromRace(u8 race, u8 skill);
	static void LoadDefaults(u8, std::vector<Secondary> &);
	static u16 GetValues(skill_t, u8);

	/* index sprite from SECSKILL */
	static u8 GetIndexSprite1(const skill_t skill);

	/* index sprite from MINISS */
	static u8 GetIndexSprite2(const skill_t skill);
    };
    
    class Primary
    {
	public:

	enum skill_t
	{
	    UNKNOWN	= 0,
	    ATTACK	= 1,
	    DEFENSE	= 2,
	    POWER	= 3,
	    KNOWLEDGE	= 4
	};

	enum type_t
	{
	    UNDEFINED,
	    MONSTER,
	    CAPTAIN,
	    HEROES
	};

	Primary();
	virtual ~Primary(){};

    	virtual u8 GetAttack(void) const = 0;
	virtual u8 GetDefense(void) const = 0;
        virtual u8 GetPower(void) const = 0;
        virtual u8 GetKnowledge(void) const = 0;
	virtual s8 GetMorale(void) const = 0;
	virtual s8 GetLuck(void) const = 0;
	virtual Race::race_t GetRace(void) const = 0;
	virtual u8 GetType(void) const = 0;

        static const char* String(const skill_t skill);
	static skill_t FromLevelUp(const u8 race, const u8 level);
	static void LoadDefaults(u8, Primary &, u8 &, u8 &);

	protected:
	u8			attack;
	u8			defense;
	u8			power;
	u8			knowledge;
    };

}

class SecondarySkillBar
{
public:
    SecondarySkillBar();

    const Rect & GetArea(void) const;
    u8 GetIndexFromCoord(const Point &);

    void SetSkills(const std::vector<Skill::Secondary> &);
    void SetUseMiniSprite(void);
    void SetPos(s16, s16);
    void SetInterval(u8);

    void Redraw(void);
    void QueueEventProcessing(void);

private:
    void CalcSize(void);

    Rect pos;
    const std::vector<Skill::Secondary> *skills;
    u8 interval;
    bool use_mini_sprite;
};

#endif

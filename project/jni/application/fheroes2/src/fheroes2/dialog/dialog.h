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
#ifndef H2DIALOG_H
#define H2DIALOG_H

#include <string>
#include <list>
#include <vector>
#include "gamedefs.h"
#include "monster.h"
#include "spell.h"
#include "skill.h"
#include "text.h"
#include "game.h"
#include "resource.h"

#define	SHADOWWIDTH	16
#define BOXAREA_WIDTH   245

class Castle;
class Heroes;
class Surface;
class SpriteCursor;

namespace Army
{
    class Troop;
    class BattleTroop;
}

namespace Maps
{
    class FileInfo;
    class Tiles;
}

namespace Dialog
{
    enum answer_t
    {
	ZERO	= 0x0000,
	YES     = 0x0001,
        OK      = 0x0002,
        NO      = 0x0004,
        CANCEL  = 0x0008,
        DISMISS = 0x0010,
        UPGRADE = 0x0020,
        MAX     = 0x0040,
	PREV	= 0x0080,
	NEXT	= 0x0100,

	WORLD	= 0x0200,
	PUZZLE	= 0x0400,
	INFO	= 0x0800,
	DIG	= 0x1000,

	READONLY= 0x2000,
	BATTLE	= 0x4000,
	BUTTONS = (YES|OK|NO|CANCEL)
    };

    answer_t AdventureOptions(const bool enabledig);
    Game::menu_t FileOptions(void);
    u8    SystemOptions(void);

    bool SelectFileLoad(std::string &);
    bool SelectFileSave(std::string &);

    // show info cell maps
    void QuickInfo(const Maps::Tiles & tile);
    void QuickInfo(const Castle & castle);
    void QuickInfo(const Heroes & heroes);
    
    // buttons: OK : CANCEL : OK|CANCEL : YES|NO
    u16 Message(const std::string &header, const std::string &message, Font::type_t ft, u16 buttons = 0);

    void ExtSettings(void);

    // other info
    Skill::Secondary::skill_t LevelUpSelectSkill(const std::string &header, const Skill::Secondary & sec1, const Skill::Secondary & sec2);
    bool SelectGoldOrExp(const std::string &header, const std::string &message, const u16 gold, const u16 expr);

    void SpellInfo(const Spell::spell_t, const bool ok_button = true);
    void SpellInfo(const std::string &, const std::string &, const Spell::spell_t, const bool ok_button = true);
    void SecondarySkillInfo(const Skill::Secondary::skill_t, const Skill::Level::type_t, const bool ok_button = true);
    void SecondarySkillInfo(const std::string &, const std::string &, const Skill::Secondary::skill_t, const Skill::Level::type_t, const bool ok_button = true);
    void PrimarySkillInfo(const std::string &, const std::string &, const Skill::Primary::skill_t);
    u16  SpriteInfo(const std::string &, const std::string &, const Surface &, u16 buttons = Dialog::OK);
    u16  ResourceInfo(const std::string &, const std::string &, const Resource::funds_t &, u16 buttons = Dialog::OK);

    Skill::Primary::skill_t SelectSkillFromArena(void);

    // redistribute count
    bool SelectCount(const std::string &header, u32 min, u32 max, u32 & res);
    bool InputString(const std::string &, std::string &);

    // recruit monster
    u16 RecruitMonster(const Monster & monster, u16 available);
    void DwellingInfo(const Monster & monster, u16 available);
    bool SetGuardian(Heroes &, Army::Troop &, bool readonly);

    // army info
    answer_t ArmyInfo(const Army::Troop & troop, u16 flags);
    // dialog marketplace
    void Marketplace(bool fromTradingPost = false);

    // dialog buy boat
    answer_t BuyBoat(bool enable);
    
    // puzzle
    void PuzzleMaps(void);

    // thieves guild
    void ThievesGuild(bool oracle);

    void GameInfo(void);

class Box : protected Background
{
public:
    Box(u16 height, bool buttons = false);
    ~Box();

    const Rect & GetArea(void){ return area; };

private:
    Rect area;
};

class FrameBorder : protected Background
{
public:
    FrameBorder();
    ~FrameBorder();

    bool isValid(void) const;

    void SetSize(u16, u16);
    void SetPosition(s16, s16, u16 = 0, u16 = 0);
    void Redraw(void);
    const Rect & GetRect(void) const;
    const Rect & GetArea(void) const;
    const Rect & GetTop(void) const;

private:
    Rect area;
    Rect top;
};

}

#endif

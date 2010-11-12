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
#ifndef H2CAPTAIN_H
#define H2CAPTAIN_H

#include "gamedefs.h"
#include "heroes_base.h"

class Castle;

class Captain : public HeroBase
{
  public:
    Captain(const Castle &);

    void LoadDefaults(void);
    bool isValid(void) const;
    u8 GetAttack(void) const;
    u8 GetDefense(void) const;
    u8 GetPower(void) const;
    u8 GetKnowledge(void) const;
    s8 GetMorale(void) const;
    s8 GetLuck(void) const;
    Race::race_t GetRace(void) const;
    Color::color_t GetColor(void) const;
    const std::string & GetName(void) const;
    u8 GetType(void) const;
    u8 GetControl(void) const;
    u16 GetIndex(void) const;

    const Castle* inCastle(void) const;

    u8 GetLevelSkill(const Skill::Secondary::skill_t) const;
    u8 GetSecondaryValues(const Skill::Secondary::skill_t skill) const;

    const Army::army_t & GetArmy(void) const;
    Army::army_t & GetArmy(void);

    u16 GetMaxSpellPoints(void) const;

    void PreBattleAction(void);

  private:
    const Castle & home;
};

#endif

/*************************************************************************** 
 *   Copyright (C) 2008 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *   Copyright (C) 2009 by Josh Matthews  <josh@joshmatthews.net>          *
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

#ifndef H2HEROESBASE_H
#define H2HEROESBASE_H

#include "bitmodes.h"
#include "skill.h"
#include "spell_book.h"
#include "artifact.h"
#include "color.h"
#include "position.h"

typedef std::vector<Artifact> BagArtifacts;

namespace Army { class army_t; }

class HeroBase : public Skill::Primary, public Maps::Position, public BitModes
{
public:
    HeroBase();

    virtual const std::string & GetName(void) const = 0;
    virtual Color::color_t GetColor(void) const = 0;
    virtual u8 GetControl(void) const = 0;
    virtual bool isValid(void) const = 0;

    virtual const Army::army_t & GetArmy(void) const = 0;
    virtual Army::army_t & GetArmy(void) = 0;

    virtual u16 GetMaxSpellPoints(void) const = 0;

    virtual u8 GetLevelSkill(const Skill::Secondary::skill_t) const = 0;
    virtual u8 GetSecondaryValues(const Skill::Secondary::skill_t) const = 0;

    virtual void PreBattleAction(void) = 0;

    virtual const Castle* inCastle(void) const = 0;

    s8 GetAttackModificator(std::string* = NULL) const;
    s8 GetDefenseModificator(std::string* = NULL) const;
    s8 GetPowerModificator(std::string* = NULL) const;
    s8 GetKnowledgeModificator(std::string* = NULL) const;
    s8 GetMoraleModificator(bool, std::string* = NULL) const;
    s8 GetLuckModificator(bool, std::string* = NULL) const;

    u16 GetSpellPoints(void) const;
    bool HaveSpellPoints(u16) const;
    void SetSpellPoints(u16);
    void TakeSpellPoints(u16);

    Spell::spell_t OpenSpellBook(SpellBook::filter_t, bool) const;
    bool HaveSpellBook(void) const;
    bool HaveSpell(Spell::spell_t) const;
    void SpellBookActivate(void);
    void AppendSpellToBook(Spell::spell_t, bool without_wisdom = false);

    BagArtifacts & GetBagArtifacts(void);
    const BagArtifacts & GetBagArtifacts(void) const;
    bool HasArtifact(Artifact::artifact_t) const;

protected:
    u16 magic_point;
    SpellBook spell_book;
    BagArtifacts bag_artifacts;
};

#endif

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

#include <algorithm>
#include "skill.h"
#include "spell_storage.h"

struct SpellEqualLevel : std::binary_function<Spell::spell_t, u8, bool>
{
    bool operator() (Spell::spell_t s, u8 l) const { return Spell::Level(s) == l; };
};

SpellStorage::SpellStorage()
{
    spells.reserve(67);
}

u8 SpellStorage::Size(u8 lvl) const
{
    switch(lvl)
    {
	case 1:	return std::count_if(spells.begin(), spells.end(), std::bind2nd(SpellEqualLevel(), 1));
	case 2:	return std::count_if(spells.begin(), spells.end(), std::bind2nd(SpellEqualLevel(), 2));
	case 3:	return std::count_if(spells.begin(), spells.end(), std::bind2nd(SpellEqualLevel(), 3));
	case 4:	return std::count_if(spells.begin(), spells.end(), std::bind2nd(SpellEqualLevel(), 4));
	case 5:	return std::count_if(spells.begin(), spells.end(), std::bind2nd(SpellEqualLevel(), 5));

	default: break;
    }

    return spells.size();
}

void SpellStorage::GetSpells(std::vector<Spell::spell_t> & v, u8 lvl) const
{
    std::vector<Spell::spell_t>::const_iterator it1 = spells.begin();
    std::vector<Spell::spell_t>::const_iterator it2 = spells.end();
    for(; it1 != it2; ++it1) if(lvl == Spell::Level(*it1)) v.push_back(*it1);
}

void SpellStorage::Append(const Spell::spell_t sp, u8 wisdom)
{
    if(spells.end() == std::find(spells.begin(), spells.end(), sp) && Spell::AllowWithWisdom(sp, wisdom))
	spells.push_back(sp);
}

void SpellStorage::Append(const Spell::spell_t sp)
{
    if(spells.end() == std::find(spells.begin(), spells.end(), sp))
	spells.push_back(sp);
}

bool SpellStorage::isPresentSpell(Spell::spell_t spell) const
{
    return spells.end() != std::find(spells.begin(), spells.end(), spell);
}

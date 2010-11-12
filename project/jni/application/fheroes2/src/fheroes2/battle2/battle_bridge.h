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

#ifndef H2BATTLE2_BRIDGE_H
#define H2BATTLE2_BRIDGE_H

#include "gamedefs.h"
#include "battle_arena.h"

namespace Battle2
{
    class Stats;

    class Bridge
    {
    public:
	Bridge(Arena & a);

	void SetDestroy(void);
	void SetDown(bool);
	void SetPassable(const Stats &);

	bool AllowUp(void);
	bool NeedDown(const Stats &, u16);
	bool isPassable(u8) const;
	bool isValid(void) const;
	bool isDestroy(void) const;
	bool isDown(void) const;

	static bool isIndex(u16);

    private:
	Arena & arena;
	bool destroy;
	bool down;
    };
}

#endif

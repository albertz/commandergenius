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

#ifndef H2BATTLE2_CELL_H
#define H2BATTLE2_CELL_H

#include "gamedefs.h"
#include "battle_arena.h"

namespace Army { class Troop; }

namespace Battle2
{
    class Cell
    {
    public:
	Cell(u16, const Rect*, Arena &);
	void Reset(void);
	void ResetDirection(void);
	void SetEnemyQuality(const Stats &);
	void SetPositionQuality(const Stats &);
	bool isPassable(const Stats &, const Cell &) const;
	bool isPassable(const Stats &, bool check_reflect) const;
	bool isPassable(void) const;
	bool isPositionIncludePoint(const Point &) const;
	u16 GetIndex(void) const;
	u8 GetDirection(void) const;
	const Rect & GetPos(void) const;

	void SetPassabilityAbroad(const Stats & b, std::vector<u16> &);

	u16 index;
	Rect pos;
	u8 object;
	u8 direction;
	s32 quality;
	Arena* arena;
    };
}

#endif

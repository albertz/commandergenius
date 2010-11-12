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

#ifndef H2BATTLE2_CATAPULT_H
#define H2BATTLE2_CATAPULT_H

#include "gamedefs.h"

class HeroBase;

namespace Battle2
{
    class Catapult
    {
    public:
	Catapult(const HeroBase &, bool, Arena &);

	Point GetTargetPosition(u8) const;
	u8 GetShots(void) const;
	u8 GetTarget(const std::vector<u8> &) const;
	u8 GetDamage(u8);
	void Action(void);

    private:
	Arena & arena;
	u8 cat_shots;
	u8 cat_first;
	bool cat_miss;
	bool cat_fort;
    };
}

#endif

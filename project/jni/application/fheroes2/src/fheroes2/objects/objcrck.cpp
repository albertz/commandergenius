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

#include "icn.h"
#include "objcrck.h"

bool ObjWasteLand::isPassable(const u16 icn, const u8 index)
{
    switch(icn)
    {
	case ICN::OBJNCRCK:
	    // artesian spring
	    if(2 < index && index < 5) return false;
	    else
	    // rock
	    if((9 < index && index < 12) || 18 == index || (20 < index && index < 23) ||
		(23 < index && index < 26) || (28 < index && index < 33) || (33 < index && index < 36) ||
		(36 < index && index < 39) || (39 < index && index < 44) || 46 == index ||
		49 == index || 52 == index || 55 == index) return false;
	    else
	    // cactus
	    if(14 == index || 16 == index) return false;
	    else
	    // skull
	    if(17 == index) return false;
	    else
	    // crater
	    if((57 < index && index < 60) || (61 < index && index < 66) || (67 < index && index < 70) ||
		(220 < index && index < 226) || (226 < index && index < 236)) return false;
	    else
	    // shrub
	    if(70 < index && index < 73) return false;
	    else
	    // wagon
	    if(74 == index) return false;
	    else
	    // tar pit
	    if(80 == index || 91 == index ||
		102 == index || 113 == index || 124 == index ||
		(134 < index && index < 138) || 148 == index || 159 == index ||
		170 == index || 181 == index) return false;
	    else
	    // troll bridge
	    if(181 < index && index < 190) return false;
	    else
	    // market
	    if(202 == index || 213 == index) return false;
	    else
	    // watering hole
	    if(216 < index && index < 221) return false;
	    else
	    // obelisk
	    if(238 == index) return false;
	    else
	    // saw mill
	    if(240 < index && index < 246) return false;

	    else return true;

	default: break;
    }

    return false;
}

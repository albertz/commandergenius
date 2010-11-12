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
#include "mp2.h"
#include "maps_tiles.h"
#include "objxloc.h"

bool ObjLoyalty::isPassable(const u16 icn, const u8 index)
{
    switch(icn)
    {
	case ICN::X_LOC1:
	    // alchemist tower
	    if(3 == index) return false;
	    else
	    // arena
	    if(31 == index || 40 == index || (48 < index && index < 51) ||
		(68 < index && index < 72)) return false;
	    else
	    // barrow mounds
	    if(74 < index && index < 78) return false;
	    else
	    // eath altar
	    if(85 == index || 94 == index || 103 == index) return false;
	    else
	    // air altar
	    if(116 < index && index < 120) return false;
	    else
	    // air altar
	    if(125 < index && index < 129) return false;
	    else
	    // water altar
	    if(133 < index && index < 137) return false;
	    else return true;

	case ICN::X_LOC2:
	    // stables
	    if(2 < index && index < 5) return false;
	    else
	    // jail
	    if(9 == index) return false;
	    else
	    // mermaid
	    if(28 == index || 37 == index || 46 == index) return false;
	    else
	    // sirens
	    if(92 == index || (100 < index && index < 103)) return false;
	    else
	    // reefs
	    if(110 < index && index < 136) return false;
	    else return true;

	case ICN::X_LOC3:
	    // hut magi
	    if(30 == index) return false;
	    else
	    // eyes magi
	    if(50 == index) return false;
	    else
	    // barrier
	    if(60 == index || 66 == index || 72 == index ||
		78 == index || 84 == index || 90 == index ||
		96 == index || 102 == index) return false;
	    else
	    // traveller tent
	    if(110 == index || 114 == index || 118 == index ||
		122 == index || 126 == index || 130 == index ||
		134 == index || 138 == index) return false;
	    else return true;

	default: break;
    }

    return false;
}

u8 ObjLoyalty::LearnObject(const Maps::TilesAddon & addon)
{
    switch(MP2::GetICNObject(addon.object))
    {
	case ICN::X_LOC1:
	    if(addon.index == 3) return MP2::OBJ_ALCHEMYTOWER;
	    else
	    if(addon.index < 3) return MP2::OBJN_ALCHEMYTOWER;
	    else
	    if(70 == addon.index) return MP2::OBJ_ARENA;
	    else
	    if(3 < addon.index && addon.index < 72) return MP2::OBJN_ARENA;
	    else
	    if(77 == addon.index) return MP2::OBJ_BARROWMOUNDS;
	    else
	    if(71 < addon.index && addon.index < 78) return MP2::OBJN_BARROWMOUNDS;
	    else
	    if(94 == addon.index) return MP2::OBJ_EARTHALTAR;
	    else
	    if(77 < addon.index && addon.index < 112) return MP2::OBJN_EARTHALTAR;
	    else
	    if(118 == addon.index) return MP2::OBJ_AIRALTAR;
	    else
	    if(111 < addon.index && addon.index < 120) return MP2::OBJN_AIRALTAR;
	    else
	    if(127 == addon.index) return MP2::OBJ_FIREALTAR;
	    else
	    if(119 < addon.index && addon.index < 129) return MP2::OBJN_FIREALTAR;
	    else
	    if(135 == addon.index) return MP2::OBJ_WATERALTAR;
	    else
	    if(128 < addon.index && addon.index < 137) return MP2::OBJN_WATERALTAR;
	    break;

	case ICN::X_LOC2:
	    if(addon.index == 4) return MP2::OBJ_STABLES;
	    else
	    if(addon.index < 4) return MP2::OBJN_STABLES;
	    else
	    if(addon.index == 9) return MP2::OBJ_JAIL;
	    else
	    if(4 < addon.index && addon.index < 10) return MP2::OBJN_JAIL;
	    else
	    if(addon.index == 37) return MP2::OBJ_MERMAID;
	    else
	    if(9 < addon.index && addon.index < 47) return MP2::OBJN_MERMAID;
	    else
	    if(addon.index == 101) return MP2::OBJ_SIRENS;
	    else
	    if(46 < addon.index && addon.index < 111) return MP2::OBJN_SIRENS;
	    else
	    if(110 < addon.index && addon.index < 136) return MP2::OBJ_REEFS;
	    break;

	case ICN::X_LOC3:
	    if(addon.index == 30) return MP2::OBJ_HUTMAGI;
	    else
	    if(addon.index < 32) return MP2::OBJN_HUTMAGI;
	    else
	    if(addon.index == 50) return MP2::OBJ_EYEMAGI;
	    else
	    if(31 < addon.index && addon.index < 59) return MP2::OBJN_EYEMAGI;
	    // fix
	    break;

	default: break;
    }

    return MP2::OBJ_ZERO;
}

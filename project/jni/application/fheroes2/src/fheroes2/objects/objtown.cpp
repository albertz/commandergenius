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
#include "maps.h"
#include "world.h"
#include "maps_tiles.h"
#include "game_focus.h"
#include "direction.h"
#include "objtown.h"

bool ObjTown::isPassable(const u16 icn, const u8 index, const s32 maps_index)
{
    switch(icn)
    {
	case ICN::OBJNTWBA:
	    if(4 < index && index < 10 && 7 != index)
	    {
		if(!Maps::isValidDirection(maps_index, Direction::BOTTOM)) return false;
		const Maps::Tiles & tiles = world.GetTiles(Maps::GetDirectionIndex(maps_index, Direction::BOTTOM));
		Game::Focus & focus = Game::Focus::Get();
		return tiles.isPassable((Game::Focus::HEROES == focus.Type() ? &focus.GetHeroes() : NULL), false);
	    }

            if(index < 5 || ( 9 < index && index < 15) || (19 < index && index < 25) ||
                 (29 < index && index < 35) || (39 < index && index < 45) || (49 < index && index < 55) ||
                 (59 < index && index < 65) || (69 < index && index < 75) || 79 < index) return false;
	    else return true;
	    break;

	case ICN::OBJNTOWN:
	    // kngt
	    if((5 < index && index < 13) || (13 < index && index < 16) ||
		(21 < index && index < 29) || (29 < index && index < 32)) return false;
	    else
	    // barb
	    if((37 < index && index < 45) || (45 < index && index < 48) ||
		(53 < index && index < 61) || (61 < index && index < 64)) return false;
	    else
	    // sorc
	    if((69 < index && index < 77) || (77 < index && index < 80) ||
		(85 < index && index < 93) || (93 < index && index < 96)) return false;
	    else
	    // wrlk
	    if((101 < index && index < 109) || (109 < index && index < 112) ||
		(117 < index && index < 125) || (125 < index && index < 128)) return false;
	    else
	    // wzrd
	    if((133 < index && index < 141) || (141 < index && index < 144) ||
		(149 < index && index < 157) || (157 < index && index < 160)) return false;
	    else
	    // necr
	    if((165 < index && index < 173) || (173 < index && index < 176) ||
		(181 < index && index < 189) || (189 < index && index < 192)) return false;

	    else return true;

	default: break;
    }

    return false;
}

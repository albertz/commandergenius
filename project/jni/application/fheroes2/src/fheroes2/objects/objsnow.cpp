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
#include "objsnow.h"

bool ObjSnow::isPassable(const u16 icn, const u8 index)
{
    switch(icn)
    {
	case ICN::OBJNSNOW:
	    // cave for centaur
	    if((1 < index && index < 4)) return false;
	    else
	    // camp fire
	    if(4 == index) return false;
	    else
	    // rock
	    if(22 == index || (25 < index && index < 29) || 30 == index ||
	    32 == index || (33 < index && index < 36) || (36 < index && index < 40)) return false;
	    else
	    // stub
	    if(40 < index && index < 43) return false;
	    else
	    // trees
	    if((48 < index && index < 51) || (54 < index && index < 58) || 60 == index || 
	    (63 < index && index < 66) || 68 == index || 71 == index || 
	    74 == index || 77 == index || 80 == index) return false;
	    else
	    // lake
	    if((80 < index && index < 93) || (93 < index && index < 96)) return false;
	    else
	    //wind mill
	    if(128 == index || 132 == index) return false;
	    else
	    // watch tower
	    if(138 == index) return false;
	    else
	    // obelisk
	    if(141 == index) return false;
	    else
	    // sign
	    if(143 == index) return false;
	    else
	    // alchemy tower
	    if(148 < index && index < 152) return false;
	    else
	    // graveyard
	    if((158 < index && index < 161) || (207 < index && index < 211)) return false;
	    else
	    // water mill
	    if(177 == index || 184 == index || 191 == index) return false;
	    else
	    // well
	    if(194 == index) return false;
	    else
	    // saw mill
	    if((196 < index && index < 202)) return false;

	    else return true;	    

	default: break;
    }

    return false;
}

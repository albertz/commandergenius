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
#include "objmult.h"

bool ObjMulti::isPassable(const u16 icn, const u8 index)
{
    switch(icn)
    {
	case ICN::OBJNMUL2:
	    // fountain
	    if(15 == index) return false;
	    else
	    // stub
	    if(16 == index || (17 < index && index < 20)) return false;
	    else
	    // alchemy tower
	    if(24 < index && index < 28) return false;
	    else
	    // dragon city
	    if(46 == index || (50 < index && index < 56)) return false;
	    else
	    // grave yard
	    if(56 < index && index < 59) return false;
	    else
	    // light house
	    if(73 == index) return false;
	    else
	    // saw mill
	    if(75 < index && index < 82) return false;
	    else
	    // water well
	    if(98 == index || 105 == index || 112 == index) return false;
	    else
	    // sign
	    if(114 == index) return false;
	    else
	    // teleport
	    if(116 == index || 119 == index || 122 == index) return false;
	    else
	    // wagon camp
	    if((123 < index && index < 127) || (127 < index && index < 130) || 136 == index) return false;
	    else
	    // well
	    if(162 == index || 165 == index) return false;
	    else
	    // freeman foundry
	    if(186 < index && index < 189) return false;
	    else
	    // magick garden
	    if(190 == index) return false;
	    else
	    // observation tower
	    if(201 == index) return false;
	    else
	    // grave yard
	    if(206 < index && index < 210) return false;
	    else
	    // saw mill
	    if(211 < index && index < 217) return false;

	    else return true;

        case ICN::OBJNMULT:
    	    // dead tree
            if(2 == index || 4 == index) return false;
	    else
	    // peasant hut
	    if(35 == index) return false;
	    else
	    // fort
	    if(57 < index && index < 60) return false;
	    else
	    // gasebo
	    if(62 == index) return false;
	    else
	    // shrub
	    if(62 < index && index < 66) return false;
	    else
	    // witch hut
	    if(69 == index) return false;
	    else
	    // mercenary camp
	    if(69 < index && index < 73) return false;
	    else
	    // ruins
	    if(72 < index && index < 75) return false;
	    else
	    // shrine
	    if(76 == index || 78 == index || 80 == index) return false;
	    else
	    // idol
	    if(82 == index) return false;
	    else
	    // standing stones
	    if(83 < index && index < 86) return false;
	    else
	    // temple
	    if(87 < index && index < 90) return false;
	    else
	    // market
	    if(104 == index || 111 == index) return false;
	    else
	    // tree house
	    if(114 == index) return false;
	    else
	    // watch tower
	    if(116 == index) return false;
	    else
	    // tree knowledge
	    if(123 == index) return false;
	    else
	    // camp fire
	    if(131 == index) return false;

	    else  return true;

	default: break;
    }

    return false;
}

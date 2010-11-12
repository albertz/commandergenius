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
#include "objswmp.h"

bool ObjSwamp::isPassable(const u16 icn, const u8 index)
{
    switch(icn)
    {
	case ICN::OBJNSWMP:
	    // witch hut
	    if(22 == index) return false;
	    else
	    // shrub
	    if((31 < index && index < 34)) return false;
	    else
	    // xanadu
	    if(67 == index || 74 == index || (80 < index && index < 83)) return false;
	    else
	    // faerie ring
	    if((83 < index && index < 86)) return false;
	    else
	    // lake
	    if((87 < index && index < 92) || (93 < index && index < 99) || (100 < index && index < 106)) return false;
	    else
	    if((107 < index && index < 111) || (111 < index && index < 114)) return false;
	    else
	    if((114 < index && index < 117) || (117 < index && index < 120) || (121 < index && index < 124)) return false;
	    else
	    // mandrake
	    if(126 == index || (127 < index && index < 130) || 131 == index || (132 < index && index < 136) || 137 == index) return false;
	    else
	    // swamp
	    if((137 < index && index < 140) || (142 < index && index < 161)) return false;
	    else
	    // sign
	    if(140 == index) return false;
	    else
	    // trees
	    if((165 < index && index < 168) || (170 < index && index < 173) || (175 < index && index < 178)) return false;
	    else
	    // swamp
	    if((178 < index && index < 193) || (211 < index && index < 214)) return false;
	    else
	    // shrub
	    if((192 < index && index < 195) || 196 == index || (197 < index && index < 202)) return false;
	    else
	    // rock
	    if(203 == index || 205 == index || (207 < index && index < 211)) return false;
	    else
	    // obelisk
	    if(216 == index) return false;
	    else return true;

	default: break;
    }

    return false;
}

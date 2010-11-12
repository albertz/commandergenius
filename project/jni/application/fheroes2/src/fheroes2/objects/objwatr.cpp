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
#include "objwatr.h"

bool ObjWater::isPassable(const u16 icn, const u8 index)
{
    switch(icn)
    {
	case ICN::OBJNWAT2:
	    // rock
	    if(2 == index) return false;
	    else
	    // ship
	    if(11 == index || 12 == index || (18 < index && index < 23)) return false;
	    
	    else return true;

	case ICN::OBJNWATR:
	    // buttle
	    if(0 == index) return false;
	    else
	    // chest
	    if(19 == index) return false;
	    else
	    // flotsam
	    if(45 == index) return false;
	    else
	    // magellan maps
	    if(62 == index || 69 == index) return false;
	    else
	    // surviror
	    if(111 == index) return false;
	    else
	    // rock
	    if((181 < index && index < 184) || (184 < index && index < 188)) return false;
	    else
	    // buoy
	    if(195 == index) return false;
	    else
	    // whirlpoll
	    if(202 == index || 206 == index || 210 == index || 214 == index || 218 == index || 222 == index) return false;
	    else
	    // ship
	    if(241 == index || 248 == index) return false;

	    else return true;

	default: break;
    }

    return false;
}

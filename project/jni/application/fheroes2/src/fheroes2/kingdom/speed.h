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
#ifndef H2SPEED_H
#define H2SPEED_H

#include <string>
#include "gamedefs.h"

namespace Speed
{
    enum speed_t
    {
	STANDING	= 0,
	CRAWLING	= 1,
        VERYSLOW	= 2,
        SLOW		= 3,
        AVERAGE		= 4,
        FAST		= 5,
        VERYFAST	= 6,
        ULTRAFAST	= 7,
        BLAZING		= 8,
        INSTANT		= 9
    };

    const char* String(u8 speed);
    
    speed_t GetOriginalSlow(u8);
    speed_t GetOriginalFast(u8);

    inline speed_t& operator++ (speed_t& speed){ return speed = ( INSTANT == speed ? INSTANT : speed_t(speed + 1)); }
    inline speed_t& operator-- (speed_t& speed){ return speed = ( CRAWLING == speed ? CRAWLING : speed_t(speed - 1)); }
}

#endif

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
#ifndef H2COLOR_H
#define H2COLOR_H

#include <string>
#include "gamedefs.h"

namespace Barrier
{
    enum color_t
    {
	NONE	= 0x00,
        AQUA	= 0x01,
        BLUE	= 0x02,
        BROWN	= 0x04,
        GOLD	= 0x08,
        GREEN	= 0x10,
        ORANGE	= 0x20,
        PURPLE	= 0x40,
        RED	= 0x80
    };

    const char* Color(u8);
    color_t FromMP2(u8);
}

namespace Color
{
    enum color_t
    {
	NONE	= 0x00,
        BLUE    = 0x01,
        GREEN   = 0x02,
        RED     = 0x04,
        YELLOW  = 0x08,
        ORANGE  = 0x10,
        PURPLE  = 0x20,
        GRAY    = 0x40
    };

    inline color_t& operator++ (color_t& color){ return color = ( GRAY == color ? BLUE : color_t(color << 1)); }
    inline color_t& operator-- (color_t& color){ return color = ( BLUE == color ? GRAY : color_t(color >> 1)); }

    const char* String(u8);

    u8 Count(u8);
    u8 GetIndex(color_t);
    color_t GetFromIndex(u8);
    u8 GetFirst(u8);
    color_t Get(u8);
}

#endif

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

#include <iostream>
#include "midi.h"

u8 MIDI::UnpackDelta(const u8 *p, u32 & d)
{
    const u8 *p2 = p;
    d = 0;

    while(*p2 & 0x80)
    {
	if(4 <= p2 - p)
	{
	    std::cerr << "Event: unpack delta mistake" << std::endl;
	    break;
	}

	d |= 0x0000007F & static_cast<u32>(*p2);
	d <<= 7;
	++p2;
    }

    d += *p2;

    return p2 - p + 1;
}

u8 MIDI::PackDelta(u8 *p, const u32 & d)
{
    const u8 c1 = static_cast<char>(d & 0x0000007F);
    const u8 c2 = static_cast<char>((d & 0x00003F80) >> 7);
    const u8 c3 = static_cast<char>((d & 0x001FC000) >> 14);
    const u8 c4 = static_cast<char>((d & 0x0FE00000) >> 21);

    if(c4)
    { p[0] = c4 | 0x80; p[1] = c3 | 0x80; p[2] = c2 | 0x80; p[3] = c1; }
    else
    if(c3)
    { p[0] = c3 | 0x80; p[1] = c2 | 0x80; p[2] = c1; }
    else
    if(c2)
    { p[0] = c2 | 0x80; p[1] = c1; }
    else
    { p[0] = c1; }

    return (c4 ? 4 : (c3 ? 3 : (c2 ? 2 : 1)));
}


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
#ifndef H2RESOURCE_H
#define H2RESOURCE_H

#include <string>
#include <vector>
#include "gamedefs.h"

namespace Resource
{
    enum resource_t
    {
	UNKNOWN = 0x00,
        WOOD	= 0x01,
        MERCURY	= 0x02,
        ORE	= 0x04,
        SULFUR	= 0x08,
        CRYSTAL	= 0x10,
        GEMS	= 0x20,
        GOLD	= 0x40
    };

    struct funds_t
    {
	funds_t() : wood(0), mercury(0), ore(0), sulfur(0), crystal(0), gems(0), gold(0) {};
	funds_t(u8 rs, u32 count);

	const funds_t operator+ (const funds_t &pm) const;
	const funds_t operator* (u32 mul) const;
	const funds_t operator- (const funds_t &pm) const;
	const funds_t & operator+= (const funds_t &pm);
	const funds_t & operator*= (u32 mul);
	const funds_t & operator-= (const funds_t &pm);

	bool operator< (const funds_t &pm) const;
	bool operator<= (const funds_t &pm) const;
	bool operator> (const funds_t &pm) const;
	bool operator>= (const funds_t &pm) const;

	u8 GetValidItems(void) const;
	void Reset(void);

        s32 wood;
        s32 mercury;
        s32 ore;
        s32 sulfur;
        s32 crystal;
        s32 gems;
        s32 gold;
    };

    const char* String(u8 resource);

    resource_t Rand(void);
    resource_t FromIndexSprite(u8 index);

    /* return index sprite objnrsrc.icn */
    u8 GetIndexSprite(u8 resource);

    /* return index sprite resource.icn */
    u8 GetIndexSprite2(u8 resource);

    class BoxSprite : protected Rect
    {
    public:
	BoxSprite(const funds_t &, u16);

	const Rect & GetArea(void) const;
	void SetPos(s16, s16);
	void Redraw(void) const;

	const funds_t & rs;
    };
}

#endif

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
#ifndef H2MAPS_H
#define H2MAPS_H

#include "gamedefs.h"
#include "direction.h"

#define TILEWIDTH 32

class Point;
class Castle;

namespace Maps
{
    enum mapsize_t
    {
	ZERO	= 0,
        SMALL	= 36,
        MEDIUM	= 72,
        LARGE	= 108,
        XLARGE	= 144,
	XLARGE2 = 256,
	XLARGE3	= 320
    };

    const char* SizeString(u16);
    const char* GetMinesName(u8);

    s32 GetDirectionIndex(s32 from, Direction::vector_t vector);
    bool isValidDirection(s32 from, Direction::vector_t vector);

    bool isValidAbsIndex(s32);
    bool isValidAbsPoint(const Point & pt);
    bool isValidAbsPoint(s16 x, s16 y);

    s32 GetIndexFromAbsPoint(const Point & mp);
    s32 GetIndexFromAbsPoint(s16 px, s16 py);

    u32 AnimationTicket(void);
    void IncreaseAnimationTicket(void);

    u16 GetDirectionAroundGround(const s32 center, const u16 ground);
    u8  GetCountAroundGround(const s32 center, const u16 ground);
    u16 GetMaxGroundAround(const s32 center);

    u16  ScanAroundObject(const s32 center, const u8 obj, const u16 exclude = 0);
    bool ScanDistanceObject(const s32 center, const u8 obj, const u16 dist, std::vector<s32> &);
    u16  TileUnderProtection(const s32 center);

    void ClearFog(s32 center, u8 scoute, const u8 color);
    u16 GetApproximateDistance(const s32 index1, const s32 index2);


    void UpdateRNDSpriteForCastle(const Point & center, u8 race, bool castle);
    void UpdateSpritesFromTownToCastle(const Point & center);
    void MinimizeAreaForCastle(const Point & center);
}

#endif

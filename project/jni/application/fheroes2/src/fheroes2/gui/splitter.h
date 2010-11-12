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
#ifndef H2SPLITTER_H
#define H2SPLITTER_H

#include "gamedefs.h"

class Splitter : public SpriteCursor
{
public:
    enum positions_t { HORIZONTAL, VERTICAL };

    Splitter();
    Splitter(const Surface &sf, const Rect &rt, positions_t pos);

    void Forward(void);
    void Backward(void);
    void Move(u16 pos);

    void SetArea(s16, s16, u16, u16);
    void SetArea(const Rect & rt);
    void SetOrientation(positions_t ps);
    void SetRange(u16 smin, u16 smax);

    u16 GetCurrent(void) const{ return cur; };
    u16 GetStep(void) const{ return step; };
    u16 Max(void) const{ return max; };
    u16 Min(void) const{ return min; };
    const Rect & GetRect(void) const{ return area; };
    const Rect & GetCursor(void) const{ return SpriteCursor::GetRect(); };

private:
    Rect area;
    u16 step;
    u16 min;
    u16 max;
    u16 cur;
    positions_t position;
};

#endif

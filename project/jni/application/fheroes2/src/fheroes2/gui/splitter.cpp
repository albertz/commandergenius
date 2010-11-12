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
#include "cursor.h"
#include "settings.h"
#include "splitter.h"

/* splitter constructor */
Splitter::Splitter()
{
}

Splitter::Splitter(const Surface &sf, const Rect &rt, positions_t pos)
    : SpriteCursor(sf, rt.x, rt.y), area(rt), step(0), min(0), max(0), cur(0), position(pos)
{
    SpriteCursor::Hide();
    SpriteCursor::Move(rt.x, rt.y);
    SpriteCursor::Show();
}

void Splitter::SetArea(s16 rx, s16 ry, u16 rw, u16 rh)
{
    area.x = rx;
    area.y = ry;
    area.w = rw;
    area.h = rh;
}

void Splitter::SetArea(const Rect & rt)
{
    area = rt;
    SpriteCursor::Move(rt.x, rt.y);
}

void Splitter::SetOrientation(positions_t ps)
{
    position = ps;
}

/* set range */
void Splitter::SetRange(u16 smin, u16 smax)
{
    min = smin;
    max = smax;

    // recalculate step
    if(max) step = (Splitter::VERTICAL == position ? 100 * (area.h - h()) / (max - min) : 100 * (area.w - w()) / (max - min));
    
    Splitter::Move(min);
}

/* move splitter to pos */
void Splitter::Move(u16 pos)
{
    if(pos && cur == pos) return;

    if(pos > max || pos < min){ DEBUG(DBG_ENGINE , DBG_WARN, "Splitter::Move: out of range. min: " << min << ", max: " << max << ", cur: " << cur << ", step: " << step); return; }

    Point pt(GetRect().x, GetRect().y);

    cur = pos;

    if(Splitter::VERTICAL == position)
        pt.y = area.y + cur * step / 100;
    else
        pt.x = area.x + cur * step / 100;

    // move center
    if(!max) Splitter::VERTICAL == position ? pt.y = area.y + (area.h - h()) / 2 : pt.x = area.x + (area.w - w());

    if(Cursor::Get().isVisible()) Cursor::Get().Hide();
    SpriteCursor::Hide();
    SpriteCursor::Move(pt);
    SpriteCursor::Show();
}

/* forward spliter */
void Splitter::Forward(void)
{
    if(cur == max) return;

    Point pt(GetRect().x, GetRect().y);

    ++cur;

    if(Splitter::VERTICAL == position)
        pt.y = area.y + cur * step / 100;
    else
        pt.x = area.x + cur * step / 100;

    SpriteCursor::Hide();
    SpriteCursor::Move(pt);
    SpriteCursor::Show();
}

/* backward spliter */
void Splitter::Backward(void)
{
    if(! cur) return;

    Point pt(GetRect().x, GetRect().y);

    --cur;
		    
    if(Splitter::VERTICAL == position)
	pt.y = area.y + cur * step / 100;
    else
        pt.x = area.x + cur * step / 100;

    SpriteCursor::Hide();
    SpriteCursor::Move(pt);
    SpriteCursor::Show();
}

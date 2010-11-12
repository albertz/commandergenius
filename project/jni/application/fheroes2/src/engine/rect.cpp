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

#include "rect.h"

Point::Point(s16 px, s16 py) : x(px), y(py)
{
}

bool Point::operator== (const Point & pt) const
{
    return (x == pt.x && y == pt.y);
}
          
bool Point::operator!= (const Point & pt) const
{
    return !(*this == pt);
}

Point & Point::operator+=(const Point & pt)
{
    x += pt.x;
    y += pt.y;

    return *this;
}

Point & Point::operator-=(const Point & pt)
{
    x -= pt.x;
    y -= pt.y;

    return *this;
}

Point operator+(const Point& pt1, const Point& pt2)
{
    return Point(pt1.x + pt2.x, pt1.y + pt2.y);
}

Point operator-(const Point& pt1, const Point& pt2)
{
    return Point(pt1.x - pt2.x, pt1.y - pt2.y);
}

Size::Size(u16 sw, u16 sh) : w(sw), h(sh)
{
}

bool Size::operator== (const Size & sz) const
{
    return (w == sz.w && h == sz.h);
}

bool Size::operator!= (const Size & sz) const
{
    return !(*this == sz);
}

bool Size::isEmpty(void) const
{
    return 0 == w && 0 == h;
}

Rect::Rect(s16 rx, s16 ry, u16 rw, u16 rh) : Point(rx, ry), Size(rw, rh)
{
}

Rect::Rect(const SDL_Rect & rt) : Point(rt.x, rt.y), Size(rt.w, rt.h)
{
}

Rect::Rect(const Point & pt, u16 rw, u16 rh) : Point(pt), Size(rw, rh)
{
}

Rect::Rect(const Point & pt, const Size & sz) : Point(pt), Size(sz)
{
}

Rect::Rect(const Rect & rt1, const Rect & rt2)
{
    x = rt1.x < rt2.x ? rt1.x : rt2.x;
    y = rt1.y < rt2.y ? rt1.y : rt2.y;
    w = rt1.x + rt1.w > rt2.x + rt2.w ? rt1.x + rt1.w - x : rt2.x + rt2.w - x;
    h = rt1.y + rt1.h > rt2.y + rt2.h ? rt1.y + rt1.h - y : rt2.y + rt2.h - y;
}

Rect::Rect(const std::vector<Rect> & vect)
{
    int x1 = 32766;
    int y1 = 32766;
    int x2 = -32766;
    int y2 = -32766;
       
    std::vector<Rect>::const_iterator it = vect.begin();
    
    for(; it != vect.end(); ++it)
    {
	if((*it).x < x1) x1 = (*it).x;
	if((*it).y < y1) y1 = (*it).y;
	if((*it).x + (*it).w > x2) x2 = (*it).x + (*it).w;
	if((*it).y + (*it).h > y2) y2 = (*it).y + (*it).h;
    }
    
    x = x1;
    y = y1;
    w = x2 - x1;
    h = y2 - y1;
}

Rect & Rect::operator= (const Point & pt)
{
    x = pt.x;
    y = pt.y;

    return *this;
}

bool Rect::operator== (const Rect & rt) const
{
    return (x == rt.x && y == rt.y && w == rt.w && h == rt.h);
}

bool Rect::operator!= (const Rect & rt) const
{
    return !(*this == rt);
}

bool Rect::operator& (const Point & pt) const
{
    return !(pt.x < x || pt.y < y || pt.x >= (x + w) || pt.y >= (y + h));
}

bool Rect::operator& (const Rect & rt) const
{
    return
	((rt.x >= x && rt.x < x + w) ||
	(rt.x + rt.w >= x && rt.x + rt.w < x + w) ||
	(x >= rt.x && x < rt.x + rt.w) ||
	(x + w >= rt.x && x + w < rt.x + rt.w)) &&
	((rt.y >= y && rt.y < y + h) ||
	(rt.y + rt.h >= y && rt.y + rt.h < y + h) ||
	(y >= rt.y && y < rt.y + rt.h) ||
	(y + h >= rt.y && y + h < rt.y + rt.h));
}

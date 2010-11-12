/***************************************************************************
 *   Copyright (C) 2010 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#include "maps.h"
#include "world.h"
#include "position.h"

Maps::Position::Position(const Point & pt) : center(pt)
{
}

const Point & Maps::Position::GetCenter(void) const
{
    return center;
}

s32 Maps::Position::GetIndex(void) const
{
    return Maps::GetIndexFromAbsPoint(center);
}

void Maps::Position::SetCenter(const Point & pt)
{
    center = pt;
}

void Maps::Position::SetIndex(s32 index)
{
    center = Maps::isValidAbsIndex(index) ?
		Point(index % world.w(), index / world.w()) : Point(-1, -1);
}

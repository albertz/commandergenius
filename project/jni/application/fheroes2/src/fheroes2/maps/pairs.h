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

#ifndef H2PAIRS_H
#define H2PAIRS_H

#include <utility>
#include "maps_tiles.h"

class IndexDistance : public std::pair<s32, u16>
{
    public:
    IndexDistance() : std::pair<s32, u16>(-1, 0) {};
    IndexDistance(s32 i, u16 d) : std::pair<s32, u16>(i, d) {};

    static bool Shortest(const IndexDistance & id1, const IndexDistance & id2){ return id1.second < id2.second; };
    static bool Longest(const IndexDistance & id1, const IndexDistance & id2){ return id1.second > id2.second; };
};

class IndexObject : public std::pair<s32, MP2::object_t>
{
    public:
    IndexObject() : std::pair<s32, MP2::object_t>(-1, MP2::OBJ_ZERO) {};
    IndexObject(const std::pair<s32, MP2::object_t> & pair) : std::pair<s32, MP2::object_t>(pair) {};
    IndexObject(const s32 index, const MP2::object_t object) : std::pair<s32, MP2::object_t>(index, object) {};
    IndexObject(const Maps::Tiles & tile) : std::pair<s32, MP2::object_t>(tile.GetIndex(), tile.GetObject()) {};

    bool isIndex(s32 index) const { return index == first; };
    bool isObject(u8 object) const { return object == second; };
};

class ObjectColor : public std::pair<MP2::object_t, Color::color_t>
{
    public:
    ObjectColor() : std::pair<MP2::object_t, Color::color_t>(MP2::OBJ_ZERO, Color::GRAY) {};
    ObjectColor(const MP2::object_t object, Color::color_t color) : std::pair<MP2::object_t, Color::color_t>(object, color) {};

    bool isObject(u8 object) const { return object == first; };
    bool isColor(u8 color) const { return color == second; };
};

#endif

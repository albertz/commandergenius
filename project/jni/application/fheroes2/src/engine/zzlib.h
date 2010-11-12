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

#ifndef H2ZLIB_H
#define H2ZLIB_H

#ifdef WITH_ZLIB

#include <string>
#include <vector>
#include "types.h"
#include "surface.h"

namespace ZLib
{
    bool	UnCompress(std::vector<char> &, const char*, size_t, bool debug = false);
    bool	UnCompress(std::vector<char> &, const std::vector<char> &, bool debug = false);
    bool	UnCompress(std::vector<char> &, const std::string &, bool debug = false);

    bool	Compress(std::vector<char> &, const char*, size_t);
    bool	Compress(std::vector<char> &, const std::vector<char> &);
    bool	Compress(std::vector<char> &, const std::string &);
}

class ZSurface : public Surface
{
public:
    ZSurface(){}

    bool Load(u16 w, u16 h, u8 b, const u8* p, size_t s, bool a);

private:
    std::vector<char> buf;
};

#endif
#endif

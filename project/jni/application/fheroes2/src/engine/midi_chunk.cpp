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

#include <cstring>
#include <iostream>
#include <iomanip>

#include "midi_chunk.h"

using namespace MIDI;

Chunk::Chunk() : size(0), data(NULL)
{
    memset(id, '\0', 4);
}

Chunk::Chunk(const char *i, const u32 s, const u8 *p) : size(s), data(NULL)
{
    i ? memcpy(id, i, 4) : memset(id, '\0', 4);

    if(size)
    {
	data = new u8[size];

	if(p) memcpy(data, p, size); else memset(data, '\0', size);
    }
}

Chunk::Chunk(std::istream & i) : size(0), data(NULL)
{
    memset(id, '\0', 4);
    Read(i);
}

Chunk::Chunk(const u8 *p) : size(0), data(NULL)
{
    memset(id, '\0', 4);
    Read(p);
}

Chunk::Chunk(const Chunk & c) : size(c.size), data(NULL)
{
    c.id ? memcpy(id, c.id, 4) : memset(id, '\0', 4);

    if(size)
    {
	data = new u8 [size];
	memcpy(data, c.data, size);
    }
}

Chunk::~Chunk()
{
    if(data) delete [] data;
}

Chunk & Chunk::operator= (const Chunk & c)
{
    if(data) delete [] data;
    data = NULL;

    c.id ? memcpy(id, c.id, 4) : memset(id, '\0', 4);
    size = c.size;

    if(size)
    {
	data = new u8 [size];
	memcpy(data, c.data, size);
    }

    return *this;
}

bool Chunk::Read(std::istream & i)
{
    if(i.fail()) return false;

    i.read(id, 4);

    i.read(reinterpret_cast<char *>(&size), 4);
    SwapBE32(size);

    if(data) delete [] data;
    data = NULL;

    if(size)
    {
	data = new u8 [size];
	i.read(reinterpret_cast<char *>(data), size);
    }

    return true;
}

bool Chunk::Read(const std::vector<u8> & b)
{
    if(8 > b.size()) return false;

    memcpy(id, &b[0], 4);

    size = ReadBE32(&b[4]);

    if(data) delete [] data;
    data = NULL;

    if(size + 8 > b.size()) size = b.size() - 8;

    if(size)
    {
	data = new u8 [size];
	memcpy(data, &b[8], size);
    }

    return true;
}

bool Chunk::Read(const u8 *p)
{
    if(NULL == p) return false;

    memcpy(id, p, 4);

    size = ReadBE32(&p[4]);

    if(data) delete [] data;
    data = NULL;

    if(size)
    {
	data = new u8 [size];
	memcpy(data, &p[8], size);
    }

    return true;
}

bool Chunk::Write(std::ostream & o) const
{
    if(o.fail()) return false;

    o.write(id, 4);

    u32 x = size;
    SwapBE32(x);
    o.write(reinterpret_cast<char *>(&x), 4);

    if(size && data) o.write(reinterpret_cast<char *>(data), size);

    return true;
}

bool Chunk::Write(u8 *p) const
{
    if(NULL == p) return false;

    memcpy(p, id, 4);

    WriteBE32(&p[4], size);

    if(size && data) memcpy(&p[8], data, size);

    return true;
}

void Chunk::Dump(void) const
{
    std::cerr << "id:   ";
    std::cerr.write(id, 4);
    std::cerr << std::endl << "size: " << std::dec << size << std::endl << "data: " << std::endl;

    u8 endline = 0;
    for(u32 ii = 0; ii < size; ++ii)
    {
        std::cerr << " 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<u32>(static_cast<u8>(data[ii])) << ":";
        ++endline;

        if(endline > 15)
        {
            endline = 0;
            std::cerr << std::endl;
        }
    }
    std::cerr << std::endl;
}

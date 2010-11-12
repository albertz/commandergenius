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

#include <fstream>
#include <iostream>
#include <cstring>
#include <vector>
#include "midi_xmi.h"

#define ID_FORM	"FORM"
#define ID_CAT	"CAT "
#define ID_XMID "XMID"
#define ID_TIMB "TIMB"
#define ID_EVNT "EVNT"

using namespace MIDI;

Xmi::Xmi()
{
}

bool Xmi::Read(const std::vector<u8> & body)
{
    if(0 == body.size())
    {
        std::cerr << "Xmi: " << "incorrect size" << std::endl;
        return false;
    }

    const u8 *ptr = &body[0];

    if(memcmp(ID_FORM, ptr, 4))
    {
        std::cerr << "Xmi: " << "incorrect id: " << ID_FORM << std::endl;
        return false;
    }

    head.Read(ptr);
    ptr += 8 + head.size;

    if(memcmp(ID_CAT, ptr, 4))
    {
        std::cerr << "Xmi: " << "incorrect id: " << ID_CAT<< std::endl;
        return false;
    }

    ptr += 8;

    if(memcmp(ID_XMID, ptr, 4))
    {
        std::cerr << "Xmi: " << "incorrect cat id: " << ID_XMID << std::endl;
        return false;
    }

    ptr += 4;

    if(memcmp(ID_FORM, ptr, 4))
    {
        std::cerr << "Xmi: " << "incorrect xmid id: " << ID_FORM << std::endl;
        return false;
    }
    else
        ptr += 8;

    if(memcmp(ID_XMID, ptr, 4))
    {
        std::cerr << "Xmi: " << "incorrect form id: " << ID_XMID << std::endl;
        return false;
    }

    ptr += 4;

    if(memcmp(ID_TIMB, ptr, 4))
    {
        std::cerr << "Xmi: " << "incorrect id: " << ID_TIMB << std::endl;
        return false;
    }

    timb.Read(ptr);
    ptr += 8 + timb.size;

    if(memcmp(ID_EVNT, ptr, 4))
    {
        std::cerr << "Xmi: " << "incorrect id: " << ID_EVNT << std::endl;
        return false;
    }

    evnt.Read(ptr);

    return true;
}

bool Xmi::Read(const std::string & filename)
{
    std::ifstream fd(filename.c_str(), std::ios::binary);

    if(!fd.is_open())
    {
        std::cerr << "Xmi: " << "error read: " << filename.c_str() << std::endl;
        return false;
    }

    fd.seekg(0, std::ios_base::end);
    const u32 size = fd.tellg();
    fd.seekg(0, std::ios_base::beg);

    std::vector<u8> body(size);
    fd.read(reinterpret_cast<char*>(&body[0]), size);
    fd.close();

    return Read(body);
}

void Xmi::Dump(void) const
{
    head.Dump();
    timb.Dump();
    evnt.Dump();
}

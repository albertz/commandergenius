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

#include <string>
#include <fstream>
#include <iostream>
#include "midi_chunk.h"
#include "midi_mid.h"

using namespace MIDI;

Mid::Mid()
{
}

Mid::Mid(const Mid & m) : mthd(m.mthd)
{
    std::list<MTrk *>::const_iterator it1 = m.tracks.begin();
    std::list<MTrk *>::const_iterator it2 = m.tracks.end();

    for(; it1 != it2; ++it1) if(*it1) tracks.push_back(new MTrk(**it1));
}

Mid::~Mid()
{
    if(tracks.size())
    {
	std::list<MTrk *>::const_iterator it1 = tracks.begin();
	std::list<MTrk *>::const_iterator it2 = tracks.end();
	
	for(; it1 != it2; ++it1) if(*it1) delete *it1;
    }
}

Mid & Mid::operator= (const Mid & m)
{
    mthd = m.mthd;

    if(tracks.size())
    {
	std::list<MTrk *>::const_iterator it1 = tracks.begin();
	std::list<MTrk *>::const_iterator it2 = tracks.end();
	
	for(; it1 != it2; ++it1) delete *it1;
    }

    std::list<MTrk *>::const_iterator it1 = m.tracks.begin();
    std::list<MTrk *>::const_iterator it2 = m.tracks.end();

    for(; it1 != it2; ++it1) if(*it1) tracks.push_back(new MTrk(**it1));

    return *this;
}

bool Mid::Read(const std::vector<u8> & body)
{
    mthd.Read(body);

    if(! mthd.isValid())
    {
        std::cerr << "Mid::Read: " << "error format" << std::endl;

        return false;
    }

    const u32 count = mthd.Tracks();
    const u8 *ptr = &body[mthd.Size()];

    for(u16 ii = 0; ii < count; ++ii)
    {
	if(ptr >= &body[0] + body.size())
	{
    	    std::cerr << "Mid::Read: " << "error read chunk, total: " << count << ", current: " << ii << std::endl;

    	    return false;
	}

	const Chunk chunk(ptr);

	if(0 == memcmp(ID_MTRK, chunk.id, 4)) tracks.push_back(new MTrk(chunk.data, chunk.size));
	else --ii;

	ptr += 8 + chunk.size;
    }

    return true;
}

bool Mid::Read(const std::string & filename)
{
    std::ifstream fd(filename.c_str(), std::ios::binary);

    if(!fd.is_open())
    {
        std::cerr << "Mid::Read: " << "error read: " << filename << std::endl;

        return false;
    }

    mthd.Read(fd);
    
    if(! mthd.isValid())
    {
        std::cerr << "Mid::Read: " << "error format: " << filename << std::endl;

        return false;
    }

    const u32 count = mthd.Tracks();

    for(u16 ii = 0; ii < count; ++ii)
    {
	if(fd.fail())
	{
    	    std::cerr << "Mid::Read: " << "error read chunk, total: " << count << ", current: " << ii << std::endl;

    	    return false;
	}

	const Chunk chunk(fd);

	if(0 == memcmp(ID_MTRK, chunk.id, 4)) tracks.push_back(new MTrk(chunk.data, chunk.size));
	else --ii;
    }

    fd.close();

    return true;
}

u32 Mid::Size(void) const
{
    u32 total = mthd.Size();

    if(tracks.size())
    {
	std::list<MTrk *>::const_iterator it1 = tracks.begin();
	std::list<MTrk *>::const_iterator it2 = tracks.end();

	for(; it1 != it2; ++it1) if(*it1) total += (*it1)->Size();
    }
    
    return total;
}

bool Mid::Write(std::vector<u8> & body)
{
    body.resize(Size());
    u8 *ptr = &body[0];

    mthd.Write(ptr);
    ptr += mthd.Size();

    if(tracks.size())
    {
	std::list<MTrk *>::const_iterator it1 = tracks.begin();
	std::list<MTrk *>::const_iterator it2 = tracks.end();

	for(; it1 != it2; ++it1) if(*it1){ (*it1)->Write(ptr); ptr += (*it1)->Size(); }
    }

    return true;
}

bool Mid::Write(const std::string & filename)
{
    std::ofstream fd(filename.c_str(), std::ios::binary);

    if(!fd.is_open())
    {
        std::cerr << "Mid::Write: " << "error write: " << filename << std::endl;

        return false;
    }

    mthd.Write(fd);

    if(tracks.size())
    {
	std::list<MTrk *>::const_iterator it1 = tracks.begin();
	std::list<MTrk *>::const_iterator it2 = tracks.end();

	for(; it1 != it2; ++it1) if(*it1) (*it1)->Write(fd);
    }

    fd.close();

    return true;
}

void Mid::Dump(void) const
{
    mthd.Dump();

    if(tracks.size())
    {
	std::list<MTrk *>::const_iterator it1 = tracks.begin();
	std::list<MTrk *>::const_iterator it2 = tracks.end();

	for(; it1 != it2; ++it1) if(*it1) (*it1)->Dump();
    }
}

void Mid::AddTrack(MTrk & track)
{
    tracks.push_back(new MTrk(track));

    mthd.SetTracks(mthd.Tracks() + 1);
}

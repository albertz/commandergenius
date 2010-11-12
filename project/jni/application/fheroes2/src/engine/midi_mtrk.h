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

#ifndef MIDI_TRACK_H
#define MIDI_TRACK_H

#include <list>
#include <ostream>
#include "midi.h"
#include "midi_chunk.h"
#include "midi_event.h"

#define ID_MTRK "MTrk"

namespace MIDI
{
    u8 UnpackDelta(const char *p, u32 & d);
    u8 PackDelta(char *p, const u32 & d);

    class MTrk
    {
    public:
	MTrk() {};
        MTrk(const u8 *p, const u32 s);
        MTrk(const MTrk & t);
	~MTrk();

	bool Write(std::ostream & o) const;
	bool Write(u8 *p) const;
	u32 Size(void) const;

	void AddEvent(const Event & e);
	void ImportXmiEVNT(const Chunk & c);
	void CloseEvents(void);

	void Dump(void) const;

    private:
	std::list<Event *> events;
    };
}

#endif

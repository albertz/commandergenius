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

#ifndef MIDI_MTHD_H
#define MIDI_MTHD_H

#include <vector>
#include <istream>
#include <cstring>
#include "midi.h"
#include "midi_chunk.h"

#define ID_MTHD	"MThd"

namespace MIDI
{
    class MThd : protected Chunk
    {
    public:
	MThd() : Chunk(ID_MTHD, 6) {};
	MThd(const u8 *p, const u32 s) : Chunk(ID_MTHD, s, p) {};
	MThd(std::istream & i) : Chunk(i) {};

	bool isValid(void) const{ return 0 == memcmp(Chunk::id, ID_MTHD, 4); };
	bool Read(std::istream & is){ return Chunk::Read(is); };
	bool Read(const std::vector<u8> & b){ return Chunk::Read(b); };
	bool Write(std::ostream & os) const{ return Chunk::Write(os); };
	bool Write(u8* b) const{ return Chunk::Write(b); };

	void SetFormat(const u16 f);
	void SetTracks(const u16 t);
	void SetPPQN(const u16 p);

	const u8* Data(void) const{ return Chunk::data; };
	u32 Size(void) const{ return 8 + size; };
	u16 Format(void) const{ return ReadBE16(reinterpret_cast<const u8*>(&data[0])); };
	u16 Tracks(void) const{ return ReadBE16(reinterpret_cast<const u8*>(&data[2])); };
	u16 PPQN(void) const{ return ReadBE16(reinterpret_cast<const u8*>(&data[4])); };

	void Dump(void) const;
    };
}

#endif

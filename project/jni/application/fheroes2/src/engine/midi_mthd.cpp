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
#include "midi_mthd.h"

using namespace MIDI;

void MThd::Dump(void) const
{
    std::cerr << "[MThd]    format: " << Format() << ", tracks: " << Tracks() << ", ppqn: " << PPQN() << std::endl;
}

void MThd::SetFormat(const u16 f)
{
    WriteBE16(reinterpret_cast<u8*>(&data[0]), f);
}

void MThd::SetTracks(const u16 t)
{
    WriteBE16(reinterpret_cast<u8*>(&data[2]), t);
}

void MThd::SetPPQN(const u16 p)
{
    WriteBE16(reinterpret_cast<u8*>(&data[4]), p);
}

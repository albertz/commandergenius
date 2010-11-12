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

#ifndef MIDI_EVENT_H
#define MIDI_EVENT_H

#include <ostream>
#include "midi.h"

namespace MIDI
{
    class Event
    {
    public:
	Event();
	Event(const u32 dl, const u8 st, const u32 sz, const u8 *p);
	Event(const Event &);
	~Event();

	Event & operator= (const Event &);

	u32	Size(void) const;
	u32	Delta(void) const { return delta; };
	u8	Status(void) const { return status; };

	void	SetDelta(const u32 dl);

	void	Dump(void) const;
	bool	Write(u8 *p) const;
	bool	Write(std::ostream & o) const;

    protected:
	u32	delta;
	char	status;

	u8*	data;
	u32	size;

	u8	pack[4];
	u8	sp;
    };
}

#endif

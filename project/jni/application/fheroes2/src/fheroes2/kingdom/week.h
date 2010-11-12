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

#ifndef H2WEEK_H
#define H2WEEK_H

#include <string>

namespace Week
{
    enum type_t
    {
	UNNAMED,
	PLAGUE,
	ANT,
	GRASSHOPPER,
	DRAGONFLY,
	SPIDER,
	BUTTERFLY,
	BUMBLEBEE,
	LOCUST,
	EARTHWORM,
	HORNET,
	BEETLE,
	SQUIRREL,
	RABBIT,
	GOPHER,
	BADGER,
	EAGLE,
	WEASEL,
	RAVEN,
	MONGOOSE,
	AARDVARK,
	LIZARD,
	TORTOISE,
	HEDGEHOG,
	CONDOR,
	
	MONSTERS	// week of monsters game
    };

    const char* GetString(type_t type);
    type_t WeekRand(void);
    type_t MonthRand(void);
    type_t Get(u8);
}

#endif

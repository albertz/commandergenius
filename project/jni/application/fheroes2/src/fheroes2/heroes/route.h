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
#ifndef H2HEROPATH_H
#define H2HEROPATH_H

#include <list>
#include "gamedefs.h"
#include "direction.h"
#include "game_io.h"

class Heroes;

namespace Route
{
    class Step : public std::pair<Direction::vector_t, u16>
    {
	public:
	Step() : std::pair<Direction::vector_t, u16>(Direction::CENTER, 0) {}
	Step(const Direction::vector_t v, const u16 c) : std::pair<Direction::vector_t, u16>(v, c) {}

	Direction::vector_t Direction() const { return first; }
	u16 Penalty() const { return second; }
    };

    class Path : public std::list<Step>
    {
	public:
	    Path(const Heroes & h);

	    s32		GetDestinationIndex(void) const{ return dst; }
	    Direction::vector_t GetFrontDirection(void) const;
	    u16		GetFrontPenalty(void) const;
	    s32		Calculate(const s32 dst_index, const u16 limit = MAXU16);

	    void	Show(void){ hide = false; }
	    void	Hide(void){ hide = true; }
	    void	Reset(void);
	    void	PopFront(void);
	    void	ScanObstacleAndReduce(void);

	    bool	isValid(void) const { return size(); }
	    bool	isShow(void) const { return !hide; }
	    u16		isUnderProtection(s32 & pos) const;
	    bool	hasObstacle(s32* res = NULL) const;

	    void	Dump(void) const;

	    u16		GetAllowStep(void) const;
	    u32		TotalPenalty(void) const;

	    const_iterator Begin(void) const{ return begin(); }
	    const_iterator End(void) const{ return end(); }

    	    static u16	GetIndexSprite(const Direction::vector_t & from, const Direction::vector_t & to, u8 mod);

	private:
	    friend class Game::IO;
	    const Heroes & hero;
	    s32		dst;
	    bool	hide;
    };
}

#endif

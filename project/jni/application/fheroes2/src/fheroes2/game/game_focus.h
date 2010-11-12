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

#ifndef H2GAMEFOCUS_H
#define H2GAMEFOCUS_H

#include "gamedefs.h"
#include "game.h"

class Castle;
class Heroes;

namespace Game
{
    class Focus
    {
    public:
	enum focus_t { UNSEL, HEROES, CASTLE };

   	static Focus &	Get(void);

	void		Set(Heroes *hr);
	void		Set(Castle *cs);
	void		SetRedraw(void);
	void		Reset(const focus_t priority = UNSEL);

	void		CheckIconsPanel(void);

	const Point &	Center(void) const;

	focus_t		Type(void) const;

	const Castle &	GetCastle(void) const;
	const Heroes &	GetHeroes(void) const;
	
	Heroes & GetHeroes(void);
	Castle & GetCastle(void);

    private:
	Focus();

	Castle *	castle;
	Heroes *	heroes;
	Point		center;
    };
}

#endif

/***************************************************************************
 *   Copyright (C) 2008 by Andrey Afletdinov <fheroes2@gmail.com>          *
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
#ifndef H2BACKGROUND_H
#define H2BACKGROUND_H

#include "types.h"
#include "rect.h"
#include "surface.h"

class Background : protected Surface, protected Rect
{
public:
    Background(const Rect &rt = Rect());
    Background(s16 x, s16 y, u16 w, u16 h);
    Background(const Point &pt, u16 w, u16 h);

    bool isValid(void) const;

    void Save(void);
    void Save(s16 ax, s16 ay);
    void Save(s16 ax, s16 ay, u16 aw ,u16 ah);
    void Save(const Point &pt);
    void Save(const Rect &rt);

    void Restore(void);
    const Rect & GetRect(void) const;
    const Point & GetPos(void) const;
    const Size & GetSize(void) const;
};

#endif

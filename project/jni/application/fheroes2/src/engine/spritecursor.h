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
#ifndef H2SPRITECURSOR_H
#define H2SPRITECURSOR_H

#include "background.h"
#include "types.h"

class Point;
class Surface;

class SpriteCursor : public Background
{
public:
    /* sprite cursor */
    SpriteCursor();
    SpriteCursor(const Surface &cursor, const Point & pt = Point());
    SpriteCursor(const Surface &cursor, s16 x, s16 y);

    u16  w(void) const;
    u16  h(void) const;

    void SetSprite(const Surface & sf);
    const Surface* Sprite(void);

    void Move(s16 ax, s16 ay);
    void Move(const Point &pt);

    void Hide(void);

    void Show(s16 ax, s16 ay);
    void Show(const Point &pt);
    void Show(void);

    void Redraw(void);
    
    bool isVisible(void) const;

private:
    const Surface* sprite;
    bool visible;
};

#endif

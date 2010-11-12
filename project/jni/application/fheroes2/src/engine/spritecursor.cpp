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

#include "rect.h"
#include "surface.h"
#include "display.h"
#include "spritecursor.h"

SpriteCursor::SpriteCursor() : Background(), sprite(NULL), visible(false)
{
}

SpriteCursor::SpriteCursor(const Surface &cursor, const Point & pt) : Background(pt, cursor.w(), cursor.h()), sprite(&cursor), visible(false)
{
}

SpriteCursor::SpriteCursor(const Surface &cursor, s16 x, s16 y) : Background(x, y, cursor.w(), cursor.h()), sprite(&cursor), visible(false)
{
}

u16  SpriteCursor::w(void) const
{
    return sprite ? sprite->w() : 0;
}

u16  SpriteCursor::h(void) const
{
    return sprite ? sprite->h() : 0;
}

void SpriteCursor::SetSprite(const Surface & sf)
{
    if(visible) Restore();

    Save(Background::x, Background::y, sf.w(), sf.h());

    sprite = &sf;
}

const Surface* SpriteCursor::Sprite(void)
{
    return sprite && sprite->isValid() ? sprite : NULL;
}

void SpriteCursor::Move(const Point &pt)
{
    Move(pt.x, pt.y);
}

void SpriteCursor::Move(s16 ax, s16 ay)
{
    if(Background::x == ax && Background::y == ay)
    {
	if(!visible) Show();
    }
    else
    {
	if(visible) Hide();
	Show(ax, ay);
    }
}

void SpriteCursor::Hide(void)
{
    if(!visible) return;

    Restore();

    visible = false;
}

void SpriteCursor::Redraw(void)
{
    if(visible)
    {
	Hide();
	Show();
    }
}

void SpriteCursor::Show(void)
{
    Show(GetPos());
}

void SpriteCursor::Show(const Point &pt)
{
    Show(pt.x, pt.y);
}

void SpriteCursor::Show(s16 ax, s16 ay)
{
    if(visible) return;

    Save(ax, ay);

    if(sprite) Display::Get().Blit(*sprite, ax, ay);
    
    visible = true;
}

bool SpriteCursor::isVisible(void) const
{
    return visible;
}

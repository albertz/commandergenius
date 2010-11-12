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

#ifndef H2SELECTARTIFACTBAR_H
#define H2SELECTARTIFACTBAR_H

#include "gamedefs.h"

class Rect;
class Surface;
class Heroes;
class SpriteCursor;
class LocalEvent;

class SelectArtifactsBar
{
public:
    SelectArtifactsBar(Heroes &);

    const Rect &GetArea(void) const;
    bool	isSelected(void) const;
    s8		GetIndexFromCoord(const Point &);
    s8		Selected(void) const;

    bool	ReadOnly(void) const;
    bool	isValid(void) const;

    void 	SetPos(const Point &);
    void 	SetPos(s16, s16);
    void 	SetBackgroundSprite(const Surface &);
    void	SetCursorSprite(const Surface &);
    void 	SetInterval(u8);

    void	SetReadOnly(void);
    void        SetUseArts32Sprite(void);

    void 	Redraw(Surface & display = Display::Get());
    void 	Reset(void);
    void	Select(u8);

    static bool QueueEventProcessing(SelectArtifactsBar &);
    static bool QueueEventProcessing(SelectArtifactsBar &, SelectArtifactsBar &);

private:
    Heroes &		hero;
    Rect		pos;
    u8			interval;
    s8			selected;
    u8			flags;
    Point		offset;

    const Surface *	background;
    SpriteCursor	spritecursor;
};

#endif

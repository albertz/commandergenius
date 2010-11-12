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

#ifndef H2SELECTARMYBAR_H
#define H2SELECTARMYBAR_H

#include "gamedefs.h"
#include "army.h"

class Rect;
class Surface;
class SpriteCursor;
class LocalEvent;
class Castle;

class SelectArmyBar
{
public:
    SelectArmyBar();

    const Rect &GetArea(void) const;
    bool	isSelected(void) const;
    s8		GetIndexFromCoord(const Point &);
    s8		Selected(void) const;

    bool	ReadOnly(void) const;
    bool	SaveLastTroop(void) const;
    bool	isValid(void) const;

    void 	SetArmy(Army::army_t &);
    void 	SetPos(const Point &);
    void 	SetPos(s16, s16);
    void 	SetBackgroundSprite(const Surface &);
    void	SetCursorSprite(const Surface &);
    void 	SetInterval(u8);
    void	SetCastle(const Castle &);

    void	SetReadOnly(void);
    void	SetSaveLastTroop(void);
    void 	SetUseMons32Sprite(void);	// second variant: for small sprite ICN::MONS32
    void	SetCount2Sprite(void);

    void 	Redraw(Surface & display = Display::Get());
    void 	Reset(void);
    void	Select(u8);

    static bool QueueEventProcessing(SelectArmyBar &);
    static bool QueueEventProcessing(SelectArmyBar &, SelectArmyBar &);

private:
    Army::army_t *	army;
    Rect		pos;
    u8			interval;
    s8			selected;
    u8			flags;
    Point		offset;

    const Surface *	background;
    SpriteCursor	spritecursor;

    const Castle *	castle;
};

#endif

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

#include "agg.h"
#include "cursor.h"
#include "settings.h"
#include "dialog.h"

#define BUTTON_HEIGHT   40
#define BOX_WIDTH       306
#define BOX_TOP         99
#define BOX_MIDDLE      45
#define BOX_BOTTOM      81
#define BOXE_TOP        88
#define BOXE_MIDDLE     45
#define BOXE_BOTTOM     81
#define BOXAREA_TOP     35
#define BOXAREA_MIDDLE  10
#define BOXAREA_BOTTOM  35

void BoxRedraw(s16 posx, s16 posy, u8 count);

Dialog::Box::Box(u16 height, bool buttons)
{
    Display & display = Display::Get();

    if(buttons) height += BUTTON_HEIGHT;

    bool evil = Settings::Get().EvilInterface();
    const u8 count_middle = (height <= BOXAREA_TOP + BOXAREA_BOTTOM ? 0 : 1 + (height - BOXAREA_TOP - BOXAREA_BOTTOM) / BOXAREA_MIDDLE);
    const u16 height_middle = count_middle * BOXAREA_MIDDLE;
    const u16 height_top_bottom = (evil ? BOXE_TOP + BOXE_BOTTOM : BOX_TOP + BOX_BOTTOM);

    area.w = BOXAREA_WIDTH;
    area.h = BOXAREA_TOP + BOXAREA_BOTTOM + height_middle;

    s16 posx = (display.w() - BOX_WIDTH) / 2;
    s16 posy = (display.h() - height_top_bottom - height_middle) / 2;

    if(Settings::Get().QVGA() && height > display.h())
	posy = display.h() - area.h - ((evil ? BOXE_TOP : BOX_TOP) - BOXAREA_TOP);

    Save(posx, posy, BOX_WIDTH, height_top_bottom + height_middle);

    area.x = Rect::x + 36;
    area.y = Rect::y + (evil ? BOXE_TOP - BOXAREA_TOP : BOX_TOP - BOXAREA_TOP);

    BoxRedraw(Rect::x, Rect::y, count_middle);
}

Dialog::Box::~Box()
{
    if(Cursor::Get().isVisible())
    {
	Cursor::Get().Hide();
	Background::Restore();
	Cursor::Get().Show();
    }
    else
	Background::Restore();
    Display::Get().Flip();
}

void BoxRedraw(s16 posx, s16 posy, u8 count)
{
    Display & display = Display::Get();
    const ICN::icn_t buybuild = Settings::Get().EvilInterface() ? ICN::BUYBUILE : ICN::BUYBUILD;

    // left top sprite
    Point pt(posx, posy);
    if(!Settings::Get().EvilInterface()) ++pt.x;
    display.Blit(AGG::GetICN(buybuild, 4), pt);

    // right top sprite
    pt.x = posx + AGG::GetICN(buybuild, 4).w();
    display.Blit(AGG::GetICN(buybuild, 0), pt);

    pt.y += AGG::GetICN(buybuild, 4).h();
    for(int i = 0; i < count; ++i)
    {
	// left middle sprite
	pt.x = posx;
	const Sprite & sl = AGG::GetICN(buybuild, 5);
	display.Blit(sl, Rect(0, 10, sl.w(), BOXAREA_MIDDLE), pt);

	// right middle sprite
	pt.x += sl.w();
	if(!Settings::Get().EvilInterface()) pt.x -= 1;
	const Sprite & sr = AGG::GetICN(buybuild, 1);
	display.Blit(sr, Rect(0, 10, sr.w(), BOXAREA_MIDDLE), pt);
	pt.y += BOXAREA_MIDDLE;
    }

    // right bottom sprite
    display.Blit(AGG::GetICN(buybuild, 2), pt);

    // left bottom sprite
    pt.x = posx;
    display.Blit(AGG::GetICN(buybuild, 6), pt);
}

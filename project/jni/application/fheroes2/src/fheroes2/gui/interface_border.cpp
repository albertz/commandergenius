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
#include "maps.h"
#include "settings.h"
#include "interface_border.h"

Interface::BorderWindow::BorderWindow()
{
}

Interface::BorderWindow & Interface::BorderWindow::Get(void)
{
    static BorderWindow borderWindow;
    return borderWindow;
}

void Interface::BorderWindow::Redraw(void)
{
    const Settings & conf = Settings::Get();
    if(conf.HideInterface()) return;

    Display & display = Display::Get();

    const bool evil = Settings::Get().EvilInterface();
    u8 count_w = (display.w() - 640) / TILEWIDTH;
    u8 count_h = (display.h() - 480) / TILEWIDTH;
    const u8 count_icons = count_h > 3 ? 8 : ( count_h < 3 ? 4 : 7);

    if(display.w() % TILEWIDTH) ++count_w;
    if(display.h() % TILEWIDTH) ++count_h;

    Rect srcrt;
    Point dstpt;
    const Sprite &icnadv = AGG::GetICN(evil ? ICN::ADVBORDE : ICN::ADVBORD, 0);

    // TOP BORDER
    srcrt.x = 0;
    srcrt.y = 0;
    srcrt.w = 223;
    srcrt.h = BORDERWIDTH;
    dstpt.x = srcrt.x;
    dstpt.y = srcrt.y;
    display.Blit(icnadv, srcrt, dstpt);
    srcrt.x = 223;
    srcrt.w = TILEWIDTH;
    dstpt.x = srcrt.x;
    dstpt.y = 0;
    for(u8 ii = 0; ii < count_w + 1; ++ii)
    {
        display.Blit(icnadv, srcrt, dstpt);
	dstpt.x += TILEWIDTH;
    }
    srcrt.x += TILEWIDTH;
    srcrt.w = icnadv.w() - srcrt.x;
    display.Blit(icnadv, srcrt, dstpt);


    // LEFT BORDER
    srcrt.x = 0;
    srcrt.y = 0;
    srcrt.w = BORDERWIDTH;
    srcrt.h = 255;
    dstpt.x = srcrt.x;
    dstpt.y = srcrt.y;
    display.Blit(icnadv, srcrt, dstpt);
    srcrt.y = 255;
    srcrt.h = TILEWIDTH;
    dstpt.x = srcrt.x;
    dstpt.y = srcrt.y;
    for(u8 ii = 0; ii < count_h + 1; ++ii)
    {
        display.Blit(icnadv, srcrt, dstpt);
	dstpt.y += TILEWIDTH;
    }
    srcrt.y += TILEWIDTH;
    srcrt.h = icnadv.h() - srcrt.y;
    display.Blit(icnadv, srcrt, dstpt);

    // MIDDLE BORDER
    srcrt.x = icnadv.w() - RADARWIDTH - 2 * BORDERWIDTH;
    srcrt.y = 0;
    srcrt.w = BORDERWIDTH;
    srcrt.h = 255;
    dstpt.x = display.w() - RADARWIDTH - 2 * BORDERWIDTH;
    dstpt.y = srcrt.y;
    display.Blit(icnadv, srcrt, dstpt);
    srcrt.y = 255;
    srcrt.h = TILEWIDTH;
    dstpt.x = display.w() - RADARWIDTH - 2 * BORDERWIDTH;
    dstpt.y = srcrt.y;
    for(u8 ii = 0; ii < count_h + 1; ++ii)
    {
        display.Blit(icnadv, srcrt, dstpt);
        dstpt.y += TILEWIDTH;
    }
    srcrt.y += TILEWIDTH;
    srcrt.h = icnadv.h() - srcrt.y;
    display.Blit(icnadv, srcrt, dstpt);

    // RIGHT BORDER
    srcrt.x = icnadv.w() - BORDERWIDTH;
    srcrt.y = 0;
    srcrt.w = BORDERWIDTH;
    srcrt.h = 255;
    dstpt.x = display.w() - BORDERWIDTH;
    dstpt.y = srcrt.y;
    display.Blit(icnadv, srcrt, dstpt);
    srcrt.y = 255;
    srcrt.h = TILEWIDTH;
    dstpt.x = display.w() - BORDERWIDTH;
    dstpt.y = srcrt.y;
    for(u8 ii = 0; ii < count_h + 1; ++ii)
    {
        display.Blit(icnadv, srcrt, dstpt);
        dstpt.y += TILEWIDTH;
    }
    srcrt.y += TILEWIDTH;
    srcrt.h = icnadv.h() - srcrt.y;
    display.Blit(icnadv, srcrt, dstpt);

    // BOTTOM BORDER
    srcrt.x = 0;
    srcrt.y = icnadv.h() - BORDERWIDTH;
    srcrt.w = 223;
    srcrt.h = BORDERWIDTH;
    dstpt.x = srcrt.x;
    dstpt.y = display.h() - BORDERWIDTH;
    display.Blit(icnadv, srcrt, dstpt);
    srcrt.x = 223;
    srcrt.w = TILEWIDTH;
    dstpt.x = srcrt.x;
    dstpt.y = display.h() - BORDERWIDTH;
    for(u8 ii = 0; ii < count_w + 1; ++ii)
    {
        display.Blit(icnadv, srcrt, dstpt);
	dstpt.x += TILEWIDTH;
    }
    srcrt.x += TILEWIDTH;
    srcrt.w = icnadv.w() - srcrt.x;
    display.Blit(icnadv, srcrt, dstpt);

    // ICON BORDER
    srcrt.x = icnadv.w() - RADARWIDTH - BORDERWIDTH;
    srcrt.y = RADARWIDTH + BORDERWIDTH;
    srcrt.w = RADARWIDTH;
    srcrt.h = BORDERWIDTH;
    dstpt.x = display.w() - RADARWIDTH - BORDERWIDTH;
    dstpt.y = srcrt.y;
    display.Blit(icnadv, srcrt, dstpt);
    dstpt.y = srcrt.y + BORDERWIDTH + count_icons * 32;
    srcrt.y = srcrt.y + BORDERWIDTH + 4 * 32;
    display.Blit(icnadv, srcrt, dstpt);
}

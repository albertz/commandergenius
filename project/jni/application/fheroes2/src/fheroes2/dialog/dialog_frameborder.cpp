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
#include "settings.h"
#include "cursor.h"
#include "dialog.h"

#define  LIMITWIDTH BORDERWIDTH * 4
#define  ANGLEWIDTH 44

void DrawBorder(s16, s16, u16, u16, Surface &);

Dialog::FrameBorder::FrameBorder()
{
}

bool Dialog::FrameBorder::isValid(void) const
{
    return Background::isValid();
}

void Dialog::FrameBorder::SetSize(u16 encw, u16 ench)
{
    Display & display = Display::Get();
    if(display.w() < encw || display.h() < ench || encw < LIMITWIDTH || ench < LIMITWIDTH)
    DEBUG(DBG_GAME , DBG_WARN, "Dialog::FrameBorder: size out of range");

    Rect::w = encw + 2 * BORDERWIDTH;
    Rect::h = ench + 2 * BORDERWIDTH;
    area.w = encw;
    area.h = ench;
}

void Dialog::FrameBorder::SetPosition(s16 posx, s16 posy, u16 encw, u16 ench)
{
    if(Background::isValid()) Background::Restore();

    if(encw && ench)
    {
    	Background::Save(posx, posy, encw + 2 * BORDERWIDTH, ench + 2 * BORDERWIDTH);
	area.w = encw;
	area.h = ench;
    }
    else
    	Background::Save(posx, posy);

    area.x = posx + BORDERWIDTH;
    area.y = posy + BORDERWIDTH;
    top = Rect(posx, posy, area.w, BORDERWIDTH);
}

const Rect & Dialog::FrameBorder::GetTop(void) const
{
    return top;
}

const Rect & Dialog::FrameBorder::GetRect(void) const
{
    return Background::GetRect();
}

const Rect & Dialog::FrameBorder::GetArea(void) const
{
    return area;
}

void Dialog::FrameBorder::Redraw(void)
{
    DrawBorder(GetRect().x, GetRect().y, GetRect().w, GetRect().h, Display::Get());
}

Dialog::FrameBorder::~FrameBorder()
{
    if(Cursor::Get().isVisible()){ Cursor::Get().Hide(); };
    Background::Restore();
}

void DrawBorder(s16 posx, s16 posy, u16 posw, u16 posh, Surface & sf)
{
    const Sprite & surdbkg = (Settings::Get().EvilInterface() ? AGG::GetICN(ICN::SURDRBKE, 0) : AGG::GetICN(ICN::SURDRBKG, 0));
    Rect  src_rt;
    Point dst_pt;

    // top left angle
    src_rt = Rect(SHADOWWIDTH, 0, ANGLEWIDTH, BORDERWIDTH);
    dst_pt = Point(posx, posy);
    sf.Blit(surdbkg, src_rt, dst_pt);

    // top bar
    src_rt = Rect(SHADOWWIDTH + ANGLEWIDTH + 20, 0, BORDERWIDTH * 2, BORDERWIDTH);
    dst_pt = Point(posx + ANGLEWIDTH, posy);
    while(dst_pt.x < posx + posw - BORDERWIDTH * 2)
    {
	sf.Blit(surdbkg, src_rt, dst_pt);
	dst_pt.x += src_rt.w;
    }

    // top right angle
    src_rt = Rect(surdbkg.w() - ANGLEWIDTH, 0, ANGLEWIDTH, BORDERWIDTH);
    dst_pt = Point(posx + posw - src_rt.w, posy);
    sf.Blit(surdbkg, src_rt, dst_pt);

    // bottom left angle
    src_rt = Rect(SHADOWWIDTH, surdbkg.h() - SHADOWWIDTH - BORDERWIDTH, ANGLEWIDTH, BORDERWIDTH);
    dst_pt = Point(posx, posy + posh - src_rt.h);
    sf.Blit(surdbkg, src_rt, dst_pt);

    // bottom bar
    src_rt = Rect(SHADOWWIDTH + ANGLEWIDTH, surdbkg.h() - SHADOWWIDTH - BORDERWIDTH, BORDERWIDTH * 2, BORDERWIDTH);
    dst_pt = Point(posx + ANGLEWIDTH, posy + posh - src_rt.h);
    while(dst_pt.x < posx + posw - BORDERWIDTH * 2)
    {
        sf.Blit(surdbkg, src_rt, dst_pt);
        dst_pt.x += src_rt.w;
    }

    // bottom right angle
    src_rt = Rect(surdbkg.w() - ANGLEWIDTH, surdbkg.h() - SHADOWWIDTH - BORDERWIDTH, ANGLEWIDTH, BORDERWIDTH);
    dst_pt = Point(posx + posw - src_rt.w, posy + posh - src_rt.h);
    sf.Blit(surdbkg, src_rt, dst_pt);

    // left top angle
    src_rt = Rect(SHADOWWIDTH, 0, BORDERWIDTH, ANGLEWIDTH);
    dst_pt = Point(posx, posy);
    sf.Blit(surdbkg, src_rt, dst_pt);

    // left bar
    src_rt = Rect(SHADOWWIDTH, ANGLEWIDTH, BORDERWIDTH, BORDERWIDTH * 2);
    dst_pt = Point(posx, posy + ANGLEWIDTH);
    while(dst_pt.y < posy + posh - BORDERWIDTH * 3)
    {
	sf.Blit(surdbkg, src_rt, dst_pt);
	dst_pt.y += src_rt.h;
    }

    // left bottom angle
    src_rt = Rect(SHADOWWIDTH, surdbkg.h() - SHADOWWIDTH - ANGLEWIDTH, BORDERWIDTH, ANGLEWIDTH);
    dst_pt = Point(posx, posy + posh - src_rt.h);
    sf.Blit(surdbkg, src_rt, dst_pt);

    // right top angle
    src_rt = Rect(surdbkg.w() - BORDERWIDTH, 0, BORDERWIDTH, ANGLEWIDTH);
    dst_pt = Point(posx + posw - src_rt.w, posy);
    sf.Blit(surdbkg, src_rt, dst_pt);

    // right bar
    src_rt = Rect(surdbkg.w() - BORDERWIDTH, ANGLEWIDTH, BORDERWIDTH, ANGLEWIDTH);
    dst_pt = Point(posx + posw - src_rt.w, posy + ANGLEWIDTH);
    while(dst_pt.y < posy + posh - BORDERWIDTH * 3)
    {
	sf.Blit(surdbkg, src_rt, dst_pt);
	dst_pt.y += src_rt.h;
    }

    // right bottom angle
    src_rt = Rect(surdbkg.w() - BORDERWIDTH, surdbkg.h() - SHADOWWIDTH - ANGLEWIDTH, BORDERWIDTH, ANGLEWIDTH);
    dst_pt = Point(posx + posw - src_rt.w, posy + posh - src_rt.h);
    sf.Blit(surdbkg, src_rt, dst_pt);
}

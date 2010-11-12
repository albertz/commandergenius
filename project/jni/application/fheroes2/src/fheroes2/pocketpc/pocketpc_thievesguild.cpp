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

#include <string>
#include <algorithm>
#include "agg.h"
#include "button.h"
#include "cursor.h"
#include "settings.h"
#include "text.h"
#include "world.h"
#include "kingdom.h"
#include "castle.h"
#include "pocketpc.h"

// dialog_thievesguild.cpp
struct ValueColors : std::pair<int, int>
{
    ValueColors();
    ValueColors(int v, int c);

    bool IsValue(int v) const;
    bool IsColor(Color::color_t c) const;

    static bool SortValueGreat(const ValueColors & v1, const ValueColors & v2);
};

void GetTownsInfo(std::vector<ValueColors> & v);
void GetCastlesInfo(std::vector<ValueColors> & v);
void GetHeroesInfo(std::vector<ValueColors> & v);
void GetGoldsInfo(std::vector<ValueColors> & v);
void GetWoodOreInfo(std::vector<ValueColors> & v);
void GetGemsCrSlfMerInfo(std::vector<ValueColors> & v);
void GetObelisksInfo(std::vector<ValueColors> & v);
void GetArmyInfo(std::vector<ValueColors> & v);
void GetIncomesInfo(std::vector<ValueColors> & v);
void GetBestHeroArmyInfo(std::vector<ValueColors> & v);
void DrawFlags(const std::vector<ValueColors> & v, const Point & pos, const u16 width, const u8 count);
void DrawHeroIcons(const std::vector<ValueColors> & v, const Point & pos, const u16 width);

void PocketPC::ThievesGuild(bool oracle)
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const u16 window_w = 320;
    const u16 window_h = 224;

    Dialog::FrameBorder frameborder;
    frameborder.SetPosition((display.w() - window_w) / 2 - BORDERWIDTH, (display.h() - window_h) / 2 - BORDERWIDTH, window_w, window_h);
    frameborder.Redraw();

    const Rect & dst_rt = frameborder.GetArea();
    const Sprite & background = AGG::GetICN(ICN::STONEBAK, 0);
    display.Blit(background, Rect(0, 0, window_w, window_h), dst_rt);

    const Point & cur_pt = dst_rt;
    Point dst_pt(cur_pt);

    const u8 count = oracle ? 0xFF : world.GetMyKingdom().GetCountBuilding(BUILD_THIEVESGUILD);

    std::vector<ValueColors> v;
    v.reserve(KINGDOMMAX);
    const u8 colors = Color::Count(Settings::Get().KingdomColors());
    u16 textx = 115;
    u16 startx = 120;
    u16 maxw = 200;
    Text text;
    text.Set(Font::SMALL);

    // head 1
    u8 ii = 0;
    for(ii = 0; ii < colors; ++ii)
    {
	switch(ii+1)
	{
	    case 1: text.Set(_("1st")); break;
	    case 2: text.Set(_("2nd")); break;
	    case 3: text.Set(_("3rd")); break;
	    case 4: text.Set(_("4th")); break;
	    case 5: text.Set(_("5th")); break;
	    case 6: text.Set(_("6th")); break;
	    default: break;
	}

	dst_pt.x = cur_pt.x + startx + maxw / (colors * 2) + ii * maxw / colors - text.w() / 2;
	dst_pt.y = cur_pt.y + 25;
	text.Blit(dst_pt);
    }

    // button exit
    const Rect rectExit(dst_rt.x + dst_rt.w - 26, dst_rt.y + 7, 25, 25);
    display.Blit(AGG::GetICN(ICN::TOWNWIND, 12), rectExit.x, rectExit.y);

    text.Set(_("Number of Towns:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 35;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetTownsInfo(v);
    DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Number of Castles:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 47;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetCastlesInfo(v);
    DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Number of Heroes:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 59;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetHeroesInfo(v);
    DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Gold in Treasury:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 71;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetGoldsInfo(v);
    if(1 < count) DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Wood & Ore:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 83;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetWoodOreInfo(v);
    if(1 < count) DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Gems, Cr, Slf & Mer:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 95;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetGemsCrSlfMerInfo(v);
    if(1 < count) DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Obelisks Found:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 107;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetObelisksInfo(v);
    if(2 < count) DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Total Army Strength:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 119;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetArmyInfo(v);
    if(3 < count) DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Income:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 131;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetIncomesInfo(v);
    if(4 < count) DrawFlags(v, dst_pt, maxw, colors);

    textx = 75;
    startx = 80;
    maxw = 240;

    // head 2
    ii = 0;
    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color)
	if(Settings::Get().KingdomColors() & color)
    {
	text.Set(Color::String(color), Font::SMALL);
	dst_pt.x = cur_pt.x + startx + maxw / (colors * 2) + ii * maxw / colors - text.w() / 2;
	dst_pt.y = cur_pt.y + 145;
	text.Blit(dst_pt);
	++ii;
    }

    text.Set(_("Best Hero:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 160;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetBestHeroArmyInfo(v);
    DrawHeroIcons(v, dst_pt, maxw);

    text.Set(_("Best Hero Stats:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 200;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    //GetBestHeroStatsInfo(v);
    //if(1 < count) DrawHeroIcons(v, dst_pt, maxw);
/*
    text.Set(_("Personality:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 388;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    //GetPersonalityInfo(v);
    //if(2 < count) DrawHeroIcons(v, dst_pt, maxw);

    text.Set(_("Best Monster:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 429;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    //GetBestMonsterInfo(v);
    //if(3 < count) DrawHeroIcons(v, dst_pt, maxw);

    //buttonExit.Draw();
*/
    cursor.Show();
    display.Flip();

    // message loop
    while(le.HandleEvents())
    {
        if(le.MouseClickLeft(rectExit) || HotKeyCloseWindow) break;
    }
}

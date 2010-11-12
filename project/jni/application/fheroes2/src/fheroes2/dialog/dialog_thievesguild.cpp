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
#include "dialog.h"

struct ValueColors : std::pair<int, int>
{
    ValueColors() : std::pair<int, int>(0, 0){};
    ValueColors(int v, int c) : std::pair<int, int>(v, c){};

    bool IsValue(int v) const { return v == first; };
    bool IsColor(Color::color_t c) const { return (c & second); };

    static bool SortValueGreat(const ValueColors & v1, const ValueColors & v2) { return v1.first > v2.first; };
};

void GetTownsInfo(std::vector<ValueColors> & v)
{
    v.clear();

    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color)
        if(Settings::Get().KingdomColors(color))
	{
	    int value = world.GetKingdom(color).GetCountTown();

	    std::vector<ValueColors>::iterator it = 
		std::find_if(v.begin(), v.end(), std::bind2nd(std::mem_fun_ref(&ValueColors::IsValue), value));

	    if(it == v.end())
		v.push_back(ValueColors(value, color));
	    else
		(*it).second |= color;
	}

    std::sort(v.begin(), v.end(), ValueColors::SortValueGreat);
}

void GetCastlesInfo(std::vector<ValueColors> & v)
{
    v.clear();

    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color)
        if(Settings::Get().KingdomColors(color))
	{
	    int value = world.GetKingdom(color).GetCountCastle();

	    std::vector<ValueColors>::iterator it = 
		std::find_if(v.begin(), v.end(), std::bind2nd(std::mem_fun_ref(&ValueColors::IsValue), value));

	    if(it == v.end())
		v.push_back(ValueColors(value, color));
	    else
		(*it).second |= color;
	}

    std::sort(v.begin(), v.end(), ValueColors::SortValueGreat);
}

void GetHeroesInfo(std::vector<ValueColors> & v)
{
    v.clear();

    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color)
        if(Settings::Get().KingdomColors(color))
	{
	    int value = world.GetKingdom(color).GetHeroes().size();

	    std::vector<ValueColors>::iterator it = 
		std::find_if(v.begin(), v.end(), std::bind2nd(std::mem_fun_ref(&ValueColors::IsValue), value));

	    if(it == v.end())
		v.push_back(ValueColors(value, color));
	    else
		(*it).second |= color;
	}

    std::sort(v.begin(), v.end(), ValueColors::SortValueGreat);
}

void GetGoldsInfo(std::vector<ValueColors> & v)
{
    v.clear();

    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color)
        if(Settings::Get().KingdomColors(color))
	{
	    int value = world.GetKingdom(color).GetFundsGold();

	    std::vector<ValueColors>::iterator it = 
		std::find_if(v.begin(), v.end(), std::bind2nd(std::mem_fun_ref(&ValueColors::IsValue), value));

	    if(it == v.end())
		v.push_back(ValueColors(value, color));
	    else
		(*it).second |= color;
	}

    std::sort(v.begin(), v.end(), ValueColors::SortValueGreat);
}

void GetWoodOreInfo(std::vector<ValueColors> & v)
{
    v.clear();

    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color)
        if(Settings::Get().KingdomColors(color))
	{
	    int value = world.GetKingdom(color).GetFundsWood() + world.GetKingdom(color).GetFundsOre();

	    std::vector<ValueColors>::iterator it = 
		std::find_if(v.begin(), v.end(), std::bind2nd(std::mem_fun_ref(&ValueColors::IsValue), value));

	    if(it == v.end())
		v.push_back(ValueColors(value, color));
	    else
		(*it).second |= color;
	}

    std::sort(v.begin(), v.end(), ValueColors::SortValueGreat);
}

void GetGemsCrSlfMerInfo(std::vector<ValueColors> & v)
{
    v.clear();

    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color)
        if(Settings::Get().KingdomColors(color))
	{
	    int value = world.GetKingdom(color).GetFundsGems() +
		world.GetKingdom(color).GetFundsCrystal() +
		world.GetKingdom(color).GetFundsSulfur() +
		world.GetKingdom(color).GetFundsMercury();

	    std::vector<ValueColors>::iterator it = 
		std::find_if(v.begin(), v.end(), std::bind2nd(std::mem_fun_ref(&ValueColors::IsValue), value));

	    if(it == v.end())
		v.push_back(ValueColors(value, color));
	    else
		(*it).second |= color;
	}

    std::sort(v.begin(), v.end(), ValueColors::SortValueGreat);
}

void GetObelisksInfo(std::vector<ValueColors> & v)
{
    v.clear();

    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color)
        if(Settings::Get().KingdomColors(color))
	{
	    int value = world.GetKingdom(color).CountVisitedObjects(MP2::OBJ_OBELISK);

	    std::vector<ValueColors>::iterator it = 
		std::find_if(v.begin(), v.end(), std::bind2nd(std::mem_fun_ref(&ValueColors::IsValue), value));

	    if(it == v.end())
		v.push_back(ValueColors(value, color));
	    else
		(*it).second |= color;
	}

    std::sort(v.begin(), v.end(), ValueColors::SortValueGreat);
}

void GetArmyInfo(std::vector<ValueColors> & v)
{
    v.clear();

    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color)
        if(Settings::Get().KingdomColors(color))
	{
	    int value = world.GetKingdom(color).GetArmiesStrength();

	    std::vector<ValueColors>::iterator it = 
		std::find_if(v.begin(), v.end(), std::bind2nd(std::mem_fun_ref(&ValueColors::IsValue), value));

	    if(it == v.end())
		v.push_back(ValueColors(value, color));
	    else
		(*it).second |= color;
	}

    std::sort(v.begin(), v.end(), ValueColors::SortValueGreat);
}

void GetIncomesInfo(std::vector<ValueColors> & v)
{
    v.clear();

    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color)
        if(Settings::Get().KingdomColors(color))
	{
	    int value = world.GetKingdom(color).GetIncome();

	    std::vector<ValueColors>::iterator it = 
		std::find_if(v.begin(), v.end(), std::bind2nd(std::mem_fun_ref(&ValueColors::IsValue), value));

	    if(it == v.end())
		v.push_back(ValueColors(value, color));
	    else
		(*it).second |= color;
	}

    std::sort(v.begin(), v.end(), ValueColors::SortValueGreat);
}

void GetBestHeroArmyInfo(std::vector<ValueColors> & v)
{
    v.clear();

    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color)
	if(Settings::Get().KingdomColors() & color)
    {
	const Heroes* hero = world.GetKingdom(color).GetBestHero();
	v.push_back(ValueColors(hero ? hero->GetID() : Heroes::UNKNOWN, color));
    }
}

void DrawFlags(const std::vector<ValueColors> & v, const Point & pos, const u16 width, const u8 count)
{
    Display & display = Display::Get();
    const u16 chunk = width / count;
    bool qvga = Settings::Get().QVGA();

    for(u8 ii = 0; ii < count; ++ii)
    {
	if(ii < v.size())
	{
	    const u8 colors = v[ii].second;
	    const u8 items = Color::Count(colors);
	    const u8 sw = qvga ? AGG::GetICN(ICN::MISC6, 7).w() : AGG::GetICN(ICN::FLAG32, 1).w();
	    u16 px = pos.x + chunk / 2 + ii * chunk - (items * sw) / 2;

	    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color) if(colors & color)
	    {
		const Sprite & flag = qvga ?
		    AGG::GetICN(ICN::MISC6, Color::GetIndex(color) + 7) :
		    AGG::GetICN(ICN::FLAG32, Color::GetIndex(color) * 2 + 1);
		display.Blit(flag, px, (qvga ? pos.y + 2 : pos.y));
		px = px + sw;
	    }
	}
    }
}

void DrawHeroIcons(const std::vector<ValueColors> & v, const Point & pos, const u16 width)
{
    if(v.size())
    {
	Display & display = Display::Get();
	const u16 chunk = width / v.size();

	for(u8 ii = 0; ii < v.size(); ++ii)
	{
	    Heroes::heroes_t id = Heroes::ConvertID(v[ii].first);
	    u16 px = pos.x + chunk / 2 + ii * chunk;

	    if(Heroes::UNKNOWN != id)
	    {
		const Sprite & window = AGG::GetICN(ICN::LOCATORS, 22);
		const Surface & icons = world.GetHeroes(id)->GetPortrait30x22();
		display.Blit(window, px - window.w() / 2, pos.y - 4);
		display.Blit(icons, px - icons.w() / 2, pos.y);
	    }
	}
    }
}

void Dialog::ThievesGuild(bool oracle)
{
    if(Settings::Get().QVGA())
    {
	PocketPC::ThievesGuild(oracle);
	return;
    }

    Display & display = Display::Get();

    // cursor
    Cursor & cursor = Cursor::Get();

    cursor.Hide();

    Dialog::FrameBorder frameborder;
    frameborder.SetPosition((display.w() - 640 - BORDERWIDTH * 2) / 2, (display.h() - 480 - BORDERWIDTH * 2) / 2, 640, 480);
    frameborder.Redraw();

    const Point cur_pt(frameborder.GetArea().x, frameborder.GetArea().y);
    Point dst_pt(cur_pt);

    display.Blit(AGG::GetICN(ICN::STONEBAK, 0), dst_pt);

    const u8 count = oracle ? 0xFF : world.GetMyKingdom().GetCountBuilding(BUILD_THIEVESGUILD);

    std::vector<ValueColors> v;
    v.reserve(KINGDOMMAX);
    const u8 colors = Color::Count(Settings::Get().KingdomColors());
    const u16 textx = 185;
    const u16 startx = 210;
    const u16 maxw = 430;
    Text text;

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
	dst_pt.y = cur_pt.y + 5;
	text.Blit(dst_pt);
    }

    // bar
    dst_pt.x = cur_pt.x;
    dst_pt.y = cur_pt.y + 461;
    display.Blit(AGG::GetICN(ICN::WELLXTRA, 2), dst_pt);

    // text bar
    text.Set(_("Thieves' Guild: Player RanKings"), Font::BIG);
    dst_pt.x = cur_pt.x + 280 - text.w() / 2;
    dst_pt.y = cur_pt.y + 461;
    text.Blit(dst_pt);

    // button exit
    dst_pt.x = cur_pt.x + 578;
    dst_pt.y = cur_pt.y + 461;
    Button buttonExit(dst_pt, ICN::WELLXTRA, 0, 1);

    text.Set(_("Number of Towns:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 28;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetTownsInfo(v);
    DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Number of Castles:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 54;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetCastlesInfo(v);
    DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Number of Heroes:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 80;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetHeroesInfo(v);
    DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Gold in Treasury:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 106;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetGoldsInfo(v);
    if(1 < count) DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Wood & Ore:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 132;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetWoodOreInfo(v);
    if(1 < count) DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Gems, Cr, Slf & Mer:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 158;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetGemsCrSlfMerInfo(v);
    if(1 < count) DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Obelisks Found:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 184;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetObelisksInfo(v);
    if(2 < count) DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Total Army Strength:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 210;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetArmyInfo(v);
    if(3 < count) DrawFlags(v, dst_pt, maxw, colors);

    text.Set(_("Income:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 238;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetIncomesInfo(v);
    if(4 < count) DrawFlags(v, dst_pt, maxw, colors);

    // head 2
    ii = 0;
    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color)
	if(Settings::Get().KingdomColors() & color)
    {
	text.Set(Color::String(color));
	dst_pt.x = cur_pt.x + startx + maxw / (colors * 2) + ii * maxw / colors - text.w() / 2;
	dst_pt.y = cur_pt.y + 270;
	text.Blit(dst_pt);
	++ii;
    }

    text.Set(_("Best Hero:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 306;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    GetBestHeroArmyInfo(v);
    DrawHeroIcons(v, dst_pt, maxw);

    text.Set(_("Best Hero Stats:"));
    dst_pt.x = cur_pt.x + textx - text.w();
    dst_pt.y = cur_pt.y + 347;
    text.Blit(dst_pt);

    dst_pt.x = cur_pt.x + startx;
    //GetBestHeroStatsInfo(v);
    //if(1 < count) DrawHeroIcons(v, dst_pt, maxw);

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

    buttonExit.Draw();

    cursor.Show();
    display.Flip();

    LocalEvent & le = LocalEvent::Get();
   
    // message loop
    while(le.HandleEvents())
    {
        le.MousePressLeft(buttonExit) ? buttonExit.PressDraw() : buttonExit.ReleaseDraw();

        if(le.MouseClickLeft(buttonExit) || HotKeyCloseWindow) break;
    }
}

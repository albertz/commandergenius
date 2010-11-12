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
#include "payment.h"
#include "world.h"
#include "button.h"
#include "kingdom.h"
#include "dialog.h"

void RedrawCurrentInfo(const Point & pos, u16 available, u32 result,
	    const payment_t & paymentMonster, const payment_t & paymentCosts)
{
    std::string str;
    Text text;

    str = _("Available: %{count}");
    String::Replace(str, "%{count}", available);
    text.Set(str, Font::SMALL);
    text.Blit(pos.x + 70 - text.w() / 2, pos.y + 130);
    str.clear();
    String::AddInt(str, result);
    text.Set(str, Font::BIG);
    text.Blit(pos.x + 167 - text.w() / 2, pos.y + 160);
    if(paymentMonster.ore ||
       paymentMonster.wood ||
       paymentMonster.mercury ||
       paymentMonster.crystal ||
       paymentMonster.sulfur ||
       paymentMonster.gems)
    {
	str.clear();
	String::AddInt(str, paymentCosts.gold);
	text.Set(str, Font::SMALL);
	text.Blit(pos.x + 133 - text.w() / 2, pos.y + 228);
	str.clear();
	if(paymentMonster.ore) String::AddInt(str, paymentCosts.ore); 
        else
	if(paymentMonster.wood) String::AddInt(str, paymentCosts.wood);
        else
        if(paymentMonster.mercury) String::AddInt(str, paymentCosts.mercury);
        else
        if(paymentMonster.crystal) String::AddInt(str, paymentCosts.crystal);
        else
        if(paymentMonster.sulfur) String::AddInt(str, paymentCosts.sulfur);
        else
        if(paymentMonster.gems) String::AddInt(str, paymentCosts.gems);
	text.Set(str, Font::SMALL);
	text.Blit(pos.x + 195 - text.w() / 2, pos.y + 228);
    }
    else
    {
	str.clear();
	String::AddInt(str, paymentCosts.gold);
	text.Set(str, Font::SMALL);
	text.Blit(pos.x + 160 - text.w() / 2, pos.y + 228);
    }
}

void RedrawResourceInfo(const Surface & sres, const Point & pos, s32 value,
	u8 px1, u8 py1, u8 px2, u8 py2)
{
    Display & display = Display::Get();
    Point dst_pt;
    std::string str;
    Text text;

    dst_pt.x = pos.x + px1;
    dst_pt.y = pos.y + py1;
    display.Blit(sres, dst_pt);

    String::AddInt(str, value);
    text.Set(str, Font::SMALL);

    dst_pt.x = pos.x + px2 - text.w() / 2;
    dst_pt.y = pos.y + py2;
    text.Blit(dst_pt);
}

u16 Dialog::RecruitMonster(const Monster & monster, u16 available)
{
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    // cursor
    Cursor & cursor = Cursor::Get();
    const Cursor::themes_t oldcursor = cursor.Themes();
    cursor.Hide();
    cursor.SetThemes(Cursor::POINTER);
    
    // calculate max count
    u32 max = 0;
    const PaymentConditions::BuyMonster paymentMonster(monster());
    const Resource::funds_t & kingdomResource = world.GetMyKingdom().GetFundsResource();
    while(Resource::funds_t(paymentMonster * max) <= kingdomResource && max <= available) ++max;

    u32 result = --max;

    payment_t paymentCosts(paymentMonster * result);

    const Sprite & box = AGG::GetICN(ICN::RECRBKG, 0);
    const Rect pos((display.w() - box.w()) / 2, Settings::Get().QVGA() ? (display.h() - box.h()) / 2 - 15 : (display.h() - box.h()) / 2, box.w(), box.h());

    Background back(pos);
    back.Save();

    display.Blit(box, pos.x, pos.y);

    Point dst_pt;
    std::string str;
    Text text;

    // smear hardcore text "Cost per troop:"
    const Sprite & smear = AGG::GetICN(ICN::TOWNNAME, 0);
    dst_pt.x = pos.x + 144;
    dst_pt.y = pos.y + 55;
    display.Blit(smear, Rect(8, 1, 120, 12), dst_pt);

    str = _("Cost per troop:");
    text.Set(str, Font::SMALL);
    dst_pt.x = pos.x + 206 - text.w() / 2;
    dst_pt.y = pos.y + 55;
    text.Blit(dst_pt);

    // text recruit monster
    str = _("Recruit %{name}");
    String::Replace(str, "%{name}", monster.GetMultiName());
    text.Set(str, Font::BIG);
    dst_pt.x = pos.x + (pos.w - text.w()) / 2;
    dst_pt.y = pos.y + 25;
    text.Blit(dst_pt);

    // sprite monster
    const Sprite & smon = AGG::GetICN(monster.ICNMonh(), 0);
    dst_pt.x = pos.x + 70 - smon.w() / 2;
    dst_pt.y = pos.y + 130 - smon.h();
    display.Blit(smon, dst_pt);

    // info resource
    // gold
    const Sprite & sgold = AGG::GetICN(ICN::RESOURCE, 6);
    dst_pt.x = pos.x + (paymentMonster.ore ||
                        paymentMonster.wood ||
                        paymentMonster.mercury ||
                        paymentMonster.crystal ||
                        paymentMonster.sulfur ||
                        paymentMonster.gems ? 150 : 175);
    dst_pt.y = pos.y + 75;
    display.Blit(sgold, dst_pt);

    dst_pt.x = pos.x + (paymentMonster.ore ||
                        paymentMonster.wood ||
                        paymentMonster.mercury ||
                        paymentMonster.crystal ||
                        paymentMonster.sulfur ||
                        paymentMonster.gems ? 105 : 130);
    dst_pt.y = pos.y + 200;
    display.Blit(sgold, dst_pt);

    str.clear();
    String::AddInt(str, paymentMonster.gold);
    text.Set(str, Font::SMALL);
    dst_pt.x = pos.x + (paymentMonster.ore ||
                        paymentMonster.wood ||
                        paymentMonster.mercury ||
                        paymentMonster.crystal ||
                        paymentMonster.sulfur ||
                        paymentMonster.gems ? 183 : 205) - text.w() / 2;
    dst_pt.y = pos.y + 103;
    text.Blit(dst_pt);

    // crystal
    if(paymentMonster.crystal)
    {
        const Sprite & sres = AGG::GetICN(ICN::RESOURCE, 4);
	RedrawResourceInfo(sres, pos, paymentMonster.crystal,
				225, 75, 240, 103);
	dst_pt.x = pos.x + 180;
	dst_pt.y = pos.y + 200;
	display.Blit(sres, dst_pt);
    }
    else
    // mercury
    if(paymentMonster.mercury)
    {
        const Sprite & sres = AGG::GetICN(ICN::RESOURCE, 1);
	RedrawResourceInfo(sres, pos, paymentMonster.mercury,
				225, 72, 240, 103);
	dst_pt.x = pos.x + 180;
	dst_pt.y = pos.y + 197;
	display.Blit(sres, dst_pt);
    }
    else
    // wood
    if(paymentMonster.wood)
    {
        const Sprite & sres = AGG::GetICN(ICN::RESOURCE, 0);
	RedrawResourceInfo(sres, pos, paymentMonster.wood,
				225, 72, 240, 103);
	dst_pt.x = pos.x + 180;
	dst_pt.y = pos.y + 197;
	display.Blit(sres, dst_pt);
    }
    else
    // ore
    if(paymentMonster.ore)
    {
        const Sprite & sres = AGG::GetICN(ICN::RESOURCE, 2);
	RedrawResourceInfo(sres, pos, paymentMonster.ore,
				225, 72, 240, 103);
	dst_pt.x = pos.x + 180;
	dst_pt.y = pos.y + 197;
	display.Blit(sres, dst_pt);
    }
    else
    // sulfur
    if(paymentMonster.sulfur)
    {
        const Sprite & sres = AGG::GetICN(ICN::RESOURCE, 3);
	RedrawResourceInfo(sres, pos, paymentMonster.sulfur,
				225, 75, 240, 103);
	dst_pt.x = pos.x + 180;
	dst_pt.y = pos.y + 200;
	display.Blit(sres, dst_pt);
    }
    else
    // gems
    if(paymentMonster.gems)
    {
        const Sprite & sres = AGG::GetICN(ICN::RESOURCE, 5);
	RedrawResourceInfo(sres, pos, paymentMonster.gems,
				225, 75, 240, 103);
	dst_pt.x = pos.x + 180;
	dst_pt.y = pos.y + 200;
	display.Blit(sres, dst_pt);
    }

    // text number buy
    text.Set(_("Number to buy:"));
    dst_pt.x = pos.x + 30;
    dst_pt.y = pos.y + 163;
    text.Blit(dst_pt);

    Background static_info(Rect(pos.x + 16, pos.y + 125, pos.w - 32, 122));
    static_info.Save();

    RedrawCurrentInfo(pos, available, result, paymentMonster, paymentCosts);

    // buttons
    dst_pt.x = pos.x + 34;
    dst_pt.y = pos.y + 249;
    Button buttonOk(dst_pt, ICN::RECRUIT, 8, 9);

    dst_pt.x = pos.x + 187;
    dst_pt.y = pos.y + 249;
    Button buttonCancel(dst_pt, ICN::RECRUIT, 6, 7);

    dst_pt.x = pos.x + 230;
    dst_pt.y = pos.y + 155;
    Button buttonMax(dst_pt, ICN::RECRUIT, 4, 5);
    dst_pt.x = pos.x + 208;
    dst_pt.y = pos.y + 156;
    Button buttonUp(dst_pt, ICN::RECRUIT, 0, 1);

    dst_pt.x = pos.x + 208;
    dst_pt.y = pos.y + 171;
    Button buttonDn(dst_pt, ICN::RECRUIT, 2, 3);

    buttonOk.Draw();
    buttonCancel.Draw();
    buttonMax.Draw();
    buttonUp.Draw();
    buttonDn.Draw();

    cursor.Show();
    display.Flip();

    bool redraw = false;

    // str loop
    while(le.HandleEvents())
    {
	le.MousePressLeft(buttonOk) ? buttonOk.PressDraw() : buttonOk.ReleaseDraw();
	le.MousePressLeft(buttonCancel) ? buttonCancel.PressDraw() : buttonCancel.ReleaseDraw();
	le.MousePressLeft(buttonMax) ? buttonMax.PressDraw() : buttonMax.ReleaseDraw();
	le.MousePressLeft(buttonUp) ? buttonUp.PressDraw() : buttonUp.ReleaseDraw();
	le.MousePressLeft(buttonDn) ? buttonDn.PressDraw() : buttonDn.ReleaseDraw();

	if(PressIntKey(0, max, result))
	{
	    paymentCosts = paymentMonster * result;
	    redraw = true;
	}

	if(le.MouseClickLeft(buttonUp) && result < max)
	{
	    ++result;
	    paymentCosts += paymentMonster;
	    redraw = true;
	}
	else
	if(le.MouseClickLeft(buttonDn) && result)
	{
	    --result;
	    paymentCosts -= paymentMonster;
	    redraw = true;
	}
	else
	if(le.MouseClickLeft(buttonMax) && result != max)
	{
	    result = max;
	    paymentCosts = paymentMonster * max;
	    redraw = true;
	}

	if(redraw)
	{
	    cursor.Hide();
	    static_info.Restore();
	    RedrawCurrentInfo(pos, available, result, paymentMonster, paymentCosts);
	    cursor.Show();
	    display.Flip();
	    redraw = false;
	}

	if(le.MouseClickLeft(buttonOk) || Game::HotKeyPress(Game::EVENT_DEFAULT_READY)) break;
	
	if(le.MouseClickLeft(buttonCancel) || Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT)){ result = 0; break; }
    }

    cursor.Hide();

    back.Restore();
    cursor.SetThemes(oldcursor);

    cursor.Show();
    display.Flip();

    return result;
}

void Dialog::DwellingInfo(const Monster & monster, u16 available)
{
    Display & display = Display::Get();

    // cursor
    Cursor & cursor = Cursor::Get();
    const Cursor::themes_t oldcursor = cursor.Themes();
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);
    
    const PaymentConditions::BuyMonster paymentMonster(monster());

    const Sprite & box = AGG::GetICN(ICN::RECR2BKG, 0);
    const Rect pos((display.w() - box.w()) / 2, (display.h() - box.h()) / 2, box.w(), box.h());

    Background back(pos);
    back.Save();

    display.Blit(box, pos.x, pos.y);
    
    LocalEvent & le = LocalEvent::Get();

    Point dst_pt;
    Text text;
    std::string str;

    // text recruit monster
    str = _("Recruit %{name}");
    String::Replace(str, "%{name}", monster.GetMultiName());
    text.Set(str, Font::BIG);
    text.Blit(pos.x + (pos.w - text.w()) / 2, pos.y + 25);

    // sprite monster
    const Sprite & smon = AGG::GetICN(monster.ICNMonh(), 0);
    dst_pt.x = pos.x + 70 - smon.w() / 2;
    dst_pt.y = pos.y + 120 - smon.h();
    display.Blit(smon, dst_pt);

    // info resource
    // gold
    const Sprite & sgold = AGG::GetICN(ICN::RESOURCE, 6);
    dst_pt.x = pos.x + (paymentMonster.ore ||
                        paymentMonster.wood ||
                        paymentMonster.mercury ||
                        paymentMonster.crystal ||
                        paymentMonster.sulfur ||
                        paymentMonster.gems ? 150 : 175);
    dst_pt.y = pos.y + 75;
    display.Blit(sgold, dst_pt);

    str.clear();
    String::AddInt(str, paymentMonster.gold);
    text.Set(str, Font::SMALL);
    dst_pt.x = pos.x + (paymentMonster.ore ||
                        paymentMonster.wood ||
                        paymentMonster.mercury ||
                        paymentMonster.crystal ||
                        paymentMonster.sulfur ||
                        paymentMonster.gems ? 183 : 205) - text.w() / 2;
    dst_pt.y = pos.y + 103;
    text.Blit(dst_pt);
    // crystal
    if(paymentMonster.crystal)
    {
	const Sprite & sres = AGG::GetICN(ICN::RESOURCE, 4);
	RedrawResourceInfo(sres, pos, paymentMonster.crystal,
				225, 75, 240, 103);
    }
    else
    // mercury
    if(paymentMonster.mercury)
    {
	const Sprite & sres = AGG::GetICN(ICN::RESOURCE, 1);
	RedrawResourceInfo(sres, pos, paymentMonster.mercury,
				225, 72, 240, 103);
    }
    else
    // wood
    if(paymentMonster.wood)
    {
	const Sprite & sres = AGG::GetICN(ICN::RESOURCE, 0);
	RedrawResourceInfo(sres, pos, paymentMonster.wood,
				225, 72, 240, 103);
    }
    else
    // ore
    if(paymentMonster.ore)
    {
	const Sprite & sres = AGG::GetICN(ICN::RESOURCE, 2);
	RedrawResourceInfo(sres, pos, paymentMonster.ore,
				225, 72, 240, 103);
    }
    else
    // sulfur
    if(paymentMonster.sulfur)
    {
	const Sprite & sres = AGG::GetICN(ICN::RESOURCE, 3);
	RedrawResourceInfo(sres, pos, paymentMonster.sulfur,
				225, 75, 240, 103);
    }
    else
    // gems
    if(paymentMonster.gems)
    {
	const Sprite & sres = AGG::GetICN(ICN::RESOURCE, 5);
	RedrawResourceInfo(sres, pos, paymentMonster.gems,
				225, 75, 240, 103);
    }

    // text available
    str = _("Available: %{count}");
    String::Replace(str, "%{count}", available);
    text.Set(str);
    text.Blit(pos.x + 70 - text.w() / 2, pos.y + 130);

    cursor.Show();
    display.Flip();

    //
    while(le.HandleEvents() && le.MousePressRight());

    cursor.Hide();

    back.Restore();
    cursor.SetThemes(oldcursor);

    cursor.Show();
    display.Flip();
}

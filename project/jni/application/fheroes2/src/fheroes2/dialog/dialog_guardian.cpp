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
#include "heroes.h"
#include "portrait.h"
#include "button.h"
#include "selectarmybar.h"
#include "heroes_indicator.h"
#include "army_troop.h"
#include "dialog.h"

class ArmyCell : public Rect
{
public:
    ArmyCell(const Army::Troop & t, const Surface & bg, const Surface & cr, const Point & pt, const bool & ro)
	: Rect(pt.x, pt.y, bg.w(), bg.h()), select(false), troop(t), back(bg), curs(cr), readonly(ro) {};

    void Redraw(void)
    {
	Display & display = Display::Get();

	display.Blit(back, *this);
	if(troop.isValid())
	{
	    const Sprite & mons32 = AGG::GetICN(ICN::MONS32, Monster::GetSpriteIndex(troop()));
	    display.Blit(mons32, x + (back.w() - mons32.w()) / 2, y + back.h() - mons32.h() - 11);

	    if(readonly)
		display.Blit(AGG::GetICN(ICN::LOCATORS, 24), x + 33, y + 5);

	    std::string str;
	    String::AddInt(str, troop.GetCount());
	    Text text(str, Font::SMALL);
	    text.Blit(x + (back.w() - text.w()) / 2, y + back.h() - 11);
	}

	if(select)
	    display.Blit(curs, *this);
    };

    bool select;
    const Army::Troop & troop;
    const Surface & back;
    const Surface & curs;
    const bool & readonly;
};

bool Dialog::SetGuardian(Heroes & hero, Army::Troop & troop, bool readonly)
{
    Display & display = Display::Get();
    //const Settings & conf = Settings::Get();
    LocalEvent & le = LocalEvent::Get();

    // cursor
    Cursor & cursor = Cursor::Get();
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const u16 window_w = 230;
    const u16 window_h = 160;

    Dialog::FrameBorder frameborder;
    frameborder.SetPosition((display.w() - window_w) / 2 - BORDERWIDTH, (display.h() - window_h) / 2 - BORDERWIDTH, window_w, window_h);
    frameborder.Redraw();

    Point dst_pt;
    const Rect & area = frameborder.GetArea();
    const Sprite & background = AGG::GetICN(ICN::STONEBAK, 0);
    display.Blit(background, Rect(0, 0, window_w, window_h), area);

    // portrait
    const Sprite & window = AGG::GetICN(ICN::BRCREST, 6);
    dst_pt.x = area.x + 3;
    dst_pt.y = area.y + 5;
    display.Blit(window, dst_pt);
    display.Blit(Portrait::Get(hero, Portrait::MEDIUM), dst_pt.x + 4, dst_pt.y + 4);

    // indicators
    dst_pt.x = area.x + 185;
    dst_pt.y = area.y + 5;
    MoraleIndicator moraleIndicator(hero);
    moraleIndicator.SetPos(dst_pt);
    moraleIndicator.Redraw();

    dst_pt.x = area.x + 185;
    dst_pt.y = area.y + 35;
    LuckIndicator luckIndicator(hero);
    luckIndicator.SetPos(dst_pt);
    luckIndicator.Redraw();

    // army bar
    dst_pt.x = area.x + 3;
    dst_pt.y = area.y + 73;

    const Sprite &backSprite = AGG::GetICN(ICN::SWAPWIN, 0);
    const Rect rt(36, 267, 43, 53);
    Surface sfb(rt.w, rt.h);
    sfb.Blit(backSprite, rt, 0, 0);
    Surface sfc(rt.w, rt.h - 10);
    Cursor::DrawCursor(sfc, 0x10, true);

    SelectArmyBar selectArmy;
    selectArmy.SetArmy(hero.GetArmy());
    selectArmy.SetPos(dst_pt);
    selectArmy.SetInterval(2);
    selectArmy.SetBackgroundSprite(sfb);
    selectArmy.SetCursorSprite(sfc);
    selectArmy.SetUseMons32Sprite();
    selectArmy.SetSaveLastTroop();
    selectArmy.Redraw();

    // guardian
    dst_pt.x = area.x + 93;
    dst_pt.y = area.y + 17;
    ArmyCell guardian(troop, sfb, sfc, dst_pt, readonly);
    guardian.Redraw();

    // label
    Text text(_("Set Guardian"), Font::SMALL);
    text.Blit(area.x + (area.w - text.w()) / 2, area.y + 3);

    ButtonGroups btnGroups(area, Dialog::OK);
    btnGroups.Draw();

    const Army::Troop shadow(troop);

    cursor.Show();
    display.Flip();

    // message loop
    u16 buttons = Dialog::ZERO;

    while(buttons == Dialog::ZERO && le.HandleEvents())
    {
	buttons = btnGroups.QueueEventProcessing();

        if(le.MouseCursor(selectArmy.GetArea()))
	{
    	    if(guardian.select && le.MouseClickLeft(selectArmy.GetArea()))
	    {
		Army::Troop & troop1 = hero.GetArmy().At(selectArmy.GetIndexFromCoord(le.GetMouseCursor()));

        	// combine
        	if(troop1() == troop())
        	{
            	    troop1.SetCount(troop.GetCount() + troop1.GetCount());
            	    troop.Reset();
		}
		else
		if(troop1.GetCount() >= MAXU16)
		    Dialog::Message("", _("Your army too big!"), Font::BIG, Dialog::OK);
		// swap
		else
		{
		    Army::SwapTroops(troop1, troop);
		}

		guardian.select = false;
		cursor.Hide();
	    }
    	    else
	    if(SelectArmyBar::QueueEventProcessing(selectArmy))
	    {
		guardian.select = false;
		cursor.Hide();
	    }
	}
	else
        if(le.MouseCursor(moraleIndicator.GetArea()))
	    MoraleIndicator::QueueEventProcessing(moraleIndicator);
	else
        if(le.MouseCursor(luckIndicator.GetArea()))
	    LuckIndicator::QueueEventProcessing(luckIndicator);
	else
        if(le.MouseClickLeft(guardian))
	{
	    if(guardian.select)
	    {
		Dialog::ArmyInfo(troop, Dialog::READONLY | Dialog::BUTTONS);
		cursor.Hide();
	    }
	    else
	    if(selectArmy.isSelected() && !readonly && !selectArmy.SaveLastTroop())
	    {
		Army::Troop & troop1 = hero.GetArmy().At(selectArmy.Selected());

        	// combine
        	if(troop1() == troop())
        	{
		    if(troop1.GetCount() + troop.GetCount() < MAXU16)
		    {
            		troop.SetCount(troop1.GetCount() + troop.GetCount());
            		troop1.Reset();
		    }
		    else
		    {
			troop1.SetCount(troop1.GetCount() + troop.GetCount() - MAXU16);
            		troop.SetCount(MAXU16);
		    }
		}
		else
		if(troop1.GetCount() >= MAXU16)
		    Dialog::Message("", _("Your army too big!"), Font::BIG, Dialog::OK);
		// swap
		else
		{
		    Army::SwapTroops(troop1, troop);
		}

		selectArmy.Reset();
		cursor.Hide();
	    }
	    else
	    // select
	    if(troop.isValid() && !readonly)
	    {
		selectArmy.Reset();
		guardian.select = true;
		cursor.Hide();
	    }
	}
	else
        if(le.MousePressRight(guardian) && troop.isValid())
	{
	    selectArmy.Reset();
	    Dialog::ArmyInfo(troop, 0);
	    cursor.Hide();
	}
	
	if(!cursor.isVisible())
	{
	    guardian.Redraw();
	    moraleIndicator.Redraw();
	    luckIndicator.Redraw();
	    selectArmy.Redraw();
	    cursor.Show();
	    display.Flip();
	}
    }

    return shadow() != troop() || shadow.GetCount() != troop.GetCount();
}

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
#include "text.h"
#include "button.h"
#include "heroes.h"
#include "heroes_indicator.h"
#include "selectarmybar.h"
#include "selectartifactbar.h"
#include "world.h"
#include "kingdom.h"
#include "pocketpc.h"

void PocketPC::HeroesMeeting(Heroes & hero1, Heroes & hero2)
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const u16 window_w = 320;
    const u16 window_h = 236;

    Dialog::FrameBorder frameborder;
    frameborder.SetPosition((display.w() - window_w) / 2 - BORDERWIDTH, (display.h() - window_h) / 2 - BORDERWIDTH, window_w, window_h);
    frameborder.Redraw();

    const Rect & dst_rt = frameborder.GetArea();
    const Sprite & background = AGG::GetICN(ICN::STONEBAK, 0);
    const Sprite & backSprite = AGG::GetICN(ICN::SWAPWIN, 0);
    display.Blit(background, Rect(0, 0, window_w, window_h), dst_rt);

    // portrait
    display.Blit(AGG::GetICN(ICN::BRCREST, 6), dst_rt.x + 4, dst_rt.y + 4);
    display.Blit(hero1.GetPortrait50x46(), dst_rt.x + 8, dst_rt.y + 8);

    display.Blit(AGG::GetICN(ICN::BRCREST, 6), dst_rt.x + 4, dst_rt.y + 118);
    display.Blit(hero2.GetPortrait50x46(), dst_rt.x + 8, dst_rt.y + 122);

    // art bar
    const Rect rt2(23, 347, 34, 34);
    Surface sfb2(rt2.w, rt2.h);
    sfb2.Blit(backSprite, rt2, 0, 0);
    Surface sfc2(rt2.w, rt2.h);
    Cursor::DrawCursor(sfc2, 0xd6, true);

    SelectArtifactsBar selectArtifacts1(hero1);
    selectArtifacts1.SetPos(dst_rt.x + 68, dst_rt.y + 2);
    selectArtifacts1.SetInterval(2);
    selectArtifacts1.SetBackgroundSprite(sfb2);
    selectArtifacts1.SetCursorSprite(sfc2);
    selectArtifacts1.SetUseArts32Sprite();
    selectArtifacts1.Redraw();

    SelectArtifactsBar selectArtifacts2(hero2);
    selectArtifacts2.SetPos(dst_rt.x + 68, dst_rt.y + 164);
    selectArtifacts2.SetInterval(2);
    selectArtifacts2.SetBackgroundSprite(sfb2);
    selectArtifacts2.SetCursorSprite(sfc2);
    selectArtifacts2.SetUseArts32Sprite();
    selectArtifacts2.Redraw();

    // army bar
    const Rect rt1(36, 267, 43, 43);
    Surface sfb1(rt1.w, rt1.h);
    sfb1.Blit(backSprite, rt1, 0, 0);
    Surface sfc1(rt1.w, rt1.h);
    Cursor::DrawCursor(sfc1, 0xd6, true);

    SelectArmyBar selectArmy1;
    selectArmy1.SetArmy(hero1.GetArmy());
    selectArmy1.SetPos(dst_rt.x + 68, dst_rt.y + 74);
    selectArmy1.SetInterval(2);
    selectArmy1.SetBackgroundSprite(sfb1);
    selectArmy1.SetCursorSprite(sfc1);
    selectArmy1.SetUseMons32Sprite();
    selectArmy1.SetCount2Sprite();
    selectArmy1.SetSaveLastTroop();
    selectArmy1.Redraw();

    SelectArmyBar selectArmy2;
    selectArmy2.SetArmy(hero2.GetArmy());
    selectArmy2.SetPos(dst_rt.x + 68, dst_rt.y + 119);
    selectArmy2.SetInterval(2);
    selectArmy2.SetBackgroundSprite(sfb1);
    selectArmy2.SetCursorSprite(sfc1);
    selectArmy2.SetUseMons32Sprite();
    selectArmy2.SetCount2Sprite();
    selectArmy2.SetSaveLastTroop();
    selectArmy2.Redraw();

    const Rect rectExit(dst_rt.x + dst_rt.w - 25, dst_rt.y + (dst_rt.h - 25) / 2, 25, 25);
    display.Blit(AGG::GetICN(ICN::TOWNWIND, 12), rectExit.x + 4, rectExit.y + 4);

    cursor.Show();
    display.Flip();

    while(le.HandleEvents())
    {
        // exit
        if(le.MouseClickLeft(rectExit) || HotKeyCloseWindow) break;

	if(SelectArmyBar::QueueEventProcessing(selectArmy1, selectArmy2))
	{
    	    if(selectArtifacts1.isSelected()) selectArtifacts1.Reset();
            else
            if(selectArtifacts2.isSelected()) selectArtifacts2.Reset();
	}

	if(SelectArtifactsBar::QueueEventProcessing(selectArtifacts1, selectArtifacts2))
        {
    	    if(selectArmy1.isSelected()) selectArmy1.Reset();
            else
            if(selectArmy2.isSelected()) selectArmy2.Reset();
    	}
    }
}

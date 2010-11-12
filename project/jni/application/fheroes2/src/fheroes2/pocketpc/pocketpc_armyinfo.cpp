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
#include "text.h"
#include "button.h"
#include "army.h"
#include "selectarmybar.h"
#include "army.h"
#include "battle_stats.h"
#include "pocketpc.h"

void DrawMonsterStats(const Point &, const Army::Troop &);
void DrawBattleStats(const Point &, const Battle2::Stats &);

Dialog::answer_t PocketPC::DialogArmyInfo(const Army::Troop & troop, u16 flags)
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

    const Monster & mons = troop;
    const Battle2::Stats* battle = troop.GetBattleStats();

    const Rect & dst_rt = frameborder.GetArea();
    const Sprite & background = AGG::GetICN(ICN::STONEBAK, 0);
    display.Blit(background, Rect(0, 0, window_w, window_h), dst_rt);

    // name
    Text text;
    text.Set(mons.GetName(), Font::BIG);
    text.Blit(dst_rt.x + (dst_rt.w - text.w()) / 2, dst_rt.y + 10);

    const Sprite & frame = AGG::GetICN(troop.ICNMonh(), 0);
    display.Blit(frame, dst_rt.x + 50 - frame.w() / 2, dst_rt.y + 145 - frame.h());

    std::string message;
    String::AddInt(message, (battle ? battle->GetCount() : troop.GetCount()));
    text.Set(message);
    text.Blit(dst_rt.x + 50 - text.w() / 2, dst_rt.y + 150);

    // stats
    DrawMonsterStats(Point(dst_rt.x + 200, dst_rt.y + 40), troop);

    if(battle)
        DrawBattleStats(Point(dst_rt.x + 160, dst_rt.y + 160), *battle);

    Button buttonDismiss(dst_rt.x + dst_rt.w / 2 - 160, dst_rt.y + dst_rt.h - 30, ICN::VIEWARMY, 1, 2);
    Button buttonUpgrade(dst_rt.x + dst_rt.w / 2 - 60, dst_rt.y + dst_rt.h - 30, ICN::VIEWARMY, 5, 6);
    Button buttonExit(dst_rt.x + dst_rt.w / 2 + 60, dst_rt.y + dst_rt.h - 30, ICN::VIEWARMY, 3, 4);

    if(Dialog::READONLY & flags)
    {
        buttonDismiss.Press();
        buttonDismiss.SetDisable(true);
    }

    if(!(Dialog::BATTLE & flags) && mons.isAllowUpgrade())
    {
	if(Dialog::UPGRADE & flags)
        {
            buttonUpgrade.SetDisable(false);
            buttonUpgrade.Draw();
        }
        else if(Dialog::READONLY & flags)
        {
            buttonUpgrade.Press();
            buttonUpgrade.SetDisable(true);
    	    buttonUpgrade.Draw();
        }
        else buttonUpgrade.SetDisable(true);
    }
    else buttonUpgrade.SetDisable(true);

    if(!(Dialog::BATTLE & flags))
    {
	buttonDismiss.Draw();
        buttonExit.Draw();
    }

    cursor.Show();
    display.Flip();

    while(le.HandleEvents())
    {
        if(buttonUpgrade.isEnable()) le.MousePressLeft(buttonUpgrade) ? (buttonUpgrade).PressDraw() : (buttonUpgrade).ReleaseDraw();
        if(buttonDismiss.isEnable()) le.MousePressLeft(buttonDismiss) ? (buttonDismiss).PressDraw() : (buttonDismiss).ReleaseDraw();
        le.MousePressLeft(buttonExit) ? (buttonExit).PressDraw() : (buttonExit).ReleaseDraw();

        if(buttonUpgrade.isEnable() && le.MouseClickLeft(buttonUpgrade)) return Dialog::UPGRADE;
        else
        if(buttonDismiss.isEnable() && le.MouseClickLeft(buttonDismiss)) return Dialog::DISMISS;
        else
        if(le.MouseClickLeft(buttonExit) || Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT)) return Dialog::CANCEL;
    }

    return Dialog::ZERO;
}

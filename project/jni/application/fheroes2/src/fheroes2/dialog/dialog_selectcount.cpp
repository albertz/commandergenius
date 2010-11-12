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
#include "button.h"
#include "pocketpc.h"
#include "dialog.h"

bool Dialog::SelectCount(const std::string &header, u32 min, u32 max, u32 & cur)
{
    Display & display = Display::Get();

    if(min >= max) min = 0;
    if(cur > max || cur < min) cur = min;

    const ICN::icn_t system = Settings::Get().EvilInterface() ? ICN::SYSTEME : ICN::SYSTEM;

    // preload
    AGG::PreloadObject(system);

    // cursor
    Cursor & cursor = Cursor::Get();
    cursor.Hide();

    Text text(header, Font::BIG);
    const u8 spacer = Settings::Get().QVGA() ? 5 : 10;

    Box box(text.h() + spacer + 30, true);

    const Rect & pos = box.GetArea();
    Point pt;
    std::string message;

    // text
    pt.x = pos.x + (pos.w - text.w()) / 2;
    pt.y = pos.y;
    text.Blit(pt);

    // sprite edit
    const Surface & sprite_edit = AGG::GetICN(ICN::TOWNWIND, 4);
    pt.x = pos.x + 80;
    pt.y = pos.y + 35;
    display.Blit(sprite_edit, pt);

    message.clear();
    String::AddInt(message, cur);
    text.Set(message);
    pt.x = pos.x + 80 + (sprite_edit.w() - text.w()) / 2;
    pt.y = pos.y + 36;
    text.Blit(pt);

    // buttons
    pt.x = pos.x + 150;
    pt.y = pos.y + 31;
    Button buttonUp(pt, ICN::TOWNWIND, 5, 6);

    pt.x = pos.x + 150;
    pt.y = pos.y + 47;
    Button buttonDn(pt, ICN::TOWNWIND, 7, 8);

    pt.x = pos.x;
    pt.y = box.GetArea().y + box.GetArea().h - AGG::GetICN(system, 1).h();
    Button buttonOk(pt, system, 1, 2);

    pt.x = pos.x + pos.w - AGG::GetICN(system, 3).w();
    pt.y = box.GetArea().y + box.GetArea().h - AGG::GetICN(system, 3).h();
    Button buttonCancel(pt, system, 3, 4);

    text.Set("MAX", Font::SMALL);
    const Rect rectMax(pos.x + 173, pos.y + 38, text.w(), text.h());
    text.Blit(rectMax.x, rectMax.y);

    LocalEvent & le = LocalEvent::Get();

    buttonUp.Draw();
    buttonDn.Draw();
    buttonOk.Draw();
    buttonCancel.Draw();

    bool redraw_count = false;
    cursor.Show();
    display.Flip();

    // message loop
    while(le.HandleEvents())
    {
	le.MousePressLeft(buttonOk) ? buttonOk.PressDraw() : buttonOk.ReleaseDraw();
        le.MousePressLeft(buttonCancel) ? buttonCancel.PressDraw() : buttonCancel.ReleaseDraw();
	le.MousePressLeft(buttonUp) ? buttonUp.PressDraw() : buttonUp.ReleaseDraw();
	le.MousePressLeft(buttonDn) ? buttonDn.PressDraw() : buttonDn.ReleaseDraw();

	if(PressIntKey(min, max, cur))
	    redraw_count = true;

        // max
        if(le.MouseClickLeft(rectMax))
        {
    	    cur = max;
    	    redraw_count = true;
        }
	else
	// up
	if((le.MouseWheelUp(pos) ||
            le.MouseClickLeft(buttonUp)) && cur < max)
	{
	    ++cur;
    	    redraw_count = true;
	}
	else
	// down
	if((le.MouseWheelDn(pos) ||
            le.MouseClickLeft(buttonDn)) && min < cur)
	{
	    --cur;
    	    redraw_count = true;
	}

	if(redraw_count)
	{
	    cursor.Hide();
	    pt.x = pos.x + 80;
	    pt.y = pos.y + 35;
	    display.Blit(sprite_edit, pt);

	    message.clear();
	    String::AddInt(message, cur);
	    text.Set(message, Font::BIG);
	    pt.x = pos.x + 80 + (sprite_edit.w() - text.w()) / 2;
	    pt.y = pos.y + 36;
	    text.Blit(pt);
	    cursor.Show();
	    display.Flip();

	    redraw_count = false;
	}

        if(Game::HotKeyPress(Game::EVENT_DEFAULT_READY) || le.MouseClickLeft(buttonOk)){ cursor.Hide(); return true; }
	else
	if(Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT) || le.MouseClickLeft(buttonCancel)){ cur = 0;  break; }
    }
    cursor.Hide();
    return false;
}

bool Dialog::InputString(const std::string &header, std::string &res)
{
    const ICN::icn_t system = Settings::Get().EvilInterface() ? ICN::SYSTEME : ICN::SYSTEM;

    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    cursor.Hide();
    Cursor::themes_t oldcursor = cursor.Themes();
    cursor.SetThemes(cursor.POINTER);

    //const bool pda = Settings::Get().PocketPC();
    if(res.size()) res.clear();
    res.reserve(48);

    TextBox textbox(header, Font::BIG, BOXAREA_WIDTH);
    Point dst_pt;
    const Surface & sprite = AGG::GetICN((Settings::Get().EvilInterface() ? ICN::BUYBUILD : ICN::BUYBUILE), 3);

    Box box(10 + textbox.h() + 10 + sprite.h(), OK|CANCEL);
    const Rect & box_rt = box.GetArea();

    // text
    dst_pt.x = box_rt.x + (box_rt.w - textbox.w()) / 2;
    dst_pt.y = box_rt.y + 10;
    textbox.Blit(dst_pt);

    dst_pt.y = box_rt.y + 10 + textbox.h() + 10;
    dst_pt.x = box_rt.x + (box_rt.w - sprite.w()) / 2;
    display.Blit(sprite, dst_pt);
    const Rect text_rt(dst_pt.x, dst_pt.y, sprite.w(), sprite.h());

    Text text("_", Font::BIG);
    display.Blit(sprite, text_rt);
    text.Blit(dst_pt.x + (sprite.w() - text.w()) / 2, dst_pt.y - 1);

    dst_pt.x = box_rt.x;
    dst_pt.y = box_rt.y + box_rt.h - AGG::GetICN(system, 1).h();
    Button buttonOk(dst_pt, system, 1, 2);

    dst_pt.x = box_rt.x + box_rt.w - AGG::GetICN(system, 3).w();
    dst_pt.y = box_rt.y + box_rt.h - AGG::GetICN(system, 3).h();
    Button buttonCancel(dst_pt, system, 3, 4);

    buttonOk.SetDisable(res.empty());
    buttonOk.Draw();
    buttonCancel.Draw();

    cursor.Show();
    display.Flip();

    LocalEvent & le = LocalEvent::Get();
    bool redraw = true;

    // message loop
    while(le.HandleEvents())
    {
	buttonOk.isEnable() && le.MousePressLeft(buttonOk) ? buttonOk.PressDraw() : buttonOk.ReleaseDraw();
        le.MousePressLeft(buttonCancel) ? buttonCancel.PressDraw() : buttonCancel.ReleaseDraw();

	if(Settings::Get().PocketPC() && le.MousePressLeft(text_rt))
	{
	    PocketPC::KeyboardDialog(res);
	    redraw = true;
	}

        if(Game::HotKeyPress(Game::EVENT_DEFAULT_READY) || (buttonOk.isEnable() && le.MouseClickLeft(buttonOk))) break;
	else
	if(Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT) || le.MouseClickLeft(buttonCancel)){ res.clear(); break; }
	else
	if(le.KeyPress())
	{
	    String::AppendKey(res, le.KeyValue(), le.KeyMod());
	    redraw = true;
	}

	if(redraw)
	{
	    buttonOk.SetDisable(res.empty());
	    buttonOk.Draw();

	    text.Set(res + "_");

	    if(text.w() < sprite.w() - 24)
	    {
		cursor.Hide();
		display.Blit(sprite, text_rt);
		text.Blit(text_rt.x + (text_rt.w - text.w()) / 2, text_rt.y - 1);
		cursor.Show();
		display.Flip();
	    }
	    redraw = false;
	}
    }

    cursor.SetThemes(oldcursor);
    cursor.Hide();

    return res.size();
}

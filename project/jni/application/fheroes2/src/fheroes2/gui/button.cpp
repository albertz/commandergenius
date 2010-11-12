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
#include "button.h"

Button::Button() : icn(ICN::UNKNOWN), index1(0), index2(0), pressed(false), disable(false)
{
}

Button::Button(const Point &pt, const ICN::icn_t n, u16 i1, u16 i2) : icn(n), index1(i1), index2(i2),
    pressed(false), disable(false)
{
    SetPos(pt);

    const Sprite & sprite1 = AGG::GetICN(icn, index1);

    w = sprite1.w();
    h = sprite1.h();
}

Button::Button(u16 ox, u16 oy, const ICN::icn_t n, u16 i1, u16 i2) : icn(n), index1(i1), index2(i2),
    pressed(false), disable(false)
{
    SetPos(ox, oy);

    const Sprite & sprite1 = AGG::GetICN(icn, index1);

    w = sprite1.w();
    h = sprite1.h();
}

void Button::SetPos(const Point & pt)
{
    SetPos(pt.x, pt.y);
}

void Button::SetPos(const u16 ox, const u16 oy)
{
    x = ox;
    y = oy;
}

void Button::SetSprite(const ICN::icn_t n, const u16 i1, const u16 i2)
{
    icn = n;
    index1 = i1;
    index2 = i2;

    const Sprite & sprite1 = AGG::GetICN(icn, index1);

    w = sprite1.w();
    h = sprite1.h();
}

void Button::Press(void)
{
    if(disable || pressed) return;

    pressed = true;
}

void Button::Release(void)
{
    if(disable || !pressed) return;

    pressed = false;
}

void Button::PressDraw(void)
{
    if(disable || pressed) return;

    Press();

    Draw();

    Display::Get().Flip();
}

void Button::ReleaseDraw(void)
{
    if(disable || !pressed) return;

    Release();

    Draw();

    Display::Get().Flip();
}

void Button::Draw(void)
{
    bool localcursor = false;
    Cursor & cursor = Cursor::Get();
    if(*this & cursor.GetRect() && cursor.isVisible()){ cursor.Hide(); localcursor = true; }

    const Sprite & sprite1 = AGG::GetICN(icn, index1);
    const Sprite & sprite2 = AGG::GetICN(icn, index2);

    Display::Get().Blit(pressed ? sprite2 : sprite1, x, y);

    if(localcursor) cursor.Show();
}

ButtonGroups::ButtonGroups(const Rect & pos, u16 btns) : button1(NULL), button2(NULL), result1(Dialog::ZERO), result2(Dialog::ZERO), buttons(btns)
{
    Point pt;
    const ICN::icn_t system = Settings::Get().EvilInterface() ? ICN::SYSTEME : ICN::SYSTEM;

    switch(buttons)
    {
	case Dialog::YES|Dialog::NO:
            pt.x = pos.x;
            pt.y = pos.y + pos.h - AGG::GetICN(system, 5).h();
	    button1 = new Button(pt, system, 5, 6);
	    result1 = Dialog::YES;
            pt.x = pos.x + pos.w - AGG::GetICN(system, 7).w();
            pt.y = pos.y + pos.h - AGG::GetICN(system, 7).h();
	    button2 = new Button(pt, system, 7, 8);
	    result2 = Dialog::NO;
	    break;

	case Dialog::OK|Dialog::CANCEL:
            pt.x = pos.x;
            pt.y = pos.y + pos.h - AGG::GetICN(system, 1).h();
	    button1 = new Button(pt, system, 1, 2);
	    result1 = Dialog::OK;
            pt.x = pos.x + pos.w - AGG::GetICN(system, 3).w();
            pt.y = pos.y + pos.h - AGG::GetICN(system, 3).h();
	    button2 = new Button(pt, system, 3, 4);
	    result2 = Dialog::CANCEL;
	    break;

	case Dialog::OK:
            pt.x = pos.x + (pos.w - AGG::GetICN(system, 1).w()) / 2;
            pt.y = pos.y + pos.h - AGG::GetICN(system, 1).h();
	    button1 = new Button(pt, system, 1, 2);
	    result1 = Dialog::OK;
	    break;

	case Dialog::CANCEL:
            pt.x = pos.x + (pos.w - AGG::GetICN(system, 3).w()) / 2;
            pt.y = pos.y + pos.h - AGG::GetICN(system, 3).h();
	    button2 = new Button(pt, system, 3, 4);
	    result2 = Dialog::CANCEL;
	    break;

	default:
	    break;
    }
}

ButtonGroups::~ButtonGroups()
{
    if(button1) delete button1;
    if(button2) delete button2;
}

void ButtonGroups::Draw(void)
{
    if(button1) (*button1).Draw();
    if(button2) (*button2).Draw();
}

u16 ButtonGroups::QueueEventProcessing(void)
{
    LocalEvent & le = LocalEvent::Get();

    if(button1 && button1->isEnable()) le.MousePressLeft(*button1) ? button1->PressDraw() : button1->ReleaseDraw();
    if(button2 && button2->isEnable()) le.MousePressLeft(*button2) ? button2->PressDraw() : button2->ReleaseDraw();

    if(button1 && button1->isEnable() && le.MouseClickLeft(*button1)) return result1;
    if(button2 && button2->isEnable() && le.MouseClickLeft(*button2)) return result2;

    if(button1 && button2)
    {
	if(buttons == (Dialog::YES|Dialog::NO) ||
	    buttons == (Dialog::OK|Dialog::CANCEL))
	{
	    if(Game::HotKeyPress(Game::EVENT_DEFAULT_READY)) return result1;
    	    if(Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT)) return result2;
	}

	if(Game::HotKeyPress(Game::EVENT_DEFAULT_LEFT)) return result1;
	else
	if(Game::HotKeyPress(Game::EVENT_DEFAULT_RIGHT)) return result2;
    }
    else
    // one button
    {
	if(HotKeyCloseWindow) return buttons;
    }

    return Dialog::ZERO;
}

void ButtonGroups::DisableButton1(bool f)
{
    if(button1) button1->SetDisable(f);
}

void ButtonGroups::DisableButton2(bool f)
{
    if(button1) button2->SetDisable(f);
}

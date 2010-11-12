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
#include "dialog.h"

bool Dialog::SelectGoldOrExp(const std::string &header, const std::string &message, const u16 gold, const u16 expr)
{
    Display & display = Display::Get();
    const ICN::icn_t system = Settings::Get().EvilInterface() ? ICN::SYSTEME : ICN::SYSTEM;
        
    // preload
    AGG::PreloadObject(system);

    // cursor
    Cursor & cursor = Cursor::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const Sprite & sprite_gold = AGG::GetICN(ICN::RESOURCE, 6);
    const Sprite & sprite_expr = AGG::GetICN(ICN::EXPMRL, 4);

    Point pt;
    TextBox box1(header, Font::YELLOW_BIG, BOXAREA_WIDTH);
    TextBox box2(message, Font::BIG, BOXAREA_WIDTH);

    Text text;
    std::string str;
    String::AddInt(str, gold);
    text.Set(str, Font::SMALL);

    const u8 spacer = Settings::Get().QVGA() ? 5 : 10;

    Box box(box1.h() + spacer + box2.h() + spacer + sprite_expr.h() + 2 + text.h(), true);

    pt.x = box.GetArea().x + box.GetArea().w / 2 - AGG::GetICN(system, 9).w() - 20;
    pt.y = box.GetArea().y + box.GetArea().h - AGG::GetICN(system, 5).h();
    Button button_yes(pt, system, 5, 6);

    pt.x = box.GetArea().x + box.GetArea().w / 2 + 20;
    pt.y = box.GetArea().y + box.GetArea().h - AGG::GetICN(system, 7).h();
    Button button_no(pt, system, 7, 8);

    Rect pos = box.GetArea();

    if(header.size()) box1.Blit(pos);
    pos.y += box1.h() + spacer;

    if(message.size()) box2.Blit(pos);
    pos.y += box2.h() + spacer;

    pos.y += sprite_expr.h();
    // sprite1
    pos.x = box.GetArea().x + box.GetArea().w / 2 - sprite_gold.w() - 30;
    display.Blit(sprite_gold, pos.x, pos.y - sprite_gold.h());
    // text
    text.Blit(pos.x + (sprite_gold.w() - text.w()) / 2, pos.y + 2);

    // sprite2
    pos.x = box.GetArea().x + box.GetArea().w / 2 + 30;
    display.Blit(sprite_expr, pos.x, pos.y - sprite_expr.h());
    // text
    str.clear();
    String::AddInt(str, expr);
    text.Set(str, Font::SMALL);
    text.Blit(pos.x + (sprite_expr.w() - text.w()) / 2, pos.y + 2);

    button_yes.Draw();
    button_no.Draw();

    cursor.Show();
    display.Flip();
    LocalEvent & le = LocalEvent::Get();
    bool result = false;

    // message loop
    while(le.HandleEvents())
    {
	le.MousePressLeft(button_yes) ? button_yes.PressDraw() : button_yes.ReleaseDraw();
	le.MousePressLeft(button_no) ? button_no.PressDraw() : button_no.ReleaseDraw();

        if(Game::HotKeyPress(Game::EVENT_DEFAULT_READY) || le.MouseClickLeft(button_yes)){ result = true; break; }
        if(Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT) || le.MouseClickLeft(button_no)){ result = false; break; }
    }

    cursor.Hide();

    return result;
}

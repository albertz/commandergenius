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

u16 Dialog::ResourceInfo(const std::string &header, const std::string &message, const Resource::funds_t & rs, u16 buttons)
{
    Display & display = Display::Get();
    const ICN::icn_t system = Settings::Get().EvilInterface() ? ICN::SYSTEME : ICN::SYSTEM;

    // preload
    AGG::PreloadObject(system);

    // cursor
    Cursor & cursor = Cursor::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    TextBox box1(header, Font::YELLOW_BIG, BOXAREA_WIDTH);
    TextBox box2(message, Font::BIG, BOXAREA_WIDTH);
    Resource::BoxSprite rbs(rs, BOXAREA_WIDTH);

    const u8 spacer = Settings::Get().QVGA();

    Box box(box1.h() + spacer + box2.h() + spacer + rbs.GetArea().h, true);
    Point pos = box.GetArea();

    if(header.size()) box1.Blit(pos);
    pos.y += box1.h() + spacer;

    if(message.size()) box2.Blit(pos);
    pos.y += box2.h() + spacer;

    rbs.SetPos(pos.x, pos.y);
    rbs.Redraw();

    LocalEvent & le = LocalEvent::Get();

    ButtonGroups btnGroups(box.GetArea(), buttons);
    btnGroups.Draw();

    cursor.Show();
    display.Flip();

    u16 result = Dialog::ZERO;

    while(result == Dialog::ZERO && le.HandleEvents())
    {
        if(!buttons && !le.MousePressRight()) break;
        result = btnGroups.QueueEventProcessing();
    }

    return result;
}

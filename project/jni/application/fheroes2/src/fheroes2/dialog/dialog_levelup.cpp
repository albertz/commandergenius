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

Skill::Secondary::skill_t Dialog::LevelUpSelectSkill(const std::string & header, const Skill::Secondary & sec1, const Skill::Secondary & sec2)
{
    Display & display = Display::Get();
    const ICN::icn_t system = Settings::Get().EvilInterface() ? ICN::SYSTEME : ICN::SYSTEM;

    // preload
    AGG::PreloadObject(system);

    // cursor
    Cursor & cursor = Cursor::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const Sprite &sprite_frame = AGG::GetICN(ICN::SECSKILL, 15);
    const Sprite & sprite_skill1 = AGG::GetICN(ICN::SECSKILL, Skill::Secondary::GetIndexSprite1(sec1.Skill()));
    const Sprite & sprite_skill2 = AGG::GetICN(ICN::SECSKILL, Skill::Secondary::GetIndexSprite1(sec2.Skill()));

    Point pt;
    std::string message = _("You may learn either %{level1} %{skill1} or %{level2} %{skill2}.");
    String::Replace(message, "%{level1}", Skill::Level::String(sec1.Level()));
    String::Replace(message, "%{skill1}", Skill::Secondary::String(sec1.Skill()));
    String::Replace(message, "%{level2}", Skill::Level::String(sec2.Level()));
    String::Replace(message, "%{skill2}", Skill::Secondary::String(sec2.Skill()));

    TextBox box1(header, Font::BIG, BOXAREA_WIDTH);
    TextBox box2(message, Font::BIG, BOXAREA_WIDTH);
    const u8 spacer = Settings::Get().QVGA() ? 5 : 10;

    Box box(box1.h() + spacer + box2.h() + 10 + sprite_frame.h(), true);

    pt.x = box.GetArea().x + box.GetArea().w / 2 - AGG::GetICN(system, 9).w() - 20;
    pt.y = box.GetArea().y + box.GetArea().h - AGG::GetICN(system, 9).h();
    Button button_learn1(pt, system, 9, 10);

    pt.x = box.GetArea().x + box.GetArea().w / 2 + 20;
    pt.y = box.GetArea().y + box.GetArea().h - AGG::GetICN(system, 9).h();
    Button button_learn2(pt, system, 9, 10);

    Rect pos = box.GetArea();

    if(header.size()) box1.Blit(pos);
    pos.y += box1.h() + spacer;

    if(message.size()) box2.Blit(pos);
    pos.y += box2.h() + spacer;

    // sprite1
    pos.x = box.GetArea().x + box.GetArea().w / 2 - sprite_frame.w() - 20;
    display.Blit(sprite_frame, pos);
    pos.x += 3;
    Rect rect_image1(pos, sprite_skill1.w(), sprite_skill1.h());
    display.Blit(sprite_skill1, pos.x, pos.y + 3);

    Text text;
    // text
    text.Set(Skill::Secondary::String(sec1.Skill()), Font::SMALL);
    text.Blit(pos.x + (sprite_skill1.w() - text.w()) / 2, pos.y + 5);
    text.Set(Skill::Level::String(sec1.Level()), Font::SMALL);
    text.Blit(pos.x + (sprite_skill1.w() - text.w()) / 2, pos.y + sprite_skill1.h() - 12);

    // sprite2
    pos.x = box.GetArea().x + box.GetArea().w / 2 + 20;
    display.Blit(sprite_frame, pos);
    pos.x += 3;
    Rect rect_image2(pos, sprite_skill2.w(), sprite_skill2.h());
    display.Blit(sprite_skill2, pos.x, pos.y + 3);
    // text
    Text name_skill2(Skill::Secondary::String(sec2.Skill()), Font::SMALL);
    name_skill2.Blit(pos.x + (sprite_skill2.w() - name_skill2.w()) / 2, pos.y + 5);
    Text name_level2(Skill::Level::String(sec2.Level()), Font::SMALL);
    name_level2.Blit(pos.x + (sprite_skill2.w() - name_level2.w()) / 2, pos.y + sprite_skill2.h() - 12);

    button_learn1.Draw();
    button_learn2.Draw();

    cursor.Show();
    display.Flip();
    LocalEvent & le = LocalEvent::Get();
    Skill::Secondary::skill_t result = Skill::Secondary::UNKNOWN;

    // message loop
    while(le.HandleEvents())
    {
	le.MousePressLeft(button_learn1) ? button_learn1.PressDraw() : button_learn1.ReleaseDraw();
	le.MousePressLeft(button_learn2) ? button_learn2.PressDraw() : button_learn2.ReleaseDraw();

        if(le.MouseClickLeft(button_learn1) || Game::HotKeyPress(Game::EVENT_DEFAULT_LEFT)){ result = sec1.Skill(); break; }
        if(le.MouseClickLeft(button_learn2) || Game::HotKeyPress(Game::EVENT_DEFAULT_RIGHT)){ result = sec2.Skill(); break; }

	if(le.MouseClickLeft(rect_image1)){ cursor.Hide(); SecondarySkillInfo(sec1.Skill(), sec1.Level()); cursor.Show(); display.Flip(); }
	else
	if(le.MouseClickLeft(rect_image2)){ cursor.Hide(); SecondarySkillInfo(sec2.Skill(), sec2.Level()); cursor.Show(); display.Flip(); }

	if(le.MousePressRight(rect_image1)){ cursor.Hide(); SecondarySkillInfo(sec1.Skill(), sec1.Level(), false); cursor.Show(); display.Flip(); }
	else
	if(le.MousePressRight(rect_image2)){ cursor.Hide(); SecondarySkillInfo(sec2.Skill(), sec2.Level(), false); cursor.Show(); display.Flip(); }
    }

    cursor.Hide();

    return result;
}

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

#include "gamedefs.h"
#include "agg.h"
#include "cursor.h"
#include "game.h"
#include "statusbar.h"
#include "dialog.h"
#include "monster.h"
#include "button.h"
#include "army_troop.h"
#include "battle_stats.h"

#ifndef BUILD_RELEASE

void TestMonsterSprite(void)
{
    // cursor
    Cursor & cursor = Cursor::Get();
    cursor.Hide();
    cursor.SetThemes(Cursor::POINTER);

    //Monster monster(Monster::PEASANT);
    Army::Troop troop(Monster::PEASANT, 1);
    troop.BattleInit();
    const Battle2::Stats & b = *troop.GetBattleStats();
    Background back;
    Rect pos;

    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    std::string str;

    StatusBar speed_bar;
    StatusBar count_bar;
    StatusBar start_bar;
    StatusBar frame_bar;
    StatusBar info_bar;

    start_bar.SetCenter(100, display.h() - 15);
    count_bar.SetCenter(200, display.h() - 15);
    speed_bar.SetCenter(300, display.h() - 15);
    frame_bar.SetCenter(400, display.h() - 15);
    info_bar.SetCenter(550, display.h() - 15);

    u32 ticket = 0;

    u8 start = 0;
    u8 count = AGG::GetICNCount(b.ICNFile());
    u8 frame = 0;
    u8 speed = 100;

    str.clear();
    String::AddInt(str, frame);
    frame_bar.ShowMessage("frame: " + str);

    str.clear();
    String::AddInt(str, speed);
    speed_bar.ShowMessage("speed: " + str);

    str.clear();
    String::AddInt(str, start);
    start_bar.ShowMessage("start: " + str);

    str.clear();
    String::AddInt(str, count);
    count_bar.ShowMessage("count: " + str);

    cursor.Show();
    display.Flip();

    // mainmenu loop
    while(le.HandleEvents())
    {
	if(Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT)) break;

	if(le.MouseClickLeft(pos))
	{
            u32 mons = troop();
            if(Dialog::SelectCount("Monster", Monster::PEASANT, Monster::WATER_ELEMENT, mons))
	    {
        	cursor.Hide();
		troop.SetMonster(Monster::FromInt(mons));
		start = 0;
		count = AGG::GetICNCount(b.ICNFile());
		frame = 0;
        	cursor.Show();
        	display.Flip();
	    }
	}

	if(le.MouseClickLeft(start_bar.GetRect()))
	{
	    u32 start2 = start;
	    if(Dialog::SelectCount("Start", 0, AGG::GetICNCount(b.ICNFile()) - 1, start2))
	    {
        	cursor.Hide();
		start = start2;
		if(start + count > AGG::GetICNCount(b.ICNFile())) count = AGG::GetICNCount(b.ICNFile()) - start;
		str.clear();
		String::AddInt(str, start);
		start_bar.ShowMessage("start: " + str);
        	cursor.Show();
        	display.Flip();
    	    }
	}

	if(le.MouseClickLeft(count_bar.GetRect()))
	{
	    u32 count2 = count;
	    if(Dialog::SelectCount("Count", 1, AGG::GetICNCount(b.ICNFile()), count2))
	    {
        	cursor.Hide();
		count = count2;
		frame = start;
		str.clear();
		String::AddInt(str, count);
		count_bar.ShowMessage("count: " + str);
        	cursor.Show();
        	display.Flip();
	    }
	}

	if(le.MouseClickLeft(speed_bar.GetRect()))
	{
	    u32 speed2 = speed;
	    if(Dialog::SelectCount("Speed", 1, 50, speed2))
	    {
        	cursor.Hide();
		speed = speed2;
		frame = start;
		str.clear();
		String::AddInt(str, speed);
		speed_bar.ShowMessage("speed: " + str);
        	cursor.Show();
        	display.Flip();
	    }
	}

        if(0 == (ticket % speed))
        {
            cursor.Hide();
            const Sprite & sprite = AGG::GetICN(b.ICNFile(), frame);
	    pos.x = 320 + sprite.x();
	    pos.y = 240 + sprite.y();
	    pos.w = sprite.w();
	    pos.h = sprite.h();
	    back.Restore();
	    back.Save(pos);
            display.Blit(sprite, pos);

	    str.clear();
	    String::AddInt(str, frame);
	    frame_bar.ShowMessage("frame: " + str);

	    str = "ox: ";
	    String::AddInt(str, sprite.x());
	    str += ", oy: ";
	    String::AddInt(str, sprite.y());
	    info_bar.ShowMessage(str);

            cursor.Show();
            display.Flip();

	    ++frame;
	    if(frame >= start + count) frame = start;
        }

        ++ticket;
    }
}

#endif

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

#include <algorithm>
#include "agg.h"
#include "settings.h"
#include "cursor.h"
#include "button.h"
#include "dialog.h"
#include "interface_list.h"

class SettingsListBox : public Interface::ListBox<u32>
{
public:
    SettingsListBox(const Point & pt) : Interface::ListBox<u32>(pt) {};

    void RedrawItem(const u32 &, s16, s16, bool);
    void RedrawBackground(const Point &);

    void ActionCurrentUp(void){};
    void ActionCurrentDn(void){};
    void ActionListDoubleClick(u32 &);
    void ActionListSingleClick(u32 &);
    void ActionListPressRight(u32 &){};
};

void SettingsListBox::RedrawItem(const u32 & item, s16 ox, s16 oy, bool current)
{
    Display & display = Display::Get();
    const Settings & conf = Settings::Get();

    const Sprite & cell = AGG::GetICN(ICN::CELLWIN, 1);
    const Sprite & mark = AGG::GetICN(ICN::CELLWIN, 2);

    display.Blit(cell, ox, oy);
    if(conf.ExtModes(item)) display.Blit(mark, ox + 3, oy + 2);

    TextBox msg(conf.ExtName(item), Font::SMALL, 250);
    msg.SetAlign(ALIGN_LEFT);

    if(1 < msg.row())
	msg.Blit(ox + cell.w() + 5, oy - 1);
    else
	msg.Blit(ox + cell.w() + 5, oy + 4);
}

void SettingsListBox::RedrawBackground(const Point & top)
{
    Display & display = Display::Get();
    const Settings & conf = Settings::Get();

    const u16 window_h = conf.QVGA() ? 224 : 400;
    const u16 ah = window_h - 54;

    display.Blit(AGG::GetICN(ICN::STONEBAK, 0), Rect(15, 25, 280, ah), top.x + 15, top.y + 25);

    for(u8 ii = 1; ii < (window_h / 25); ++ii)
	display.Blit(AGG::GetICN(ICN::DROPLISL, 11), top.x + 295, top.y + 35 + (19 * ii));

    display.Blit(AGG::GetICN(ICN::DROPLISL, 10), top.x + 295, top.y + 46);
    display.Blit(AGG::GetICN(ICN::DROPLISL, 12), top.x + 295, top.y + ah - 14);
}

void SettingsListBox::ActionListDoubleClick(u32 & item)
{
    ActionListSingleClick(item);
}

void SettingsListBox::ActionListSingleClick(u32 & item)
{
    Settings & conf = Settings::Get();
    conf.ExtModes(item) ? conf.ExtResetModes(item) : conf.ExtSetModes(item);
}

void Dialog::ExtSettings(void)
{
    Display & display = Display::Get();
    const Settings & conf = Settings::Get();

    // cursor
    Cursor & cursor = Cursor::Get();
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const u16 window_w = 320;
    const u16 window_h = conf.QVGA() ? 224 : 400;

    Dialog::FrameBorder frameborder;
    frameborder.SetPosition((display.w() - window_w) / 2 - BORDERWIDTH, (display.h() - window_h) / 2 - BORDERWIDTH, window_w, window_h);
    frameborder.Redraw();

    const Rect & area = frameborder.GetArea();
    const Sprite & background = AGG::GetICN(ICN::STONEBAK, 0);
    display.Blit(background, Rect(0, 0, window_w, window_h), area);

    Text text("FHeroes2 Settings", Font::YELLOW_BIG);
    text.Blit(area.x + (area.w - text.w()) / 2, area.y + 6);

    std::vector<u32> states;

    states.push_back(Settings::GAME_SAVE_REWRITE_CONFIRM);
    states.push_back(Settings::GAME_ALSO_CONFIRM_AUTOSAVE);
    states.push_back(Settings::GAME_REMEMBER_LAST_FOCUS);
    states.push_back(Settings::GAME_REMEMBER_LAST_FILENAME);
    states.push_back(Settings::GAME_SHOW_SYSTEM_INFO);
    states.push_back(Settings::GAME_EVIL_INTERFACE);
    if(!conf.PocketPC())
	states.push_back(Settings::GAME_DYNAMIC_INTERFACE);
    if(conf.VideoMode().w >= 640 && conf.VideoMode().w >= 480)
	states.push_back(Settings::GAME_HIDE_INTERFACE);
    states.push_back(Settings::GAME_AUTOSAVE_ON);
    if(conf.VideoMode().w == 640 && conf.VideoMode().w == 480)
	states.push_back(Settings::GAME_USE_FADE);
#ifdef BUILD_RELEASE
    states.push_back(Settings::GAME_SHOW_SDL_LOGO);
#endif
    states.push_back(Settings::WORLD_SHOW_VISITED_CONTENT);
    states.push_back(Settings::WORLD_ABANDONED_MINE_RANDOM);
    states.push_back(Settings::WORLD_SAVE_MONSTER_BATTLE);
    states.push_back(Settings::WORLD_ALLOW_SET_GUARDIAN);
    states.push_back(Settings::WORLD_NOREQ_FOR_ARTIFACTS);
    states.push_back(Settings::WORLD_SCOUTING_EXTENDED);
    states.push_back(Settings::WORLD_ARTIFACT_CRYSTAL_BALL);
    states.push_back(Settings::WORLD_ONLY_FIRST_MONSTER_ATTACK);
    states.push_back(Settings::WORLD_EYE_EAGLE_AS_SCHOLAR);
    states.push_back(Settings::WORLD_BUY_BOOK_FROM_SHRINES);
    states.push_back(Settings::HEROES_FORCE_RACE_FROM_TYPE);
    states.push_back(Settings::HEROES_LEARN_SPELLS_WITH_DAY);
    states.push_back(Settings::HEROES_COST_DEPENDED_FROM_LEVEL);
    states.push_back(Settings::HEROES_REMEMBER_POINTS_RETREAT);
    states.push_back(Settings::HEROES_SURRENDERING_GIVE_EXP);
    states.push_back(Settings::HEROES_RECALCULATE_MOVEMENT);

    if(conf.VideoMode().w >= 640 && conf.VideoMode().w >= 480)
	states.push_back(Settings::CASTLE_ALLOW_BUY_FROM_WELL);

    states.push_back(Settings::UNIONS_ALLOW_HERO_MEETINGS);
    states.push_back(Settings::UNIONS_ALLOW_CASTLE_VISITING);
    states.push_back(Settings::UNIONS_ALLOW_VIEW_MAPS);

    states.push_back(Settings::BATTLE_SHOW_DAMAGE);
    states.push_back(Settings::BATTLE_SOFT_WAITING);
    states.push_back(Settings::BATTLE_TROOP_DIRECTION);
    states.push_back(Settings::BATTLE_SHOW_GRID);
    states.push_back(Settings::BATTLE_SHOW_MOUSE_SHADOW);
    states.push_back(Settings::BATTLE_SHOW_MOVE_SHADOW);
    states.push_back(Settings::BATTLE_OBJECTS_ARCHERS_PENALTY);
    states.push_back(Settings::BATTLE_MERGE_ARMIES);
    states.push_back(Settings::BATTLE_ARCHMAGE_RESIST_BAD_SPELL);
    states.push_back(Settings::BATTLE_MAGIC_TROOP_RESIST);
    states.push_back(Settings::BATTLE_SKIP_INCREASE_DEFENSE);

    if(conf.PocketPC())
    {
	states.push_back(Settings::POCKETPC_HIDE_CURSOR);
	states.push_back(Settings::POCKETPC_TAP_MODE);
	states.push_back(Settings::POCKETPC_LOW_MEMORY);
    }

    SettingsListBox listbox(area);

    const u16 ah = window_h - 54;

    listbox.RedrawBackground(area);
    listbox.SetScrollButtonUp(ICN::DROPLISL, 6, 7, Point(area.x + 295, area.y + 25));
    listbox.SetScrollButtonDn(ICN::DROPLISL, 8, 9, Point(area.x + 295, area.y + ah + 5));
    listbox.SetScrollSplitter(AGG::GetICN(ICN::DROPLISL, 13), Rect(area.x + 300, area.y + 49, 12, ah - 48));
    listbox.SetAreaMaxItems(ah / 36);
    listbox.SetAreaItems(Rect(area.x + 10, area.y + 30, 290, ah));
    listbox.SetListContent(states);
    listbox.Redraw();

    LocalEvent & le = LocalEvent::Get();

    ButtonGroups btnGroups(area, Dialog::OK|Dialog::CANCEL);
    btnGroups.Draw();

    cursor.Show();
    display.Flip();

    // message loop
    u16 result = Dialog::ZERO;

    while(result == Dialog::ZERO && le.HandleEvents())
    {
	result = btnGroups.QueueEventProcessing();

	listbox.QueueEventProcessing();

	if(!cursor.isVisible())
	{
	    listbox.Redraw();
	    cursor.Show();
	    display.Flip();
	}
    }

    // store
    if(result == Dialog::OK)
    {
	le.SetTapMode(conf.ExtTapMode());
	Settings::Get().BinarySave();
    }
}

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

#include <ctime>
#include <sstream>
#include "agg.h"
#include "settings.h"
#include "maps.h"
#include "mp2.h"
#include "world.h"
#include "dialog.h"
#include "game_focus.h"
#include "game_interface.h"

bool Interface::NoGUI(void)
{
    const Settings & conf = Settings::Get();

    return conf.NetworkDedicatedServer();
}

Interface::Basic::Basic() : gameArea(GameArea::Get()), radar(Radar::Get()),
    iconsPanel(IconsPanel::Get()), buttonsArea(ButtonsArea::Get()),
    statusWindow(StatusWindow::Get()), borderWindow(BorderWindow::Get()),
    controlPanel(ControlPanel::Get()), redraw(0)
{
    Settings & conf = Settings::Get().Get();
    const Display & display = Display::Get();
    const u16 & px = display.w() - BORDERWIDTH - RADARWIDTH;
    const u8 scroll_width = conf.QVGA() ? 12 : BORDERWIDTH;

    if(conf.HideInterface())
    {
        iconsPanel.SetCount(2);
	conf.SetShowPanel(true);

	Point pos_radr = conf.PosRadar();
	Point pos_bttn = conf.PosButtons();
	Point pos_icon = conf.PosIcons();
	Point pos_stat = conf.PosStatus();

	if(0 == pos_radr.x && 0 == pos_radr.y)
	    pos_radr = Point(BORDERWIDTH, conf.QVGA() ? TILEWIDTH : BORDERWIDTH);
	if(0 == pos_icon.x && 0 == pos_icon.y)
	    pos_icon = Point(conf.QVGA() ? BORDERWIDTH : px - BORDERWIDTH, conf.QVGA() ? TILEWIDTH : radar.GetArea().y + radar.GetArea().h);
	if(0 == pos_bttn.x && 0 == pos_bttn.y)
	    pos_bttn = Point(conf.QVGA() ? BORDERWIDTH : px - BORDERWIDTH, conf.QVGA() ? TILEWIDTH : iconsPanel.GetArea().y + iconsPanel.GetArea().h);
	if(0 == pos_stat.x && 0 == pos_stat.y)
	    pos_stat = Point(conf.QVGA() ? BORDERWIDTH : px - BORDERWIDTH, conf.QVGA() ? TILEWIDTH : buttonsArea.GetArea().y + buttonsArea.GetArea().h);

	controlPanel.SetPos(display.w() - controlPanel.GetArea().w - scroll_width, 0);
	radar.SetPos(pos_radr.x, pos_radr.y);
	iconsPanel.SetPos(pos_icon.x, pos_icon.y);
	buttonsArea.SetPos(pos_bttn.x, pos_bttn.y);
	statusWindow.SetPos(pos_stat.x, pos_stat.y);
    }
    else
    {
	radar.SetPos(px, BORDERWIDTH);
	iconsPanel.SetPos(px, radar.GetArea().y + radar.GetArea().h + BORDERWIDTH);

        const u8 count_h = (display.h() - 480) / TILEWIDTH;
        iconsPanel.SetCount(count_h > 3 ? 8 : ( count_h < 3 ? 4 : 7));

	buttonsArea.SetPos(px, iconsPanel.GetArea().y + iconsPanel.GetArea().h + BORDERWIDTH);
	statusWindow.SetPos(px, buttonsArea.GetArea().y + buttonsArea.GetArea().h);
    }

    scrollLeft = Rect(0, 0, scroll_width, display.h());
    scrollRight = Rect(display.w() - scroll_width, 0, scroll_width, display.h());
    scrollTop = conf.QVGA() ? Rect(0, 0, controlPanel.GetArea().x, scroll_width) : Rect(0, 0, display.w() - radar.GetArea().w, scroll_width);
    scrollBottom = Rect(0, display.h() - scroll_width, display.w(), scroll_width);
    
    system_info.Set(Font::YELLOW_SMALL);
}

Interface::Basic & Interface::Basic::Get(void)
{
    static Basic basic;
    return basic;
}

const Rect & Interface::Basic::GetAreaScrollLeft(void) const
{
    return scrollLeft;
}

const Rect & Interface::Basic::GetAreaScrollRight(void) const
{
    return scrollRight;
}

const Rect & Interface::Basic::GetAreaScrollTop(void) const
{
    return scrollTop;
}

const Rect & Interface::Basic::GetAreaScrollBottom(void) const
{
    return scrollBottom;
}


bool Interface::Basic::NeedRedraw(void) const
{
    return redraw;
}

void Interface::Basic::SetRedraw(u8 f)
{
    redraw |= f;
}

void Interface::Basic::Redraw(u8 force)
{
    Settings & conf = Settings::Get();

    if((redraw | force) & REDRAW_GAMEAREA) gameArea.Redraw(Display::Get(), LEVEL_ALL);

    if((conf.HideInterface() && conf.ShowRadar()) || ((redraw | force) & REDRAW_RADAR)) radar.Redraw();

    if((conf.HideInterface() && conf.ShowIcons()) || ((redraw | force) & REDRAW_ICONS)) iconsPanel.Redraw();
    else
    if((redraw | force) & REDRAW_HEROES) iconsPanel.RedrawIcons(ICON_HEROES);
    else
    if((redraw | force) & REDRAW_CASTLES) iconsPanel.RedrawIcons(ICON_CASTLES);

    if((conf.HideInterface() && conf.ShowButtons()) || ((redraw | force) & REDRAW_BUTTONS)) buttonsArea.Redraw();

    if((conf.HideInterface() && conf.ShowStatus()) || ((redraw | force) & REDRAW_STATUS)) statusWindow.Redraw();

    if(conf.HideInterface() && conf.ShowControlPanel() && (redraw & REDRAW_GAMEAREA)) controlPanel.Redraw();

    u32 usage = GetMemoryUsage();

    // show system info
    if(conf.ExtShowSystemInfo() && usage)
	RedrawSystemInfo((conf.HideInterface() ? 10 : 26), Display::Get().h() - (conf.HideInterface() ? 14 : 30), usage);

    // memory limit trigger
    if(conf.ExtLowMemory() && conf.MemoryLimit() && usage)
    {
	if(conf.MemoryLimit() < usage)
	{
	    Display & display = Display::Get();
	    Cursor & cursor = Cursor::Get();

	    cursor.Hide();
	    Mixer::Reset();

	    Rect rect((display.w() - 90) / 2, (display.h() - 30) / 2, 90, 45);
	    TextBox text("memory limit\nclear cache\nwaiting...", Font::SMALL, rect.w);

	    display.FillRect(0, 0, 0, rect);
	    text.Blit(rect.x, rect.y);
	    
	    display.Flip();

	    AGG::Cache & cache = AGG::Cache::Get();

	    VERBOSE("MemoryLimit: " << "settings: " << conf.MemoryLimit() << ", game usage: " << usage);
	    cache.ClearAllICN();
	    VERBOSE("MemoryLimit: " << "free all " << "ICN" << ", game usage: " << GetMemoryUsage());
	    cache.ClearAllWAV();
	    VERBOSE("MemoryLimit: " << "free all " << "WAV" << ", game usage: " << GetMemoryUsage());
	    cache.ClearAllMID();
	    VERBOSE("MemoryLimit: " << "free all " << "MID" << ", game usage: " << GetMemoryUsage());

	    redraw = 0xFF;
	    if(conf.HideInterface()) redraw &= ~REDRAW_BORDER;

	    cursor.SetThemes(cursor.Themes(), true);
	    cursor.Show();

	    AGG::PlayMusic(MUS::FromGround(world.GetTiles(Game::Focus::Get().Center()).GetGround()));
	    Game::EnvironmentSoundMixer();
	}

	usage = GetMemoryUsage();

	if(conf.MemoryLimit() < usage + (300 * 1024))
	{
	    VERBOSE("MemoryLimit: " << "settings: " << conf.MemoryLimit() << ", too small");

	    // increase + 300Kb
	    conf.SetMemoryLimit(usage + (300 * 1024));

	    VERBOSE("MemoryLimit: " << "settings: " << "increase limit on 300kb, current value: " << conf.MemoryLimit());
	}
    }

    if((redraw | force) & REDRAW_BORDER) borderWindow.Redraw();

    redraw = 0;
}

void Interface::Basic::RedrawSystemInfo(s16 cx, s16 cy, u32 usage)
{
    std::ostringstream os;

    os << "mem. usage: " << usage / 1024 << "Kb" << ", cur. time: ";

    time_t rawtime;
    std::time(&rawtime);
    // strtime format: Www Mmm dd hh:mm:ss yyyy
    const char* strtime = std::ctime(&rawtime);

    // draw info
    os << std::string(&strtime[11], 8);

    system_info.Set(os.str());
    system_info.Blit(cx, cy);
}

s32 Interface::Basic::GetDimensionDoorDestination(const s32 from, const u8 distance) const
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    Settings & conf = Settings::Get();
    LocalEvent & le = LocalEvent::Get();
    s32 dst = -1;

    while(le.HandleEvents())
    {
	const Point & mp = le.GetMouseCursor();
	dst = gameArea.GetIndexFromMousePoint(mp);
	if(0 > dst) break;

	const Maps::Tiles & tile = world.GetTiles(dst);

	const bool valid = ((gameArea.GetArea() & mp) &&
			dst >= 0 &&
			(! tile.isFog(conf.MyColor())) &&
			MP2::isClearGroundObject(tile.GetObject()) &&
			Maps::Ground::WATER != world.GetTiles(dst).GetGround() &&
			distance >= Maps::GetApproximateDistance(from, dst));

	cursor.SetThemes(valid ? Cursor::MOVE : Cursor::WAR_NONE);

	// exit
	if(le.MousePressRight()) break;
	else
	if(Cursor::MOVE == cursor.Themes() && le.MouseClickLeft()) return dst;

	// redraw cursor
        if(!cursor.isVisible())
	{
    	    cursor.Show();
	    display.Flip();
        }
    }

    return -1;
}

void Interface::FixOutOfDisplay(const Rect & rt, s16 & ox, s16 & oy)
{
    Display & display = Display::Get();

    if(ox + rt.w < 0) ox = 0;
    else
    if(ox > display.w() - rt.w + BORDERWIDTH) ox = display.w() - rt.w;

    if(oy + rt.h < 0) oy = 0;
    else
    if(oy > display.h() - rt.h + BORDERWIDTH) oy = display.h() - rt.h;
}

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
#include "game_interface.h"
#include "interface_cpanel.h"

Interface::ControlPanel::ControlPanel() : alpha(130)
{
    w = 180;
    h = 36;

    rt_radr.w = 36;
    rt_radr.h = 36;
    rt_icon.w = 36;
    rt_icon.h = 36;
    rt_bttn.w = 36;
    rt_bttn.h = 36;
    rt_stat.w = 36;
    rt_stat.h = 36;
    rt_quit.w = 36;
    rt_quit.h = 36;

    btn_radr.Set(h, h);
    btn_icon.Set(h, h);
    btn_bttn.Set(h, h);
    btn_stat.Set(h, h);
    btn_quit.Set(h, h);

    ResetTheme();
}

void Interface::ControlPanel::ResetTheme(void)
{
    ICN::icn_t icn = Settings::Get().EvilInterface() ? ICN::ADVEBTNS : ICN::ADVBTNS;

    btn_radr.Blit(AGG::GetICN(icn, 4));
    btn_icon.Blit(AGG::GetICN(icn, 0));
    btn_bttn.Blit(AGG::GetICN(icn, 12));
    btn_stat.Blit(AGG::GetICN(icn, 10));
    btn_quit.Blit(AGG::GetICN(icn, 8));

    btn_radr.SetAlpha(alpha);
    btn_icon.SetAlpha(alpha);
    btn_bttn.SetAlpha(alpha);
    btn_stat.SetAlpha(alpha);
    btn_quit.SetAlpha(alpha);
}

Interface::ControlPanel & Interface::ControlPanel::Get(void)
{
    static ControlPanel cp;
    return cp;
}

const Rect & Interface::ControlPanel::GetArea(void)
{
    return *this;
}

void Interface::ControlPanel::SetPos(s16 ox, s16 oy)
{
    x = ox;
    y = oy;

    rt_radr.x = x;
    rt_radr.y = y;
    rt_icon.x = x + 36;
    rt_icon.y = y;
    rt_bttn.x = x + 72;
    rt_bttn.y = y;
    rt_stat.x = x + 108;
    rt_stat.y = y;
    rt_quit.x = x + 144;
    rt_quit.y = y;
}

void Interface::ControlPanel::Redraw(void)
{
    Display & display = Display::Get();

    display.Blit(btn_radr,  x, y);
    display.Blit(btn_icon,  x + 36, y);
    display.Blit(btn_bttn, x + 72, y);
    display.Blit(btn_stat, x + 108, y);
    display.Blit(btn_quit,  x + 144, y);
}

void Interface::ControlPanel::QueueEventProcessing(Game::menu_t & ret)
{
    LocalEvent & le = LocalEvent::Get();

    if(le.MouseClickLeft(rt_radr))	Game::EventSwitchShowRadar();
    else
    if(le.MouseClickLeft(rt_icon))	Game::EventSwitchShowIcons();
    else
    if(le.MouseClickLeft(rt_bttn))	Game::EventSwitchShowButtons();
    else
    if(le.MouseClickLeft(rt_stat))	Game::EventSwitchShowStatus();
    else
    if(le.MouseClickLeft(rt_quit))	Game::EventEndTurn(ret);
}

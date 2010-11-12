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

#include <vector>
#include "cursor.h"
#include "settings.h"
#include "agg.h"
#include "button.h"
#include "game_interface.h"
#include "interface_gamearea.h"
#include "interface_radar.h"
#include "world.h"
#include "puzzle.h"

bool ClosedTilesExists(const Puzzle &, const u8*, const u8*);
void ZoneOpenRandomTiles(Puzzle &, u8 &, const u8*, const u8*);
void ShowStandardDialog(const Puzzle &, const Surface &);
void ShowExtendedDialog(const Puzzle &, const Surface &);
void PuzzlesDraw(const Puzzle &, const Surface &, s16, s16);

Puzzle::Puzzle()
{
}

Puzzle & Puzzle::operator= (const char* str)
{
    while(str && *str)
    {
	*this <<= 1;
	if(*str == 0x31) set(0);
	++str;
    }

    return *this;
}

void Puzzle::Update(u8 open_obelisk, u8 total_obelisk)
{
    u8 open_puzzle = open_obelisk * PUZZLETILES / total_obelisk;
    u8 need_puzzle = open_puzzle > count() ? open_puzzle - count() : 0;

    const u8 zone1_index[] = { 0, 1, 2, 3, 4, 5, 6, 11, 12, 17, 18, 23, 24, 29, 30, 35, 36, 41, 42, 43, 44, 45, 46, 47 };
    const u8 zone2_index[] = { 7, 8, 9, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 38, 39, 40 };
    const u8 zone3_index[] = { 14, 15, 32, 33 };
    const u8 zone4_index[] = { 20, 21, 26, 27 };

    if(need_puzzle && ClosedTilesExists(*this, &zone1_index[0], &zone1_index[23]))
	ZoneOpenRandomTiles(*this, need_puzzle, &zone1_index[0], &zone1_index[23]);

    if(need_puzzle && ClosedTilesExists(*this, &zone2_index[0], &zone2_index[15]))
	ZoneOpenRandomTiles(*this, need_puzzle, &zone2_index[0], &zone2_index[15]);

    if(need_puzzle && ClosedTilesExists(*this, &zone3_index[0], &zone3_index[3]))
	ZoneOpenRandomTiles(*this, need_puzzle, &zone3_index[0], &zone3_index[3]);

    if(need_puzzle && ClosedTilesExists(*this, &zone4_index[0], &zone4_index[3]))
	ZoneOpenRandomTiles(*this, need_puzzle, &zone4_index[0], &zone4_index[3]);
}

void Puzzle::ShowMapsDialog(void) const
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    Cursor::themes_t old_cursor = cursor.Themes();

    if(! Settings::Get().MusicMIDI()) AGG::PlayMusic(MUS::PUZZLE);

    const Surface & sf = world.GetPuzzleSurface();

    cursor.Hide();

    if(display.w() == 640 && display.h() == 480 && !Settings::Get().HideInterface())
	ShowStandardDialog(*this, sf);
    else
	ShowExtendedDialog(*this, sf);

    cursor.SetThemes(old_cursor);
}

bool ClosedTilesExists(const Puzzle & pzl, const u8* it1, const u8* it2)
{
    while(it1 && it2 && it1 <= it2) if(! pzl.test(*it1++)) return true;
    return false;
}

void ZoneOpenRandomTiles(Puzzle & pzl, u8 & opens, const u8* it1, const u8* it2)
{
    std::vector<u8> values;
    values.reserve(25);
    const u8* it = NULL;

    while(opens)
    {
	values.clear();
	it = it1;
	while(it && it2 && it <= it2){ if(! pzl.test(*it)) values.push_back(*it); ++it; }
	if(values.empty()) break;
	pzl.set(*Rand::Get(values));
	--opens;
    }
}

void ShowStandardDialog(const Puzzle & pzl, const Surface & sf)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();

    const Rect & radar_pos = Interface::Radar::Get().GetArea();
    bool evil_interface = Settings::Get().EvilInterface();

    Background back(BORDERWIDTH, BORDERWIDTH, sf.w(), sf.h());
    back.Save();

    display.Blit(AGG::GetICN((evil_interface ? ICN::EVIWPUZL : ICN::VIEWPUZL), 0), radar_pos);
    display.Blit(sf, BORDERWIDTH, BORDERWIDTH);

    Point dst_pt(radar_pos.x + 32, radar_pos.y + radar_pos.h - 37);
    Button buttonExit(dst_pt, (evil_interface ? ICN::LGNDXTRE : ICN::LGNDXTRA), 4, 5);

    buttonExit.Draw();
    PuzzlesDraw(pzl, sf, BORDERWIDTH, BORDERWIDTH);

    cursor.SetThemes(Cursor::POINTER);
    cursor.Show();
    display.Flip();
    LocalEvent & le = LocalEvent::Get();

    while(le.HandleEvents())
    {
        le.MousePressLeft(buttonExit) ? buttonExit.PressDraw() : buttonExit.ReleaseDraw();
        if(le.MouseClickLeft(buttonExit) || HotKeyCloseWindow) break;
    }

    Interface::Basic::Get().SetRedraw(REDRAW_RADAR);

    cursor.Hide();
    back.Restore();
}

void ShowExtendedDialog(const Puzzle & pzl, const Surface & sf)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    bool evil_interface = Settings::Get().EvilInterface();

    Dialog::FrameBorder frameborder;
    frameborder.SetPosition((display.w() - BORDERWIDTH * 2 - sf.w()) / 2,
	    (display.h() - sf.h() - BORDERWIDTH * 2 - 32) / 2,
	    sf.w(),
	    sf.h() + (Settings::Get().QVGA() ? 25 : 32));
    frameborder.Redraw();

    if(evil_interface)
	display.FillRect(80, 80, 80, frameborder.GetArea());
    else
	display.FillRect(128, 64, 32, frameborder.GetArea());
    display.Blit(sf, frameborder.GetArea());

    Button buttonExit(frameborder.GetArea().x + sf.w() / 2 - 40,
	frameborder.GetArea().y + sf.h() + (Settings::Get().QVGA() ? 0 : 5),
	(evil_interface ? ICN::LGNDXTRE : ICN::LGNDXTRA), 4, 5);

    buttonExit.Draw();
    PuzzlesDraw(pzl, sf, frameborder.GetArea().x, frameborder.GetArea().y);

    cursor.SetThemes(Cursor::POINTER);
    cursor.Show();
    display.Flip();
    LocalEvent & le = LocalEvent::Get();

    while(le.HandleEvents())
    {
        le.MousePressLeft(buttonExit) ? buttonExit.PressDraw() : buttonExit.ReleaseDraw();
        if(le.MouseClickLeft(buttonExit) || HotKeyCloseWindow) break;
        if(Settings::Get().QVGA() && le.MouseClickLeft(frameborder.GetArea())) break;
    }
}

void PuzzlesDraw(const Puzzle & pzl, const Surface & sf, s16 dstx, s16 dsty)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();

    // show all for debug
    if(IS_DEVEL()) return;

    u8 alpha = 250;
    LocalEvent & le = LocalEvent::Get();

    while(le.HandleEvents() && 0 < alpha)
    {
        if(Game::AnimateInfrequent(Game::PUZZLE_FADE_DELAY))
        {
    	    cursor.Hide();
	    display.Blit(sf, dstx, dsty);
	    for(size_t ii = 0; ii < pzl.size(); ++ii)
	    {
    		const Sprite & piece = AGG::GetICN(ICN::PUZZLE, ii);
		if(pzl.test(ii))
		{
		    Surface fade(piece.w(), piece.h());
		    fade.SetColorKey();
		    fade.Blit(piece);
		    fade.SetAlpha(alpha);
		    if(Settings::Get().QVGA())
		    display.Blit(fade, dstx + 8 + piece.x() - BORDERWIDTH, dsty + 8 + piece.y() - BORDERWIDTH);
		    else
		    display.Blit(fade, dstx + piece.x() - BORDERWIDTH, dsty + piece.y() - BORDERWIDTH);
		}
		else
		{
		    if(Settings::Get().QVGA())
		    display.Blit(piece, dstx + 8 + piece.x() - BORDERWIDTH, dsty + 8 + piece.y() - BORDERWIDTH);
		    else
		    display.Blit(piece, dstx + piece.x() - BORDERWIDTH, dsty + piece.y() - BORDERWIDTH);
		}
	    }
	    cursor.Show();
    	    display.Flip();
    	    alpha -= 10;
	}
    }
    cursor.Hide();
}

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
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <ctime>
#include "gamedefs.h"
#include "agg.h"
#include "cursor.h"
#include "button.h"
#include "dialog.h"
#include "settings.h"
#include "world.h"
#include "sdlnet.h"
#include "zzlib.h"
#include "game.h"

#define HGS_ID	0xF1F2
#define HGS_MAX	10

struct hgs_t
{
    hgs_t() : days(0), rating(0) {};

    bool operator== (const hgs_t &) const;

    std::string player;
    std::string land;
    u32 localtime;
    u16 days;
    u16 rating;
};

bool hgs_t::operator== (const hgs_t & h) const
{
    return player == h.player && land == h.land && days == h.days;
}

bool RatingSort(const hgs_t & h1, const hgs_t & h2)
{
    return h1.rating < h2.rating;
}

class HGSData : public QueueMessage
{
public:
    HGSData(){};

    bool Load(const char*);
    bool Save(const char*);
    void ScoreRegistry(const std::string &, const std::string &, u16, u16);
    void RedrawList(s16, s16);
private:
    std::vector<hgs_t> list;
};

bool HGSData::Load(const char* fn)
{
    std::ifstream fs(fn, std::ios::binary);

    if(!fs.is_open()) return false;

    fs.seekg(0, std::ios_base::end);
    dtsz = fs.tellg();
    fs.seekg(0, std::ios_base::beg);

    if(data) delete [] data;
    data = new char [dtsz];
    fs.read(data, dtsz);
    fs.close();

#ifdef WITH_ZLIB
    std::vector<char> v;
    if(ZLib::UnCompress(v, data, dtsz))
    {
    	dtsz = v.size();
    	delete [] data;
    	data = new char [dtsz];
    	std::memcpy(data, &v[0], dtsz);
	v.clear();
    }
#endif

    itd1 = data;
    itd2 = data + dtsz;

    u16 byte16;

    // check id
    Pop(byte16);
    if(byte16 != HGS_ID) return false;

    // size
    Pop(byte16);
    list.resize(byte16);

    std::vector<hgs_t>::iterator it1 = list.begin();
    std::vector<hgs_t>::const_iterator it2 = list.end();

    for(; it1 != it2; ++it1)
    {
	hgs_t & hgs = *it1;

	Pop(hgs.player);
	Pop(hgs.land);
	Pop(hgs.localtime);
	Pop(hgs.days);
	Pop(hgs.rating);
    }

    return true;
}

bool HGSData::Save(const char* fn)
{
    std::ofstream fs(fn, std::ios::binary);

    if(!fs.is_open()) return false;

    Reset();

    // set id
    Push(static_cast<u16>(HGS_ID));
    // set size
    Push(static_cast<u16>(list.size()));

    std::vector<hgs_t>::const_iterator it1 = list.begin();
    std::vector<hgs_t>::const_iterator it2 = list.end();

    for(; it1 != it2; ++it1)
    {
	const hgs_t & hgs = *it1;

	Push(hgs.player);
	Push(hgs.land);
	Push(hgs.localtime);
	Push(hgs.days);
	Push(hgs.rating);
    }

#ifdef WITH_ZLIB
    std::vector<char> v;
    if(!ZLib::Compress(v, data, DtSz())) return false;
    fs.write(&v[0], v.size());
#else
    fs.write(data, DtSz());
#endif
    fs.close();

    return true;
}

void HGSData::ScoreRegistry(const std::string & p, const std::string & m, u16 r, u16 s)
{
    hgs_t h;

    h.player = p;
    h.land = m;
    h.localtime = std::time(NULL);
    h.days = r;
    h.rating = s;

    if(IS_DEVEL()) h.player+= " (+)";

    if(list.end() == std::find(list.begin(), list.end(), h)) list.push_back(h);
    if(list.size() > HGS_MAX) list.resize(HGS_MAX);
}

void HGSData::RedrawList(s16 ox, s16 oy)
{
    const Settings & conf = Settings::Get();
    Display & display = Display::Get();

    // image background
    const Sprite &back = AGG::GetICN(ICN::HSBKG, 0);
    display.Blit(back, ox, oy);

    std::sort(list.begin(), list.end(), RatingSort);

    std::vector<hgs_t>::const_iterator it1 = list.begin();
    std::vector<hgs_t>::const_iterator it2 = list.end();

    Text text;
    text.Set(conf.QVGA() ? Font::SMALL : Font::BIG);
    std::string str;

    for(; it1 != it2 && (it1 - list.begin() < HGS_MAX); ++it1)
    {
	const hgs_t & hgs = *it1;

	text.Set(hgs.player);
	text.Blit(ox + (conf.QVGA() ? 45 : 88), oy + (conf.QVGA() ? 33 : 70));

	text.Set(hgs.land);
	text.Blit(ox + (conf.QVGA() ? 170 : 260), oy + (conf.QVGA() ? 33 : 70));

	str.clear();
	String::AddInt(str, hgs.days);
	text.Set(str);
	text.Blit(ox + (conf.QVGA() ? 250 : 420), oy + (conf.QVGA() ? 33 : 70));

	str.clear();
	String::AddInt(str, hgs.rating);
	text.Set(str);
	text.Blit(ox + (conf.QVGA() ? 270 : 480), oy + (conf.QVGA() ? 33 : 70));

	oy += conf.QVGA() ? 20 : 40;
    }
}

Game::menu_t Game::HighScores(void)
{
    const Settings & conf = Settings::Get();
    HGSData hgs;

    std::ostringstream stream;
    stream << conf.LocalPrefix() << SEPARATOR << "files" << SEPARATOR << "save" << SEPARATOR << "fheroes2.hgs";

    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();

    cursor.SetThemes(cursor.POINTER);
    Mixer::Pause();
    AGG::PlayMusic(MUS::MAINMENU);
    hgs.Load(stream.str().c_str());

    const Sprite &back = AGG::GetICN(ICN::HSBKG, 0);

    cursor.Hide();
    display.Fill(0, 0, 0);
    const Point top((display.w() - back.w()) / 2, (display.h() - back.h()) / 2);

    hgs.RedrawList(top.x, top.y);

    LocalEvent & le = LocalEvent::Get();

    Button buttonCampain(top.x + (conf.QVGA() ? 0 : 9), top.y + (conf.QVGA() ? 100 : 315), ICN::HISCORE, 0, 1);
    Button buttonExit(top.x + back.w() - (conf.QVGA() ? 27 : 36), top.y + (conf.QVGA() ? 100 : 315), ICN::HISCORE, 4, 5);

    buttonCampain.Draw();
    buttonExit.Draw();

    cursor.Show();
    display.Flip();

    const u16 rating = GetGameOverScores();
    const u16 days = world.CountDay();
    GameOver::Result & gameResult = GameOver::Result::Get();

    if(rating && (gameResult.GetResult() & GameOver::WINS))
    {
	std::string player("Unknown Hero");
	Dialog::InputString("Your Name", player);
	cursor.Hide();
	if(player.empty()) player = "Unknown Hero";
	hgs.ScoreRegistry(player, Settings::Get().CurrentFileInfo().name, days, rating);
	hgs.Save(stream.str().c_str());
	hgs.RedrawList(top.x, top.y);
	buttonCampain.Draw();
	buttonExit.Draw();
	cursor.Show();
	display.Flip();
	gameResult.Reset();
    }

    // highscores loop
    while(le.HandleEvents())
    {
	// key code info
        if(IS_DEBUG(DBG_ENGINE, DBG_INFO) && le.KeyPress())
        {
            std::string str;
            String::AddInt(str, le.KeyValue());
            Dialog::Message("Key Press:", str, Font::SMALL, Dialog::OK);
        }
	le.MousePressLeft(buttonCampain) ? buttonCampain.PressDraw() : buttonCampain.ReleaseDraw();
	le.MousePressLeft(buttonExit) ? buttonExit.PressDraw() : buttonExit.ReleaseDraw();

	if(le.MouseClickLeft(buttonExit) || HotKeyCloseWindow) return MAINMENU;
    }

    return QUITGAME;
}

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
#include "cursor.h"
#include "settings.h"
#include "text.h"
#include "button.h"
#include "difficulty.h"
#include "world.h"
#include "pocketpc.h"

void UpdateCoordOpponentsInfo(const Point &, std::vector<Rect> &);
void UpdateCoordClassInfo(const Point &, std::vector<Rect> &);

Game::menu_t PocketPC::ScenarioInfo(void)
{
    Game::SetFixVideoMode();

    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    Settings & conf = Settings::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const u16 window_w = 380;
    const u16 window_h = 224;
    
    Dialog::FrameBorder frameborder;
    frameborder.SetPosition((display.w() - window_w) / 2 - BORDERWIDTH, (display.h() - window_h) / 2 - BORDERWIDTH, window_w, window_h);
    frameborder.Redraw();

    const Rect & dst_rt = frameborder.GetArea();
    const Sprite & background = AGG::GetICN(ICN::STONEBAK, 0);
    display.Blit(background, Rect(0, 0, window_w, window_h), dst_rt);

    Button buttonOk(dst_rt.x + dst_rt.w / 2 - 160, dst_rt.y + dst_rt.h - 30, ICN::NGEXTRA, 66, 67);
    Button buttonCancel(dst_rt.x + dst_rt.w / 2 + 60, dst_rt.y + dst_rt.h - 30, ICN::NGEXTRA, 68, 69);

    Text text;
    text.Set(conf.CurrentFileInfo().name, Font::BIG);
    text.Blit(dst_rt.x + (dst_rt.w - text.w()) / 2, dst_rt.y + 5);

    const Rect box(dst_rt.x + 4, dst_rt.y + 24, 372, 200);
    const Point pointDifficultyInfo(dst_rt.x + 4, dst_rt.y + 24);
    const Point pointOpponentInfo(dst_rt.x + 4, dst_rt.y + 94);
    const Point pointClassInfo(dst_rt.x + 4, dst_rt.y + 144);

    std::vector<Rect>::iterator itr;
    std::vector<Rect> coordDifficulty(5);
    std::vector<Rect> coordColors(KINGDOMMAX);
    std::vector<Rect> coordClass(KINGDOMMAX);

    Game::Scenario::RedrawDifficultyInfo(pointDifficultyInfo, false);

    UpdateCoordOpponentsInfo(pointOpponentInfo, coordColors);
    Game::Scenario::RedrawOpponentsInfo(pointOpponentInfo);

    UpdateCoordClassInfo(pointClassInfo, coordClass);
    Game::Scenario::RedrawClassInfo(pointClassInfo, false);

    const Point pointDifficultyNormal(dst_rt.x + 78, dst_rt.y + 21);
    SpriteCursor levelCursor(AGG::GetICN(ICN::NGEXTRA, 62), pointDifficultyNormal);
    levelCursor.Show(pointDifficultyNormal);
    conf.SetGameDifficulty(Difficulty::NORMAL);
                
    coordDifficulty[0] = Rect(dst_rt.x + 1, dst_rt.y + 21,  levelCursor.w(), levelCursor.h());
    coordDifficulty[1] = Rect(pointDifficultyNormal.x, pointDifficultyNormal.y,  levelCursor.w(), levelCursor.h());
    coordDifficulty[2] = Rect(dst_rt.x + 154, dst_rt.y + 21, levelCursor.w(), levelCursor.h());
    coordDifficulty[3] = Rect(dst_rt.x + 231, dst_rt.y + 21, levelCursor.w(), levelCursor.h());
    coordDifficulty[4] = Rect(dst_rt.x + 308, dst_rt.y + 21, levelCursor.w(), levelCursor.h());
                                    
    buttonOk.Draw();
    buttonCancel.Draw();

    cursor.Show();
    display.Flip();

    // mainmenu loop
    while(le.HandleEvents())
    {
        le.MousePressLeft(buttonOk) ? buttonOk.PressDraw() : buttonOk.ReleaseDraw();
        le.MousePressLeft(buttonCancel) ? buttonCancel.PressDraw() : buttonCancel.ReleaseDraw();

        // click cancel
        if(le.MouseClickLeft(buttonCancel) ||
		Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT))
        {
            Settings::Get().SetGameType(Game::UNKNOWN);
            return Game::MAINMENU;
            break;
        }
        else
	// click ok
        if(le.MouseClickLeft(buttonOk) ||
		Game::HotKeyPress(Game::EVENT_DEFAULT_READY))
        {
            DEBUG(DBG_GAME , DBG_INFO, "PocketPC::ScenarioInfo: select maps: " << conf.MapsFile() \
		    << ", difficulty: " << Difficulty::String(conf.GameDifficulty()) << ", select color: " << Color::String(conf.MyColor()));
            conf.FixKingdomRandomRace();
    	    Game::ShowLoadMapsText();
	    cursor.Hide();
            world.LoadMaps(conf.MapsFile());
            return Game::STARTGAME;
            break;
        }
	else
	if(le.MouseClickLeft(box))
	{
	    // select difficulty
	    if(coordDifficulty.end() != (itr = std::find_if(coordDifficulty.begin(), coordDifficulty.end(), std::bind2nd(RectIncludePoint(), le.GetMouseCursor()))))
	    {
		cursor.Hide();
		levelCursor.Move((*itr).x, (*itr).y);
		conf.SetGameDifficulty(Difficulty::Get(itr - coordDifficulty.begin()));
		cursor.Show();
		display.Flip();
	    }
	    else
	    // select color
	    if(coordColors.end() != (itr = std::find_if(coordColors.begin(), coordColors.end(), std::bind2nd(RectIncludePoint(), le.GetMouseCursor()))))
	    {
		Color::color_t color = Color::GetFromIndex(itr - coordColors.begin());
		if(conf.KingdomColors(color) && conf.AllowColors(color))
		{
		    cursor.Hide();
		    switch(conf.GameType())
		    {
			case Game::NETWORK:
			case Game::HOTSEAT:
			    conf.SetPlayersColors(conf.PlayersColors() & color ? conf.PlayersColors() & ~color : conf.PlayersColors() | color);
			    break;
			default:
			    conf.SetMyColor(color);
			    conf.SetPlayersColors(conf.MyColor());
	    		    break;
		    }
		    Game::Scenario::RedrawOpponentsInfo(pointOpponentInfo);
		    cursor.Show();
		    display.Flip();
		}
	    }
	    else
	    // select class
	    if(coordClass.end() != (itr = std::find_if(coordClass.begin(), coordClass.end(), std::bind2nd(RectIncludePoint(), le.GetMouseCursor()))))
	    {
		Color::color_t color = Color::GetFromIndex(itr - coordClass.begin());
		if(conf.AllowChangeRace(color))
		{
		    cursor.Hide();
		    u8 index = 0;
		    Race::race_t race = conf.KingdomRace(color);
		    switch(race)
		    {
			case Race::KNGT: index = 52; race = Race::BARB; break;
			case Race::BARB: index = 53; race = Race::SORC; break;
			case Race::SORC: index = 54; race = Race::WRLK; break;
			case Race::WRLK: index = 55; race = Race::WZRD; break;
			case Race::WZRD: index = 56; race = Race::NECR; break;
			case Race::NECR: index = 58; race = Race::RAND; break;
			case Race::RAND: index = 51; race = Race::KNGT; break;
			default: break;
		    }
		    conf.SetKingdomRace(color, race);
		    Game::Scenario::RedrawClassInfo(pointClassInfo, false);
		    cursor.Show();
		    display.Flip();
		}
	    }
	}
    }

    return Game::QUITGAME;
}

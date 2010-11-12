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
#include <algorithm>

#include "agg.h"
#include "engine.h"
#include "button.h"
#include "dialog.h"
#include "world.h"
#include "cursor.h"
#include "castle.h"
#include "heroes.h"
#include "splitter.h"
#include "maps_tiles.h"
#include "ground.h"
#include "gameevent.h"
#include "game_interface.h"
#include "game_io.h"
#include "settings.h"
#include "route.h"
#include "game_focus.h"
#include "kingdom.h"
#include "localclient.h"

// heroes_action.cpp
u16 DialogWithArtifact(const std::string & hdr, const std::string & msg, const Artifact::artifact_t art, const u16 buttons = Dialog::OK);

namespace Game
{
    Cursor::themes_t GetCursor(const s32);
    void OpenCastle(Castle *castle);
    void OpenHeroes(Heroes *heroes);
    void ShowPathOrStartMoveHero(Heroes *hero, const s32 dst_index);
    menu_t HumanTurn(void);
    bool DiggingForArtifacts(const Heroes & hero);
    void DialogPlayers(const Color::color_t, const std::string &);
    void MoveHeroFromArrowKeys(Heroes & hero, Direction::vector_t direct);

    void MouseCursorAreaClickLeft(s32);
    void FocusHeroesClickLeftAction(Heroes &, s32);
    void FocusCastleClickLeftAction(Castle &, s32);
    void MouseCursorAreaPressRight(s32);

    void StartNewGame(menu_t &);

    void KeyArrowPress(Direction::vector_t);

    void NewWeekDialog(void);
    void ShowEventDay(void);
    void ShowWarningLostTowns(menu_t &);
}

void Game::MoveHeroFromArrowKeys(Heroes & hero, Direction::vector_t direct)
{
    if(Maps::isValidDirection(hero.GetIndex(), direct))
    {
	s32 dst = Maps::GetDirectionIndex(hero.GetIndex(), direct);
	const Maps::Tiles & tile = world.GetTiles(dst);
	bool allow = false;

	switch(tile.GetObject())
	{
    	    case MP2::OBJN_CASTLE:
    	    {
    		const Castle *to_castle = world.GetCastle(dst);
		if(to_castle)
		{
		    dst = to_castle->GetIndex();
		    allow = true;
		}
		break;
	    }

    	    case MP2::OBJ_BOAT:
    	    case MP2::OBJ_CASTLE:
    	    case MP2::OBJ_HEROES:
    	    case MP2::OBJ_MONSTER:
		allow = true;
		break;

	    default:
		allow = (tile.isPassable(&hero) || MP2::isActionObject(tile.GetObject(), hero.isShipMaster()));
		break;
	}

	if(allow) ShowPathOrStartMoveHero(&hero, dst);
    }
}
                                   
void Game::DialogPlayers(const Color::color_t color, const std::string & str)
{
    const Sprite & border = AGG::GetICN(ICN::BRCREST, 6);

    Surface sign(border.w(), border.h());
    sign.Blit(border);

    switch(color)
    {
	    case Color::BLUE:	sign.Blit(AGG::GetICN(ICN::BRCREST, 0), 4, 4); break;
	    case Color::GREEN:	sign.Blit(AGG::GetICN(ICN::BRCREST, 1), 4, 4); break;
	    case Color::RED:	sign.Blit(AGG::GetICN(ICN::BRCREST, 2), 4, 4); break;
	    case Color::YELLOW:	sign.Blit(AGG::GetICN(ICN::BRCREST, 3), 4, 4); break;
	    case Color::ORANGE:	sign.Blit(AGG::GetICN(ICN::BRCREST, 4), 4, 4); break;
	    case Color::PURPLE:	sign.Blit(AGG::GetICN(ICN::BRCREST, 5), 4, 4); break;
   	    default: break;
    }

    Dialog::SpriteInfo("", str, sign);
}

Game::menu_t Game::StartGame(void)
{
    SetFixVideoMode();

    // cursor
    Cursor & cursor = Cursor::Get();
    Settings & conf = Settings::Get();
    Display & display = Display::Get();

    GameOver::Result::Get().Reset();

    cursor.Hide();

    AGG::ICNRegistryFreeObjects();
    AGG::ICNRegistryEnable(false);

    AGG::Cache::Get().ResetMixer();

    // draw interface
    Interface::Basic & I = Interface::Basic::Get();

    Interface::GameArea & areaMaps = I.gameArea;
    areaMaps.Build();

    Game::Focus & global_focus = Focus::Get();
    global_focus.Reset();

    Interface::Radar & radar = I.radar;
    Interface::StatusWindow& statusWin = I.statusWindow;

    I.iconsPanel.ResetIcons();

    radar.Build();

    I.Redraw(REDRAW_ICONS | REDRAW_BUTTONS | REDRAW_BORDER);
    I.iconsPanel.HideIcons();

    Game::menu_t m = ENDTURN;

    while(m == ENDTURN)
    {
	if(!conf.LoadedGameVersion())
	world.NewDay();

	for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color)
	{
	    Kingdom & kingdom = world.GetKingdom(color);

	    if(!kingdom.isPlay() ||
	    (conf.LoadedGameVersion() && color != conf.MyColor())) continue;

	    if(IS_DEBUG(DBG_GAME, DBG_INFO)) kingdom.Dump();

	    radar.SetHide(true);
	    I.SetRedraw(REDRAW_RADAR);
	    conf.SetCurrentColor(color);
	    world.ClearFog(color);
	    kingdom.ActionBeforeTurn();

	    switch(kingdom.Control())
	    {
		case LOCAL:
		    if(Game::HOTSEAT == conf.GameType())
		    {
			cursor.Hide();
			conf.SetMyColor(Color::GRAY);
			I.iconsPanel.HideIcons();
			statusWin.Reset();
			I.SetRedraw(REDRAW_GAMEAREA | REDRAW_STATUS | REDRAW_ICONS);
			I.Redraw();
			display.Flip();
			std::string str = _("%{color} player's turn");
			String::Replace(str, "%{color}", Color::String(color));
			DialogPlayers(color, str);
		    }
		    I.SetRedraw(REDRAW_ICONS);
		    I.iconsPanel.ShowIcons();
		    conf.SetMyColor(color);
		    m = HumanTurn();
		    if(m == ENDTURN && conf.LoadedGameVersion()) conf.SetLoadedGameVersion(false);
		break;

		// AI turn
		default:
        	    if(m == ENDTURN)
		    {
			statusWin.Reset();
			statusWin.SetState(STATUS_AITURN);

			// for pocketpc: show status window
			if(conf.QVGA() && !conf.ShowStatus())
			{
			    conf.SetShowStatus(true);
			    I.SetRedraw(REDRAW_STATUS);
			}

			cursor.Hide();
			cursor.SetThemes(Cursor::WAIT);
			I.Redraw();
			cursor.Show();
			display.Flip();

			kingdom.AITurns();
		    }
		break;
	    }

	    if(m != ENDTURN) break;
	}

	DELAY(10);
    }

    return m == ENDTURN ? QUITGAME : m;
}

/* open castle wrapper */
void Game::OpenCastle(Castle *castle)
{
    if(! castle) return;

    Mixer::Reduce();

    Cursor & cursor = Cursor::Get();
    Kingdom & myKingdom = world.GetMyKingdom();
    const Settings & conf = Settings::Get();
    std::vector<Castle *> & myCastles = myKingdom.GetCastles();
    Display & display = Display::Get();
    std::vector<Castle *>::const_iterator it = std::find(myCastles.begin(), myCastles.end(), castle);
    Game::Focus & globalfocus = Game::Focus::Get();
    Interface::StatusWindow::ResetTimer();
    bool show_position = !Settings::Get().QVGA() && (640 != display.w() || 480 != display.h());
    bool need_fade = !show_position;

    if(it != myCastles.end() || conf.IsUnions(conf.MyColor(), castle->GetColor()))
    {
	Dialog::answer_t result = Dialog::ZERO;

	while(Dialog::CANCEL != result)
	{
	    if(show_position)
	    {
		globalfocus.Set(*it);
		globalfocus.SetRedraw();
		cursor.Hide();
		Interface::Basic::Get().Redraw();
		cursor.Show();
		display.Flip();
		DELAY(100);
	    }

	    if(Settings::Get().ExtLowMemory())
    		AGG::ICNRegistryEnable(true);

	    result = castle->OpenDialog((conf.MyColor() != castle->GetColor()), need_fade);
	    if(need_fade) need_fade = false;

	    if(Settings::Get().ExtLowMemory())
	    {
    		AGG::ICNRegistryEnable(false);
    		AGG::ICNRegistryFreeObjects();
	    }

	    if(it != myCastles.end())
	    {
		if(Dialog::PREV == result)
		{
		    if(it == myCastles.begin()) it = myCastles.end();
		    --it;
		}
		else
		if(Dialog::NEXT == result)
		{
		    ++it;
		    if(it == myCastles.end()) it = myCastles.begin();
		}

		castle = (*it);
	    }
	}
    }

    if(it != myCastles.end())
    {
	globalfocus.Set(*it);
	globalfocus.Set((*it)->GetHeroes());
    }
    globalfocus.SetRedraw();

    Mixer::Enhance();
}

/* open heroes wrapper */
void Game::OpenHeroes(Heroes *hero)
{
    if(! hero) return;

    Mixer::Reduce();

    Cursor & cursor = Cursor::Get();
    Kingdom & myKingdom = world.GetMyKingdom();
    std::vector<Heroes *> & myHeroes = myKingdom.GetHeroes();
    Display & display = Display::Get();
    std::vector<Heroes *>::const_iterator it = std::find(myHeroes.begin(), myHeroes.end(), hero);
    Game::Focus & globalfocus = Game::Focus::Get();
    Interface::StatusWindow::ResetTimer();
    Interface::Basic & I = Interface::Basic::Get();
    bool show_position = !Settings::Get().QVGA() && (640 != display.w() || 480 != display.h());
    bool need_fade = !show_position;

    if(it != myHeroes.end())
    {
	Dialog::answer_t result = Dialog::ZERO;

	while(Dialog::CANCEL != result)
	{
	    if(show_position)
	    {
		globalfocus.Set(*it);
		globalfocus.SetRedraw();
		cursor.Hide();
		I.Redraw();
		cursor.Show();
		display.Flip();
		DELAY(100);
	    }

	    if(Settings::Get().ExtLowMemory())
		AGG::ICNRegistryEnable(true);

	    result = (*it)->OpenDialog(false, need_fade);
	    if(need_fade) need_fade = false;

	    if(Settings::Get().ExtLowMemory())
	    {
    		AGG::ICNRegistryEnable(false);
    		AGG::ICNRegistryFreeObjects();
	    }

	    switch(result)
	    {
		case Dialog::PREV:
	    	    if(it == myHeroes.begin()) it = myHeroes.end();
		    --it;
		    break;

		case Dialog::NEXT:
		    ++it;
		    if(it == myHeroes.end()) it = myHeroes.begin();
		    break;

		case Dialog::DISMISS:
		    AGG::PlaySound(M82::KILLFADE);

		    (*it)->GetPath().Hide();
		    I.SetRedraw(REDRAW_GAMEAREA);

		    (*it)->FadeOut();
		    (*it)->SetFreeman(0);
		    it = myHeroes.begin();
		    result = Dialog::CANCEL;
		    break;

		default: break;
	    }
	}
    }

    if(it != myHeroes.end())
	globalfocus.Set(*it);
    else
        globalfocus.Reset(Game::Focus::HEROES);
    globalfocus.SetRedraw();

    Mixer::Enhance();
}

/* return changee cursor */
Cursor::themes_t Game::GetCursor(const s32 dst_index)
{
    const Maps::Tiles & tile = world.GetTiles(dst_index);
    if(tile.isFog(Settings::Get().MyColor())) return Cursor::POINTER;

    const Game::Focus & focus = Game::Focus::Get();
    const Settings & conf = Settings::Get();
    
    switch(focus.Type())
    {
	case Focus::HEROES:
	{
	    const Heroes & from_hero = focus.GetHeroes();

	    if(from_hero.Modes(Heroes::ENABLEMOVE)) return Cursor::Get().Themes();

	    if(from_hero.isShipMaster())
	    {
		switch(tile.GetObject())
		{
		    case MP2::OBJ_BOAT:
    			return Cursor::POINTER;

		    case MP2::OBJN_CASTLE:
    		    {
    			const Castle *castle = world.GetCastle(dst_index);

    			if(NULL != castle)
    			    return from_hero.GetColor() == castle->GetColor() ? Cursor::CASTLE : Cursor::POINTER;
    		    }
    		    break;

    		    case MP2::OBJ_CASTLE:
    		    {
    			const Castle *castle = world.GetCastle(dst_index);

    			if(NULL != castle)
			    return from_hero.GetColor() == castle->GetColor() ? Cursor::CASTLE : Cursor::POINTER;
        	    }
        	    break;

		    case MP2::OBJ_HEROES:
		    {
			const Heroes * to_hero = world.GetHeroes(dst_index);

    			if(NULL != to_hero && to_hero->isShipMaster())
    			{
			    if(to_hero->GetCenter() == from_hero.GetCenter())
				return Cursor::HEROES;
			    else
			    if(from_hero.GetColor() == to_hero->GetColor())
				return Cursor::DistanceThemes(Cursor::CHANGE, from_hero.GetRangeRouteDays(dst_index));
			    else
			    if(conf.IsUnions(from_hero.GetColor(), to_hero->GetColor()))
			    	return conf.ExtUnionsAllowHeroesMeetings() ? Cursor::CHANGE : Cursor::POINTER;
			    else
			    if(to_hero->AllowBattle())
				return Cursor::DistanceThemes(Cursor::FIGHT, from_hero.GetRangeRouteDays(dst_index));
			}
    		    }
    		    break;

		    case MP2::OBJ_COAST:
			return Cursor::DistanceThemes(Cursor::ANCHOR, from_hero.GetRangeRouteDays(dst_index));

		    default:
			if(MP2::isWaterObject(tile.GetObject()))
			    return Cursor::DistanceThemes(Cursor::REDBOAT, from_hero.GetRangeRouteDays(dst_index));
			else
			if(tile.isPassable(&from_hero))
			    return Cursor::DistanceThemes(Cursor::BOAT, from_hero.GetRangeRouteDays(dst_index));
			else
			    return Cursor::POINTER;
		}
	    }
	    else
	    {
		switch(tile.GetObject())
		{
    		    case MP2::OBJ_MONSTER:
    			return Cursor::DistanceThemes(Cursor::FIGHT, from_hero.GetRangeRouteDays(dst_index));

		    case MP2::OBJN_CASTLE:
    		    {
    			const Castle *castle = world.GetCastle(dst_index);

    			if(NULL != castle)
			{
			    if(from_hero.GetColor() == castle->GetColor())
				return Cursor::CASTLE;
			    else
			    if(conf.IsUnions(from_hero.GetColor(), castle->GetColor()))
			    	return conf.ExtUnionsAllowCastleVisiting() ? Cursor::ACTION : Cursor::POINTER;
			    else
			    if(castle->GetArmy().isValid())
				return Cursor::DistanceThemes(Cursor::FIGHT, from_hero.GetRangeRouteDays(dst_index));
			    else
				return Cursor::DistanceThemes(Cursor::ACTION, from_hero.GetRangeRouteDays(dst_index));
			}
    		    }
    		    break;

    		    case MP2::OBJ_CASTLE:
    		    {
    			const Castle *castle = world.GetCastle(dst_index);

    			if(NULL != castle)
			{
			    if(from_hero.GetColor() == castle->GetColor())
				return Cursor::DistanceThemes(Cursor::ACTION, from_hero.GetRangeRouteDays(dst_index));
			    else
			    if(conf.IsUnions(from_hero.GetColor(), castle->GetColor()))
			    	return conf.ExtUnionsAllowCastleVisiting() ? Cursor::ACTION : Cursor::POINTER;
			    else
			    if(castle->GetArmy().isValid())
				return Cursor::DistanceThemes(Cursor::FIGHT, from_hero.GetRangeRouteDays(dst_index));
			    else
				return Cursor::DistanceThemes(Cursor::ACTION, from_hero.GetRangeRouteDays(dst_index));
			}
        	    }
        	    break;

		    case MP2::OBJ_HEROES:
		    {
			const Heroes * to_hero = world.GetHeroes(dst_index);

    			if(NULL != to_hero && (!to_hero->isShipMaster() ||
			    from_hero.CanPassToShipMaster(*to_hero)))
    			{
			    if(to_hero->GetCenter() == from_hero.GetCenter())
				return Cursor::HEROES;
			    else
			    if(from_hero.GetColor() == to_hero->GetColor())
				return Cursor::DistanceThemes(Cursor::CHANGE, from_hero.GetRangeRouteDays(dst_index));
			    else
			    if(conf.IsUnions(from_hero.GetColor(), to_hero->GetColor()))
			    	return conf.ExtUnionsAllowHeroesMeetings() ? Cursor::CHANGE : Cursor::POINTER;
			    else
				return Cursor::DistanceThemes(Cursor::FIGHT, from_hero.GetRangeRouteDays(dst_index));
			}
    		    }
    		    break;

    		    case MP2::OBJ_BOAT:
    			return Cursor::DistanceThemes(Cursor::BOAT, from_hero.GetRangeRouteDays(dst_index));

		    default:
			if(MP2::isGroundObject(tile.GetObject()))
			{
				bool protection = (MP2::isPickupObject(tile.GetObject()) ? false :
					(Maps::TileUnderProtection(dst_index) || tile.CheckEnemyGuardians(from_hero.GetColor())));

				return Cursor::DistanceThemes((protection ? Cursor::FIGHT : Cursor::ACTION),
								from_hero.GetRangeRouteDays(dst_index));
			}
			else
			if(tile.isPassable(&from_hero))
			{
				bool protection = Maps::TileUnderProtection(dst_index);

				return Cursor::DistanceThemes((protection ? Cursor::FIGHT : Cursor::MOVE),
								from_hero.GetRangeRouteDays(dst_index));
			}
			else
				return Cursor::POINTER;
		}
	    }
	}
	break;

	case Focus::CASTLE:
	{
	    const Castle & from_castle = focus.GetCastle();

	    switch(tile.GetObject())
	    {
    		case MP2::OBJN_CASTLE:
    		case MP2::OBJ_CASTLE:
    		{
    		    const Castle *to_castle = world.GetCastle(dst_index);

    		    if(NULL != to_castle)
    			return to_castle->GetColor() == from_castle.GetColor() ? Cursor::CASTLE : Cursor::POINTER;
		}
		break;

		case MP2::OBJ_HEROES:
    		{
    		    const Heroes *heroes = world.GetHeroes(dst_index);

		    if(NULL != heroes)
    			return heroes->GetColor() == from_castle.GetColor() ? Cursor::HEROES : Cursor::POINTER;
		}
		break;

		default:
		    return Cursor::POINTER;
	    }
	}
	break;

    	default:
    	break;
    }

    return Cursor::POINTER;
}

void Game::ShowPathOrStartMoveHero(Heroes *hero, const s32 dst_index)
{
    if(!hero) return;

    Route::Path & path = hero->GetPath();
    Interface::Basic & I = Interface::Basic::Get();
    Cursor & cursor = Cursor::Get();

    // show path
    if(path.GetDestinationIndex() != dst_index)
    {
        hero->SetMove(false);
	path.Calculate(dst_index);
        if(IS_DEBUG(DBG_GAME, DBG_TRACE)) path.Dump();
        path.Show();
	I.SetRedraw(REDRAW_GAMEAREA);
	cursor.SetThemes(Game::GetCursor(dst_index));
    }
    // start move
    else
    if(path.isValid())
    {
        Game::Focus::Get().Set(hero);
        Game::Focus::Get().SetRedraw();
        hero->SetMove(true);
	cursor.SetThemes(Cursor::WAIT);
    }
}

Game::menu_t Game::HumanTurn(void)
{
    Game::Focus & global_focus = Focus::Get();

    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    Settings & conf = Settings::Get();

    LocalEvent & le = LocalEvent::Get();
    
    Game::menu_t res = CANCEL;

    cursor.Hide();
    Interface::Basic & I = Interface::Basic::Get();

    Kingdom & myKingdom = world.GetMyKingdom();
    const std::vector<Castle *> & myCastles = myKingdom.GetCastles();
    const std::vector<Heroes *> & myHeroes = myKingdom.GetHeroes();

    GameOver::Result & gameResult = GameOver::Result::Get();

    // set focus
    if(Game::HOTSEAT == conf.GameType()) global_focus.Reset();

    if(!conf.ExtRememberLastFocus() && myHeroes.size())
	global_focus.Set(myHeroes.front());
    else
	global_focus.Reset(Focus::HEROES);
    if(Focus::HEROES == global_focus.Type() && global_focus.GetHeroes().GetPath().isValid()) global_focus.GetHeroes().GetPath().Show();

    I.radar.SetHide(false);
    I.statusWindow.Reset();
    I.gameArea.SetUpdateCursor();
    I.Redraw(REDRAW_GAMEAREA | REDRAW_RADAR | REDRAW_ICONS | REDRAW_BUTTONS | REDRAW_STATUS | REDRAW_BORDER);

    AGG::PlayMusic(MUS::FromGround(world.GetTiles(global_focus.Center()).GetGround()));
    Game::EnvironmentSoundMixer();

    cursor.Show();
    display.Flip();

    if(!conf.LoadedGameVersion())
    {
	// new week dialog
	if(1 < world.CountWeek() && world.BeginWeek())
	    NewWeekDialog();

	// show event day
	 ShowEventDay();

	// check game over
	gameResult.CheckGameOver(res);
    }

    // warning lost all town
    if(myCastles.empty()) ShowWarningLostTowns(res);

    // check around actions (and skip for h2 orig, bug?)
    if(!conf.ExtOnlyFirstMonsterAttack()) myKingdom.HeroesActionNewPosition();

    // auto hide status
    bool autohide_status = conf.QVGA() && conf.ShowStatus();
    if(autohide_status) AnimateDelayReset(AUTOHIDE_STATUS_DELAY);

    // startgame loop
    while(CANCEL == res && le.HandleEvents())
    {
	// for pocketpc: auto hide status if start turn
	if(autohide_status && AnimateInfrequent(AUTOHIDE_STATUS_DELAY))
	{
	    EventSwitchShowStatus();
	    autohide_status = false;
	}

	// hot keys
	if(le.KeyPress())
	{
#ifdef WITHOUT_MOUSE
	    if(HotKeyPress(EVENT_EMULATETOGGLE)) le.ToggleEmulateMouse();
	    else
#endif
	    // exit dialog
	    if(HotKeyPress(EVENT_DEFAULT_EXIT)) EventExit(res);
	    else
    	    // end turn
	    if(HotKeyPress(EVENT_ENDTURN)) EventEndTurn(res);
	    else
    	    // next hero
	    if(HotKeyPress(EVENT_NEXTHERO)) EventNextHero();
	    else
    	    // next town
	    if(HotKeyPress(EVENT_NEXTTOWN)) EventNextTown();
	    else
	    // hero movement
	    if(HotKeyPress(EVENT_CONTINUE)) EventContinueMovement();
	    else
	    // save game
	    if(HotKeyPress(EVENT_SAVEGAME)) EventSaveGame();
	    else
	    // load game
	    if(HotKeyPress(EVENT_LOADGAME))
	    {
		EventLoadGame(res);
		if(res != CANCEL) break;
	    }
	    else
	    // file options
	    if(HotKeyPress(EVENT_FILEOPTIONS)) EventFileDialog(res);
	    else
	    // system options
	    if(HotKeyPress(EVENT_SYSTEMOPTIONS)) EventSystemDialog();
	    else
	    // puzzle map
	    if(HotKeyPress(EVENT_PUZZLEMAPS)) EventPuzzleMaps();
	    else
	    // info game
	    if(HotKeyPress(EVENT_INFOGAME)) EventGameInfo();
	    else
	    // dig artifact
	    if(HotKeyPress(EVENT_DIGARTIFACT)) EventDigArtifact(res);
	    else
	    // cast spell
	    if(HotKeyPress(EVENT_CASTSPELL)) EventCastSpell();
	    else
    	    // show/hide control panel
	    if(HotKeyPress(EVENT_CTRLPANEL)) EventSwitchShowControlPanel();
	    else
	    // hide/show radar
	    if(HotKeyPress(EVENT_SHOWRADAR)) EventSwitchShowRadar();
	    else
	    // hide/show buttons
	    if(HotKeyPress(EVENT_SHOWBUTTONS)) EventSwitchShowButtons();
	    else
	    // hide/show status window
	    if(HotKeyPress(EVENT_SHOWSTATUS)) EventSwitchShowStatus();
	    else
	    // hide/show hero/town icons
	    if(HotKeyPress(EVENT_SHOWICONS)) EventSwitchShowIcons();
	    else
	    // move hero
	    if(HotKeyPress(EVENT_MOVELEFT)) KeyArrowPress(Direction::LEFT);
	    else
	    if(HotKeyPress(EVENT_MOVERIGHT)) KeyArrowPress(Direction::RIGHT);
	    else
	    if(HotKeyPress(EVENT_MOVETOP)) KeyArrowPress(Direction::TOP);
	    else
	    if(HotKeyPress(EVENT_MOVEBOTTOM)) KeyArrowPress(Direction::BOTTOM);
	    else
	    if(HotKeyPress(EVENT_MOVETOPLEFT)) KeyArrowPress(Direction::TOP_LEFT);
	    else
	    if(HotKeyPress(EVENT_MOVETOPRIGHT)) KeyArrowPress(Direction::TOP_RIGHT);
	    else
	    if(HotKeyPress(EVENT_MOVEBOTTOMLEFT)) KeyArrowPress(Direction::BOTTOM_LEFT);
	    else
	    if(HotKeyPress(EVENT_MOVEBOTTOMRIGHT)) KeyArrowPress(Direction::BOTTOM_RIGHT);
	    else
	    // scroll maps
	    if(HotKeyPress(EVENT_SCROLLLEFT)) I.gameArea.SetScroll(SCROLL_LEFT);
	    else
	    if(HotKeyPress(EVENT_SCROLLRIGHT)) I.gameArea.SetScroll(SCROLL_RIGHT);
	    else
	    if(HotKeyPress(EVENT_SCROLLUP)) I.gameArea.SetScroll(SCROLL_TOP);
	    else
	    if(HotKeyPress(EVENT_SCROLLDOWN)) I.gameArea.SetScroll(SCROLL_BOTTOM);
	    // default action
	    else
	    if(HotKeyPress(EVENT_DEFAULTACTION)) EventDefaultAction();
	    // open focus
	    else
	    if(HotKeyPress(EVENT_OPENFOCUS)) EventOpenFocus();
	    // switch group
	    else
	    if(HotKeyPress(EVENT_SWITCHGROUP)) EventSwitchGroup();
	}

	if(conf.ExtTapMode())
	{
	    // scroll area maps left
	    if(le.MouseCursor(I.GetAreaScrollLeft()) && le.MousePressLeft()) I.gameArea.SetScroll(SCROLL_LEFT);
    	    else
	    // scroll area maps right
	    if(le.MouseCursor(I.GetAreaScrollRight()) && le.MousePressLeft()) I.gameArea.SetScroll(SCROLL_RIGHT);
	    else
	    // scroll area maps top
	    if(le.MouseCursor(I.GetAreaScrollTop()) && le.MousePressLeft()) I.gameArea.SetScroll(SCROLL_TOP);
	    else
	    // scroll area maps bottom
	    if(le.MouseCursor(I.GetAreaScrollBottom()) && le.MousePressLeft()) I.gameArea.SetScroll(SCROLL_BOTTOM);

	    // disable right click emulation
	    if(I.gameArea.NeedScroll()) le.SetTapMode(false);
	}
	else
	{
	    // scroll area maps left
	    if(le.MouseCursor(I.GetAreaScrollLeft())) I.gameArea.SetScroll(SCROLL_LEFT);
    	    else
	    // scroll area maps right
	    if(le.MouseCursor(I.GetAreaScrollRight())) I.gameArea.SetScroll(SCROLL_RIGHT);
	    else
	    // scroll area maps top
	    if(le.MouseCursor(I.GetAreaScrollTop())) I.gameArea.SetScroll(SCROLL_TOP);
	    else
	    // scroll area maps bottom
	    if(le.MouseCursor(I.GetAreaScrollBottom())) I.gameArea.SetScroll(SCROLL_BOTTOM);
	}

	// cursor over radar
        if((!conf.HideInterface() || conf.ShowRadar()) &&
           le.MouseCursor(I.radar.GetArea()))
	{
	    if(Cursor::POINTER != cursor.Themes())
	    {
		cursor.SetThemes(Cursor::POINTER);
	    }
	    I.radar.QueueEventProcessing();
	}
	else
	// cursor over icons panel
        if((!conf.HideInterface() || conf.ShowIcons()) &&
           le.MouseCursor(I.iconsPanel.GetArea()))
	{
	    if(Cursor::POINTER != cursor.Themes())
	    {
		cursor.SetThemes(Cursor::POINTER);
	    }
	    I.iconsPanel.QueueEventProcessing();
	}
	else
	// cursor over buttons area
        if((!conf.HideInterface() || conf.ShowButtons()) &&
           le.MouseCursor(I.buttonsArea.GetArea()))
	{
	    if(Cursor::POINTER != cursor.Themes())
	    {
		cursor.SetThemes(Cursor::POINTER);
	    }
	    I.buttonsArea.QueueEventProcessing(res);
	}
	else
        // cursor over status area
        if((!conf.HideInterface() || conf.ShowStatus()) &&
           le.MouseCursor(I.statusWindow.GetArea()))
	{
	    if(Cursor::POINTER != cursor.Themes())
	    {
		cursor.SetThemes(Cursor::POINTER);
	    }
	    I.statusWindow.QueueEventProcessing();
	}
	else
        // cursor over control panel
        if(conf.HideInterface() && conf.ShowControlPanel() &&
           le.MouseCursor(I.controlPanel.GetArea()))
	{
	    if(Cursor::POINTER != cursor.Themes())
	    {
		cursor.SetThemes(Cursor::POINTER);
	    }
	    I.controlPanel.QueueEventProcessing(res);
	}
	else
	// cursor over game area
	if(le.MouseCursor(I.gameArea.GetArea()) && !I.gameArea.NeedScroll())
	{
    	    I.gameArea.QueueEventProcessing();
	}

        // fast scroll
	if(I.gameArea.NeedScroll() && AnimateInfrequent(SCROLL_DELAY))
        {
    	    cursor.Hide();

	    if(le.MouseCursor(I.GetAreaScrollLeft()) ||
		   le.MouseCursor(I.GetAreaScrollRight()) ||
		   le.MouseCursor(I.GetAreaScrollTop()) ||
		   le.MouseCursor(I.GetAreaScrollBottom()))
    		    cursor.SetThemes(I.gameArea.GetScrollCursor());

    	    I.gameArea.Scroll();

    	    // need stop hero
    	    if(Game::Focus::HEROES == global_focus.Type() && global_focus.GetHeroes().isEnableMove())
    		    global_focus.GetHeroes().SetMove(false);

	    I.SetRedraw(REDRAW_GAMEAREA|REDRAW_RADAR);
    	    I.Redraw();
    	    cursor.Show();
    	    display.Flip();

            // enable right click emulation
	    if(conf.ExtTapMode())
        	le.SetTapMode(true);

	    continue;
        }

	// heroes move animation
        if(AnimateInfrequent(CURRENT_HERO_DELAY))
        {
    	    if(Game::Focus::HEROES == global_focus.Type())
	    {
		Heroes & hero = global_focus.GetHeroes();
		if(hero.isEnableMove())
		{
		    if(hero.Move(0 == conf.HeroesMoveSpeed()))
		    {
            		I.gameArea.Center(global_focus.Center());
            		global_focus.Reset(Focus::HEROES);
            		global_focus.SetRedraw();

            		I.gameArea.SetUpdateCursor();
		    }
		    else
		    {
			I.SetRedraw(REDRAW_GAMEAREA);
		    }

		    if(hero.isAction())
		    {
			// check game over
			gameResult.CheckGameOver(res);
			hero.ResetAction();
		    }
		}
		else
		{
		    hero.SetMove(false);
		    if(Cursor::WAIT == cursor.Themes()) cursor.SetThemes(Cursor::POINTER);
		}
    	    }
	}

	// slow maps objects animation
        if(AnimateInfrequent(MAPS_DELAY))
	{
	    Maps::IncreaseAnimationTicket();
	    I.SetRedraw(REDRAW_GAMEAREA);
	}

	if(I.NeedRedraw())
	{
    	    cursor.Hide();
    	    I.Redraw();
    	    cursor.Show();
    	    display.Flip();
	}
	else
	if(!cursor.isVisible())
	{
    	    cursor.Show();
    	    display.Flip();
	}
    }

    if(ENDTURN == res)
    {
	// warning lost all town
	if(myHeroes.size() && myCastles.empty() && Game::GetLostTownDays() < myKingdom.GetLostTownDays())
	{
	    std::string str = _("%{color} player, you have lost your last town. If you do not conquer another town in next week, you will be eliminated.");
	    String::Replace(str, "%{color}", Color::String(conf.MyColor()));
	    DialogPlayers(conf.MyColor(), str);
	}

	if(Game::Focus::HEROES == global_focus.Type())
	{
	    global_focus.GetHeroes().ShowPath(false);
	    global_focus.SetRedraw();
	}

	if(conf.ExtAutoSaveOn())
	{
	    std::string filename(conf.LocalPrefix() + SEPARATOR + "files" + SEPARATOR + "save" + SEPARATOR +  "autosave.sav");
	    Game::Save(filename);
	}
    }

    return res;
}

bool Game::DiggingForArtifacts(const Heroes & hero)
{
    if(hero.GetMaxMovePoints() == hero.GetMovePoints())
    {
	if(! world.GetTiles(hero.GetCenter()).GoodForUltimateArtifact())
	{
	    Dialog::Message("", _("Try searching on clear ground."), Font::BIG, Dialog::OK);
	    return false;
	}

	AGG::PlaySound(M82::DIGSOUND);

	const_cast<Heroes &>(hero).ResetMovePoints();
	const Artifact ultimate(world.GetUltimateArtifact());

	if(world.DiggingForUltimateArtifact(hero.GetCenter()) && ultimate != Artifact::UNKNOWN)
	{
	    AGG::PlaySound(M82::TREASURE);
	    // check returns
	    const_cast<Heroes &>(hero).PickupArtifact(ultimate());
	    std::string msg(_("After spending many hours digging here, you have uncovered the "));
	    msg.append(ultimate.GetName());
	    DialogWithArtifact(_("Congratulations!"), msg, ultimate());
	}
	else
	    Dialog::Message("", _("Nothing here. Where could it be?"), Font::BIG, Dialog::OK);

	Cursor::Get().Hide();
	Interface::IconsPanel::Get().RedrawIcons(ICON_HEROES);
	Cursor::Get().Show();
	Display::Get().Flip();
    }
    else
	Dialog::Message("", _("Digging for artifacts requires a whole day, try again tomorrow."), Font::BIG, Dialog::OK);

    return false;
}

void Game::MouseCursorAreaClickLeft(s32 index_maps)
{
    Game::Focus & global_focus = Focus::Get();
    switch(global_focus.Type())
    {
	case Focus::HEROES:	FocusHeroesClickLeftAction(global_focus.GetHeroes(), index_maps); break;
	case Focus::CASTLE:	FocusCastleClickLeftAction(global_focus.GetCastle(), index_maps); break;
	default: break;
    }
}

void Game::FocusHeroesClickLeftAction(Heroes & from_hero, s32 index_maps)
{
    Game::Focus & global_focus = Focus::Get();
    Maps::Tiles & tile = world.GetTiles(index_maps);

    switch(tile.GetObject())
    {
	// from hero to castle
	case MP2::OBJN_CASTLE:
	{
    	    const Castle *to_castle = world.GetCastle(index_maps);
	    if(NULL == to_castle) break;
	    else
	    if(from_hero.GetColor() == to_castle->GetColor())
	    {
		global_focus.Set(const_cast<Castle *>(to_castle));
		global_focus.SetRedraw();
	    }
	    else
		ShowPathOrStartMoveHero(&from_hero, to_castle->GetIndex());
	}
	break;

    	// from hero to hero
    	case MP2::OBJ_HEROES:
	{
	    const Heroes * to_hero = world.GetHeroes(index_maps);
	    if(NULL == to_hero) break;
	    else
	    if(from_hero.GetCenter() == to_hero->GetCenter())
		OpenHeroes(&from_hero);
	    else
		ShowPathOrStartMoveHero(&from_hero, index_maps);
    	}
	break;

	default:
	    if(tile.isPassable(&from_hero) || MP2::isActionObject(tile.GetObject(), from_hero.isShipMaster()))
		ShowPathOrStartMoveHero(&from_hero, index_maps);
	    break;
    }
}

void Game::FocusCastleClickLeftAction(Castle & from_castle, s32 index_maps)
{
    Game::Focus & global_focus = Focus::Get();
    Maps::Tiles & tile = world.GetTiles(index_maps);

    switch(tile.GetObject())
    {
	// from castle to castle
	case MP2::OBJN_CASTLE:
	case MP2::OBJ_CASTLE:
	{
	    const Castle *to_castle = world.GetCastle(index_maps);
	    if(NULL != to_castle &&
		from_castle.GetColor() == to_castle->GetColor())
    	    {
		// is selected open dialog
		if(from_castle.GetCenter() == to_castle->GetCenter())
		    OpenCastle(&from_castle);
		// select other castle
		else
		{
		    global_focus.Set(const_cast<Castle *>(to_castle));
		    global_focus.SetRedraw();
		}
	    }
	}
	break;

	// from castle to heroes
	case MP2::OBJ_HEROES:
	{
	    const Heroes *to_hero = world.GetHeroes(index_maps);
	    if(NULL != to_hero &&
		from_castle.GetColor() == to_hero->GetColor())
	    {
    		global_focus.Set(const_cast<Heroes *>(to_hero));
    		global_focus.SetRedraw();
    	    }
	}
	break;

	default: break;
    }
}

void Game::MouseCursorAreaPressRight(s32 index_maps)
{
    Focus & global_focus = Focus::Get();

    // stop hero
    if(Game::Focus::HEROES == global_focus.Type() && global_focus.GetHeroes().isEnableMove())
    {
	global_focus.GetHeroes().SetMove(false);
	Cursor::Get().SetThemes(Game::GetCursor(index_maps));
    }
    else
    {
	Settings & conf = Settings::Get();
	Maps::Tiles & tile = world.GetTiles(index_maps);

	if(IS_DEVEL()) tile.DebugInfo();

	if(!IS_DEVEL() && tile.isFog(conf.MyColor()))
	    Dialog::QuickInfo(tile);
	else
	switch(tile.GetObject())
	{
	    case MP2::OBJN_CASTLE:
	    case MP2::OBJ_CASTLE:
	    {
    		const Castle *castle = world.GetCastle(tile.GetIndex());
		if(castle) Dialog::QuickInfo(*castle);
	    }
	    break;

	    case MP2::OBJ_HEROES:
    	    {
		const Heroes *heroes = world.GetHeroes(tile.GetIndex());
		if(heroes) Dialog::QuickInfo(*heroes);
	    }
	    break;

	    default:
		Dialog::QuickInfo(tile);
	    break;
	}
    }
}

void Game::EventNextHero(void)
{
    Game::Focus & global_focus = Focus::Get();

    const Kingdom & myKingdom = world.GetMyKingdom();
    const std::vector<Heroes *> & myHeroes = myKingdom.GetHeroes();

    if(myHeroes.empty()) return;

    if(Game::Focus::HEROES != global_focus.Type())
    {
	global_focus.Reset(Game::Focus::HEROES);
    }
    else
    {
	std::vector<Heroes *>::const_iterator it = std::find(myHeroes.begin(), myHeroes.end(), &global_focus.GetHeroes());
	++it;
	if(it == myHeroes.end()) it = myHeroes.begin();
	global_focus.Set(*it);
    }
    global_focus.SetRedraw();
}

void Game::EventContinueMovement(void)
{
    Game::Focus & global_focus = Focus::Get();
    if(Game::Focus::HEROES == global_focus.Type() &&
	global_focus.GetHeroes().GetPath().isValid())
	global_focus.GetHeroes().SetMove(!global_focus.GetHeroes().isEnableMove());
}

void Game::EventKingdomInfo(void)
{
}

void Game::EventCastSpell(void)
{
    Game::Focus & global_focus = Focus::Get();
    Interface::Basic & I = Interface::Basic::Get();

    if(Game::Focus::HEROES == global_focus.Type())
    {
	Heroes & hero = global_focus.GetHeroes();
	// apply cast spell
	hero.ActionSpellCast(hero.OpenSpellBook(SpellBook::ADVN, true));
	I.SetRedraw(REDRAW_ICONS);
    }
}

void Game::EventEndTurn(Game::menu_t & ret)
{
    Game::Focus & global_focus = Focus::Get();
    const Kingdom & myKingdom = world.GetMyKingdom();

    if(Game::Focus::HEROES == global_focus.Type())
	global_focus.GetHeroes().SetMove(false);

    if(!myKingdom.HeroesMayStillMove() ||
	Dialog::YES == Dialog::Message("", _("One or more heroes may still move, are you sure you want to end your turn?"), Font::BIG, Dialog::YES | Dialog::NO))
	ret = ENDTURN;
}

void Game::EventAdventureDialog(Game::menu_t & ret)
{
    Game::Focus & global_focus = Focus::Get();
    Mixer::Reduce();
    switch(Dialog::AdventureOptions(Game::Focus::HEROES == global_focus.Type()))
    {
	case Dialog::WORLD:
	    break;

	case Dialog::PUZZLE: 
	    EventPuzzleMaps();
	    break;

	case Dialog::INFO:
	    EventGameInfo();
	    break;

	case Dialog::DIG:
	    EventDigArtifact(ret);
	    break;

	default: break;
    }
    Mixer::Enhance();
}

void Game::StartNewGame(Game::menu_t & ret)
{
    if(Dialog::YES == Dialog::Message("", _("Are you sure you want to restart? (Your current game will be lost)"), Font::BIG, Dialog::YES|Dialog::NO))
	ret = NEWGAME;
}

void Game::EventFileDialog(Game::menu_t & ret)
{
    switch(Dialog::FileOptions())
    {
	case NEWGAME:
	    StartNewGame(ret);
	    break;

	case QUITGAME:
	    ret = QUITGAME;
	    break;

	case LOADGAME:
	    EventLoadGame(ret);
	    break;

	case SAVEGAME:
	    EventSaveGame();
	    break;

	default:
	break;
    }
}

void Game::EventSystemDialog(void)
{
    // Change and save system settings
    const u8 changes = Dialog::SystemOptions();
    Interface::Basic & I = Interface::Basic::Get();

    if(0x10 & changes)
    {
	Game::Focus & focus = Game::Focus::Get();
	// hardcore reset pos
	I.gameArea.Center(0, 0);
	I.gameArea.Center(focus.Center());
        I.SetRedraw(REDRAW_GAMEAREA);

	if(Settings::Get().HideInterface())
	    I.controlPanel.ResetTheme();
    }

    if(0x08 & changes)
        I.SetRedraw(REDRAW_ICONS | REDRAW_BUTTONS | REDRAW_STATUS | REDRAW_BORDER);
}

void Game::EventExit(menu_t & ret)
{
    Focus & global_focus = Focus::Get();

    // stop hero
    if(Game::Focus::HEROES == global_focus.Type() && global_focus.GetHeroes().isEnableMove())
	global_focus.GetHeroes().SetMove(false);
    else
    if(Dialog::YES & Dialog::Message("", _("Are you sure you want to quit?"), Font::BIG, Dialog::YES|Dialog::NO))
	ret = QUITGAME;
}

void Game::EventNextTown(void)
{
    Kingdom & myKingdom = world.GetMyKingdom();
    std::vector<Castle *> & myCastles = myKingdom.GetCastles();

    if(myCastles.size())
    {
	Focus & global_focus = Focus::Get();

	if(Game::Focus::CASTLE != global_focus.Type())
	    global_focus.Reset(Game::Focus::CASTLE);
	else
	{
	    std::vector<Castle *>::const_iterator it = std::find(myCastles.begin(), myCastles.end(), &global_focus.GetCastle());
    	    ++it;
    	    if(it == myCastles.end()) it = myCastles.begin();
	    global_focus.Set(*it);
        }
        global_focus.SetRedraw();
    }
}

void Game::EventSaveGame(void)
{
    std::string filename;
    if(Dialog::SelectFileSave(filename) && filename.size())
    {
	Game::Save(filename);
	Dialog::Message("", _("Game saved successfully."), Font::BIG, Dialog::OK);
    }
}

void Game::EventLoadGame(menu_t & ret)
{
    if(Dialog::YES == Dialog::Message("", _("Are you sure you want to load a new game? (Your current game will be lost)"), Font::BIG, Dialog::YES|Dialog::NO))
    ret = LOADGAME;
}

void Game::EventPuzzleMaps(void)
{
    world.GetMyKingdom().PuzzleMaps().ShowMapsDialog();
}

void Game::EventGameInfo(void)
{
    Dialog::GameInfo();
}

void Game::EventDigArtifact(menu_t & ret)
{
    Focus & global_focus = Focus::Get();
    if(Game::Focus::HEROES == global_focus.Type())
    {
	DiggingForArtifacts(global_focus.GetHeroes());
	// check game over for ultimate artifact
	GameOver::Result::Get().CheckGameOver(ret);
    }
}

void Game::EventDefaultAction(void)
{
    Focus & global_focus = Focus::Get();
    if(Game::Focus::HEROES == global_focus.Type())
    {
	Heroes & hero = global_focus.GetHeroes();
	Interface::Basic & I = Interface::Basic::Get();

	// 1. continue
        if(global_focus.GetHeroes().GetPath().isValid())
    	    global_focus.GetHeroes().SetMove(true);
	else
	// 2. action object
	if(MP2::isActionObject(hero.GetUnderObject(), hero.isShipMaster()))
	{
	    hero.Action(hero.GetIndex());
	    if(MP2::OBJ_STONELIGHTS == hero.GetUnderObject() || MP2::OBJ_WHIRLPOOL == hero.GetUnderObject())
	    {
		hero.ApplyPenaltyMovement();
		I.SetRedraw(REDRAW_HEROES);
	    }
	    I.SetRedraw(REDRAW_GAMEAREA);
	}
	else
	// 3. hero dialog
	    OpenHeroes(&hero);
    }
    else
    // 4. town dialog
    if(Game::Focus::CASTLE == global_focus.Type())
    {
	Castle & castl = global_focus.GetCastle();
	OpenCastle(&castl);
    }
}

void Game::EventOpenFocus(void)
{
    Focus & global_focus = Focus::Get();
    if(Game::Focus::HEROES == global_focus.Type())
    {
	Heroes & hero = global_focus.GetHeroes();
	OpenHeroes(&hero);
    }
    else
    if(Game::Focus::CASTLE == global_focus.Type())
    {
	Castle & castl = global_focus.GetCastle();
	OpenCastle(&castl);
    }
}

void Game::KeyArrowPress(Direction::vector_t dir)
{
    Focus & global_focus = Focus::Get();
    Interface::GameArea & area = Interface::GameArea::Get();

    // move hero
    if(Focus::HEROES == global_focus.Type()) MoveHeroFromArrowKeys(global_focus.GetHeroes(), dir);
    else
    // scroll map
    switch(dir)
    {
	case Direction::TOP_LEFT:	area.SetScroll(SCROLL_TOP); area.SetScroll(SCROLL_LEFT); break;
	case Direction::TOP:		area.SetScroll(SCROLL_TOP); break;
        case Direction::TOP_RIGHT:	area.SetScroll(SCROLL_TOP); area.SetScroll(SCROLL_RIGHT); break;
        case Direction::RIGHT:		area.SetScroll(SCROLL_RIGHT); break;
        case Direction::BOTTOM_RIGHT:	area.SetScroll(SCROLL_BOTTOM); area.SetScroll(SCROLL_RIGHT); break;
        case Direction::BOTTOM:		area.SetScroll(SCROLL_BOTTOM); break;
        case Direction::BOTTOM_LEFT:	area.SetScroll(SCROLL_BOTTOM); area.SetScroll(SCROLL_LEFT); break;
        case Direction::LEFT:		area.SetScroll(SCROLL_LEFT); break;
	default: break;
    }
}

void Game::NewWeekDialog(void)
{
    const Week::type_t name = world.GetWeekType();
    std::string message = world.BeginMonth() ? _("Astrologers proclaim month of the %{name}.") : _("Astrologers proclaim week of the %{name}.");
    AGG::PlayMusic(world.BeginMonth() ? MUS::WEEK2_MONTH1 : MUS::WEEK1, false);
    String::Replace(message, "%{name}", Week::GetString(name));
    message += "\n \n";
    message += (name == Week::PLAGUE ? _(" All populations are halved.") : _(" All dwellings increase population."));
    Dialog::Message("", message, Font::BIG, Dialog::OK);
}

void Game::ShowEventDay(void)
{
    Kingdom & myKingdom = world.GetMyKingdom();
    std::vector<GameEvent::Day *> events;
    events.reserve(5);
    world.GetEventDay(myKingdom.GetColor(), events);
    std::vector<GameEvent::Day *>::const_iterator it1 = events.begin();
    std::vector<GameEvent::Day *>::const_iterator it2 = events.end();

    for(; it1 != it2; ++it1) if(*it1)
    {
    	AGG::PlayMusic(MUS::NEWS, false);
	if((*it1)->GetResource().GetValidItems())
	    Dialog::ResourceInfo("", (*it1)->GetMessage(), (*it1)->GetResource());
	else
	if((*it1)->GetMessage().size())
	    Dialog::Message("", (*it1)->GetMessage(), Font::BIG, Dialog::OK);
    }
}

void Game::ShowWarningLostTowns(menu_t & ret)
{
    const Kingdom & myKingdom = world.GetMyKingdom();
    Settings & conf = Settings::Get();
    if(0 == myKingdom.GetLostTownDays())
    {
    	    AGG::PlayMusic(MUS::DEATH, false);
	    std::string str = _("%{color} player, your heroes abandon you, and you are banished from this land.");
	    String::Replace(str, "%{color}", Color::String(conf.MyColor()));
	    DialogPlayers(conf.MyColor(), str);
	    GameOver::Result::Get().SetResult(GameOver::LOSS_ALL);
	    ret = MAINMENU;
    }
    else
    if(1 == myKingdom.GetLostTownDays())
    {
	    std::string str = _("%{color} player, this is your last day to capture a town, or you will be banished from this land.");
	    String::Replace(str, "%{color}", Color::String(conf.MyColor()));
	    DialogPlayers(conf.MyColor(), str);
    }
    else
    if(Game::GetLostTownDays() >= myKingdom.GetLostTownDays())
    {
	    std::string str = _("%{color} player, you only have %{day} days left to capture a town, or you will be banished from this land.");
	    String::Replace(str, "%{color}", Color::String(conf.MyColor()));
	    String::Replace(str, "%{day}", myKingdom.GetLostTownDays());
	    DialogPlayers(conf.MyColor(), str);
    }
}

void Game::EventSwitchShowRadar(void)
{
    Settings & conf = Settings::Get();

    if(conf.HideInterface())
    {
	if(conf.ShowRadar())
	{
	    conf.SetShowRadar(false);
	    Interface::Basic::Get().SetRedraw(REDRAW_GAMEAREA);
	}
	else
	{
	    if(conf.QVGA() && (conf.ShowIcons() || conf.ShowStatus() || conf.ShowButtons()))
	    {
		conf.SetShowIcons(false);
		conf.SetShowStatus(false);
		conf.SetShowButtons(false);
		Interface::Basic::Get().SetRedraw(REDRAW_GAMEAREA);
	    }
	    conf.SetShowRadar(true);
	    Interface::Basic::Get().SetRedraw(REDRAW_RADAR);
	}
    }
}

void Game::EventSwitchShowButtons(void)
{
    Settings & conf = Settings::Get();

    if(conf.HideInterface())
    {
	if(conf.ShowButtons())
	{
	    conf.SetShowButtons(false);
	    Interface::Basic::Get().SetRedraw(REDRAW_GAMEAREA);
	}
	else
	{
	    if(conf.QVGA() && (conf.ShowRadar() || conf.ShowStatus() || conf.ShowIcons()))
	    {
		conf.SetShowIcons(false);
		conf.SetShowStatus(false);
		conf.SetShowRadar(false);
		Interface::Basic::Get().SetRedraw(REDRAW_GAMEAREA);
	    }
	    conf.SetShowButtons(true);
	    Interface::Basic::Get().SetRedraw(REDRAW_BUTTONS);
	}
    }
}

void Game::EventSwitchShowStatus(void)
{
    Settings & conf = Settings::Get();

    if(conf.HideInterface())
    {
	if(conf.ShowStatus())
	{
	    conf.SetShowStatus(false);
	    Interface::Basic::Get().SetRedraw(REDRAW_GAMEAREA);
	}
	else
	{
	    if(conf.QVGA() && (conf.ShowRadar() || conf.ShowIcons() || conf.ShowButtons()))
	    {
		conf.SetShowIcons(false);
		conf.SetShowButtons(false);
		conf.SetShowRadar(false);
		Interface::Basic::Get().SetRedraw(REDRAW_GAMEAREA);
	    }
	    conf.SetShowStatus(true);
	    Interface::Basic::Get().SetRedraw(REDRAW_STATUS);
	}
    }
}

void Game::EventSwitchShowIcons(void)
{
    Settings & conf = Settings::Get();
    Interface::Basic & I = Interface::Basic::Get();

    if(conf.HideInterface())
    {
	if(conf.ShowIcons())
	{
	    conf.SetShowIcons(false);
	    I.SetRedraw(REDRAW_GAMEAREA);
	}
	else
	{
	    if(conf.QVGA() && (conf.ShowRadar() || conf.ShowStatus() || conf.ShowButtons()))
	    {
		conf.SetShowButtons(false);
		conf.SetShowRadar(false);
		conf.SetShowStatus(false);
		I.SetRedraw(REDRAW_GAMEAREA);
	    }
	    conf.SetShowIcons(true);
	    I.iconsPanel.SetCurrentVisible();
	    I.SetRedraw(REDRAW_ICONS);
	}
    }
}

void Game::EventSwitchShowControlPanel(void)
{
    Settings & conf = Settings::Get();

    if(conf.HideInterface())
    {
	conf.SetShowPanel(!conf.ShowControlPanel());
	Interface::Basic::Get().SetRedraw(REDRAW_GAMEAREA);
    }
}

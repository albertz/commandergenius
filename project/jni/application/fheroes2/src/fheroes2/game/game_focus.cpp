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

#include "castle.h"
#include "heroes.h"
#include "cursor.h"
#include "settings.h"
#include "game_interface.h"
#include "agg.h"
#include "world.h"
#include "kingdom.h"
#include "game_focus.h"

Game::Focus::Focus() : castle(NULL), heroes(NULL)
{
}

Game::Focus & Game::Focus::Get(void)
{
    static Game::Focus gfocus;
    
    return gfocus;
}

void Game::Focus::Set(Heroes *hero2)
{
    if(NULL == hero2) return;

    if(castle)
	castle = NULL;

    if(heroes && hero2 != heroes)
    {
	heroes->SetMove(false);
        heroes->ShowPath(false);
    }

    heroes = hero2;
    heroes->RescanPath();
    heroes->ShowPath(true);

    Interface::Basic & I = Interface::Basic::Get();
    
    I.iconsPanel.Select(*heroes);
    I.gameArea.Center(heroes->GetCenter());
    I.statusWindow.SetState(STATUS_ARMY);

    AGG::PlayMusic(MUS::FromGround(world.GetTiles(heroes->GetCenter()).GetGround()));
    Game::EnvironmentSoundMixer();    
}

void Game::Focus::Set(Castle *castle2)
{
    if(NULL == castle2) return;

    if(heroes)
    {
	heroes->SetMove(false);
        heroes->ShowPath(false);
	heroes = NULL;
    }

    castle = castle2;

    Interface::Basic & I = Interface::Basic::Get();

    I.iconsPanel.Select(*castle);
    I.gameArea.Center(castle->GetCenter());
    I.statusWindow.SetState(STATUS_FUNDS);

    AGG::PlayMusic(MUS::FromGround(world.GetTiles(castle->GetCenter()).GetGround()));
    Game::EnvironmentSoundMixer();
}

void Game::Focus::Reset(const focus_t priority)
{
    Kingdom & myKingdom = world.GetMyKingdom();

    Interface::Basic & I = Interface::Basic::Get();

    I.iconsPanel.ResetIcons();

    switch(priority)
    {
	case HEROES:
	    if(heroes && heroes->GetColor() == Settings::Get().MyColor())
        	Set(heroes);
	    else
	    if(myKingdom.GetHeroes().size())
        	Set(myKingdom.GetHeroes().front());
	    else
            if(myKingdom.GetCastles().size())
            {
                I.SetRedraw(REDRAW_HEROES);
                Set(myKingdom.GetCastles().front());
            }
            else
	    {
	    	castle = NULL;
		heroes = NULL;
	    }
	    break;

	case CASTLE:
	    if(castle && castle->GetColor() == Settings::Get().MyColor())
        	Set(castle);
	    else
	    if(myKingdom.GetCastles().size())
        	Set(myKingdom.GetCastles().front());
	    else
            if(myKingdom.GetHeroes().size())
            {
                I.SetRedraw(REDRAW_CASTLES);
                Set(myKingdom.GetHeroes().front());
            }
            else
	    {
		castle = NULL;
		heroes = NULL;
	    }
	    break;

	default:
	    castle = NULL;
	    heroes = NULL;
	    break;
    }
}

Game::Focus::focus_t Game::Focus::Type(void) const
{
    if(heroes) return HEROES;
    else
    if(castle) return CASTLE;

    return UNSEL;
}

const Castle & Game::Focus::GetCastle(void) const
{
    if(NULL == castle) DEBUG(DBG_GAME , DBG_WARN, "Game::Focus::GetCastle: is NULL");

    return *castle;
}

Castle & Game::Focus::GetCastle(void)
{
    if(NULL == castle) DEBUG(DBG_GAME , DBG_WARN, "Game::Focus::GetCastle: is NULL");

    return *castle;
}

const Heroes & Game::Focus::GetHeroes(void) const
{
    if(NULL == heroes) DEBUG(DBG_GAME , DBG_WARN, "Game::Focus::GetHeroes: is NULL");

    return *heroes;
}

Heroes & Game::Focus::GetHeroes(void)
{
    if(NULL == heroes) DEBUG(DBG_GAME , DBG_WARN, "Game::Focus::GetHeroes: is NULL");

    return *heroes;
}

const Point & Game::Focus::Center(void) const
{
    if(heroes) return heroes->GetCenter();
    else
    if(castle) return castle->GetCenter();

    return center;
}

void Game::Focus::CheckIconsPanel(void)
{
    Interface::Basic & I = Interface::Basic::Get();

    if(!heroes && I.iconsPanel.IsSelected(ICON_HEROES))
    {
	I.iconsPanel.ResetIcons(ICON_HEROES);
	I.SetRedraw(REDRAW_ICONS);
    }
    else
    if(heroes && !I.iconsPanel.IsSelected(ICON_HEROES))
    {
	I.iconsPanel.Select(*heroes);
	I.SetRedraw(REDRAW_ICONS);
    }

    if(!castle && I.iconsPanel.IsSelected(ICON_CASTLES))
    {
	I.iconsPanel.ResetIcons(ICON_CASTLES);
	I.SetRedraw(REDRAW_ICONS);
    }
    else
    if(castle && !I.iconsPanel.IsSelected(ICON_CASTLES))
    {
	I.iconsPanel.Select(*castle);
	I.SetRedraw(REDRAW_ICONS);
    }
}

void Game::Focus::SetRedraw(void)
{
    Interface::Basic & I = Interface::Basic::Get();

    CheckIconsPanel();

    I.SetRedraw(REDRAW_GAMEAREA | REDRAW_RADAR);

    if(heroes) I.SetRedraw(REDRAW_HEROES);
    else
    if(castle) I.SetRedraw(REDRAW_CASTLES);

    I.SetRedraw(REDRAW_STATUS);
}

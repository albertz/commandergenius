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

#ifndef H2GAME_H
#define H2GAME_H

#include <string>
#include "rect.h"
#include "types.h"

class Heroes;
class Castle;
class Surface;
class Kingdom;
class Player;

namespace Game
{
    enum menu_t
    {
	CANCEL = 0,
	QUITGAME,
	MAINMENU,
        NEWGAME,
        LOADGAME,
        HIGHSCORES,
        CREDITS,
        NEWSTANDARD,
        NEWCAMPAIN,
        NEWMULTI,
        NEWHOTSEAT,
        NEWNETWORK,
        LOADSTANDARD,
        LOADCAMPAIN,
        LOADMULTI,
        SCENARIOINFO,
        SELECTSCENARIO,
	STARTGAME,
	SAVEGAME,
	EDITMAINMENU,
	EDITNEWMAP,
	EDITLOADMAP,
	EDITSAVEMAP,
	EDITSTART,
	ENDTURN,
	TESTING
    };

    void Init(void);

    enum type_t { UNKNOWN = 0, STANDARD = 1, CAMPAIGN = 2, HOTSEAT = 4, NETWORK = 8, MULTI = HOTSEAT | NETWORK };
    enum control_t { NONE = 0, LOCAL = 1, REMOTE = 2, AI = 4 };
    enum distance_t { VIEW_TOWN  = 0, VIEW_CASTLE = 1, VIEW_HEROES = 2, VIEW_TELESCOPE = 3, VIEW_OBSERVATION_TOWER = 4, VIEW_MAGI_EYES = 5 };

    enum events_t
    {
	EVENT_NONE,
	EVENT_BUTTON_NEWGAME,
	EVENT_BUTTON_LOADGAME,
	EVENT_BUTTON_HIGHSCORES,
	EVENT_BUTTON_CREDITS,
	EVENT_BUTTON_STANDARD,
	EVENT_BUTTON_CAMPAIN,
	EVENT_BUTTON_MULTI,
	EVENT_BUTTON_SETTINGS,
	EVENT_BUTTON_SELECT,
	EVENT_BUTTON_HOTSEAT,
	EVENT_BUTTON_NETWORK,
	EVENT_BUTTON_HOST,
	EVENT_BUTTON_GUEST,
	EVENT_DEFAULT_READY,
	EVENT_DEFAULT_EXIT,
	EVENT_DEFAULT_LEFT,
	EVENT_DEFAULT_RIGHT,
	EVENT_SYSTEM_FULLSCREEN,
	EVENT_SYSTEM_SCREENSHOT,
	EVENT_SYSTEM_DEBUG1,
	EVENT_SYSTEM_DEBUG2,
	EVENT_ENDTURN,
	EVENT_NEXTHERO,
	EVENT_NEXTTOWN,
	EVENT_CONTINUE,
	EVENT_SAVEGAME,
	EVENT_LOADGAME,
	EVENT_FILEOPTIONS,
	EVENT_PUZZLEMAPS,
	EVENT_INFOGAME,
	EVENT_DIGARTIFACT,
	EVENT_CASTSPELL,
	EVENT_DEFAULTACTION,
	EVENT_OPENFOCUS,
	EVENT_SYSTEMOPTIONS,
	EVENT_BATTLE_CASTSPELL,
	EVENT_BATTLE_RETREAT,
	EVENT_BATTLE_SURRENDER,
	EVENT_BATTLE_AUTOSWITCH,
	EVENT_BATTLE_OPTIONS,
	EVENT_BATTLE_HARDSKIP,
	EVENT_BATTLE_SOFTSKIP,
	EVENT_MOVELEFT,
	EVENT_MOVERIGHT,
	EVENT_MOVETOP,
	EVENT_MOVEBOTTOM,
	EVENT_MOVETOPLEFT,
	EVENT_MOVETOPRIGHT,
	EVENT_MOVEBOTTOMLEFT,
	EVENT_MOVEBOTTOMRIGHT,
	EVENT_SCROLLLEFT,
	EVENT_SCROLLRIGHT,
	EVENT_SCROLLUP,
	EVENT_SCROLLDOWN,
	EVENT_CTRLPANEL,
	EVENT_SHOWRADAR,
	EVENT_SHOWBUTTONS,
	EVENT_SHOWSTATUS,
	EVENT_SHOWICONS,
	EVENT_SWITCHGROUP,
	EVENT_EMULATETOGGLE,
	EVENT_LAST
    };

    events_t & operator++ (events_t &);

    bool HotKeyPress(events_t);

    enum delay_t
    {
	SCROLL_DELAY,
	MAIN_MENU_DELAY,
	MAPS_DELAY,
	CASTLE_TAVERN_DELAY,
	CASTLE_AROUND_DELAY,
	CASTLE_BUYHERO_DELAY,
	CASTLE_BUILD_DELAY,
	HEROES_MOVE_DELAY,
	HEROES_FADE_DELAY,
	HEROES_PICKUP_DELAY,
	PUZZLE_FADE_DELAY,
	BATTLE_DIALOG_DELAY,
	BATTLE_FRAME_DELAY,
	BATTLE_MISSILE_DELAY,
	BATTLE_SPELL_DELAY,
	BATTLE_DISRUPTING_DELAY,
	BATTLE_CATAPULT_DELAY,
	BATTLE_CATAPULT2_DELAY,
	BATTLE_CATAPULT3_DELAY,
	BATTLE_BRIDGE_DELAY,
	BATTLE_IDLE_DELAY,
	BATTLE_IDLE2_DELAY,
	BATTLE_OPPONENTS_DELAY,
	BATTLE_FLAGS_DELAY,
	AUTOHIDE_STATUS_DELAY,
	//
	CURRENT_HERO_DELAY,
	CURRENT_AI_DELAY,
	//
	LAST_DELAY
    };

    bool AnimateInfrequent(delay_t);
    void AnimateDelayReset(delay_t);
    void UpdateHeroesMoveSpeed(void);

    control_t GetControl(u8);
    type_t GetType(u8);

    menu_t MainMenu(void);
    menu_t NewGame(void);
    menu_t LoadGame(void);
    menu_t HighScores(void);
    menu_t Credits(void);
    menu_t NewStandard(void);
    menu_t NewCampain(void);
    menu_t NewMulti(void);
    menu_t NewHotSeat(void);
    menu_t NewNetwork(void);
    menu_t LoadStandard(void);
    menu_t LoadCampain(void);
    menu_t LoadMulti(void);
    menu_t ScenarioInfo(void);
    menu_t SelectScenario(void);
    menu_t StartGame(void);

    menu_t NetworkHost(void);
    menu_t NetworkGuest(void);

    menu_t Testing(u8);

    void DrawInterface(void);
    
    void SetFixVideoMode(void);
    
    void EnvironmentSoundMixer(void);

    u8  GetRating(void);
    u16 GetGameOverScores(void);
    u8  GetLostTownDays(void);
    u8  GetViewDistance(distance_t);
    u8  GetWhirlpoolPercent(void);
    u8  GetHeroRestoreSpellPointsPerDay(void);

    void ShowLoadMapsText(void);

    void EventEndTurn(menu_t &);
    void EventExit(menu_t &);
    void EventNextHero(void);
    void EventNextTown(void);
    void EventDefaultAction(void);
    void EventOpenFocus(void);
    void EventSaveGame(void);
    void EventLoadGame(menu_t &);
    void EventContinueMovement(void);
    void EventKingdomInfo(void);
    void EventCastSpell(void);
    void EventPuzzleMaps(void);
    void EventGameInfo(void);
    void EventDigArtifact(menu_t &);
    void EventAdventureDialog(menu_t &);
    void EventFileDialog(menu_t &);
    void EventSystemDialog(void);
    void EventSwitchShowRadar(void);
    void EventSwitchShowStatus(void);
    void EventSwitchShowButtons(void);
    void EventSwitchShowIcons(void);
    void EventSwitchShowControlPanel(void);
    void EventSwitchGroup(void);

    namespace Scenario
    {
	void RedrawStaticInfo(const Point &);
	void RedrawDifficultyInfo(const Point & dst, bool label = true);
	void RedrawOpponentsInfo(const Point &, const std::vector<Player> *players = NULL);
	void RedrawClassInfo(const Point &, bool label = true);
    }

    namespace Editor
    {
	Game::menu_t MainMenu(void);
	Game::menu_t NewMaps(void);
	Game::menu_t LoadMaps(void);
	Game::menu_t StartGame(void);
	Game::menu_t StartGame(void);
    }
}

#define HotKeyCloseWindow (Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT) || Game::HotKeyPress(Game::EVENT_DEFAULT_READY))

#endif

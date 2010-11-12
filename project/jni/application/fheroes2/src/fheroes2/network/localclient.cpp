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
#include "dir.h"
#include "game.h"
#include "dialog.h"
#include "settings.h"
#include "network.h"
#include "remoteclient.h"
#include "localclient.h"
#include "cursor.h"
#include "button.h"
#include "world.h"
#include "agg.h"
#include "zzlib.h"
#include "dialog_selectscenario.h"
#include "server.h"

#ifdef WITH_NET

// game_scenarioinfo.cpp
void UpdateCoordOpponentsInfo(const Point &, std::vector<Rect> &);
void UpdateCoordClassInfo(const Point &, std::vector<Rect> &);



FH2LocalClient & FH2LocalClient::Get(void)
{
    static FH2LocalClient fh2localclient;

    return fh2localclient;
}

FH2LocalClient::FH2LocalClient() : admin_id(0)
{
    players.reserve(6);
}

u8 GetPlayersColors(const std::vector<Player> & v)
{
    u8 res = 0;
    std::vector<Player>::const_iterator it1 = v.begin();
    std::vector<Player>::const_iterator it2 = v.end();
    for(; it1 != it2; ++it1) if((*it1).player_id && (*it1).player_color) res |= (*it1).player_color;

    return res;
}

bool FH2LocalClient::Connect(const std::string & srv, u16 port)
{
    server = srv;
    IPaddress ip;
    if(Network::ResolveHost(ip, srv.c_str(), port) && Open(ip))
    {
	Settings::Get().SetNetworkLocalClient(true);
	return true;
    }
    return false;
}

void FH2LocalClient::PopPlayersInfo(QueueMessage & msg)
{
    Player cur;
    u8 size, admin;
    players.clear();
    admin_id = 0;
    msg.Pop(size);
    for(u8 ii = 0; ii < size; ++ii)
    {
        msg.Pop(cur.player_color);
        msg.Pop(cur.player_race);
        msg.Pop(cur.player_name);
        msg.Pop(cur.player_id);
        msg.Pop(admin);
	if(admin) admin_id = cur.player_id;
        if(cur.player_id) players.push_back(cur);
    }
}

int FH2LocalClient::Main(void)
{
    if(ConnectionChat())
    {
	if(ScenarioInfoDialog())
	{
	    Cursor & cursor = Cursor::Get();
	    Display & display = Display::Get();

	    cursor.Hide();
    	    display.Fill(0, 0, 0);
            TextBox(_("Please wait..."), Font::BIG, Rect(0, display.h()/2, display.w(), display.h()/2));
            display.Flip();

/*
	    if(Game::IO::LoadBIN(packet))
	    {
			conf.SetMyColor(Color::Get(player_color));
			cursor.Hide();
			return true;
	    }
*/

    	    StartGame();
	}
    }

    Logout();

    return 1;
}

void FH2LocalClient::Logout(void)
{
    packet.Reset();
    packet.SetID(MSG_LOGOUT);
    packet.Push(player_name);
    Send(packet);
    DELAY(100);
    Close();
    modes = 0;

    if(Modes(ST_LOCALSERVER))
    {
        FH2Server & server = FH2Server::Get();
        if(server.IsRun())
	{
	    server.SetExit();
    	    DELAY(100);
	}
    }
}

bool FH2LocalClient::ConnectionChat(void)
{
    Settings & conf = Settings::Get();

    player_color = 0;
    player_race = 0;
    player_name.clear();
    player_id = 0;

    // recv ready, banner
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::" << "ConnectionChat: " << "wait ready");
    if(!Wait(packet, MSG_READY)) return false;

    // get banner
    std::string str;
    packet.Pop(str);

    // dialog: input name
    if(!Dialog::InputString("Connected to " + server + "\n \n" + str + "\n \nEnter player name:", player_name))
	return false;

    // send hello
    packet.Reset();
    packet.SetID(MSG_HELLO);
    packet.Push(player_name);
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::" << "ConnectionChat: " << "send hello");
    if(!Send(packet)) return false;

    // recv hello, modes, player_id, cur maps
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::" << "ConnectionChat: " << "wait hello");
    if(!Wait(packet, MSG_HELLO)) return false;
    packet.Pop(modes);
    packet.Pop(player_id);
    packet.Pop(player_color);
    if(0 == player_id || 0 == player_color) DEBUG(DBG_NETWORK , DBG_WARN, "FH2LocalClient::" << "ConnectionChat: " << "player zero values");
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::" << "ConnectionChat: " << (Modes(ST_ADMIN) ? "admin" : "client") << " mode");

    // get cur maps
    Network::PacketPopMapsFileInfo(packet, conf.CurrentFileInfo());

    // get players
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::" << "ConnectionChat: " << "wait players info");
    if(!Wait(packet, MSG_UPDATE_PLAYERS)) return false;

    MsgUpdatePlayers();

    return true;
}

bool FH2LocalClient::ScenarioInfoDialog(void)
{
    Settings & conf = Settings::Get();

    // draw info dialog
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    const Point pointPanel(204, 32);
    const Point pointOpponentInfo(pointPanel.x + 24, pointPanel.y + 202);
    const Point pointClassInfo(pointPanel.x + 24, pointPanel.y + 282);
    const Rect  box(pointOpponentInfo, 360, 180);

    std::vector<Rect>::const_iterator itr;

    std::vector<Rect> coordColors(KINGDOMMAX);
    std::vector<Rect> coordRaces(KINGDOMMAX);

    UpdateCoordOpponentsInfo(pointOpponentInfo, coordColors);
    UpdateCoordClassInfo(pointClassInfo, coordRaces);

    Game::Scenario::RedrawStaticInfo(pointPanel);
    Game::Scenario::RedrawOpponentsInfo(pointOpponentInfo, &players);
    Game::Scenario::RedrawClassInfo(pointClassInfo);

    Button buttonSelectMaps(pointPanel.x + 309, pointPanel.y + 45, ICN::NGEXTRA, 64, 65);
    Button buttonOk(pointPanel.x + 31, pointPanel.y + 380, ICN::NGEXTRA, 66, 67);
    Button buttonCancel(pointPanel.x + 287, pointPanel.y + 380, ICN::NGEXTRA, 68, 69);

    SpriteCursor sp;
    sp.SetSprite(AGG::GetICN(ICN::NGEXTRA, 80));

    if(! Modes(ST_ADMIN))
    {
	buttonOk.Press();
	buttonOk.SetDisable(true);
	buttonSelectMaps.Press();
	buttonSelectMaps.SetDisable(true);
    }

    buttonSelectMaps.Draw();
    buttonOk.Draw();
    buttonCancel.Draw();
    cursor.Show();
    display.Flip();

    bool exit = false;
    bool update_info = false;
    u8 change_color = Color::NONE;
    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::" << "ScenarioInfoDialog: " << "start queue");

    while(!exit && le.HandleEvents())
    {
        if(Ready())
	{
	    if(!Recv(packet)) return false;
	    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::" << "ScenarioInfoDialog: " << "recv: " << Network::GetMsgString(packet.GetID()));

	    switch(packet.GetID())
    	    {
		case MSG_READY:
		    break;

		case MSG_CHANGE_RACE:
		    Network::UnpackRaceColors(packet);
		    update_info = true;
		    break;

		case MSG_UPDATE_PLAYERS:
		{
		    MsgUpdatePlayers();
		    if(Modes(ST_ADMIN))
		    {
			buttonOk.Release();
			buttonOk.SetDisable(false);
			buttonSelectMaps.Release();
			buttonSelectMaps.SetDisable(false);
		    }
		    else
		    {
			buttonOk.Press();
			buttonOk.SetDisable(true);
			buttonSelectMaps.Press();
			buttonSelectMaps.SetDisable(true);
		    }
		    update_info = true;
		    break;
		}

		case MSG_SET_CURRENT_MAP:
		    Network::PacketPopMapsFileInfo(packet, conf.CurrentFileInfo());
		    UpdateCoordOpponentsInfo(pointOpponentInfo, coordColors);
		    UpdateCoordClassInfo(pointClassInfo, coordRaces);
		    update_info = true;
		    break;

		case MSG_MESSAGE:
		    break;

		case MSG_SHUTDOWN:
		    exit = true;
		    break;


		default: break;
	    }
	}

	if(update_info)
	{
	    cursor.Hide();
	    Game::Scenario::RedrawStaticInfo(pointPanel);
	    Game::Scenario::RedrawOpponentsInfo(pointOpponentInfo, &players);
	    Game::Scenario::RedrawClassInfo(pointClassInfo);
	    buttonSelectMaps.Draw();
	    buttonOk.Draw();
	    buttonCancel.Draw();
	    cursor.Show();
	    display.Flip();
	    update_info = false;
	}

	// press button
        if(buttonSelectMaps.isEnable()) le.MousePressLeft(buttonSelectMaps) ? buttonSelectMaps.PressDraw() : buttonSelectMaps.ReleaseDraw();
        if(buttonOk.isEnable()) le.MousePressLeft(buttonOk) ? buttonOk.PressDraw() : buttonOk.ReleaseDraw();
        le.MousePressLeft(buttonCancel) ? buttonCancel.PressDraw() : buttonCancel.ReleaseDraw();

	// click select
	if(Game::HotKeyPress(Game::EVENT_BUTTON_SELECT) ||
	    (buttonSelectMaps.isEnable() && le.MouseClickLeft(buttonSelectMaps)))
	{
	    cursor.Hide();
	    sp.Hide();
	    cursor.Show();

	    // recv maps_info_list
	    packet.Reset();
	    packet.SetID(MSG_GET_MAPS_LIST);
	    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::" << "ScenarioInfoDialog: " << "send: " << Network::GetMsgString(packet.GetID()));
	    if(!Send(packet)) return false;

	    DEBUG(DBG_NETWORK , DBG_INFO,  "FH2LocalClient::" << "ScenarioInfoDialog: " << "wait: " << Network::GetMsgString(MSG_GET_MAPS_LIST));
	    if(Wait(packet, MSG_GET_MAPS_LIST))
	    {
		MapsFileInfoList lists;
		Network::PacketPopMapsFileInfoList(packet, lists);

        	std::string filemaps;
	        if(Dialog::SelectScenario(lists, filemaps) && filemaps.size())
        	{
		    // send set_maps_info
	    	    packet.Reset();
		    packet.SetID(MSG_SET_CURRENT_MAP);
	    	    packet.Push(filemaps);
		    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::" << "ScenarioInfoDialog: " << "send: " << Network::GetMsgString(packet.GetID()));
	    	    if(!Send(packet)) return false;
		}
		update_info = true;
	    }
	}
	else
	// click cancel
        if(le.MouseClickLeft(buttonCancel) || Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT))
    	    return false;
        else
        // click ok
        if(Game::HotKeyPress(Game::EVENT_DEFAULT_READY) || (buttonOk.isEnable() && le.MouseClickLeft(buttonOk)))
    	{
	    packet.Reset();
	    packet.SetID(MSG_START_GAME);
	    DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::" << "ScenarioInfoDialog: " << "send: " << Network::GetMsgString(packet.GetID()));
	    if(!Send(packet)) return false;
	    cursor.Hide();
	    return true;
	}
	else
	if(Modes(ST_ADMIN) && le.MouseClickLeft(box))
	{
	    // click colors
	    if(coordColors.end() !=
		(itr = std::find_if(coordColors.begin(), coordColors.end(), std::bind2nd(RectIncludePoint(), le.GetMouseCursor()))))
	    {
		u8 color = Color::GetFromIndex(itr - coordColors.begin());
		if((conf.PlayersColors() & color) && Color::NONE == change_color)
		{
		    cursor.Hide();
		    sp.Move((*itr).x - 3, (*itr).y - 3);
		    cursor.Show();
		    display.Flip();
		    change_color = color;
		}
		else
		if(conf.AllowColors(color))
		{
		    cursor.Hide();
		    sp.Hide();
		    cursor.Show();
		    display.Flip();
		    if(Color::NONE != change_color)
		    {
			packet.Reset();
			packet.SetID(MSG_CHANGE_COLORS);
			packet.Push(change_color);
			packet.Push(color);
		        DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::" << "ScenarioInfoDialog: " << "send: " << Network::GetMsgString(packet.GetID()));
			if(!Send(packet)) return false;
			change_color = Color::NONE;
		    }
		}
	    }
	    else
	    // click races
	    if(coordRaces.end() !=
		(itr = std::find_if(coordRaces.begin(), coordRaces.end(), std::bind2nd(RectIncludePoint(), le.GetMouseCursor()))))
	    {
		u8 color = Color::GetFromIndex(itr - coordRaces.begin());
		if(conf.AllowChangeRace(color))
		{
                    u8 race = conf.KingdomRace(color);
                    switch(race)
                    {
                        case Race::KNGT: race = Race::BARB; break;
                        case Race::BARB: race = Race::SORC; break;
                        case Race::SORC: race = Race::WRLK; break;
                        case Race::WRLK: race = Race::WZRD; break;
                        case Race::WZRD: race = Race::NECR; break;
                        case Race::NECR: race = Race::RAND; break;
                        case Race::RAND: race = Race::KNGT; break;
                        default: break;
                    }
		    if(change_color)
		    {
			cursor.Hide();
			sp.Hide();
			cursor.Show();
			display.Flip();
			change_color = Color::NONE;
		    }
		    if((race & Race::ALL) || race == Race::RAND)
		    {
			packet.Reset();
			packet.SetID(MSG_CHANGE_RACE);
			packet.Push(color);
			packet.Push(race);
		        DEBUG(DBG_NETWORK , DBG_INFO, "FH2LocalClient::" << "ScenarioInfoDialog: " << "send: " << Network::GetMsgString(packet.GetID()));
			if(!Send(packet)) return false;
		    }
		}
	    }
	}

        DELAY(10);
    }

    return false;
}

void FH2LocalClient::MsgUpdatePlayers(void)
{
    PopPlayersInfo(packet);
    Settings::Get().SetPlayersColors(GetPlayersColors(players));
    std::vector<Player>::iterator itp = std::find_if(players.begin(), players.end(), std::bind2nd(std::mem_fun_ref(&Player::isID), player_id));
    if(itp != players.end())
    {
	player_color = (*itp).player_color;
	player_race = (*itp).player_race;
	(admin_id == player_id ? SetModes(ST_ADMIN) : ResetModes(ST_ADMIN));
    }
}

Game::menu_t Game::NetworkGuest(void)
{
    Settings & conf = Settings::Get();
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    
    // clear background
    const Sprite &back = AGG::GetICN(ICN::HEROES, 0);
    cursor.Hide();
    display.Blit(back);
    cursor.Show();
    display.Flip();

    std::string server;
    if(!Dialog::InputString("Server Name", server)) return MAINMENU;

    FH2LocalClient & client = FH2LocalClient::Get();

    if(! client.Connect(server, conf.GetPort()))
    {
        Dialog::Message(_("Error"), Network::GetError(), Font::BIG, Dialog::OK);
	return Game::MAINMENU;
    }
    conf.SetGameType(Game::NETWORK);

    // main procedure
    client.Main();

    return QUITGAME;
}

#else
Game::menu_t Game::NetworkGuest(void)
{
    Dialog::Message(_("Error"), _("This release is compiled without network support."), Font::BIG, Dialog::OK);
    return MAINMENU;
}
#endif

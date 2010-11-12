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

#include <string>
#include <utility>
#include <algorithm>
#include "agg.h"
#include "button.h"
#include "world.h"
#include "cursor.h"
#include "settings.h"
#include "resource.h"
#include "castle.h"
#include "heroes.h"
#include "payment.h"
#include "profit.h"
#include "kingdom.h"
#include "tools.h"
#include "text.h"
#include "portrait.h"
#include "dialog.h"
#include "statusbar.h"
#include "selectarmybar.h"
#include "pocketpc.h"

void PackOrdersBuilding(const Castle & castle, std::vector<building_t> & orders_building)
{
    switch(castle.GetRace())
    {
	case Race::KNGT:
	    orders_building.push_back(BUILD_CASTLE);
	    orders_building.push_back(BUILD_WEL2);
	    orders_building.push_back(BUILD_CAPTAIN);
	    orders_building.push_back(BUILD_LEFTTURRET);
	    orders_building.push_back(BUILD_RIGHTTURRET);
	    orders_building.push_back(BUILD_MOAT);
	    orders_building.push_back(BUILD_MARKETPLACE);
	    orders_building.push_back(DWELLING_MONSTER2);
	    orders_building.push_back(BUILD_THIEVESGUILD);
	    orders_building.push_back(BUILD_TAVERN);
	    orders_building.push_back(BUILD_MAGEGUILD1);
	    orders_building.push_back(BUILD_MAGEGUILD2);
	    orders_building.push_back(BUILD_MAGEGUILD3);
	    orders_building.push_back(BUILD_MAGEGUILD4);
	    orders_building.push_back(BUILD_MAGEGUILD5);
	    orders_building.push_back(DWELLING_MONSTER5);
	    orders_building.push_back(DWELLING_MONSTER6);
	    orders_building.push_back(DWELLING_MONSTER1);
	    orders_building.push_back(DWELLING_MONSTER3);
	    orders_building.push_back(DWELLING_MONSTER4);
	    orders_building.push_back(BUILD_WELL);
	    orders_building.push_back(BUILD_STATUE);
	    orders_building.push_back(BUILD_SHIPYARD);
	    break;
	case Race::BARB:
	    orders_building.push_back(BUILD_SPEC);
	    orders_building.push_back(BUILD_WEL2);
	    orders_building.push_back(DWELLING_MONSTER6);
	    orders_building.push_back(BUILD_MAGEGUILD1);
	    orders_building.push_back(BUILD_MAGEGUILD2);
	    orders_building.push_back(BUILD_MAGEGUILD3);
	    orders_building.push_back(BUILD_MAGEGUILD4);
	    orders_building.push_back(BUILD_MAGEGUILD5);
	    orders_building.push_back(BUILD_CAPTAIN);
	    orders_building.push_back(BUILD_CASTLE);
	    orders_building.push_back(BUILD_LEFTTURRET);
	    orders_building.push_back(BUILD_RIGHTTURRET);
	    orders_building.push_back(BUILD_MOAT);
	    orders_building.push_back(DWELLING_MONSTER3);
	    orders_building.push_back(BUILD_THIEVESGUILD);
	    orders_building.push_back(BUILD_TAVERN);
	    orders_building.push_back(DWELLING_MONSTER1);
	    orders_building.push_back(BUILD_MARKETPLACE);
	    orders_building.push_back(DWELLING_MONSTER2);
	    orders_building.push_back(DWELLING_MONSTER4);
	    orders_building.push_back(DWELLING_MONSTER5);
	    orders_building.push_back(BUILD_WELL);
	    orders_building.push_back(BUILD_STATUE);
	    orders_building.push_back(BUILD_SHIPYARD);
	    break;
	case Race::SORC:
	    orders_building.push_back(BUILD_SPEC);
	    orders_building.push_back(DWELLING_MONSTER6);
	    orders_building.push_back(BUILD_MAGEGUILD1);
	    orders_building.push_back(BUILD_MAGEGUILD2);
	    orders_building.push_back(BUILD_MAGEGUILD3);
	    orders_building.push_back(BUILD_MAGEGUILD4);
	    orders_building.push_back(BUILD_MAGEGUILD5);
	    orders_building.push_back(BUILD_CAPTAIN);
	    orders_building.push_back(BUILD_CASTLE);
	    orders_building.push_back(BUILD_LEFTTURRET);
	    orders_building.push_back(BUILD_RIGHTTURRET);
	    orders_building.push_back(BUILD_MOAT);
	    orders_building.push_back(DWELLING_MONSTER3);
	    orders_building.push_back(BUILD_SHIPYARD);
	    orders_building.push_back(BUILD_MARKETPLACE);
	    orders_building.push_back(DWELLING_MONSTER2);
	    orders_building.push_back(BUILD_THIEVESGUILD);
	    orders_building.push_back(DWELLING_MONSTER1);
	    orders_building.push_back(BUILD_TAVERN);
	    orders_building.push_back(BUILD_STATUE);
	    orders_building.push_back(BUILD_WEL2);
	    orders_building.push_back(DWELLING_MONSTER4);
	    orders_building.push_back(BUILD_WELL);
	    orders_building.push_back(DWELLING_MONSTER5);
	    break;
	case Race::WRLK:
	    orders_building.push_back(DWELLING_MONSTER5);
	    orders_building.push_back(DWELLING_MONSTER3);
	    orders_building.push_back(BUILD_CASTLE);
	    orders_building.push_back(BUILD_LEFTTURRET);
	    orders_building.push_back(BUILD_RIGHTTURRET);
	    orders_building.push_back(BUILD_CAPTAIN);
	    orders_building.push_back(BUILD_MOAT);
	    orders_building.push_back(BUILD_SHIPYARD);
	    orders_building.push_back(BUILD_MAGEGUILD1);
	    orders_building.push_back(BUILD_MAGEGUILD2);
	    orders_building.push_back(BUILD_MAGEGUILD3);
	    orders_building.push_back(BUILD_MAGEGUILD4);
	    orders_building.push_back(BUILD_MAGEGUILD5);
	    orders_building.push_back(BUILD_TAVERN);
	    orders_building.push_back(BUILD_THIEVESGUILD);
	    orders_building.push_back(BUILD_MARKETPLACE);
	    orders_building.push_back(BUILD_STATUE);
	    orders_building.push_back(DWELLING_MONSTER1);
	    orders_building.push_back(BUILD_WEL2);
	    orders_building.push_back(BUILD_SPEC);
	    orders_building.push_back(DWELLING_MONSTER4);
	    orders_building.push_back(DWELLING_MONSTER2);
	    orders_building.push_back(DWELLING_MONSTER6);
	    orders_building.push_back(BUILD_WELL);
	    break;
	case Race::WZRD:
	    orders_building.push_back(DWELLING_MONSTER6);
	    orders_building.push_back(BUILD_CASTLE);
	    orders_building.push_back(BUILD_LEFTTURRET);
	    orders_building.push_back(BUILD_RIGHTTURRET);
	    orders_building.push_back(BUILD_MOAT);
	    orders_building.push_back(BUILD_CAPTAIN);
	    orders_building.push_back(DWELLING_MONSTER2);
	    orders_building.push_back(BUILD_THIEVESGUILD);
	    orders_building.push_back(BUILD_TAVERN);
	    orders_building.push_back(BUILD_SHIPYARD);
	    orders_building.push_back(BUILD_WELL);
	    orders_building.push_back(BUILD_SPEC);
	    orders_building.push_back(DWELLING_MONSTER3);
	    orders_building.push_back(DWELLING_MONSTER5);
	    orders_building.push_back(BUILD_MAGEGUILD1);
	    orders_building.push_back(BUILD_MAGEGUILD2);
	    orders_building.push_back(BUILD_MAGEGUILD3);
	    orders_building.push_back(BUILD_MAGEGUILD4);
	    orders_building.push_back(BUILD_MAGEGUILD5);
	    orders_building.push_back(BUILD_STATUE);
	    orders_building.push_back(DWELLING_MONSTER1);
	    orders_building.push_back(DWELLING_MONSTER4);
	    orders_building.push_back(BUILD_MARKETPLACE);
	    orders_building.push_back(BUILD_WEL2);
	    break;
	case Race::NECR:
	    orders_building.push_back(BUILD_SPEC);
	    if(Settings::Get().PriceLoyaltyVersion()) orders_building.push_back(BUILD_TAVERN); // shrine
	    orders_building.push_back(BUILD_CASTLE);
	    orders_building.push_back(BUILD_CAPTAIN);
	    orders_building.push_back(BUILD_LEFTTURRET);
	    orders_building.push_back(BUILD_RIGHTTURRET);
	    orders_building.push_back(DWELLING_MONSTER6);
	    orders_building.push_back(BUILD_MOAT);
	    orders_building.push_back(DWELLING_MONSTER1);
	    orders_building.push_back(BUILD_SHIPYARD);
	    orders_building.push_back(BUILD_THIEVESGUILD);
	    orders_building.push_back(BUILD_TAVERN);
	    orders_building.push_back(DWELLING_MONSTER3);
	    orders_building.push_back(DWELLING_MONSTER5);
	    orders_building.push_back(DWELLING_MONSTER2);
	    orders_building.push_back(DWELLING_MONSTER4);
	    orders_building.push_back(BUILD_MAGEGUILD1);
	    orders_building.push_back(BUILD_MAGEGUILD2);
	    orders_building.push_back(BUILD_MAGEGUILD3);
	    orders_building.push_back(BUILD_MAGEGUILD4);
	    orders_building.push_back(BUILD_MAGEGUILD5);
	    orders_building.push_back(BUILD_WEL2);
	    orders_building.push_back(BUILD_MARKETPLACE);
	    orders_building.push_back(BUILD_STATUE);
	    orders_building.push_back(BUILD_WELL);
	    break;
	default: break;
    }
}

Dialog::answer_t Castle::OpenDialog(bool readonly, bool fade)
{
    Settings & conf = Settings::Get();
    
    if(conf.QVGA()) return PocketPC::CastleOpenDialog(*this);

    const bool interface = conf.EvilInterface();
    if(conf.DynamicInterface())
    	conf.SetEvilInterface(GetRace() & (Race::BARB | Race::WRLK | Race::NECR));

    Display & display = Display::Get();
    castle_heroes = GetHeroes();

    // cursor
    Cursor & cursor = Cursor::Get();
    
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    Dialog::FrameBorder background;
    background.SetPosition((display.w() - 640 - BORDERWIDTH * 2) / 2, (display.h() - 480 - BORDERWIDTH * 2) / 2, 640, 480);
    background.Redraw();

    // fade
    if(conf.ExtUseFade()) display.Fade();
        
    const Point cur_pt(background.GetArea().x, background.GetArea().y);
    Point dst_pt(cur_pt);

    // button prev castle
    dst_pt.y += 480 - 19;
    Button buttonPrevCastle(dst_pt, ICN::SMALLBAR, 1, 2);

    // bottom small bar
    const Sprite & bar = AGG::GetICN(ICN::SMALLBAR, 0);
    dst_pt.x += buttonPrevCastle.w;
    display.Blit(bar, dst_pt);

    StatusBar statusBar;
    statusBar.SetFont(Font::BIG);
    statusBar.SetCenter(dst_pt.x + bar.w() / 2, dst_pt.y + 11);

    // button next castle
    dst_pt.x += bar.w();
    Button buttonNextCastle(dst_pt, ICN::SMALLBAR, 3, 4);

    // strip grid
    dst_pt.x = cur_pt.x;
    dst_pt.y = cur_pt.y + 256;

    display.Blit(AGG::GetICN(ICN::STRIP, 0), dst_pt);

    // color crest
    const Sprite & crest = AGG::GetICN(ICN::CREST, Color::GetIndex(color));

    dst_pt.x = cur_pt.x + 5;
    dst_pt.y = cur_pt.y + 262;
    const Rect rectSign(dst_pt, crest.w(), crest.h());
    std::string str_date = _("Month: %{month}, Week: %{week}, Day: %{day}");
    String::Replace(str_date, "%{month}", world.GetMonth());
    String::Replace(str_date, "%{week}", world.GetWeek());
    String::Replace(str_date, "%{day}", world.GetDay());

    display.Blit(crest, dst_pt);

    // castle troops selector
    dst_pt.x = cur_pt.x + 112;
    dst_pt.y = cur_pt.y + 262;

    SelectArmyBar selectCaptainArmy;
    selectCaptainArmy.SetArmy(army);
    selectCaptainArmy.SetPos(dst_pt);
    selectCaptainArmy.SetInterval(6);
    selectCaptainArmy.SetBackgroundSprite(AGG::GetICN(ICN::STRIP, 2));
    selectCaptainArmy.SetCursorSprite(AGG::GetICN(ICN::STRIP, 1));
    selectCaptainArmy.SetCastle(*this);
    selectCaptainArmy.Redraw();

    // portrait heroes or captain or sign
    dst_pt.x = cur_pt.x + 5;
    dst_pt.y = cur_pt.y + 361;
    
    const Rect rectHeroPortrait(dst_pt.x, dst_pt.y, 100, 92);

    if(castle_heroes)
	display.Blit(Portrait::Hero((*castle_heroes), Portrait::BIG), dst_pt);
    else
    if(isBuild(BUILD_CAPTAIN))
	display.Blit(Portrait::Captain(race, Portrait::BIG), dst_pt);
    else
    	display.Blit(AGG::GetICN(ICN::STRIP, 3), dst_pt);

    // castle_heroes troops background
    dst_pt.x = cur_pt.x + 112;
    dst_pt.y = cur_pt.y + 361;

    SelectArmyBar selectHeroesArmy;
    selectHeroesArmy.SetPos(dst_pt);
    selectHeroesArmy.SetInterval(6);
    selectHeroesArmy.SetBackgroundSprite(AGG::GetICN(ICN::STRIP, 2));
    selectHeroesArmy.SetCursorSprite(AGG::GetICN(ICN::STRIP, 1));
    selectHeroesArmy.SetSaveLastTroop();
    selectHeroesArmy.SetCastle(*this);

    if(castle_heroes)
    {
	castle_heroes->MovePointsScaleFixed();
        selectHeroesArmy.SetArmy(castle_heroes->GetArmy());
        selectHeroesArmy.Redraw();
    }
    else
    	display.Blit(AGG::GetICN(ICN::STRIP, 11), dst_pt);
    
    // resource
    RedrawResourcePanel(cur_pt);

    // button exit
    dst_pt.x = cur_pt.x + 553;
    dst_pt.y = cur_pt.y + 428;
    Button buttonExit(dst_pt, ICN::SWAPBTN, 0, 1);

    const Rect coordBuildingThievesGuild(GetCoordBuilding(BUILD_THIEVESGUILD, cur_pt));
    const Rect coordBuildingTavern(GetCoordBuilding(BUILD_TAVERN, cur_pt));
    const Rect coordBuildingShipyard(GetCoordBuilding(BUILD_SHIPYARD, cur_pt));
    const Rect coordBuildingWell(GetCoordBuilding(BUILD_WELL, cur_pt));
    const Rect coordBuildingStatue(GetCoordBuilding(BUILD_STATUE, cur_pt));
    const Rect coordBuildingMarketplace(GetCoordBuilding(BUILD_MARKETPLACE, cur_pt));
    const Rect coordBuildingWel2(GetCoordBuilding(BUILD_WEL2, cur_pt));
    const Rect coordBuildingMoat(GetCoordBuilding(BUILD_MOAT, cur_pt));
    const Rect coordBuildingSpec(GetCoordBuilding(BUILD_SPEC, cur_pt));
    const Rect coordBuildingCastle(GetCoordBuilding(BUILD_CASTLE, cur_pt));
    const Rect coordBuildingCaptain(GetCoordBuilding(BUILD_CAPTAIN, cur_pt));
    const Rect coordBuildingTent(GetCoordBuilding(BUILD_TENT, cur_pt));

    const Rect coordDwellingMonster1(GetCoordBuilding(DWELLING_MONSTER1, cur_pt));
    const Rect coordDwellingMonster2(GetCoordBuilding(DWELLING_MONSTER2, cur_pt));
    const Rect coordDwellingMonster3(GetCoordBuilding(DWELLING_MONSTER3, cur_pt));
    const Rect coordDwellingMonster4(GetCoordBuilding(DWELLING_MONSTER4, cur_pt));
    const Rect coordDwellingMonster5(GetCoordBuilding(DWELLING_MONSTER5, cur_pt));
    const Rect coordDwellingMonster6(GetCoordBuilding(DWELLING_MONSTER6, cur_pt));

    // orders draw building
    std::vector<building_t> orders_building;
    orders_building.reserve(25);

    PackOrdersBuilding(*this, orders_building);

    // update extra description
    payment_t profit;
    std::string description_well = GetDescriptionBuilding(BUILD_WELL, race);
    std::string description_wel2 = GetDescriptionBuilding(BUILD_WEL2, race);
    std::string description_castle = GetDescriptionBuilding(BUILD_CASTLE, race);
    std::string description_statue = GetDescriptionBuilding(BUILD_STATUE, race);
    std::string description_spec = GetDescriptionBuilding(BUILD_SPEC, race);
    String::Replace(description_well, "%{count}", grown_well);
    String::Replace(description_wel2, "%{count}", grown_wel2);
    profit = ProfitConditions::FromBuilding(BUILD_CASTLE, race);
    String::Replace(description_castle, "%{count}", profit.gold);
    profit = ProfitConditions::FromBuilding(BUILD_STATUE, race);
    String::Replace(description_statue, "%{count}", profit.gold);
    profit = ProfitConditions::FromBuilding(BUILD_SPEC, race);
    String::Replace(description_spec, "%{count}", profit.gold);

    // draw building
    RedrawAllBuilding(*this, cur_pt, orders_building);

    if(2 > world.GetMyKingdom().GetCastles().size() || readonly)
    {
	buttonPrevCastle.Press();
        buttonPrevCastle.SetDisable(true);

        buttonNextCastle.Press();
        buttonNextCastle.SetDisable(true);
    }

    buttonPrevCastle.Draw();
    buttonNextCastle.Draw();
    buttonExit.Draw();

    Mixer::Reset();
    AGG::PlayMusic(MUS::FromRace(race));
    
    LocalEvent & le = LocalEvent::Get();

    cursor.Show();

    display.Flip();

    Dialog::answer_t result = Dialog::ZERO;

    bool army_redraw = false;

    // dialog menu loop
    while(le.HandleEvents())
    {
        // exit
	if(le.MouseClickLeft(buttonExit) || Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT)){ result = Dialog::CANCEL; break; }

        if(buttonPrevCastle.isEnable()) le.MousePressLeft(buttonPrevCastle) ? buttonPrevCastle.PressDraw() : buttonPrevCastle.ReleaseDraw();
        if(buttonNextCastle.isEnable()) le.MousePressLeft(buttonNextCastle) ? buttonNextCastle.PressDraw() : buttonNextCastle.ReleaseDraw();

        le.MousePressLeft(buttonExit) ? buttonExit.PressDraw() : buttonExit.ReleaseDraw();

       // selector troops event
        if(castle_heroes && selectHeroesArmy.isValid())
        {
            if(le.MouseCursor(selectCaptainArmy.GetArea()) || le.MouseCursor(selectHeroesArmy.GetArea()))
                SelectArmyBar::QueueEventProcessing(selectCaptainArmy, selectHeroesArmy);
        }
        else
        {
            if(le.MouseCursor(selectCaptainArmy.GetArea()))
                SelectArmyBar::QueueEventProcessing(selectCaptainArmy);
        }

	// view hero
	if(!readonly && castle_heroes && le.MouseClickLeft(rectHeroPortrait))
	{
	    const std::vector<Heroes *> & myHeroes = world.GetMyKingdom().GetHeroes();
	    Dialog::answer_t result = Dialog::ZERO;
	    
	    std::vector<Heroes *>::const_iterator it = std::find(myHeroes.begin(), myHeroes.end(), castle_heroes);
	    bool need_fade = (640 == display.w() && 480 == display.h());

	    while(Dialog::CANCEL != result)
	    {
	        Display::Get().Flip();
	        cursor.Hide();

                result = (*it)->OpenDialog(false, need_fade);
                if(need_fade) need_fade = false;

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

            	    default: break;
		}
		cursor.Show();
	    }

	    cursor.Hide();
            if(selectCaptainArmy.isSelected()) selectCaptainArmy.Reset();
            if(selectHeroesArmy.isSelected()) selectHeroesArmy.Reset();
            selectHeroesArmy.Redraw();
	    cursor.Show();
	    display.Flip();
	}

        // prev castle
	if(buttonPrevCastle.isEnable() && le.MouseClickLeft(buttonPrevCastle)){ result = Dialog::PREV; break; }
	else
        // next castle
	if(buttonNextCastle.isEnable() && le.MouseClickLeft(buttonNextCastle)){ result = Dialog::NEXT; break; }
	else
	// left click building
	if((building & BUILD_THIEVESGUILD) && le.MouseClickLeft(coordBuildingThievesGuild))
	    Dialog::ThievesGuild(false);
	else
	if((building & BUILD_TAVERN) && le.MouseClickLeft(coordBuildingTavern))
	{
	    if(Race::NECR == race && conf.PriceLoyaltyVersion())
		Dialog::Message(GetStringBuilding(BUILD_TAVERN, race), GetDescriptionBuilding(BUILD_TAVERN, race), Font::BIG, Dialog::OK);
	    else
		OpenTavern();
	}
	else
	if(!readonly && (building & BUILD_SHIPYARD) && le.MouseClickLeft(coordBuildingShipyard))
	{
	    cursor.Hide();

	    if(Dialog::OK == Dialog::BuyBoat(AllowBuyBoat()))
	    {
		BuyBoat();
		RedrawResourcePanel(cur_pt);
    		// RedrawResourcePanel destroy sprite buttonExit
		if(buttonExit.isPressed()) buttonExit.Draw();
	    }
	    cursor.Show();
	    display.Flip();
	}
	else
	if((building & BUILD_WELL) && le.MouseClickLeft(coordBuildingWell))
	{
	    if(readonly)
		Dialog::Message(GetStringBuilding(BUILD_WELL, race), description_well, Font::BIG, Dialog::OK);
	    else
	    {
		OpenWell();
		army_redraw = true;
	    }
	}
	else
	if((building & BUILD_STATUE) && le.MouseClickLeft(coordBuildingStatue))
	    Dialog::Message(GetStringBuilding(BUILD_STATUE), description_statue, Font::BIG, Dialog::OK);
	else
	if((building & BUILD_MARKETPLACE) && le.MouseClickLeft(coordBuildingMarketplace))
	{
	    if(readonly)
		Dialog::Message(GetStringBuilding(BUILD_MARKETPLACE, race), GetDescriptionBuilding(BUILD_MARKETPLACE), Font::BIG, Dialog::OK);
	    else
	    {
		cursor.Hide();
		Dialog::Marketplace();
		cursor.Show();
		display.Flip();
		RedrawResourcePanel(cur_pt);
		// RedrawResourcePanel destroy sprite buttonExit
		if(buttonExit.isPressed()) buttonExit.Draw();
	    }
	}
	else
	if((building & BUILD_WEL2) && le.MouseClickLeft(coordBuildingWel2))
	    Dialog::Message(GetStringBuilding(BUILD_WEL2, race), description_wel2, Font::BIG, Dialog::OK);
	else
	if((building & BUILD_MOAT) && le.MouseClickLeft(coordBuildingMoat))
	    Dialog::Message(GetStringBuilding(BUILD_MOAT), GetDescriptionBuilding(BUILD_MOAT), Font::BIG, Dialog::OK);
	else
	if((building & BUILD_SPEC) && le.MouseClickLeft(coordBuildingSpec))
	    Dialog::Message(GetStringBuilding(BUILD_SPEC, race), description_spec, Font::BIG, Dialog::OK);
	else
	if(readonly &&
	    (((building & BUILD_CASTLE) && le.MouseClickLeft(coordBuildingCastle)) ||
	     (!(building & BUILD_CASTLE) && le.MouseClickLeft(coordBuildingTent))))
		Dialog::Message(GetStringBuilding(BUILD_CASTLE, race), description_castle, Font::BIG, Dialog::OK);
	else
	if(building & BUILD_CASTLE && le.MouseClickLeft(coordBuildingCastle))
	{
	    const Heroes * prev = castle_heroes;
	    const u32 build = OpenTown();
	    const bool buyhero = ((castle_heroes != prev) && (castle_heroes != NULL));

	    if(BUILD_NOTHING != build)
	    {
		cursor.Hide();

                if(selectCaptainArmy.isSelected()) selectCaptainArmy.Reset();

		// play sound
    		AGG::PlaySound(M82::BUILDTWN);

		RedrawAnimationBuilding(*this, cur_pt, orders_building, build);
		BuyBuilding(build);
		RedrawResourcePanel(cur_pt);

		if(BUILD_CAPTAIN == build && ! castle_heroes)
		    display.Blit(Portrait::Captain(race, Portrait::BIG), cur_pt.x + 5, cur_pt.y + 361);

    		// RedrawResourcePanel destroy sprite buttonExit
		if(buttonExit.isPressed()) buttonExit.Draw();

		cursor.Show();
		display.Flip();
	    }

	    if(castle_heroes)
	    {
		cursor.Hide();

                if(selectHeroesArmy.isSelected()) selectHeroesArmy.Reset();

		if(buyhero)
		{
		    const Rect rt(0, 98, 552, 107);
		    Surface sf(rt.w, rt.h);
		    sf.SetColorKey();
            	    sf.Blit(AGG::GetICN(ICN::STRIP, 0), rt, 0, 0);
		    const Surface & port = Portrait::Hero((*castle_heroes), Portrait::BIG);
		    sf.Blit(port, 6, 6);
            	    selectHeroesArmy.SetArmy(castle_heroes->GetArmy());
		    selectHeroesArmy.SetPos(112, 5);
		    selectHeroesArmy.Redraw(sf);
		    selectHeroesArmy.SetPos(cur_pt.x + 112, cur_pt.y + 361);

		    AGG::PlaySound(M82::BUILDTWN);
		    RedrawResourcePanel(cur_pt);

		    LocalEvent & le = LocalEvent::Get();
		    u8 alpha = 0;

		    while(le.HandleEvents() && alpha < 250)
		    {
    			if(Game::AnimateInfrequent(Game::CASTLE_BUYHERO_DELAY))
    			{
    			    cursor.Hide();
        		    sf.SetAlpha(alpha);
        		    display.Blit(sf, cur_pt.x, cur_pt.y + 356);
    			    cursor.Show();
        		    display.Flip();
        		    alpha += 10;
    			}
		    }

		    cursor.Hide();
            	    display.Blit(AGG::GetICN(ICN::STRIP, 0), cur_pt.x, cur_pt.y + 256);
            	    display.Blit(Portrait::Hero((*castle_heroes), Portrait::BIG), cur_pt.x + 5, cur_pt.y + 361);
            	    selectHeroesArmy.Redraw();
		}

		cursor.Show();
		display.Flip();
	    }
	}
	else
	// buy castle
	if(!(building & BUILD_CASTLE) && le.MouseClickLeft(coordBuildingTent))
	{
	    if(!Modes(ALLOWCASTLE))
	    {
		Dialog::Message(_("Town"), _("This town may not be upgraded to a castle."), Font::BIG, Dialog::OK);
	    }
	    else
	    if(Dialog::OK == DialogBuyCastle(true))
	    {
		cursor.Hide();

		// play sound
    		AGG::PlaySound(M82::BUILDTWN);

		RedrawAnimationBuilding(*this, cur_pt, orders_building, BUILD_CASTLE);
		BuyBuilding(BUILD_CASTLE);
		RedrawResourcePanel(cur_pt);

		// RedrawResourcePanel destroy sprite buttonExit
		if(buttonExit.isPressed()) buttonExit.Draw();

		cursor.Show();
		display.Flip();
	    }
	}
	else
	// captain
	if((building & BUILD_CAPTAIN) && le.MouseClickLeft(coordBuildingCaptain))
	    Dialog::Message(GetStringBuilding(BUILD_CAPTAIN), GetDescriptionBuilding(BUILD_CAPTAIN), Font::BIG, Dialog::OK);
	else
	// left click mage guild
	if((building & (BUILD_MAGEGUILD5 | BUILD_MAGEGUILD4 | BUILD_MAGEGUILD3 | BUILD_MAGEGUILD2 | BUILD_MAGEGUILD1)) &&
	    le.MouseClickLeft(GetCoordBuilding(BUILD_MAGEGUILD5, cur_pt)))
	{
		// buy spell book
		if(!castle_heroes || (*castle_heroes).HasArtifact(Artifact::MAGIC_BOOK))
		    OpenMageGuild();
		else
		if((*castle_heroes).BuySpellBook(&mageguild))
		    army_redraw = true;
	}
	else
	// left click dwelling monster
	if(!readonly && (building & DWELLING_MONSTER1) && le.MouseClickLeft(coordDwellingMonster1) &&
	    Castle::RecruitMonster(DWELLING_MONSTER1, Dialog::RecruitMonster(Monster(race, DWELLING_MONSTER1), dwelling[0])))
		army_redraw = true;
	else
	if(!readonly && (building & DWELLING_MONSTER2) && le.MouseClickLeft(coordDwellingMonster2) &&
	    Castle::RecruitMonster(DWELLING_MONSTER2, Dialog::RecruitMonster(Monster(race, GetActualDwelling(DWELLING_MONSTER2)), dwelling[1])))
		army_redraw = true;
	else
	if(!readonly && (building & DWELLING_MONSTER3) && le.MouseClickLeft(coordDwellingMonster3) &&
	    Castle::RecruitMonster(DWELLING_MONSTER3, Dialog::RecruitMonster(Monster(race, GetActualDwelling(DWELLING_MONSTER3)), dwelling[2])))
		army_redraw = true;
	else
	if(!readonly && (building & DWELLING_MONSTER4) && le.MouseClickLeft(coordDwellingMonster4) &&
	    Castle::RecruitMonster(DWELLING_MONSTER4, Dialog::RecruitMonster(Monster(race, GetActualDwelling(DWELLING_MONSTER4)), dwelling[3])))
		army_redraw = true;
	else
	if(!readonly && (building & DWELLING_MONSTER5) && le.MouseClickLeft(coordDwellingMonster5) &&
	    Castle::RecruitMonster(DWELLING_MONSTER5, Dialog::RecruitMonster(Monster(race, GetActualDwelling(DWELLING_MONSTER5)), dwelling[4])))
		army_redraw = true;
	else
	if(!readonly && (building & DWELLING_MONSTER6) && le.MouseClickLeft(coordDwellingMonster6) &&
	    Castle::RecruitMonster(DWELLING_MONSTER6, Dialog::RecruitMonster(Monster(race, GetActualDwelling(DWELLING_MONSTER6)), dwelling[5])))
		army_redraw = true;

	if(army_redraw)
	{
	    cursor.Hide();
	    selectCaptainArmy.Redraw();
	    RedrawResourcePanel(cur_pt);
	    // RedrawResourcePanel destroy sprite buttonExit
	    if(buttonExit.isPressed()) buttonExit.Draw();
	    cursor.Show();
	    display.Flip();
	    army_redraw = false;
	}

	// right press building
	if(building & BUILD_THIEVESGUILD && le.MousePressRight(coordBuildingThievesGuild)) Dialog::Message(GetStringBuilding(BUILD_THIEVESGUILD), GetDescriptionBuilding(BUILD_THIEVESGUILD), Font::BIG);
	else
	if(building & BUILD_TAVERN && le.MousePressRight(coordBuildingTavern)) Dialog::Message(GetStringBuilding(BUILD_TAVERN, race), GetDescriptionBuilding(BUILD_TAVERN, race), Font::BIG);
	else
	if(building & BUILD_SHIPYARD && le.MousePressRight(coordBuildingShipyard)) Dialog::Message(GetStringBuilding(BUILD_SHIPYARD), GetDescriptionBuilding(BUILD_SHIPYARD), Font::BIG);
	else
	if(building & BUILD_WELL && le.MousePressRight(coordBuildingWell)) Dialog::Message(GetStringBuilding(BUILD_WELL), description_well, Font::BIG);
	else
	if(building & BUILD_STATUE && le.MousePressRight(coordBuildingStatue)) Dialog::Message(GetStringBuilding(BUILD_STATUE), description_statue, Font::BIG);
	else
	if(building & BUILD_MARKETPLACE && le.MousePressRight(coordBuildingMarketplace)) Dialog::Message(GetStringBuilding(BUILD_MARKETPLACE), GetDescriptionBuilding(BUILD_MARKETPLACE), Font::BIG);
	else
	if(building & BUILD_WEL2 && le.MousePressRight(coordBuildingWel2)) Dialog::Message(GetStringBuilding(BUILD_WEL2, race), description_wel2, Font::BIG);
	else
	if(building & BUILD_MOAT && le.MousePressRight(coordBuildingMoat)) Dialog::Message(GetStringBuilding(BUILD_MOAT), GetDescriptionBuilding(BUILD_MOAT), Font::BIG);
	else
	if(building & BUILD_SPEC && le.MousePressRight(coordBuildingSpec)) Dialog::Message(GetStringBuilding(BUILD_SPEC, race), description_spec, Font::BIG);
	else
	if(building & BUILD_CASTLE && le.MousePressRight(coordBuildingCastle)) Dialog::Message(GetStringBuilding(BUILD_CASTLE), description_castle, Font::BIG);
	else
	if(!(building & BUILD_CASTLE) && le.MousePressRight(coordBuildingTent)) Dialog::Message(GetStringBuilding(BUILD_TENT), GetDescriptionBuilding(BUILD_TENT), Font::BIG);
	else
	if(building & BUILD_CAPTAIN && le.MousePressRight(coordBuildingCaptain)) Dialog::Message(GetStringBuilding(BUILD_CAPTAIN), GetDescriptionBuilding(BUILD_CAPTAIN), Font::BIG);
	else
	// right press mage guild
	if(building & (BUILD_MAGEGUILD5 | BUILD_MAGEGUILD4 | BUILD_MAGEGUILD3 | BUILD_MAGEGUILD2 | BUILD_MAGEGUILD1) &&
	    le.MousePressRight(GetCoordBuilding(BUILD_MAGEGUILD5, cur_pt))) Dialog::Message(GetStringBuilding(BUILD_MAGEGUILD1), GetDescriptionBuilding(BUILD_MAGEGUILD1), Font::BIG);
	else
	// right press dwelling monster
	if(building & DWELLING_MONSTER1 && le.MousePressRight(coordDwellingMonster1))
	    Dialog::DwellingInfo(Monster(race, DWELLING_MONSTER1), dwelling[0]);
	else
	if(building & DWELLING_MONSTER2 && le.MousePressRight(coordDwellingMonster2))
	    Dialog::DwellingInfo(Monster(race, GetActualDwelling(DWELLING_MONSTER2)), dwelling[1]);
	else
	if(building & DWELLING_MONSTER3 && le.MousePressRight(coordDwellingMonster3))
	    Dialog::DwellingInfo(Monster(race, GetActualDwelling(DWELLING_MONSTER3)), dwelling[2]);
	else
	if(building & DWELLING_MONSTER4 && le.MousePressRight(coordDwellingMonster4))
	    Dialog::DwellingInfo(Monster(race, GetActualDwelling(DWELLING_MONSTER4)), dwelling[3]);
	else
	if(building & DWELLING_MONSTER5 && le.MousePressRight(coordDwellingMonster5))
	    Dialog::DwellingInfo(Monster(race, GetActualDwelling(DWELLING_MONSTER5)), dwelling[4]);
	else
	if(building & DWELLING_MONSTER6 && le.MousePressRight(coordDwellingMonster6))
	    Dialog::DwellingInfo(Monster(race, GetActualDwelling(DWELLING_MONSTER6)), dwelling[5]);

	// status message exit
	if(le.MouseCursor(buttonExit)) statusBar.ShowMessage(isCastle() ? _("Exit castle") : _("Exit town"));
	else
	// status message prev castle
	if(le.MouseCursor(buttonPrevCastle)) statusBar.ShowMessage(_("Show previous town"));
	else
	// status message next castle
	if(le.MouseCursor(buttonNextCastle)) statusBar.ShowMessage(_("Show next town"));
	else
	// status message over sign
	if(le.MouseCursor(rectSign)) statusBar.ShowMessage(str_date);
	else
	// status message view hero
	if(castle_heroes && le.MouseCursor(rectHeroPortrait)) statusBar.ShowMessage(_("View Hero"));
	else
	// building
	if(building & BUILD_THIEVESGUILD && le.MouseCursor(coordBuildingThievesGuild)) statusBar.ShowMessage(GetStringBuilding(BUILD_THIEVESGUILD));
	else
	if(building & BUILD_TAVERN && le.MouseCursor(coordBuildingTavern)) statusBar.ShowMessage(GetStringBuilding(BUILD_TAVERN, race));
	else
	if(building & BUILD_SHIPYARD && le.MouseCursor(coordBuildingShipyard)) statusBar.ShowMessage(GetStringBuilding(BUILD_SHIPYARD));
	else
	if(building & BUILD_WELL && le.MouseCursor(coordBuildingWell)) statusBar.ShowMessage(GetStringBuilding(BUILD_WELL));
	else
	if(building & BUILD_STATUE && le.MouseCursor(coordBuildingStatue)) statusBar.ShowMessage(GetStringBuilding(BUILD_STATUE));
	else
	if(building & BUILD_MARKETPLACE && le.MouseCursor(coordBuildingMarketplace)) statusBar.ShowMessage(GetStringBuilding(BUILD_MARKETPLACE));
	else
	if(building & BUILD_WEL2 && le.MouseCursor(coordBuildingWel2)) statusBar.ShowMessage(GetStringBuilding(BUILD_WEL2, race));
	else
	if(building & BUILD_MOAT && le.MouseCursor(coordBuildingMoat)) statusBar.ShowMessage(GetStringBuilding(BUILD_MOAT));
	else
	if(building & BUILD_SPEC && le.MouseCursor(coordBuildingSpec)) statusBar.ShowMessage(GetStringBuilding(BUILD_SPEC, race));
	else
	if(building & BUILD_CASTLE && le.MouseCursor(coordBuildingCastle)) statusBar.ShowMessage(GetStringBuilding(BUILD_CASTLE));
	else
	if(!(building & BUILD_CASTLE) && le.MouseCursor(coordBuildingTent)) statusBar.ShowMessage(GetStringBuilding(BUILD_TENT));
	else
	if(building & BUILD_CAPTAIN && le.MouseCursor(coordBuildingCaptain)) statusBar.ShowMessage(GetStringBuilding(BUILD_CAPTAIN));
	else
	// mage guild
	if(building & BUILD_MAGEGUILD5 && le.MouseCursor(GetCoordBuilding(BUILD_MAGEGUILD5, cur_pt))) statusBar.ShowMessage(GetStringBuilding(BUILD_MAGEGUILD5));
	else
	if(building & BUILD_MAGEGUILD4 && le.MouseCursor(GetCoordBuilding(BUILD_MAGEGUILD4, cur_pt))) statusBar.ShowMessage(GetStringBuilding(BUILD_MAGEGUILD4));
	else
	if(building & BUILD_MAGEGUILD3 && le.MouseCursor(GetCoordBuilding(BUILD_MAGEGUILD3, cur_pt))) statusBar.ShowMessage(GetStringBuilding(BUILD_MAGEGUILD3));
	else
	if(building & BUILD_MAGEGUILD2 && le.MouseCursor(GetCoordBuilding(BUILD_MAGEGUILD2, cur_pt))) statusBar.ShowMessage(GetStringBuilding(BUILD_MAGEGUILD2));
	else
	if(building & BUILD_MAGEGUILD1 && le.MouseCursor(GetCoordBuilding(BUILD_MAGEGUILD1, cur_pt))) statusBar.ShowMessage(GetStringBuilding(BUILD_MAGEGUILD1));
	else
	// dwelling monster
	if(building & DWELLING_MONSTER1 && le.MouseCursor(coordDwellingMonster1)) statusBar.ShowMessage(Monster(race, DWELLING_MONSTER1).GetName());
	else
	if(building & DWELLING_MONSTER2 && le.MouseCursor(coordDwellingMonster2)) statusBar.ShowMessage(Monster(race, DWELLING_UPGRADE2 & building ? DWELLING_UPGRADE2 : DWELLING_MONSTER2).GetName());
	else
	if(building & DWELLING_MONSTER3 && le.MouseCursor(coordDwellingMonster3)) statusBar.ShowMessage(Monster(race, DWELLING_UPGRADE3 & building ? DWELLING_UPGRADE3 : DWELLING_MONSTER3).GetName());
	else
	if(building & DWELLING_MONSTER4 && le.MouseCursor(coordDwellingMonster4)) statusBar.ShowMessage(Monster(race, DWELLING_UPGRADE4 & building ? DWELLING_UPGRADE4 : DWELLING_MONSTER4).GetName());
	else
	if(building & DWELLING_MONSTER5 && le.MouseCursor(coordDwellingMonster5)) statusBar.ShowMessage(Monster(race, DWELLING_UPGRADE5 & building ? DWELLING_UPGRADE5 : DWELLING_MONSTER5).GetName());
	else
	if(building & DWELLING_MONSTER6 && le.MouseCursor(coordDwellingMonster6)) statusBar.ShowMessage(Monster(race, DWELLING_UPGRADE6 & building ? (DWELLING_UPGRADE7 & building ? DWELLING_UPGRADE7 : DWELLING_UPGRADE6) : DWELLING_MONSTER6).GetName());
	else
	// status message over castle troops
	if(le.MouseCursor(selectCaptainArmy.GetArea()))
	{
            const s8 index1 = selectCaptainArmy.GetIndexFromCoord(le.GetMouseCursor());
            if(0 <= index1)
            {
		const Army::Troop & troop1 = army.At(index1);
		const std::string & monster1 = troop1.GetName();
		std::string str;

		if(selectCaptainArmy.isSelected())
		{
		    const u8 index2 = selectCaptainArmy.Selected();
		    const Army::Troop & troop2 = army.At(index2);
		    const std::string & monster2 = troop2.GetName();

		    if(index1 == index2)
		    {
			str = _("View %{name}");
			String::Replace(str, "%{name}", monster1);
		    }
		    else
		    if(troop1.isValid() && troop2.isValid())
		    {
			str = troop1() == troop2() ? _("Combine %{name} armies") : _("Exchange %{name2} with %{name}");
			String::Replace(str, "%{name}", monster1);
			String::Replace(str, "%{name2}", monster2);
		    }
		    else
		    {
		    	str = _("Move and right click Redistribute %{name}");
			String::Replace(str, "%{name}", monster2);
		    }
		}
		else
		if(castle_heroes && selectHeroesArmy.isSelected())
		{
		    const u8 index2 = selectHeroesArmy.Selected();
		    const Army::Troop & troop2 = castle_heroes->GetArmy().At(index2);
		    const std::string & monster2 = troop2.GetName();

		    if(selectHeroesArmy.SaveLastTroop() && !troop1.isValid())
			str = _("Cannot move last army to garrison");
		    else
		    if(troop1.isValid() && troop2.isValid())
		    {
			str = troop1() == troop2() ? _("Combine %{name} armies") : _("Exchange %{name2} with %{name}");
			String::Replace(str, "%{name}", monster1);
			String::Replace(str, "%{name2}", monster2);
		    }
		    else
		    {
		    	str = _("Move and right click Redistribute %{name}");
			String::Replace(str, "%{name}", monster2);
		    }
		}
		else
		if(troop1.isValid())
		{
		    str = _("Select %{name}");
		    String::Replace(str, "%{name}", monster1);
		}
		else
		    str = _("Empty");

		    statusBar.ShowMessage(str);
	    }
	}
	else
	// status message over castle_heroes troops
	if(castle_heroes && le.MouseCursor(selectHeroesArmy.GetArea()))
	{
            const s8 index1 = selectHeroesArmy.GetIndexFromCoord(le.GetMouseCursor());
            if(0 <= index1)
            {
		const Army::Troop & troop1 = castle_heroes->GetArmy().At(index1);
		const std::string & monster1 = troop1.GetName();
		std::string str;
		
		if(selectHeroesArmy.isSelected())
		{
		    const u8 index2 = selectHeroesArmy.Selected();
		    const Army::Troop & troop2 = castle_heroes->GetArmy().At(index2);
		    const std::string & monster2 = troop2.GetName();

		    if(index1 == index2)
		    {
			str = _("View %{name}");
			String::Replace(str, "%{name}", monster1);
		    }
		    else
		    if(troop1.isValid() && troop2.isValid())
		    {
			str = troop1() == troop2() ? _("Combine %{name} armies") : _("Exchange %{name2} with %{name}");
			String::Replace(str, "%{name}", monster1);
			String::Replace(str, "%{name2}", monster2);
		    }
		    else
		    {
			str = _("Move and right click Redistribute %{name}");
			String::Replace(str, "%{name}", monster2);
		    }
		}
		else
		if(selectCaptainArmy.isSelected())
		{
		    const u8 index2 = selectCaptainArmy.Selected();
		    const Army::Troop & troop2 = army.At(index2);
		    const std::string & monster2 = troop2.GetName();

		    if(troop1.isValid() && troop2.isValid())
		    {
			str = troop1() == troop2() ? _("Combine %{name} armies") : _("Exchange %{name2} with %{name}");
			String::Replace(str, "%{name}", monster1);
			String::Replace(str, "%{name2}", monster2);
		    }
		    else
		    {
			str = _("Move and right click Redistribute %{name}");
			String::Replace(str, "%{name}", monster2);
		    }
		}
		else
		if(troop1.isValid())
		    {
			str = _("Select %{name}");
			String::Replace(str, "%{name}", monster1);
		    }
		else
		    str = _("Empty");

		    statusBar.ShowMessage(str);
	    }
	}
	else
	{
		std::string str;
	    str = _("Empty");
	    statusBar.ShowMessage(str);
	}

	// animation sprite
	if(Game::AnimateInfrequent(Game::CASTLE_AROUND_DELAY))
	{
	    cursor.Hide();
	    RedrawAllBuilding(*this, cur_pt, orders_building);
	    cursor.Show();
	    display.Flip();
	}
    }

    if(castle_heroes && conf.ExtHeroRecalculateMovement())
	castle_heroes->RecalculateMovePoints();

    if(conf.DynamicInterface())
	conf.SetEvilInterface(interface);

    return result;
}

/* redraw resource info panel */
void Castle::RedrawResourcePanel(const Point & pt)
{
    Display & display = Display::Get();
    const Resource::funds_t & resource = world.GetMyKingdom().GetFundsResource();

    Point dst_pt = pt;

    Rect src_rt(dst_pt.x + 552, dst_pt.y + 262, 82, 192);
    display.FillRect(0, 0, 0, src_rt);

    std::string count;
    Text text;

    // sprite wood
    dst_pt.x = src_rt.x + 1;
    dst_pt.y = src_rt.y + 10;
    const Sprite & wood = AGG::GetICN(ICN::RESOURCE, 0);
    display.Blit(wood, dst_pt);
    
    // count wood
    count.erase();
    String::AddInt(count, resource.wood);
    text.Set(count, Font::SMALL);
    dst_pt.y += 22;
    text.Blit(dst_pt.x + (wood.w() - text.w()) / 2, dst_pt.y);

    // sprite sulfur
    dst_pt.x = src_rt.x + 42;
    dst_pt.y = src_rt.y + 6;
    const Sprite & sulfur = AGG::GetICN(ICN::RESOURCE, 3);
    display.Blit(sulfur, dst_pt);
    
    // count sulfur
    count.erase();
    String::AddInt(count, resource.sulfur);
    text.Set(count);
    dst_pt.y += 26;
    text.Blit(dst_pt.x + (sulfur.w() - text.w()) / 2, dst_pt.y);

    // sprite crystal
    dst_pt.x = src_rt.x + 1;
    dst_pt.y = src_rt.y + 45;
    const Sprite & crystal = AGG::GetICN(ICN::RESOURCE, 4);
    display.Blit(crystal, dst_pt);
    
    // count crystal
    count.erase();
    String::AddInt(count, resource.crystal);
    text.Set(count);
    dst_pt.y += 33;
    text.Blit(dst_pt.x + (crystal.w() - text.w()) / 2, dst_pt.y);

    // sprite mercury
    dst_pt.x = src_rt.x + 44;
    dst_pt.y = src_rt.y + 47;
    const Sprite & mercury = AGG::GetICN(ICN::RESOURCE, 1);
    display.Blit(mercury, dst_pt);
    
    // count mercury
    count.erase();
    String::AddInt(count, resource.mercury);
    text.Set(count);
    dst_pt.y += 34;
    text.Blit(dst_pt.x + (mercury.w() - text.w()) / 2, dst_pt.y);

    // sprite ore
    dst_pt.x = src_rt.x + 1;
    dst_pt.y = src_rt.y + 92;
    const Sprite & ore = AGG::GetICN(ICN::RESOURCE, 2);
    display.Blit(ore, dst_pt);
    
    // count ore
    count.erase();
    String::AddInt(count, resource.ore);
    text.Set(count);
    dst_pt.y += 26;
    text.Blit(dst_pt.x + (ore.w() - text.w()) / 2, dst_pt.y);

    // sprite gems
    dst_pt.x = src_rt.x + 45;
    dst_pt.y = src_rt.y + 92;
    const Sprite & gems = AGG::GetICN(ICN::RESOURCE, 5);
    display.Blit(gems, dst_pt);
    
    // count gems
    count.erase();
    String::AddInt(count, resource.gems);
    text.Set(count);
    dst_pt.y += 26;
    text.Blit(dst_pt.x + (gems.w() - text.w()) / 2, dst_pt.y);

    // sprite gold
    dst_pt.x = src_rt.x + 6;
    dst_pt.y = src_rt.y + 130;
    const Sprite & gold = AGG::GetICN(ICN::RESOURCE, 6);
    display.Blit(gold, dst_pt);
    
    // count gold
    count.erase();
    String::AddInt(count, resource.gold);
    text.Set(count);
    dst_pt.y += 24;
    text.Blit(dst_pt.x + (gold.w() - text.w()) / 2, dst_pt.y);
    
    // sprite button exit
    dst_pt.x = src_rt.x + 1;
    dst_pt.y = src_rt.y + 166;
    const Sprite & exit = AGG::GetICN(ICN::SWAPBTN, 0);
    display.Blit(exit, dst_pt);
}

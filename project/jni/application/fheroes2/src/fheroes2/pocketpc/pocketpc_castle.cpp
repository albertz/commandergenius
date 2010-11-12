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
#include <vector>
#include "agg.h"
#include "cursor.h"
#include "settings.h"
#include "text.h"
#include "button.h"
#include "castle.h"
#include "kingdom.h"
#include "heroes.h"
#include "portrait.h"
#include "world.h"
#include "selectarmybar.h"
#include "buildinginfo.h"
#include "profit.h"
#include "pocketpc.h"

class DwellingBar : protected Rect
{
public:
    DwellingBar(const Point &, const Castle &);
    void Redraw(void) const;
    const Rect & GetArea(void) const;
    bool QueueEventProcessing(void);
    static u32 GetDwellingFromIndex(u8);

private:
    const Castle & castle;
    std::vector<Rect> dw;
};

void RedrawTownSprite(const Rect &, const Castle &);
void RedrawBackground(const Rect &, const Castle &);
void RedrawResourceBar(const Point &, const Resource::funds_t &);

enum screen_t { SCREENOUT, SCREENOUT_PREV, SCREENOUT_NEXT, SCREEN1, SCREEN2, SCREEN3, SCREEN4, SCREEN5, SCREEN6 };

screen_t CastleOpenDialog1(Castle &);
screen_t CastleOpenDialog2(Castle &);
screen_t CastleOpenDialog3(Castle &);
screen_t CastleOpenDialog4(Castle &);
screen_t CastleOpenDialog5(Castle &);
screen_t CastleOpenDialog6(Castle &);

Dialog::answer_t PocketPC::CastleOpenDialog(Castle & castle)
{
    Mixer::Reset();
    AGG::PlayMusic(MUS::FromRace(castle.GetRace()));

    screen_t screen = CastleOpenDialog1(castle);
    while(SCREENOUT != screen)
	switch(screen)
	{
	    case SCREEN1: screen = CastleOpenDialog1(castle); break;
	    case SCREEN2: screen = CastleOpenDialog2(castle); break;
	    case SCREEN3: screen = CastleOpenDialog3(castle); break;
	    case SCREEN4: screen = CastleOpenDialog4(castle); break;
	    case SCREEN5: screen = CastleOpenDialog5(castle); break;
	    case SCREEN6: screen = CastleOpenDialog6(castle); break;
	    case SCREENOUT_PREV: return Dialog::PREV;
	    case SCREENOUT_NEXT: return Dialog::NEXT;
	    default: break;
	}
    return Dialog::CANCEL;
}

screen_t CastleOpenDialog1(Castle & castle)
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const u16 window_w = 320;
    const u16 window_h = 224;

    Dialog::FrameBorder frameborder;
    frameborder.SetPosition((display.w() - window_w) / 2 - BORDERWIDTH, (display.h() - window_h) / 2 - BORDERWIDTH, window_w, window_h);
    frameborder.Redraw();

    const Rect & dst_rt = frameborder.GetArea();
    const Sprite & backSprite = AGG::GetICN(ICN::SWAPWIN, 0);
    const Sprite & background = AGG::GetICN(ICN::STONEBAK, 0);
    display.Blit(background, Rect(0, 0, window_w, window_h), dst_rt);

    RedrawBackground(dst_rt, castle);

    Text text;
    text.Set(castle.GetName(), Font::YELLOW_SMALL);
    text.Blit(dst_rt.x + (dst_rt.w - text.w()) / 2, dst_rt.y + 3);

    // town icon
    const Sprite & slock = AGG::GetICN(ICN::LOCATORS, 23);
    const Rect rectTown(dst_rt.x, dst_rt.y + 2, slock.w(), slock.h());
    RedrawTownSprite(rectTown, castle);

    // dwelling bar
    DwellingBar dwbar(Point(dst_rt.x + 2, dst_rt.y + 34), castle);
    dwbar.Redraw();

    Point pt;

    // castle army bar
    pt.x = dst_rt.x + 2;
    pt.y = dst_rt.y + 79;

    display.Blit(AGG::GetICN(ICN::SWAPWIN, 0), Rect(36, 267, 43, 43), pt.x, pt.y);
    if(castle.isBuild(BUILD_CAPTAIN))
    {
	const Surface & icon = Portrait::Captain(castle.GetRace(), Portrait::BIG);
	display.Blit(icon, Rect((icon.w() - 41) / 2, 15, 41, 41), pt.x + 1, pt.y + 1);
    }
    else
    {
	const Sprite & crest = AGG::GetICN(ICN::BRCREST, Color::GetIndex(castle.GetColor()));
	display.Blit(crest, Rect((crest.w() - 41) / 2, (crest.h() - 41) / 2, 41, 41), pt.x + 1, pt.y + 1);
    }

    const Rect rt1(36, 267, 43, 53);
    Surface sfb1(rt1.w, rt1.h);
    sfb1.Blit(backSprite, rt1, 0, 0);
    Surface sfc1(rt1.w, rt1.h - 10);
    Cursor::DrawCursor(sfc1, 0x10, true);

    SelectArmyBar selectCastleArmy;
    selectCastleArmy.SetArmy(castle.GetArmy());
    selectCastleArmy.SetPos(pt.x + 45, pt.y);
    selectCastleArmy.SetInterval(2);
    selectCastleArmy.SetBackgroundSprite(sfb1);
    selectCastleArmy.SetCursorSprite(sfc1);
    selectCastleArmy.SetUseMons32Sprite();
    selectCastleArmy.SetCastle(castle);
    selectCastleArmy.Redraw();

    // hero army bar
    pt.x = dst_rt.x + 2;
    pt.y = dst_rt.y + 132;
    const Heroes* hero = castle.GetHeroes();

    SelectArmyBar selectHeroesArmy;
    selectHeroesArmy.SetPos(pt.x + 45, pt.y);
    selectHeroesArmy.SetInterval(2);
    selectHeroesArmy.SetBackgroundSprite(sfb1);
    selectHeroesArmy.SetCursorSprite(sfc1);
    selectHeroesArmy.SetUseMons32Sprite();
    selectHeroesArmy.SetSaveLastTroop();
    selectHeroesArmy.SetCastle(castle);

    display.Blit(AGG::GetICN(ICN::SWAPWIN, 0), Rect(36, 267, 43, 43), pt.x, pt.y);
    const Rect heroSign(pt.x + 1, pt.y + 1, 41, 41);
    if(hero)
    {
	const Surface & icon = Portrait::Hero(*hero, Portrait::MEDIUM);
	display.Blit(icon, Rect((icon.w() - 41) / 2, (icon.h() - 41) / 2, 41, 41), heroSign);

	selectHeroesArmy.SetArmy(const_cast<Army::army_t &>(hero->GetArmy()));
	selectHeroesArmy.Redraw();
    }
    else
    {
	const Sprite & crest = AGG::GetICN(ICN::BRCREST, Color::GetIndex(castle.GetColor()));
	display.Blit(crest, Rect((crest.w() - 41) / 2, (crest.h() - 41) / 2, 41, 41), heroSign);
    }

    // resource bar
    RedrawResourceBar(Point(dst_rt.x + 4, dst_rt.y + 181), world.GetMyKingdom().GetFundsResource());

    const Rect rectExit(dst_rt.x + dst_rt.w - 26, dst_rt.y + 7, 25, 25);
    display.Blit(AGG::GetICN(ICN::TOWNWIND, 12), rectExit.x, rectExit.y);

    const Rect rectScreen1(dst_rt.x + dst_rt.w - 27, dst_rt.y + 32, 25, 25);
    const Rect rectScreen2(dst_rt.x + dst_rt.w - 27, dst_rt.y + 58, 25, 25);
    const Rect rectScreen3(dst_rt.x + dst_rt.w - 27, dst_rt.y + 84, 25, 25);
    const Rect rectScreen4(dst_rt.x + dst_rt.w - 27, dst_rt.y + 110, 25, 25);
    const Rect rectScreen5(dst_rt.x + dst_rt.w - 27, dst_rt.y + 136, 25, 25);
    const Rect rectScreen6(dst_rt.x + dst_rt.w - 27, dst_rt.y + 162, 25, 25);

    display.Blit(AGG::GetICN(ICN::REQUESTS, 20), rectScreen1.x, rectScreen1.y);

    if(castle.isBuild(BUILD_CASTLE))
    {
	display.Blit(AGG::GetICN(ICN::REQUESTS, 21), rectScreen2.x, rectScreen2.y);
	display.Blit(AGG::GetICN(ICN::REQUESTS, 22), rectScreen3.x, rectScreen3.y);
	display.Blit(AGG::GetICN(ICN::REQUESTS, 23), rectScreen4.x, rectScreen4.y);
    }
    if(castle.isBuild(BUILD_CASTLE) || castle.isBuild(BUILD_TAVERN))
	display.Blit(AGG::GetICN(ICN::REQUESTS, 24), rectScreen5.x, rectScreen5.y);
    if(castle.isBuild(BUILD_MAGEGUILD1))
	display.Blit(AGG::GetICN(ICN::REQUESTS, 25), rectScreen6.x, rectScreen6.y);

    // update extra description
    std::string description_castle = castle.GetDescriptionBuilding(BUILD_CASTLE, castle.GetRace());
    {
        payment_t profit = ProfitConditions::FromBuilding(BUILD_CASTLE, castle.GetRace());
        String::Replace(description_castle, "%{count}", profit.gold);
    }

    Button buttonPrev(dst_rt.x + 64, dst_rt.y + 5, ICN::TRADPOST, 3, 4);
    Button buttonNext(dst_rt.x + 245, dst_rt.y + 5, ICN::TRADPOST, 5, 6);
    if(2 > world.GetMyKingdom().GetCastles().size())
    {
	buttonNext.Press();
        buttonPrev.Press();
	buttonNext.SetDisable(true);
        buttonPrev.SetDisable(true);
    }
    buttonNext.Draw();
    buttonPrev.Draw();

    cursor.Show();
    display.Flip();

    while(le.HandleEvents())
    {
        le.MousePressLeft(buttonNext) ? buttonNext.PressDraw() : buttonNext.ReleaseDraw();
        le.MousePressLeft(buttonPrev) ? buttonPrev.PressDraw() : buttonPrev.ReleaseDraw();

        //if(le.MouseClickLeft(rectScreen1)) return SCREEN1;
        //else
        if(castle.isBuild(BUILD_CASTLE) && le.MouseClickLeft(rectScreen2)) return SCREEN2;
        else
        if(castle.isBuild(BUILD_CASTLE) && le.MouseClickLeft(rectScreen3)) return SCREEN3;
        else
        if(castle.isBuild(BUILD_CASTLE) && le.MouseClickLeft(rectScreen4)) return SCREEN4;
        else
        if(castle.isBuild(BUILD_CASTLE) && le.MouseClickLeft(rectScreen5)) return SCREEN5;
        else
        if(castle.isBuild(BUILD_MAGEGUILD1) && le.MouseClickLeft(rectScreen6)) return SCREEN6;
        else
        // exit
        if(le.MouseClickLeft(rectExit) || Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT)) break;
	else
        if(buttonNext.isEnable() && le.MouseClickLeft(buttonNext)) return SCREENOUT_NEXT;
        else
        if(buttonPrev.isEnable() && le.MouseClickLeft(buttonPrev)) return SCREENOUT_PREV;
    	else
	if(le.MouseClickLeft(rectTown))
	{
	    if(castle.isBuild(BUILD_CASTLE))
		Dialog::Message(castle.GetStringBuilding(BUILD_CASTLE), description_castle, Font::BIG, Dialog::OK);
	    else
	    if(!castle.Modes(Castle::ALLOWCASTLE))
        	Dialog::Message(_("Town"), _("This town may not be upgraded to a castle."), Font::BIG, Dialog::OK);
    	    else
            if(Dialog::OK == castle.DialogBuyCastle(true))
	    {
		// play sound
    		AGG::PlaySound(M82::BUILDTWN);
                castle.BuyBuilding(BUILD_CASTLE);
                cursor.Hide();
                RedrawTownSprite(rectTown, castle);
                cursor.Show();
                display.Flip();
            }
	}
	else
	if(le.MouseClickLeft(dwbar.GetArea()) && dwbar.QueueEventProcessing())
	{
	    cursor.Hide();
	    dwbar.Redraw();
	    selectCastleArmy.Redraw();
	    RedrawResourceBar(Point(dst_rt.x + 4, dst_rt.y + 181), world.GetMyKingdom().GetFundsResource());
	    cursor.Show();
	    display.Flip();
	}
	else
	if(hero && le.MouseClickLeft(heroSign))
	{
	    cursor.Hide();
	    const_cast<Heroes *>(hero)->OpenDialog(false, false);
	    cursor.Show();
	    display.Flip();
	}

	// troops event
        if(hero && selectHeroesArmy.isValid())
        {
    	    if(le.MouseCursor(selectCastleArmy.GetArea()) || le.MouseCursor(selectHeroesArmy.GetArea()))
    		SelectArmyBar::QueueEventProcessing(selectCastleArmy, selectHeroesArmy);
	}
        else
        {
    	    if(le.MouseCursor(selectCastleArmy.GetArea()))
    		SelectArmyBar::QueueEventProcessing(selectCastleArmy);
	}
    }
    return SCREENOUT;
}

screen_t CastleOpenDialog2(Castle & castle)
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const u16 window_w = 320;
    const u16 window_h = 224;

    Dialog::FrameBorder frameborder;
    frameborder.SetPosition((display.w() - window_w) / 2 - BORDERWIDTH, (display.h() - window_h) / 2 - BORDERWIDTH, window_w, window_h);
    frameborder.Redraw();


    const Rect & dst_rt = frameborder.GetArea();
    const Sprite & background = AGG::GetICN(ICN::STONEBAK, 0);
    display.Blit(background, Rect(0, 0, window_w, window_h), dst_rt);

    const Rect rectExit(dst_rt.x + dst_rt.w - 26, dst_rt.y + 7, 25, 25);
    display.Blit(AGG::GetICN(ICN::TOWNWIND, 12), rectExit.x, rectExit.y);

    const Rect rectScreen1(dst_rt.x + dst_rt.w - 27, dst_rt.y + 32, 25, 25);
    const Rect rectScreen2(dst_rt.x + dst_rt.w - 27, dst_rt.y + 58, 25, 25);
    const Rect rectScreen3(dst_rt.x + dst_rt.w - 27, dst_rt.y + 84, 25, 25);
    const Rect rectScreen4(dst_rt.x + dst_rt.w - 27, dst_rt.y + 110, 25, 25);
    const Rect rectScreen5(dst_rt.x + dst_rt.w - 27, dst_rt.y + 136, 25, 25);
    const Rect rectScreen6(dst_rt.x + dst_rt.w - 27, dst_rt.y + 162, 25, 25);

    display.Blit(AGG::GetICN(ICN::REQUESTS, 20), rectScreen1.x, rectScreen1.y);
    display.Blit(AGG::GetICN(ICN::REQUESTS, 21), rectScreen2.x, rectScreen2.y);
    display.Blit(AGG::GetICN(ICN::REQUESTS, 22), rectScreen3.x, rectScreen3.y);
    display.Blit(AGG::GetICN(ICN::REQUESTS, 23), rectScreen4.x, rectScreen4.y);
    display.Blit(AGG::GetICN(ICN::REQUESTS, 24), rectScreen5.x, rectScreen5.y);
    if(castle.isBuild(BUILD_MAGEGUILD1)) display.Blit(AGG::GetICN(ICN::REQUESTS, 25), rectScreen6.x, rectScreen6.y);

    BuildingInfo dwelling1(castle, DWELLING_MONSTER1);
    dwelling1.SetPos(dst_rt.x + 2, dst_rt.y + 2);
    dwelling1.Redraw();

    BuildingInfo dwelling2(castle, DWELLING_MONSTER2);
    dwelling2.SetPos(dst_rt.x + 141, dst_rt.y + 2);
    dwelling2.Redraw();

    BuildingInfo dwelling3(castle, DWELLING_MONSTER3);
    dwelling3.SetPos(dst_rt.x + 2, dst_rt.y + 76);
    dwelling3.Redraw();

    BuildingInfo dwelling4(castle, DWELLING_MONSTER4);
    dwelling4.SetPos(dst_rt.x + 141, dst_rt.y + 76);
    dwelling4.Redraw();

    BuildingInfo dwelling5(castle, DWELLING_MONSTER5);
    dwelling5.SetPos(dst_rt.x + 2, dst_rt.y + 150);
    dwelling5.Redraw();

    BuildingInfo dwelling6(castle, DWELLING_MONSTER6);
    dwelling6.SetPos(dst_rt.x + 141, dst_rt.y + 150);
    dwelling6.Redraw();

    cursor.Show();
    display.Flip();

    while(le.HandleEvents())
    {
        if(le.MouseClickLeft(rectScreen1)) return SCREEN1;
        else
        if(le.MouseClickLeft(rectScreen3)) return SCREEN3;
        else
        if(le.MouseClickLeft(rectScreen4)) return SCREEN4;
        else
        if(le.MouseClickLeft(rectScreen5)) return SCREEN5;
        else
        if(castle.isBuild(BUILD_MAGEGUILD1) && le.MouseClickLeft(rectScreen6)) return SCREEN6;
        else
        // exit
        if(le.MouseClickLeft(rectExit) || Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT)) break;

	if(le.MouseCursor(dwelling1.GetArea()) && dwelling1.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(dwelling1()); return SCREEN1; }
	else
	if(le.MouseCursor(dwelling2.GetArea()) && dwelling2.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(dwelling2()); return SCREEN1; }
	else
	if(le.MouseCursor(dwelling3.GetArea()) && dwelling3.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(dwelling3()); return SCREEN1; }
	else
	if(le.MouseCursor(dwelling4.GetArea()) && dwelling4.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(dwelling4()); return SCREEN1; }
	else
	if(le.MouseCursor(dwelling5.GetArea()) && dwelling5.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(dwelling5()); return SCREEN1; }
	else
	if(le.MouseCursor(dwelling6.GetArea()) && dwelling6.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(dwelling6()); return SCREEN1; }
    }
    return SCREENOUT;
}

screen_t CastleOpenDialog3(Castle & castle)
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const u16 window_w = 320;
    const u16 window_h = 224;

    Dialog::FrameBorder frameborder;
    frameborder.SetPosition((display.w() - window_w) / 2 - BORDERWIDTH, (display.h() - window_h) / 2 - BORDERWIDTH, window_w, window_h);
    frameborder.Redraw();

    const Rect & dst_rt = frameborder.GetArea();
    const Sprite & background = AGG::GetICN(ICN::STONEBAK, 0);
    display.Blit(background, Rect(0, 0, window_w, window_h), dst_rt);

    const Rect rectExit(dst_rt.x + dst_rt.w - 26, dst_rt.y + 7, 25, 25);
    display.Blit(AGG::GetICN(ICN::TOWNWIND, 12), rectExit.x, rectExit.y);

    const Rect rectScreen1(dst_rt.x + dst_rt.w - 27, dst_rt.y + 32, 25, 25);
    const Rect rectScreen2(dst_rt.x + dst_rt.w - 27, dst_rt.y + 58, 25, 25);
    const Rect rectScreen3(dst_rt.x + dst_rt.w - 27, dst_rt.y + 84, 25, 25);
    const Rect rectScreen4(dst_rt.x + dst_rt.w - 27, dst_rt.y + 110, 25, 25);
    const Rect rectScreen5(dst_rt.x + dst_rt.w - 27, dst_rt.y + 136, 25, 25);
    const Rect rectScreen6(dst_rt.x + dst_rt.w - 27, dst_rt.y + 162, 25, 25);

    display.Blit(AGG::GetICN(ICN::REQUESTS, 20), rectScreen1.x, rectScreen1.y);
    display.Blit(AGG::GetICN(ICN::REQUESTS, 21), rectScreen2.x, rectScreen2.y);
    display.Blit(AGG::GetICN(ICN::REQUESTS, 22), rectScreen3.x, rectScreen3.y);
    display.Blit(AGG::GetICN(ICN::REQUESTS, 23), rectScreen4.x, rectScreen4.y);
    display.Blit(AGG::GetICN(ICN::REQUESTS, 24), rectScreen5.x, rectScreen5.y);
    if(castle.isBuild(BUILD_MAGEGUILD1)) display.Blit(AGG::GetICN(ICN::REQUESTS, 25), rectScreen6.x, rectScreen6.y);

    building_t level = BUILD_NOTHING;
    switch(castle.GetLevelMageGuild())
    {
        case 0: level = BUILD_MAGEGUILD1; break;
        case 1: level = BUILD_MAGEGUILD2; break;
        case 2: level = BUILD_MAGEGUILD3; break;
        case 3: level = BUILD_MAGEGUILD4; break;
        default:level = BUILD_MAGEGUILD5; break;
    }

    BuildingInfo building1(castle, level);
    building1.SetPos(dst_rt.x + 2, dst_rt.y + 2);
    building1.Redraw();

    BuildingInfo building2(castle, BUILD_TAVERN);
    building2.SetPos(dst_rt.x + 141, dst_rt.y + 2);
    building2.Redraw();

    BuildingInfo building3(castle, BUILD_THIEVESGUILD);
    building3.SetPos(dst_rt.x + 2, dst_rt.y + 76);
    building3.Redraw();

    BuildingInfo building4(castle, BUILD_SHIPYARD);
    building4.SetPos(dst_rt.x + 141, dst_rt.y + 76);
    building4.Redraw();

    BuildingInfo building5(castle, BUILD_STATUE);
    building5.SetPos(dst_rt.x + 2, dst_rt.y + 150);
    building5.Redraw();

    BuildingInfo building6(castle, BUILD_MARKETPLACE);
    building6.SetPos(dst_rt.x + 141, dst_rt.y + 150);
    building6.Redraw();

    cursor.Show();
    display.Flip();

    while(le.HandleEvents())
    {
        if(le.MouseClickLeft(rectScreen1)) return SCREEN1;
        else
        if(le.MouseClickLeft(rectScreen2)) return SCREEN2;
        else
        if(le.MouseClickLeft(rectScreen4)) return SCREEN4;
        else
        if(le.MouseClickLeft(rectScreen5)) return SCREEN5;
        else
        if(castle.isBuild(BUILD_MAGEGUILD1) && le.MouseClickLeft(rectScreen6)) return SCREEN6;
        else
        // exit
        if(le.MouseClickLeft(rectExit) || Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT)) break;

	if(le.MouseCursor(building1.GetArea()) && building1.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(building1()); return SCREEN1; }
	else
	if(le.MouseCursor(building2.GetArea()) && building2.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(building2()); return SCREEN1; }
	else
	if(le.MouseCursor(building3.GetArea()))
	{
	    // show thieves guild
	    if(castle.isBuild(BUILD_THIEVESGUILD))
	    {
		if(le.MouseClickLeft(building3.GetArea())){ PocketPC::ThievesGuild(false); return SCREEN1; }
	    }
	    else
	    if(building3.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(building3()); return SCREEN1; }
	}
	else
	if(le.MouseCursor(building4.GetArea()))
	{
	    // buy boat
	    if(castle.isBuild(BUILD_SHIPYARD))
	    {
		if(le.MouseClickLeft(building4.GetArea()) && Dialog::OK == Dialog::BuyBoat(castle.AllowBuyBoat())) castle.BuyBoat();
	    }
	    else
	    if(building4.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(building4()); return SCREEN1; }
	}
	else
	if(le.MouseCursor(building5.GetArea()) && building5.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(building5()); return SCREEN1; }
	else
	if(le.MouseCursor(building6.GetArea()))
	{
	    // show marketplace
	    if(castle.isBuild(BUILD_MARKETPLACE))
	    {
		if(le.MouseClickLeft(building6.GetArea())) Dialog::Marketplace();
	    }
	    else
	    if(building6.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(building6()); return SCREEN1; }
	}
    }
    return SCREENOUT;
}

screen_t CastleOpenDialog4(Castle & castle)
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const u16 window_w = 320;
    const u16 window_h = 224;

    Dialog::FrameBorder frameborder;
    frameborder.SetPosition((display.w() - window_w) / 2 - BORDERWIDTH, (display.h() - window_h) / 2 - BORDERWIDTH, window_w, window_h);
    frameborder.Redraw();

    const Rect & dst_rt = frameborder.GetArea();
    const Sprite & background = AGG::GetICN(ICN::STONEBAK, 0);
    display.Blit(background, Rect(0, 0, window_w, window_h), dst_rt);

    const Rect rectExit(dst_rt.x + dst_rt.w - 26, dst_rt.y + 7, 25, 25);
    display.Blit(AGG::GetICN(ICN::TOWNWIND, 12), rectExit.x, rectExit.y);

    const Rect rectScreen1(dst_rt.x + dst_rt.w - 27, dst_rt.y + 32, 25, 25);
    const Rect rectScreen2(dst_rt.x + dst_rt.w - 27, dst_rt.y + 58, 25, 25);
    const Rect rectScreen3(dst_rt.x + dst_rt.w - 27, dst_rt.y + 84, 25, 25);
    const Rect rectScreen4(dst_rt.x + dst_rt.w - 27, dst_rt.y + 110, 25, 25);
    const Rect rectScreen5(dst_rt.x + dst_rt.w - 27, dst_rt.y + 136, 25, 25);
    const Rect rectScreen6(dst_rt.x + dst_rt.w - 27, dst_rt.y + 162, 25, 25);

    display.Blit(AGG::GetICN(ICN::REQUESTS, 20), rectScreen1.x, rectScreen1.y);
    display.Blit(AGG::GetICN(ICN::REQUESTS, 21), rectScreen2.x, rectScreen2.y);
    display.Blit(AGG::GetICN(ICN::REQUESTS, 22), rectScreen3.x, rectScreen3.y);
    display.Blit(AGG::GetICN(ICN::REQUESTS, 23), rectScreen4.x, rectScreen4.y);
    display.Blit(AGG::GetICN(ICN::REQUESTS, 24), rectScreen5.x, rectScreen5.y);
    if(castle.isBuild(BUILD_MAGEGUILD1)) display.Blit(AGG::GetICN(ICN::REQUESTS, 25), rectScreen6.x, rectScreen6.y);

    BuildingInfo building1(castle, BUILD_WELL);
    building1.SetPos(dst_rt.x + 2, dst_rt.y + 2);
    building1.Redraw();

    BuildingInfo building2(castle, BUILD_WEL2);
    building2.SetPos(dst_rt.x + 141, dst_rt.y + 2);
    building2.Redraw();

    BuildingInfo building3(castle, BUILD_SPEC);
    building3.SetPos(dst_rt.x + 2, dst_rt.y + 76);
    building3.Redraw();

    BuildingInfo building4(castle, BUILD_LEFTTURRET);
    building4.SetPos(dst_rt.x + 141, dst_rt.y + 76);
    building4.Redraw();

    BuildingInfo building5(castle, BUILD_RIGHTTURRET);
    building5.SetPos(dst_rt.x + 2, dst_rt.y + 150);
    building5.Redraw();

    BuildingInfo building6(castle, BUILD_MOAT);
    building6.SetPos(dst_rt.x + 141, dst_rt.y + 150);
    building6.Redraw();

    cursor.Show();
    display.Flip();

    while(le.HandleEvents())
    {
        if(le.MouseClickLeft(rectScreen1)) return SCREEN1;
        else
        if(le.MouseClickLeft(rectScreen2)) return SCREEN2;
        else
        if(le.MouseClickLeft(rectScreen3)) return SCREEN3;
        else
        if(le.MouseClickLeft(rectScreen5)) return SCREEN5;
        else
        if(castle.isBuild(BUILD_MAGEGUILD1) && le.MouseClickLeft(rectScreen6)) return SCREEN6;
        else
        // exit
        if(le.MouseClickLeft(rectExit) || Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT)) break;

	if(le.MouseCursor(building1.GetArea()) && building1.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(building1()); return SCREEN1; }
	else
	if(le.MouseCursor(building2.GetArea()) && building2.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(building2()); return SCREEN1; }
	else
	if(le.MouseCursor(building3.GetArea()) && building3.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(building3()); return SCREEN1; }
	else
	if(le.MouseCursor(building4.GetArea()) && building4.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(building4()); return SCREEN1; }
	else
	if(le.MouseCursor(building5.GetArea()) && building5.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(building5()); return SCREEN1; }
	else
	if(le.MouseCursor(building6.GetArea()) && building6.QueueEventProcessing()) { AGG::PlaySound(M82::BUILDTWN); castle.BuyBuilding(building6()); return SCREEN1; }
    }
    return SCREENOUT;
}

screen_t CastleOpenDialog5(Castle & castle)
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const u16 window_w = 320;
    const u16 window_h = 224;

    Dialog::FrameBorder frameborder;
    frameborder.SetPosition((display.w() - window_w) / 2 - BORDERWIDTH, (display.h() - window_h) / 2 - BORDERWIDTH, window_w, window_h);
    frameborder.Redraw();

    const Rect & dst_rt = frameborder.GetArea();
    const Sprite & background = AGG::GetICN(ICN::STONEBAK, 0);
    display.Blit(background, Rect(0, 0, window_w, window_h), dst_rt);

    // tavern
    Point dst_pt;
    if(castle.isBuild(BUILD_TAVERN))
    {
	Text text;
	text.Set(castle.GetStringBuilding(BUILD_TAVERN), Font::SMALL);
	text.Blit(dst_rt.x + (dst_rt.w - text.w()) / 2, dst_rt.y + 3);

	TextBox box1(_("A generous tip for the barkeep yields the following rumor:"), Font::SMALL, 186);
	TextBox box2(world.GetRumors(), Font::SMALL, 186);

	box1.Blit(dst_rt.x + 67, dst_rt.y + 120);
	box2.Blit(dst_rt.x + 67, dst_rt.y + 130 + box1.h());

	const Sprite & s1 = AGG::GetICN(ICN::TAVWIN, 0);
	dst_pt = Point(dst_rt.x + (dst_rt.w - s1.w()) / 2, dst_rt.y + 18);
	display.Blit(s1, dst_pt);

	const Sprite & s20 = AGG::GetICN(ICN::TAVWIN, 1);
	display.Blit(s20, dst_pt.x + 3, dst_pt.y + 3);
    }

    Rect rectRecruit1, rectRecruit2, rectCaptain;
    Heroes* hero1 = NULL;
    Heroes* hero2 = NULL;

    if(castle.isBuild(BUILD_CASTLE))
    {
	// hero
	const Sprite & crest = AGG::GetICN(ICN::BRCREST, 6);
	rectRecruit1 = Rect(dst_rt.x + 4, dst_rt.y + 18, crest.w(), crest.h());
	rectRecruit2 = Rect(dst_rt.x + 4, dst_rt.y + 77, crest.w(), crest.h());
	rectCaptain = Rect(dst_rt.x + 4, dst_rt.y + 136, crest.w(), crest.h());

	hero1 = world.GetMyKingdom().GetRecruits().GetHero1();
	hero2 = world.GetMyKingdom().GetRecruits().GetHero2();

	display.Blit(crest, rectRecruit1);
	if(hero1) display.Blit(hero1->GetPortrait50x46(), rectRecruit1.x + 4, rectRecruit1.y + 4);

	display.Blit(crest, rectRecruit2);
        if(hero2) display.Blit(hero2->GetPortrait50x46(), rectRecruit2.x + 4, rectRecruit2.y + 4);

	// captain
	display.Blit(crest, rectCaptain);
	const Surface & captain = Portrait::Captain(castle.GetRace(), Portrait::BIG);
        display.Blit(captain, Rect((captain.w() - 50) / 2, 15, 50, 47), rectCaptain.x + 4, rectCaptain.y + 4);
    }

    // buttons
    const Rect rectExit(dst_rt.x + dst_rt.w - 26, dst_rt.y + 7, 25, 25);
    display.Blit(AGG::GetICN(ICN::TOWNWIND, 12), rectExit.x, rectExit.y);

    const Rect rectScreen1(dst_rt.x + dst_rt.w - 27, dst_rt.y + 32, 25, 25);
    const Rect rectScreen2(dst_rt.x + dst_rt.w - 27, dst_rt.y + 58, 25, 25);
    const Rect rectScreen3(dst_rt.x + dst_rt.w - 27, dst_rt.y + 84, 25, 25);
    const Rect rectScreen4(dst_rt.x + dst_rt.w - 27, dst_rt.y + 110, 25, 25);
    const Rect rectScreen5(dst_rt.x + dst_rt.w - 27, dst_rt.y + 136, 25, 25);
    const Rect rectScreen6(dst_rt.x + dst_rt.w - 27, dst_rt.y + 162, 25, 25);

    display.Blit(AGG::GetICN(ICN::REQUESTS, 20), rectScreen1.x, rectScreen1.y);
    if(castle.isBuild(BUILD_CASTLE))
    {
	display.Blit(AGG::GetICN(ICN::REQUESTS, 21), rectScreen2.x, rectScreen2.y);
	display.Blit(AGG::GetICN(ICN::REQUESTS, 22), rectScreen3.x, rectScreen3.y);
	display.Blit(AGG::GetICN(ICN::REQUESTS, 23), rectScreen4.x, rectScreen4.y);
    }
    display.Blit(AGG::GetICN(ICN::REQUESTS, 24), rectScreen5.x, rectScreen5.y);
    if(castle.isBuild(BUILD_MAGEGUILD1)) display.Blit(AGG::GetICN(ICN::REQUESTS, 25), rectScreen6.x, rectScreen6.y);

    u32 frame = 0;
    
    cursor.Show();
    display.Flip();

    while(le.HandleEvents())
    {
        if(le.MouseClickLeft(rectScreen1)) return SCREEN1;
        else
        if(le.MouseClickLeft(rectScreen2)) return SCREEN2;
        else
        if(le.MouseClickLeft(rectScreen3)) return SCREEN3;
        else
        if(le.MouseClickLeft(rectScreen4)) return SCREEN4;
        else
        if(castle.isBuild(BUILD_MAGEGUILD1) && le.MouseClickLeft(rectScreen6)) return SCREEN6;
        else
        // exit
        if(le.MouseClickLeft(rectExit) || Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT)) break;
	else
	if(hero1 && le.MouseClickLeft(rectRecruit1) &&
	    Dialog::OK == castle.DialogBuyHero(hero1))
	{
    	    castle.RecruitHero(hero1);
	    return SCREEN1;
        }
	else
	if(hero2 && le.MouseClickLeft(rectRecruit2) &&
	    Dialog::OK == castle.DialogBuyHero(hero2))
	{
    	    castle.RecruitHero(hero2);
	    return SCREEN1;
        }
	else
	if(le.MouseClickLeft(rectCaptain))
	{
	    BuildingInfo b(castle, BUILD_CAPTAIN);
	    if(castle.isBuild(BUILD_CAPTAIN))
		Dialog::Message(b.GetName(), b.GetDescription(), Font::SMALL, Dialog::OK);
	    else
	    if(b.DialogBuyBuilding(true))
	    {
		AGG::PlaySound(M82::BUILDTWN);
		castle.BuyBuilding(b());
		return SCREEN1;
	    }
	}

        // animation
        if(castle.isBuild(BUILD_TAVERN) && Game::AnimateInfrequent(Game::CASTLE_TAVERN_DELAY))
        {
            cursor.Hide();
	    const Sprite & s20 = AGG::GetICN(ICN::TAVWIN, 1);
            display.Blit(s20, dst_pt.x + 3, dst_pt.y + 3);
            if(const u16 index = ICN::AnimationFrame(ICN::TAVWIN, 0, frame++))
            {
        	const Sprite & s22 = AGG::GetICN(ICN::TAVWIN, index);
                display.Blit(s22, dst_pt.x + s22.x() + 3, dst_pt.y + s22.y() + 3);
            }
    	    cursor.Show();
    	    display.Flip();
        }
    }

    return SCREENOUT;
}

screen_t CastleOpenDialog6(Castle & castle)
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const u16 window_w = 320;
    const u16 window_h = 224;

    Dialog::FrameBorder frameborder;
    frameborder.SetPosition((display.w() - window_w) / 2 - BORDERWIDTH, (display.h() - window_h) / 2 - BORDERWIDTH, window_w, window_h);
    frameborder.Redraw();

    const Rect & dst_rt = frameborder.GetArea();
    const Sprite & background = AGG::GetICN(ICN::STONEBAK, 0);
    display.Blit(background, Rect(0, 0, window_w, window_h), dst_rt);


    RowSpells spells1(Point(dst_rt.x + 38, dst_rt.y + 220 - 44), castle.GetMageGuild(), 1);
    RowSpells spells2(Point(dst_rt.x + 38, dst_rt.y + 220 - 44 * 2), castle.GetMageGuild(), 2);
    RowSpells spells3(Point(dst_rt.x + 38, dst_rt.y + 220 - 44 * 3), castle.GetMageGuild(), 3);
    RowSpells spells4(Point(dst_rt.x + 38, dst_rt.y + 220 - 44 * 4), castle.GetMageGuild(), 4);
    RowSpells spells5(Point(dst_rt.x + 38, dst_rt.y + 220 - 44 * 5), castle.GetMageGuild(), 5);

    spells1.Redraw();
    spells2.Redraw();
    spells3.Redraw();
    spells4.Redraw();
    spells5.Redraw();

    // magic book sprite
    const Heroes* hero = castle.GetHeroes();
    bool need_buy_book = hero && !hero->HasArtifact(Artifact::MAGIC_BOOK) && castle.GetLevelMageGuild();
    const Rect book_pos(dst_rt.x + 250, dst_rt.y + 5, 32, 32);
    if(need_buy_book)
    {
	display.Blit(AGG::GetICN(ICN::ARTFX, 81), book_pos);
	Text text(_("buy"), Font::SMALL);
	text.Blit(book_pos.x + (book_pos.w - text.w()) / 2, book_pos.y + book_pos.h - 12);
    }

    // buttons
    const Rect rectExit(dst_rt.x + dst_rt.w - 26, dst_rt.y + 7, 25, 25);
    display.Blit(AGG::GetICN(ICN::TOWNWIND, 12), rectExit.x, rectExit.y);

    const Rect rectScreen1(dst_rt.x + dst_rt.w - 27, dst_rt.y + 32, 25, 25);
    const Rect rectScreen2(dst_rt.x + dst_rt.w - 27, dst_rt.y + 58, 25, 25);
    const Rect rectScreen3(dst_rt.x + dst_rt.w - 27, dst_rt.y + 84, 25, 25);
    const Rect rectScreen4(dst_rt.x + dst_rt.w - 27, dst_rt.y + 110, 25, 25);
    const Rect rectScreen5(dst_rt.x + dst_rt.w - 27, dst_rt.y + 136, 25, 25);
    const Rect rectScreen6(dst_rt.x + dst_rt.w - 27, dst_rt.y + 162, 25, 25);

    display.Blit(AGG::GetICN(ICN::REQUESTS, 20), rectScreen1.x, rectScreen1.y);
    if(castle.isBuild(BUILD_CASTLE))
    {
	display.Blit(AGG::GetICN(ICN::REQUESTS, 21), rectScreen2.x, rectScreen2.y);
	display.Blit(AGG::GetICN(ICN::REQUESTS, 22), rectScreen3.x, rectScreen3.y);
	display.Blit(AGG::GetICN(ICN::REQUESTS, 23), rectScreen4.x, rectScreen4.y);
    }
    if(castle.isBuild(BUILD_CASTLE) || castle.isBuild(BUILD_TAVERN))
	display.Blit(AGG::GetICN(ICN::REQUESTS, 24), rectScreen5.x, rectScreen5.y);
    display.Blit(AGG::GetICN(ICN::REQUESTS, 25), rectScreen6.x, rectScreen6.y);

    cursor.Show();
    display.Flip();

    while(le.HandleEvents())
    {
        if(le.MouseClickLeft(rectScreen1)) return SCREEN1;
        else
        if(le.MouseClickLeft(rectScreen2)) return SCREEN2;
        else
        if(le.MouseClickLeft(rectScreen3)) return SCREEN3;
        else
        if(le.MouseClickLeft(rectScreen4)) return SCREEN4;
        else
        if(le.MouseClickLeft(rectScreen5)) return SCREEN5;
        else
        // exit
        if(le.MouseClickLeft(rectExit) || Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT)) break;
	else
	if(need_buy_book && le.MouseClickLeft(book_pos)) { const_cast<Heroes *>(hero)->BuySpellBook(&castle.GetMageGuild()); return SCREEN1; }

        spells1.QueueEventProcessing();
        spells2.QueueEventProcessing();
        spells3.QueueEventProcessing();
        spells4.QueueEventProcessing();
        spells5.QueueEventProcessing();
    }

    return SCREENOUT;
}


void RedrawResourceBar(const Point & dst, const Resource::funds_t & rs)
{
    Display & display = Display::Get();
    display.Blit(AGG::GetICN(ICN::STONEBAK, 0), Rect(0, 0, 312, 13), dst.x, dst.y + 30);

    const Sprite & ore = AGG::GetICN(ICN::RESOURCE, 2);
    const Sprite & wood = AGG::GetICN(ICN::RESOURCE, 0);
    const Sprite & mercury = AGG::GetICN(ICN::RESOURCE, 1);
    const Sprite & sulfur = AGG::GetICN(ICN::RESOURCE, 3);
    const Sprite & crystal = AGG::GetICN(ICN::RESOURCE, 4);
    const Sprite & gems = AGG::GetICN(ICN::RESOURCE, 5);
    const Sprite & gold = AGG::GetICN(ICN::RESOURCE, 6);

    Text text;
    std::string str;

    display.Blit(ore, dst.x + 22 - ore.w() / 2, dst.y + 34 - ore.h());
    str.clear();
    String::AddInt(str, rs.ore);
    text.Set(str, Font::SMALL);
    text.Blit(dst.x + 22 - text.w() / 2, dst.y + 33);

    display.Blit(wood, dst.x + 68 - wood.w() / 2, dst.y + 34 - wood.h());
    str.clear();
    String::AddInt(str, rs.wood);
    text.Set(str, Font::SMALL);
    text.Blit(dst.x + 68 - text.w() / 2, dst.y + 33);

    display.Blit(mercury, dst.x + 114 - mercury.w() / 2, dst.y + 34 - mercury.h());
    str.clear();
    String::AddInt(str, rs.mercury);
    text.Set(str, Font::SMALL);
    text.Blit(dst.x + 114 - text.w() / 2, dst.y + 33);

    display.Blit(sulfur, dst.x + 160 - sulfur.w() / 2, dst.y + 34 - sulfur.h());
    str.clear();
    String::AddInt(str, rs.sulfur);
    text.Set(str, Font::SMALL);
    text.Blit(dst.x + 160 - text.w() / 2, dst.y + 33);

    display.Blit(crystal, dst.x + 206 - crystal.w() / 2, dst.y + 34 - crystal.h());
    str.clear();
    String::AddInt(str, rs.crystal);
    text.Set(str, Font::SMALL);
    text.Blit(dst.x + 206 - text.w() / 2, dst.y + 33);

    display.Blit(gems, dst.x + 252 - gems.w() / 2, dst.y + 34 - gems.h());
    str.clear();
    String::AddInt(str, rs.gems);
    text.Set(str, Font::SMALL);
    text.Blit(dst.x + 252 - text.w() / 2, dst.y + 33);

    display.Blit(gold, Rect(0, 0, 40, gold.h()), dst.x + 292 - 20, dst.y + 34 - gold.h());
    str.clear();
    String::AddInt(str, rs.gold);
    text.Set(str, Font::SMALL);
    text.Blit(dst.x + 292 - text.w() / 2, dst.y + 33);
}

void RedrawBackground(const Rect & rt, const Castle & castle)
{
    const Sprite* sprite = NULL;
    Rect src;

    switch(castle.GetRace())
    {
	case Race::KNGT: sprite = &AGG::GetICN(ICN::TOWNBKG0, 0); src = Rect(148, 0, rt.w, 123); break;
	case Race::BARB: sprite = &AGG::GetICN(ICN::TOWNBKG1, 0); src = Rect(142, 0, rt.w, 123); break;
	case Race::SORC: sprite = &AGG::GetICN(ICN::TOWNBKG2, 0); src = Rect(218, 0, rt.w, 123); break;
	case Race::WRLK: sprite = &AGG::GetICN(ICN::TOWNBKG3, 0); src = Rect(300, 0, rt.w, 123); break;
	case Race::WZRD: sprite = &AGG::GetICN(ICN::TOWNBKG4, 0); src = Rect(150, 0, rt.w, 123); break;
	case Race::NECR: sprite = &AGG::GetICN(ICN::TOWNBKG5, 0); src = Rect(0, 0, rt.w, 123); break;
	default: break;
    }

    if(sprite) Display::Get().Blit(*sprite, src, rt.x, rt.y);
}

const Rect & DwellingBar::GetArea(void) const
{
    return *this;
}

DwellingBar::DwellingBar(const Point & dst, const Castle & cst) : Rect(dst.x, dst.y, 0, 0), castle(cst), dw(CASTLEMAXMONSTER)
{
    for(u8 ii = 0; ii < dw.size(); ++ii)
	dw[ii] = Rect(dst.x + ii * (43 + 2), dst.y, 43, 43);

    const Rect max = Rect(dw);

    w = max.w;
    h = max.h;
}

u32 DwellingBar::GetDwellingFromIndex(u8 ii)
{
    switch(ii)
    {
	case 0: return DWELLING_MONSTER1;
	case 1: return DWELLING_MONSTER2;
	case 2: return DWELLING_MONSTER3;
	case 3: return DWELLING_MONSTER4;
	case 4: return DWELLING_MONSTER5;
	case 5: return DWELLING_MONSTER6;
	default: break;
    }
    return 0;
}

void DwellingBar::Redraw(void) const
{
    Display & display = Display::Get();
    const u8 w = 43;
    const u8 h = 43;
    std::string str;
    Text text(str, Font::SMALL);

    for(u8 ii = 0; ii < dw.size(); ++ii)
    {
    	display.Blit(AGG::GetICN(ICN::SWAPWIN, 0), Rect(36, 267, w, h), dw[ii].x, dw[ii].y);
    	const u32 dwelling = castle.GetActualDwelling(GetDwellingFromIndex(ii));
	Monster m(castle.GetRace(), dwelling);
	const Sprite & mons32 = AGG::GetICN(ICN::MONS32, Monster::GetSpriteIndex(m()));
    	display.Blit(mons32, dw[ii].x + (w - mons32.w()) / 2, dw[ii].y + (h - 3 - mons32.h()));

    	if(castle.isBuild(dwelling))
    	{
	    // count
    	    str.clear();
    	    String::AddInt(str, castle.GetDwellingLivedCount(dwelling));
    	    text.Set(str);
    	    text.Blit(dw[ii].x + dw[ii].w - text.w() - 3, dw[ii].y + dw[ii].h - text.h() - 1);

	    // grown
	    u8 grown = m.GetGrown();
	    if(castle.isBuild(BUILD_WELL)) grown += Castle::GetGrownWell();
	    if(castle.isBuild(BUILD_WEL2) && DWELLING_MONSTER1 == dwelling) grown += Castle::GetGrownWel2();
	    str = "+";
    	    String::AddInt(str, grown);
    	    text.Set(str, Font::YELLOW_SMALL);
    	    text.Blit(dw[ii].x + dw[ii].w - text.w() - 3, dw[ii].y + 2);
    	}
    	else
    	{
    	    display.Blit(AGG::GetICN(ICN::LOCATORS, 24), dw[ii].x + w - 8, dw[ii].y + 3);
    	}
    }
}

bool DwellingBar::QueueEventProcessing(void)
{
    LocalEvent & le = LocalEvent::Get();
    std::vector<Rect>::const_iterator itr;

    if(dw.end() != (itr = std::find_if(dw.begin(), dw.end(), std::bind2nd(RectIncludePoint(), le.GetMouseCursor()))))
    {
	const u32 dwelling = GetDwellingFromIndex(itr - dw.begin());
	if(castle.isBuild(dwelling))
	{
	    const u16 recruit = Dialog::RecruitMonster(Monster(castle.GetRace(), castle.GetActualDwelling(dwelling)), castle.GetDwellingLivedCount(dwelling));
            return const_cast<Castle &>(castle).RecruitMonster(dwelling, recruit);
	}
	else
	{
	    BuildingInfo dwelling2(castle, static_cast<building_t>(dwelling));
	    if(dwelling2.DialogBuyBuilding(true))
	    {
		AGG::PlaySound(M82::BUILDTWN);
		const_cast<Castle &>(castle).BuyBuilding(dwelling);
		return true;
	    }
	}
    }
    return false;
}

void RedrawTownSprite(const Rect & rt, const Castle & castle)
{
    Display & display = Display::Get();
    const Sprite & slock = AGG::GetICN(ICN::LOCATORS, 23);
    display.Blit(slock, rt.x, rt.y);
    switch(castle.GetRace())
    {
        case Race::KNGT: display.Blit(AGG::GetICN(ICN::LOCATORS, castle.isCastle() ?  9 : 15),  rt.x + 4, rt.y + 4); break;
        case Race::BARB: display.Blit(AGG::GetICN(ICN::LOCATORS, castle.isCastle() ?  10 : 16), rt.x + 4, rt.y + 4); break;
        case Race::SORC: display.Blit(AGG::GetICN(ICN::LOCATORS, castle.isCastle() ?  11 : 17), rt.x + 4, rt.y + 4); break;
        case Race::WRLK: display.Blit(AGG::GetICN(ICN::LOCATORS, castle.isCastle() ?  12 : 18), rt.x + 4, rt.y + 4); break;
        case Race::WZRD: display.Blit(AGG::GetICN(ICN::LOCATORS, castle.isCastle() ?  13 : 19), rt.x + 4, rt.y + 4); break;
        case Race::NECR: display.Blit(AGG::GetICN(ICN::LOCATORS, castle.isCastle() ?  14 : 20), rt.x + 4, rt.y + 4); break;
        default: break;
    }
    if(! castle.AllowBuild()) display.Blit(AGG::GetICN(ICN::LOCATORS, 24), rt.x + 43, rt.y + 5);
}

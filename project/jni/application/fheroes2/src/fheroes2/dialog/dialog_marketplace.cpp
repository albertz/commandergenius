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
#include <vector>
#include "agg.h"
#include "button.h"
#include "world.h"
#include "cursor.h"
#include "settings.h"
#include "resource.h"
#include "kingdom.h"
#include "splitter.h"
#include "marketplace.h"
#include "dialog.h"
#include "localclient.h"

void RedrawFromResource(const Point & pt, const Resource::funds_t & rs);
void RedrawToResource(const Point & pt, bool showcost, bool tradingPost, u8 from_resource = 0);
void GetStringTradeCosts(std::string & str, u8 rs_from, u8 rs_to, bool tradingPost);
u16 GetTradeCosts(u8 rs_from, u8 rs_to, bool tradingPost);

class TradeWindowGUI
{
public:
    TradeWindowGUI(const Rect & rt) : 
	pos_rt(rt), back(rt.x - 5, rt.y + 15, rt.w + 10, 160),
	tradpost(Settings::Get().EvilInterface() ? ICN::TRADPOSE : ICN::TRADPOST)
    {
	back.Save();

        buttonTrade.SetPos(pos_rt.x + (pos_rt.w - AGG::GetICN(tradpost, 17).w()) / 2, pos_rt.y + 150);
	buttonLeft.SetPos(pos_rt.x + 11, pos_rt.y + 129);
	buttonRight.SetPos(pos_rt.x + 220, pos_rt.y + 129);

	buttonTrade.SetSprite(tradpost, 15, 16);
	buttonLeft.SetSprite(tradpost, 3, 4);
	buttonRight.SetSprite(tradpost, 5, 6);

        splitter.SetSprite(AGG::GetICN(tradpost, 2));
        splitter.SetArea(Rect(pos_rt.x + (pos_rt.w - AGG::GetICN(tradpost, 1).w()) / 2 + 22, pos_rt.y + 132, 188, 10));
        splitter.Hide();
	splitter.SetOrientation(Splitter::HORIZONTAL);

	TextBox(_("Please inspect our fine wares. If you feel like offering a trade, click on the items you wish to trade with and for."), Font::BIG, Rect(pos_rt.x, pos_rt.y + 30, pos_rt.w, 100));

        textSell.SetFont(Font::SMALL);
        textBuy.SetFont(Font::SMALL);
    };

    void RedrawInfoBuySell(u32 count_sell, u32 count_buy);
    void ShowTradeArea(u8 resourceFrom, u8 resourceTo, u32 max_buy, u32 max_sell, u32 count_buy, u32 count_sell, bool fromTradingPost);

    Button buttonTrade;
    Button buttonLeft;
    Button buttonRight;
    Splitter splitter;

private:
    Rect pos_rt;
    Background back;
    ICN::icn_t tradpost;

    TextSprite textSell;
    TextSprite textBuy;
};

void TradeWindowGUI::ShowTradeArea(u8 resourceFrom, u8 resourceTo, u32 max_buy, u32 max_sell, u32 count_buy, u32 count_sell, bool fromTradingPost)
{
    Cursor &cursor = Cursor::Get();
    Display &display = Display::Get();

    if(resourceFrom == resourceTo || (Resource::GOLD != resourceTo && 0 == max_buy))
    {
        cursor.Hide();
        back.Restore();
        Rect dst_rt(pos_rt.x, pos_rt.y + 30, pos_rt.w, 100);
        TextBox(_("You have received quite a bargain. I expect to make no profit on the deal. Can I interest you in any of my other wares?"), Font::BIG, dst_rt);
	buttonTrade.SetDisable(true);
        buttonLeft.SetDisable(true);
        buttonRight.SetDisable(true);
        cursor.Show();
        display.Flip();
    }
    else
    {
        cursor.Hide();
        back.Restore();

        Point dst_pt;
        const Sprite & bar = AGG::GetICN(tradpost, 1);
        dst_pt.x = pos_rt.x + (pos_rt.w - bar.w()) / 2 - 2;
        dst_pt.y = pos_rt.y + 128;
        display.Blit(bar, dst_pt);
        splitter.SetRange(0, (Resource::GOLD == resourceTo ? max_sell : max_buy));
        Resource::resource_t rs_from = static_cast<Resource::resource_t>(resourceFrom);
        Resource::resource_t rs_to   = static_cast<Resource::resource_t>(resourceTo);
        u16 exchange_rate = GetTradeCosts(resourceFrom, resourceTo, fromTradingPost);
	std::string message;
        if(Resource::GOLD == resourceTo)
        {
            message = _("I can offer you %{count} for 1 unit of %{resfrom}.");
            String::Replace(message, "%{count}", exchange_rate);
            String::Replace(message, "%{resfrom}", Resource::String(rs_from));
        }
        else
        {
            message = _("I can offer you 1 unit of %{resto} for %{count} units of %{resfrom}.");
            String::Replace(message, "%{resto}", Resource::String(rs_to));
            String::Replace(message, "%{resfrom}", Resource::String(rs_from));
            String::Replace(message, "%{count}", exchange_rate);
        }
        TextBox(message, Font::BIG, Rect(pos_rt.x, pos_rt.y + 30, pos_rt.w, 100));
        const Sprite & sprite_from = AGG::GetICN(ICN::RESOURCE, Resource::GetIndexSprite2(rs_from));
        dst_pt.x = pos_rt.x + pos_rt.w / 2 - 70 - sprite_from.w() / 2;
        dst_pt.y = pos_rt.y + 115 - sprite_from.h();
        display.Blit(sprite_from, dst_pt);
        message.clear();
        String::AddInt(message, count_sell);
        const Sprite & sprite_to = AGG::GetICN(ICN::RESOURCE, Resource::GetIndexSprite2(rs_to));
        dst_pt.x = pos_rt.x + pos_rt.w / 2 + 70 - sprite_to.w() / 2;
        dst_pt.y = pos_rt.y + 115 - sprite_to.h();
        display.Blit(sprite_to, dst_pt);
        message.clear();
        String::AddInt(message, count_buy);
        const Sprite & sprite_fromto = AGG::GetICN(tradpost, 0);
        dst_pt.x = pos_rt.x + pos_rt.w / 2 - sprite_fromto.w() / 2;
        dst_pt.y = pos_rt.y + 90;
        display.Blit(sprite_fromto, dst_pt);
        Text text(_("Qty to trade"), Font::SMALL);
        dst_pt.x = pos_rt.x + (pos_rt.w - text.w()) / 2;
        dst_pt.y = pos_rt.y + 110;
        text.Blit(dst_pt);

        buttonTrade.SetDisable(false);
        buttonLeft.SetDisable(false);
        buttonRight.SetDisable(false);

        buttonTrade.Draw();
        buttonLeft.Draw();
        buttonRight.Draw();

        RedrawInfoBuySell(count_sell, count_buy);
        cursor.Show();
        display.Flip();
    }
}

void TradeWindowGUI::RedrawInfoBuySell(u32 count_sell, u32 count_buy)
{
    Point dst_pt;
    std::string message;

    String::AddInt(message, count_sell);
    textSell.Hide();
    textSell.SetText(message);
    dst_pt.x = pos_rt.x + pos_rt.w / 2 - 70 - textSell.w() / 2;
    dst_pt.y = pos_rt.y + 116;
    textSell.SetPos(dst_pt);
    textSell.Show();

    message.clear();
    String::AddInt(message, count_buy);
    textBuy.Hide();
    textBuy.SetText(message);
    dst_pt.x = pos_rt.x + pos_rt.w / 2 + 70 - textBuy.w() / 2;
    dst_pt.y = pos_rt.y + 116;
    textBuy.SetPos(dst_pt);
    textBuy.Show();
}

void Dialog::Marketplace(bool fromTradingPost)
{
    Display & display = Display::Get();
    const ICN::icn_t tradpost = Settings::Get().EvilInterface() ? ICN::TRADPOSE : ICN::TRADPOST;
    const std::string & header = _("Marketplace");

    Cursor & cursor = Cursor::Get();
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    Dialog::Box box(260, true);

    const Rect & pos_rt = box.GetArea();
    Point dst_pt(pos_rt.x, pos_rt.y);
    Rect dst_rt(pos_rt);
    Text text;

    // header
    text.Set(header, Font::BIG);
    dst_pt.x = pos_rt.x + (pos_rt.w - text.w()) / 2;
    dst_pt.y = pos_rt.y;
    text.Blit(dst_pt);

    TradeWindowGUI gui(pos_rt);

    Kingdom & kingdom = world.GetMyKingdom();
    const Sprite & spritecursor = AGG::GetICN(tradpost, 14);

    const std::string & header_from = _("Your Resources");

    Resource::funds_t fundsFrom = kingdom.GetFundsResource();
    u8 resourceFrom = 0;
    const Point pt1(pos_rt.x, pos_rt.y + 190);
    std::vector<Rect> rectsFrom(7);
    rectsFrom[0] = Rect(pt1.x, pt1.y, 34, 34);		// wood
    rectsFrom[1] = Rect(pt1.x + 37, pt1.y, 34, 34);	// mercury
    rectsFrom[2] = Rect(pt1.x + 74, pt1.y, 34, 34);	// ore
    rectsFrom[3] = Rect(pt1.x, pt1.y + 37, 34, 34);	// sulfur
    rectsFrom[4] = Rect(pt1.x + 37, pt1.y + 37, 34, 34);// crystal
    rectsFrom[5] = Rect(pt1.x + 74, pt1.y + 37, 34, 34);// gems
    rectsFrom[6] = Rect(pt1.x + 37, pt1.y + 74, 34, 34);// gold
    SpriteCursor cursorFrom(spritecursor);
    text.Set(header_from, Font::SMALL);
    dst_pt.x = pt1.x + (108 - text.w()) / 2;
    dst_pt.y = pt1.y - 15;
    text.Blit(dst_pt);
    RedrawFromResource(pt1, fundsFrom);

    const std::string & header_to = _("Available Trades");

    Resource::funds_t fundsTo;
    u8 resourceTo = 0;
    const Point pt2(138 + pos_rt.x, pos_rt.y + 190);
    std::vector<Rect> rectsTo(7);
    rectsTo[0] = Rect(pt2.x, pt2.y, 34, 34);		// wood
    rectsTo[1] = Rect(pt2.x + 37, pt2.y, 34, 34);	// mercury
    rectsTo[2] = Rect(pt2.x + 74, pt2.y, 34, 34);	// ore
    rectsTo[3] = Rect(pt2.x, pt2.y + 37, 34, 34);	// sulfur
    rectsTo[4] = Rect(pt2.x + 37, pt2.y + 37, 34, 34);	// crystal
    rectsTo[5] = Rect(pt2.x + 74, pt2.y + 37, 34, 34);	// gems
    rectsTo[6] = Rect(pt2.x + 37, pt2.y + 74, 34, 34);	// gold
    SpriteCursor cursorTo(spritecursor);
    text.Set(header_to, Font::SMALL);
    dst_pt.x = pt2.x + (108 - text.w()) / 2;
    dst_pt.y = pt2.y - 15;
    text.Blit(dst_pt);
    RedrawToResource(pt2, false, fromTradingPost);

    u32 count_sell = 0;
    u32 count_buy = 0;
    
    u32 max_sell = 0;
    u32 max_buy = 0;

    Button & buttonTrade = gui.buttonTrade;
    Button & buttonLeft = gui.buttonLeft;
    Button & buttonRight = gui.buttonRight;
    Splitter & splitter = gui.splitter;

    // button exit
    const Sprite & sprite_exit = AGG::GetICN(tradpost, 17);
    dst_pt.x = pos_rt.x + (pos_rt.w - sprite_exit.w()) / 2;
    dst_pt.y = pos_rt.y + pos_rt.h - sprite_exit.h();
    Button buttonExit(dst_pt, tradpost, 17, 18);

    buttonExit.Draw();

    cursor.Show();
    display.Flip();

    LocalEvent & le = LocalEvent::Get();
   
    // message loop
    while(le.HandleEvents())
    {
        if(buttonTrade.isEnable()) le.MousePressLeft(buttonTrade) ? buttonTrade.PressDraw() : buttonTrade.ReleaseDraw();
        if(buttonLeft.isEnable()) le.MousePressLeft(buttonLeft) ? buttonLeft.PressDraw() : buttonLeft.ReleaseDraw();
        if(buttonRight.isEnable()) le.MousePressLeft(buttonRight) ? buttonRight.PressDraw() : buttonRight.ReleaseDraw();

        le.MousePressLeft(buttonExit) ? buttonExit.PressDraw() : buttonExit.ReleaseDraw();

        if(le.MouseClickLeft(buttonExit) || HotKeyCloseWindow) break;
	
        // click from
        for(u8 ii = 0; ii < rectsFrom.size(); ++ii)
        {
            const Rect & rect_from = rectsFrom[ii];
            
            if(le.MouseClickLeft(rect_from))
            {
                switch(ii)
                {
                    case 0:
                        resourceFrom = Resource::WOOD;
                        max_sell = fundsFrom.wood;
                        break;
                    case 1:
                        resourceFrom = Resource::MERCURY;
                        max_sell = fundsFrom.mercury;
                        break;
                    case 2:
                        resourceFrom = Resource::ORE;
                        max_sell = fundsFrom.ore;
                        break;
                    case 3:
                        resourceFrom = Resource::SULFUR;
                        max_sell = fundsFrom.sulfur;
                        break;
                    case 4:
                        resourceFrom = Resource::CRYSTAL;
                        max_sell = fundsFrom.crystal;
                        break;
                    case 5:
                        resourceFrom = Resource::GEMS;
                        max_sell = fundsFrom.gems;
                        break;
                    case 6:
                        resourceFrom = Resource::GOLD;
                        max_sell = fundsFrom.gold;
                        break;
                    default: break;
                }
                
                if(GetTradeCosts(resourceFrom, resourceTo, fromTradingPost))
                {
                    max_buy = Resource::GOLD == resourceTo ? 
                        max_sell * GetTradeCosts(resourceFrom, resourceTo, fromTradingPost) :
                        max_sell / GetTradeCosts(resourceFrom, resourceTo, fromTradingPost);
                }
                
                count_sell = 0;
                count_buy = 0;
                
                cursor.Hide();
                cursorFrom.Move(rect_from.x - 2, rect_from.y - 2);
                cursorFrom.Show();
                
                if(resourceTo) cursorTo.Hide();
                RedrawToResource(pt2, true, fromTradingPost, resourceFrom);
                if(resourceTo) cursorTo.Show();
                if(resourceTo) gui.ShowTradeArea(resourceFrom, resourceTo, max_buy, max_sell, count_buy, count_sell, fromTradingPost);
                
                cursor.Show();
                display.Flip();
            }
        }
        
        // click to
        for(u8 ii = 0; ii < rectsTo.size(); ++ii)
        {
            const Rect & rect_to = rectsTo[ii];
            
            if(le.MouseClickLeft(rect_to))
            {
                switch(ii)
                {
                    case 0: resourceTo = Resource::WOOD; break;
                    case 1: resourceTo = Resource::MERCURY; break;
                    case 2: resourceTo = Resource::ORE; break;
                    case 3: resourceTo = Resource::SULFUR; break;
                    case 4: resourceTo = Resource::CRYSTAL; break;
                    case 5: resourceTo = Resource::GEMS; break;
                    case 6: resourceTo = Resource::GOLD; break;
                    default: break;
                }
                
                if(GetTradeCosts(resourceFrom, resourceTo, fromTradingPost))
                {
                    max_buy = Resource::GOLD == resourceTo ? 
                        max_sell * GetTradeCosts(resourceFrom, resourceTo, fromTradingPost) :
                        max_sell / GetTradeCosts(resourceFrom, resourceTo, fromTradingPost);
                }
                
                count_sell = 0;
                count_buy = 0;
                
                cursor.Hide();
                cursorTo.Move(rect_to.x - 2, rect_to.y - 2);
                
                if(resourceFrom)
                {
                    cursorTo.Hide();
                    RedrawToResource(pt2, true, fromTradingPost, resourceFrom);
                    cursorTo.Show();
                    gui.ShowTradeArea(resourceFrom, resourceTo, max_buy, max_sell, count_buy, count_sell, fromTradingPost);
                }
                cursor.Show();
                display.Flip();
            }
        }
        
        // move splitter
        if(buttonLeft.isEnable() && buttonRight.isEnable() && max_buy && le.MousePressLeft(splitter.GetRect()))
        {
            u32 seek = (le.GetMouseCursor().x - splitter.GetRect().x) * 100 / splitter.GetStep();
            
            if(seek < splitter.Min()) seek = splitter.Min();
            else
            if(seek > splitter.Max()) seek = splitter.Max();
            
            count_buy = seek * (Resource::GOLD == resourceTo ? GetTradeCosts(resourceFrom, resourceTo, fromTradingPost) : 1);
            count_sell = seek * (Resource::GOLD == resourceTo ? 1: GetTradeCosts(resourceFrom, resourceTo, fromTradingPost));
            
            cursor.Hide();
            splitter.Move(seek);
            gui.RedrawInfoBuySell(count_sell, count_buy);
            cursor.Show();
            display.Flip();
        }
        
        // trade
        if(buttonTrade.isEnable() && le.MouseClickLeft(buttonTrade) && count_sell && count_buy)
        {
            kingdom.OddFundsResource(Resource::funds_t(static_cast<Resource::resource_t>(resourceFrom), count_sell));
            kingdom.AddFundsResource(Resource::funds_t(static_cast<Resource::resource_t>(resourceTo), count_buy));
#ifdef WITH_NET
	    FH2LocalClient::SendMarketSellResource(kingdom, resourceFrom, count_sell, fromTradingPost);
	    FH2LocalClient::SendMarketBuyResource(kingdom, resourceTo, count_buy, fromTradingPost);
#endif            
            resourceTo = resourceFrom = Resource::UNKNOWN;
            gui.ShowTradeArea(resourceFrom, resourceTo, 0, 0, 0, 0, fromTradingPost);

            fundsFrom = kingdom.GetFundsResource();
            cursorTo.Hide();
            cursorFrom.Hide();
            RedrawFromResource(pt1, fundsFrom);
            RedrawToResource(pt2, false, fromTradingPost, resourceFrom);
            display.Flip();
        }
        
        // decrease trade resource
        if(count_buy &&
           ((buttonLeft.isEnable() && le.MouseClickLeft(gui.buttonLeft)) ||
            le.MouseWheelDn(splitter.GetRect())))
        {
            count_buy -= Resource::GOLD == resourceTo ? GetTradeCosts(resourceFrom, resourceTo, fromTradingPost) : 1;
            
            count_sell -= Resource::GOLD == resourceTo ? 1: GetTradeCosts(resourceFrom, resourceTo, fromTradingPost);
            
            cursor.Hide();
            splitter.Backward();
            gui.RedrawInfoBuySell(count_sell, count_buy);
            cursor.Show();
            display.Flip();
        }
        
        // increase trade resource
        if( count_buy < max_buy &&
            ((buttonRight.isEnable() && le.MouseClickLeft(buttonRight)) ||
             le.MouseWheelUp(splitter.GetRect())))
        {
            count_buy += Resource::GOLD == resourceTo ? GetTradeCosts(resourceFrom, resourceTo, fromTradingPost) : 1;
            
            count_sell += Resource::GOLD == resourceTo ? 1: GetTradeCosts(resourceFrom, resourceTo, fromTradingPost);
            
            cursor.Hide();
            splitter.Forward();
            gui.RedrawInfoBuySell(count_sell, count_buy);
            cursor.Show();
            display.Flip();            
        }
    }
}

void RedrawResourceSprite(const Surface & sf, s16 px, s16 py, s32 value)
{
    Display & display = Display::Get();
    std::string str;
    Text text;
    Point dst_pt(px, py);

    display.Blit(sf, dst_pt);
    String::AddInt(str, value);
    text.Set(str, Font::SMALL);
    dst_pt.x += (34 - text.w()) / 2;
    dst_pt.y += 21;
    text.Blit(dst_pt);
}

void RedrawFromResource(const Point & pt, const Resource::funds_t & rs)
{
    const ICN::icn_t tradpost = Settings::Get().EvilInterface() ? ICN::TRADPOSE : ICN::TRADPOST;

    // wood
    RedrawResourceSprite(AGG::GetICN(tradpost, 7), 0, 0, rs.wood);
    // mercury
    RedrawResourceSprite(AGG::GetICN(tradpost, 8), 37, 0, rs.mercury);
    // ore
    RedrawResourceSprite(AGG::GetICN(tradpost, 9), 74, 0, rs.ore);
    // sulfur
    RedrawResourceSprite(AGG::GetICN(tradpost, 10), 0, 37, rs.sulfur);
    // crystal
    RedrawResourceSprite(AGG::GetICN(tradpost, 11), 37, 37, rs.crystal);
    // gems
    RedrawResourceSprite(AGG::GetICN(tradpost, 12), 74, 37, rs.gems);
    // gold
    RedrawResourceSprite(AGG::GetICN(tradpost, 13), 37, 74, rs.gold);
}

void RedrawToResource(const Point & pt, bool showcost, bool tradingPost, u8 from_resource)
{
    Display & display = Display::Get();
    const ICN::icn_t tradpost = Settings::Get().EvilInterface() ? ICN::TRADPOSE : ICN::TRADPOST;
    std::string str;
    Point dst_pt;
    Text text;
    text.Set(Font::SMALL);

    // wood
    dst_pt.x = pt.x;
    dst_pt.y = pt.y;
    display.Blit(AGG::GetICN(tradpost, 7), dst_pt);
    if(showcost)
    {
	GetStringTradeCosts(str, from_resource, Resource::WOOD, tradingPost);
	text.Set(str);
	dst_pt.x += (34 - text.w()) / 2;
	dst_pt.y += 21;
	text.Blit(dst_pt);
    }
    
    // mercury
    dst_pt.x = pt.x + 37;
    dst_pt.y = pt.y;
    display.Blit(AGG::GetICN(tradpost, 8), dst_pt);
    if(showcost)
    {
	GetStringTradeCosts(str, from_resource, Resource::MERCURY, tradingPost);
	text.Set(str);
	dst_pt.x += (34 - text.w()) / 2;
	dst_pt.y += 21;
	text.Blit(dst_pt);
    }

    // ore
    dst_pt.x = pt.x + 74;
    dst_pt.y = pt.y;
    display.Blit(AGG::GetICN(tradpost, 9), dst_pt);
    if(showcost)
    {
	GetStringTradeCosts(str, from_resource, Resource::ORE, tradingPost);
	text.Set(str);
	dst_pt.x += (34 - text.w()) / 2;
	dst_pt.y += 21;
	text.Blit(dst_pt);
    }

    // sulfur
    dst_pt.x = pt.x;
    dst_pt.y = pt.y + 37;
    display.Blit(AGG::GetICN(tradpost, 10), dst_pt);
    if(showcost)
    {
	GetStringTradeCosts(str, from_resource, Resource::SULFUR, tradingPost);
	text.Set(str);
	dst_pt.x += (34 - text.w()) / 2;
	dst_pt.y += 21;
	text.Blit(dst_pt);
    }

    // crystal
    dst_pt.x = pt.x + 37;
    dst_pt.y = pt.y + 37;
    display.Blit(AGG::GetICN(tradpost, 11), dst_pt);
    if(showcost)
    {
	GetStringTradeCosts(str, from_resource, Resource::CRYSTAL, tradingPost);
	text.Set(str);
	dst_pt.x += (34 - text.w()) / 2;
	dst_pt.y += 21;
	text.Blit(dst_pt);
    }

    // gems
    dst_pt.x = pt.x + 74;
    dst_pt.y = pt.y + 37;
    display.Blit(AGG::GetICN(tradpost, 12), dst_pt);
    if(showcost)
    {
	GetStringTradeCosts(str, from_resource, Resource::GEMS, tradingPost);
	text.Set(str);
	dst_pt.x += (34 - text.w()) / 2;
	dst_pt.y += 21;
	text.Blit(dst_pt);
    }

    // gold
    dst_pt.x = pt.x + 37;
    dst_pt.y = pt.y + 74;
    display.Blit(AGG::GetICN(tradpost, 13), dst_pt);
    if(showcost)
    {
	GetStringTradeCosts(str, from_resource, Resource::GOLD, tradingPost);
	text.Set(str);
	dst_pt.x += (34 - text.w()) / 2;
	dst_pt.y += 21;
	text.Blit(dst_pt);
    }
}

void GetStringTradeCosts(std::string & str, u8 rs_from, u8 rs_to, bool tradingPost)
{
    if(str.size()) str.clear();

    if(rs_from == rs_to)
    {
	str = _("n/a");
	
	return;
    }

    if(Resource::GOLD != rs_from && Resource::GOLD != rs_to) str = "1/";

    String::AddInt(str, GetTradeCosts(rs_from, rs_to, tradingPost));
}

u16 GetTradeCosts(u8 rs_from, u8 rs_to, bool tradingPost)
{
    const u8 markets = tradingPost ? 3 : world.GetMyKingdom().GetCountMarketplace();
    
    if(rs_from == rs_to) return 0;

    switch(rs_from)
    {
	// uncostly
	case Resource::WOOD:
	case Resource::ORE:

    	    switch(rs_to)
    	    {
    		// sale uncostly
    		case Resource::GOLD:
    		    if(1 == markets) return SALE_UNCOSTLY1;
        	    else
        	    if(2 == markets) return SALE_UNCOSTLY2;
        	    else
        	    if(3 == markets) return SALE_UNCOSTLY3;
        	    else
        	    if(4 == markets) return SALE_UNCOSTLY4;
        	    else
        	    if(5 == markets) return SALE_UNCOSTLY5;
        	    else
        	    if(6 == markets) return SALE_UNCOSTLY6;
        	    else
        	    if(7 == markets) return SALE_UNCOSTLY7;
        	    else
        	    if(8 == markets) return SALE_UNCOSTLY8;
        	    else
        	    if(8 <  markets) return SALE_UNCOSTLY9;
    		    break;

		// change uncostly to costly
		case Resource::MERCURY:
		case Resource::SULFUR:
		case Resource::CRYSTAL:
		case Resource::GEMS:
    		    if(1 == markets) return UNCOSTLY_COSTLY1;
        	    else
        	    if(2 == markets) return UNCOSTLY_COSTLY2;
        	    else
        	    if(3 == markets) return UNCOSTLY_COSTLY3;
        	    else
        	    if(4 == markets) return UNCOSTLY_COSTLY4;
        	    else
        	    if(5 == markets) return UNCOSTLY_COSTLY5;
        	    else
        	    if(6 == markets) return UNCOSTLY_COSTLY6;
        	    else
        	    if(7 == markets) return UNCOSTLY_COSTLY7;
        	    else
        	    if(8 == markets) return UNCOSTLY_COSTLY8;
        	    else
        	    if(8 <  markets) return UNCOSTLY_COSTLY9;
    		    break;

		// change uncostly to uncostly
		case Resource::WOOD:
		case Resource::ORE:
    		    if(1 == markets) return COSTLY_COSTLY1;
        	    else
        	    if(2 == markets) return COSTLY_COSTLY2;
        	    else
        	    if(3 == markets) return COSTLY_COSTLY3;
        	    else
        	    if(4 == markets) return COSTLY_COSTLY4;
        	    else
        	    if(5 == markets) return COSTLY_COSTLY5;
        	    else
        	    if(6 == markets) return COSTLY_COSTLY6;
        	    else
        	    if(7 == markets) return COSTLY_COSTLY7;
        	    else
        	    if(8 == markets) return COSTLY_COSTLY8;
        	    else
        	    if(8 <  markets) return COSTLY_COSTLY9;
        	    break;
    	    }
	    break;

	// costly
	case Resource::MERCURY:
	case Resource::SULFUR:
	case Resource::CRYSTAL:
	case Resource::GEMS:
    	    
    	    switch(rs_to)
    	    {
    		// sale costly
    		case Resource::GOLD:
    		    if(1 == markets) return SALE_COSTLY1;
        	    else
        	    if(2 == markets) return SALE_COSTLY2;
        	    else
        	    if(3 == markets) return SALE_COSTLY3;
        	    else
        	    if(4 == markets) return SALE_COSTLY4;
        	    else
        	    if(5 == markets) return SALE_COSTLY5;
        	    else
        	    if(6 == markets) return SALE_COSTLY6;
        	    else
        	    if(7 == markets) return SALE_COSTLY7;
        	    else
        	    if(8 == markets) return SALE_COSTLY8;
        	    else
        	    if(8 <  markets) return SALE_COSTLY9;
        	    break;
        	
		// change costly to costly
		case Resource::MERCURY:
		case Resource::SULFUR:
		case Resource::CRYSTAL:
		case Resource::GEMS:
    		    if(1 == markets) return COSTLY_COSTLY1;
        	    else
        	    if(2 == markets) return COSTLY_COSTLY2;
        	    else
        	    if(3 == markets) return COSTLY_COSTLY3;
        	    else
        	    if(4 == markets) return COSTLY_COSTLY4;
        	    else
        	    if(5 == markets) return COSTLY_COSTLY5;
        	    else
        	    if(6 == markets) return COSTLY_COSTLY6;
        	    else
        	    if(7 == markets) return COSTLY_COSTLY7;
        	    else
        	    if(8 == markets) return COSTLY_COSTLY8;
        	    else
        	    if(8 <  markets) return COSTLY_COSTLY9;
        	    break;

		// change costly to uncostly
		case Resource::WOOD:
		case Resource::ORE:
    		    if(1 == markets) return COSTLY_UNCOSTLY1;
        	    else
        	    if(2 == markets) return COSTLY_UNCOSTLY2;
        	    else
        	    if(3 == markets) return COSTLY_UNCOSTLY3;
        	    else
        	    if(4 == markets) return COSTLY_UNCOSTLY4;
        	    else
        	    if(5 == markets) return COSTLY_UNCOSTLY5;
        	    else
        	    if(6 == markets) return COSTLY_UNCOSTLY6;
        	    else
        	    if(7 == markets) return COSTLY_UNCOSTLY7;
        	    else
        	    if(8 == markets) return COSTLY_UNCOSTLY8;
        	    else
        	    if(8 <  markets) return COSTLY_UNCOSTLY9;
        	    break;
    	    }
	    break;

	// gold
	case Resource::GOLD:
    	    
    	    switch(rs_to)
    	    {
    		default: break;
        	
		// buy costly
		case Resource::MERCURY:
		case Resource::SULFUR:
		case Resource::CRYSTAL:
		case Resource::GEMS:
    		    if(1 == markets) return BUY_COSTLY1;
        	    else
        	    if(2 == markets) return BUY_COSTLY2;
        	    else
        	    if(3 == markets) return BUY_COSTLY3;
        	    else
        	    if(4 == markets) return BUY_COSTLY4;
        	    else
        	    if(5 == markets) return BUY_COSTLY5;
        	    else
        	    if(6 == markets) return BUY_COSTLY6;
        	    else
        	    if(7 == markets) return BUY_COSTLY7;
        	    else
        	    if(8 == markets) return BUY_COSTLY8;
        	    else
        	    if(8 <  markets) return BUY_COSTLY9;
        	    break;

		// buy uncostly
		case Resource::WOOD:
		case Resource::ORE:
    		    if(1 == markets) return BUY_UNCOSTLY1;
        	    else
        	    if(2 == markets) return BUY_UNCOSTLY2;
        	    else
        	    if(3 == markets) return BUY_UNCOSTLY3;
        	    else
        	    if(4 == markets) return BUY_UNCOSTLY4;
        	    else
        	    if(5 == markets) return BUY_UNCOSTLY5;
        	    else
        	    if(6 == markets) return BUY_UNCOSTLY6;
        	    else
        	    if(7 == markets) return BUY_UNCOSTLY7;
        	    else
        	    if(8 == markets) return BUY_UNCOSTLY8;
        	    else
        	    if(8 <  markets) return BUY_UNCOSTLY9;
        	    break;
    	    }
	    break;

	// not select
	default:  break;
    }

    return 0;
}

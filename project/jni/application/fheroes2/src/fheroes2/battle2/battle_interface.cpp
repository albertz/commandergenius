/***************************************************************************
 *   Copyright (C) 2010 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#include <cmath>
#include <algorithm>
#include "engine.h"
#include "cursor.h"
#include "agg.h"
#include "settings.h"
#include "kingdom.h"
#include "world.h"
#include "castle.h"
#include "settings.h"
#include "pocketpc.h"
#include "battle_arena.h"
#include "battle_cell.h"
#include "battle_stats.h"
#include "battle_tower.h"
#include "battle_bridge.h"
#include "battle_catapult.h"
#include "battle_interface.h"

namespace Battle2
{
    u8  	GetIndexIndicator(const Stats &);
    direction_t	GetCellDirection(const Rect &, const Point &);
    u16		GetSwordCursorDirection(direction_t);
    direction_t GetDirectionFromCursorSword(u16);
    u16         GetCursorFromSpell(u8);

    bool ArmyNextIdleAnimation(Army::army_t &);
    bool ArmySetIdleAnimation(Army::army_t &);

    struct CursorPosition
    {
	CursorPosition() : index(-1) {};

	Point coord;
	s16 index;
    };
}

void DrawHexagon(Surface & sf, u8 index_color)
{
    u8 r, l, w, h;

    if(Settings::Get().QVGA())
    {
	r = 11;
	l = 7;
	w = 23;
	h = 26;

	sf.Set(w, h);
	sf.SetColorKey();

	u32 color = sf.GetColor(index_color);

	sf.DrawLine(r, 0, 0, l, color);
	sf.DrawLine(r, 0, w - 1, l, color);

	sf.DrawLine(0, l + 1, 0, h - l - 1, color);
	sf.DrawLine(w - 1, l + 1, w - 1, h - l - 1, color);

	sf.DrawLine(r, h - 1, 0, h - l - 1, color);
	sf.DrawLine(r, h - 1, w - 1, h - l - 1, color);
    }
    else
    {
	r = 22;
	l = 13;
	w = 45;
	h = 52;

	sf.Set(w, h);
	sf.SetColorKey();

	u32 color = sf.GetColor(index_color);

	sf.DrawLine(r, 0, 0, l, color);
	sf.DrawLine(r, 0, w - 1, l, color);

	sf.DrawLine(0, l + 1, 0, h - l - 1, color);
	sf.DrawLine(w - 1, l + 1, w - 1, h - l - 1, color);

	sf.DrawLine(r, h - 1, 0, h - l - 1, color);
	sf.DrawLine(r, h - 1, w - 1, h - l - 1, color);
    }

}

void DrawHexagonShadow(Surface & sf)
{
    u8 l, w, h;

    if(Settings::Get().QVGA())
    {
	//r = 11;
	l = 7;
	w = 23;
	h = 26;
    }
    else
    {
	//r = 22;
	l = 13;
	w = 45;
	h = 52;
    }

    sf.Set(w, h);
    sf.SetColorKey();

    Rect rt(0, l, w, 2 * l);
    sf.FillRect(0x00, 0x00, 0x00, rt);
    for(u8 i = 1; i < w / 2; i += 2)
    {
	--rt.y;
	rt.h += 2;
	rt.x += 2;
	rt.w -= 4;
	sf.FillRect(0x00, 0x00, 0x00, rt);
    }
    sf.SetAlpha(0x30);
}

void GetLinePoints(const Point & pt1, const Point & pt2, u16 step, std::vector<Point> & res)
{
    const u16 dx = std::abs(pt2.x - pt1.x);
    const u16 dy = std::abs(pt2.y - pt1.y);

    s16 ns = std::div((dx > dy ? dx : dy), 2).quot;
    Point pt(pt1);

    for(u16 i = 0; i <= (dx > dy ? dx : dy); ++i)
    {
	if(dx > dy)
	{
    	    pt.x < pt2.x ? ++pt.x : --pt.x;
    	    ns -= dy;
	}
	else
	{
            pt.y < pt2.y ? ++pt.y : --pt.y;
            ns -= dx;
	}

        if(ns < 0)
        {
	    if(dx > dy)
	    {
                pt.y < pt2.y ? ++pt.y : --pt.y;
                ns += dx;
	    }
	    else
	    {
                pt.x < pt2.x ? ++pt.x : --pt.x;
                ns += dy;
	    }
    	}

	if(0 == (i % step)) res.push_back(pt);
    }
}

void GetArcPoints(const Point & from, const Point & to, const Point & max, u16 step, std::vector<Point> & res)
{
    Point pt1, pt2;

    pt1 = from;
    pt2 = Point(from.x + std::abs(max.x - from.x) / 2, from.y - std::abs(max.y - from.y) * 3 / 4);
    GetLinePoints(pt1, pt2, step, res);

    pt1 = pt2;
    pt2 = max;
    GetLinePoints(pt1, pt2, step, res);

    pt1 = max;
    pt2 = Point(max.x + std::abs(to.x - max.x)  / 2, to.y - std::abs(to.y - max.y) * 3 / 4);
    GetLinePoints(pt1, pt2, step, res);

    pt1 = pt2;
    pt2 = to;    
    GetLinePoints(pt1, pt2, step, res);
}

bool Battle2::TargetInfo::isFinishAnimFrame(void) const
{
    return defender && defender->isFinishAnimFrame();
}

u16 Battle2::GetCursorFromSpell(u8 spell)
{
    switch(spell)
    {
	case Spell::SLOW:	return Cursor::SP_SLOW;
	case Spell::CURSE:	return Cursor::SP_CURSE;
	case Spell::CURE:	return Cursor::SP_CURE;
	case Spell::BLESS:	return Cursor::SP_BLESS;
	case Spell::FIREBALL:	return Cursor::SP_FIREBALL;
	case Spell::FIREBLAST:	return Cursor::SP_FIREBLAST;
	case Spell::TELEPORT:	return Cursor::SP_TELEPORT;
	case Spell::RESURRECT:	return Cursor::SP_RESURRECT;
	case Spell::HASTE:	return Cursor::SP_HASTE;
	case Spell::SHIELD:	return Cursor::SP_SHIELD;
	case Spell::ARMAGEDDON:	return Cursor::SP_ARMAGEDDON;
	case Spell::ANTIMAGIC:	return Cursor::SP_ANTIMAGIC;
	case Spell::BERSERKER:	return Cursor::SP_BERSERKER;
	case Spell::PARALYZE:	return Cursor::SP_PARALYZE;
	case Spell::BLIND:	return Cursor::SP_BLIND;

	case Spell::LIGHTNINGBOLT:	return Cursor::SP_LIGHTNINGBOLT;
	case Spell::CHAINLIGHTNING:	return Cursor::SP_CHAINLIGHTNING;
	case Spell::ELEMENTALSTORM:	return Cursor::SP_ELEMENTALSTORM;
	case Spell::RESURRECTTRUE:	return Cursor::SP_RESURRECTTRUE;
	case Spell::DISPEL:		return Cursor::SP_DISPEL;
	case Spell::HOLYWORD:		return Cursor::SP_HOLYWORD;
	case Spell::HOLYSHOUT:		return Cursor::SP_HOLYSHOUT;
	case Spell::METEORSHOWER:	return Cursor::SP_METEORSHOWER;

	case Spell::ANIMATEDEAD:	return Cursor::SP_ANIMATEDEAD;
	case Spell::MIRRORIMAGE:	return Cursor::SP_MIRRORIMAGE;
	case Spell::BLOODLUST:		return Cursor::SP_BLOODLUST;
	case Spell::DEATHRIPPLE:	return Cursor::SP_DEATHRIPPLE;
	case Spell::DEATHWAVE:		return Cursor::SP_DEATHWAVE;
	case Spell::STEELSKIN:		return Cursor::SP_STEELSKIN;
	case Spell::STONESKIN:		return Cursor::SP_STONESKIN;
	case Spell::DRAGONSLAYER:	return Cursor::SP_DRAGONSLAYER;
	case Spell::EARTHQUAKE:		return Cursor::SP_EARTHQUAKE;
	case Spell::DISRUPTINGRAY:	return Cursor::SP_DISRUPTINGRAY;
	case Spell::COLDRING:		return Cursor::SP_COLDRING;
	case Spell::COLDRAY:		return Cursor::SP_COLDRAY;
	case Spell::HYPNOTIZE:		return Cursor::SP_HYPNOTIZE;
	case Spell::ARROW:		return Cursor::SP_ARROW;

	default: break;
    }
    return Cursor::WAR_NONE;
}

u16 Battle2::GetSwordCursorDirection(direction_t dir)
{
    switch(dir)
    {    
	case BOTTOM_RIGHT:	return Cursor::SWORD_TOPLEFT;
    	case BOTTOM_LEFT:	return Cursor::SWORD_TOPRIGHT;
	case RIGHT:		return Cursor::SWORD_LEFT;
	case TOP_RIGHT:		return Cursor::SWORD_BOTTOMLEFT;
    	case TOP_LEFT:		return Cursor::SWORD_BOTTOMRIGHT;
    	case LEFT:		return Cursor::SWORD_RIGHT;
	default: break;
    }
    return 0;
}

Battle2::direction_t Battle2::GetDirectionFromCursorSword(u16 sword)
{
    switch(sword)
    {    
	case Cursor::SWORD_TOPLEFT:	return BOTTOM_RIGHT;
    	case Cursor::SWORD_TOPRIGHT:	return BOTTOM_LEFT;
	case Cursor::SWORD_LEFT:	return RIGHT;
	case Cursor::SWORD_BOTTOMLEFT:	return TOP_RIGHT;
    	case Cursor::SWORD_BOTTOMRIGHT:	return TOP_LEFT;
    	case Cursor::SWORD_RIGHT:	return LEFT;
	default: break;
    }

    return UNKNOWN;
}

Battle2::direction_t Battle2::GetCellDirection(const Rect & rt, const Point & pt)
{
    const u16 cw = rt.w / 2;
    const u16 ch = (rt.h + (Settings::Get().QVGA() ? 3 : 6)) / 3;

    if(Rect(rt.x, rt.y, cw, ch) & pt)
	return TOP_LEFT;
    else
    if(Rect(rt.x, rt.y + ch, cw, ch) & pt)
	return LEFT;
    else
    if(Rect(rt.x, rt.y + 2 * ch, cw, ch) & pt)
	return BOTTOM_LEFT;
    else
    if(Rect(rt.x + cw, rt.y, cw, ch) & pt)
	return TOP_RIGHT;
    else
    if(Rect(rt.x + cw, rt.y + ch, cw, ch) & pt)
	return RIGHT;
    else
    if(Rect(rt.x + cw, rt.y + 2 * ch, cw, ch) & pt)
	return BOTTOM_RIGHT;

    return UNKNOWN;
}

Battle2::OpponentSprite::OpponentSprite(const Rect & area, const HeroBase *b, bool r) : base(b),
    icn(ICN::UNKNOWN), animframe(0), animframe_start(0), animframe_count(0), reflect(r)
{
    ResetAnimFrame(OP_IDLE);

    if(Settings::Get().QVGA())
    {
	if(reflect)
	{
	    pos.x = area.x + area.w - 40;
	    pos.y = area.y + 50;
	}
	else
	{
	    pos.x = area.x + 5;
	    pos.y = area.y + 50;
	}

	const Sprite & sprite = AGG::GetICN(icn, animframe, reflect);

	pos.w = sprite.w();
	pos.h = sprite.h();
    }
    else
    {

	if(reflect)
	{
	    pos.x = area.x + area.w - 60;
	    pos.y = area.y + 50;
	}
	else
	{
	    pos.x = area.x + 5;
	    pos.y = area.y + 50;
	}

	const Sprite & sprite = AGG::GetICN(icn, animframe, reflect);

	pos.w = sprite.w();
	pos.h = sprite.h();
    }
}

u8 Battle2::OpponentSprite::GetColor(void) const
{
    return base ? base->GetColor() : 0;
}

const HeroBase* Battle2::OpponentSprite::GetHero(void) const
{
    return base;
}

void Battle2::OpponentSprite::IncreaseAnimFrame(bool loop)
{
    if(animframe < animframe_start + animframe_count - 1)
	++animframe;
    else
    if(loop)
	animframe = animframe_start;
}

void Battle2::OpponentSprite::ResetAnimFrame(u8 rule)
{
    if(Settings::Get().QVGA())
    {
	switch(base->GetColor())
	{
	    case Color::BLUE:	animframe = 0; break;
	    case Color::GREEN:	animframe = 7; break;
	    case Color::RED:	animframe = 14; break;
	    case Color::YELLOW:	animframe = 21; break;
	    case Color::ORANGE:	animframe = 28; break;
	    case Color::PURPLE:	animframe = 35; break;
	    default: break;
	}

	switch(base->GetRace())
	{
	    case Race::KNGT:	animframe += 0; break;
	    case Race::BARB:	animframe += 1; break;
	    case Race::SORC:	animframe += 2; break;
	    case Race::WRLK:	animframe += 3; break;
	    case Race::WZRD:	animframe += 4; break;
	    case Race::NECR:	animframe += 5; break;
	    default: break;
	}

	icn = ICN::MINIHERO;
    }
    else
    {
	if(Skill::Primary::HEROES == base->GetType())
	{
    	    switch(base->GetRace())
	    {
        	case Race::BARB:
		    icn = ICN::CMBTHROB;
		    switch(rule)
		    {
			case OP_IDLE:	animframe_start = 12; animframe_count = 7; break;
			case OP_SRRW:	animframe_start = 1; animframe_count = 5; break;
			case OP_CAST:	animframe_start = 7; animframe_count = 5; break;
			default: break;
		    }
		    break;
        	case Race::KNGT:
		    icn = ICN::CMBTHROK;
		    switch(rule)
		    {
			case OP_IDLE:	animframe_start = 12; animframe_count = 8; break;
			case OP_SRRW:	animframe_start = 1; animframe_count = 5; break;
			case OP_CAST:	animframe_start = 7; animframe_count = 5; break;
			default: break;
		    }
		    break;
        	case Race::NECR:
		    icn = ICN::CMBTHRON;
		    switch(rule)
		    {
			case OP_IDLE:	animframe_start = 16; animframe_count = 4; break;
			case OP_SRRW:	animframe_start = 1; animframe_count = 5; break;
			case OP_CAST:	animframe_start = 8; animframe_count = 6; break;
			default: break;
		    }
		    break;
        	case Race::SORC:
		    icn = ICN::CMBTHROS;
		    switch(rule)
		    {
			case OP_IDLE:	animframe_start = 10; animframe_count = 6; break;
			case OP_SRRW:	animframe_start = 1; animframe_count = 5; break;
			case OP_CAST:	animframe_start = 7; animframe_count = 3; break;
			default: break;
		    }
		    break;
        	case Race::WRLK:
		    icn = ICN::CMBTHROW;
		    switch(rule)
		    {
			case OP_IDLE:	animframe_start = 13; animframe_count = 4; break;
			case OP_SRRW:	animframe_start = 1; animframe_count = 5; break;
			case OP_CAST:	animframe_start = 7; animframe_count = 6; break;
			default: break;
		    }
		    break;
        	case Race::WZRD:
		    icn = ICN::CMBTHROZ;
		    switch(rule)
		    {
			case OP_IDLE:	animframe_start = 16; animframe_count = 3; break;
			case OP_SRRW:	animframe_start = 1; animframe_count = 5; break;
			case OP_CAST:	animframe_start = 7; animframe_count = 9; break;
			default: break;
		    }
		    break;
        	default: break;
    	    }
	    animframe = animframe_start;
	}
	else
	if(Skill::Primary::CAPTAIN == base->GetType())
	{
	    icn = ICN::CMBTCAPB;
    	    switch(base->GetRace())
    	    {
        	case Race::BARB: icn = ICN::CMBTCAPB; break;
        	case Race::KNGT: icn = ICN::CMBTCAPK; break;
        	case Race::NECR: icn = ICN::CMBTCAPN; break;
        	case Race::SORC: icn = ICN::CMBTCAPS; break;
        	case Race::WRLK: icn = ICN::CMBTCAPW; break;
        	case Race::WZRD: icn = ICN::CMBTCAPZ; break;
        	default: break;
    	    }

	    switch(rule)
	    {
		case OP_IDLE:	animframe_start = 1; animframe_count = 1; break;
		case OP_SRRW:	animframe_start = 1; animframe_count = 1; break;
		case OP_CAST:	animframe_start = 3; animframe_count = 5; break;
		default: break;
	    }
	    animframe = animframe_start;
	}
    }
}

bool Battle2::OpponentSprite::isFinishFrame(void) const
{
    return !animframe_count || animframe >= animframe_start + animframe_count - 1;
}

bool Battle2::OpponentSprite::isStartFrame(void) const
{
    return animframe_count && animframe == animframe_start;
}

const Rect & Battle2::OpponentSprite::GetArea(void) const
{
    return pos;
}

void Battle2::OpponentSprite::Redraw(void) const
{
    Display::Get().Blit(AGG::GetICN(icn, animframe, reflect), pos.x, pos.y);
}

Battle2::Status::Status() : back1(AGG::GetICN(ICN::TEXTBAR, 8)), back2(AGG::GetICN(ICN::TEXTBAR, 9))
{
    bar1.Set(Settings::Get().QVGA() ? Font::SMALL : Font::BIG);
    bar2.Set(Settings::Get().QVGA() ? Font::SMALL : Font::BIG);
}

void Battle2::Status::SetPosition(s16 cx, s16 cy)
{
    pos.x = cx;
    pos.y = cy;
}

void Battle2::Status::SetMessage(const std::string & str, bool top)
{

    if(top)
	bar1.Set(str);
    else
    if(str != message)
    {
	bar2.Set(str);
	message = str;
    }
}

void Battle2::Status::Redraw(void)
{
    Display & display = Display::Get();

    display.Blit(back1, pos.x, pos.y);
    display.Blit(back2, pos.x, pos.y + back1.h());

    if(bar1.Size()) bar1.Blit(pos.x + (back1.w() - bar1.w()) / 2, pos.y + (Settings::Get().QVGA() ? -1 : 3));
    if(bar2.Size()) bar2.Blit(pos.x + (back2.w() - bar2.w()) / 2, pos.y + back1.h() + (Settings::Get().QVGA() ? -3 : 0));
}

const std::string & Battle2::Status::GetMessage(void) const
{
    return message;
}

Battle2::Interface::Interface(Arena & a, s32 center) : arena(a), icn_cbkg(ICN::UNKNOWN), icn_frng(ICN::UNKNOWN), humanturn_spell(Spell::NONE),
    humanturn_exit(true), humanturn_redraw(true), animation_frame(0), catapult_frame(0),
    b_current(NULL), b_move(NULL), b_fly(NULL), b_current_sprite(NULL), teleport_src(MAXU16)
{
    const Settings & conf = Settings::Get();
    bool pda = conf.QVGA();

    // border
    Display & display = Display::Get();
    const u16 arenaw = pda ? 320 : 640;
    const u16 arenah = pda ? (display.h() < 240 ? display.h() : 240) : 480;
    border.SetPosition((display.w() - arenaw) / 2 - BORDERWIDTH, (display.h() - arenah) / 2 - BORDERWIDTH, arenaw, arenah);

    // cover
    bool trees = Maps::ScanAroundObject(center, MP2::OBJ_TREES);
    const Heroes *hero = world.GetHeroes(center);
    bool grave = hero && MP2::OBJ_GRAVEYARD == hero->GetUnderObject();
    bool light = true;

    switch(world.GetTiles(center).GetGround())
    {
        case Maps::Ground::DESERT:      icn_cbkg = ICN::CBKGDSRT; light = false; icn_frng = ICN::FRNG0004; break;
	case Maps::Ground::SNOW:        icn_cbkg = trees ? ICN::CBKGSNTR : ICN::CBKGSNMT; light = false; icn_frng = trees ? ICN::FRNG0006 : ICN::FRNG0007; break;
        case Maps::Ground::SWAMP:       icn_cbkg = ICN::CBKGSWMP; icn_frng = ICN::FRNG0008; break;
        case Maps::Ground::WASTELAND:   icn_cbkg = ICN::CBKGCRCK; light = false; icn_frng = ICN::FRNG0003; break;
        case Maps::Ground::BEACH:       icn_cbkg = ICN::CBKGBEAC; light = false; icn_frng = ICN::FRNG0002; break;
        case Maps::Ground::LAVA:        icn_cbkg = ICN::CBKGLAVA; icn_frng = ICN::FRNG0005; break;
        case Maps::Ground::DIRT:        icn_cbkg = trees ? ICN::CBKGDITR : ICN::CBKGDIMT; icn_frng = trees ? ICN::FRNG0010 : ICN::FRNG0009; break;
        case Maps::Ground::GRASS:       icn_cbkg = trees ? ICN::CBKGGRTR : ICN::CBKGGRMT; icn_frng = trees ? ICN::FRNG0011 : ICN::FRNG0012; break;
        case Maps::Ground::WATER:       icn_cbkg = ICN::CBKGWATR; icn_frng = ICN::FRNG0013; break;
        default: break;
    }

    if(grave){ icn_cbkg = ICN::CBKGGRAV; light = true; icn_frng = ICN::FRNG0001; }
    if(conf.QVGA() || conf.ExtLowMemory()) icn_frng = ICN::UNKNOWN;

    // hexagon
    DrawHexagon(sf_hexagon, (light ? 0xE0 : 0xE5));
    DrawHexagon(sf_cursor, 0xD8);
    DrawHexagonShadow(sf_shadow);

    // buttons
    const Rect & area = border.GetArea();

    btn_auto.SetSprite(ICN::TEXTBAR, 4, 5);
    btn_settings.SetSprite(ICN::TEXTBAR, 6, 7);
    btn_skip.SetSprite(ICN::TEXTBAR, 0, 1);

    btn_auto.SetPos(area.x, area.y + area.h - btn_settings.h - btn_auto.h);
    btn_settings.SetPos(area.x, area.y + area.h - btn_settings.h);
    btn_skip.SetPos(area.x + area.w - btn_skip.w, area.y + area.h - btn_skip.h);

    status.SetPosition(area.x + btn_auto.w, btn_auto.y);

    // opponents
    opponent1 = arena.army1.GetCommander() ? new OpponentSprite(area, arena.army1.GetCommander(), false) : NULL;
    opponent2 = arena.army2.GetCommander() ? new OpponentSprite(area, arena.army2.GetCommander(), true) : NULL;
}

Battle2::Interface::~Interface()
{
    if(opponent1) delete opponent1;
    if(opponent2) delete opponent2;
    if(Settings::Get().AutoBattle()) Settings::Get().SetAutoBattle(false);
}

const Rect & Battle2::Interface::GetArea(void) const
{
    return border.GetArea();
}

void Battle2::Interface::SetStatus(const std::string & msg, bool top)
{
    if(top)
    {
        status.SetMessage(msg, true);
        status.SetMessage("", false);
    }
    else
    {
	status.SetMessage(msg);
    }
    humanturn_redraw = true;
}

void Battle2::Interface::Redraw(void)
{
    RedrawCover();
    RedrawOpponents();
    if(arena.castle) RedrawCastle3();
    RedrawArmies();
    RedrawInterface();
    RedrawBorder();
    if(Settings::Get().QVGA()) RedrawPocketControls();
}

void Battle2::Interface::RedrawInterface(void)
{
    status.Redraw();

    btn_auto.Draw();
    btn_settings.Draw();
    btn_skip.Draw();
}

void Battle2::Interface::RedrawArmies(void) const
{
    for(u16 ii = 0; ii < arena.board.size(); ++ii)
    {
        if(arena.castle) RedrawCastle2(ii);
	RedrawObjects(ii);

	const Stats* b = arena.GetTroopBoard(ii);
	if(!b || (b->isWide() && ii == b->GetTailIndex())) continue;

	const Cell & cell = arena.board[ii];

	if(b_fly != b)
	{
	    RedrawTroopSprite(*b, cell.pos);

	    if(b_move != b)
		RedrawTroopCount(*b, cell.pos);
	}
    }

    if(b_fly)
    {
	RedrawTroopSprite(*b_fly, b_fly->GetCellPosition());
    }
}

void Battle2::Interface::RedrawOpponents(void) const
{
    if(opponent1) opponent1->Redraw();
    if(opponent2) opponent2->Redraw();

    RedrawOpponentsFlags();
}

void Battle2::Interface::RedrawOpponentsFlags(void) const
{
    if(!Settings::Get().QVGA() && opponent1)
    {
	ICN::icn_t icn = ICN::UNKNOWN;

	switch(arena.army1.GetColor())
	{
    	    case Color::BLUE:       icn = ICN::HEROFL00; break;
    	    case Color::GREEN:      icn = ICN::HEROFL01; break;
    	    case Color::RED:        icn = ICN::HEROFL02; break;
    	    case Color::YELLOW:     icn = ICN::HEROFL03; break;
    	    case Color::ORANGE:     icn = ICN::HEROFL04; break;
    	    case Color::PURPLE:     icn = ICN::HEROFL05; break;
    	    default:                icn = ICN::HEROFL06; break;
	}

	const Sprite & flag = AGG::GetICN(icn, ICN::AnimationFrame(icn, 0, animation_frame), false);
	Display::Get().Blit(flag, opponent1->GetArea().x + 38 - flag.w(), opponent1->GetArea().y + 5);
    }

    if(!Settings::Get().QVGA() && opponent2)
    {
	ICN::icn_t icn = ICN::UNKNOWN;

	switch(arena.army2.GetColor())
	{
    	    case Color::BLUE:       icn = ICN::HEROFL00; break;
    	    case Color::GREEN:      icn = ICN::HEROFL01; break;
    	    case Color::RED:        icn = ICN::HEROFL02; break;
    	    case Color::YELLOW:     icn = ICN::HEROFL03; break;
    	    case Color::ORANGE:     icn = ICN::HEROFL04; break;
    	    case Color::PURPLE:     icn = ICN::HEROFL05; break;
    	    default:                icn = ICN::HEROFL06; break;
	}

	const Sprite & flag = AGG::GetICN(icn, ICN::AnimationFrame(icn, 0, animation_frame), true);
	const u8 ox = Skill::Primary::HEROES == opponent2->GetHero()->GetType() ? 38 : 26;
	Display::Get().Blit(flag, opponent2->GetArea().x + ox - flag.w(), opponent2->GetArea().y + 5);
    }
}

void Battle2::Interface::RedrawTroopSprite(const Stats & b, const Rect & rt) const
{
    Display & display = Display::Get();
    const  MonsterInfo & msi = b.GetMonsterInfo();
    const Sprite* spmon1 = NULL;
    const Sprite* spmon2 = NULL;

    // redraw current
    if(b_current == &b)
    {
	spmon1 = &AGG::GetICN(msi.icn_file, msi.frm_idle.start, b.reflect);
	spmon2 = (b.reflect ? b.contours[1] : b.contours[0]);

	if(b_current_sprite)
	{
	    spmon1 = b_current_sprite;
	    spmon2 = NULL;
	}
    }
    else
    if(b.Modes(SP_STONE))
    {
	// black wite sprite
	spmon1 = (b.reflect ? b.contours[3] : b.contours[2]);
    }
    else
    {
	spmon1 = &AGG::GetICN(msi.icn_file, b.animframe, b.reflect);
    }

    if(spmon1)
    {
	// offset
	s16 sx = b.isReflect() ? 
		rt.x + (b.isWide() ? rt.w + rt.w / 2 : rt.w / 2) - spmon1->w() - spmon1->x() :
		rt.x + (b.isWide() ? -rt.w / 2 : rt.w / 2) + spmon1->x();
	s16 sy = rt.y + rt.h + spmon1->y() - 10;

	// move offset
	if(b_move == &b)
	{
	    const animframe_t & frm = b_move->GetFrameState();
	    const Sprite* spmon0 = &AGG::GetICN(msi.icn_file, frm.start, b.reflect);
	    const s16 ox = spmon0 ? spmon1->x() - spmon0->x() : 0;
	    const Rect & pos = b_move->GetCellPosition();

	    if(frm.count)
	    {
		const s16 cx = p_move.x - pos.x;
		const s16 cy = p_move.y - pos.y;

	        sy += ((b_move->animframe - frm.start) * cy) / frm.count;
		if(0 != Sign(cy)) sx -= Sign(cx) * ox / 2;
	    }
	}
	else
	// fly offset
	if(b_fly == &b)
	{
	    const Point & pos = b_fly->GetCellPosition();

	    if(b_fly->GetFrameCount())
	    {
		const s16 cx = p_fly.x - pos.x;
		const s16 cy = p_fly.y - pos.y;

		sx += cx + Sign(cx) * b_fly->GetFrameOffset() * std::abs((p_fly.x - p_move.x) / b_fly->GetFrameCount());
		sy += cy + Sign(cy) * b_fly->GetFrameOffset() * std::abs((p_fly.y - p_move.y) / b_fly->GetFrameCount());
	    }
	}

	// sprite monster
	display.Blit(*spmon1, sx, sy);
	// contour
	if(spmon2) display.Blit(*spmon2, sx - 1, sy - 1);
    }
}

void Battle2::Interface::RedrawTroopCount(const Stats & b, const Rect & rt) const
{
    Display & display = Display::Get();
    const Sprite & bar = AGG::GetICN(ICN::TEXTBAR, GetIndexIndicator(b));

    s16 sx = 0;
    s16 sy = 0;

    if(Settings::Get().QVGA())
    {
	sy = rt.y + rt.h - bar.h();

	if(b.isWide())
	    sx = rt.x - (b.reflect ? 0 : rt.w) + (2 * rt.w - bar.w()) / 2;
	else
	    sx = rt.x + (rt.w - bar.w()) / 2;
    }
    else
    {
	sy = rt.y + rt.h - bar.h() - 5;

	if(b.reflect)
	    sx = rt.x + 3;
	else
	    sx = rt.x + rt.w - bar.w() - 3;
    }

    std::string str;

    display.Blit(bar, sx, sy);

    if(b.count < 1000)
	String::AddInt(str, b.count);
    else
    if(b.count < 1000000)
    {
	String::AddInt(str, b.count / 1000);
    	str += "K";
    }
    else
    {
	String::AddInt(str, b.count / 1000000);
	str += "M";
    }

    Text text(str, Font::SMALL);
    text.Blit(sx + (bar.w() - text.w()) / 2, sy);
}

void Battle2::Interface::RedrawCover(void)
{
    const Settings & conf = Settings::Get();
    Display & display = Display::Get();
    const Point & topleft = border.GetArea();
    const s16 cursor = arena.board.GetIndexAbsPosition(LocalEvent::Get().GetMouseCursor());

    if(icn_cbkg != ICN::UNKNOWN)
    {
	const Sprite & cbkg = AGG::GetICN(icn_cbkg, 0);
	display.Blit(cbkg, topleft);
    }

    if(icn_frng != ICN::UNKNOWN)
    {
	const Sprite & frng = AGG::GetICN(icn_frng, 0);
	display.Blit(frng, topleft.x + frng.x(), topleft.x + frng.y());
    }

    if(arena.icn_covr != ICN::UNKNOWN)
    {
	const Sprite & cover = AGG::GetICN(arena.icn_covr, 0);
	display.Blit(cover, topleft.x + cover.x(), topleft.y + cover.y());
    }

    if(arena.castle) RedrawCastle1();

    // shadow
    if(!b_move && conf.ExtBattleShowMoveShadow())
    {
	Board::const_iterator it1 = arena.board.begin();
	Board::const_iterator it2 = arena.board.end();

	for(; it1 != it2; ++it1) if((*it1).isPassable() && UNKNOWN != (*it1).GetDirection())
	    display.Blit(sf_shadow, (*it1).GetPos().x, (*it1).GetPos().y);
    }

    // grid
    if(conf.ExtBattleShowGrid())
    {
	Board::const_iterator it1 = arena.board.begin();
	Board::const_iterator it2 = arena.board.end();

	for(; it1 != it2; ++it1)
	    if((*it1).object == 0) display.Blit(sf_hexagon, (*it1).GetPos());
    }

    // cursor
    if(conf.ExtBattleShowMouseShadow() && 0 <= cursor && b_current && Cursor::Get().Themes() != Cursor::WAR_NONE)
	display.Blit(sf_cursor, arena.board[cursor].GetPos());

    RedrawKilled();
}

void Battle2::Interface::RedrawCastle1(void) const
{
    Display & display = Display::Get();
    const Point & topleft = border.GetArea();
    const bool fortification = arena.castle && (Race::KNGT == arena.castle->GetRace()) && arena.castle->isBuild(BUILD_SPEC);

    ICN::icn_t icn_castbkg = ICN::UNKNOWN;
    ICN::icn_t icn_castle  = ICN::UNKNOWN;

    switch(arena.castle->GetRace())
    {
        default:
        case Race::BARB: icn_castbkg = ICN::CASTBKGB; break;
        case Race::KNGT: icn_castbkg = ICN::CASTBKGK; break;
        case Race::NECR: icn_castbkg = ICN::CASTBKGN; break;
        case Race::SORC: icn_castbkg = ICN::CASTBKGS; break;
        case Race::WRLK: icn_castbkg = ICN::CASTBKGW; break;
        case Race::WZRD: icn_castbkg = ICN::CASTBKGZ; break;
    }

    switch(arena.castle->GetRace())
    {
        default:
        case Race::BARB: icn_castle = ICN::CASTLEB; break;
        case Race::KNGT: icn_castle = ICN::CASTLEK; break;
        case Race::NECR: icn_castle = ICN::CASTLEN; break;
        case Race::SORC: icn_castle = ICN::CASTLES; break;
        case Race::WRLK: icn_castle = ICN::CASTLEW; break;
        case Race::WZRD: icn_castle = ICN::CASTLEZ; break;
    }

    // castle cover
    const Sprite & sprite1 = AGG::GetICN(icn_castbkg, 1);
    display.Blit(sprite1, sprite1.x() + topleft.x, sprite1.y() + topleft.y);

    // moat
    if(arena.castle->isBuild(BUILD_MOAT))
    {
        const Sprite & sprite = AGG::GetICN(ICN::MOATWHOL, 0);
        display.Blit(sprite, sprite.x() + topleft.x, sprite.y() + topleft.y);
    }

    // top wall
    const Sprite & sprite2 = AGG::GetICN(icn_castbkg, fortification ? 4 : 3);
    display.Blit(sprite2, sprite2.x() + topleft.x, sprite2.y() + topleft.y);

    // bridge
    if(arena.bridge->isDown())
    {
    	const Sprite & sprite3 = AGG::GetICN(icn_castle, arena.bridge->isDestroy() ? 24 : 21);
    	display.Blit(sprite3, sprite3.x() + topleft.x, sprite3.y() + topleft.y);
    }
}

void Battle2::Interface::RedrawCastle2(const u16 cell_index) const
{
    const Settings & conf = Settings::Get();
    Display & display = Display::Get();
    const Point & topleft = border.GetArea();
    ICN::icn_t icn_castle  = ICN::UNKNOWN;

    switch(arena.castle->GetRace())
    {
        default:
        case Race::BARB: icn_castle = ICN::CASTLEB; break;
        case Race::KNGT: icn_castle = ICN::CASTLEK; break;
        case Race::NECR: icn_castle = ICN::CASTLEN; break;
        case Race::SORC: icn_castle = ICN::CASTLES; break;
        case Race::WRLK: icn_castle = ICN::CASTLEW; break;
        case Race::WZRD: icn_castle = ICN::CASTLEZ; break;
    }

    // catapult
    if(77 == cell_index)
    {
        const Sprite & sprite = AGG::GetICN(ICN::CATAPULT, catapult_frame);
	const Rect & pos = arena.board[cell_index].pos;
        display.Blit(sprite, sprite.x() + pos.x - pos.w, sprite.y() + pos.y + pos.h - 10);
    }
    else
    // castle gate
    if(50 == cell_index)
    {
        const Sprite & sprite = AGG::GetICN(icn_castle, 4);
        display.Blit(sprite, sprite.x() + topleft.x, sprite.y() + topleft.y);
    }
    else
    // castle wall
    if(8 == cell_index || 29 == cell_index || 73 == cell_index || 96 == cell_index)
    {
        u8 index = 0;
	const bool fortification = arena.castle && (Race::KNGT == arena.castle->GetRace()) && arena.castle->isBuild(BUILD_SPEC);

        switch(cell_index)
        {
            case 8:     index = 5; break;
            case 29:    index = 6; break;
            case 73:    index = 7; break;
            case 96:    index = 8; break;
            default: break;
        }

	if(fortification)
	{
	    switch(arena.board[cell_index].object)
	    {
		case 0:	index += 31; break;
		case 1:	index += 35; break;
		case 2:	index += 27; break;
		case 3:	index += 23; break;
		break;
	    }
	}
	else
	{
	    switch(arena.board[cell_index].object)
	    {
		case 0:	index += 8; break;
		case 1:	index += 4; break;
		case 2:	index += 0; break;
		break;
	    }
	}

        const Sprite & sprite = AGG::GetICN(icn_castle, index);
        display.Blit(sprite, sprite.x() + topleft.x, sprite.y() + topleft.y);
    }
    else
    // castle archer towers
    if(19 == cell_index)
    {
	u8 index = 17;

	if(arena.castle->isBuild(BUILD_LEFTTURRET))
	{
	    index = (arena.towers[0] && arena.towers[0]->isValid() ? 18 : 19);
    	}

        display.Blit(AGG::GetICN(icn_castle, index), topleft.x + (conf.QVGA() ? 207 : 415), topleft.y + (conf.QVGA() ? 20 : 40));
    }
    else
    if(85 == cell_index)
    {
	u8 index = 17;

	if(arena.castle->isBuild(BUILD_RIGHTTURRET))
	{
	    index = ( arena.towers[2] && arena.towers[2]->isValid() ? 18 : 19);
    	}

        display.Blit(AGG::GetICN(icn_castle, index), topleft.x + (conf.QVGA() ? 207 : 415), topleft.y + (conf.QVGA() ? 145 : 290));
    }
    else
    // castle towers
    if(40 == cell_index)
        display.Blit(AGG::GetICN(icn_castle, 17), topleft.x + (conf.QVGA() ? 187 : 375), topleft.y + (conf.QVGA() ? 60 : 120));
    else
    // castle towers
    if(62 == cell_index)
	display.Blit(AGG::GetICN(icn_castle, 17), topleft.x + (conf.QVGA() ? 187 : 375), topleft.y + (conf.QVGA() ? 102 : 205));
}

void Battle2::Interface::RedrawCastle3(void) const
{
    //const Settings & conf = Settings::Get();
    Display & display = Display::Get();
    const Point & topleft = border.GetArea();
    ICN::icn_t icn_castle  = ICN::UNKNOWN;

    switch(arena.castle->GetRace())
    {
        default:
        case Race::BARB: icn_castle = ICN::CASTLEB; break;
        case Race::KNGT: icn_castle = ICN::CASTLEK; break;
        case Race::NECR: icn_castle = ICN::CASTLEN; break;
        case Race::SORC: icn_castle = ICN::CASTLES; break;
        case Race::WRLK: icn_castle = ICN::CASTLEW; break;
        case Race::WZRD: icn_castle = ICN::CASTLEZ; break;
    }

    const Sprite & sprite = AGG::GetICN(icn_castle, (arena.towers[1]->isValid() ? 20 : 26));

    display.Blit(sprite, topleft.x + sprite.x() ,topleft.y + sprite.y());
}

void Battle2::Interface::RedrawObjects(const u16 cell_index) const
{
    const Sprite* sprite = NULL;
    switch(arena.board[cell_index].object)
    {
	case 0x80:	sprite = &AGG::GetICN(ICN::COBJ0000, 0); break;
	case 0x81:	sprite = &AGG::GetICN(ICN::COBJ0001, 0); break;
	case 0x82:	sprite = &AGG::GetICN(ICN::COBJ0002, 0); break;
	case 0x83:	sprite = &AGG::GetICN(ICN::COBJ0003, 0); break;
	case 0x84:	sprite = &AGG::GetICN(ICN::COBJ0004, 0); break;
	case 0x85:	sprite = &AGG::GetICN(ICN::COBJ0005, 0); break;
	case 0x86:	sprite = &AGG::GetICN(ICN::COBJ0006, 0); break;
	case 0x87:	sprite = &AGG::GetICN(ICN::COBJ0007, 0); break;
	case 0x88:	sprite = &AGG::GetICN(ICN::COBJ0008, 0); break;
	case 0x89:	sprite = &AGG::GetICN(ICN::COBJ0009, 0); break;
	case 0x8A:	sprite = &AGG::GetICN(ICN::COBJ0010, 0); break;
	case 0x8B:	sprite = &AGG::GetICN(ICN::COBJ0011, 0); break;
	case 0x8C:	sprite = &AGG::GetICN(ICN::COBJ0012, 0); break;
	case 0x8D:	sprite = &AGG::GetICN(ICN::COBJ0013, 0); break;
	case 0x8E:	sprite = &AGG::GetICN(ICN::COBJ0014, 0); break;
	case 0x8F:	sprite = &AGG::GetICN(ICN::COBJ0015, 0); break;
	case 0x90:	sprite = &AGG::GetICN(ICN::COBJ0016, 0); break;
	case 0x91:	sprite = &AGG::GetICN(ICN::COBJ0017, 0); break;
	case 0x92:	sprite = &AGG::GetICN(ICN::COBJ0018, 0); break;
	case 0x93:	sprite = &AGG::GetICN(ICN::COBJ0019, 0); break;
	case 0x94:	sprite = &AGG::GetICN(ICN::COBJ0020, 0); break;
	case 0x95:	sprite = &AGG::GetICN(ICN::COBJ0021, 0); break;
	case 0x96:	sprite = &AGG::GetICN(ICN::COBJ0022, 0); break;
	case 0x97:	sprite = &AGG::GetICN(ICN::COBJ0023, 0); break;
	case 0x98:	sprite = &AGG::GetICN(ICN::COBJ0024, 0); break;
	case 0x99:	sprite = &AGG::GetICN(ICN::COBJ0025, 0); break;
	case 0x9A:	sprite = &AGG::GetICN(ICN::COBJ0026, 0); break;
	case 0x9B:	sprite = &AGG::GetICN(ICN::COBJ0027, 0); break;
	case 0x9C:	sprite = &AGG::GetICN(ICN::COBJ0028, 0); break;
	case 0x9D:	sprite = &AGG::GetICN(ICN::COBJ0029, 0); break;
	case 0x9E:	sprite = &AGG::GetICN(ICN::COBJ0030, 0); break;
	case 0x9F:	sprite = &AGG::GetICN(ICN::COBJ0031, 0); break;
	default: break;
    }

    if(sprite)
    {
	Display & display = Display::Get();
	//const Point & topleft = border.GetArea();
	const Rect & pt = arena.board[cell_index].pos;
	display.Blit(*sprite, pt.x + pt.w / 2 + sprite->x(), pt.y + pt.h + sprite->y() - (Settings::Get().QVGA() ? 5 : 10));
    }
}

void Battle2::Interface::RedrawKilled(void)
{
    // redraw killed troop
    std::vector<u16> cells;
    arena.graveyard.GetClosedCells(cells);
    if(cells.size())
    {
	std::vector<u16>::const_iterator it;

	for(it = cells.begin(); it != cells.end(); ++it)
	{
    	    const u16 id_killed = arena.graveyard.GetLastTroopIDFromCell(*it);

	    if(id_killed)
	    {
		const Stats* b = arena.GetTroopID(id_killed);
		if(b)
		{
		    if(b->isWide() && *it == b->GetTailIndex()) continue;
		    RedrawTroopSprite(*b, arena.board[*it].pos);
		}
	    }
	}
    }
}

void Battle2::Interface::RedrawBorder(void)
{
    border.Redraw();
}

void Battle2::Interface::RedrawPocketControls(void) const
{
    const HeroBase* hero = b_current ? b_current->GetCommander() : NULL;
    if(hero && hero->HaveSpellBook() && !hero->Modes(Heroes::SPELLCASTED))
    {
        Display::Get().Blit(AGG::GetICN(ICN::ARTFX, 81), pocket_book);
    }
}

u16 Battle2::Interface::GetBattleCursor(const Point & mouse, std::string & status)
{
    status.clear();

    const s16 index = arena.board.GetIndexAbsPosition(mouse);

    if(0 <= index && b_current)
    {
	const Cell & cell = arena.board[index];
	const Stats* b_enemy = arena.GetTroopBoard(index);
	if(b_enemy)
	{
	    if(b_current->GetColor() == b_enemy->GetColor() && !b_enemy->Modes(SP_HYPNOTIZE))
	    {
		status = _("View %{monster} info.");
		String::Replace(status, "%{monster}", b_enemy->GetName());
		return Cursor::WAR_INFO;
	    }
	    else
	    {
		if(b_current->isArchers() && !b_current->isHandFighting())
		{
		    status = _("Shot %{monster} (%{count} shot(s) left)");
		    String::Replace(status, "%{monster}", b_enemy->GetName());
		    String::Replace(status, "%{count}", b_current->GetShots());
		    return b_enemy->GetObstaclesPenalty(*b_current) ? Cursor::WAR_BROKENARROW : Cursor::WAR_ARROW;
		}
		else
		{
		    const direction_t dir = GetCellDirection(cell.GetPos(), mouse);
		    const u16 cursor = GetSwordCursorDirection(dir);
		    if(cursor && Board::isValidDirection(index, dir))
		    {
			const u16 from = Board::GetIndexDirection(index, dir);

			// if free cell or it is b_current
			if(UNKNOWN != arena.board[from].GetDirection() ||
			    from == b_current->GetPosition() ||
			    (b_current->isWide() && from == b_current->GetTailIndex()))
			{
			    status = _("Attack %{monster}");
			    String::Replace(status, "%{monster}", b_enemy->GetName());
			    return cursor;
			}
		    }
		}
	    }
	}
	else
	if(cell.isPassable(*b_current, false) && UNKNOWN != cell.direction)
	{
	    status = b_current->isFly() ? _("Fly %{monster} here.") : _("Move %{monster} here.");
	    String::Replace(status, "%{monster}", b_current->GetName());
	    return b_current->isFly() ? Cursor::WAR_FLY : Cursor::WAR_MOVE;
	}
    }

    return Cursor::WAR_NONE;
}

u16 Battle2::Interface::GetBattleSpellCursor(const Point & mouse, std::string & status)
{
    status.clear();

    const s16 index = arena.board.GetIndexAbsPosition(mouse);
    const Spell::spell_t spell = Spell::FromInt(humanturn_spell);

    if(0 <= index && b_current && Spell::NONE != spell)
    {
	const Stats* b_stats = arena.GetTroopBoard(index);
	const Cell* cell = arena.GetCell(index);

	// over graveyard
	if(!b_stats && arena.isAllowResurrectFromGraveyard(spell, index))
	{
    	    const u16 id_killed = arena.graveyard.GetLastTroopIDFromCell(index);
    	    if(id_killed) b_stats = arena.GetTroopID(id_killed);
	}

	// teleport check first
	if(MAXU16 != teleport_src)
	{
	    if(!b_stats && cell && cell->isPassable(*b_current, false))
	    {
		status = _("Teleport Here");
		return Cursor::SP_TELEPORT;
	    }

	    status = _("Invalid Teleport Destination");
	    return Cursor::WAR_NONE;
	}
	else
	if(b_stats && b_stats->AllowApplySpell(spell, b_current->GetCommander()))
	{
	    status = _("Cast %{spell} on %{monster}");
	    String::Replace(status, "%{spell}", Spell::GetName(spell));
	    String::Replace(status, "%{monster}", b_stats->GetName());
	    return GetCursorFromSpell(spell);
    	}
	else
	if(!Spell::isApplyToFriends(spell) &&
	   !Spell::isApplyToEnemies(spell) &&
	   !Spell::isApplyToAnyTroops(spell))
	{
	    status = _("Cast %{spell}");
	    String::Replace(status, "%{spell}", Spell::GetName(spell));
	    return GetCursorFromSpell(spell);
	}
    }

    status = _("Select Spell Target");

    return Cursor::WAR_NONE;
}

void Battle2::Interface::HumanTurn(const Stats & b, Actions & a)
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();
    Settings & conf = Settings::Get();

    cursor.SetThemes(Cursor::WAR_NONE);
    b_current = &b;
    humanturn_redraw = false;
    humanturn_exit = false;
    catapult_frame = 0;

    arena.ResetBoard();
    arena.ScanPassabilityBoard(b);

    rectBoard = arena.board.GetArea();

    if(conf.Music() && !Music::isPlaying()) AGG::PlayMusic(MUS::GetBattleRandom(), false);

    if(conf.QVGA() && arena.current_commander && arena.current_commander->HaveSpellBook())
    {
	const Rect & area = border.GetArea();
    	const Sprite & book = AGG::GetICN(ICN::ARTFX, 81);
	const u16 ox = (arena.army1.GetColor() == arena.current_commander->GetColor() ? 0 : 320 - book.w());
	pocket_book = Rect(area.x + ox, area.y + area.h - 19 - book.h(), book.w(), book.h());
    }

    // safe position coord
    CursorPosition cursorPosition;

    cursor.Hide();
    Redraw();
    cursor.Show();
    display.Flip();

    std::string msg;
    animation_frame = 0;

    while(!humanturn_exit && le.HandleEvents())
    {
	if(humanturn_spell != Spell::NONE)
	    HumanCastSpellTurn(b, a, msg);
	else
	    HumanBattleTurn(b, a, msg);

	// update status
	if(msg != status.GetMessage())
	{
	    status.SetMessage(msg);
	    humanturn_redraw = true;
	}

	// animation troops
	if(IdleTroopsAnimation()) humanturn_redraw = true;

	CheckGlobalEvents(le);

	// redraw arena
	if(humanturn_redraw)
	{
	    cursor.Hide();
	    Redraw();
	    cursor.Show();
	    display.Flip();
	    humanturn_redraw = false;
	}
        else
        if(!cursor.isVisible())
        {
            cursor.Show();
            display.Flip();
        }
    }
    
    b_current = NULL;
}

void Battle2::Interface::HumanBattleTurn(const Stats & b, Actions & a, std::string & msg)
{
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();
    Settings & conf = Settings::Get();

    if(le.KeyPress())
    {
        // skip
	if(Game::HotKeyPress(Game::EVENT_BATTLE_HARDSKIP))
	{
	    a.AddedSkipAction(b, true);
	    humanturn_exit = true;
	}
	else
	// soft skip
	if(Game::HotKeyPress(Game::EVENT_BATTLE_SOFTSKIP))
	{
	    a.AddedSkipAction(b, !conf.ExtBattleSoftWait());
	    humanturn_exit = true;
	}
	else
	// options
	if(Game::HotKeyPress(Game::EVENT_BATTLE_OPTIONS))
	    EventShowOptions();
	else
	// auto switch
	if(Game::HotKeyPress(Game::EVENT_BATTLE_AUTOSWITCH))
	    EventAutoSwitch(b, a);
	else
	// cast
	if(Game::HotKeyPress(Game::EVENT_BATTLE_CASTSPELL))
	    ProcessingHeroDialogResult(1, a);
	else
	// retreat
	if(Game::HotKeyPress(Game::EVENT_BATTLE_RETREAT))
	    ProcessingHeroDialogResult(2, a);
	else
	// surrender
	if(Game::HotKeyPress(Game::EVENT_BATTLE_SURRENDER))
	    ProcessingHeroDialogResult(3, a);

	// debug only
	if(IS_DEVEL()) switch(le.KeyValue())
	{
	    case KEY_w:
		if(arena.result_game)
		{
		    // fast wins game
		    arena.result_game->army1 = RESULT_WINS;
		    humanturn_exit = true;
		}
		break;

	    case KEY_l:
		if(arena.result_game)
		{
		    // fast loss game
		    arena.result_game->army1 = RESULT_LOSS;
		    humanturn_exit = true;
		}
		break;

    	    default: break;
    	}
    }

    if(pocket_book.w && le.MouseCursor(pocket_book))
    {
	cursor.SetThemes(Cursor::WAR_POINTER);
	msg = _("Spell cast");

	if(le.MouseClickLeft(pocket_book))
	{
	    ProcessingHeroDialogResult(1, a);
	    humanturn_redraw = true;
	}
    }
    else
    if(le.MouseCursor(btn_auto))
    {
	cursor.SetThemes(Cursor::WAR_POINTER);
	msg = _("Auto combat");
	ButtonAutoAction(b, a);
    }
    else
    if(le.MouseCursor(btn_settings))
    {
	cursor.SetThemes(Cursor::WAR_POINTER);
	msg = _("Customize system options.");
	ButtonSettingsAction();
    }
    else
    if(le.MouseCursor(btn_skip))
    {
	cursor.SetThemes(Cursor::WAR_POINTER);
	msg = _("Skip this unit");
	ButtonSkipAction(a);
    }
    else
    if(opponent1 && le.MouseCursor(opponent1->GetArea()))
    {
	if(conf.MyColor() == arena.army1.GetColor())
	{
	    msg = _("Hero's Options");
	    cursor.SetThemes(Cursor::WAR_HERO);

	    if(le.MouseClickLeft(opponent1->GetArea()))
	    {
		ProcessingHeroDialogResult(arena.DialogBattleHero(*opponent1->GetHero()), a);
		humanturn_redraw = true;
	    }
	}
	else
	{
	    msg = _("View Opposing Hero");
	    cursor.SetThemes(Cursor::WAR_INFO);

	    if(le.MouseClickLeft(opponent1->GetArea()))
	    {
		arena.DialogBattleHero(*opponent1->GetHero());
		humanturn_redraw = true;
	    }
	}
    }
    else
    if(opponent2 && le.MouseCursor(opponent2->GetArea()))
    {
	if(conf.MyColor() == arena.army2.GetColor())
	{
	    msg = _("Hero's Options");
	    cursor.SetThemes(Cursor::WAR_HERO);

	    if(le.MouseClickLeft(opponent2->GetArea()))
	    {
		ProcessingHeroDialogResult(arena.DialogBattleHero(*opponent2->GetHero()), a);
		humanturn_redraw = true;
	    }
	}
	else
	{
	    msg = _("View Opposing Hero");
	    cursor.SetThemes(Cursor::WAR_INFO);

	    if(le.MouseClickLeft(opponent2->GetArea()))
	    {
		arena.DialogBattleHero(*opponent2->GetHero());
		humanturn_redraw = true;
	    }
	}
    }
    else
    if(le.MouseCursor(rectBoard))
    {
	const u16 themes = GetBattleCursor(le.GetMouseCursor(), msg);

	if(cursor.Themes() != themes)
	    cursor.SetThemes(themes);

	if(le.MouseClickLeft())
	    MouseLeftClickBoardAction(themes, arena.board.GetIndexAbsPosition(le.GetMouseCursor()), a);
	else
	if(le.MousePressRight())
	    MousePressRightBoardAction(themes, arena.board.GetIndexAbsPosition(le.GetMouseCursor()), a);
    }
    else
    {
	cursor.SetThemes(Cursor::WAR_NONE);
    }
}

void Battle2::Interface::HumanCastSpellTurn(const Stats & b, Actions & a, std::string & msg)
{
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    // reset cast
    if(le.MousePressRight())
	humanturn_spell = Spell::NONE;
    else
    if(le.MouseCursor(rectBoard) && humanturn_spell != Spell::NONE)
    {
	const u16 themes = GetBattleSpellCursor(le.GetMouseCursor(), msg);

	if(cursor.Themes() != themes)
	    cursor.SetThemes(themes);

	if(le.MouseClickLeft() && Cursor::WAR_NONE != cursor.Themes())
	{
	    const s16 index = arena.board.GetIndexAbsPosition(le.GetMouseCursor());

	    if(index < 0)
	    {
		DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Interface::HumanCastSpellTurn: " << " dst out of range");
		return;
	    }

	    if(Cursor::SP_TELEPORT == cursor.Themes())
	    {
		if(MAXU16 == teleport_src)
		    teleport_src = index;
		else
		{
		    DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Interface::HumanCastSpellTurn: " << Spell::GetName(Spell::FromInt(humanturn_spell)) << ", dst: " << index);
		    a.AddedCastTeleportAction(teleport_src, index);
		    humanturn_spell = Spell::NONE;
		    humanturn_exit = true;
		    teleport_src = MAXU16;
		}
	    }
	    else
	    {
		DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Interface::HumanCastSpellTurn: " << Spell::GetName(Spell::FromInt(humanturn_spell)) << ", dst: " << index);
		a.AddedCastAction(humanturn_spell, index);
		humanturn_spell = Spell::NONE;
		humanturn_exit = true;
	    }
	}
    }
    else
    {
	cursor.SetThemes(Cursor::WAR_NONE);
    }
}

void Battle2::Interface::FadeArena(void)
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    Settings & conf = Settings::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.WAR_POINTER);
    Redraw();
    cursor.Show();
    display.Flip();
    if(!conf.QVGA())
    {
	display.Fade(100);
	display.Flip();
    }
}

u8 Battle2::GetIndexIndicator(const Stats & b)
{
    // yellow
    if(b.Modes(IS_GREEN_STATUS) && b.Modes(IS_RED_STATUS)) return 13;
    else
    // green
    if(b.Modes(IS_GREEN_STATUS))	return 12;
    else
    // red
    if(b.Modes(IS_RED_STATUS))		return 14;

    return 10;
}

void Battle2::Interface::SetAutoBattle(const Stats & b, Actions & a)
{
    Settings::Get().SetAutoBattle(true);
    Cursor::Get().SetThemes(Cursor::WAR_NONE);
    status.SetMessage(_("Set auto battle on"), true);
    arena.AITurn(b, a);
    humanturn_redraw = true;
    humanturn_exit = true;
}

void Battle2::Interface::ResetAutoBattle(void)
{
    Settings::Get().SetAutoBattle(false);
    status.SetMessage(_("Set auto battle off"), true);
    humanturn_redraw = true;
}

void Battle2::Interface::EventShowOptions(void)
{
    btn_settings.PressDraw();
    DialogBattleSettings();
    btn_settings.ReleaseDraw();
    humanturn_redraw = true;
}

void Battle2::Interface::EventAutoSwitch(const Stats & b, Actions & a)
{
    btn_auto.PressDraw();
    if(Settings::Get().AutoBattle())
	ResetAutoBattle();
    else
	SetAutoBattle(b, a);
    btn_auto.ReleaseDraw();
}

void Battle2::Interface::ButtonAutoAction(const Stats & b, Actions & a)
{
    LocalEvent & le = LocalEvent::Get();

    le.MousePressLeft(btn_auto) ? btn_auto.PressDraw() : btn_auto.ReleaseDraw();

    if(le.MouseClickLeft(btn_auto))
    {
	if(Settings::Get().AutoBattle())
	    ResetAutoBattle();
	else
	    SetAutoBattle(b, a);
    }
}

void Battle2::Interface::ButtonSettingsAction(void)
{
    LocalEvent & le = LocalEvent::Get();

    le.MousePressLeft(btn_settings) ? btn_settings.PressDraw() : btn_settings.ReleaseDraw();

    if(le.MouseClickLeft(btn_settings))
    {
	DialogBattleSettings();
	humanturn_redraw = true;
    }
}

void Battle2::Interface::ButtonSkipAction(Actions & a)

{
    LocalEvent & le = LocalEvent::Get();

    le.MousePressLeft(btn_skip) ? btn_skip.PressDraw() : btn_skip.ReleaseDraw();

    if(le.MouseClickLeft(btn_skip) && b_current)
    {
	a.AddedSkipAction(*b_current, true);
	humanturn_exit = true;
    }
}

u8 Battle2::Interface::GetAllowSwordDirection(u16 index)
{
    u8 res = 0;

    if(b_current)
    {
	for(direction_t dir = TOP_LEFT; dir < CENTER; ++dir)
	{
	    if(Board::isValidDirection(index, dir))
	    {
    		const u16 from = Board::GetIndexDirection(index, dir);

    		if(UNKNOWN != arena.board[from].GetDirection() ||
            	    from == b_current->GetPosition() ||
            	    (b_current->isWide() && from == b_current->GetTailIndex()))
		{
		    res |= dir;
		}
	    }
	}
    }

    return res;
}

void Battle2::Interface::MousePressRightBoardAction(u16 themes, s16 index, Actions & a)
{
    if(index >= 0)
    {
	const Stats* b = arena.GetTroopBoard(index);
	if(b)
	{
	    const Settings & conf = Settings::Get();
	    const u8 allow = GetAllowSwordDirection(index);

	    if(conf.MyColor() == b->GetColor() || !conf.ExtTapMode() || !allow)
		Dialog::ArmyInfo(b->troop, Dialog::READONLY);
	    else
	    switch(PocketPC::GetCursorAttackDialog(b->GetCellPosition(), allow))
	    {
		case Cursor::SWORD_TOPLEFT:     MouseLeftClickBoardAction(Cursor::SWORD_TOPLEFT, index, a); break;
		case Cursor::SWORD_TOPRIGHT:    MouseLeftClickBoardAction(Cursor::SWORD_TOPRIGHT, index, a); break;
		case Cursor::SWORD_RIGHT:       MouseLeftClickBoardAction(Cursor::SWORD_RIGHT, index, a); break;
		case Cursor::SWORD_BOTTOMRIGHT: MouseLeftClickBoardAction(Cursor::SWORD_BOTTOMRIGHT, index, a); break;
		case Cursor::SWORD_BOTTOMLEFT:  MouseLeftClickBoardAction(Cursor::SWORD_BOTTOMLEFT, index, a); break;
		case Cursor::SWORD_LEFT:        MouseLeftClickBoardAction(Cursor::SWORD_LEFT, index, a); break;
	
		default: Dialog::ArmyInfo(b->troop, Dialog::READONLY); break;
	    }
	}
    }
}

void Battle2::Interface::MouseLeftClickBoardAction(u16 themes, s16 index, Actions & a)
{
    if(index >= 0 && b_current)
    switch(themes)
    {
	case Cursor::WAR_FLY:
	case Cursor::WAR_MOVE:
	    a.AddedMoveAction(*b_current, index);
	    a.AddedEndAction(*b_current);
	    humanturn_exit = true;
	    break;

	case Cursor::SWORD_TOPLEFT:
	case Cursor::SWORD_TOPRIGHT:
	case Cursor::SWORD_RIGHT:
	case Cursor::SWORD_BOTTOMRIGHT:
	case Cursor::SWORD_BOTTOMLEFT:
	case Cursor::SWORD_LEFT:
	{
	    const Stats* enemy = arena.GetTroopBoard(index);
	    const direction_t dir = GetDirectionFromCursorSword(themes);

	    if(enemy && Board::isValidDirection(index, dir))
	    {
		const u16 move = Board::GetIndexDirection(index, dir);
		if(b_current->GetPosition() != move)
		    a.AddedMoveAction(*b_current, move);
		a.AddedAttackAction(*b_current, *enemy);
		a.AddedEndAction(*b_current);
		humanturn_exit = true;
	    }
	    break;
	}

	case Cursor::WAR_BROKENARROW:
	case Cursor::WAR_ARROW:
	{
	    const Stats* enemy = arena.GetTroopBoard(index);
	    if(enemy)
	    {
		a.AddedAttackAction(*b_current, *enemy);
		a.AddedEndAction(*b_current);
		humanturn_exit = true;
	    }
	    break;
	}

	case Cursor::WAR_INFO:
	{
	    const Stats* b = arena.GetTroopBoard(index);
	    if(b)
	    {
		Dialog::ArmyInfo(b->troop, Dialog::BUTTONS | Dialog::READONLY);
		humanturn_redraw = true;
	    }
	    break;
	}

	default: break;
    }
}

void Battle2::Interface::RedrawTroopFrameAnimation(Stats & b)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    while(le.HandleEvents())
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_FRAME_DELAY))
    	{
	    cursor.Hide();
	    Redraw();
	    cursor.Show();
	    display.Flip();
    	    if(b.isFinishAnimFrame()) break;
	    b.IncreaseAnimFrame();
	}
    }
}

void Battle2::Interface::RedrawActionSkipStatus(const Stats & attacker)
{
    std::string msg;
    if(attacker.Modes(TR_HARDSKIP))
    {
	msg = _("%{name} skipping turn");
	if(Settings::Get().ExtBattleSkipIncreaseDefense()) msg.append(_(", and get +2 defense"));
    }
    else
	msg = _("%{name} waiting turn");

    String::Replace(msg, "%{name}", attacker.GetName());
    status.SetMessage(msg, true);
}

void Battle2::Interface::RedrawActionAttackPart1(Stats & attacker, Stats & defender, const std::vector<TargetInfo> & targets)
{
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();
    Cursor & cursor = Cursor::Get();

    cursor.SetThemes(Cursor::WAR_NONE);

    b_current = NULL;
    b_move = &attacker;
    p_move = attacker.GetCellPosition();

    u8 action0 = AS_ATTK0;
    u8 action1 = 0;

    const Rect & pos1 = attacker.GetCellPosition();
    const Rect & pos2 = defender.GetCellPosition();

    if(pos2.y < pos1.y)
	action1 = AS_ATTK1;
    else
    if(pos2.y > pos1.y)
	action1 = AS_ATTK3;
    else
	action1 = AS_ATTK2;

    // long distance attack animation
    if(attacker.isDoubleCellAttack() && 2 == targets.size())
    {
	action0 = AS_SHOT0;
	if(action1 == AS_ATTK1)
	    action1 = AS_SHOT1;
	else
	if(action1 == AS_ATTK3)
	    action1 = AS_SHOT3;
	else
	    action1 = AS_SHOT2;
    }

    // check archers
    const bool archer = attacker.isArchers() && !attacker.isHandFighting();

    if(archer)
    {
	const float dx = pos1.x - pos2.x;
	const float dy = pos1.y - pos2.y;
        const float tan = std::fabs(dy / dx);

	action0 = AS_SHOT0;
	action1 = (0.6 >= tan ? AS_SHOT2 : (dy > 0 ? AS_SHOT1 : AS_SHOT3));
    }

    // redraw luck animation
    if(attacker.Modes(LUCK_GOOD))
    {
	std::string msg = _("Good luck shines on the  %{attacker}");
	String::Replace(msg, "%{attacker}", attacker.GetName());
	status.SetMessage(msg, true);
	status.SetMessage("", false);

	RedrawTroopWithFrameAnimation(attacker, ICN::BLESS, M82::FromSpell(Spell::BLESS), false);
    }
    else
    if(attacker.Modes(LUCK_BAD))
    {
	std::string msg = _("Bad luck descends on the %{attacker}");
	String::Replace(msg, "%{attacker}", attacker.GetName());
	status.SetMessage(msg, true);
	status.SetMessage("", false);
    }

    AGG::PlaySound(attacker.M82Attk());

    // redraw attack animation
    attacker.ResetAnimFrame(action0);
    RedrawTroopFrameAnimation(attacker);

    if(attacker.GetFrameState(action1).count)
    {
	attacker.ResetAnimFrame(action1);
	RedrawTroopFrameAnimation(attacker);
    }
    DELAY(200);

    // draw missile animation
    if(archer)
    {
	const Sprite & missile = AGG::GetICN(attacker.ICNMiss(), ICN::GetMissIndex(attacker.ICNMiss(), pos1.x - pos2.x, pos1.y - pos2.y), pos1.x > pos2.x);
	std::vector<Point> points;
	std::vector<Point>::const_iterator pnt;
	
	if(Settings::Get().QVGA())
	{
	    const Point line_from(pos1.x + (attacker.isReflect() ? -5 : pos1.w), pos1.y + attacker.GetStartMissileOffset(action1) / 2);
	    const Point line_to(pos2.x + (attacker.isReflect() ? pos1.w : 0), pos2.y);
	    GetLinePoints(line_from, line_to, (missile.w() < 8 ? 8 : missile.w()), points);
	}
	else
	{
	    const Point line_from(pos1.x + (attacker.isReflect() ? -10 : pos1.w), pos1.y + attacker.GetStartMissileOffset(action1));
	    const Point line_to(pos2.x + (attacker.isReflect() ? pos1.w : 0), pos2.y);
	    GetLinePoints(line_from, line_to, (missile.w() < 8 ? 8 : missile.w()), points);
	}

	pnt = points.begin();
	while(le.HandleEvents(false) && pnt != points.end())
	{
	    CheckGlobalEvents(le);

	    if(Game::AnimateInfrequent(Game::BATTLE_MISSILE_DELAY))
    	    {
		cursor.Hide();
		Redraw();
		display.Blit(missile, *pnt);
		cursor.Show();
		display.Flip();
		++pnt;
	    }
	}
    }

    // post attack action
    switch(attacker.troop.GetID())
    {
	case Monster::VAMPIRE_LORD:
	    // possible: vampire ressurect animation
	    //RedrawTroopWithFrameAnimation(attacker, , );
	    break;

	case Monster::LICH:
	case Monster::POWER_LICH:
	    // lich clod animation
	    RedrawTroopWithFrameAnimation(defender, ICN::LICHCLOD, attacker.M82Expl(), true);
	    break;

	default: break;
    }
}

void Battle2::Interface::RedrawActionAttackPart2(Stats & attacker, std::vector<TargetInfo> & targets)
{
    // targets damage animation
    RedrawActionWinces(targets);

    // draw status for first defender
    if(targets.size())
    {
	std::string msg = _("%{attacker} do %{damage} damage.");
	String::Replace(msg, "%{attacker}", attacker.GetName());

	if(1 < targets.size())
	{
	    u32 killed = 0;
	    u32 damage = 0;
	    std::vector<TargetInfo>::const_iterator it1 = targets.begin();
	    std::vector<TargetInfo>::const_iterator it2 = targets.end();
	    for(; it1 != it2; ++it1){ killed += (*it1).killed; damage += (*it1).damage; }

	    String::Replace(msg, "%{damage}", damage);

	    if(killed)
	    {
	        msg.append(" ");
		msg.append(_("%{count} creatures perished."));
    		String::Replace(msg, "%{count}", killed);
	    }
	}
	else
	{
	    TargetInfo & target = targets.front();
	    String::Replace(msg, "%{damage}", target.damage);

	    if(target.killed)
	    {
		msg.append(" ");
		msg.append(ngettext("one %{defender} perish.", "%{count} %{defender} perished.", target.killed));
    		String::Replace(msg, "%{count}", target.killed);
    		String::Replace(msg, "%{defender}", target.defender->GetPluralName(target.killed));
	    }
	}

	status.SetMessage(msg, true);
	status.SetMessage("", false);
    }

    // targets killed animation
    RedrawActionKills(targets);

    // restore
    std::vector<TargetInfo>::iterator it = targets.begin();
    for(; it != targets.end(); ++it) if((*it).defender)
    {
	TargetInfo & target = *it;
	if(!target.defender->isValid())
	{
	    const animframe_t & frm = target.defender->GetFrameState(AS_KILL);
	    target.defender->animframe = frm.start + frm.count - 1;
	}
	else
	    target.defender->ResetAnimFrame(AS_IDLE);
    }
    if(opponent1) opponent1->ResetAnimFrame(OP_IDLE);
    if(opponent2) opponent2->ResetAnimFrame(OP_IDLE);
    b_move = NULL;
    attacker.ResetAnimFrame(AS_IDLE);
}

void Battle2::Interface::RedrawActionWinces(std::vector<TargetInfo> & targets)
{
    const Settings & conf = Settings::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();
    Cursor & cursor = Cursor::Get();

    // targets damage animation
    std::vector<TargetInfo>::iterator it = targets.begin();
    std::string msg;
    u16 py = (conf.QVGA() ? 20 : 50);
    u8 finish = 0;

    for(; it != targets.end(); ++it) if((*it).defender)
    {
	if((*it).damage)
	{
	    TargetInfo & target = *it;
	    // wnce animation
	    target.defender->ResetAnimFrame(AS_WNCE);
	    AGG::PlaySound(target.defender->M82Wnce());
	    ++finish;
	}
	else
	{
	    // defense
	    AGG::PlaySound(M82::RSBRYFZL);
	}
    }

    // targets damage animation loop
    while(le.HandleEvents() && finish != std::count_if(targets.begin(), targets.end(), std::mem_fun_ref(&TargetInfo::isFinishAnimFrame)))
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_FRAME_DELAY))
    	{
	    it = targets.begin();
	    for(; it != targets.end(); ++it) if((*it).defender)
	    {
		TargetInfo & target = *it;
		const Rect & pos = target.defender->GetCellPosition();

		cursor.Hide();
		Redraw();

		// extended damage info
		if(conf.ExtBattleShowDamage() && target.killed)
		{
		    msg = "-";
		    String::AddInt(msg, target.killed);
		    Text txt(msg, Font::YELLOW_SMALL);
		    txt.Blit(pos.x + (target.defender->isWide() ? 0 : (pos.w - txt.w()) / 2), pos.y - py);
		}

		cursor.Show();
		display.Flip();
    		target.defender->IncreaseAnimFrame();
	    }
	    py += (conf.QVGA() ? 5 : 10);
	}
    }

    DELAY(200);
}

void Battle2::Interface::RedrawActionKills(std::vector<TargetInfo> & targets)
{
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();
    Cursor & cursor = Cursor::Get();

    // targets damage animation
    std::vector<TargetInfo>::iterator it = targets.begin();
    std::string msg;
    u8 finish = 0;

    // targets killed animation
    it = targets.begin();
    finish = 0;
    for(; it != targets.end(); ++it) if((*it).defender)
    {
	TargetInfo & target = *it;

	if(!target.defender->isValid())
	{
	    AGG::PlaySound(target.defender->M82Kill());
	    target.defender->ResetAnimFrame(AS_KILL);
	    ++finish;
	    // set opponent OP_SRRW animation
 	    OpponentSprite* commander = NULL;
	    if(target.defender->GetColor() != Color::GRAY)
	    {
		commander = target.defender->GetColor() == arena.army1.GetColor() ? opponent1 : opponent2;
	    }
	    if(commander) commander->ResetAnimFrame(OP_SRRW);
	}
	else
	    target.defender->ResetAnimFrame(AS_IDLE);
    }

    // targets killed animation loop
    while(le.HandleEvents() && finish != std::count_if(targets.begin(), targets.end(), std::mem_fun_ref(&TargetInfo::isFinishAnimFrame)))
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_FRAME_DELAY))
    	{
	    it = targets.begin();
	    for(; it != targets.end(); ++it) if((*it).defender)
	    {
		TargetInfo & target = *it;

		cursor.Hide();
		Redraw();
		cursor.Show();
		display.Flip();

		if(!target.defender->isValid()) target.defender->IncreaseAnimFrame();
	    }
	}
    }
}

void Battle2::Interface::RedrawActionMove(Stats & b, const std::vector<u16> & path)
{
    Cursor & cursor = Cursor::Get();
    std::vector<u16>::const_iterator dst = path.begin();

    cursor.SetThemes(Cursor::WAR_NONE);

    b_current = NULL;
    b_move = &b;

    while(dst != path.end() && *dst < arena.board.size())
    {
	const Cell & cell = arena.board[*dst];
	p_move = cell.pos;
    
	if(arena.bridge)
	{
	    if(!arena.bridge->isDown() && arena.bridge->NeedDown(b, *dst))
	    {
		RedrawBridgeAnimation(true);
		arena.bridge->SetDown(true);
	    }
	    else
	    if(arena.bridge->isValid() && arena.bridge->isDown() && arena.bridge->AllowUp())
	    {
		RedrawBridgeAnimation(false);
		arena.bridge->SetDown(false);
	    }
	}

	if(b.isWide() && b.GetTailIndex() == *dst)
	    b.UpdateDirection(cell);
	else
	{
	    b.UpdateDirection(cell);
	    AGG::PlaySound(b.M82Move());
	    b.ResetAnimFrame(AS_MOVE);
	    RedrawTroopFrameAnimation(b);
	}
	b.position = *dst;
	++dst;
    }

    // restore
    b_fly = NULL;
    b_move = NULL;
    b_current = NULL;
    b.ResetAnimFrame(AS_IDLE);
}

void Battle2::Interface::RedrawActionFly(Stats & b, u16 dst)
{
    Cursor & cursor = Cursor::Get();
    const Rect & pos1 = b.GetCellPosition();
    const Rect & pos2 = arena.board[dst].pos;
    std::vector<Point> points;
    std::vector<Point>::const_iterator pnt;

    Point pt1(pos1.x, pos1.y);
    Point pt2(pos2.x, pos2.y);

    cursor.SetThemes(Cursor::WAR_NONE);
    const u8 step = b.isWide() ? 80 : 40;
    GetLinePoints(pt1, pt2, Settings::Get().QVGA() ? step / 2 : step, points);

    pnt = points.begin();

    // jump up
    b_current = NULL;
    b_move = NULL;
    p_move = pnt != points.end() ? *pnt : pt1;
    b_fly = NULL;
    b_move = &b;
    p_fly = pt1;

    b.ResetAnimFrame(AS_FLY1);
    RedrawTroopFrameAnimation(b);

    b_move = NULL;
    b_fly = &b;
    p_fly = p_move;
    if(pnt != points.end()) ++pnt;

    while(pnt != points.end())
    {
	p_move = *pnt;

	AGG::PlaySound(b.M82Move());
	b.ResetAnimFrame(AS_FLY2);
	RedrawTroopFrameAnimation(b);

	p_fly = p_move;
	++pnt;
    }

    b.position = dst;

    // jump down
    b_fly = NULL;
    b_move = &b;
    p_move = pt2;
    b.ResetAnimFrame(AS_FLY3);
    RedrawTroopFrameAnimation(b);

    // restore
    b_move = NULL;
    b.ResetAnimFrame(AS_IDLE);
}

void Battle2::Interface::RedrawActionResistSpell(const Stats & target)
{
    std::string str(_("The %{name} resist the spell!"));
    String::Replace(str, "%{name}", target.GetName());
    status.SetMessage(str, true);
    status.SetMessage("", false);
}

void Battle2::Interface::RedrawActionSpellCastPart1(u8 spell2, u16 dst, const std::string & name, const std::vector<TargetInfo> & targets)
{
    Spell::spell_t spell = Spell::FromInt(spell2);
    std::string msg;
    Stats* target = targets.size() ? targets.front().defender : NULL;

    if(target && target->GetPosition() == dst)
    {
	msg = _("%{name} casts %{spell} on the %{troop}.");
	String::Replace(msg, "%{troop}", target->GetName());
    }
    else
    if(Spell::isApplyWithoutFocusObject(spell))
	msg = _("%{name} casts %{spell}.");

    if(msg.size())
    {
	String::Replace(msg, "%{name}", name);
	String::Replace(msg, "%{spell}", Spell::GetName(Spell::FromInt(spell)));
	status.SetMessage(msg, true);
	status.SetMessage("", false);
    }

    // without object
    switch(spell)
    {
    	    case Spell::FIREBALL:	RedrawTargetsWithFrameAnimation(dst, targets, ICN::FIREBALL, M82::FromSpell(spell)); break;
    	    case Spell::FIREBLAST:	RedrawTargetsWithFrameAnimation(dst, targets, ICN::FIREBAL2, M82::FromSpell(spell)); break;
    	    case Spell::METEORSHOWER:	RedrawTargetsWithFrameAnimation(dst, targets, ICN::METEOR, M82::FromSpell(spell)); break;
    	    case Spell::COLDRING:   	RedrawActionColdRingSpell(dst, targets); break;

    	    case Spell::MASSSHIELD:	RedrawTargetsWithFrameAnimation(targets, ICN::SHIELD, M82::FromSpell(spell), false); break;
    	    case Spell::MASSCURE:	RedrawTargetsWithFrameAnimation(targets, ICN::MAGIC01, M82::FromSpell(spell), false); break;
    	    case Spell::MASSHASTE:	RedrawTargetsWithFrameAnimation(targets, ICN::HASTE, M82::FromSpell(spell), false); break;
    	    case Spell::MASSSLOW:	RedrawTargetsWithFrameAnimation(targets, ICN::MAGIC02, M82::FromSpell(spell), false); break;
    	    case Spell::MASSBLESS:	RedrawTargetsWithFrameAnimation(targets, ICN::BLESS, M82::FromSpell(spell), false); break;
    	    case Spell::MASSCURSE:	RedrawTargetsWithFrameAnimation(targets, ICN::CURSE, M82::FromSpell(spell), false); break;
    	    case Spell::MASSDISPEL:	RedrawTargetsWithFrameAnimation(targets, ICN::MAGIC07, M82::FromSpell(spell), false); break;

    	    case Spell::DEATHRIPPLE:
    	    case Spell::DEATHWAVE:	RedrawTargetsWithFrameAnimation(targets, ICN::REDDEATH, M82::FromSpell(spell), true); break;

    	    case Spell::HOLYWORD:
    	    case Spell::HOLYSHOUT:	RedrawTargetsWithFrameAnimation(targets, ICN::BLUEFIRE, M82::FromSpell(spell), true); break;

	    case Spell::ELEMENTALSTORM:	RedrawActionElementalStormSpell(targets); break;
	    case Spell::ARMAGEDDON:	RedrawActionArmageddonSpell(targets); break;

	    default: break;
    }

    // with object
    if(target)
    {
	if(Spell::isResurrect(spell))
	    RedrawActionResurrectSpell(*target, spell);
	else
	switch(spell)
	{
	    // simple spell animation
	    case Spell::BLESS:		RedrawTroopWithFrameAnimation(*target, ICN::BLESS, M82::FromSpell(spell), false); break;
	    case Spell::BLIND:		RedrawTroopWithFrameAnimation(*target, ICN::BLIND, M82::FromSpell(spell), false); break;
	    case Spell::CURE:		RedrawTroopWithFrameAnimation(*target, ICN::MAGIC01, M82::FromSpell(spell), false); break;
	    case Spell::SLOW:		RedrawTroopWithFrameAnimation(*target, ICN::MAGIC02, M82::FromSpell(spell), false); break;
	    case Spell::SHIELD:		RedrawTroopWithFrameAnimation(*target, ICN::SHIELD, M82::FromSpell(spell), false); break;
	    case Spell::HASTE:		RedrawTroopWithFrameAnimation(*target, ICN::HASTE, M82::FromSpell(spell), false); break;
	    case Spell::CURSE:		RedrawTroopWithFrameAnimation(*target, ICN::CURSE, M82::FromSpell(spell), false); break;
	    case Spell::ANTIMAGIC:	RedrawTroopWithFrameAnimation(*target, ICN::MAGIC06, M82::FromSpell(spell), false); break;
	    case Spell::DISPEL:		RedrawTroopWithFrameAnimation(*target, ICN::MAGIC07, M82::FromSpell(spell), false); break;
	    case Spell::STONESKIN:	RedrawTroopWithFrameAnimation(*target, ICN::STONSKIN, M82::FromSpell(spell), false); break;
	    case Spell::STEELSKIN:	RedrawTroopWithFrameAnimation(*target, ICN::STELSKIN, M82::FromSpell(spell), false); break;
	    case Spell::PARALYZE:	RedrawTroopWithFrameAnimation(*target, ICN::PARALYZE, M82::FromSpell(spell), false); break;
	    case Spell::HYPNOTIZE:	RedrawTroopWithFrameAnimation(*target, ICN::HYPNOTIZ, M82::FromSpell(spell), false); break;
	    case Spell::DRAGONSLAYER:	RedrawTroopWithFrameAnimation(*target, ICN::DRAGSLAY, M82::FromSpell(spell), false); break;
	    case Spell::BERSERKER:	RedrawTroopWithFrameAnimation(*target, ICN::BERZERK, M82::FromSpell(spell), false); break;

	    // uniq spell animation
	    case Spell::LIGHTNINGBOLT:	RedrawActionLightningBoltSpell(*target); break;
	    case Spell::CHAINLIGHTNING:	RedrawActionChainLightningSpell(targets); break;
	    case Spell::ARROW:		RedrawActionArrowSpell(*target); break;
    	    case Spell::COLDRAY:   	RedrawActionColdRaySpell(*target); break;
    	    case Spell::DISRUPTINGRAY: 	RedrawActionDisruptingRaySpell(*target); break;
    	    case Spell::BLOODLUST:   	RedrawActionBloodLustSpell(*target); break;
	    default: break;
	}
    }
}

void Battle2::Interface::RedrawActionSpellCastPart2(u8 spell, std::vector<TargetInfo> & targets)
{
    if(Spell::isDamage(spell))
    {
        // targets damage animation
	RedrawActionWinces(targets);

	u32 killed = 0;
	u32 damage = 0;
	std::vector<TargetInfo>::const_iterator it1 = targets.begin();
	std::vector<TargetInfo>::const_iterator it2 = targets.end();
	for(; it1 != it2; ++it1){ killed += (*it1).killed; damage += (*it1).damage; }

	if(damage)
	{
	    std::string msg;
	    if(Spell::isUndeadOnly(spell))
		msg = _("The %{spell} spell does %{damage} damage to all undead creatures.");
	    else
	    if(Spell::isALiveOnly(spell))
		msg = _("The %{spell} spell does %{damage} damage to all living creatures.");
	    else
		msg = _("The %{spell} does %{damage} damage.");
	    String::Replace(msg, "%{spell}", Spell::GetName(Spell::FromInt(spell)));
	    String::Replace(msg, "%{damage}", damage);

	    if(killed)
	    {
		msg.append(" ");
		msg.append(ngettext("one creature perish.", "%{count} creatures perished.", killed));
    		String::Replace(msg, "%{count}", killed);
	    }

	    status.SetMessage(msg, true);
	}

	// target killed animation
	RedrawActionKills(targets);
    }

    status.SetMessage(" ", false);

    // restore
    std::vector<TargetInfo>::iterator it = targets.begin();
    for(; it != targets.end(); ++it) if((*it).defender)
    {
	TargetInfo & target = *it;
	if(!target.defender->isValid())
	{
	    const animframe_t & frm = target.defender->GetFrameState(AS_KILL);
	    target.defender->animframe = frm.start + frm.count - 1;
	}
	else
	    target.defender->ResetAnimFrame(AS_IDLE);
    }
    if(opponent1) opponent1->ResetAnimFrame(OP_IDLE);
    if(opponent2) opponent2->ResetAnimFrame(OP_IDLE);
    b_move = NULL;
}

void Battle2::Interface::RedrawActionMonsterSpellCastStatus(const Stats & attacker, const TargetInfo & target)
{
    const char* msg = NULL;

    switch(attacker.troop())
    {
	case Monster::UNICORN:		msg = _("The Unicorns attack blinds the %{name}!"); break;
	case Monster::MEDUSA:		msg = _("The Medusas gaze turns the %{name} to stone!"); break;
	case Monster::ROYAL_MUMMY:
	case Monster::MUMMY:		msg = _("The Mummies' curse falls upon the %{name}!"); break;
	case Monster::CYCLOPS:		msg = _("The %{name} are paralyzed by the Cyclopes!"); break;
	case Monster::ARCHMAGE:		msg = _("The Archmagi dispel all good spells on your %{name}!"); break;
	default: break;
    }

    if(msg)
    {
	std::string str(msg);
	String::Replace(str, "%{name}", target.defender->GetName());

	status.SetMessage(str, true);
	status.SetMessage("", false);
    }
}

void Battle2::Interface::RedrawActionMorale(Stats & b, bool good)
{
    std::string msg;

    if(good)
    {
	msg = _("High morale enables the %{monster} to attack again.");
	String::Replace(msg, "%{monster}", b.GetName());
	status.SetMessage(msg, true);
	RedrawTroopWithFrameAnimation(b, ICN::MORALEG, M82::GOODMRLE, false);
    }
    else
    {
    	msg = _("Low morale causes the %{monster} to freeze in panic.");
	String::Replace(msg, "%{monster}", b.GetName());
	status.SetMessage(msg, true);
	RedrawTroopWithFrameAnimation(b, ICN::MORALEB, M82::BADMRLE, true);
    }
}

void Battle2::Interface::RedrawActionTowerPart1(Tower & tower, Stats & defender)
{
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();
    Cursor & cursor = Cursor::Get();

    cursor.SetThemes(Cursor::WAR_NONE);
    b_current = NULL;

    const Point pos1 = tower.GetPortPosition();
    const Rect & pos2 = defender.GetCellPosition();

    AGG::PlaySound(M82::KEEPSHOT);

    // draw missile animation
    const Sprite & missile = AGG::GetICN(ICN::KEEP, ICN::GetMissIndex(ICN::KEEP, pos1.x - pos2.x, pos1.y - pos2.y), pos1.x > pos2.x);
    std::vector<Point> points;
    std::vector<Point>::const_iterator pnt;

    GetLinePoints(pos1, Point(pos2.x + pos2.w, pos2.y), missile.w(), points);

    pnt = points.begin();
    while(le.HandleEvents(false) && pnt != points.end())
    {
	CheckGlobalEvents(le);

	// fast draw
	if(Game::AnimateInfrequent(Game::BATTLE_MISSILE_DELAY))
    	{
	    cursor.Hide();
	    Redraw();
	    display.Blit(missile, *pnt);
	    cursor.Show();
	    display.Flip();
	    ++pnt;
	}
    }
}

void Battle2::Interface::RedrawActionTowerPart2(Tower & tower, TargetInfo & target)
{
    std::vector<TargetInfo> targets;
    targets.push_back(target);

    // targets damage animation
    RedrawActionWinces(targets);

    // draw status for first defender
    std::string msg = _("Tower do %{damage} damage.");
    String::Replace(msg, "%{damage}", target.damage);
    if(target.killed)
    {
	msg.append(" ");
	msg.append(ngettext("one %{defender} perish.", "%{count} %{defender} perished.", target.killed));
    	String::Replace(msg, "%{count}", target.killed);
    	String::Replace(msg, "%{defender}", target.defender->GetName());
    }
    status.SetMessage(msg, true);
    status.SetMessage("", false);

    // targets killed animation
    RedrawActionKills(targets);

    // restore
    if(!target.defender->isValid())
    {
	const animframe_t & frm = target.defender->GetFrameState(AS_KILL);
	target.defender->animframe = frm.start + frm.count - 1;
    }
    else
	target.defender->ResetAnimFrame(AS_IDLE);

    if(opponent1) opponent1->ResetAnimFrame(OP_IDLE);
    if(opponent2) opponent2->ResetAnimFrame(OP_IDLE);
    b_move = NULL;
}

void Battle2::Interface::RedrawActionCatapult(u8 target)
{
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();
    Cursor & cursor = Cursor::Get();

    const Sprite & missile = AGG::GetICN(ICN::BOULDER, 0);
    const Rect & area = border.GetArea();

    std::vector<Point> points;
    std::vector<Point>::const_iterator pnt;
    AGG::PlaySound(M82::CATSND00);

    // catapult animation
    while(le.HandleEvents(false) && catapult_frame < 6)
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_CATAPULT_DELAY))
    	{
	    cursor.Hide();
	    Redraw();
	    cursor.Show();
	    display.Flip();
	    ++catapult_frame;
	}
    }

    // boulder animation
    Point pt1(90, 220);
    Point pt2 = arena.catapult->GetTargetPosition(target);
    Point max(300, 20);

    if(Settings::Get().QVGA())
    {
	pt1.x /= 2;
	pt1.y /= 2;
	pt2.x /= 2;
	pt2.y /= 2;
	max.x /= 2;
	max.y /= 2;
    }

    pt1.x += area.x;
    pt2.x += area.x;
    max.x += area.x;
    pt1.y += area.y;
    pt2.y += area.y;
    max.y += area.y;

    GetArcPoints(pt1, pt2, max, missile.w(), points);

    pnt = points.begin();
    while(le.HandleEvents(false) && pnt != points.end())
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_CATAPULT2_DELAY))
	{
	    if(catapult_frame < 9) ++catapult_frame;

	    cursor.Hide();
	    Redraw();
	    display.Blit(missile, *pnt);
	    cursor.Show();
	    display.Flip();
	    ++pnt;
	}
    }

    // clod
    u8 frame = 0;
    ICN::icn_t icn = target == CAT_MISS ? ICN::SMALCLOD : ICN::LICHCLOD;
    AGG::PlaySound(M82::CATSND02);

    while(le.HandleEvents() && frame < AGG::GetICNCount(icn))
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_CATAPULT3_DELAY))
    	{
	    if(catapult_frame < 9) ++catapult_frame;

	    cursor.Hide();
	    Redraw();
	    const Sprite & sprite = AGG::GetICN(icn, frame);
	    display.Blit(sprite, pt2.x + sprite.x(), pt2.y + sprite.y());
	    cursor.Show();
	    display.Flip();

	    ++frame;
	}
    }

    catapult_frame = 0;
}

void Battle2::Interface::RedrawActionArrowSpell(const Stats & target)
{
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();
    Cursor & cursor = Cursor::Get();

    if(arena.current_commander)
    {
	Point pt_from, pt_to;

	if(arena.current_commander == opponent1->GetHero())
	{
	    const Rect & pos1 = opponent1->GetArea();
	    pt_from = Point(pos1.x + pos1.w, pos1.y + pos1.h / 2);

	    const Rect & pos2 = target.GetCellPosition();
	    pt_to = Point(pos2.x, pos2.y);
	}
	else
	{
	    const Rect & pos = opponent2->GetArea();
	    pt_from = Point(pos.x, pos.y + pos.h / 2);

	    const Rect & pos2 = target.GetCellPosition();
	    pt_to = Point(pos2.x + pos2.w, pos2.y);
	}

	const Sprite & missile = AGG::GetICN(ICN::ARCH_MSL, ICN::GetMissIndex(ICN::ARCH_MSL, pt_from.x - pt_to.x, pt_from.y - pt_to.y), pt_from.x > pt_to.x);
	std::vector<Point> points;
	std::vector<Point>::const_iterator pnt;

	GetLinePoints(pt_from, pt_to, missile.w(), points);

	cursor.SetThemes(Cursor::WAR_NONE);
	AGG::PlaySound(M82::MAGCAROW);

	pnt = points.begin();
	while(le.HandleEvents(false) && pnt != points.end())
	{
	    CheckGlobalEvents(le);

	    if(Game::AnimateInfrequent(Game::BATTLE_MISSILE_DELAY))
    	    {
		cursor.Hide();
		Redraw();
		display.Blit(missile, *pnt);
		cursor.Show();
		display.Flip();
		++pnt;
	    }
	}
    }
}

void Battle2::Interface::RedrawActionTeleportSpell(Stats & target, const u16 dst)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    u8 alpha = 250;

    const MonsterInfo & msi = target.GetMonsterInfo();
    const Sprite & sprite1 = AGG::GetICN(msi.icn_file, msi.frm_idle.start, target.reflect);
    Sprite sprite2;
    sprite2.Set(sprite1.w(), sprite1.h());
    sprite2.SetColorKey();
    sprite2.Blit(sprite1);
    sprite2.SetOffset(sprite1.x(), sprite1.y());


    cursor.SetThemes(Cursor::WAR_NONE);
    cursor.Hide();

    b_current = &target;
    b_current_sprite = &sprite2;

    AGG::PlaySound(M82::TELPTOUT);

    while(le.HandleEvents() && alpha > 30)
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    sprite2.SetAlpha(alpha);
	    Redraw();
	    cursor.Show();
	    display.Flip();

	    alpha -= 20;
	}
    }

    sprite2.SetAlpha(0);
    cursor.Hide();
    Redraw();
    while(Mixer::isValid() && Mixer::isPlaying(-1)) DELAY(10);

    target.SetPosition(dst);
    AGG::PlaySound(M82::TELPTIN);

    while(le.HandleEvents() && alpha < 220)
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    sprite2.SetAlpha(alpha);
	    Redraw();
	    cursor.Show();
	    display.Flip();

	    alpha += 20;
	}
    }

    b_current = NULL;
    b_current_sprite = NULL;
}

void Battle2::Interface::RedrawActionSummonElementalSpell(const Stats & target)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    u8 alpha = 0;

    const MonsterInfo & msi = target.GetMonsterInfo();
    const Sprite & sprite1 = AGG::GetICN(msi.icn_file, msi.frm_idle.start, target.reflect);
    Sprite sprite2;
    sprite2.Set(sprite1.w(), sprite1.h());
    sprite2.SetColorKey();
    sprite2.Blit(sprite1);
    sprite2.SetOffset(sprite1.x(), sprite1.y());

    cursor.SetThemes(Cursor::WAR_NONE);
    cursor.Hide();

    b_current = &target;
    b_current_sprite = &sprite2;

    AGG::PlaySound(M82::SUMNELM);

    while(le.HandleEvents() && alpha < 220)
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    sprite2.SetAlpha(alpha);
	    Redraw();
	    cursor.Show();
	    display.Flip();

	    alpha += 20;
	}
    }

    b_current = NULL;
    b_current_sprite = NULL;
}

void Battle2::Interface::RedrawActionMirrorImageSpell(const Stats & target, u16 dst)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    const MonsterInfo & msi = target.GetMonsterInfo();
    const Sprite & sprite = AGG::GetICN(msi.icn_file, msi.frm_idle.start, target.reflect);
    std::vector<Point> points;
    std::vector<Point>::const_iterator pnt;
    const Rect  & rt1 = target.GetCellPosition();
    const Point & pt2 = arena.board[dst].GetPos();

    GetLinePoints(rt1, pt2, 5, points);

    cursor.SetThemes(Cursor::WAR_NONE);
    cursor.Hide();

    AGG::PlaySound(M82::MIRRORIM);

    pnt = points.begin();
    while(le.HandleEvents() && pnt != points.end())
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();

    	    const s16 sx = target.isReflect() ?
                (*pnt).x + (target.isWide() ? rt1.w + rt1.w / 2 : rt1.w / 2) - sprite.w() - sprite.x() :
                (*pnt).x + (target.isWide() ? -rt1.w / 2 : rt1.w / 2) + sprite.x();
    	    const s16 sy = (*pnt).y + rt1.h + sprite.y() - 10;

	    Redraw();
	    display.Blit(sprite, sx, sy);
	    cursor.Show();
	    display.Flip();

	    ++pnt;
	}
    }
}

void Battle2::Interface::RedrawActionLightningBoltSpell(Stats & target)
{
    // FIX: LightningBolt draw
    RedrawTroopWithFrameAnimation(target, ICN::SPARKS, M82::FromSpell(Spell::LIGHTNINGBOLT), true);
}

void Battle2::Interface::RedrawActionChainLightningSpell(const std::vector<TargetInfo> & targets)
{
    std::vector<TargetInfo>::const_iterator it1 = targets.begin();
    std::vector<TargetInfo>::const_iterator it2 = targets.end();

    // FIX: ChainLightning draw
    //AGG::PlaySound(targets.size() > 1 ? M82::CHAINLTE : M82::LIGHTBLT);

    for(; it1 != it2; ++it1)
	RedrawTroopWithFrameAnimation(*(it1->defender), ICN::SPARKS, M82::FromSpell(Spell::LIGHTNINGBOLT), true);
}

void Battle2::Interface::RedrawActionBloodLustSpell(Stats & target)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    u8 alpha = 10;

    const MonsterInfo & msi = target.GetMonsterInfo();
    const Sprite & sprite1 = AGG::GetICN(msi.icn_file, msi.frm_idle.start, target.reflect);
    Sprite sprite2;
    sprite2.SetOffset(sprite1.x(), sprite1.y());
    sprite2.Set(sprite1.w(), sprite1.h());
    sprite2.SetColorKey();
    sprite2.Blit(sprite1);
    Surface sprite3;
    Surface::MakeStencil(sprite3, sprite1, sprite1.GetColor(0xD8));

    cursor.SetThemes(Cursor::WAR_NONE);
    cursor.Hide();

    b_current = &target;
    b_current_sprite = &sprite2;
    AGG::PlaySound(M82::BLOODLUS);

    while(le.HandleEvents() && alpha < 120)
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    sprite3.SetAlpha(alpha);
	    sprite2.Blit(sprite1);
	    sprite2.Blit(sprite3);
	    Redraw();
	    cursor.Show();
	    display.Flip();

	    alpha += 10;
	}
    }

    while(Mixer::isValid() && Mixer::isPlaying(-1)) DELAY(10);

    b_current = NULL;
    b_current_sprite = NULL;
}

void Battle2::Interface::RedrawActionColdRaySpell(Stats & target)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    const ICN::icn_t icn = ICN::COLDRAY;
    u8 frame = 0;

    std::vector<Point> points;
    std::vector<Point>::const_iterator pnt;
    Point pt_from, pt_to;

    if(arena.current_commander == opponent1->GetHero())
    {
	const Rect & pos1 = opponent1->GetArea();
	pt_from = Point(pos1.x + pos1.w, pos1.y + pos1.h / 2);

	const Rect & pos2 = target.GetCellPosition();
	pt_to = Point(pos2.x, pos2.y);
    }
    else
    {
	const Rect & pos = opponent2->GetArea();
	pt_from = Point(pos.x, pos.y + pos.h / 2);

	const Rect & pos2 = target.GetCellPosition();
	pt_to = Point(pos2.x + pos2.w, pos2.y);
    }

    const u16 dx = std::abs(pt_from.x - pt_to.x);
    const u16 dy = std::abs(pt_from.y - pt_to.y);
    const u16 step = (dx > dy ? dx / AGG::GetICNCount(icn) : dy / AGG::GetICNCount(icn));
    GetLinePoints(pt_from, pt_to, step, points);

    cursor.SetThemes(Cursor::WAR_NONE);
    AGG::PlaySound(M82::COLDRAY);

    pnt = points.begin();
    while(le.HandleEvents() && frame < AGG::GetICNCount(icn) && pnt != points.end())
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    const Sprite & sprite = AGG::GetICN(icn, frame);
	    display.Blit(sprite, (*pnt).x - sprite.w() / 2, (*pnt).y - sprite.h() / 2);
	    cursor.Show();
	    display.Flip();

	    ++frame;
	    ++pnt;
	}
    }

    RedrawTroopWithFrameAnimation(target, ICN::ICECLOUD, M82::UNKNOWN, true);
}

void Battle2::Interface::RedrawActionResurrectSpell(Stats & target, u8 spell)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    M82::m82_t wav = M82::UNKNOWN;

    switch(spell)
    {
        case Spell::ANIMATEDEAD:	wav = M82::MNRDEATH; break;
        case Spell::RESURRECT:		wav = M82::RESURECT; break;
        case Spell::RESURRECTTRUE:	wav = M82::RESURTRU; break;
	default: break;
    }

    AGG::PlaySound(wav);

    if(!target.isValid())
    {
        target.animstep = -1;

	while(le.HandleEvents() && !target.isFinishAnimFrame())
	{
	    CheckGlobalEvents(le);

	    if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	    {
		cursor.Hide();
		Redraw();
		cursor.Show();
		display.Flip();
		target.IncreaseAnimFrame();
	    }
	}

        target.animstep = 1;
    }

    RedrawTroopWithFrameAnimation(target, ICN::YINYANG, M82::UNKNOWN, false);
}

void Battle2::Interface::RedrawActionDisruptingRaySpell(Stats & target)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    const MonsterInfo & msi = target.GetMonsterInfo();
    const Sprite & sprite1 = AGG::GetICN(msi.icn_file, msi.frm_idle.start, target.reflect);
    Sprite sprite2(*target.contours[target.isReflect() ? 3 : 2]);
    sprite2.SetOffset(sprite1.x(), sprite1.y());

    const ICN::icn_t icn = ICN::DISRRAY;
    u8 frame = 0;
    std::vector<Point> points;
    std::vector<Point>::const_iterator pnt;
    Point pt_from, pt_to;

    if(arena.current_commander == opponent1->GetHero())
    {
	const Rect & pos1 = opponent1->GetArea();
	pt_from = Point(pos1.x + pos1.w, pos1.y + pos1.h / 2);

	const Rect & pos2 = target.GetCellPosition();
	pt_to = Point(pos2.x, pos2.y);
    }
    else
    {
	const Rect & pos = opponent2->GetArea();
	pt_from = Point(pos.x, pos.y + pos.h / 2);

	const Rect & pos2 = target.GetCellPosition();
	pt_to = Point(pos2.x + pos2.w, pos2.y);
    }

    const u16 dx = std::abs(pt_from.x - pt_to.x);
    const u16 dy = std::abs(pt_from.y - pt_to.y);
    const u16 step = (dx > dy ? dx / AGG::GetICNCount(icn) : dy / AGG::GetICNCount(icn));
    GetLinePoints(pt_from, pt_to, step, points);

    cursor.SetThemes(Cursor::WAR_NONE);
    AGG::PlaySound(M82::DISRUPTR);

    pnt = points.begin();
    while(le.HandleEvents() && frame < AGG::GetICNCount(icn) && pnt != points.end())
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    const Sprite & sprite = AGG::GetICN(icn, frame);
	    display.Blit(sprite, (*pnt).x - sprite.w() / 2, (*pnt).y - sprite.h() / 2);
	    cursor.Show();
	    display.Flip();

	    ++frame;
	    ++pnt;
	}
    }

    // part 2
    frame = 0;
    const Stats* old_current = b_current;
    b_current = &target;
    b_current_sprite = &sprite2;
    p_move = Point(0, 0);

    while(le.HandleEvents() && frame < 20)
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_DISRUPTING_DELAY))
    	{
	    cursor.Hide();
	    sprite2.SetOffset(sprite1.x() + (frame % 2 ? -1 : 1), sprite1.y());
	    Redraw();
	    cursor.Show();
	    display.Flip();

	    ++frame;
	}
    }

    b_current = old_current;
    b_current_sprite = NULL;
}

void Battle2::Interface::RedrawActionColdRingSpell(const u16 dst, const std::vector<TargetInfo> & targets)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    const ICN::icn_t icn = ICN::COLDRING;
    const M82::m82_t m82 = M82::FromSpell(Spell::COLDRING);

    u8 frame = 0;
    std::vector<TargetInfo>::const_iterator it;
    const Rect & center = arena.board[dst].GetPos();

    cursor.SetThemes(Cursor::WAR_NONE);

    // set WNCE
    b_current = NULL;
    for(it = targets.begin(); it != targets.end(); ++it)
	if((*it).defender && (*it).damage) (*it).defender->ResetAnimFrame(AS_WNCE);

    if(M82::UNKNOWN != m82) AGG::PlaySound(m82);

    while(le.HandleEvents() && frame < AGG::GetICNCount(icn))
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    Redraw();

	    const Sprite & sprite1 = AGG::GetICN(icn, frame);
	    display.Blit(sprite1, center.x + center.w / 2 + sprite1.x(), center.y + center.h / 2 + sprite1.y());
	    const Sprite & sprite2 = AGG::GetICN(icn, frame, true);
	    display.Blit(sprite2, center.x + center.w / 2 - sprite2.w() - sprite2.x(), center.y + center.h / 2 + sprite2.y());
	    cursor.Show();
	    display.Flip();

	    for(it = targets.begin(); it != targets.end(); ++it) if((*it).defender && (*it).damage)
		(*it).defender->IncreaseAnimFrame(false);
	    ++frame;
	}
    }


    for(it = targets.begin(); it != targets.end(); ++it) if((*it).defender)
    {
        (*it).defender->ResetAnimFrame(AS_IDLE);
	b_current = NULL;
    }
}

void Battle2::Interface::RedrawActionElementalStormSpell(const std::vector<TargetInfo> & targets)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    const ICN::icn_t icn = ICN::STORM;
    const M82::m82_t m82 = M82::FromSpell(Spell::ELEMENTALSTORM);
    const Rect & area = border.GetArea();

    u8 frame = 0;
    u8 repeat = 4;
    std::vector<TargetInfo>::const_iterator it;
    Point center;

    cursor.SetThemes(Cursor::WAR_NONE);

    b_current = NULL;
    for(it = targets.begin(); it != targets.end(); ++it)
	if((*it).defender && (*it).damage) (*it).defender->ResetAnimFrame(AS_WNCE);

    if(M82::UNKNOWN != m82) AGG::PlaySound(m82);

    while(le.HandleEvents() && frame < AGG::GetICNCount(icn))
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    Redraw();

	    const Sprite & sprite = AGG::GetICN(icn, frame);
	    for(center.y = area.y; center.y + sprite.h() < area.y + area.h - 20; center.y += sprite.h())
		for(center.x = area.x; center.x + sprite.w() < area.x + area.w; center.x += sprite.w())
		    display.Blit(sprite, center);

	    RedrawInterface();
	    cursor.Show();
	    display.Flip();

	    for(it = targets.begin(); it != targets.end(); ++it) if((*it).defender && (*it).damage)
		(*it).defender->IncreaseAnimFrame(false);
	    ++frame;

	    if(frame == AGG::GetICNCount(icn) && repeat)
	    {
		--repeat;
		frame = 0;
	    }
	}
    }


    for(it = targets.begin(); it != targets.end(); ++it) if((*it).defender)
    {
        (*it).defender->ResetAnimFrame(AS_IDLE);
	b_current = NULL;
    }
}

void Battle2::Interface::RedrawActionArmageddonSpell(const std::vector<TargetInfo> & targets)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();
    Rect area = border.GetArea();

    u8 alpha = 10;
    area.h -= Settings::Get().QVGA() ? 18 : 36;

    Surface sprite1;
    Surface sprite2;
    sprite1.Set(area.w, area.h);
    sprite2.Set(area.w, area.h);

    cursor.SetThemes(Cursor::WAR_NONE);
    cursor.Hide();

    sprite1.Blit(display, area, 0, 0);
    sprite2.Fill(sprite2.GetColor(0xD8));

    b_current = NULL;
    AGG::PlaySound(M82::ARMGEDN);

    while(le.HandleEvents() && alpha < 180)
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    sprite2.SetAlpha(alpha);
	    Redraw();
	    display.Blit(sprite1, area.x, area.y);
	    display.Blit(sprite2, area.x, area.y);
	    RedrawInterface();
	    cursor.Show();
	    display.Flip();

	    alpha += 10;
	}
    }

    cursor.Hide();

    alpha = 0;
    const u8 offset = Settings::Get().QVGA() ? 5 : 10;
    bool restore = false;

    while(le.HandleEvents() && alpha < 20)
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    if(restore)
	    {
		display.Blit(sprite1, area.x, area.y);
		restore = false;
	    }
	    else
	    {
		switch(Rand::Get(1, 4))
		{
		    case 1:	    display.Blit(sprite1, area.x + offset, area.y + offset); break;
		    case 2:	    display.Blit(sprite1, area.x - offset, area.y - offset); break;
		    case 3:	    display.Blit(sprite1, area.x - offset, area.y + offset); break;
		    case 4:	    display.Blit(sprite1, area.x + offset, area.y - offset); break;
		    default: break;
		}
		restore = true;
	    }

	    display.Blit(sprite2, area.x, area.y);
	    RedrawInterface();
	    RedrawBorder();
	    cursor.Show();
	    display.Flip();
	    ++alpha;
	}
    }

    while(Mixer::isValid() && Mixer::isPlaying(-1)) DELAY(10);
}

void Battle2::Interface::RedrawActionEarthQuakeSpell(const std::vector<u8> & targets)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();
    Rect area = border.GetArea();

    u8 frame = 0;
    area.h -= Settings::Get().QVGA() ? 19 : 38;

    std::vector<u8>::const_iterator it;

    Surface sprite;
    sprite.Set(area.w, area.h);

    cursor.SetThemes(Cursor::WAR_NONE);
    cursor.Hide();

    sprite.Blit(display, area, 0, 0);

    b_current = NULL;
    AGG::PlaySound(M82::ERTHQUAK);

    const u8 offset = Settings::Get().QVGA() ? 5 : 10;
    bool restore = false;

    while(le.HandleEvents() && frame < 18)
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    if(restore)
	    {
		display.Blit(sprite, area.x, area.y);
		restore = false;
	    }
	    else
	    {
		switch(Rand::Get(1, 4))
		{
		    case 1:	    display.Blit(sprite, area.x + offset, area.y + offset); break;
		    case 2:	    display.Blit(sprite, area.x - offset, area.y - offset); break;
		    case 3:	    display.Blit(sprite, area.x - offset, area.y + offset); break;
		    case 4:	    display.Blit(sprite, area.x + offset, area.y - offset); break;
		    default: break;
		}
		restore = true;
	    }

	    RedrawInterface();
	    RedrawBorder();
	    cursor.Show();
	    display.Flip();
	    ++frame;
	}
    }

    // clod
    frame = 0;
    ICN::icn_t icn = ICN::LICHCLOD;
    AGG::PlaySound(M82::CATSND02);

    while(le.HandleEvents() && frame < AGG::GetICNCount(icn))
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    Redraw();

	    it = targets.begin();
	    for(; it != targets.end(); ++it)
	    {
		Point pt2 = arena.catapult->GetTargetPosition(*it);

		if(Settings::Get().QVGA())
		{
		    pt2.x /= 2;
		    pt2.y /= 2;
		}
		pt2.x += area.x;
		pt2.y += area.y;

		const Sprite & sprite = AGG::GetICN(icn, frame);
		display.Blit(sprite, pt2.x + sprite.x(), pt2.y + sprite.y());
	    }

	    cursor.Show();
	    display.Flip();

	    ++frame;
	}
    }
}

void Battle2::Interface::RedrawActionRemoveMirrorImage(const Stats & mirror)
{
    DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Interface::RedrawActionRemoveMirrorImage: FIXME");
}

void Battle2::Interface::RedrawTargetsWithFrameAnimation(const u16 dst, const std::vector<TargetInfo> & targets, ICN::icn_t icn, M82::m82_t m82)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    u8 frame = 0;
    std::vector<TargetInfo>::const_iterator it;
    const Rect & center = arena.board[dst].GetPos();

    cursor.SetThemes(Cursor::WAR_NONE);

    b_current = NULL;
    for(it = targets.begin(); it != targets.end(); ++it)
	if((*it).defender && (*it).damage) (*it).defender->ResetAnimFrame(AS_WNCE);

    if(M82::UNKNOWN != m82) AGG::PlaySound(m82);

    while(le.HandleEvents() && frame < AGG::GetICNCount(icn))
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    Redraw();

	    const Sprite & sprite = AGG::GetICN(icn, frame);
	    display.Blit(sprite, center.x + center.w / 2 + sprite.x(), center.y + center.h / 2 + sprite.y());
	    cursor.Show();
	    display.Flip();

	    for(it = targets.begin(); it != targets.end(); ++it) if((*it).defender && (*it).damage)
		(*it).defender->IncreaseAnimFrame(false);
	    ++frame;
	}
    }

    for(it = targets.begin(); it != targets.end(); ++it) if((*it).defender)
    {
        (*it).defender->ResetAnimFrame(AS_IDLE);
	b_current = NULL;
    }
}

void Battle2::Interface::RedrawTargetsWithFrameAnimation(const std::vector<TargetInfo> & targets, ICN::icn_t icn, M82::m82_t m82, bool wnce)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    u8 frame = 0;
    std::vector<TargetInfo>::const_iterator it;

    cursor.SetThemes(Cursor::WAR_NONE);

    b_current = NULL;

    if(wnce)
    for(it = targets.begin(); it != targets.end(); ++it)
	if((*it).defender && (*it).damage) (*it).defender->ResetAnimFrame(AS_WNCE);

    if(M82::UNKNOWN != m82) AGG::PlaySound(m82);

    while(le.HandleEvents() && frame < AGG::GetICNCount(icn))
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    Redraw();

	    for(it = targets.begin(); it != targets.end(); ++it) if((*it).defender)
	    {
		const Rect & pos = (*it).defender->GetCellPosition();
		bool reflect = false;

		switch(icn)
		{
		    case ICN::SHIELD: reflect = (*it).defender->isReflect(); break;
		    default: break;
		}

		const Sprite & sprite = AGG::GetICN(icn, frame, reflect);
		display.Blit(sprite, pos.x + ((*it).defender->isWide() ? ((*it).defender->isReflect() ? pos.w : 0) : pos.w / 2) + sprite.x() - (reflect ? pos.w : 0),
				pos.y + pos.h / 2 + sprite.y());
	    }
	    cursor.Show();
	    display.Flip();

	    if(wnce)
	    for(it = targets.begin(); it != targets.end(); ++it) if((*it).defender && (*it).damage)
		(*it).defender->IncreaseAnimFrame(false);
	    ++frame;
	}
    }

    if(wnce)
    for(it = targets.begin(); it != targets.end(); ++it) if((*it).defender)
    {
        (*it).defender->ResetAnimFrame(AS_IDLE);
	b_current = NULL;
    }
}

void Battle2::Interface::RedrawTroopWithFrameAnimation(Stats & b, ICN::icn_t icn, M82::m82_t m82, bool pain)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    const Rect & pos = b.GetCellPosition();

    u8 frame = 0;
    bool reflect = false;

    switch(icn)
    {
	case ICN::SHIELD: reflect = b.isReflect(); break;
	default: break;
    }

    cursor.SetThemes(Cursor::WAR_NONE);

    if(pain)
    {
	b_current = NULL;
	b.ResetAnimFrame(AS_WNCE);
    }

    if(M82::UNKNOWN != m82) AGG::PlaySound(m82);

    while(le.HandleEvents() && frame < AGG::GetICNCount(icn))
    {
	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_SPELL_DELAY))
    	{
	    cursor.Hide();
	    Redraw();
	    const Sprite & sprite = AGG::GetICN(icn, frame, reflect);
	    display.Blit(sprite, pos.x + (b.isWide() ? (b.isReflect() ? pos.w : 0) : pos.w / 2) + sprite.x() - (reflect ? pos.w : 0),
		    pos.y + pos.h / 2 + sprite.y());
	    cursor.Show();
	    display.Flip();

    	    if(pain) b.IncreaseAnimFrame(false);
	    ++frame;
	}
    }

    if(pain)
    {
        b.ResetAnimFrame(AS_IDLE);
	b_current = NULL;
    }
}

void Battle2::Interface::RedrawBridgeAnimation(bool down)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();
    const Point & topleft = border.GetArea();

    ICN::icn_t icn_castle  = ICN::UNKNOWN;

    switch(arena.castle->GetRace())
    {
        default:
        case Race::BARB: icn_castle = ICN::CASTLEB; break;
        case Race::KNGT: icn_castle = ICN::CASTLEK; break;
        case Race::NECR: icn_castle = ICN::CASTLEN; break;
        case Race::SORC: icn_castle = ICN::CASTLES; break;
        case Race::WRLK: icn_castle = ICN::CASTLEW; break;
        case Race::WZRD: icn_castle = ICN::CASTLEZ; break;
    }

    u8 frame = down ? 23 : 21;

    if(down) AGG::PlaySound(M82::DRAWBRG);

    while(le.HandleEvents())
    {
	if(down)
	{
	    if(frame < 21) break;
	}
	else
	{
	    if(frame > 23) break;
	}

	CheckGlobalEvents(le);

	if(Game::AnimateInfrequent(Game::BATTLE_BRIDGE_DELAY))
    	{
	    cursor.Hide();
	    Redraw();
    	    const Sprite & sprite = AGG::GetICN(icn_castle, frame);
    	    display.Blit(sprite, sprite.x() + topleft.x, sprite.y() + topleft.y);
	    cursor.Show();
	    display.Flip();

	    if(down)
		--frame;
	    else
		++frame;
	}
    }

    if(!down) AGG::PlaySound(M82::DRAWBRG);
}

bool Battle2::ArmySetIdleAnimation(Army::army_t & army)
{
    bool res = false;

    for(u8 ii = 0; ii < army.Size(); ++ii)
    {
	Army::Troop & troop = army.At(ii);
	if(troop.isValid())
	{
	    Stats* b = troop.GetBattleStats();
	    if(b && b->isValid())
	    {
		if(b->isFinishAnimFrame())
		    b->ResetAnimFrame(AS_IDLE);
		else
		if(b->isStartAnimFrame() && 3 > Rand::Get(1, 10))
		{
		    b->IncreaseAnimFrame();
		    res = true;
		}
	    }
	}
    }

    return res;
}

bool Battle2::ArmyNextIdleAnimation(Army::army_t & army)
{
    bool res = false;

    for(u8 ii = 0; ii < army.Size(); ++ii)
    {
	Army::Troop & troop = army.At(ii);
	if(troop.isValid())
	{
	    Stats* b = troop.GetBattleStats();
	    if(b && b->isValid() && !b->isStartAnimFrame())
	    {
		b->IncreaseAnimFrame(false);
		res = true;
	    }
	}
    }

    return res;
}

bool Battle2::Interface::IdleTroopsAnimation(void)
{
    bool res = false;

    // set animation
    if(Game::AnimateInfrequent(Game::BATTLE_IDLE_DELAY))
    {
	if(ArmySetIdleAnimation(arena.army1)) res = true;
	if(ArmySetIdleAnimation(arena.army2)) res = true;
    }
    else
    // next animation
    if(Game::AnimateInfrequent(Game::BATTLE_IDLE2_DELAY))
    {
	if(ArmyNextIdleAnimation(arena.army1)) res = true;
	if(ArmyNextIdleAnimation(arena.army2)) res = true;
    }

    return res;
}

void Battle2::Interface::CheckGlobalEvents(LocalEvent & le)
{
    // reset auto battle
    if(le.KeyPress() && Settings::Get().AutoBattle()) ResetAutoBattle();

    // animation opponents
    if(Game::AnimateInfrequent(Game::BATTLE_OPPONENTS_DELAY))
    {
	if(opponent1)
	{
	    if(!opponent1->isStartFrame() || 2 > Rand::Get(1, 10)) opponent1->IncreaseAnimFrame();
	}

	if(opponent2)
	{
	    if(!opponent2->isStartFrame() || 2 > Rand::Get(1, 10)) opponent2->IncreaseAnimFrame();
	}
	humanturn_redraw = true;
    }

    // animation flags
    if(Game::AnimateInfrequent(Game::BATTLE_FLAGS_DELAY))
    {
	if(opponent1 && opponent1->isFinishFrame()) opponent1->ResetAnimFrame(OP_IDLE);
	if(opponent2 && opponent2->isFinishFrame()) opponent2->ResetAnimFrame(OP_IDLE);

	++animation_frame;
	humanturn_redraw = true;
    }
}

void Battle2::Interface::ProcessingHeroDialogResult(u8 res, Actions & a)
{
    const Settings & conf = Settings::Get();

    switch(res)
    {
	//cast
	case 1:
	{
	    const HeroBase* hero = b_current ? b_current->GetCommander() : NULL;
	    if(hero)
	    {
		if(hero->HaveSpellBook())
		{
		    std::string msg;
		    if(arena.isDisableCastSpell(Spell::NONE, &msg))
			Dialog::Message("", msg, Font::BIG, Dialog::OK);
		    else
		    {
			Spell::spell_t spell = hero->OpenSpellBook(SpellBook::CMBT, true);

			if(arena.isDisableCastSpell(spell, &msg))
			    Dialog::Message("", msg, Font::BIG, Dialog::OK);
			else
			if(hero->HaveSpellPoints(Spell::CostManaPoints(spell, hero)))
			{
			    if(Spell::isApplyWithoutFocusObject(spell))
			    {
				a.AddedCastAction(spell, MAXU16);
				humanturn_redraw = true;
				humanturn_exit = true;
			    }
			    else
				humanturn_spell = spell;
			}
		    }
		}
		else
		    Dialog::Message("", _("No spells to cast."), Font::BIG, Dialog::OK);
	    }
        }
	break;

	// retreat
	case 2:
	    if(b_current->GetCommander() && arena.CanRetreatOpponent(b_current->GetColor()) &&
		Dialog::YES == Dialog::Message("", _("Are you sure you want to retreat?"), Font::BIG, Dialog::YES | Dialog::NO))
	    {
		a.AddedRetreatAction();
		a.AddedEndAction(*b_current);
		humanturn_exit = true;
	    }
	    break;

	//surrender
	case 3:
	{
	    const Army::army_t & army = arena.army1.GetColor() == conf.MyColor() ? arena.army1 : arena.army2;
	    const HeroBase* enemy = arena.army1.GetColor() == conf.MyColor() ? arena.army2.GetCommander() : arena.army1.GetCommander();
	    const u32 cost = army.GetSurrenderCost();
	    if(enemy && DialogBattleSurrender(*enemy, cost))
	    {
		if(world.GetKingdom(conf.MyColor()).GetFundsGold() < cost)
		    Dialog::Message("", _("You don't have enough gold!"), Font::BIG, Dialog::OK);
		else
		{
		    a.AddedSurrenderAction();
		    a.AddedEndAction(*b_current);
		    humanturn_exit = true;
		}
	    }
	}
        break;

	default: break;
    }
}

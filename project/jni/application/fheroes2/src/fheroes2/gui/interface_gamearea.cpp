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

#include "agg.h"
#include "settings.h"
#include "world.h"
#include "maps.h"
#include "game.h"
#include "game_focus.h"
#include "game_interface.h"
#include "route.h"
#include "interface_gamearea.h"

#define SCROLL_MIN	8
#define SCROLL_MAX	TILEWIDTH

namespace Game
{
    // game_startgame.cpp
    Cursor::themes_t GetCursor(s32);
    void MouseCursorAreaClickLeft(s32);
    void MouseCursorAreaPressRight(s32);
}

Interface::GameArea & Interface::GameArea::Get(void)
{
    static Interface::GameArea ga;
    return ga;
}

Interface::GameArea::GameArea() : oldIndexPos(0), updateCursor(false)
{
}

const Rect & Interface::GameArea::GetArea(void) const
{ return areaPosition; }

const Point& Interface::GameArea::GetMapsPos(void) const
{ return rectMapsPosition; }

const Rect & Interface::GameArea::GetRectMaps(void) const
{ return rectMaps; }

/* fixed src rect image */
void Interface::GameArea::SrcRectFixed(Rect & src, Point & dst, const u16 rw, const u16 rh)
{
    SrcRectFixed(src, dst.x, dst.y, rw, rh);
}

void Interface::GameArea::SrcRectFixed(Rect & src, s16 & dst_x, s16 & dst_y, const u16 rw, const u16 rh)
{
    src = Rect(0, 0, 0, 0);
    const Rect & areaPosition = Interface::GameArea::Get().GetArea();

    if(0 != rw && 0 != rh &&
        dst_x + rw > areaPosition.x && dst_y + rh > areaPosition.y &&
        dst_x < areaPosition.x + areaPosition.w && dst_y < areaPosition.y + areaPosition.h)
    {
	src.w = rw;
	src.h = rh;

	if(dst_x < areaPosition.x)
	{
    	    src.x = areaPosition.x - dst_x;
    	    dst_x = areaPosition.x;
	}

	if(dst_y < areaPosition.y)
	{
    	    src.y = areaPosition.y - dst_y;
    	    dst_y = areaPosition.y;
	}

	if(dst_x + rw > areaPosition.x + areaPosition.w)
	    src.w = areaPosition.x + areaPosition.w - dst_x;

	if(dst_y + rh > areaPosition.y + areaPosition.h)
	    src.h = areaPosition.y + areaPosition.h - dst_y;
    }
}

void Interface::GameArea::Build(void)
{
    if(Settings::Get().HideInterface())
	SetAreaPosition(0, 0,
		    Display::Get().w(),
		    Display::Get().h());
    else
	SetAreaPosition(BORDERWIDTH, BORDERWIDTH,
		    Display::Get().w() - RADARWIDTH - 3 * BORDERWIDTH,
		    Display::Get().h() - 2 * BORDERWIDTH);
}

void Interface::GameArea::SetAreaPosition(s16 x, s16 y, u16 w, u16 h)
{
    areaPosition.x = x;
    areaPosition.y = y;
    areaPosition.w = w;
    areaPosition.h = h;

    rectMaps.x = 0;
    rectMaps.y = 0;
    rectMaps.w = (areaPosition.w / TILEWIDTH) + 2;
    rectMaps.h = (areaPosition.h / TILEWIDTH) + 2;

    scrollOffset.x = 0;
    scrollOffset.y = 0;
    scrollStep = Settings::Get().ScrollSpeed();

    rectMapsPosition.x = areaPosition.x + scrollOffset.x;
    rectMapsPosition.y = areaPosition.y + scrollOffset.y;
}

void Interface::GameArea::BlitOnTile(Surface & dst, const Sprite & src, const Point & mp) const
{
    BlitOnTile(dst, src, src.x(), src.y(), mp);
}

void Interface::GameArea::BlitOnTile(Surface & dst, const Surface & src, const s16 ox, const s16 oy, const Point & mp) const
{
    const s16 & dstx = rectMapsPosition.x + TILEWIDTH * (mp.x - rectMaps.x);
    const s16 & dsty = rectMapsPosition.y + TILEWIDTH * (mp.y - rectMaps.y);

    Point dstpt(dstx + ox, dsty + oy);

    if(areaPosition & Rect(dstpt, src.w(), src.h()))
    {
	Rect srcrt;
	SrcRectFixed(srcrt, dstpt, src.w(), src.h());
	dst.Blit(src, srcrt, dstpt);
    }
}

void Interface::GameArea::Redraw(Surface & dst, u8 flag) const
{
    return Redraw(dst, flag, Rect(0, 0, rectMaps.w, rectMaps.h));
}

void Interface::GameArea::Redraw(Surface & dst, u8 flag, const Rect & rt) const
{
    // tile
    for(s16 oy = rt.y; oy < rt.y + rt.h; ++oy)
	for(s16 ox = rt.x; ox < rt.x + rt.w; ++ox)
	    world.GetTiles(rectMaps.x + ox, rectMaps.y + oy).RedrawTile(dst);

    // bottom
    if(flag & LEVEL_BOTTOM)
    for(s16 oy = rt.y; oy < rt.y + rt.h; ++oy)
	for(s16 ox = rt.x; ox < rt.x + rt.w; ++ox)
	    world.GetTiles(rectMaps.x + ox, rectMaps.y + oy).RedrawBottom(dst);

    // ext object
    if(flag & LEVEL_OBJECTS)
    for(s16 oy = rt.y; oy < rt.y + rt.h; ++oy)
	for(s16 ox = rt.x; ox < rt.x + rt.w; ++ox)
	    world.GetTiles(rectMaps.x + ox, rectMaps.y + oy).RedrawObjects(dst);

    // top
    if(flag & LEVEL_TOP)
    for(s16 oy = rt.y; oy < rt.y + rt.h; ++oy)
	for(s16 ox = rt.x; ox < rt.x + rt.w; ++ox)
	    world.GetTiles(rectMaps.x + ox, rectMaps.y + oy).RedrawTop(dst);

    // heroes
    for(s16 oy = rt.y; oy < rt.y + rt.h; ++oy)
	for(s16 ox = rt.x; ox < rt.x + rt.w; ++ox)
    {
	const Maps::Tiles & tile = world.GetTiles(rectMaps.x + ox, rectMaps.y + oy);

	if(tile.GetObject() == MP2::OBJ_HEROES && (flag & LEVEL_HEROES))
	{
	    const Heroes *hero = world.GetHeroes(tile.GetIndex());
	    if(hero) hero->Redraw(dst, rectMapsPosition.x + TILEWIDTH * ox, rectMapsPosition.y + TILEWIDTH * oy, true);
	}
    }

    // route
    const Game::Focus & focus = Game::Focus::Get();

    if((flag & LEVEL_HEROES) &&
	Game::Focus::HEROES == focus.Type() &&
	focus.GetHeroes().GetPath().isShow())
    {
	const Heroes & hero = focus.GetHeroes();
	s32 from = hero.GetIndex();
	s16 green = hero.GetPath().GetAllowStep();

	const bool skipfirst = hero.isEnableMove() && 45 > hero.GetSpriteIndex() && 2 < (hero.GetSpriteIndex() % 9);

	Route::Path::const_iterator it1 = hero.GetPath().begin();
	Route::Path::const_iterator it2 = hero.GetPath().end();
	Route::Path::const_iterator it3 = it1;

	for(; it1 != it2; ++it1)
	{
	    from = Maps::GetDirectionIndex(from, (*it1).Direction());
    	    const Point mp(from % world.w(), from / world.h());

	    ++it3;
	    --green;

            if(!(Rect(rectMaps.x + rt.x, rectMaps.y + rt.y, rt.w, rt.h) & mp)) continue;
	    if(it1 == hero.GetPath().begin() && skipfirst) continue;

	    const u16 index = (it3 == it2 ? 0 :
		    Route::Path::GetIndexSprite((*it1).Direction(), (*it3).Direction(), 
			Maps::Ground::GetBasePenalty(from, hero.GetLevelSkill(Skill::Secondary::PATHFINDING))));

	    const Sprite & sprite = AGG::GetICN(0 > green ? ICN::ROUTERED : ICN::ROUTE, index);
	    BlitOnTile(dst, sprite, sprite.x() - 14, sprite.y(), mp);
	}
    }

    if(IS_DEVEL())
    {
	// redraw grid
	if(flag & LEVEL_ALL)
	{
	    for(s16 oy = rt.y; oy < rt.y + rt.h; ++oy)
		for(s16 ox = rt.x; ox < rt.x + rt.w; ++ox)
	    {
    		const Point dstpt(rectMapsPosition.x + TILEWIDTH * ox,
				rectMapsPosition.y + TILEWIDTH * oy);
		if(areaPosition & dstpt)
    		{
		    dst.Lock();
    		    dst.SetPixel(dstpt.x, dstpt.y, dst.GetColor(0x40));
    		    dst.Unlock();
		}
	    }
	}
    }
    else
    // redraw fog
    if(flag & LEVEL_FOG)
	for(s16 oy = rt.y; oy < rt.y + rt.h; ++oy)
	    for(s16 ox = rt.x; ox < rt.x + rt.w; ++ox)
    {
	const Maps::Tiles & tile = world.GetTiles(rectMaps.x + ox, rectMaps.y + oy);
	if(tile.isFog(Settings::Get().MyColor()))
	    tile.RedrawFogs(dst, Settings::Get().MyColor());
    }
}

/* scroll area */
void Interface::GameArea::Scroll(void)
{
    if(scrollDirection & SCROLL_LEFT)
    {
	if(0 < scrollOffset.x)
	    scrollOffset.x -= scrollStep;
	else
	if(0 < rectMaps.x)
	{
	    scrollOffset.x = SCROLL_MAX - scrollStep;
	    --rectMaps.x;
	}
    }
    else
    if(scrollDirection & SCROLL_RIGHT)
    {
	if(scrollOffset.x < SCROLL_MAX * 2)
	    scrollOffset.x += scrollStep;
	else
	if(world.w() - rectMaps.w > rectMaps.x)
	{
	    scrollOffset.x = SCROLL_MAX + scrollStep;
	    ++rectMaps.x;
	}
    }

    if(scrollDirection & SCROLL_TOP)
    {
	if(0 < scrollOffset.y)
	    scrollOffset.y -= scrollStep;
	else
	if(0 < rectMaps.y)
	{
	    scrollOffset.y = SCROLL_MAX - scrollStep;
	    --rectMaps.y;
	}
    }
    else
    if(scrollDirection & SCROLL_BOTTOM)
    {
	if(scrollOffset.y < SCROLL_MAX * 2)
	    scrollOffset.y += scrollStep;
	else
	if(world.h() - rectMaps.h > rectMaps.y)
	{
	    scrollOffset.y = SCROLL_MAX + scrollStep;
	    ++rectMaps.y;
	}
    }

    rectMapsPosition.x = areaPosition.x - scrollOffset.x;
    rectMapsPosition.y = areaPosition.y - scrollOffset.y;

    scrollDirection = 0;
}

/* scroll area to center point maps */
void Interface::GameArea::Center(const Point &pt)
{
    Center(pt.x, pt.y);
}

void Interface::GameArea::Center(s16 px, s16 py)
{
    Point pos(px - rectMaps.w / 2, py - rectMaps.h / 2);

    // our of range
    if(pos.x < 0) pos.x = 0;
    else
    if(pos.x > world.w() - rectMaps.w) pos.x = world.w() - rectMaps.w;

    if(pos.y < 0) pos.y = 0;
    else
    if(pos.y > world.h() - rectMaps.h) pos.y = world.h() - rectMaps.h;

    if(pos.x == rectMaps.x && pos.y == rectMaps.y) return;

    // possible fast scroll
    if(pos.y == rectMaps.y && 1 == (pos.x - rectMaps.x)) scrollDirection |= SCROLL_RIGHT;
    else
    if(pos.y == rectMaps.y && -1 == (pos.x - rectMaps.x)) scrollDirection |= SCROLL_LEFT;
    else
    if(pos.x == rectMaps.x && 1 == (pos.y - rectMaps.y)) scrollDirection |= SCROLL_BOTTOM;
    else
    if(pos.x == rectMaps.x && -1 == (pos.y - rectMaps.y)) scrollDirection |= SCROLL_TOP;
    else
    // diagonal
    if(-1 == (pos.y - rectMaps.y) && 1 == (pos.x - rectMaps.x))
    {
	scrollDirection |= SCROLL_TOP | SCROLL_RIGHT;
    }
    else
    if(-1 == (pos.y - rectMaps.y) && -1 == (pos.x - rectMaps.x))
    {
	scrollDirection |= SCROLL_TOP | SCROLL_LEFT;
    }
    else
    if(1 == (pos.y - rectMaps.y) && 1 == (pos.x - rectMaps.x))
    {
	scrollDirection |= SCROLL_BOTTOM | SCROLL_RIGHT;
    }
    else
    if(1 == (pos.y - rectMaps.y) && -1 == (pos.x - rectMaps.x))
    {
	scrollDirection |= SCROLL_BOTTOM | SCROLL_LEFT;
    }

    else
    {
	rectMaps.x = pos.x;
	rectMaps.y = pos.y;
	scrollDirection = 0;

	if(pos.x == 0) scrollOffset.x = 0;
	else
	if(pos.x == world.w() - rectMaps.w)
	    scrollOffset.x = SCROLL_MAX * 2;
	else
	    scrollOffset.x = SCROLL_MAX + TILEWIDTH / 2;

	if(pos.y == 0) scrollOffset.y = 0;
	else
	if(pos.y == world.h() - rectMaps.h)
	    scrollOffset.y = SCROLL_MAX * 2;
	else
	    scrollOffset.y = SCROLL_MAX + TILEWIDTH / 2;

	rectMapsPosition.x = areaPosition.x - scrollOffset.x;
	rectMapsPosition.y = areaPosition.y - scrollOffset.y;

	scrollStep = Settings::Get().ScrollSpeed();
    }

    if(scrollDirection) Scroll();
}

void Interface::GameArea::GenerateUltimateArtifactAreaSurface(const s32 index, Surface & sf)
{
    if(Interface::NoGUI()) return;

    if(Maps::isValidAbsIndex(index))
    {
	sf.Set(448, 448);

	GameArea & gamearea = Get();
	const Rect origPosition(gamearea.areaPosition);
	gamearea.SetAreaPosition(0, 0, sf.w(), sf.h());

	const Rect & rectMaps = gamearea.GetRectMaps();
	const Rect & areaPosition = gamearea.GetArea();
	Point pt(index % world.w(), index / world.h());

        gamearea.Center(pt);
	gamearea.Redraw(sf, LEVEL_BOTTOM | LEVEL_TOP);

	// blit marker
	for(u8 ii = 0; ii < rectMaps.h; ++ii) if(index < Maps::GetIndexFromAbsPoint(rectMaps.x + rectMaps.w - 1, rectMaps.y + ii))
	{
	    pt.y = ii;
	    break;
	}
	for(u8 ii = 0; ii < rectMaps.w; ++ii) if(index == Maps::GetIndexFromAbsPoint(rectMaps.x + ii, rectMaps.y + pt.y))
	{
	    pt.x = ii;
	    break;
	}
	const Sprite & marker = AGG::GetICN(ICN::ROUTE, 0);
	const Point dst(areaPosition.x + pt.x * TILEWIDTH, areaPosition.y + pt.y * TILEWIDTH);
	sf.Blit(marker, dst.x, dst.y + 8);

	Settings::Get().EvilInterface() ? sf.GrayScale() : sf.Sepia();

	if(Settings::Get().QVGA())
	{
    	    Surface sf2;
    	    Surface::ScaleMinifyByTwo(sf2, sf);
    	    Surface::Swap(sf2, sf);
	}

	gamearea.SetAreaPosition(origPosition.x, origPosition.y, origPosition.w, origPosition.h);
    }
    else
    DEBUG(DBG_ENGINE , DBG_WARN, "Interface::GameArea::GenerateUltimateArtifactAreaSurface: artifact not found");
}

bool Interface::GameArea::NeedScroll(void) const
{
    return scrollDirection;
}

Cursor::themes_t Interface::GameArea::GetScrollCursor(void) const
{
    switch(scrollDirection)
    {
	case SCROLL_TOP:		  return Cursor::SCROLL_TOP;
	case SCROLL_BOTTOM:		  return Cursor::SCROLL_BOTTOM;
	case SCROLL_RIGHT:		  return Cursor::SCROLL_RIGHT;
	case SCROLL_LEFT:		  return Cursor::SCROLL_LEFT;
	case SCROLL_LEFT | SCROLL_TOP:	  return Cursor::SCROLL_TOPLEFT;
	case SCROLL_LEFT | SCROLL_BOTTOM: return Cursor::SCROLL_BOTTOMLEFT;
	case SCROLL_RIGHT | SCROLL_TOP:	  return Cursor::SCROLL_TOPRIGHT;
	case SCROLL_RIGHT | SCROLL_BOTTOM:return Cursor::SCROLL_BOTTOMRIGHT;

	default: break;
    }

    return Cursor::NONE;
}

void Interface::GameArea::SetScroll(scroll_t direct)
{
    switch(direct)
    {
	case SCROLL_LEFT:
	    if(0 < rectMaps.x || 0 < scrollOffset.x)
	    {
	    	scrollDirection |= direct;
		updateCursor = true;
	    }
	    break;

	case SCROLL_RIGHT:
	    if(world.w() - rectMaps.w > rectMaps.x || SCROLL_MAX * 2 > scrollOffset.x)
	    {
		scrollDirection |= direct;
		updateCursor = true;
	    }
	    break;

	case SCROLL_TOP:
	    if(0 < rectMaps.y || 0 < scrollOffset.y)
	    {
		scrollDirection |= direct;
		updateCursor = true;
	    }
	    break;

	case SCROLL_BOTTOM:
	    if(world.h() - rectMaps.h > rectMaps.y || SCROLL_MAX * 2 > scrollOffset.y)
	    {
		scrollDirection |= direct;
		updateCursor = true;
	    }
	    break;

	default: break;
    }

    scrollTime.Start();
}

/* convert area point to index maps */
s32 Interface::GameArea::GetIndexFromMousePoint(const Point & pt) const
{
    s32 result = (rectMaps.y + (pt.y - rectMapsPosition.y) / TILEWIDTH) * world.w() +
		    rectMaps.x + (pt.x - rectMapsPosition.x) / TILEWIDTH;
    const s32 & max = world.w() * world.h() - 1;

    return result > max || result < Maps::GetIndexFromAbsPoint(rectMaps.x, rectMaps.y) ? -1 : result;
}

void Interface::GameArea::SetUpdateCursor(void)
{
    updateCursor = true;
}

void Interface::GameArea::QueueEventProcessing(void)
{
    const Settings & conf = Settings::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();
    const Point & mp = le.GetMouseCursor();

    s32 index = GetIndexFromMousePoint(mp);
    
    // out of range
    if(index < 0) return;

    const Rect tile_pos(rectMapsPosition.x + ((u16) (mp.x - rectMapsPosition.x) / TILEWIDTH) * TILEWIDTH,
	                rectMapsPosition.y + ((u16) (mp.y - rectMapsPosition.y) / TILEWIDTH) * TILEWIDTH,
	                TILEWIDTH, TILEWIDTH);

    // change cusor if need
    if(updateCursor || index != oldIndexPos)
    {
	cursor.SetThemes(Game::GetCursor(index));
	oldIndexPos = index;
	updateCursor = false;
    }

    // fixed pocket pc tap mode
    if(conf.HideInterface() && conf.ShowControlPanel() && le.MouseCursor(Interface::ControlPanel::Get().GetArea())) return;

    // fixed pocket pc: click on maps after scroll (pause: ~800 ms)
    if(conf.ExtTapMode())
    {
	scrollTime.Stop();
	if(800 > scrollTime.Get()) return;
    }

    if(le.MouseClickLeft(tile_pos) && Cursor::POINTER != cursor.Themes())
        Game::MouseCursorAreaClickLeft(index);
    else
    if(le.MousePressRight(tile_pos))
        Game::MouseCursorAreaPressRight(index);
}

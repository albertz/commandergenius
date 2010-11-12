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

#include "world.h"
#include "agg.h"
#include "cursor.h"
#include "settings.h"
#include "game_interface.h"
#include "kingdom.h"
#include "maps_tiles.h"
#include "castle.h"
#include "direction.h"
#include "heroes.h"

bool ReflectSprite(const u16 from);
void PlayWalkSound(Maps::Ground::ground_t ground);
const Sprite & SpriteHero(const Heroes & hero, const u8 index, const bool reflect, const bool rotate = false);
const Sprite & SpriteFlag(const Heroes & hero, const u8 index, const bool reflect, const bool rotate = false);
const Sprite & SpriteShad(const Heroes & hero, const u8 index);
bool isNeedStayFrontObject(const Heroes & hero, const Maps::Tiles & next);

void PlayWalkSound(Maps::Ground::ground_t ground)
{
    M82::m82_t wav = M82::UNKNOWN;

    const u8 speed = (4 > Settings::Get().HeroesMoveSpeed() ? 1 : (7 > Settings::Get().HeroesMoveSpeed() ? 2 : 3));

    // play sound
    switch(ground)
    {
    	case Maps::Ground::WATER:       wav = (1 == speed ? M82::WSND00 : (2 == speed ? M82::WSND10 : M82::WSND20)); break;
    	case Maps::Ground::GRASS:       wav = (1 == speed ? M82::WSND01 : (2 == speed ? M82::WSND11 : M82::WSND21)); break;
    	case Maps::Ground::WASTELAND:   wav = (1 == speed ? M82::WSND02 : (2 == speed ? M82::WSND12 : M82::WSND22)); break;
    	case Maps::Ground::SWAMP:
    	case Maps::Ground::BEACH:       wav = (1 == speed ? M82::WSND03 : (2 == speed ? M82::WSND13 : M82::WSND23)); break;
    	case Maps::Ground::LAVA:        wav = (1 == speed ? M82::WSND04 : (2 == speed ? M82::WSND14 : M82::WSND24)); break;
    	case Maps::Ground::DESERT:
    	case Maps::Ground::SNOW:        wav = (1 == speed ? M82::WSND05 : (2 == speed ? M82::WSND15 : M82::WSND25)); break;
    	case Maps::Ground::DIRT:        wav = (1 == speed ? M82::WSND06 : (2 == speed ? M82::WSND16 : M82::WSND26)); break;

    	default: break;
    }

    if(wav != M82::UNKNOWN) AGG::PlaySound(wav);
}

bool ReflectSprite(const u16 from)
{
    switch(from)
    {
        case Direction::BOTTOM_LEFT:
        case Direction::LEFT:
        case Direction::TOP_LEFT:		return true;

        default: break;
    }

    return false;
}

const Sprite & SpriteHero(const Heroes & hero, const u8 index, const bool reflect, const bool rotate)
{
    ICN::icn_t icn_hero = ICN::UNKNOWN;
    u16 index_sprite = 0;

    if(hero.isShipMaster()) icn_hero = ICN::BOAT32;
    else
    switch(hero.GetRace())
    {
        case Race::KNGT: icn_hero = ICN::KNGT32; break;
        case Race::BARB: icn_hero = ICN::BARB32; break;
        case Race::SORC: icn_hero = ICN::SORC32; break;
        case Race::WRLK: icn_hero = ICN::WRLK32; break;
        case Race::WZRD: icn_hero = ICN::WZRD32; break;
        case Race::NECR: icn_hero = ICN::NECR32; break;

        default: DEBUG(DBG_GAME , DBG_WARN, "Heroes::SpriteHero: unknown race"); break;
    }

    if(rotate)				index_sprite = 45;
    else
    switch(hero.GetDirection())
    {
        case Direction::TOP:            index_sprite =  0; break;
        case Direction::TOP_RIGHT:      index_sprite =  9; break;
        case Direction::RIGHT:          index_sprite = 18; break;
        case Direction::BOTTOM_RIGHT:   index_sprite = 27; break;
        case Direction::BOTTOM:         index_sprite = 36; break;
        case Direction::BOTTOM_LEFT:    index_sprite = 27; break;
        case Direction::LEFT:           index_sprite = 18; break;
        case Direction::TOP_LEFT:       index_sprite =  9; break;

        default: DEBUG(DBG_GAME , DBG_WARN, "Heroes::SpriteHero: unknown direction"); break;
    }

    return AGG::GetICN(icn_hero, index_sprite + (index % 9), reflect);
}

const Sprite & SpriteFlag(const Heroes & hero, const u8 index, const bool reflect, const bool rotate)
{
    ICN::icn_t icn_flag = ICN::UNKNOWN;
    u16 index_sprite = 0;

    switch(hero.GetColor())
    {
        case Color::BLUE:       icn_flag = hero.isShipMaster() ? ICN::B_BFLG32 : ICN::B_FLAG32; break;
        case Color::GREEN:      icn_flag = hero.isShipMaster() ? ICN::G_BFLG32 : ICN::G_FLAG32; break;
        case Color::RED:        icn_flag = hero.isShipMaster() ? ICN::R_BFLG32 : ICN::R_FLAG32; break;
        case Color::YELLOW:     icn_flag = hero.isShipMaster() ? ICN::Y_BFLG32 : ICN::Y_FLAG32; break;
        case Color::ORANGE:     icn_flag = hero.isShipMaster() ? ICN::O_BFLG32 : ICN::O_FLAG32; break;
        case Color::PURPLE:     icn_flag = hero.isShipMaster() ? ICN::P_BFLG32 : ICN::P_FLAG32; break;

        default: DEBUG(DBG_GAME , DBG_WARN, "Heroes::SpriteFlag: unknown color hero"); break;
    }

    if(rotate)				index_sprite = 45;
    else
    switch(hero.GetDirection())
    {
        case Direction::TOP:            index_sprite =  0; break;
        case Direction::TOP_RIGHT:      index_sprite =  9; break;
        case Direction::RIGHT:          index_sprite = 18; break;
        case Direction::BOTTOM_RIGHT:   index_sprite = 27; break;
        case Direction::BOTTOM:         index_sprite = 36; break;
        case Direction::BOTTOM_LEFT:    index_sprite = 27; break;
        case Direction::LEFT:           index_sprite = 18; break;
        case Direction::TOP_LEFT:       index_sprite =  9; break;

        default: DEBUG(DBG_GAME , DBG_WARN, "Heroes::SpriteFlag: unknown direction"); break;
    }

    return AGG::GetICN(icn_flag, index_sprite + (index % 9), reflect);
}

const Sprite & SpriteShad(const Heroes & hero, const u8 index)
{
    const ICN::icn_t icn_shad = hero.isShipMaster() ? ICN::BOATSHAD : ICN::SHADOW32;
    u16 index_sprite = 0;

    switch(hero.GetDirection())
    {
        case Direction::TOP:            index_sprite =  0; break;
        case Direction::TOP_RIGHT:      index_sprite =  9; break;
        case Direction::RIGHT:          index_sprite = 18; break;
        case Direction::BOTTOM_RIGHT:   index_sprite = 27; break;
        case Direction::BOTTOM:         index_sprite = 36; break;
        case Direction::BOTTOM_LEFT:    index_sprite = 45; break;
        case Direction::LEFT:           index_sprite = 54; break;
        case Direction::TOP_LEFT:       index_sprite = 63; break;

        default: DEBUG(DBG_GAME , DBG_WARN, "Heroes::SpriteShadow: unknown direction"); break;
    }

    return AGG::GetICN(icn_shad, index_sprite + (index % 9));
}

const Sprite & SpriteFroth(const Heroes & hero, const u8 index, const bool reflect)
{
    u16 index_sprite = 0;

    switch(hero.GetDirection())
    {
        case Direction::TOP:            index_sprite =  0; break;
        case Direction::TOP_RIGHT:      index_sprite =  9; break;
        case Direction::RIGHT:          index_sprite = 18; break;
        case Direction::BOTTOM_RIGHT:   index_sprite = 27; break;
        case Direction::BOTTOM:         index_sprite = 36; break;
        case Direction::BOTTOM_LEFT:    index_sprite = 27; break;
        case Direction::LEFT:           index_sprite = 18; break;
        case Direction::TOP_LEFT:       index_sprite =  9; break;

        default: DEBUG(DBG_GAME , DBG_WARN, "Heroes::SpriteFroth: unknown direction"); break;
    }

    return AGG::GetICN(ICN::FROTH, index_sprite + (index % 9), reflect);
}

bool isNeedStayFrontObject(const Heroes & hero, const Maps::Tiles & next)
{
    if(hero.isShipMaster())
        switch(next.GetObject())
        {
            case MP2::OBJ_WATERCHEST:
            case MP2::OBJ_SHIPWRECKSURVIROR:
            case MP2::OBJ_FLOTSAM:
            case MP2::OBJ_BOTTLE:
            case MP2::OBJ_COAST:
            case MP2::OBJ_HEROES:
            return true;

            default: break;
        }
    else
        switch(next.GetObject())
        {
            case MP2::OBJ_TREASURECHEST:
            case MP2::OBJ_ANCIENTLAMP:
            case MP2::OBJ_CAMPFIRE:
            case MP2::OBJ_MONSTER:
            case MP2::OBJ_RESOURCE:
            case MP2::OBJ_ARTIFACT:
            case MP2::OBJ_HEROES:
            case MP2::OBJ_BOAT:
	    case MP2::OBJ_BARRIER:
            case MP2::OBJ_JAIL:
            return true;

            case MP2::OBJ_CASTLE:
	    {
		const Castle *castle = world.GetCastle(next.GetIndex());

		return (castle &&
			hero.GetColor() != castle->GetColor() &&
		        !Settings::Get().IsUnions(hero.GetColor(), castle->GetColor()));
	    }
	    break;

            default: break;
        }

    return false;
}

void Heroes::Redraw(Surface & dst, bool with_shadow) const
{
    const Point & mp = GetCenter();
    const Interface::GameArea & gamearea = Interface::GameArea::Get();
    s16 dx = gamearea.GetMapsPos().x + TILEWIDTH * (mp.x - gamearea.GetRectMaps().x);
    s16 dy = gamearea.GetMapsPos().y + TILEWIDTH * (mp.y - gamearea.GetRectMaps().y);

    Redraw(dst, dx, dy, with_shadow);
}

void Heroes::Redraw(Surface & dst, const s16 dx, const s16 dy, bool with_shadow) const
{
    const Point & mp = GetCenter();
    const Interface::GameArea & gamearea = Interface::GameArea::Get();
    if(!(gamearea.GetRectMaps() & mp)) return;

    bool reflect = ReflectSprite(direction);

    const Sprite & sprite1 = SpriteHero(*this, sprite_index, reflect);
    const Sprite & sprite2 = SpriteFlag(*this, sprite_index, reflect);
    const Sprite & sprite3 = SpriteShad(*this, sprite_index);
    const Sprite & sprite4 = SpriteFroth(*this, sprite_index, reflect);

    Point dst_pt1(dx + (reflect ? TILEWIDTH - sprite1.x() - sprite1.w() : sprite1.x()), dy + sprite1.y() + TILEWIDTH);
    Point dst_pt2(dx + (reflect ? TILEWIDTH - sprite2.x() - sprite2.w() : sprite2.x()), dy + sprite2.y() + TILEWIDTH);
    Point dst_pt3(dx + sprite3.x(), dy + sprite3.y() + TILEWIDTH);
    Point dst_pt4(dx + (reflect ? TILEWIDTH - sprite4.x() - sprite4.w() : sprite4.x()), dy + sprite4.y() + TILEWIDTH);

    // apply offset
    if(sprite_index < 45)
    {
	s16 ox = 0;
	s16 oy = 0;
	const u8 frame = (sprite_index % 9);

	switch(direction)
	{
    	    case Direction::TOP:            oy = -4 * frame; break;
    	    case Direction::TOP_RIGHT:      ox = 4 * frame; oy = -4 * frame; break;
    	    case Direction::TOP_LEFT:       ox = -4 * frame; oy = -4 * frame; break;
    	    case Direction::BOTTOM_RIGHT:   ox = 4 * frame; oy = 4 * frame; break;
    	    case Direction::BOTTOM:         oy = 4 * frame; break;
    	    case Direction::BOTTOM_LEFT:    ox = -4 * frame; oy = 4 * frame; break;
    	    case Direction::RIGHT:          ox = 4 * frame; break;
    	    case Direction::LEFT:           ox = -4 * frame; break;
    	    default: break;
	}

	dst_pt1.x += ox;
	dst_pt1.y += oy;
	dst_pt2.x += ox;
	dst_pt2.y += oy;
	dst_pt3.x += ox;
	dst_pt3.y += oy;
	dst_pt4.x += ox;
	dst_pt4.y += oy;
    }

    Rect src_rt;


    if(isShipMaster())
    {
	gamearea.SrcRectFixed(src_rt, dst_pt4, sprite4.w(), sprite4.h());
	dst.Blit(sprite4, src_rt, dst_pt4);
    }

    // redraw sprites for shadow
    if(with_shadow)
    {
	gamearea.SrcRectFixed(src_rt, dst_pt3, sprite3.w(), sprite3.h());
	dst.Blit(sprite3, src_rt, dst_pt3);
    }

    // redraw sprites hero and flag
    gamearea.SrcRectFixed(src_rt, dst_pt1, sprite1.w(), sprite1.h());
    dst.Blit(sprite1, src_rt, dst_pt1);

    gamearea.SrcRectFixed(src_rt, dst_pt2, sprite2.w(), sprite2.h());
    dst.Blit(sprite2, src_rt, dst_pt2);

    // redraw dependences tiles
    const s32 center = GetIndex();
    bool skip_ground = MP2::isActionObject(save_maps_object, isShipMaster());

    world.GetTiles(center).RedrawTop(dst);

    if(Maps::isValidDirection(center, Direction::TOP))
	world.GetTiles(Maps::GetDirectionIndex(center, Direction::TOP)).RedrawTop4Hero(dst, skip_ground);

    if(Maps::isValidDirection(center, Direction::BOTTOM))
    {
	Maps::Tiles & tile_bottom = world.GetTiles(Maps::GetDirectionIndex(center, Direction::BOTTOM));
	tile_bottom.RedrawBottom4Hero(dst);
	tile_bottom.RedrawTop(dst);
    }

    if(45 > GetSpriteIndex() &&
	Direction::BOTTOM != direction &&
	Direction::TOP != direction &&
	Maps::isValidDirection(center, direction))
    {
	if(Maps::isValidDirection(Maps::GetDirectionIndex(center, direction), Direction::BOTTOM))
	{
	    Maps::Tiles & tile_dir_bottom = world.GetTiles(Maps::GetDirectionIndex(Maps::GetDirectionIndex(center, direction), Direction::BOTTOM));
    	    tile_dir_bottom.RedrawBottom4Hero(dst);
	    tile_dir_bottom.RedrawTop(dst);
	}
	if(Maps::isValidDirection(Maps::GetDirectionIndex(center, direction), Direction::TOP))
	{
	    Maps::Tiles & tile_dir_top = world.GetTiles(Maps::GetDirectionIndex(Maps::GetDirectionIndex(center, direction), Direction::TOP));
	    tile_dir_top.RedrawTop4Hero(dst, skip_ground);
	}
    }

    if(Maps::isValidDirection(center, direction))
    {
	if(Direction::TOP == direction)
	    world.GetTiles(Maps::GetDirectionIndex(center, direction)).RedrawTop4Hero(dst, skip_ground);
	else
	    world.GetTiles(Maps::GetDirectionIndex(center, direction)).RedrawTop(dst);
    }
}


bool Heroes::MoveStep(bool fast)
{
    s32 index_from = GetIndex();
    s32 index_to = Maps::GetDirectionIndex(index_from, path.GetFrontDirection());
    s32 index_dst = path.GetDestinationIndex();
    const Point & mp = GetCenter();

    if(fast)
    {
	if(index_to == index_dst && isNeedStayFrontObject(*this, world.GetTiles(index_to)))
	{
	    ApplyPenaltyMovement();
	    path.Reset();
	    Action(index_to);
	    SetMove(false);
	}
	else
	{
	    Maps::Tiles & tiles_from = world.GetTiles(index_from);
	    Maps::Tiles & tiles_to = world.GetTiles(index_to);

	    if(MP2::OBJ_HEROES != save_maps_object) tiles_from.SetObject(save_maps_object);

	    SetIndex(index_to);
	    save_maps_object = tiles_to.GetObject();
	    tiles_to.SetObject(MP2::OBJ_HEROES);
	    Scoute();
	    ApplyPenaltyMovement();
	    path.PopFront();

	    if(MP2::OBJ_EVENT == save_maps_object)
	    {
		Action(index_to);
		SetMove(false);
	    }

	    // check protection tile
	    ActionNewPosition();

	    // possible hero is die
	    if(!isFreeman())
	    {
		if(index_to == index_dst)
		{
		    path.Reset();
		    Action(index_to);
		    SetMove(false);
		}
	    }
	}
	return true;
    }
    else
    if(0 == sprite_index % 9)
    {
	if(index_to == index_dst && isNeedStayFrontObject(*this, world.GetTiles(index_to)))
	{
	    ApplyPenaltyMovement();
	    path.Reset();
	    Action(index_to);
	    SetMove(false);
	    return true;
	}
	else
	{
	    if(Game::LOCAL == world.GetKingdom(GetColor()).Control())
	    // play sound
	    PlayWalkSound(world.GetTiles(mp).GetGround());
	}
    }
    else
    if(8 == sprite_index % 9)
    {
	Maps::Tiles & tiles_from = world.GetTiles(index_from);
	Maps::Tiles & tiles_to = world.GetTiles(index_to);

	if(MP2::OBJ_HEROES != save_maps_object) tiles_from.SetObject(save_maps_object);

	SetIndex(index_to);
	save_maps_object = tiles_to.GetObject();
	tiles_to.SetObject(MP2::OBJ_HEROES);
	Scoute();
	ApplyPenaltyMovement();
	sprite_index -= 8;
	path.PopFront();

	if(MP2::OBJ_EVENT == save_maps_object)
	{
	    Action(index_to);
	    SetMove(false);
	}

	// check protection tile
	ActionNewPosition();

	// possible hero is die
	if(!isFreeman())
	{
	    if(index_to == index_dst)
	    {
		path.Reset();
		Action(index_to);
		SetMove(false);
	    }
	}

	return true;
    }

    ++sprite_index;

    return false;
}

void Heroes::AngleStep(const Direction::vector_t to_direct)
{
    //bool check = false;
    bool clockwise = ShortDistanceClockWise(direction, to_direct);

    // start index
    if(45 > sprite_index && 0 == sprite_index % 9)
    {
	switch(direction)
	{
    	    case Direction::TOP:		sprite_index = 45; break;
    	    case Direction::TOP_RIGHT:		sprite_index = clockwise ? 47 : 46; break;
    	    case Direction::TOP_LEFT:		sprite_index = clockwise ? 46 : 47; break;
    	    case Direction::RIGHT:		sprite_index = clockwise ? 49 : 48; break;
    	    case Direction::LEFT:		sprite_index = clockwise ? 48 : 49; break;
    	    case Direction::BOTTOM_RIGHT:	sprite_index = clockwise ? 51 : 50; break;
    	    case Direction::BOTTOM_LEFT:	sprite_index = clockwise ? 50 : 51; break;
    	    case Direction::BOTTOM:		sprite_index = clockwise ? 52 : 53; break;

	    default: break;
	}
    }
    // animation process
    else
    {
	switch(direction)
	{
    	    case Direction::TOP_RIGHT:
    	    case Direction::RIGHT:
    	    case Direction::BOTTOM_RIGHT:
		clockwise ? ++sprite_index : --sprite_index;
		break;

    	    case Direction::TOP:
		++sprite_index;
		break;

    	    case Direction::TOP_LEFT:
    	    case Direction::LEFT:
    	    case Direction::BOTTOM_LEFT:
		clockwise ? --sprite_index : ++sprite_index;
		break;

    	    case Direction::BOTTOM:
		--sprite_index;
		break;

	    default: break;
	}

	bool end = false;

	Direction::vector_t next = Direction::UNKNOWN;

	switch(direction)
	{
    	    case Direction::TOP:		next = clockwise ? Direction::TOP_RIGHT : Direction::TOP_LEFT; break;
    	    case Direction::TOP_RIGHT:		next = clockwise ? Direction::RIGHT : Direction::TOP; break;
    	    case Direction::TOP_LEFT:		next = clockwise ? Direction::TOP : Direction::LEFT; break;
    	    case Direction::RIGHT:		next = clockwise ? Direction::BOTTOM_RIGHT : Direction::TOP_RIGHT; break;
    	    case Direction::LEFT:		next = clockwise ? Direction::TOP_LEFT : Direction::BOTTOM_LEFT; break;
    	    case Direction::BOTTOM_RIGHT:	next = clockwise ? Direction::BOTTOM : Direction::RIGHT; break;
    	    case Direction::BOTTOM_LEFT:	next = clockwise ? Direction::LEFT : Direction::BOTTOM; break;
    	    case Direction::BOTTOM:		next = clockwise ? Direction::BOTTOM_LEFT : Direction::BOTTOM_RIGHT; break;

	    default: break;
	}

	switch(next)
	{
    	    case Direction::TOP:		end = (sprite_index == 44); break;
    	    case Direction::TOP_RIGHT:		end = (sprite_index == (clockwise ? 47 : 46)); break;
    	    case Direction::TOP_LEFT:		end = (sprite_index == (clockwise ? 46 : 47)); break;
    	    case Direction::RIGHT:		end = (sprite_index == (clockwise ? 49 : 48)); break;
    	    case Direction::LEFT:		end = (sprite_index == (clockwise ? 48 : 49)); break;
    	    case Direction::BOTTOM_RIGHT:	end = (sprite_index == (clockwise ? 51 : 50)); break;
    	    case Direction::BOTTOM_LEFT:	end = (sprite_index == (clockwise ? 50 : 51)); break;
    	    case Direction::BOTTOM:		end = (sprite_index == 53); break;

	    default: break;
	}

	if(end)
	{
	    switch(next)
	    {
    		case Direction::TOP:            sprite_index = 0; break;
    		case Direction::BOTTOM:         sprite_index = 36; break;
    		case Direction::TOP_RIGHT:
    		case Direction::TOP_LEFT:       sprite_index = 9; break;
    		case Direction::BOTTOM_RIGHT:
    		case Direction::BOTTOM_LEFT:    sprite_index = 27; break;
    		case Direction::RIGHT:
    		case Direction::LEFT:           sprite_index = 18; break;

		default: break;
	    }

	    direction = next;
	}
    }
}

void Heroes::FadeOut(void) const
{
    const Point & mp = GetCenter();
    const Interface::GameArea & gamearea = Interface::GameArea::Get();

    if(!(gamearea.GetRectMaps() & mp)) return;

    Display & display = Display::Get();

    bool reflect = ReflectSprite(direction);

    s16 dx = gamearea.GetMapsPos().x + TILEWIDTH * (mp.x - gamearea.GetRectMaps().x);
    s16 dy = gamearea.GetMapsPos().y + TILEWIDTH * (mp.y - gamearea.GetRectMaps().y);

    const Sprite & sprite1 = SpriteHero(*this, sprite_index, reflect);

    Surface sf(sprite1.w(), sprite1.h());
    sf.SetColorKey();
    sf.Blit(sprite1);

    Point dst_pt1(dx + (reflect ? TILEWIDTH - sprite1.x() - sprite1.w() : sprite1.x()), dy + sprite1.y() + TILEWIDTH);
    Rect src_rt;

    gamearea.SrcRectFixed(src_rt, dst_pt1, sprite1.w(), sprite1.h());

    LocalEvent & le = LocalEvent::Get();
    u8 alpha = 250;

    while(le.HandleEvents() && alpha > 0)
    {
        if(Game::AnimateInfrequent(Game::HEROES_FADE_DELAY))
        {
            Cursor::Get().Hide();

	    for(s16 y = mp.y - 1; y <= mp.y + 1; ++y)
		for(s16 x = mp.x - 1; x <= mp.x + 1; ++x)
    		    if(Maps::isValidAbsPoint(x, y))
	    {
        	const Maps::Tiles & tile = world.GetTiles(Maps::GetIndexFromAbsPoint(x, y));

        	tile.RedrawTile(display);
        	tile.RedrawBottom(display);
        	tile.RedrawObjects(display);
    	    }

    	    sf.SetAlpha(alpha);
    	    display.Blit(sf, src_rt, dst_pt1);

	    for(s16 y = mp.y - 1; y <= mp.y + 1; ++y)
		for(s16 x = mp.x - 1; x <= mp.x + 1; ++x)
    		    if(Maps::isValidAbsPoint(x, y))
	    {
        	const Maps::Tiles & tile = world.GetTiles(Maps::GetIndexFromAbsPoint(x, y));

        	tile.RedrawTop(display);
    	    }

	    Cursor::Get().Show();
	    display.Flip();
            alpha -= 10;
        }
    }
}

void Heroes::FadeIn(void) const
{
    const Point & mp = GetCenter();
    const Interface::GameArea & gamearea = Interface::GameArea::Get();

    if(!(gamearea.GetRectMaps() & mp)) return;

    Display & display = Display::Get();

    bool reflect = ReflectSprite(direction);

    s16 dx = gamearea.GetMapsPos().x + TILEWIDTH * (mp.x - gamearea.GetRectMaps().x);
    s16 dy = gamearea.GetMapsPos().y + TILEWIDTH * (mp.y - gamearea.GetRectMaps().y);

    const Sprite & sprite1 = SpriteHero(*this, sprite_index, reflect);

    Surface sf(sprite1.w(), sprite1.h());
    sf.SetColorKey();
    sf.Blit(sprite1);

    Point dst_pt1(dx + (reflect ? TILEWIDTH - sprite1.x() - sprite1.w() : sprite1.x()), dy + sprite1.y() + TILEWIDTH);
    Rect src_rt;

    gamearea.SrcRectFixed(src_rt, dst_pt1, sprite1.w(), sprite1.h());

    LocalEvent & le = LocalEvent::Get();
    u8 alpha = 0;

    while(le.HandleEvents() && alpha < 250)
    {
        if(Game::AnimateInfrequent(Game::HEROES_FADE_DELAY))
        {
            Cursor::Get().Hide();

	    for(s16 y = mp.y - 1; y <= mp.y + 1; ++y)
		for(s16 x = mp.x - 1; x <= mp.x + 1; ++x)
    		    if(Maps::isValidAbsPoint(x, y))
	    {
        	const Maps::Tiles & tile = world.GetTiles(Maps::GetIndexFromAbsPoint(x, y));

        	tile.RedrawTile(display);
        	tile.RedrawBottom(display);
        	tile.RedrawObjects(display);
    	    }

    	    sf.SetAlpha(alpha);
    	    display.Blit(sf, src_rt, dst_pt1);

	    for(s16 y = mp.y - 1; y <= mp.y + 1; ++y)
		for(s16 x = mp.x - 1; x <= mp.x + 1; ++x)
    		    if(Maps::isValidAbsPoint(x, y))
	    {
        	const Maps::Tiles & tile = world.GetTiles(Maps::GetIndexFromAbsPoint(x, y));

        	tile.RedrawTop(display);
    	    }

	    Cursor::Get().Show();
	    display.Flip();
            alpha += 10;
        }
    }
}

bool Heroes::Move(bool fast)
{
    if(Modes(ACTION)) ResetModes(ACTION);

    // move hero
    if(path.isValid() &&
           (isEnableMove() || (GetSpriteIndex() < 45 && GetSpriteIndex() % 9) || GetSpriteIndex() >= 45))
    {
	// fast move for hide AI
	if(fast)
	{
	    direction = path.GetFrontDirection();
    	    MoveStep(true);

	    return true;
	}
	else
	{
    	    // if need change through the circle
	    if(GetDirection() != path.GetFrontDirection())
    	    {
                AngleStep(path.GetFrontDirection());
            }
            else
            // move
    	    if(MoveStep())
    	    {
    		if(isFreeman()) return false;

		return true;
    	    }
	}
    }
    else
    {
	SetMove(false);
    }

    return false;
}

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

#include <cstdlib>
#include <map>
#include "maps.h"
#include "world.h"
#include "direction.h"
#include "settings.h"
#include "object.h"
#include "heroes.h"
#include "route.h"
#include "algorithm.h"

struct cell_t
{
    cell_t() : cost_g(MAXU16), cost_t(MAXU16), cost_d(MAXU16), parent(-1), open(true){};

    u16		cost_g;
    u16		cost_t;
    u16		cost_d;
    s32		parent;
    bool	open;
};

bool ImpassableCorners(const s32 from, const Direction::vector_t to, const Heroes *hero)
{
    if( to & (Direction::TOP | Direction::BOTTOM | Direction::LEFT | Direction::RIGHT)) return false;

    if(to & (Direction::TOP_LEFT | Direction::BOTTOM_LEFT))
    {
    	if(Maps::isValidDirection(from, Direction::LEFT) &&
	   !world.GetTiles(Maps::GetDirectionIndex(from, Direction::LEFT)).isPassable(hero)) return true;
    }

    if(to & (Direction::TOP_RIGHT | Direction::BOTTOM_RIGHT))
    {
    	if(Maps::isValidDirection(from, Direction::RIGHT) &&
	   !world.GetTiles(Maps::GetDirectionIndex(from, Direction::RIGHT)).isPassable(hero)) return true;
    }

    if(to & (Direction::TOP_LEFT | Direction::TOP_RIGHT))
    {
    	if(Maps::isValidDirection(from, Direction::TOP) &&
	   !world.GetTiles(Maps::GetDirectionIndex(from, Direction::TOP)).isPassable(hero)) return true;
    }

    if(to & (Direction::BOTTOM_LEFT | Direction::BOTTOM_RIGHT))
    {
    	if(Maps::isValidDirection(from, Direction::BOTTOM) &&
	   !world.GetTiles(Maps::GetDirectionIndex(from, Direction::BOTTOM)).isPassable(hero)) return true;
    }

    return false;
}

u32 GetCurrentLength(std::map<s32, cell_t> & list, s32 cur)
{
    u32 res = 0;
    const cell_t* cell = &list[cur];
    while(-1 != cell->parent){ cell = &list[cell->parent]; ++res; };
    return res;
}

bool MonsterDestination(const s32 from, const u16 around, const s32 dst)
{
    for(Direction::vector_t dir = Direction::TOP_LEFT; dir < Direction::CENTER; ++dir)
	if((around & dir) && dst == Maps::GetDirectionIndex(from, dir))
	    return true;

    return false;
}

bool Algorithm::PathFind(std::list<Route::Step> *result, const s32 from, const s32 to, const u16 limit, const Heroes *hero)
{
    const u8 pathfinding = (hero ? hero->GetLevelSkill(Skill::Secondary::PATHFINDING) : Skill::Level::NONE);
    const u8 under = (hero ? hero->GetUnderObject() : MP2::OBJ_ZERO);

    s32 cur = from;
    s32 alt = 0;
    s32 tmp = 0;
    std::map<s32, cell_t> list;
    std::map<s32, cell_t>::iterator it1 = list.begin();
    std::map<s32, cell_t>::iterator it2 = list.end();
    Direction::vector_t direct = Direction::CENTER;

    list[cur].cost_g = 0;
    list[cur].cost_t = 0;
    list[cur].parent = -1;
    list[cur].open   = false;

    u16 mons = 0;
    cell_t cell;

    while(cur != to)
    {
	LocalEvent::Get().HandleEvents(false);

	for(direct = Direction::TOP_LEFT; direct != Direction::CENTER; ++direct)
	{
    	    if(Maps::isValidDirection(cur, direct))
	    {
		tmp = Maps::GetDirectionIndex(cur, direct);

		if(list[tmp].open)
		{
		    // new
		    if(-1 == list[tmp].parent)
		    {
	    		cell.cost_g = Maps::Ground::GetPenalty(tmp, direct, pathfinding);
			cell.parent = cur;
			cell.open = true;
	    		cell.cost_t = cell.cost_g + list[cur].cost_t;
			cell.cost_d = 50 * Maps::GetApproximateDistance(tmp, to);

			if(MAXU16 == cell.cost_g) continue;

			// check monster protection
			if(tmp != to && (mons = Maps::TileUnderProtection(tmp)) && ! MonsterDestination(tmp, mons, to)) continue;

			// check direct from object
			const Maps::Tiles & tile1 = world.GetTiles(cur);
			if(MP2::OBJ_ZERO != under && MP2::OBJ_HEROES == tile1.GetObject() &&  ! Object::AllowDirect(under, direct)) continue;

			// check obstacles as corners
			//if(ImpassableCorners(cur, direct, hero)) continue;  // disable, need fix more objects with passable option

			// check direct to object
			const Maps::Tiles & tile2 = world.GetTiles(tmp);
			if(! Object::AllowDirect(tile2.GetObject(), Direction::Reflect(direct))) continue;

			if(tile2.isPassable(hero) || tmp == to) list[tmp] = cell;
		    }
		    // check alt
		    else
		    {
			alt = Maps::Ground::GetPenalty(cur, direct, pathfinding);
			if(list[tmp].cost_t > list[cur].cost_t + alt)
			{
			    list[tmp].parent = cur;
			    list[tmp].cost_g = alt;
			    list[tmp].cost_t = list[cur].cost_t + alt;
			}
		    }
    		}
	    }
	}

	if(cur == to) break;
	else
	list[cur].open = false;

	DEBUG(DBG_GAME , DBG_TRACE, "Algorithm::PathFind: route, from: " << cur);

	it1 = list.begin();
	alt = -1;
	tmp = MAXU16;
	
	// find minimal cost
	for(; it1 != it2; ++it1) if((*it1).second.open)
	{
	    const cell_t & cell2 = (*it1).second;

	    if(IS_DEBUG(DBG_GAME, DBG_TRACE) && cell2.cost_g != MAXU16)
	    {
		direct = Direction::Get(cur, (*it1).first);
		if(Direction::UNKNOWN != direct)
		{
		    std::cout << "  direct: " << Direction::String(direct);
		    std::cout << ", index: " << (*it1).first;
		    std::cout << ", cost g: " << cell2.cost_g;
		    std::cout << ", cost t: " << cell2.cost_t;
		    std::cout << ", cost d: " << cell2.cost_d << std::endl;
		}
	    }


	    if(cell2.cost_t + cell2.cost_d < tmp)
	    {
    		tmp = cell2.cost_t + cell2.cost_d;
    		alt = (*it1).first;
	    }
	}

	// not found, and exception
	if(MAXU16 == tmp || -1 == alt || (limit && GetCurrentLength(list, cur) > limit)) break;
	else
	DEBUG(DBG_GAME , DBG_TRACE, "Algorithm::PathFind: select: " << alt);

	cur = alt;
    }

    // save path
    if(cur == to)
    {
	while(cur != from)
	{
	    if(-1 == list[cur].parent) break;
	    alt = cur;
    	    cur = list[alt].parent;
	    if(result) result->push_front(Route::Step(Direction::Get(cur, alt), list[alt].cost_g));
	}
        return true;
    }

    DEBUG(DBG_GAME , DBG_TRACE, "Algorithm::PathFind: not found, from:" << from << ", to: " << to);
    list.clear();

    return false;
}

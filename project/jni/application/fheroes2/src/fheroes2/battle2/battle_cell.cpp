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

#include "army_troop.h"
#include "battle_arena.h"
#include "settings.h"
#include "battle_stats.h"
#include "battle_cell.h"

#define CELLW 45
#define CELLH 52
#define CELLW2 23
#define CELLH2 26

Battle2::Cell::Cell(u16 i, const Rect* area, Arena & a) : index(i), arena(&a)
{
    object = 0;
    direction = UNKNOWN;
    quality = 0;

    if(Settings::Get().QVGA())
    {
	pos.x = (area ? area->x : 0) + 45 - ((i / ARENAW) % 2 ? CELLW2 / 2 : 0) + (CELLW2 - 1) * (i % ARENAW);
	pos.y = (area ? area->y + area->h - 188 : 0) + ((CELLH2 / 4) * 3) * (i / ARENAW);
	pos.w = CELLW2;
	pos.h = CELLH2;
    }
    else
    {
	pos.x = (area ? area->x : 0) + 88 - ((i / ARENAW) % 2 ? CELLW / 2 : 0) + (CELLW - 1) * (i % ARENAW);
	pos.y = (area ? area->y : 0) + 85 + ((CELLH / 4) * 3 - 1) * (i / ARENAW);
	pos.w = CELLW;
	pos.h = CELLH;
    }
}

bool Battle2::Cell::isPositionIncludePoint(const Point & pt) const
{
    return pos & pt;
}

u16 Battle2::Cell::GetIndex(void) const
{
    return index;
}

u8 Battle2::Cell::GetDirection(void) const
{
    return direction;
}

const Rect & Battle2::Cell::GetPos(void) const
{
    return pos;
}

bool GetReflectFromDirection(Battle2::direction_t dir)
{
    switch(dir)
    {
        case Battle2::TOP_LEFT:
        case Battle2::BOTTOM_LEFT:
        case Battle2::LEFT:
            return true;

        default: break;
    }
    return false;
}

bool Battle2::Cell::isPassable(const Stats & b, const Cell & from) const
{
    if(b.isWide())
    {
	switch(Board::GetDirection(from.index, index))
	{
	    case BOTTOM_RIGHT:
	    case TOP_RIGHT:
	    {
		if(CENTER == from.direction && b.isReflect()) return false;
		else
		if(GetReflectFromDirection(Board::GetReflectDirection(from.direction))) return false;
		const Cell* cell2 = arena->GetCell(index, LEFT);
		return cell2 && cell2->isPassable() && isPassable();
	    }

	    case BOTTOM_LEFT:
	    case TOP_LEFT:
	    {
		if(CENTER == from.direction && !b.isReflect()) return false;
		else
		if(!GetReflectFromDirection(Board::GetReflectDirection(from.direction))) return false;
		const Cell* cell2 = arena->GetCell(index, RIGHT);
		return cell2 && cell2->isPassable() && isPassable();
	    }

	    case LEFT:
	    case RIGHT:
		return isPassable() || index == b.GetTailIndex() || index == b.GetPosition();
	
	    default: break;
	}
    }
    return isPassable();
}

bool Battle2::Cell::isPassable(const Stats & b, bool check_reflect) const
{
    if(b.isWide())
    {
	if(index == b.GetTailIndex() || index == b.GetPosition()) return true;

	if(check_reflect)
	{
	    const Cell* cell = arena->GetCell(index, b.isReflect() ? RIGHT : LEFT);
	    return cell &&
		(cell->isPassable() || cell->index == b.GetTailIndex() || cell->index == b.GetPosition()) &&
		isPassable();
	}
	else
	{
	    Cell* left = arena->GetCell(index, LEFT);
	    Cell* right = arena->GetCell(index, RIGHT);
	    return ((left && (left->isPassable() || left->index == b.GetTailIndex() || left->index == b.GetPosition())) ||
                    (right && (right->isPassable() || right->index == b.GetTailIndex() || right->index == b.GetPosition()))) &&
                    isPassable();
	}
    }

    return isPassable();
}

bool Battle2::Cell::isPassable(void) const
{
    return NULL == arena->GetTroopBoard(index) && 0 == object;
}

void Battle2::Cell::Reset(void)
{
    direction = UNKNOWN;
    quality = 0;
}

void Battle2::Cell::ResetDirection(void)
{
    direction = UNKNOWN;
}

void Battle2::Cell::SetEnemyQuality(const Stats & my)
{
    const Stats* b = arena->GetTroopBoard(index);
    if(b && b->GetColor() != my.GetColor() && my.isValid())
    {
	// strength monster quality
	quality = b->GetScoreQuality(my) * b->HowMuchWillKilled(my.GetDamage(*b));
	quality += b->GetExtraQuality(quality);

	if(quality < 1) quality = 1;
	DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Cell::SetEnemyQuality: " << quality << " for " << b->GetName());
    }
}

void Battle2::Cell::SetPositionQuality(const Stats & a)
{
    const Stats* b = arena->GetTroopBoard(index);
    if(b && b->isWide() && b->GetTailIndex() == index) return;
    if(b && b->GetColor() != a.GetColor())
    {
	for(direction_t dir = TOP_LEFT; dir < CENTER; ++dir)
	{
	    Cell* cell = arena->GetCell(index, dir);
	    if(cell && cell->isPassable()) cell->quality += quality;
	}

	// update for wide
	if(b->isWide())
	{
	    Cell* cell = NULL;
	    if(b->isReflect())
	    {
		cell = arena->GetCell(b->GetTailIndex(), TOP_RIGHT);
		if(cell && cell->isPassable()) cell->quality += quality;

		cell = arena->GetCell(b->GetTailIndex(), RIGHT);
		if(cell && cell->isPassable()) cell->quality += quality;

		cell = arena->GetCell(b->GetTailIndex(), BOTTOM_RIGHT);
		if(cell && cell->isPassable()) cell->quality += quality;
	    }
	    else
	    {
		cell = arena->GetCell(b->GetTailIndex(), TOP_LEFT);
		if(cell && cell->isPassable()) cell->quality += quality;

		cell = arena->GetCell(b->GetTailIndex(), LEFT);
		if(cell && cell->isPassable()) cell->quality += quality;

		cell = arena->GetCell(b->GetTailIndex(), BOTTOM_LEFT);
		if(cell && cell->isPassable()) cell->quality += quality;
	    }
	}
    }
}

void Battle2::Cell::SetPassabilityAbroad(const Stats & b, std::vector<u16> & opens)
{
    for(direction_t dir = TOP_LEFT; dir < CENTER; ++dir)
    {
	Cell* cell = arena->GetCell(index, dir);
        if(cell && UNKNOWN == cell->direction && cell->isPassable(b, *this))
	{
	    cell->direction = Board::GetReflectDirection(dir);
	    opens.push_back(cell->index);
	}
    }

    if(b.isWide())
    {
	Cell* tail = arena->GetCell(b.GetTailIndex());

	if(tail)
	for(direction_t dir = TOP_LEFT; dir < CENTER; ++dir)
	{
	    Cell* cell = arena->GetCell(tail->index, dir);
    	    if(cell && UNKNOWN == cell->direction && cell->isPassable(b, *tail))
	    {
		cell->direction = Board::GetReflectDirection(dir);
		opens.push_back(cell->index);
	    }
	}
    }
}

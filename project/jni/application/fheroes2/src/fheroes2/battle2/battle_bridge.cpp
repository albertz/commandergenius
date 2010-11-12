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

#include "castle.h"
#include "battle_stats.h"
#include "battle_cell.h"
#include "battle_bridge.h"

Battle2::Bridge::Bridge(Arena & a) : arena(a), destroy(false), down(false)
{
}

bool Battle2::Bridge::isIndex(u16 index)
{
    switch(index)
    {
	case 49:
	case 50: return true;
	default: break;
    }
    return false;
}

bool Battle2::Bridge::isValid(void) const
{
    return !isDestroy();
}

bool Battle2::Bridge::isDestroy(void) const
{
    return destroy;
}

bool Battle2::Bridge::isDown(void) const
{
    return down || isDestroy();
}

void Battle2::Bridge::SetDown(bool f)
{
    down = f;
}

bool Battle2::Bridge::AllowUp(void)
{
    return NULL == arena.GetTroopBoard(49) && NULL == arena.GetTroopBoard(50);
}

bool Battle2::Bridge::NeedDown(const Stats & b, u16 pos2)
{
    const u16 pos1 = b.GetPosition();

    if(pos2 == 50)
    {
	if(pos1 == 51) return true;
	if((pos1 == 61 || pos1 == 39) && b.GetColor() == arena.castle->GetColor()) return true;
    }
    else
    if(pos2 == 49)
    {
	if(pos1 != 50 && b.GetColor() == arena.castle->GetColor()) return true;
    }

    return false;
}

bool Battle2::Bridge::isPassable(u8 color) const
{
    return color == arena.castle->GetColor() || isDown();
}

void Battle2::Bridge::SetDestroy(void)
{
    destroy = true;
    arena.board[49].object = 0;
    arena.board[50].object = 0;
}

void Battle2::Bridge::SetPassable(const Stats & b)
{
    if(Board::inCastle(b.GetPosition()) || b.GetColor() == arena.castle->GetColor())
    {
	arena.board[49].object = 0;
	arena.board[50].object = 0;
    }
    else
    {
	arena.board[49].object = 1;
	arena.board[50].object = 1;
    }
}

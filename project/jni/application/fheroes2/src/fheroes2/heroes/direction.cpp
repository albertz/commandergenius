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

#include "maps.h"
#include "direction.h"

const char* Direction::String(vector_t direct)
{
   const char* str_direct[] = { "center", "top", "top right", "right", "bottom right", "bottom", "bottom left", "left", "top left", "unknown" };
   
   switch(direct)
   {
	case CENTER:		return str_direct[0];
	case TOP:		return str_direct[1];
	case TOP_RIGHT:		return str_direct[2];
	case RIGHT:		return str_direct[3];
	case BOTTOM_RIGHT:	return str_direct[4];
	case BOTTOM:		return str_direct[5];
	case BOTTOM_LEFT:	return str_direct[6];
	case LEFT:		return str_direct[7];
	case TOP_LEFT:		return str_direct[8];
	default:	break;
   }
 
    return str_direct[9];
}


Direction::vector_t Direction::Get(s32 from, s32 to)
{
    if(to == from)
	return CENTER;
    else
    if(to == Maps::GetDirectionIndex(from, TOP))
	return TOP;
    else
    if(to == Maps::GetDirectionIndex(from, TOP_RIGHT))
    	return TOP_RIGHT;
    else
    if(to == Maps::GetDirectionIndex(from, RIGHT))
    	return RIGHT;
    else
    if(to == Maps::GetDirectionIndex(from, BOTTOM_RIGHT))
    	return BOTTOM_RIGHT;
    else
    if(to == Maps::GetDirectionIndex(from, BOTTOM))
    	return BOTTOM;
    else
    if(to == Maps::GetDirectionIndex(from, BOTTOM_LEFT))
    	return BOTTOM_LEFT;
    else
    if(to == Maps::GetDirectionIndex(from, LEFT))
    	return LEFT;
    else
    if(to == Maps::GetDirectionIndex(from, TOP_LEFT))
    	return TOP_LEFT;

    return UNKNOWN;
}

bool Direction::ShortDistanceClockWise(const vector_t from , const vector_t to)
{
   switch(from)
   {
	case TOP:
	    switch(to)
	    {
		case TOP_RIGHT:
		case RIGHT:
		case BOTTOM_RIGHT:
		case BOTTOM:		return true;

		default: break;
	    }
	    break;

	case TOP_RIGHT:
	    switch(to)
	    {
		case RIGHT:
		case BOTTOM_RIGHT:
		case BOTTOM:
		case BOTTOM_LEFT:	return true;

		default: break;
	    }
	    break;

	case RIGHT:
	    switch(to)
	    {
		case BOTTOM_RIGHT:
		case BOTTOM:
		case BOTTOM_LEFT:
		case LEFT:		return true;

		default: break;
	    }
	    break;

	case BOTTOM_RIGHT:
	    switch(to)
	    {
		case BOTTOM:
		case BOTTOM_LEFT:
		case LEFT:
		case TOP_LEFT:		return true;

		default: break;
	    }
	    break;

	case BOTTOM:
	    switch(to)
	    {
		case BOTTOM_LEFT:
		case LEFT:
		case TOP_LEFT:		return true;

		default: break;
	    }
	    break;

	case BOTTOM_LEFT:
	    switch(to)
	    {
		case TOP:
		case TOP_RIGHT:
		case LEFT:
		case TOP_LEFT:		return true;

		default: break;
	    }
	    break;

	case LEFT:
	    switch(to)
	    {
		case TOP:
		case TOP_RIGHT:
		case RIGHT:
		case TOP_LEFT:		return true;

		default: break;
	    }
	    break;

	case TOP_LEFT:
	    switch(to)
	    {
		case TOP:
		case TOP_RIGHT:
		case RIGHT:
		case BOTTOM_RIGHT:	return true;

		default: break;
	    }
	    break;

	default: break;
   }

    return false;
}

Direction::vector_t Direction::Reflect(Direction::vector_t from)
{
    switch(from)
    {
	case TOP_LEFT:		return BOTTOM_RIGHT;
	case TOP:		return BOTTOM;
	case TOP_RIGHT:		return BOTTOM_LEFT;
	case RIGHT:		return LEFT;
	case BOTTOM_RIGHT:	return TOP_LEFT;
	case BOTTOM:		return TOP;
	case BOTTOM_LEFT:	return TOP_RIGHT;
	case LEFT:		return RIGHT;
	case CENTER:		return CENTER;
	default: break;
    }

    return UNKNOWN;
}

Direction::vector_t Direction::FromInt(u16 index)
{
    switch(index)
    {
	case 0x0001:	return TOP_LEFT;
	case 0x0002:	return TOP;
	case 0x0004:	return TOP_RIGHT;
	case 0x0008:	return RIGHT;
	case 0x0010:	return BOTTOM_RIGHT;
	case 0x0020:	return BOTTOM;
	case 0x0040:	return BOTTOM_LEFT;
	case 0x0080:	return LEFT;
	case 0x0100:	return CENTER;
	default: break;
    }

    return UNKNOWN;
}

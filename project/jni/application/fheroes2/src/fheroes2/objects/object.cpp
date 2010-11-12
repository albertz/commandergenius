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
#include "maps_tiles.h"
#include "mounts.h"
#include "trees.h"
#include "objxloc.h"
#include "objtown.h"
#include "objwatr.h"
#include "objlava.h"
#include "objmult.h"
#include "objdsrt.h"
#include "objdirt.h"
#include "objsnow.h"
#include "objswmp.h"
#include "objgras.h"
#include "objcrck.h"
#include "direction.h"
#include "object.h"

Object::Object(const MP2::object_t obj, const u16 sicn, const u32 uid)
    : object(obj), icn(sicn), id(uid ? uid : World::GetUniq())
{
}

Object::~Object()
{
}

bool Object::isPassable(const std::list<Maps::TilesAddon> & bottoms, const s32 maps_index)
{
    if(bottoms.size())
    {
        std::list<Maps::TilesAddon>::const_iterator it1 = bottoms.begin();
        std::list<Maps::TilesAddon>::const_iterator it2 = bottoms.end();

        for(; it1 != it2; ++it1)
        {
            const Maps::TilesAddon & addon = *it1;
	    const ICN::icn_t icn = MP2::GetICNObject(addon.object);

            switch(icn)
            {
		case ICN::MTNCRCK:
		case ICN::MTNSNOW:
		case ICN::MTNSWMP:
	    	case ICN::MTNLAVA:
	    	case ICN::MTNDSRT:
	    	case ICN::MTNDIRT:
	    	case ICN::MTNMULT:
	    	case ICN::MTNGRAS:	if(! Mounts::isPassable(icn, addon.index)) return false; break;

		case ICN::TREJNGL:
		case ICN::TREEVIL:
		case ICN::TRESNOW:
		case ICN::TREFIR:
		case ICN::TREFALL:
		case ICN::TREDECI:	if(! Trees::isPassable(icn, addon.index)) return false; break;

		case ICN::OBJNSNOW:	if(! ObjSnow::isPassable(icn, addon.index)) return false; break;
		case ICN::OBJNSWMP:	if(! ObjSwamp::isPassable(icn, addon.index)) return false; break;
		case ICN::OBJNGRAS:
		case ICN::OBJNGRA2:	if(! ObjGrass::isPassable(icn, addon.index)) return false; break;
		case ICN::OBJNCRCK:	if(! ObjWasteLand::isPassable(icn, addon.index)) return false; break;
		case ICN::OBJNDIRT:	if(! ObjDirt::isPassable(icn, addon.index)) return false; break;
		case ICN::OBJNDSRT:	if(! ObjDesert::isPassable(icn, addon.index)) return false; break;
		case ICN::OBJNMUL2:
		case ICN::OBJNMULT:	if(! ObjMulti::isPassable(icn, addon.index)) return false; break;
		case ICN::OBJNLAVA:
		case ICN::OBJNLAV3:
		case ICN::OBJNLAV2:	if(! ObjLava::isPassable(icn, addon.index)) return false; break;
		case ICN::OBJNWAT2:
		case ICN::OBJNWATR:	if(! ObjWater::isPassable(icn, addon.index)) return false; break;

		case ICN::MONS32:
		case ICN::MINIMON:	return false;

		case ICN::OBJNARTI:
		case ICN::OBJNRSRC:	if(addon.index % 2) return false; break;

		case ICN::OBJNTWBA:
		case ICN::OBJNTOWN:	if(! ObjTown::isPassable(icn, addon.index, maps_index)) return false; break;

		case ICN::X_LOC1:
		case ICN::X_LOC2:
		case ICN::X_LOC3:	if(! ObjLoyalty::isPassable(icn, addon.index)) return false; break;

		// MANUAL.ICN
		case ICN::TREASURE:
		case ICN::TELEPORT1:
	        case ICN::TELEPORT2:
		case ICN::TELEPORT3:
		case ICN::FOUNTAIN:	return false; break;

		default: break;
	    }
        }
    }

    return true;
}

bool Object::AllowDirect(const u8 general, const u16 direct)
{
    switch(general)
    {
	case MP2::OBJ_SHIPWRECK:
	    return (direct & (Direction::LEFT | Direction::BOTTOM_LEFT));

	case MP2::OBJ_DERELICTSHIP:
	    return (direct & (Direction::LEFT | Direction::BOTTOM_LEFT | Direction::BOTTOM));

	case MP2::OBJ_ARCHERHOUSE:
	case MP2::OBJ_DOCTORHUT:
	case MP2::OBJ_DWARFCOTT:
	case MP2::OBJ_THATCHEDHUT:
	case MP2::OBJ_FOUNTAIN:
	case MP2::OBJ_IDOL:
	case MP2::OBJ_LIGHTHOUSE:
	case MP2::OBJ_OBELISK:
	case MP2::OBJ_SIGN:
	case MP2::OBJ_WATCHTOWER:
	case MP2::OBJ_WITCHSHUT:
	case MP2::OBJ_GAZEBO:
	case MP2::OBJ_MAGICWELL:
	case MP2::OBJ_OBSERVATIONTOWER:
	case MP2::OBJ_PEASANTHUT:
	case MP2::OBJ_STONELIGHTS:
	case MP2::OBJ_STANDINGSTONES:
	case MP2::OBJ_GOBLINHUT:
	case MP2::OBJ_SHRINE1:
	case MP2::OBJ_SHRINE2:
	case MP2::OBJ_SHRINE3:
	case MP2::OBJ_TREEHOUSE:
	case MP2::OBJ_ARTESIANSPRING:
	case MP2::OBJ_SKELETON:
	case MP2::OBJ_TREEKNOWLEDGE:
	case MP2::OBJ_ORACLE:
	case MP2::OBJ_OASIS:
	case MP2::OBJ_LEANTO:
	case MP2::OBJ_MAGICGARDEN:
	case MP2::OBJ_WAGON:
	//case MP2::OBJ_BARRIER: // because Barrier used with any direction
	case MP2::OBJ_TRAVELLERTENT:
	case MP2::OBJ_JAIL:
	case MP2::OBJ_ALCHEMYTOWER:
        case MP2::OBJ_HUTMAGI:
        case MP2::OBJ_EYEMAGI:
	case MP2::OBJ_WATERINGHOLE:
	    return (direct & (Direction::LEFT | Direction::RIGHT | DIRECTION_BOTTOM_ROW));

	case MP2::OBJ_TRADINGPOST:
	case MP2::OBJ_EXCAVATION:
	case MP2::OBJ_DESERTTENT:
	case MP2::OBJ_DAEMONCAVE:
	case MP2::OBJ_PYRAMID:
	case MP2::OBJ_FORT:
	case MP2::OBJ_RUINS:
	case MP2::OBJ_TROLLBRIDGE:
	case MP2::OBJ_WATERWHEEL:
	case MP2::OBJ_HILLFORT:
	case MP2::OBJ_FREEMANFOUNDRY:
	case MP2::OBJ_SAWMILL:
	    return (direct & (Direction::RIGHT | DIRECTION_BOTTOM_ROW));

	case MP2::OBJ_TREECITY:
	case MP2::OBJ_MAGELLANMAPS:
	case MP2::OBJ_SPHINX:
	case MP2::OBJ_TEMPLE:
	case MP2::OBJ_WINDMILL:
	case MP2::OBJ_FAERIERING:
	case MP2::OBJ_BARROWMOUNDS:
	case MP2::OBJ_STABLES:
	    return (direct & (Direction::LEFT | DIRECTION_BOTTOM_ROW));

	case MP2::OBJ_MINES:
	case MP2::OBJ_ABANDONEDMINE:
	case MP2::OBJ_ALCHEMYLAB:
	case MP2::OBJ_CAVE:
	case MP2::OBJ_CITYDEAD:
	case MP2::OBJ_GRAVEYARD:
	case MP2::OBJ_DRAGONCITY:
	case MP2::OBJ_XANADU:
	case MP2::OBJ_MERCENARYCAMP:
	case MP2::OBJ_HALFLINGHOLE:
	case MP2::OBJ_WAGONCAMP:
	case MP2::OBJ_WATERALTAR:
	case MP2::OBJ_AIRALTAR:
	case MP2::OBJ_FIREALTAR:
	case MP2::OBJ_EARTHALTAR:
	case MP2::OBJ_ARENA:
	case MP2::OBJ_SIRENS:
	case MP2::OBJ_MERMAID:
	    return (direct & (DIRECTION_BOTTOM_ROW));

	case MP2::OBJ_CASTLE:
	    return (direct & Direction::BOTTOM);

	default: break;
    }

    return true;
}

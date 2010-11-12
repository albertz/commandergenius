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

#include "settings.h"
#include "mp2.h"

/* return name icn object */
ICN::icn_t MP2::GetICNObject(const u8 type)
{
    switch(type)
    {
	// manual
	case 0x11:
	    return ICN::TELEPORT1;
	case 0x12:
	    return ICN::TELEPORT2;
	case 0x13:
	    return ICN::TELEPORT3;
	case 0x14:
	    return ICN::FOUNTAIN;
	case 0x15:
	    return ICN::TREASURE;

	// artifact
	case 0x2C:
	case 0x2D:
	case 0x2E:
	case 0x2F:
	    return ICN::OBJNARTI;

	// monster
	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	    return ICN::MONS32;

	// castle flags
	case 0x38:
	case 0x39:
	case 0x3A:
	case 0x3B:
	    return ICN::FLAG32;

	// heroes
	case 0x54:
	case 0x55:
	case 0x56:
	case 0x57:
	    return ICN::MINIHERO;

	// relief: snow
	case 0x58:
	case 0x59:
	case 0x5A:
	case 0x5B:
	    return ICN::MTNSNOW;

	// relief: swamp
	case 0x5C:
	case 0x5D:
	case 0x5E:
	case 0x5F:
	    return ICN::MTNSWMP;

	// relief: lava
	case 0x60:
	case 0x61:
	case 0x62:
	case 0x63:
	    return ICN::MTNLAVA;

	// relief: desert
	case 0x64:
	case 0x65:
	case 0x66:
	case 0x67:
	    return ICN::MTNDSRT;

	// relief: dirt
	case 0x68:
	case 0x69:
	case 0x6A:
	case 0x6B:
	    return ICN::MTNDIRT;

	// relief: others
	case 0x6C:
	case 0x6D:
	case 0x6E:
	case 0x6F:
	    return ICN::MTNMULT;

	// mines
	case 0x74:
	    return ICN::EXTRAOVR;

	// road
	case 0x78:
	case 0x79:
	case 0x7A:
	case 0x7B:
	    return ICN::ROAD;

	// relief: crck
	case 0x7C:
	case 0x7D:
	case 0x7E:
	case 0x7F:
	    return ICN::MTNCRCK;

	// relief: gras
	case 0x80:
	case 0x81:
	case 0x82:
	case 0x83:
	    return ICN::MTNGRAS;

	// trees jungle
	case 0x84:
	case 0x85:
	case 0x86:
	case 0x87:
	    return ICN::TREJNGL;

	// trees evil
	case 0x88:
	case 0x89:
	case 0x8A:
	case 0x8B:
	    return ICN::TREEVIL;

	// castle and tower
	case 0x8C:
	case 0x8D:
	case 0x8E:
	case 0x8F:
	    return ICN::OBJNTOWN;

	// castle lands
	case 0x90:
	case 0x91:
	case 0x92:
	case 0x93:
	    return ICN::OBJNTWBA;

	// castle shadow
	case 0x94:
	case 0x95:
	case 0x96:
	case 0x97:
	    return ICN::OBJNTWSH;

	// random castle
	case 0x98:
	case 0x99:
	case 0x9A:
	case 0x9B:
	    return ICN::OBJNTWRD;

	// water object
	case 0xA0:
	case 0xA1:
	case 0xA2:
	case 0xA3:
	    return ICN::OBJNWAT2;

	// object other
	case 0xA4:
	case 0xA5:
	case 0xA6:
	case 0xA7:
	    return ICN::OBJNMUL2;

	// trees snow
	case 0xA8:
	case 0xA9:
	case 0xAA:
	case 0xAB:
	    return ICN::TRESNOW;

	// trees trefir
	case 0xAC:
	case 0xAD:
	case 0xAE:
	case 0xAF:
	    return ICN::TREFIR;

	// trees
	case 0xB0:
	case 0xB1:
	case 0xB2:
	case 0xB3:
	    return ICN::TREFALL;

	// river
	case 0xB4:
	case 0xB5:
	case 0xB6:
	case 0xB7:
	    return ICN::STREAM;

	// resource
	case 0xB8:
	case 0xB9:
	case 0xBA:
	case 0xBB:
	    return ICN::OBJNRSRC;

	// gras object
	case 0xC0:
	case 0xC1:
	case 0xC2:
	case 0xC3:
	    return ICN::OBJNGRA2;

	// trees tredeci
	case 0xC4:
	case 0xC5:
	case 0xC6:
	case 0xC7:
	    return ICN::TREDECI;

	// sea object
	case 0xC8:
	case 0xC9:
	case 0xCA:
	case 0xCB:
	    return ICN::OBJNWATR;

	// vegetation gras
	case 0xCC:
	case 0xCD:
	case 0xCE:
	case 0xCF:
	    return ICN::OBJNGRAS;

	// object on snow
	case 0xD0:
	case 0xD1:
	case 0xD2:
	case 0xD3:
	    return ICN::OBJNSNOW;

	// object on swamp
	case 0xD4:
	case 0xD5:
	case 0xD6:
	case 0xD7:
	    return ICN::OBJNSWMP;

	// object on lava
	case 0xD8:
	case 0xD9:
	case 0xDA:
	case 0xDB:
	    return ICN::OBJNLAVA;

	// object on desert
	case 0xDC:
	case 0xDD:
	case 0xDE:
	case 0xDF:
	    return ICN::OBJNDSRT;

	// object on dirt
	case 0xE0:
	case 0xE1:
	case 0xE2:
	case 0xE3:
	    return ICN::OBJNDIRT;

	// object on crck
	case 0xE4:
	case 0xE5:
	case 0xE6:
	case 0xE7:
	    return ICN::OBJNCRCK;

	// object on lava
	case 0xE8:
	case 0xE9:
	case 0xEA:
	case 0xEB:
	    return ICN::OBJNLAV3;

	// object on earth
	case 0xEC:
	case 0xED:
	case 0xEE:
	case 0xEF:
	    return ICN::OBJNMULT;
	    
	//  object on lava
	case 0xF0:
	case 0xF1:
	case 0xF2:
	case 0xF3:
	    return ICN::OBJNLAV2;

	// extra objects for loyalty version
	case 0xF4:
	case 0xF5:
	case 0xF6:
	case 0xF7:
	    if(Settings::Get().PriceLoyaltyVersion()) return ICN::X_LOC1;
	    break;

	// extra objects for loyalty version
	case 0xF8:
	case 0xF9:
	case 0xFA:
	case 0xFB:
	    if(Settings::Get().PriceLoyaltyVersion()) return ICN::X_LOC2;
	    break;

	// extra objects for loyalty version
	case 0xFC:
	case 0xFD:
	case 0xFE:
	case 0xFF:
	    if(Settings::Get().PriceLoyaltyVersion()) return ICN::X_LOC3;
	    break;

	default:
	    break;
    }

    DEBUG(DBG_GAME , DBG_WARN, "MP2::GetICNObject: unknown type: " << static_cast<int>(type));

    return ICN::UNKNOWN;
}

const char* MP2::StringObject(u8 object)
{
    switch(object)
    {
        case MP2::OBJ_ZERO:			return "OBJ_ZERO";
        case MP2::OBJN_ALCHEMYLAB:
        case MP2::OBJ_ALCHEMYLAB:		return _("Alchemist Lab");
        case MP2::OBJN_DAEMONCAVE:
        case MP2::OBJ_DAEMONCAVE:		return _("Daemon Cave");
        case MP2::OBJN_FAERIERING:
        case MP2::OBJ_FAERIERING:		return _("Faerie Ring");
        case MP2::OBJN_GRAVEYARD:
        case MP2::OBJ_GRAVEYARD:		return _("Graveyard");
        case MP2::OBJN_DRAGONCITY:
        case MP2::OBJ_DRAGONCITY:		return _("Dragon City");
        case MP2::OBJN_LIGHTHOUSE:
        case MP2::OBJ_LIGHTHOUSE:		return _("Light House");
        case MP2::OBJN_WATERWHEEL:
        case MP2::OBJ_WATERWHEEL:		return _("Water Wheel");
        case MP2::OBJN_MINES:
        case MP2::OBJ_MINES:			return _("Mines");
        case MP2::OBJN_OBELISK:
        case MP2::OBJ_OBELISK:			return _("Obelisk");
        case MP2::OBJN_OASIS:
        case MP2::OBJ_OASIS:			return _("Oasis");
        case MP2::OBJN_SAWMILL:
        case MP2::OBJ_SAWMILL:			return _("Sawmill");
        case MP2::OBJN_ORACLE:
        case MP2::OBJ_ORACLE:			return _("Oracle");
        case MP2::OBJN_DESERTTENT:
        case MP2::OBJ_DESERTTENT:		return _("Desert Tent");
        case MP2::OBJN_CASTLE:
        case MP2::OBJ_CASTLE:			return _("Castle");
        case MP2::OBJN_WAGONCAMP:
        case MP2::OBJ_WAGONCAMP:		return _("Wagon Camp");
        case MP2::OBJN_WINDMILL:
        case MP2::OBJ_WINDMILL:			return _("Windmill");
        case MP2::OBJN_RNDTOWN:
        case MP2::OBJ_RNDTOWN:			return _("Random Town");
        case MP2::OBJN_RNDCASTLE:
        case MP2::OBJ_RNDCASTLE:		return _("Random Castle");
        case MP2::OBJN_WATCHTOWER:
        case MP2::OBJ_WATCHTOWER:            	return _("Watch Tower");
        case MP2::OBJN_TREECITY:
        case MP2::OBJ_TREECITY:			return _("Tree City");
        case MP2::OBJN_TREEHOUSE:
        case MP2::OBJ_TREEHOUSE:             	return _("Tree House");
        case MP2::OBJN_RUINS:
        case MP2::OBJ_RUINS:			return _("Ruins");
        case MP2::OBJN_FORT:
        case MP2::OBJ_FORT:			return _("Fort");
        case MP2::OBJN_TRADINGPOST:
        case MP2::OBJ_TRADINGPOST:		return _("Trading Post");
        case MP2::OBJN_ABANDONEDMINE:
        case MP2::OBJ_ABANDONEDMINE:		return _("Abandoned Mine");
        case MP2::OBJN_TREEKNOWLEDGE:
        case MP2::OBJ_TREEKNOWLEDGE:		return _("Tree of Knowledge");
        case MP2::OBJN_DOCTORHUT:
        case MP2::OBJ_DOCTORHUT:		return _("Witch Doctor's Hut");
        case MP2::OBJN_TEMPLE:
        case MP2::OBJ_TEMPLE:			return _("Temple");
        case MP2::OBJN_HILLFORT:
        case MP2::OBJ_HILLFORT:			return _("Hill Fort");
        case MP2::OBJN_HALFLINGHOLE:
        case MP2::OBJ_HALFLINGHOLE:		return _("Halfling Hole");
        case MP2::OBJN_MERCENARYCAMP:
        case MP2::OBJ_MERCENARYCAMP:		return _("Mercenary Camp");
        case MP2::OBJN_PYRAMID:
        case MP2::OBJ_PYRAMID:			return _("Pyramid");
        case MP2::OBJN_CITYDEAD:
        case MP2::OBJ_CITYDEAD:			return _("City of the Dead");
        case MP2::OBJN_EXCAVATION:
        case MP2::OBJ_EXCAVATION:		return _("Excavation");
        case MP2::OBJN_SPHINX:
        case MP2::OBJ_SPHINX:			return _("Sphinx");
        case MP2::OBJN_TROLLBRIDGE:
        case MP2::OBJ_TROLLBRIDGE:		return _("Troll Bridge");
        case MP2::OBJN_WITCHSHUT:
        case MP2::OBJ_WITCHSHUT:		return _("Witch Hut");
        case MP2::OBJN_XANADU:
        case MP2::OBJ_XANADU:			return _("Xanadu");
        case MP2::OBJN_CAVE:
        case MP2::OBJ_CAVE:			return _("Cave");
        case MP2::OBJN_MAGELLANMAPS:
        case MP2::OBJ_MAGELLANMAPS:		return _("Magellan Maps");
        case MP2::OBJN_DERELICTSHIP:
        case MP2::OBJ_DERELICTSHIP:		return _("Derelict Ship");
        case MP2::OBJN_SHIPWRECK:
        case MP2::OBJ_SHIPWRECK:		return _("Ship Wreck");
        case MP2::OBJN_OBSERVATIONTOWER:
        case MP2::OBJ_OBSERVATIONTOWER:		return _("Observation Tower");
        case MP2::OBJN_FREEMANFOUNDRY:
        case MP2::OBJ_FREEMANFOUNDRY:		return _("Freeman Foundry");
        case MP2::OBJN_WATERINGHOLE:
        case MP2::OBJ_WATERINGHOLE:            	return _("Watering Hole");
        case MP2::OBJN_ARTESIANSPRING:
        case MP2::OBJ_ARTESIANSPRING:        	return _("Artesian Spring");
        case MP2::OBJN_GAZEBO:
        case MP2::OBJ_GAZEBO:			return _("Gazebo");
        case MP2::OBJN_ARCHERHOUSE:
        case MP2::OBJ_ARCHERHOUSE:		return _("Archer's House");
        case MP2::OBJN_PEASANTHUT:
        case MP2::OBJ_PEASANTHUT:		return _("Peasant Hut");
        case MP2::OBJN_DWARFCOTT:
        case MP2::OBJ_DWARFCOTT:		return _("Dwarf Cottage");
        case MP2::OBJN_STONELIGHTS:
        case MP2::OBJ_STONELIGHTS:		return _("Stone Liths");
        case MP2::OBJN_MAGICWELL:
        case MP2::OBJ_MAGICWELL:             	return _("Magic Well");
        case MP2::OBJ_HEROES:			return _("Heroes");
        case MP2::OBJ_SIGN:			return _("Sign");
        case MP2::OBJ_SHRUB2:			return _("Shrub");
        case MP2::OBJ_NOTHINGSPECIAL:		return _("Nothing Special");
        case MP2::OBJ_TARPIT:			return _("Tar Pit");
        case MP2::OBJ_COAST:			return _("Coast");
        case MP2::OBJ_MOUND:			return _("Mound");
        case MP2::OBJ_DUNE:			return _("Dune");
	case MP2::OBJ_STUMP:			return _("Stump");
	case MP2::OBJ_CACTUS:			return _("Cactus");
        case MP2::OBJ_TREES:			return _("Trees");
        case MP2::OBJ_DEADTREE:			return _("Dead Tree");
        case MP2::OBJ_MOUNTS:			return _("Mountains");
        case MP2::OBJ_VOLCANO:			return _("Volcano");
        case MP2::OBJ_STONES:			return _("Rock");
        case MP2::OBJ_FLOWERS:			return _("Flowers");
        case MP2::OBJ_WATERLAKE:		return _("Water Lake");
        case MP2::OBJ_MANDRAKE:			return _("Mandrake");
        case MP2::OBJ_CRATER:			return _("Crater");
        case MP2::OBJ_LAVAPOOL:			return _("Lava Pool");
        case MP2::OBJ_SHRUB:			return _("Shrub");
        case MP2::OBJ_BUOY:			return _("Buoy");
        case MP2::OBJ_SKELETON:			return _("Skeleton");
        case MP2::OBJ_TREASURECHEST:
        case MP2::OBJ_WATERCHEST:		return _("Treasure Chest");
        case MP2::OBJ_CAMPFIRE:			return _("Campfire");
        case MP2::OBJ_FOUNTAIN:			return _("Fountain");
        case MP2::OBJ_ANCIENTLAMP:		return _("Genie Lamp");
        case MP2::OBJ_GOBLINHUT:		return _("Goblin Hut");
        case MP2::OBJ_THATCHEDHUT:		return _("Thatched Hut");
        case MP2::OBJ_MONSTER:			return _("Monster");
        case MP2::OBJ_RESOURCE:			return _("Resource");
        case MP2::OBJ_WHIRLPOOL:		return _("Whirlpool");
        case MP2::OBJ_ARTIFACT:			return _("Artifact");
        case MP2::OBJ_BOAT:			return _("Boat");
        case MP2::OBJ_RNDARTIFACT:		return "Random Artifact";
        case MP2::OBJ_RNDRESOURCE:		return "Random Resource";
        case MP2::OBJ_RNDMONSTER1:           	return "OBJ_RNDMONSTER1";
        case MP2::OBJ_RNDMONSTER2:           	return "OBJ_RNDMONSTER2";
        case MP2::OBJ_RNDMONSTER3:           	return "OBJ_RNDMONSTER3";
        case MP2::OBJ_RNDMONSTER4:           	return "OBJ_RNDMONSTER4";
        case MP2::OBJ_STANDINGSTONES:        	return _("Standing Stones");
        case MP2::OBJ_EVENT:                 	return "OBJ_EVENT";
        case MP2::OBJ_RNDMONSTER:            	return "OBJ_RNDMONSTER";
        case MP2::OBJ_RNDULTIMATEARTIFACT:   	return "OBJ_RNDULTIMATEARTIFACT";
        case MP2::OBJ_IDOL:                  	return _("Idol");
        case MP2::OBJ_SHRINE1:               	return _("Shrine of the First Circle");
        case MP2::OBJ_SHRINE2:               	return _("Shrine of the Second Circle");
        case MP2::OBJ_SHRINE3:               	return _("Shrine of the Third Circle");
        case MP2::OBJ_WAGON:                 	return _("Wagon");
        case MP2::OBJ_LEANTO:                	return _("Lean To");
        case MP2::OBJ_FLOTSAM:               	return _("Flotsam");
        case MP2::OBJ_SHIPWRECKSURVIROR:     	return _("Shipwreck Surviror");
        case MP2::OBJ_BOTTLE:                	return _("Bottle");
        case MP2::OBJ_MAGICGARDEN:           	return _("Magic Garden");
        case MP2::OBJ_RNDARTIFACT1:          	return "OBJ_RNDARTIFACT1";
        case MP2::OBJ_RNDARTIFACT2:          	return "OBJ_RNDARTIFACT2";
        case MP2::OBJ_RNDARTIFACT3:          	return "OBJ_RNDARTIFACT3";

	case MP2::OBJN_JAIL:
	case MP2::OBJ_JAIL:			return _("Jail");
	case MP2::OBJN_TRAVELLERTENT:
	case MP2::OBJ_TRAVELLERTENT:		return _("Traveller's Tent");
	case MP2::OBJ_BARRIER:			return _("Barrier");

	case MP2::OBJN_FIREALTAR:
	case MP2::OBJ_FIREALTAR:		return _("Fire Summoning Altar");
	case MP2::OBJN_AIRALTAR:
	case MP2::OBJ_AIRALTAR:			return _("Air Summoning Altar");
	case MP2::OBJN_EARTHALTAR:
	case MP2::OBJ_EARTHALTAR:		return _("Earth Summoning Altar");
	case MP2::OBJN_WATERALTAR:
	case MP2::OBJ_WATERALTAR:		return _("Water Summoning Altar");
	case MP2::OBJN_BARROWMOUNDS:
	case MP2::OBJ_BARROWMOUNDS:		return _("Barrow Mounds");
	case MP2::OBJN_ARENA:
	case MP2::OBJ_ARENA:			return _("Arena");
	case MP2::OBJN_STABLES:
	case MP2::OBJ_STABLES:			return _("Stables");
	case MP2::OBJN_ALCHEMYTOWER:
	case MP2::OBJ_ALCHEMYTOWER:		return _("Alchemist's Tower");
	case MP2::OBJN_HUTMAGI:
	case MP2::OBJ_HUTMAGI:			return _("Hut of the Magi");
	case MP2::OBJN_EYEMAGI:
	case MP2::OBJ_EYEMAGI:			return _("Eye of the Magi");
	case MP2::OBJN_MERMAID:
	case MP2::OBJ_MERMAID:			return _("Mermaid");
	case MP2::OBJN_SIRENS:
	case MP2::OBJ_SIRENS:			return _("Sirens");
	case MP2::OBJ_REEFS:			return _("Reefs");

	case MP2::OBJ_UNKNW_02:		return "OBJ_UNKNW_02";
	case MP2::OBJ_UNKNW_03:		return "OBJ_UNKNW_03";
	case MP2::OBJ_UNKNW_04:		return "OBJ_UNKNW_04";
	case MP2::OBJ_UNKNW_06:		return "OBJ_UNKNW_06";
	case MP2::OBJ_UNKNW_08:		return "OBJ_UNKNW_08";
	case MP2::OBJ_UNKNW_09:		return "OBJ_UNKNW_09";
	case MP2::OBJ_UNKNW_0B:		return "OBJ_UNKNW_0B";
	case MP2::OBJ_UNKNW_0E:		return "OBJ_UNKNW_0E";
	case MP2::OBJ_UNKNW_11:		return "OBJ_UNKNW_11";
	case MP2::OBJ_UNKNW_12:		return "OBJ_UNKNW_12";
	case MP2::OBJ_UNKNW_13:		return "OBJ_UNKNW_13";
	case MP2::OBJ_UNKNW_18:		return "OBJ_UNKNW_18";
	case MP2::OBJ_UNKNW_1B:		return "OBJ_UNKNW_1B";
	case MP2::OBJ_UNKNW_1F:		return "OBJ_UNKNW_1F";
	case MP2::OBJ_UNKNW_21:		return "OBJ_UNKNW_21";
	case MP2::OBJ_UNKNW_26:		return "OBJ_UNKNW_26";
	case MP2::OBJ_UNKNW_27:		return "OBJ_UNKNW_27";
	case MP2::OBJ_UNKNW_29:		return "OBJ_UNKNW_29";
	case MP2::OBJ_UNKNW_2A:		return "OBJ_UNKNW_2A";
	case MP2::OBJ_UNKNW_2B:		return "OBJ_UNKNW_2B";
	case MP2::OBJ_UNKNW_2C:		return "OBJ_UNKNW_2C";
	case MP2::OBJ_UNKNW_2D:		return "OBJ_UNKNW_2D";
	case MP2::OBJ_UNKNW_2E:		return "OBJ_UNKNW_2E";
	case MP2::OBJ_UNKNW_2F:		return "OBJ_UNKNW_2F";
	case MP2::OBJ_UNKNW_32:		return "OBJ_UNKNW_32";
	case MP2::OBJ_UNKNW_33:		return "OBJ_UNKNW_33";
	case MP2::OBJ_UNKNW_34:		return "OBJ_UNKNW_34";
	case MP2::OBJ_UNKNW_35:		return "OBJ_UNKNW_35";
	case MP2::OBJ_UNKNW_36:		return "OBJ_UNKNW_36";
	case MP2::OBJ_UNKNW_37:		return "OBJ_UNKNW_37";
	case MP2::OBJ_UNKNW_41:		return "OBJ_UNKNW_41";
	case MP2::OBJ_UNKNW_42:		return "OBJ_UNKNW_42";
	case MP2::OBJ_UNKNW_43:		return "OBJ_UNKNW_43";
	case MP2::OBJ_UNKNW_4A:		return "OBJ_UNKNW_4A";
	case MP2::OBJ_UNKNW_4B:		return "OBJ_UNKNW_4B";
	case MP2::OBJ_UNKNW_50:		return "OBJ_UNKNW_50";
	case MP2::OBJ_UNKNW_58:		return "OBJ_UNKNW_58";
	case MP2::OBJ_UNKNW_5A:		return "OBJ_UNKNW_5A";
	case MP2::OBJ_UNKNW_5C:		return "OBJ_UNKNW_5C";
	case MP2::OBJ_UNKNW_5D:		return "OBJ_UNKNW_5D";
	case MP2::OBJ_UNKNW_5F:		return "OBJ_UNKNW_5F";
	case MP2::OBJ_UNKNW_62:		return "OBJ_UNKNW_62";
	case MP2::OBJ_UNKNW_79:		return "OBJ_UNKNW_79";
	case MP2::OBJ_UNKNW_7A:		return "OBJ_UNKNW_7A";
	case MP2::OBJ_UNKNW_91:		return "OBJ_UNKNW_91";
	case MP2::OBJ_UNKNW_92:		return "OBJ_UNKNW_92";
	case MP2::OBJ_UNKNW_A1:		return "OBJ_UNKNW_A1";
	case MP2::OBJ_UNKNW_A6:		return "OBJ_UNKNW_A6";
	case MP2::OBJ_UNKNW_AA:		return "OBJ_UNKNW_AA";
	case MP2::OBJ_UNKNW_B2:		return "OBJ_UNKNW_B2";
	case MP2::OBJ_UNKNW_B8:		return "OBJ_UNKNW_B8";
	case MP2::OBJ_UNKNW_B9:		return "OBJ_UNKNW_B9";
	case MP2::OBJ_UNKNW_D1:		return "OBJ_UNKNW_D1";
	case MP2::OBJ_UNKNW_E2:		return "OBJ_UNKNW_E2";
	case MP2::OBJ_UNKNW_E3:		return "OBJ_UNKNW_E3";
	case MP2::OBJ_UNKNW_E4:		return "OBJ_UNKNW_E4";
	case MP2::OBJ_UNKNW_E5:		return "OBJ_UNKNW_E5";
	case MP2::OBJ_UNKNW_E6:		return "OBJ_UNKNW_E6";
	case MP2::OBJ_UNKNW_E7:		return "OBJ_UNKNW_E7";
	case MP2::OBJ_UNKNW_E8:		return "OBJ_UNKNW_E8";
	case MP2::OBJ_UNKNW_F9:		return "OBJ_UNKNW_F9";
	case MP2::OBJ_UNKNW_FA:		return "OBJ_UNKNW_FA";

	default:
	    DEBUG(DBG_GAME , DBG_WARN, "MP2::Object: unknown, " << static_cast<int>(object));
	    break;
    }
    
    return NULL;
}

bool MP2::isDayLife(const u8 obj)
{
    // FIXME: list day object life
    switch(obj)
    {
	case OBJ_MAGICWELL:
	    return true;

	default: break;
    }

    return false;
}

bool MP2::isWeekLife(const u8 obj)
{
    // FIXME: list week object life
    switch(obj)
    {
	case OBJ_STABLES:
        case OBJ_MAGICGARDEN:
        case OBJ_WATERWHEEL:
        case OBJ_WINDMILL:
        case OBJ_ARTESIANSPRING:

	// join army
        case OBJ_WATCHTOWER:
        case OBJ_EXCAVATION:
        case OBJ_CAVE:
        case OBJ_TREEHOUSE:
        case OBJ_ARCHERHOUSE:
        case OBJ_GOBLINHUT:
        case OBJ_DWARFCOTT:
        case OBJ_HALFLINGHOLE:
        case OBJ_PEASANTHUT:
        case OBJ_THATCHEDHUT:

        // recruit army
        case OBJ_RUINS:
        case OBJ_TREECITY:
        case OBJ_WAGONCAMP:
        case OBJ_DESERTTENT:

        // battle and recruit army
        case OBJ_DRAGONCITY:
        case OBJ_CITYDEAD:
        case OBJ_TROLLBRIDGE:

	// for AI
	case MP2::OBJ_COAST:

	    return true;

	default: break;
    }

    return false;
}

bool MP2::isMonthLife(const u8 obj)
{
    // FIXME: list month object life
    switch(obj)
    {
	// for AI
        case OBJ_CASTLE:

	default: break;
    }

    return false;
}

bool MP2::isBattleLife(const u8 obj)
{
    // FIXME: list battle object life
    switch(obj)
    {
	// luck modificators
	case OBJ_IDOL:
	case OBJ_FOUNTAIN:
	case OBJ_FAERIERING:
	case OBJ_PYRAMID:

	// morale modificators
	case OBJ_BUOY:
	case OBJ_OASIS:
	case OBJ_TEMPLE:
	case OBJ_WATERINGHOLE:
	case OBJ_GRAVEYARD:
        case OBJ_DERELICTSHIP:
        case OBJ_SHIPWRECK:

	case OBJ_MERMAID:

	    return true;

	default: break;
    }

    return false;
}

bool MP2::isActionObject(const u8 obj, const bool water)
{
    if(water) return isWaterObject(obj);

    return isGroundObject(obj);
}

bool MP2::isWaterObject(const u8 obj)
{
    switch(obj)
    {
    	    case OBJ_WATERCHEST:
	    case OBJ_DERELICTSHIP:
	    case OBJ_WHIRLPOOL:
	    case OBJ_BUOY:
	    case OBJ_BOTTLE:
	    case OBJ_SHIPWRECKSURVIROR:
	    case OBJ_FLOTSAM:
	    case OBJ_MAGELLANMAPS:
	    case OBJ_COAST:

    	    case MP2::OBJ_MERMAID:
    	    case MP2::OBJ_SIRENS:
            return true;

        default: break;
    }

    return false;
}

bool MP2::isGroundObject(const u8 obj)
{
    switch(obj)
    {
    	    case OBJ_TREASURECHEST:
    	    case OBJ_ALCHEMYLAB:
    	    case OBJ_SIGN:
    	    case OBJ_SKELETON:
    	    case OBJ_DAEMONCAVE:
    	    case OBJ_FAERIERING:
    	    case OBJ_CAMPFIRE:
    	    case OBJ_FOUNTAIN:
    	    case OBJ_GAZEBO:
    	    case OBJ_ANCIENTLAMP:
    	    case OBJ_GRAVEYARD:
    	    case OBJ_ARCHERHOUSE:
    	    case OBJ_GOBLINHUT:
    	    case OBJ_DWARFCOTT:
    	    case OBJ_PEASANTHUT:
    	    case OBJ_THATCHEDHUT:
    	    case OBJ_DRAGONCITY:
    	    case OBJ_LIGHTHOUSE:
    	    case OBJ_WATERWHEEL:
    	    case OBJ_MINES:
	    case OBJ_OBELISK:
	    case OBJ_OASIS:
	    case OBJ_RESOURCE:
	    case OBJ_SAWMILL:
	    case OBJ_ORACLE:
	    case OBJ_SHIPWRECK:
	    case OBJ_DESERTTENT:
	    case OBJ_STONELIGHTS:
	    case OBJ_WAGONCAMP:
	    case OBJ_WINDMILL:
	    case OBJ_ARTIFACT:
	    case OBJ_WATCHTOWER:
	    case OBJ_TREEHOUSE:
	    case OBJ_TREECITY:
	    case OBJ_RUINS:
	    case OBJ_FORT:
    	    case OBJ_TRADINGPOST:
    	    case OBJ_ABANDONEDMINE:
    	    case OBJ_STANDINGSTONES:
    	    case OBJ_IDOL:
    	    case OBJ_TREEKNOWLEDGE:
    	    case OBJ_DOCTORHUT:
    	    case OBJ_TEMPLE:
    	    case OBJ_HILLFORT:
    	    case OBJ_HALFLINGHOLE:
    	    case OBJ_MERCENARYCAMP:
    	    case OBJ_WATERINGHOLE:
	    case OBJ_SHRINE1:
    	    case OBJ_SHRINE2:
    	    case OBJ_SHRINE3:
    	    case OBJ_PYRAMID:
    	    case OBJ_CITYDEAD:
    	    case OBJ_EXCAVATION:
    	    case OBJ_SPHINX:
    	    case OBJ_WAGON:
    	    case OBJ_ARTESIANSPRING:
    	    case OBJ_TROLLBRIDGE:
    	    case OBJ_WITCHSHUT:
    	    case OBJ_XANADU:
    	    case OBJ_CAVE:
    	    case OBJ_LEANTO:
    	    case OBJ_MAGICWELL:
    	    case OBJ_MAGICGARDEN:
    	    case OBJ_OBSERVATIONTOWER:
    	    case OBJ_FREEMANFOUNDRY:
    	    
    	    case OBJ_MONSTER:
    	    case OBJ_CASTLE:
    	    case OBJ_BOAT:

	    case MP2::OBJ_BARRIER:
	    case MP2::OBJ_TRAVELLERTENT:
	    case MP2::OBJ_FIREALTAR:
	    case MP2::OBJ_AIRALTAR:
	    case MP2::OBJ_EARTHALTAR:
	    case MP2::OBJ_WATERALTAR:
	    case MP2::OBJ_BARROWMOUNDS:
	    case MP2::OBJ_ARENA:
	    case MP2::OBJ_JAIL:
	    case MP2::OBJ_STABLES:
	    case MP2::OBJ_ALCHEMYTOWER:
	    case MP2::OBJ_HUTMAGI:
	    case MP2::OBJ_EYEMAGI:
		return true;

	    default: break;
    }

    return false;
}

bool MP2::isQuantityObject(const u8 obj)
{
    switch(obj)
    {
        case OBJ_SKELETON:
        case OBJ_WAGON:
        case OBJ_ARTIFACT:
        case OBJ_RESOURCE:
        case OBJ_MAGICGARDEN:
        case OBJ_WATERWHEEL:
        case OBJ_WINDMILL:
        case OBJ_LEANTO:
        case OBJ_CAMPFIRE:
        case OBJ_FLOTSAM:
        case OBJ_SHIPWRECKSURVIROR:
        case OBJ_TREASURECHEST:
        case OBJ_WATERCHEST:
        case OBJ_DERELICTSHIP:
        case OBJ_SHIPWRECK:
        case OBJ_GRAVEYARD:
        case OBJ_PYRAMID:
        case OBJ_DAEMONCAVE:
        case OBJ_ABANDONEDMINE:
            return true;

        default: break;
    }

    if(isPickupObject(obj)) return true;

    return false;
}

bool MP2::isCaptureObject(const u8 obj)
{
    switch(obj)
    {
        case OBJ_MINES:
        case OBJ_ALCHEMYLAB:
        case OBJ_SAWMILL:
        case OBJ_LIGHTHOUSE:
        case OBJ_CASTLE:
	    return true;

	default: break;
    }

    return false;
}

bool MP2::isPickupObject(const u8 obj)
{
    switch(obj)
    {
	case MP2::OBJ_WATERCHEST:
	case MP2::OBJ_SHIPWRECKSURVIROR:
        case MP2::OBJ_FLOTSAM:
        case MP2::OBJ_BOTTLE:
        case MP2::OBJ_TREASURECHEST:
        case MP2::OBJ_ANCIENTLAMP:
	case MP2::OBJ_CAMPFIRE:
        case MP2::OBJ_RESOURCE:
        case MP2::OBJ_ARTIFACT:
	    return true;

	default: break;
    }

    return false;
}

bool MP2::isClearGroundObject(const u8 obj)
{
    switch(obj)
    {
	case MP2::OBJ_ZERO:
	case MP2::OBJ_COAST:
	    return true;

	default: break;
    }

    return false;
}

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
#include "gamedefs.h"
#include "race.h"
#include "heroes.h"
#include "settings.h"
#include "portrait.h"

const Surface & Portrait::Get(const HeroBase &hero, const Portrait::size_t sz)
{
    if(hero.GetType() == Skill::Primary::CAPTAIN)
        return Captain(hero.GetRace(), sz);
    else return Hero(dynamic_cast<const Heroes &>(hero), sz);
}

const Surface & Portrait::Captain(const Race::race_t rc, const Portrait::size_t sz)
{
    switch(sz)
    {
	case BIG:
	    switch(rc)
	    {
    		case Race::KNGT:        return AGG::GetICN(ICN::PORT0090, 0);
    		case Race::BARB:        return AGG::GetICN(ICN::PORT0091, 0);
    		case Race::SORC:        return AGG::GetICN(ICN::PORT0092, 0);
    		case Race::WRLK:        return AGG::GetICN(ICN::PORT0093, 0);
    		case Race::WZRD:        return AGG::GetICN(ICN::PORT0094, 0);
    		case Race::NECR:        return AGG::GetICN(ICN::PORT0095, 0);
    		default: break;
	    }
	    break;

	case MEDIUM:
	case SMALL:
	    switch(rc)
	    {
    		case Race::KNGT:        return AGG::GetICN(ICN::MINICAPT, 0);
    		case Race::BARB:        return AGG::GetICN(ICN::MINICAPT, 1);
    		case Race::SORC:        return AGG::GetICN(ICN::MINICAPT, 2);
    		case Race::WRLK:        return AGG::GetICN(ICN::MINICAPT, 3);
    		case Race::WZRD:        return AGG::GetICN(ICN::MINICAPT, 4);
    		case Race::NECR:        return AGG::GetICN(ICN::MINICAPT, 5);
    		default: break;
	    }
	    break;
    }

    DEBUG(DBG_GAME , DBG_WARN, "Portrait::Captain: unknown race.");

    return AGG::GetICN(ICN::PORT0090, 0);
}

const Surface & Portrait::Hero(const Heroes & hero, const size_t sz)
{
    switch(sz)
    {
	case BIG:	return hero.GetPortrait101x93();
	case MEDIUM:	return hero.GetPortrait50x46();
	case SMALL:	return hero.GetPortrait30x22();
	default: break;
    }

    return AGG::GetICN(ICN::PORTMEDI, 0);
}

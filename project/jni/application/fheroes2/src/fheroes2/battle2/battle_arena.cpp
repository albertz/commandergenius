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

#include <algorithm>
#include "settings.h"
#include "tools.h"
#include "army.h"
#include "cursor.h"
#include "castle.h"
#include "world.h"
#include "agg.h"
#include "speed.h"
#include "army_troop.h"
#include "server.h"
#include "remoteclient.h"
#include "battle_arena.h"
#include "battle_cell.h"
#include "battle_stats.h"
#include "battle_tower.h"
#include "battle_catapult.h"
#include "battle_bridge.h"
#include "battle_interface.h"

ICN::icn_t GetCovr(u16 ground)
{
    std::vector<ICN::icn_t> covrs;

    switch(ground)
    {
        case Maps::Ground::SNOW:
            covrs.push_back(ICN::COVR0007);
            covrs.push_back(ICN::COVR0008);
            covrs.push_back(ICN::COVR0009);
            covrs.push_back(ICN::COVR0010);
            covrs.push_back(ICN::COVR0011);
            covrs.push_back(ICN::COVR0012);
            break;

        case Maps::Ground::WASTELAND:
            covrs.push_back(ICN::COVR0019);
            covrs.push_back(ICN::COVR0020);
            covrs.push_back(ICN::COVR0021);
            covrs.push_back(ICN::COVR0022);
            covrs.push_back(ICN::COVR0023);
            covrs.push_back(ICN::COVR0024);
            break;

        case Maps::Ground::DIRT:
            covrs.push_back(ICN::COVR0013);
            covrs.push_back(ICN::COVR0014);
            covrs.push_back(ICN::COVR0015);
            covrs.push_back(ICN::COVR0016);
            covrs.push_back(ICN::COVR0017);
            covrs.push_back(ICN::COVR0018);
            break;

        case Maps::Ground::GRASS:
            covrs.push_back(ICN::COVR0001);
            covrs.push_back(ICN::COVR0002);
            covrs.push_back(ICN::COVR0003);
            covrs.push_back(ICN::COVR0004);
            covrs.push_back(ICN::COVR0005);
            covrs.push_back(ICN::COVR0006);
            break;

        default: break;
    }

    return covrs.empty() ? ICN::UNKNOWN : *Rand::Get(covrs);
}

u16 GetObstaclePosition(void)
{
    return Rand::Get(3, 6) + (11 * Rand::Get(1, 7));
}

Battle2::Board::Board()
{
}

Rect Battle2::Board::GetArea(void) const
{
    std::vector<Rect> rects;
    rects.reserve(size());
    std::vector<Cell>::const_iterator it1 = begin();
    std::vector<Cell>::const_iterator it2 = end();
    for(; it1 != it2; ++it1) rects.push_back((*it1).pos);
    return Rect(rects);
}

void Battle2::Board::SetPositionQuality(const Stats & b)
{
    iterator it1 = begin();
    iterator it2 = end();
    for(; it1 != it2; ++it1) (*it1).SetPositionQuality(b);
}


void Battle2::Board::SetEnemyQuality(const Stats & b)
{
    iterator it1 = begin();
    iterator it2 = end();
    for(; it1 != it2; ++it1) (*it1).SetEnemyQuality(b);
}

u16 Battle2::Board::GetDistance(u16 index1, u16 index2)
{
    const s16 dx = (index1 % ARENAW) - (index2 % ARENAW);
    const s16 dy = (index1 / ARENAW) - (index2 / ARENAW);

    return Sign(dx) == Sign(dy) ? std::max(std::abs(dx), std::abs(dy)) : std::abs(dx) + std::abs(dy);
}

Battle2::direction_t Battle2::Board::GetDirection(u16 from, u16 to)
{
    if(from == to) return CENTER;
    else
    for(direction_t dir = TOP_LEFT; dir < CENTER; ++dir)
    {
	if(isValidDirection(from, dir) && to == GetIndexDirection(from, dir)) return dir;
    }

    return UNKNOWN;
}

Battle2::direction_t Battle2::Board::GetReflectDirection(u8 d)
{
    switch(d)
    {
        case TOP_LEFT:		return BOTTOM_RIGHT;
        case TOP_RIGHT:		return BOTTOM_LEFT;
        case LEFT:		return RIGHT;
        case RIGHT:		return LEFT;
        case BOTTOM_LEFT:	return TOP_RIGHT;
        case BOTTOM_RIGHT:	return TOP_LEFT;
	default:		break;
    }

    return UNKNOWN;
}

bool Battle2::Board::isValidDirection(u16 i, u8 d)
{
    const u16 x = i % ARENAW;
    const u16 y = i / ARENAW;

    switch(d)
    {
	case CENTER:		break;
        case TOP_LEFT:		if(0 == y || (0 == x && (y % 2))) return false; break;
        case TOP_RIGHT:		if(0 == y || ((ARENAW - 1) == x && !(y % 2))) return false; break;
        case LEFT:		if(0 == x) return false; break;
        case RIGHT:		if((ARENAW - 1) == x) return false; break;
        case BOTTOM_LEFT:	if((ARENAH - 1) == y || (0 == x && (y % 2))) return false; break;
        case BOTTOM_RIGHT:	if((ARENAH - 1) == y || ((ARENAW - 1) == x && !(y % 2))) return false; break;
	default:		return false;
    }

    return true;
}

u16 Battle2::Board::GetIndexDirection(u16 i, u8 d)
{
    const u16 y = i / ARENAW;

    switch(d)
    {
        case TOP_LEFT:         return i - (y % 2 ? ARENAW + 1 : ARENAW);
        case TOP_RIGHT:        return i - (y % 2 ? ARENAW : ARENAW - 1);
        case LEFT:             return i - 1;
        case RIGHT:            return i + 1;
        case BOTTOM_LEFT:      return i + (y % 2 ? ARENAW - 1 : ARENAW);
        case BOTTOM_RIGHT:     return i + (y % 2 ? ARENAW : ARENAW + 1);
        default: break;
    }

    return i;
}

s16 Battle2::Board::GetIndexAbsPosition(const Point & pt) const
{
    const_iterator it1 = begin();
    const_iterator it2 = end();
    for(; it1 != it2; ++it1) if((*it1).isPositionIncludePoint(pt)) break;
    return it1 != it2 ? (*it1).GetIndex() : -1;
}

void Battle2::Board::GetIndexesFromAbsPoints(std::vector<u16> & indexes, const std::vector<Point> & points) const
{
    std::vector<Point>::const_iterator it = points.begin();
    for(; it != points.end(); ++it)
    {
	const s16 index = GetIndexAbsPosition(*it);
	if(0 <= index) indexes.push_back(index);
    }

    if(indexes.size())
    {
	std::sort(indexes.begin(), indexes.end());

	std::vector<u16>::iterator it1 = std::unique(indexes.begin(), indexes.end());
	indexes.resize(it1 - indexes.begin());
    }
}

bool Battle2::Board::inCastle(u16 ii)
{
 return((8 < ii && ii <= 10) ||
       (19 < ii && ii <= 21) ||
       (29 < ii && ii <= 32) ||
       (40 < ii && ii <= 43) ||
       (50 < ii && ii <= 54) ||
       (62 < ii && ii <= 65) ||
       (73 < ii && ii <= 76) ||
       (85 < ii && ii <= 87) ||
       (96 < ii && ii <= 98));
}

bool Battle2::Board::isMoatIndex(u16 ii)
{
    switch(ii)
    {
	case 7:
	case 18:
	case 28:
	case 39:
	case 61:
	case 72:
	case 84:
	case 95:
	    return true;

	default: break;
    }
    return false;
}

void Battle2::Board::GetAbroadPositions(u16 center, u8 radius, std::vector<u16> & positions) const
{
    if(center < size())
    {
	if(positions.size()) positions.clear();
	positions.reserve(radius * 9);

	std::vector<u16> v1, v2;
	std::vector<u16>::iterator it;

	v1.reserve(ARENASIZE / 2);
	v2.reserve(ARENASIZE / 2);

	v1.push_back(center);

	while(v1.size() && radius)
	{
	    for(it = v1.begin(); it != v1.end(); ++it)
	    {
		for(direction_t dir = TOP_LEFT; dir < CENTER; ++dir) if(isValidDirection(*it, dir))
		{
		    const u16 index = GetIndexDirection(*it, dir);
		    if(v1.end() == std::find(v1.begin(), v1.end(), index) &&
			positions.end() == std::find(positions.begin(), positions.end(), index) &&
			center != index)
		    {
			v2.push_back(index);
			positions.push_back(index);
		    }
		}
	    }

	    v1 = v2;
	    v2.clear();

	    --radius;
	}
    }
}

void Battle2::Board::SetCobjObjects(s32 center)
{
//    bool trees = Maps::ScanAroundObject(center, MP2::OBJ_TREES, false);
    const Heroes* hero = world.GetHeroes(center);
    bool grave = hero && MP2::OBJ_GRAVEYARD == hero->GetUnderObject();
    u16 ground = world.GetTiles(center).GetGround();
    std::vector<ICN::icn_t> objs;

    if(grave)
    {
	objs.push_back(ICN::COBJ0000);
	objs.push_back(ICN::COBJ0001);
    }
    else
    switch(ground)
    {
	case Maps::Ground::DESERT:
	    objs.push_back(ICN::COBJ0009);
	    objs.push_back(ICN::COBJ0012);
	    objs.push_back(ICN::COBJ0017);
	    objs.push_back(ICN::COBJ0024);
	    break;

        case Maps::Ground::SNOW:
	    objs.push_back(ICN::COBJ0022);
	    objs.push_back(ICN::COBJ0026);
	    break;

        case Maps::Ground::SWAMP:
	    objs.push_back(ICN::COBJ0006);
	    objs.push_back(ICN::COBJ0015);
	    objs.push_back(ICN::COBJ0016);
	    objs.push_back(ICN::COBJ0019);
	    objs.push_back(ICN::COBJ0025);
	    objs.push_back(ICN::COBJ0027);
	    break;

        case Maps::Ground::BEACH:
	    objs.push_back(ICN::COBJ0017);
	    break;

        case Maps::Ground::DIRT:
	    objs.push_back(ICN::COBJ0011);
        case Maps::Ground::GRASS:
	    objs.push_back(ICN::COBJ0002);
	    objs.push_back(ICN::COBJ0004);
	    objs.push_back(ICN::COBJ0005);
	    objs.push_back(ICN::COBJ0008);
	    objs.push_back(ICN::COBJ0012);
	    objs.push_back(ICN::COBJ0028);
	    break;

        case Maps::Ground::WASTELAND:
	    objs.push_back(ICN::COBJ0013);
	    objs.push_back(ICN::COBJ0018);
	    objs.push_back(ICN::COBJ0020);
	    objs.push_back(ICN::COBJ0021);
	    break;

        case Maps::Ground::LAVA:
	    objs.push_back(ICN::COBJ0007);
	    objs.push_back(ICN::COBJ0029);
	    objs.push_back(ICN::COBJ0030);
	    objs.push_back(ICN::COBJ0031);
	    break;

        case Maps::Ground::WATER:
	    objs.push_back(ICN::COBJ0003);
	    objs.push_back(ICN::COBJ0010);
	    objs.push_back(ICN::COBJ0023);
	    break;

	default: break;
    }

    if(objs.size() && 2 < Rand::Get(1, 10))
    {
	// 80% 1 obj
	u16 dst = GetObstaclePosition();
	SetCobjObject(*Rand::Get(objs), dst);

	// 50% 2 obj
	while(at(dst).object) dst = GetObstaclePosition();
	if(objs.size() > 1 && 5 < Rand::Get(1, 10)) SetCobjObject(*Rand::Get(objs), dst);

	// 30% 3 obj
        while(at(dst).object) dst = GetObstaclePosition();
	if(objs.size() > 1 && 7 < Rand::Get(1, 10)) SetCobjObject(*Rand::Get(objs), dst);
    }
}

void Battle2::Board::SetCobjObject(u16 icn, u16 dst)
{
    switch(icn)
    {
	case ICN::COBJ0000:	at(dst).object = 0x80; break;
	case ICN::COBJ0001:	at(dst).object = 0x81; break;
	case ICN::COBJ0002:	at(dst).object = 0x82; break;
	case ICN::COBJ0003:	at(dst).object = 0x83; break;
	case ICN::COBJ0004:	at(dst).object = 0x84; break;
	case ICN::COBJ0005:	at(dst).object = 0x85; break;
	case ICN::COBJ0006:	at(dst).object = 0x86; break;
	case ICN::COBJ0007:	at(dst).object = 0x87; break;
	case ICN::COBJ0008:	at(dst).object = 0x88; break;
	case ICN::COBJ0009:	at(dst).object = 0x89; break;
	case ICN::COBJ0010:	at(dst).object = 0x8A; break;
	case ICN::COBJ0011:	at(dst).object = 0x8B; break;
	case ICN::COBJ0012:	at(dst).object = 0x8C; break;
	case ICN::COBJ0013:	at(dst).object = 0x8D; break;
	case ICN::COBJ0014:	at(dst).object = 0x8E; break;
	case ICN::COBJ0015:	at(dst).object = 0x8F; break;
	case ICN::COBJ0016:	at(dst).object = 0x90; break;
	case ICN::COBJ0017:	at(dst).object = 0x91; break;
	case ICN::COBJ0018:	at(dst).object = 0x92; break;
	case ICN::COBJ0019:	at(dst).object = 0x93; break;
	case ICN::COBJ0020:	at(dst).object = 0x94; break;
	case ICN::COBJ0021:	at(dst).object = 0x95; break;
	case ICN::COBJ0022:	at(dst).object = 0x96; break;
	case ICN::COBJ0023:	at(dst).object = 0x97; break;
	case ICN::COBJ0024:	at(dst).object = 0x98; break;
	case ICN::COBJ0025:	at(dst).object = 0x99; break;
	case ICN::COBJ0026:	at(dst).object = 0x9A; break;
	case ICN::COBJ0027:	at(dst).object = 0x9B; break;
	case ICN::COBJ0028:	at(dst).object = 0x9C; break;
	case ICN::COBJ0029:	at(dst).object = 0x9D; break;
	case ICN::COBJ0030:	at(dst).object = 0x9E; break;
	case ICN::COBJ0031:	at(dst).object = 0x9F; break;

	default: break;
    }

    switch(icn)
    {
	case ICN::COBJ0004:
	case ICN::COBJ0005:
	case ICN::COBJ0007:
	case ICN::COBJ0011:
	case ICN::COBJ0014:
	case ICN::COBJ0015:
	case ICN::COBJ0017:
	case ICN::COBJ0018:
	case ICN::COBJ0019:
	case ICN::COBJ0020:
	case ICN::COBJ0022:
	case ICN::COBJ0030:
	case ICN::COBJ0031:
	    at(dst + 1).object = 0x40;
	    break;

	default: break;
    }
}

void Battle2::Board::SetCovrObjects(u16 icn)
{
    switch(icn)
    {
        case ICN::COVR0001:
        case ICN::COVR0007:
        case ICN::COVR0013:
        case ICN::COVR0019:
	    at(15).object = 0x40;
	    at(16).object = 0x40;
	    at(17).object = 0x40;
	    at(25).object = 0x40;
	    at(26).object = 0x40;
	    at(27).object = 0x40;
	    at(28).object = 0x40;
	    at(40).object = 0x40;
	    at(51).object = 0x40;
	    break;

        case ICN::COVR0002:
        case ICN::COVR0008:
        case ICN::COVR0014:
        case ICN::COVR0020:
	    at(47).object = 0x40;
	    at(48).object = 0x40;
	    at(49).object = 0x40;
	    at(50).object = 0x40;
	    at(51).object = 0x40;
	    break;

        case ICN::COVR0003:
        case ICN::COVR0009:
        case ICN::COVR0015:
        case ICN::COVR0021:
	    at(35).object = 0x40;
	    at(41).object = 0x40;
	    at(46).object = 0x40;
	    at(47).object = 0x40;
	    at(48).object = 0x40;
	    at(49).object = 0x40;
	    at(50).object = 0x40;
	    at(51).object = 0x40;
	    break;

        case ICN::COVR0004:
        case ICN::COVR0010:
        case ICN::COVR0016:
        case ICN::COVR0022:
	    at(41).object = 0x40;
	    at(51).object = 0x40;
	    at(58).object = 0x40;
	    at(59).object = 0x40;
	    at(60).object = 0x40;
	    at(61).object = 0x40;
	    at(62).object = 0x40;
	    break;

        case ICN::COVR0005:
        case ICN::COVR0017:
	    at(24).object = 0x40;
	    at(25).object = 0x40;
	    at(26).object = 0x40;
	    at(27).object = 0x40;
	    at(28).object = 0x40;
	    at(29).object = 0x40;
	    at(30).object = 0x40;
	    at(58).object = 0x40;
	    at(59).object = 0x40;
	    at(60).object = 0x40;
	    at(61).object = 0x40;
	    at(62).object = 0x40;
	    at(63).object = 0x40;
	    at(68).object = 0x40;
	    at(74).object = 0x40;
	    break;

        case ICN::COVR0006:
        case ICN::COVR0018:
	    at(14).object = 0x40;
	    at(15).object = 0x40;
	    at(16).object = 0x40;
	    at(17).object = 0x40;
	    at(18).object = 0x40;
	    at(24).object = 0x40;
	    at(68).object = 0x40;
	    at(80).object = 0x40;
	    at(81).object = 0x40;
	    at(82).object = 0x40;
	    at(83).object = 0x40;
	    at(84).object = 0x40;
	    break;

        case ICN::COVR0011:
        case ICN::COVR0023:
	    at(15).object = 0x40;
	    at(25).object = 0x40;
	    at(36).object = 0x40;
	    at(51).object = 0x40;
	    at(62).object = 0x40;
	    at(71).object = 0x40;
	    at(72).object = 0x40;
	    break;

        case ICN::COVR0012:
        case ICN::COVR0024:
	    at(18).object = 0x40;
	    at(29).object = 0x40;
	    at(41).object = 0x40;
	    at(59).object = 0x40;
	    at(70).object = 0x40;
	    at(82).object = 0x40;
	    at(83).object = 0x40;
	    break;

	default: break;
    }
}

Battle2::GraveyardTroop::GraveyardTroop(const Arena & a) : arena(a)
{
}

void Battle2::GraveyardTroop::GetClosedCells(std::vector<u16> & v) const
{
    v.clear();
    std::map<u16, std::vector<u16> >::const_iterator it1 = map.begin();
    std::map<u16, std::vector<u16> >::const_iterator it2 = map.end();
    for(; it1 != it2; ++it1) v.push_back((*it1).first);
}

u16 Battle2::GraveyardTroop::GetLastTroopIDFromCell(u16 index) const
{
    std::map<u16, std::vector<u16> >::const_iterator it1 = map.begin();
    std::map<u16, std::vector<u16> >::const_iterator it2 = map.end();
    for(; it1 != it2; ++it1) if(index == (*it1).first) break;

    return it1 != it2 && (*it1).second.size() ? (*it1).second.back() : 0;
}

void Battle2::GraveyardTroop::AddTroopID(u16 id)
{
    const Stats* b = arena.GetTroopID(id);
    if(b)
    {
	std::vector<u16> & v = map[b->position];
	if(v.empty()) v.reserve(3);
	v.push_back(id);

	if(b->isWide())
	{
	    std::vector<u16> & v2 = map[b->GetTailIndex()];
	    if(v2.empty()) v2.reserve(3);
	    v2.push_back(id);
	}
    }
}

void Battle2::GraveyardTroop::RemoveTroopID(u16 id)
{
    const Stats* b = arena.GetTroopID(id);
    if(b)
    {
	std::vector<u16> & v = map[b->position];
	std::vector<u16>::iterator it = std::find(v.begin(), v.end(), id);
	if(it != v.end()) v.erase(it);

	if(b->isWide())
	{
	    std::vector<u16> & v2 = map[b->GetTailIndex()];
	    std::vector<u16>::iterator it = std::find(v2.begin(), v2.end(), id);
	    if(it != v2.end()) v2.erase(it);
	}
    }
}

Battle2::Arena::Arena(Army::army_t & a1, Army::army_t & a2, s32 index, bool local) :
	army1(a1), army2(a2), castle(NULL), current_commander(NULL), catapult(NULL), bridge(NULL), interface(NULL), result_game(NULL), graveyard(*this),
	icn_covr(ICN::UNKNOWN)
{
    const Settings & conf = Settings::Get();
    usage_spells.reserve(10);

    interface = local ? new Interface(*this, index) : NULL;

    if(interface && conf.Sound())
	AGG::PlaySound(M82::PREBATTL);

    board.reserve(ARENASIZE);
    for(u16 ii = 0; ii < ARENASIZE; ++ii) board.push_back(Cell(ii, (interface ? &interface->GetArea() : NULL), *this));

    for(u8 ii = 0; ii < army1.Size(); ++ii)
    {
	// place army1 troops
	const u8 index1 = Army::FORMAT_SPREAD == army1.GetCombatFormat() ? ii * 22 : 22 + ii * 11;
	Army::Troop & troop1 = army1.At(ii);
	if(troop1.isValid() && troop1.BattleInit())
	{
	    Stats* stats = troop1.GetBattleStats();
	    stats->SetPosition(troop1.isWide() ? index1 + 1 : index1);
	    stats->SetArena(this);
	    stats->SetReflection(false);
	    if(interface) stats->InitContours();
	}
    }

    for(u8 ii = 0; ii < army2.Size(); ++ii)
    {
    	// place army2 troops
	const u8 index2 = Army::FORMAT_SPREAD == army2.GetCombatFormat() ? 10 + ii * 22 : 32 + ii * 11;
	Army::Troop & troop2 = army2.At(ii);
	if(troop2.isValid() && troop2.BattleInit())
	{
	    Stats* stats = troop2.GetBattleStats();
	    stats->SetPosition(index2);
	    stats->SetArena(this);
	    stats->SetReflection(true);
	    if(interface) stats->InitContours();
	}
    }

    castle = world.GetCastle(index);
    if(castle && (castle->GetIndex() != index || !castle->isCastle())) castle = NULL;
    towers[0] = NULL;
    towers[1] = NULL;
    towers[2] = NULL;

    if(castle)
    {
	// init
	towers[0] = castle->isBuild(BUILD_LEFTTURRET) ? new Tower(*castle, TWR_LEFT, *this) : NULL;
	towers[1] = new Tower(*castle, TWR_CENTER, *this);
	towers[2] = castle->isBuild(BUILD_RIGHTTURRET) ? new Tower(*castle, TWR_RIGHT, *this) : NULL;
	bool fortification = (Race::KNGT == castle->GetRace()) && castle->isBuild(BUILD_SPEC);
	catapult = army1.GetCommander() ? new Catapult(*army1.GetCommander(), fortification, *this) : NULL;
	bridge = new Bridge(*this);

	// catapult cell
	board[77].object = 1;

	// wall (3,2,1,0)
	board[8].object =  fortification ? 3 : 2;
	board[29].object = fortification ? 3 : 2;
	board[73].object = fortification ? 3 : 2;
	board[96].object = fortification ? 3 : 2;

	// tower
	board[40].object = 2;
	board[62].object = 2;

	// archers tower
	board[19].object = 2;
        board[85].object = 2;

	// bridge
	board[49].object = 1;
	board[50].object = 1;
    }
    else
    // set obstacles
    {
	icn_covr = Maps::ScanAroundObject(index, MP2::OBJ_CRATER) ? GetCovr(world.GetTiles(index).GetGround()) : ICN::UNKNOWN;

	if(icn_covr != ICN::UNKNOWN)
	    board.SetCovrObjects(icn_covr);
	else
	    board.SetCobjObjects(index);
    }

#ifdef WITH_NET
    if(Network::isRemoteClient())
    {
    	if(Game::REMOTE == army1.GetControl()) FH2RemoteClient::SendBattleBoard(army1.GetColor(), *this);
	if(Game::REMOTE == army2.GetControl()) FH2RemoteClient::SendBattleBoard(army2.GetColor(), *this);
    }
#endif

    //
    if(interface)
    {
	Cursor & cursor = Cursor::Get();
	Display & display = Display::Get();

	cursor.Hide();
	if(conf.ExtUseFade()) display.Fade();
	cursor.SetThemes(Cursor::WAR_NONE);
	interface->Redraw();
	cursor.Show();
	display.Flip();

	// pause for play M82::PREBATTL
	if(conf.Sound())
	    while(LocalEvent::Get().HandleEvents() && Mixer::isPlaying(-1));
    }
}

Battle2::Arena::~Arena()
{
    if(towers[0]) delete towers[0];
    if(towers[1]) delete towers[1];
    if(towers[2]) delete towers[2];

    if(catapult) delete catapult;
    if(interface) delete interface;
}

Battle2::Interface* Battle2::Arena::GetInterface(void)
{
    return interface;
}

void Battle2::Arena::Turns(u16 turn, Result & result)
{
    DEBUG(DBG_BATTLE , DBG_TRACE, "Battle2::Arena::Turns: " << turn);

    result_game = &result;

    army1.BattleNewTurn();
    army2.BattleNewTurn();

    Actions actions;
    Stats* current_troop = NULL;
    u8 equal_color = 0;
    u8 current_color = 0;
    bool tower_moved = false;
    bool catapult_moved = false;

    // turn
    while(1)
    {
	// check exit
    	if(!army1.isValid() || !army2.isValid() || result.army1 || result.army2) break;

	if(NULL == current_troop)
	{
	    Battle2::Stats* btroop1 = army1.BattleFastestTroop(false);
	    Battle2::Stats* btroop2 = army2.BattleFastestTroop(false);

	    if(btroop1 && btroop2)
	    {
		if(btroop1->GetSpeed() > btroop2->GetSpeed())
		{
		    current_troop = btroop1;
		    current_color = army1.GetColor();
		    equal_color = 0;
		}
		else
		if(btroop1->GetSpeed() < btroop2->GetSpeed())
		{
		    current_troop = btroop2;
		    current_color = army2.GetColor();
		    equal_color = 0;
		}
		else
		// equal speed
		{
		    // first attacker moved
		    if(0 == equal_color)
		    {
			current_troop = btroop1;
			current_color = army1.GetColor();
			equal_color = army1.GetColor();
		    }
		    else
		    // changed
		    if(equal_color == army1.GetColor())
		    {
			current_troop = btroop2;
			current_color = army2.GetColor();
			equal_color = army2.GetColor();
		    }
		    else
		    {
			current_troop = btroop1;
			current_color = army1.GetColor();
			equal_color = army1.GetColor();
		    }
		}
	    }
	    else
	    if(btroop1)
	    {
	    	current_troop = btroop1;
		current_color = army1.GetColor();
	    }
	    else
	    if(btroop2)
	    {
	    	current_troop = btroop2;
		current_color = army2.GetColor();
	    }
	    else
	    {
		btroop1 = army1.BattleSlowestTroop(true);
		btroop2 = army2.BattleSlowestTroop(true);

		if(btroop1 && btroop2)
		{
		    if(btroop1->GetSpeed() < btroop2->GetSpeed())
		    {
		    	current_troop = btroop1;
			current_color = army1.GetColor();
			equal_color = 0;
		    }
		    else
		    if(btroop1->GetSpeed() > btroop2->GetSpeed())
		    {
		    	current_troop = btroop2;
			current_color = army2.GetColor();
			equal_color = 0;
		    }
		    else
		    // equal speed
		    {
			// first defender moved (attacker have priority)
			if(0 == equal_color)
			{
			    current_troop = btroop2;
			    current_color = army2.GetColor();
			    equal_color = army2.GetColor();
			}
			else
			// changed
			if(equal_color == army1.GetColor())
			{
			    current_troop = btroop2;
			    current_color = army2.GetColor();
			    equal_color = army2.GetColor();
			}
			else
			{
			    current_troop = btroop1;
			    current_color = army1.GetColor();
			    equal_color = army1.GetColor();
			}
		    }
		}
		else
		if(btroop1)
		{
		    current_troop = btroop1;
		    current_color = army1.GetColor();
		}
		else
		if(btroop2)
		{
		    current_troop = btroop2;
		    current_color = army2.GetColor();
		}
	    }

	    // end turns
	    if(!current_troop) break;
	}

	DEBUG(DBG_BATTLE , DBG_TRACE, "Battle2::Arena::Turns: " << current_troop->GetName() << ", color: " << \
	    Color::String(current_troop->GetColor()) << ", speed: " << Speed::String(current_troop->GetSpeed()) << "(" << static_cast<int>(current_troop->GetSpeed()) << ")");

	current_commander = current_troop->GetCommander();

	// first turn: castle and catapult action
	if(castle)
	{
	    if(!catapult_moved && current_color == army1.GetColor())
	    {
		catapult->Action();
		catapult_moved = true;
	    }

	    if(!tower_moved && current_color == army2.GetColor())
	    {
		if(towers[0] && towers[0]->isValid()) towers[0]->Action();
		if(towers[1] && towers[1]->isValid()) towers[1]->Action();
		if(towers[2] && towers[2]->isValid()) towers[2]->Action();
		tower_moved = true;

    		// check dead last army from towers
    		if(!army1.isValid() || !army2.isValid() || result.army1 || result.army2) break;
	    }
	}

	// set bridge passable
	if(bridge && bridge->isValid() && !bridge->isDown()) bridge->SetPassable(*current_troop);

	// bad morale
	if(current_troop->Modes(MORALE_BAD))
	{
    	    actions.AddedMoraleAction(*current_troop, false);
	}
	else
	// turn opponents
	switch(current_troop->GetControl())
	{
    	    case Game::REMOTE:	RemoteTurn(*current_troop, actions); break;
    	    case Game::LOCAL:   HumanTurn(*current_troop, actions); break;
    	    default:		AITurn(*current_troop, actions); break;
	}

	// apply task
	while(actions.size())
	{
	    bool check_morale = (MSG_BATTLE_END_TURN == actions.front().GetID());

	    ApplyAction(actions.front());
	    actions.pop_front();

	    // good morale
	    if(check_morale && !current_troop->Modes(TR_SKIPMOVE) &&
		current_troop->Modes(TR_MOVED) && current_troop->Modes(MORALE_GOOD) && army1.isValid() && army2.isValid() &&
		0 == result.army1 && 0 == result.army2)
		actions.AddedMoraleAction(*current_troop, true);
	}

	ResetBoard();

	// current troop moved!
	if(current_troop->Modes(TR_SKIPMOVE | TR_MOVED)) current_troop = NULL;

	DELAY(10);
    }

    // end turn: fix result
    if(!army1.isValid() || (result.army1 & (RESULT_RETREAT | RESULT_SURRENDER)))
    {
	result.army1 |= RESULT_LOSS;
	if(army2.isValid()) result.army2 = RESULT_WINS;
    }

    if(!army2.isValid() || (result.army2 & (RESULT_RETREAT | RESULT_SURRENDER)))
    {
	result.army2 |= RESULT_LOSS;
	if(army1.isValid()) result.army1 = RESULT_WINS;
    }
}

void Battle2::Arena::RemoteTurn(const Stats & b, Actions & a)
{
#ifdef WITH_NET
    if(current_commander)
    {
	FH2RemoteClient* remote = FH2Server::Get().GetRemoteClient(current_commander->GetColor());
	if(remote)
	{
	    remote->RecvBattleHumanTurn(b, *this, a);
	    return;
	}
	DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::RemoteTurn: " << "remote client is NULL");
    }
    else
    {
	DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::RemoteTurn: " << "current commander is NULL");
    }
#endif
    DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::RemoteTurn: " << "AI turn");
    AITurn(b, a);
}

void Battle2::Arena::HumanTurn(const Stats & b, Actions & a)
{
    Settings & conf = Settings::Get();

    conf.SetMyColor(Color::Get(b.GetColor()));

    if(conf.AutoBattle())
        AITurn(b, a);
    else
    if(interface)
        interface->HumanTurn(b, a);
}

const Battle2::Cell* Battle2::Arena::GetCell(u16 position, direction_t dir) const
{
    if(position < board.size())
    {
	if(dir == CENTER || dir == UNKNOWN)
	    return &board[position];
	else
	if(Board::isValidDirection(position, dir))
	    return &board[Board::GetIndexDirection(position, dir)];
    }
    return NULL;
}

Battle2::Cell* Battle2::Arena::GetCell(u16 position, direction_t dir)
{
    if(position < board.size())
    {
	if(dir == CENTER || dir == UNKNOWN)
	    return &board[position];
	else
	if(Board::isValidDirection(position, dir))
	    return &board[Board::GetIndexDirection(position, dir)];
    }
    return NULL;
}

void Battle2::Arena::ResetBoard(void)
{
    std::for_each(board.begin(), board.end(), std::mem_fun_ref(&Cell::Reset));
}

void Battle2::Arena::ScanPassabilityBoard(const Stats & b, bool skip_speed)
{
    u16 from = b.GetPosition();
    u8 max_turn = skip_speed ? 255 : b.GetSpeed();

    if(b.isFly())
    {
	std::vector<Cell>::iterator it = board.begin();
	for(; it != board.end(); ++it) if((*it).isPassable(b, false)) (*it).direction = CENTER;
    }
    else
    {
	std::vector<u16> v1, v2;
	std::vector<u16>::iterator it;

        v1.reserve(ARENASIZE / 2);
	v2.reserve(ARENASIZE / 2);

	v1.push_back(from);
	board[from].direction = CENTER;

	while(v1.size() && max_turn)
	{
	    for(it = v1.begin(); it != v1.end(); ++it) board[*it].SetPassabilityAbroad(b, v2);

	    v1.clear();
	    for(it = v2.begin(); it != v2.end(); ++it)
	    {
		if(!skip_speed && castle)
		{
		    // skip moat positions
		    if((castle->isBuild(BUILD_MOAT) && Board::isMoatIndex(*it)) ||
		    // skip bridge position
		       (Bridge::isIndex(*it) && !bridge->isPassable(b.GetColor()))) continue;
		}
		v1.push_back(*it);
	    }
	    v2.clear();

	    --max_turn;
	}
    }
}

u16 Battle2::Arena::GetShortDistance(u16 from, const std::vector<u16> & dst)
{
    u16 len = MAXU16;
    u16 res = MAXU16;

    for(std::vector<u16>::const_iterator it = dst.begin(); it != dst.end(); ++it)
    {
	const u16 length = Board::GetDistance(from, *it);
	if(len > length)
	{
	    len = length;
	    res = *it;
	}
    }

    DEBUG(DBG_BATTLE, DBG_TRACE, "Battle2::Arena::GetShortDistance: " << res);

    return res;
}

u16 Battle2::Arena::GetPath(const Stats & b, u16 to, std::vector<u16> & v)
{
    if(v.size()) v.clear();
    v.reserve(15);

    if(b.GetPosition() < board.size())
    {
	ScanPassabilityBoard(b, true);

	u16 cur = to;
	while(CENTER != board[cur].direction && UNKNOWN != board[cur].direction){ v.push_back(cur); cur = Board::GetIndexDirection(cur, board[cur].direction); }
    }

    std::reverse(v.begin(), v.end());

    // correct distance movement
    if(v.size() > b.GetSpeed()) v.resize(b.GetSpeed());

    // correct moat present
    if(castle && castle->isBuild(BUILD_MOAT))
    {
	std::vector<u16>::iterator it = std::find_if(v.begin(), v.end(), Board::isMoatIndex);
	if(it != v.end()) v.resize(std::distance(v.begin(), it) + 1);
    }

    if(IS_DEBUG(DBG_BATTLE, DBG_TRACE))
    {
	std::cout << "Battle2::Arena::GetPath: from: " << b.GetPosition() << ", to: " << to << " :: ";
	for(u16 ii = 0; ii < v.size(); ++ii) std::cout << v[ii] << ", ";
	std::cout << std::endl;
    }

    return v.size();
}

void Battle2::Arena::GetPassableQualityPositions(const Stats & b, std::vector<u16> & v)
{
    ScanPassabilityBoard(b, true);

    std::vector<Cell>::iterator it;
    v.clear();

    for(it = board.begin(); it != board.end(); ++it)
	if((*it).isPassable(b, false) && (*it).quality && UNKNOWN != (*it).direction)
	    v.push_back((*it).index);

    if(IS_DEBUG(DBG_BATTLE, DBG_TRACE))
    {
        std::cout << "Battle2::Arena::GetPassableQualityPositions: ";
        if(v.empty()) std::cout << "empty";
        for(u16 ii = 0; ii < v.size(); ++ii) std::cout << v[ii] << ", ";
        std::cout << std::endl;
    }
}

Battle2::Stats* Battle2::Arena::GetTroopBoard(u16 index)
{
    for(u8 ii = 0; ii < army1.Size(); ++ii)
	if(army1.At(ii).isValid() &&
	    (army1.At(ii).GetBattleStats()->position == index ||
		(army1.At(ii).isWide() && army1.At(ii).GetBattleStats()->GetTailIndex() == index))) return army1.At(ii).GetBattleStats();

    for(u8 ii = 0; ii < army2.Size(); ++ii)
	if(army2.At(ii).isValid() &&
	    (army2.At(ii).GetBattleStats()->position == index ||
		(army2.At(ii).isWide() && army2.At(ii).GetBattleStats()->GetTailIndex() == index))) return army2.At(ii).GetBattleStats();

    return NULL;
}

const Battle2::Stats* Battle2::Arena::GetTroopBoard(u16 index) const
{
    for(u8 ii = 0; ii < army1.Size(); ++ii)
	if(army1.At(ii).isValid() && army1.At(ii).GetBattleStats()->position == index) return army1.At(ii).GetBattleStats();

    for(u8 ii = 0; ii < army2.Size(); ++ii)
	if(army2.At(ii).isValid() && army2.At(ii).GetBattleStats()->position == index) return army2.At(ii).GetBattleStats();

    return NULL;
}

const Army::army_t* Battle2::Arena::GetArmy(u8 color) const
{
    if(army1.GetColor() == color) return &army1;
    else
    if(army2.GetColor() == color) return &army2;

    return NULL;
}

Army::army_t* Battle2::Arena::GetArmy(u8 color)
{
    if(army1.GetColor() == color) return &army1;
    else
    if(army2.GetColor() == color) return &army2;

    return NULL;
}

void Battle2::Arena::GetArmyPositions(u8 color, std::vector<u16> & res) const
{
    if(res.size()) res.clear();

    const Army::army_t* army = GetArmy(color);
    if(army)
    {
	for(u8 ii = 0; ii < army->Size(); ++ii)
	    if(army->At(ii).isValid()) res.push_back(army->At(ii).GetBattleStats()->GetPosition());
    }
}

Battle2::Stats* Battle2::Arena::GetTroopID(u16 id)
{
    Army::army_t* army = GetArmy(id >> 8);
    return army && army->Size() > (0x000F & id) ? army->At(0x000F & id).GetBattleStats() : NULL;
}

const Battle2::Stats* Battle2::Arena::GetTroopID(u16 id) const
{
    const Army::army_t* army = GetArmy(id >> 8);
    return army && army->Size() > (0x000F & id) ? army->At(0x000F & id).GetBattleStats() : NULL;
}

const Battle2::Stats* Battle2::Arena::GetEnemyAbroadMaxQuality(u16 position, u8 color) const
{
    const Stats* res = NULL;
    const Stats* enemy = NULL;
    const Cell* cell = NULL;
    s32 quality = 0;

    for(direction_t dir = TOP_LEFT; dir < CENTER; ++dir)
    {
        cell = GetCell(position, dir);
        if(cell && NULL != (enemy = GetTroopBoard(cell->index)) &&
            enemy->GetColor() != color && quality < cell->quality)
        {
            res = enemy;
            quality = cell->quality;
        }
    }

    return res;
}

u16 Battle2::Arena::GetMaxQualityPosition(const std::vector<u16> & positions) const
{
    std::vector<u16>::const_iterator it1 = positions.begin();
    std::vector<u16>::const_iterator it2 = positions.end();
    u16 res = MAXU16;

    for(; it1 != it2; ++it1) if(board.size() > *it1)
    {
	if(res == MAXU16 ||
	    board[res].quality < board[*it1].quality) res = *it1;
    }

    return res;
}

const Battle2::Stats* Battle2::Arena::GetEnemyAbroadMaxQuality(const Stats & b) const
{
    const Stats* res1 = GetEnemyAbroadMaxQuality(b.GetPosition(), b.GetColor());

    if(b.isWide())
    {
	const Stats* res2 = GetEnemyAbroadMaxQuality(b.GetTailIndex(), b.GetColor());

	if(!res1) return res2;
	else
	if(!res2) return res1;
	else
	return res1->GetCellQuality() > res2->GetCellQuality() ? res1 : res2;
    }

    return res1;
}

Battle2::Stats* Battle2::Arena::GetEnemyAbroadMaxQuality(u16 position, u8 color)
{
    Stats* res = NULL;
    Stats* enemy = NULL;
    Cell* cell = NULL;
    s32 quality = 0;

    for(direction_t dir = TOP_LEFT; dir < CENTER; ++dir)
    {
        cell = GetCell(position, dir);
        if(cell && NULL != (enemy = GetTroopBoard(cell->index)) &&
            enemy->GetColor() != color && quality < cell->quality)
        {
            res = enemy;
            quality = cell->quality;
        }
    }

    return res;
}

Battle2::Stats* Battle2::Arena::GetEnemyAbroadMaxQuality(const Stats & b)
{
    Stats* res1 = GetEnemyAbroadMaxQuality(b.GetPosition(), b.GetColor());

    if(b.isWide())
    {
	Stats* res2 = GetEnemyAbroadMaxQuality(b.GetTailIndex(), b.GetColor());

	if(!res1) return res2;
	else
	if(!res2) return res1;
	else
	return res1->GetCellQuality() > res2->GetCellQuality() ? res1 : res2;
    }

    return res1;
}

const Battle2::Stats* Battle2::Arena::GetEnemyMaxQuality(u8 my_color) const
{
    const Stats* res = NULL;
    s32 quality = 0;
    Board::const_iterator it1 = board.begin();
    Board::const_iterator it2 = board.end();
    for(; it1 != it2; ++it1)
    {
	const Stats* enemy = GetTroopBoard((*it1).index);
        if(enemy && enemy->isWide() && enemy->GetTailIndex() == (*it1).index) continue;
        if(enemy && enemy->GetColor() != my_color && quality < (*it1).quality)
        {
            res = enemy;
            quality = (*it1).quality;
        }
    }
    return res;
}

Battle2::Stats* Battle2::Arena::GetEnemyMaxQuality(u8 my_color)
{
    Stats* res = NULL;
    s32 quality = 0;
    Board::iterator it1 = board.begin();
    Board::iterator it2 = board.end();
    for(; it1 != it2; ++it1)
    {
	Stats* enemy = GetTroopBoard((*it1).index);
        if(enemy && enemy->isWide() && enemy->GetTailIndex() == (*it1).index) continue;
        if(enemy && enemy->GetColor() != my_color && quality < (*it1).quality)
        {
            res = enemy;
            quality = (*it1).quality;
        }
    }
    return res;
}

u8 Battle2::Arena::GetOppositeColor(u8 color) const
{
    if(army1.GetColor() == color) return army2.GetColor();
    else
    if(army2.GetColor() == color) return army1.GetColor();

    return 0;
}

void Battle2::Arena::FadeArena(void) const
{
    if(interface) interface->FadeArena();
}

const std::vector<u8> & Battle2::Arena::GetUsageSpells(void) const
{
    return usage_spells;
}

void Battle2::Arena::AddSpell(u8 spell)
{
    if(usage_spells.end() == std::find(usage_spells.begin(), usage_spells.end(), spell)) usage_spells.push_back(spell);
}

u16 Battle2::Arena::GetFreePositionNearHero(u8 color) const
{
    const u8 cells1[] = { 11, 22, 33 };
    const u8 cells2[] = { 21, 32, 43 };
    const u8* cells = NULL;

    if(army1.GetColor() == color) cells = cells1;
    else
    if(army2.GetColor() == color) cells = cells2;

    if(cells) for(u8 ii = 0; ii < 3; ++ii) if(board[cells[ii]].isPassable() && NULL == GetTroopBoard(cells[ii])) return cells[ii];

    return 0;
}

void Battle2::Arena::DumpBoard(void) const
{
    Board::const_iterator it1 = board.begin();
    Board::const_iterator it2 = board.end();
    for(; it1 != it2; ++it1)
    {
	const Battle2::Stats* b = GetTroopBoard((*it1).index);
	if(b) b->Dump();
    }
}

struct IndexDistanceEqualDistance : std::binary_function<IndexDistance, u16, bool>
{
    bool operator() (const IndexDistance & id, u16 dist) const { return id.second == dist; };
};

u16 Battle2::Arena::GetNearestTroops(u16 pos, std::vector<u16> & res, const std::vector<u16>* black) const
{
    std::vector<IndexDistance> dists;
    dists.reserve(15);

    Board::const_iterator it1 = board.begin();
    Board::const_iterator it2 = board.end();
    for(; it1 != it2; ++it1)
    {
	const Battle2::Stats* b = GetTroopBoard((*it1).index);
	if(b)
	{
	    // check black list
	    if(black && black->end() != std::find(black->begin(), black->end(), b->GetPosition())) continue;
	    // added
	    if(pos != b->GetPosition()) dists.push_back(IndexDistance(b->GetPosition(), Board::GetDistance(pos, b->GetPosition())));
	}
    }

    if(1 < dists.size())
    {
	std::sort(dists.begin(), dists.end(), IndexDistance::Shortest);
        dists.resize(std::count_if(dists.begin(), dists.end(), std::bind2nd(IndexDistanceEqualDistance(), dists.front().second)));
    }

    if(dists.size())
    {
	if(res.capacity() < res.size() + dists.size()) res.reserve(res.size() + dists.size());
	std::vector<IndexDistance>::const_iterator it1 = dists.begin();
	std::vector<IndexDistance>::const_iterator it2 = dists.end();
	for(; it1 != it2; ++it1) res.push_back((*it1).first);
    }

    return res.size();
}

bool Battle2::Arena::CanSurrenderOpponent(u8 color) const
{
    const Army::army_t* enemy = GetArmy(GetOppositeColor(color));
    const Army::army_t* army = GetArmy(color);

    return army && army->GetCommander() && army->GetCommander()->GetType() == Skill::Primary::HEROES &&
           enemy && enemy->GetCommander();
}

bool Battle2::Arena::CanRetreatOpponent(u8 color) const
{
    const Army::army_t* army = GetArmy(color);

    return army && army->GetCommander() && army->GetCommander()->GetType() == Skill::Primary::HEROES &&
	    NULL == army->GetCommander()->inCastle();
}

bool Battle2::Arena::isDisableCastSpell(u8 spell, std::string* msg) const
{
    const HeroBase* hero1 = army1.GetCommander();
    const HeroBase* hero2 = army2.GetCommander();

    // check sphere negation (only for heroes)
    if((hero1 && hero1->HasArtifact(Artifact::SPHERE_NEGATION)) ||
       (hero2 && hero2->HasArtifact(Artifact::SPHERE_NEGATION)))
    {
        if(msg) *msg = _("The Sphere of Negation artifact is in effect for this battle, disabling all combat spells.");
        return true;
    }

    // check casted
    if(current_commander)
    {
	if(current_commander->Modes(Heroes::SPELLCASTED))
	{
	    if(msg) *msg = _("You have already cast a spell this round.");
	    return true;
	}

	if(spell == Spell::EARTHQUAKE && !castle)
	{
	    *msg = _("That spell will affect no one!");
	    return true;
	}
	else
	if(Spell::isSummon(spell))
	{
	    const Army::Troop* elem = GetArmy(current_commander->GetColor())->BattleFindModes(CAP_SUMMONELEM);
	    bool affect = true;
	    if(elem) switch(spell)
	    {
		case Spell::SUMMONEELEMENT: if(elem->GetID() != Monster::EARTH_ELEMENT) affect = false; break;
		case Spell::SUMMONAELEMENT: if(elem->GetID() != Monster::AIR_ELEMENT) affect = false; break;
		case Spell::SUMMONFELEMENT: if(elem->GetID() != Monster::FIRE_ELEMENT) affect = false; break;
		case Spell::SUMMONWELEMENT: if(elem->GetID() != Monster::WATER_ELEMENT) affect = false; break;
		default: break;
	    }
	    if(!affect)
	    {
		*msg = _("You may only summon one type of elemental per combat.");
		return true;
	    }

	    if(0 == GetFreePositionNearHero(current_commander->GetColor()))
	    {
		*msg = _("There is no open space adjacent to your hero to summon an Elemental to.");
		return true;
	    }
	}
	else
	if(spell != Spell::NONE)
	{
	    // check army
	    Board::const_iterator it1 = board.begin();
	    Board::const_iterator it2 = board.end();
	    for(; it1 != it2; ++it1)
	    {
		const Battle2::Stats* b = GetTroopBoard((*it1).index);

		if(b)
		{
		    if(b->AllowApplySpell(spell, current_commander, NULL))
			return false;
		}
		else
		// check graveyard
		if(isAllowResurrectFromGraveyard(spell, (*it1).index))
			return false;
	    }
	    *msg = _("That spell will affect no one!");
	    return true;
	}
    }

    // may be check other..
    /*
    */

    return false;
}

bool Battle2::Arena::isAllowResurrectFromGraveyard(u8 spell, u16 cell) const
{
    if(Spell::isResurrect(spell))
    {
	std::vector<u16> closed;
	std::vector<u16>::const_iterator it_closed;
	graveyard.GetClosedCells(closed);

	it_closed = std::find(closed.begin(), closed.end(), cell);

	if(current_commander && it_closed != closed.end())
	{
	    const u16 id_killed = graveyard.GetLastTroopIDFromCell(cell);
	    if(id_killed)
	    {
		const Battle2::Stats* killed = GetTroopID(id_killed);
		if(killed->AllowApplySpell(spell, current_commander, NULL)) return true;
	    }
	}
    }
    return false;
}

void Battle2::Arena::SetCastleTargetValue(u8 target, u8 value)
{
    switch(target)
    {
        case CAT_WALL1: board[8].object = value; break;
        case CAT_WALL2: board[29].object = value; break;
        case CAT_WALL3: board[73].object = value; break;
        case CAT_WALL4: board[96].object = value; break;

        case CAT_TOWER1:if(towers[0] && towers[0]->isValid()) towers[0]->SetDestroy(); break;
        case CAT_TOWER2:if(towers[2] && towers[2]->isValid()) towers[2]->SetDestroy(); break;
        case CAT_TOWER3:if(towers[1] && towers[1]->isValid()) towers[1]->SetDestroy(); break;

        case CAT_BRIDGE:
	    if(bridge->isValid())
	    {
                if(interface) interface->RedrawBridgeAnimation(true);
		bridge->SetDown(true);
		bridge->SetDestroy();
	    }
	    break;

        default: break;
    }
}

u8 Battle2::Arena::GetCastleTargetValue(u8 target) const
{
    switch(target)
    {
        case CAT_WALL1: return board[8].object;
        case CAT_WALL2: return board[29].object;
        case CAT_WALL3: return board[73].object;
        case CAT_WALL4: return board[96].object;

        case CAT_TOWER1:return towers[0] && towers[0]->isValid();
        case CAT_TOWER2:return towers[2] && towers[2]->isValid();
        case CAT_TOWER3:return towers[1] && towers[1]->isValid();

        case CAT_BRIDGE:return bridge->isValid();

        default: break;
    }
    return 0;
}

Battle2::Tower* Battle2::Arena::GetTower(u8 type)
{
    switch(type)
    {
        case TWR_LEFT:  return towers[0];
        case TWR_CENTER:return towers[1];
        case TWR_RIGHT: return towers[2];
        default: break;
    }
    return NULL;
}

void Battle2::Arena::PackBoard(Action & msg) const
{
    msg.Push(static_cast<u32>(board.size()));

    Board::const_iterator it = board.begin();
    while(it != board.end())
    {
        msg.Push((*it).object);
        msg.Push((*it).direction);
        msg.Push(static_cast<u32>((*it).quality));
        ++it;
    }
}

void Battle2::Arena::UnpackBoard(Action & msg)
{
    u32 size;
    msg.Pop(size);

    if(size == board.size())
    {
	Board::iterator it = board.begin();
        while(it != board.end())
        {
	    u32 byte32;
            msg.Pop((*it).object);
    	    msg.Pop((*it).direction);
    	    msg.Pop(byte32);
	    (*it).quality = byte32;
            ++it;
        }
    }
    else
	DEBUG(DBG_BATTLE, DBG_WARN, "Battle2::Arena::UnpackBoard: " << "incorrect param");
}

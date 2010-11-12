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
#include "kingdom.h"
#include "heroes.h"
#include "castle.h"

void Castle::AIJoinRNDArmy(void)
{
    const Monster mon1(race, DWELLING_MONSTER1);
    const Monster mon2(race, DWELLING_MONSTER2);
    const Monster mon3(race, DWELLING_MONSTER3);

    switch(Rand::Get(1, 4))
    {
        case 1:
            army.JoinTroop(mon1, mon1.GetRNDSize(false) * 3);
            army.JoinTroop(mon2, mon2.GetRNDSize(false));
            break;

        case 2:
            army.JoinTroop(mon1, mon1.GetRNDSize(false) * 2);
            army.JoinTroop(mon2, mon2.GetRNDSize(false) * 2);
            break;

        case 3:
            army.JoinTroop(mon1, mon1.GetRNDSize(false) * 2);
            army.JoinTroop(mon2, mon2.GetRNDSize(false));
            army.JoinTroop(mon3, mon3.GetRNDSize(false) * 2 / 3);
            break;

	default:
	    army.JoinTroop(mon1, mon1.GetRNDSize(false));
            army.JoinTroop(mon3, mon3.GetRNDSize(false));
            break;
    }
}

void Castle::AIDefense(void)
{
    if(isCastle())
    {
        if(!isBuild(BUILD_LEFTTURRET))  BuyBuilding(BUILD_LEFTTURRET);
        if(!isBuild(BUILD_RIGHTTURRET)) BuyBuilding(BUILD_RIGHTTURRET);
        if(!isBuild(BUILD_MOAT))        BuyBuilding(BUILD_MOAT);
        if(!isBuild(BUILD_CAPTAIN) && NULL == GetHeroes())      BuyBuilding(BUILD_CAPTAIN);
        if(!isBuild(BUILD_TAVERN) && Race::KNGT == GetRace())   BuyBuilding(BUILD_TAVERN);
        if(!isBuild(BUILD_SPEC))        BuyBuilding(BUILD_SPEC);
    }
    RecruitAllMonster();
}

void Castle::AIDevelopment(void)
{
    if(world.GetKingdom(GetColor()).GetHeroes().size() && isCastle() && isCapital())
    {
	if(!isBuild(BUILD_STATUE))	BuyBuilding(BUILD_STATUE);
	if(!isBuild(BUILD_SPEC) && Race::WRLK == race)	BuyBuilding(BUILD_SPEC);
	if(!isBuild(BUILD_TAVERN) && Race::KNGT == race)	BuyBuilding(BUILD_TAVERN);
	if(!isBuild(BUILD_MAGEGUILD1) && ((Race::SORC | Race::WZRD | Race::WRLK | Race::NECR) & race)) BuyBuilding(BUILD_MAGEGUILD1);
	if(!isBuild(BUILD_WELL))		BuyBuilding(BUILD_WELL);

	if(!isBuild(DWELLING_MONSTER1))	BuyBuilding(DWELLING_MONSTER1);
	if(!isBuild(DWELLING_MONSTER2))	BuyBuilding(DWELLING_MONSTER2);
	if(!isBuild(DWELLING_MONSTER3))	BuyBuilding(DWELLING_MONSTER3);
	if(!isBuild(DWELLING_MONSTER4))	BuyBuilding(DWELLING_MONSTER4);

	if(!isBuild(BUILD_THIEVESGUILD) && ((Race::NECR) & race)) BuyBuilding(BUILD_THIEVESGUILD);
	if(!isBuild(BUILD_MARKETPLACE))	BuyBuilding(BUILD_MARKETPLACE);

	if(!isBuild(BUILD_MAGEGUILD1))	BuyBuilding(BUILD_MAGEGUILD1);
	if(!isBuild(BUILD_MAGEGUILD2) && ((Race::SORC | Race::WZRD | Race::WRLK | Race::NECR) & race)) BuyBuilding(BUILD_MAGEGUILD2);

	if(!isBuild(DWELLING_UPGRADE2))	BuyBuilding(DWELLING_UPGRADE2);
	if(!isBuild(DWELLING_UPGRADE3))	BuyBuilding(DWELLING_UPGRADE3);
	if(!isBuild(DWELLING_UPGRADE4))	BuyBuilding(DWELLING_UPGRADE4);

	if(!isBuild(BUILD_LEFTTURRET))	BuyBuilding(BUILD_LEFTTURRET);
	if(!isBuild(BUILD_RIGHTTURRET))	BuyBuilding(BUILD_RIGHTTURRET);
	if(!isBuild(BUILD_MOAT))		BuyBuilding(BUILD_MOAT);
	if(!isBuild(BUILD_CAPTAIN))	BuyBuilding(BUILD_CAPTAIN);

	if(!isBuild(BUILD_MAGEGUILD2))	BuyBuilding(BUILD_MAGEGUILD2);

	if(!isBuild(DWELLING_MONSTER5))	BuyBuilding(DWELLING_MONSTER5);
	if(!isBuild(DWELLING_MONSTER6))	BuyBuilding(DWELLING_MONSTER6);

	if(!isBuild(BUILD_MAGEGUILD3))	BuyBuilding(BUILD_MAGEGUILD3);

	if(!isBuild(DWELLING_UPGRADE5))	BuyBuilding(DWELLING_UPGRADE5);
	if(!isBuild(DWELLING_UPGRADE6))	BuyBuilding(DWELLING_UPGRADE6);
    }
    else
    // build castle only monday or tuesday or for capital
    if(3 > world.GetDay() || !isCastle() || isCapital()) BuyBuilding(BUILD_CASTLE);

    // last day and buy monster
    if(world.LastDay()) RecruitAllMonster();
}

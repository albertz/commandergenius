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
#include "settings.h"
#include "gamedefs.h"
#include "battle2.h"
#include "world.h"
#include "army.h"
#include "castle.h"
#include "kingdom.h"
#include "heroes.h"
#include "test.h"

#ifndef BUILD_RELEASE

void RunTest1(void);
void RunTest2(void);
void RunTest3(void);

void TestMonsterSprite(void);

void Test::Run(int num)
{
    switch(num)
    {
	case 1: RunTest1(); break;
	case 2: RunTest2(); break;
	case 3: RunTest3(); break;

	case 9: TestMonsterSprite(); break;

	default: DEBUG(DBG_ENGINE , DBG_WARN, "Test::Run: unknown test."); break;
    }
}

void RunTest1(void)
{
    VERBOSE("Run Test1");
}

void RunTest2(void)
{
    VERBOSE("Run Test2");
}

void RunTest3(void)
{
    VERBOSE("Run Test3");
    Settings & conf = Settings::Get();
    //conf.SetPlayers(Color::BLUE|Color::GREEN);

    world.LoadMaps("/opt/projects/fh2/maps/beltway.mp2");

    Heroes & hero1 = *world.GetHeroes(Heroes::SANDYSANDY);
    Heroes & hero2 = *world.GetHeroes(Heroes::BAX);

    Kingdom & kingdom1 = world.GetKingdom(Color::RED);
    Kingdom & kingdom2 = world.GetKingdom(Color::YELLOW);

    conf.SetMyColor(Color::RED);

    hero1.SetSpellPoints(150);

    kingdom1.SetControl(Game::LOCAL);
    kingdom2.SetControl(Game::LOCAL);

    if(kingdom1.GetCastles().size())
    hero1.Recruit(kingdom1.GetColor(), Point(20, 20));
    hero2.Recruit(kingdom2.GetColor(), Point(20, 21));

    Army::army_t & army1 = hero1.GetArmy();

    Castle* castle = kingdom2.GetCastles().at(0);
    castle->BuyBuilding(BUILD_CAPTAIN);
    castle->ActionNewDay();
    castle->BuyBuilding(BUILD_MAGEGUILD1);

    //Army::army_t army2;
    //Army::army_t & army2 = hero2.GetArmy();
    Army::army_t & army2 = castle->GetArmy();

    army1.Clear();
    //army1.JoinTroop(Monster::PHOENIX, 10);
    //army1.JoinTroop(Monster::RANGER, 80);
    army1.At(0) = Army::Troop(Monster::ROYAL_MUMMY, 50);
    //army1.JoinTroop(Monster::GARGOYLE, 100);

    //army1.JoinTroop(Monster::Rand(Monster::LEVEL1), 30);
    //army1.JoinTroop(Monster::Rand(Monster::LEVEL2), 20);
    //army1.JoinTroop(Monster::Rand(Monster::LEVEL3), 10);

    army2.Clear();
//    army2.At(0) = Army::Troop(Monster::OGRE, 1);
    army2.At(0) = Army::Troop(Monster::DWARF, 2);
    army2.At(1) = Army::Troop(Monster::DWARF, 2);
    army2.At(2) = Army::Troop(Monster::DWARF, 2);
    army2.At(3) = Army::Troop(Monster::DWARF, 2);
    army2.At(4) = Army::Troop(Monster::DWARF, 2);
//    army2.JoinTroop(static_cast<Monster::monster_t>(1), 10);
//    army2.JoinTroop(static_cast<Monster::monster_t>(4), 10);
//    army2.JoinTroop(static_cast<Monster::monster_t>(6), 10);
//    army2.JoinTroop(static_cast<Monster::monster_t>(8), 10);

//    kingdom2.Dump();
    
    Battle2::Loader(army1, army2, castle->GetIndex());
}

#endif

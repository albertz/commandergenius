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

#include <functional>
#include <algorithm>
#include <deque>
#include "game.h"
#include "cursor.h"
#include "engine.h"
#include "game_interface.h"
#include "interface_gamearea.h"
#include "heroes.h"
#include "castle.h"
#include "world.h"
#include "settings.h"
#include "kingdom.h"
#include "agg.h"

#define HERO_MAX_SHEDULED_TASK 7

void Kingdom::AIDumpCacheObjects(const IndexDistance & id) const
{
    std::map<s32, MP2::object_t>::const_iterator it = ai_objects.find(id.first);
    if(it != ai_objects.end())
    DEBUG(DBG_AI , DBG_TRACE, "AIDumpCacheObjects: " << MP2::StringObject((*it).second) << ", maps index: " << id.first << ", dist: " << id.second);
}

void Kingdom::AITurns(void)
{
    if(isLoss() || color == Color::GRAY)
    {
	LossPostActions();
	return;
    }
    
    if(! Settings::Get().MusicMIDI()) AGG::PlayMusic(MUS::COMPUTER);

    Interface::StatusWindow *status = Interface::NoGUI() ? NULL : &Interface::StatusWindow::Get();

    // turn indicator
    if(status) status->RedrawTurnProgress(0);

    // scan map
    ai_objects.clear();
    world.StoreActionObject(GetColor(), ai_objects);
    DEBUG(DBG_AI , DBG_INFO, "Kingdom::AITurns: " << Color::String(color) << ", size cache objects: " << ai_objects.size());

    // set capital
    if(NULL == ai_capital && castles.size())
    {
	std::vector<Castle *>::iterator it = std::find_if(castles.begin(), castles.end(), Castle::PredicateIsCastle);

	if(castles.end() != it)
	{
	    if(*it)
	    {
		ai_capital = *it;
		ai_capital->SetModes(Castle::CAPITAL);
	    }
	}
	else
	// first town
	{
	    ai_capital = castles.front();
	    ai_capital->SetModes(Castle::CAPITAL);
	}
    }

    // turn indicator
    if(status) status->RedrawTurnProgress(1);

    // castle AI turn
    AICastlesTurns();

    const u8 maxhero = Maps::XLARGE > world.w() ? (Maps::LARGE > world.w() ? 3 : 2) : 4;

    // buy hero in capital
    if((heroes.empty() || heroes.size() < maxhero) &&
	ai_capital && ai_capital->isCastle())
    {
	Recruits & rec = GetRecruits();

	if(rec.GetHero1()) ai_capital->RecruitHero(rec.GetHero1());
	else
	if(rec.GetHero2()) ai_capital->RecruitHero(rec.GetHero2());
    }

    // update roles
    if(heroes.size())
    {
	std::for_each(heroes.begin(), heroes.end(), std::mem_fun(&Heroes::ResetStupidFlag));
	std::for_each(heroes.begin(), heroes.end(), std::bind2nd(std::mem_fun(&Heroes::SetModes), Heroes::SCOUTER));
	heroes.front()->SetModes(Heroes::HUNTER);
    }

    // turn indicator
    if(status) status->RedrawTurnProgress(2);

    // heroes turns
    std::vector<Heroes *>::const_iterator ith1 = heroes.begin();
    std::vector<Heroes *>::const_iterator ith2 = heroes.end();

    for(; ith1 != ith2; ++ith1) if(*ith1)
    {
	Heroes & hero = **ith1;

	while(hero.MayStillMove())
	{
	    // turn indicator
	    if(status) status->RedrawTurnProgress(3);
	    //if(status) status->RedrawTurnProgress(4);

	    // get task for heroes
	    AIHeroesGetTask(hero);

	    // turn indicator
	    if(status) status->RedrawTurnProgress(5);
	    //if(status) status->RedrawTurnProgress(6);

	    // heroes AI turn
	    if(Interface::NoGUI())
		AIHeroesNoGUITurns(hero);
	    else
		AIHeroesTurns(hero);

	    // turn indicator
	    if(status) status->RedrawTurnProgress(7);
	    //if(status) status->RedrawTurnProgress(8);
	}
    }

    // turn indicator
    if(status) status->RedrawTurnProgress(9);

    DEBUG(DBG_AI , DBG_INFO, "Kingdom::AITurns: " << Color::String(color) << " moved");
}

void Kingdom::AICastlesTurns(void)
{
    std::vector<Castle *>::const_iterator itc1 = castles.begin();
    std::vector<Castle *>::const_iterator itc2 = castles.end();

    for(; itc1 != itc2; ++itc1) if(*itc1)
    {
	Castle & castle = **itc1;
	const s8 range = Game::GetViewDistance(castle.isCastle() ? Game::VIEW_CASTLE : Game::VIEW_TOWN);
	bool defense = false;

	// find enemy hero
	for(s8 y = -range; y <= range && !defense; ++y)
    	    for(s8 x = -range; x <= range && !defense; ++x)
	{
    	    if(!y && !x) continue;

    	    if(Maps::isValidAbsPoint(castle.GetCenter().x + x, castle.GetCenter().y + y))
    	    {
        	const Maps::Tiles & tile = world.GetTiles(Maps::GetIndexFromAbsPoint(castle.GetCenter().x + x, castle.GetCenter().y + y));
        	const Heroes* hero = NULL;

        	if(MP2::OBJ_HEROES == tile.GetObject() && NULL != (hero = world.GetHeroes(tile.GetIndex())) && GetColor() != hero->GetColor())
		    defense = true;
	    }
	}

	defense ? castle.AIDefense() : castle.AIDevelopment();
    }
}

void Kingdom::AIHeroesTurns(Heroes &hero)
{
    if(hero.GetPath().isValid()) hero.SetMove(true);
    else return;

    const Settings & conf = Settings::Get();
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    Interface::Basic & I = Interface::Basic::Get();

    cursor.Hide();

    if(0 != conf.AIMoveSpeed() && hero.isShow(Settings::Get().MyColor()))
    {
	    cursor.Hide();
	    I.gameArea.Center(hero.GetCenter());
	    I.Redraw(REDRAW_GAMEAREA);
	    cursor.Show();
	    display.Flip();
    }

    bool hide_move = (0 == conf.AIMoveSpeed()) ||
	    (! IS_DEVEL() && !hero.isShow(Settings::Get().MyColor()));

    while(LocalEvent::Get().HandleEvents())
    {
	if(hero.isFreeman() || !hero.isEnableMove()) break;

	if(hide_move)
	{
	    hero.Move(true);
	}
	else
	if(Game::AnimateInfrequent(Game::CURRENT_AI_DELAY))
	{
	    cursor.Hide();
	    hero.Move();

	    I.gameArea.Center(hero.GetCenter());
	    I.Redraw(REDRAW_GAMEAREA);
	    cursor.Show();
	    display.Flip();
	}

    	if(Game::AnimateInfrequent(Game::MAPS_DELAY))
	{
	    Maps::IncreaseAnimationTicket();
	    cursor.Hide();
	    I.Redraw(REDRAW_GAMEAREA);
	    cursor.Show();
	    display.Flip();
	}
    }

    // 0.2 sec delay for show enemy hero position
    if(!hero.isFreeman() && !hide_move) DELAY(200);
}

void Kingdom::AIHeroesNoGUITurns(Heroes &hero)
{
    if(hero.GetPath().isValid()) hero.SetMove(true);
    else return;

    while(1)
    {
	if(hero.isFreeman() || !hero.isEnableMove()) break;

	hero.Move(true);

	DELAY(10);
    }
}

void Kingdom::AIHeroesGetTask(Heroes & hero)
{
    std::deque<s32> & task = hero.GetAITasks();

    std::vector<s32> results;
    results.reserve(5);

    // patrol task
    if(hero.Modes(Heroes::PATROL))
    {
	// goto patrol center
	if(hero.GetCenterPatrol() != hero.GetCenter() &&
	   hero.GetPath().Calculate(Maps::GetIndexFromAbsPoint(hero.GetCenterPatrol())))
		return;

	// scan enemy hero
	if(Maps::ScanDistanceObject(Maps::GetIndexFromAbsPoint(hero.GetCenterPatrol()),
				    MP2::OBJ_HEROES, hero.GetSquarePatrol(), results))
	{
	    std::vector<s32>::const_iterator it = results.begin();
	    for(; it != results.end(); ++it)
	    {
		const Heroes* enemy = world.GetHeroes(*it);
		if(enemy && !Settings::Get().IsUnions(enemy->GetColor(), hero.GetColor()))
		{
		    if(hero.GetPath().Calculate(enemy->GetIndex()))
			return;
		}
	    }
	}

	hero.SetModes(Heroes::STUPID);
	return;
    }

    // primary target
    if(Maps::isValidAbsIndex(hero.GetPrimaryTarget()))
    {
	if(hero.GetIndex() == hero.GetPrimaryTarget())
	    hero.SetPrimaryTarget(-1);
	else
	if(hero.GetPrimaryTarget() != hero.GetPath().GetDestinationIndex() &&
	    !hero.GetPath().Calculate(hero.GetPrimaryTarget()))
	{
	    DEBUG(DBG_AI , DBG_INFO, "AI::HeroesTask: " << Color::String(color) <<
	    ", Hero " << hero.GetName() << ": path unknown, primary target reset");

	    hero.SetPrimaryTarget(-1);
	}

	if(hero.GetPath().isValid()) return;
    }

	// scan heroes and castle
	if(Maps::ScanDistanceObject(hero.GetIndex(), MP2::OBJ_CASTLE, hero.GetScoute(), results) ||
    	    Maps::ScanDistanceObject(hero.GetIndex(), MP2::OBJ_HEROES, hero.GetScoute(), results))
	{
	    std::vector<s32>::const_iterator it = results.begin();

	    for(; it != results.end(); ++it)
		if(hero.AIPriorityObject(*it) &&
		    hero.GetPath().Calculate(*it))
	    {
		DEBUG(DBG_AI , DBG_INFO, "AI::HeroesTask: " << Color::String(color) <<
		", Hero " << hero.GetName() << ": set primary target: " <<
		MP2::StringObject(world.GetTiles(*it).GetObject()) << "(" << *it << ")");

		hero.SetPrimaryTarget(*it);
		return;
	    }
	}

    // check destination
    if(hero.GetPath().isValid())
    {
	if(!hero.AIValidObject(hero.GetPath().GetDestinationIndex()))
	    hero.GetPath().Reset();
	else
	if(hero.GetPath().size() < 5)
	{
	    DEBUG(DBG_AI , DBG_INFO, "AI::HeroesTask: " << Color::String(color) <<
		", Hero " << hero.GetName() << ": continue" << " short");

	    return;
	}
    }

	// scan 2x2 pickup objects
	if(Maps::ScanDistanceObject(hero.GetIndex(), MP2::OBJ_ARTIFACT, 2, results) ||
    	    Maps::ScanDistanceObject(hero.GetIndex(), MP2::OBJ_RESOURCE, 2, results) ||
    	    Maps::ScanDistanceObject(hero.GetIndex(), MP2::OBJ_CAMPFIRE, 2, results) ||
    	    Maps::ScanDistanceObject(hero.GetIndex(), MP2::OBJ_TREASURECHEST, 2, results) ||
    	    // scan 3x3 capture objects
    	    Maps::ScanDistanceObject(hero.GetIndex(), MP2::OBJ_SAWMILL, 3, results) ||
    	    Maps::ScanDistanceObject(hero.GetIndex(), MP2::OBJ_MINES, 3, results) ||
    	    Maps::ScanDistanceObject(hero.GetIndex(), MP2::OBJ_ALCHEMYLAB, 3, results))
	{
	    std::vector<s32>::const_iterator it = results.begin();
	    for(; it != results.end(); ++it)
    		if(hero.AIPriorityObject(*it) &&
		    hero.GetPath().Calculate(*it))
	    {
		ai_objects.erase(*it);

		DEBUG(DBG_AI , DBG_INFO, "AI::HeroesTask: " << Color::String(color) <<
		", Hero " << hero.GetName() << ": find object: " <<
		MP2::StringObject(world.GetTiles(*it).GetObject()) << "(" << *it << ")");

		return;
	    }
	}

    if(hero.GetPath().isValid())
    {
	DEBUG(DBG_AI , DBG_INFO, "AI::HeroesTask: " << Color::String(color) <<
		", Hero " << hero.GetName() << ": continue");

	return;
    }

    // get task from kingdom
    if(task.empty()) AIHeroesPrepareTask(hero);

    // random shuffle
    if(1 < task.size())
	std::random_shuffle(task.begin(), task.begin() + (2 < task.size() ? 2 : 1));

    // find passable index
    while(task.size())
    {
	const s32 & index = task.front();

	if(hero.GetPath().Calculate(index)) break;

	DEBUG(DBG_AI , DBG_INFO, "AI::HeroesTask: " << Color::String(color) <<
	", Hero " << hero.GetName() << " say: unable get object: " <<
	MP2::StringObject(world.GetTiles(index).GetObject()) << ", index: " <<
	index << ", remove task...");

	task.pop_front();
    }

    // success
    if(task.size())
    {
	const s32 & index = task.front();

	DEBUG(DBG_AI , DBG_INFO, "AI::HeroesTask: " << Color::String(color) <<
	", Hero " << hero.GetName() << " say: go to object: " <<
	MP2::StringObject(world.GetTiles(index).GetObject()) << ", index: " << index);

	ai_objects.erase(index);
	task.pop_front();

	if(IS_DEBUG(DBG_AI, DBG_TRACE)) hero.GetPath().Dump();
    }
    else
    {
	hero.GetPath().Reset();
	DEBUG(DBG_AI , DBG_INFO, "AI::HeroesTask: " << Color::String(color) <<
	", Hero " << hero.GetName() << " say: unknown task., help my please..");

	hero.SetModes(Heroes::STUPID);
    }
}

void Kingdom::AIHeroesPrepareTask(Heroes & hero)
{
    if(hero.GetPath().isValid()) return;

    std::deque<s32> & task = hero.GetAITasks();

    Castle *castle = hero.inCastle();

    DEBUG(DBG_AI , DBG_INFO, "Kingdom::AIHeroesTask: " << Color::String(color) <<
		    ", hero: " << hero.GetName() << ", task prepare");

    // if hero in castle
    if(castle)
    {
	castle->RecruitAllMonster();
	hero.GetArmy().UpgradeTroops(*castle);

	// recruit army
	if(hero.Modes(Heroes::HUNTER))
		hero.GetArmy().JoinStrongestFromArmy(castle->GetArmy());
	else
	if(hero.Modes(Heroes::SCOUTER))
		hero.GetArmy().KeepOnlyWeakestTroops(castle->GetArmy());
    }

    // load minimal distance tasks
    std::vector<IndexDistance> objs;
    objs.reserve(ai_objects.size());

    for(std::map<s32, MP2::object_t>::const_iterator
	it = ai_objects.begin(); it != ai_objects.end(); ++it)
    {
	if(hero.isShipMaster())
	{
	    if(MP2::isGroundObject((*it).second)) continue;

	    // check previous positions
	    if(MP2::OBJ_COAST == (*it).second &&
		hero.isVisited(world.GetTiles((*it).first))) continue;
	}
	else
	{
	    if(MP2::isWaterObject((*it).second)) continue;
	}

	objs.push_back(IndexDistance((*it).first,
			    Maps::GetApproximateDistance(hero.GetIndex(), (*it).first)));
    }

    std::sort(objs.begin(), objs.end(), IndexDistance::Shortest);

    for(std::vector<IndexDistance>::const_iterator
	it = objs.begin(); it != objs.end(); ++it)
    {
	if(task.size() >= HERO_MAX_SHEDULED_TASK) break;

	if(hero.AIValidObject((*it).first) &&
	    hero.GetPath().Calculate((*it).first))
	{
	    s32 pos = 0;

	    // path dangerous
	    if(u16 around = hero.GetPath().isUnderProtection(pos))
	    {
		bool skip = false;
    		for(Direction::vector_t
        	    dir = Direction::TOP_LEFT; dir < Direction::CENTER && !skip; ++dir) if(around & dir)
    		{
        	    const s32 dst = Maps::GetDirectionIndex(pos, dir);
        	    Army::army_t enemy;
        	    enemy.FromGuardian(world.GetTiles(dst));
        	    if(enemy.isValid() && enemy.StrongerEnemyArmy(hero.GetArmy())) skip = true;
    		}
		if(skip) continue;
	    }

	    DEBUG(DBG_AI , DBG_INFO, "Kingdom::AIHeroesTask: " << Color::String(color) <<
		    ", hero: " << hero.GetName() << ", added tasks: " << 
		    MP2::StringObject(ai_objects[(*it).first]) << ", index: " << (*it).first <<
		    ", distance: " << (*it).second);

	    task.push_back((*it).first);
	    ai_objects.erase((*it).first);
	}
	else
	{
	    DEBUG(DBG_AI , DBG_TRACE, "Kingdom::AIHeroesTask: " << Color::String(color) <<
		    ", hero: " << hero.GetName() << ", impossible: " << 
		    MP2::StringObject(ai_objects[(*it).first]) << ", index: " << (*it).first <<
		    ", distance: " << (*it).second);
	}
    }

    if(task.empty())
	hero.AIRescueWhereMove();
}

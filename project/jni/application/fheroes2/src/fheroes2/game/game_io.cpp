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

#include <fstream>
#include <sstream>
#include <cstring>
#include <ctime>
#include "zzlib.h"
#include "settings.h"
#include "kingdom.h"
#include "heroes.h"
#include "castle.h"
#include "army.h"
#include "world.h"
#include "gameevent.h"
#include "interface_gamearea.h"
#include "settings.h"
#include "tools.h"

std::string Game::IO::last_name;

bool Game::Save(const std::string &fn)
{
    DEBUG(DBG_GAME , DBG_INFO, "Game::Save: " << fn);
    const bool autosave = std::string::npos != fn.find("autosave.sav");

    if(Settings::Get().ExtRewriteConfirm() && FilePresent(fn) &&
	(!autosave || Settings::Get().ExtAutosaveConfirm()) &&
	Dialog::NO == Dialog::Message("", _("Are you sure you want to overwrite the save with this name?"), Font::BIG, Dialog::YES|Dialog::NO))
    {
	return false;
    }

    Game::IO msg;

    if(! Game::IO::SaveBIN(msg)) return false;

#ifdef WITH_ZLIB
    std::ofstream fs(fn.c_str(), std::ios::binary);
    if(!fs.is_open()) return false;
    std::vector<char> v;
    if(!ZLib::Compress(v, msg.DtPt(), msg.DtSz())) return false;
    fs.write(&v[0], v.size());
    fs.close();
#else
    msg.Save(fn.c_str());
#endif

    if(! autosave) Game::IO::last_name = fn;

    return true;
}

bool Game::IO::LoadSAV(const std::string & fn)
{
    if(fn.empty()) return false;

    Load(fn.c_str());

#ifdef WITH_ZLIB
    std::vector<char> v;
    if(ZLib::UnCompress(v, data, dtsz))
    {
	dtsz = v.size();
	delete [] data;
        data = new char [dtsz + 1];
	itd1 = data;
	itd2 = data + dtsz;

	std::memcpy(data, &v[0], dtsz);
	v.clear();
    }
#endif

    return true;
}

bool Game::Load(const std::string & fn)
{
    DEBUG(DBG_GAME , DBG_INFO, "Game::Load: " << fn);

    // loading info
    Game::ShowLoadMapsText();

    Game::IO msg;
    if(!msg.LoadSAV(fn) || !Game::IO::LoadBIN(msg)) return false;

    Game::IO::last_name = fn;
    Settings::Get().SetLoadedGameVersion(true);

    return true;
}

bool Game::LoadSAV2FileInfo(const std::string & fn,  Maps::FileInfo & maps_file)
{
    Game::IO msg;

    if(!msg.LoadSAV(fn)) return false;

    u8 byte8;
    u16 byte16, version;
    u32 byte32;
    std::string str;

    maps_file.file = fn;

    msg.Pop(byte16);
    if(byte16 != 0xFF01) return false;
    // format version
    msg.Pop(version);
    // major version
    msg.Pop(byte8);
    // minor version
    msg.Pop(byte8);
    // svn
    msg.Pop(str);
    // time
    msg.Pop(byte32);
    maps_file.localtime = byte32;

    // maps
    msg.Pop(byte16);
    if(FORMAT_VERSION_2031 <= version)
    {
	msg.Pop(maps_file.size_w);
	msg.Pop(maps_file.size_h);
    }
    else
    {
	msg.Pop(byte8); maps_file.size_w = byte8;
	msg.Pop(byte8); maps_file.size_h = byte8;
    }
    msg.Pop(str);
    msg.Pop(maps_file.difficulty);
    msg.Pop(maps_file.kingdom_colors);
    msg.Pop(maps_file.human_colors);
    msg.Pop(maps_file.computer_colors);
    msg.Pop(maps_file.rnd_races);
    msg.Pop(maps_file.conditions_wins);
    msg.Pop(maps_file.wins1);
    msg.Pop(maps_file.wins2);
    msg.Pop(maps_file.wins3);
    msg.Pop(maps_file.wins4);
    msg.Pop(maps_file.conditions_loss);
    msg.Pop(maps_file.loss1);
    msg.Pop(maps_file.loss2);
    // races
    msg.Pop(byte16);
    msg.Pop(byte8); maps_file.races[0] = byte8;
    msg.Pop(byte8); maps_file.races[1] = byte8;
    msg.Pop(byte8); maps_file.races[2] = byte8;
    msg.Pop(byte8); maps_file.races[3] = byte8;
    msg.Pop(byte8); maps_file.races[4] = byte8;
    msg.Pop(byte8); maps_file.races[5] = byte8;
    // maps name
    msg.Pop(maps_file.name);
    // maps description
    msg.Pop(maps_file.description);

    return true;
}

bool Game::IO::SaveBIN(QueueMessage & msg)
{
    const Settings & conf = Settings::Get();

    msg.Reserve(world.w() > Maps::MEDIUM ? 600 * 1024 : 200 * 1024);

    msg.Push(static_cast<u16>(0xFF01));
    // format version
    msg.Push(static_cast<u16>(CURRENT_FORMAT_VERSION));
    // version
    msg.Push(conf.major_version);
    msg.Push(conf.minor_version);
    // svn
    msg.Push(conf.svn_version);
    // time
    msg.Push(static_cast<u32>(std::time(NULL)));
    // lang
    msg.Push(conf.force_lang);

    // maps
    msg.Push(static_cast<u16>(0xFF02));
    msg.Push(conf.current_maps_file.size_w);
    msg.Push(conf.current_maps_file.size_h);
    msg.Push(GetBasename(conf.current_maps_file.file));
    msg.Push(conf.current_maps_file.difficulty);
    msg.Push(conf.current_maps_file.kingdom_colors);
    msg.Push(conf.current_maps_file.human_colors);
    msg.Push(conf.current_maps_file.computer_colors);
    msg.Push(conf.current_maps_file.rnd_races);
    msg.Push(conf.current_maps_file.conditions_wins);
    msg.Push(conf.current_maps_file.wins1);
    msg.Push(conf.current_maps_file.wins2);
    msg.Push(conf.current_maps_file.wins3);
    msg.Push(conf.current_maps_file.wins4);
    msg.Push(conf.current_maps_file.conditions_loss);
    msg.Push(conf.current_maps_file.loss1);
    msg.Push(conf.current_maps_file.loss2);
    // races
    msg.Push(static_cast<u16>(0xFF03));
    msg.Push(static_cast<u32>(KINGDOMMAX));
    for(u32 ii = 0; ii < KINGDOMMAX; ++ii)
	msg.Push(conf.current_maps_file.races[ii]);
    // unions
    msg.Push(static_cast<u32>(KINGDOMMAX));
    for(u32 ii = 0; ii < KINGDOMMAX; ++ii)
	msg.Push(conf.current_maps_file.unions[ii]);
    // maps name
    msg.Push(conf.current_maps_file.name);
    // maps description
    msg.Push(conf.current_maps_file.description);
    // game
    msg.Push(static_cast<u16>(0xFF04));
    msg.Push(static_cast<u8>(conf.game_difficulty));
    msg.Push(static_cast<u8>(conf.my_color));
    msg.Push(static_cast<u8>(conf.cur_color));
    msg.Push(conf.game_type);
    msg.Push(conf.players_colors);
    msg.Push(conf.preferably_count_players);
    msg.Push(conf.debug);

    msg.Push(conf.opt_game());
    msg.Push(conf.opt_world());
    msg.Push(conf.opt_battle());

    // world
    msg.Push(static_cast<u16>(0xFF05));
    msg.Push(world.width);
    msg.Push(world.height);
    msg.Push(world.ultimate_artifact);
    msg.Push(world.uniq0);
    msg.Push(static_cast<u8>(world.week_name));
    msg.Push(static_cast<u8>(world.heroes_cond_wins));
    msg.Push(static_cast<u8>(world.heroes_cond_loss));
    msg.Push(world.month);
    msg.Push(world.week);
    msg.Push(world.day);

    // tiles
    msg.Push(static_cast<u16>(0xFF06));
    msg.Push(static_cast<u32>(world.vec_tiles.size()));
    for(u32 ii = 0; ii < world.vec_tiles.size(); ++ii)
    {
	if(NULL == world.vec_tiles[ii]){ DEBUG(DBG_GAME , DBG_WARN, "Game::IO::SaveBIN: tiles is NULL"); return false; }
	PackTile(msg, *world.vec_tiles[ii]);
    }

    // heroes
    msg.Push(static_cast<u16>(0xFF07));
    msg.Push(static_cast<u32>(world.vec_heroes.size()));
    for(u32 ii = 0; ii < world.vec_heroes.size(); ++ii)
    {
	if(NULL == world.vec_heroes[ii]){ DEBUG(DBG_GAME , DBG_WARN, "Game::IO::SaveBIN: heroes is NULL"); return false; }
	PackHeroes(msg, *world.vec_heroes[ii]);
    }

    // castles
    msg.Push(static_cast<u16>(0xFF08));
    msg.Push(static_cast<u32>(world.vec_castles.size()));
    for(u32 ii = 0; ii < world.vec_castles.size(); ++ii)
    {
	if(NULL == world.vec_castles[ii]){ DEBUG(DBG_GAME , DBG_WARN, "Game::IO::SaveBIN: castles is NULL"); return false; }
	PackCastle(msg, *world.vec_castles[ii]);
    }

    // kingdoms
    msg.Push(static_cast<u16>(0xFF09));
    msg.Push(static_cast<u32>(world.vec_kingdoms.size()));
    for(u32 ii = 0; ii < world.vec_kingdoms.size(); ++ii)
    {
	if(NULL == world.vec_kingdoms[ii]){ DEBUG(DBG_GAME , DBG_WARN, "Game::IO::SaveBIN: kingdoms is NULL"); return false; }
	PackKingdom(msg, *world.vec_kingdoms[ii]);
    }

    // signs
    msg.Push(static_cast<u16>(0xFF0A));
    msg.Push(static_cast<u32>(world.map_sign.size()));
    {
        std::map<s32, std::string>::const_iterator it1 = world.map_sign.begin();
	std::map<s32, std::string>::const_iterator it2 = world.map_sign.end();
	for(; it1 != it2; ++it1)
	{
	    msg.Push((*it1).first);
	    msg.Push((*it1).second);
	}
    }

    // captured object
    msg.Push(static_cast<u16>(0xFF0B));
    msg.Push(static_cast<u32>(world.map_captureobj.size()));
    {
	std::map<s32, ObjectColor>::const_iterator it1 = world.map_captureobj.begin();
	std::map<s32, ObjectColor>::const_iterator it2 = world.map_captureobj.end();
	for(; it1 != it2; ++it1)
	{
	    msg.Push((*it1).first);
	    msg.Push(static_cast<u8>((*it1).second.first));
	    msg.Push(static_cast<u8>((*it1).second.second));
	}
    }

    // rumors
    msg.Push(static_cast<u16>(0xFF0C));
    msg.Push(static_cast<u32>(world.vec_rumors.size()));
    {
	std::vector<std::string>::const_iterator it1 = world.vec_rumors.begin();
	std::vector<std::string>::const_iterator it2 = world.vec_rumors.end();
	for(; it1 != it2; ++it1) msg.Push(*it1);
    }

    // day events
    msg.Push(static_cast<u16>(0xFF0D));
    msg.Push(static_cast<u32>(world.vec_eventsday.size()));
    {
	std::vector<GameEvent::Day *>::const_iterator it1 = world.vec_eventsday.begin();
	std::vector<GameEvent::Day *>::const_iterator it2 = world.vec_eventsday.end();
	for(; it1 != it2; ++it1)
	{
	    if(NULL == *it1){ DEBUG(DBG_GAME , DBG_WARN, "Game::IO::SaveBIN: eventday is NULL"); return false; }

	    msg.Push((*it1)->resource.wood);
	    msg.Push((*it1)->resource.mercury);
	    msg.Push((*it1)->resource.ore);
	    msg.Push((*it1)->resource.sulfur);
	    msg.Push((*it1)->resource.crystal);
	    msg.Push((*it1)->resource.gems);
	    msg.Push((*it1)->resource.gold);
	    msg.Push(static_cast<u8>((*it1)->computer));
	    msg.Push((*it1)->first);
	    msg.Push((*it1)->subsequent);
	    msg.Push((*it1)->colors);
	    msg.Push((*it1)->message);
	}
    }

    // coord events
    msg.Push(static_cast<u16>(0xFF0E));
    msg.Push(static_cast<u32>(world.vec_eventsmap.size()));
    {
	std::vector<GameEvent::Coord *>::const_iterator it1 = world.vec_eventsmap.begin();
	std::vector<GameEvent::Coord *>::const_iterator it2 = world.vec_eventsmap.end();
	for(; it1 != it2; ++it1)
	{
	    if(NULL == *it1){ DEBUG(DBG_GAME , DBG_WARN, "Game::IO::SaveBIN: eventmaps is NULL"); return false; }

	    msg.Push((*it1)->index_map);
	    msg.Push((*it1)->resource.wood);
	    msg.Push((*it1)->resource.mercury);
	    msg.Push((*it1)->resource.ore);
	    msg.Push((*it1)->resource.sulfur);
	    msg.Push((*it1)->resource.crystal);
	    msg.Push((*it1)->resource.gems);
	    msg.Push((*it1)->resource.gold);
	    msg.Push(static_cast<u8>((*it1)->artifact));
	    msg.Push(static_cast<u8>((*it1)->computer));
	    msg.Push(static_cast<u8>((*it1)->cancel));
	    msg.Push((*it1)->colors);
	    msg.Push((*it1)->message);
	}
    }

    // sphinx riddles
    msg.Push(static_cast<u16>(0xFF0F));
    msg.Push(static_cast<u32>(world.vec_riddles.size()));
    {
	std::vector<GameEvent::Riddle *>::const_iterator it1 = world.vec_riddles.begin();
	std::vector<GameEvent::Riddle *>::const_iterator it2 = world.vec_riddles.end();
	for(; it1 != it2; ++it1)
	{
	    if(NULL == *it1){ DEBUG(DBG_GAME , DBG_WARN, "Game::IO::SaveBIN: riddles is NULL"); return false; }

	    msg.Push((*it1)->index_map);
	    msg.Push((*it1)->resource.wood);
	    msg.Push((*it1)->resource.mercury);
	    msg.Push((*it1)->resource.ore);
	    msg.Push((*it1)->resource.sulfur);
	    msg.Push((*it1)->resource.crystal);
	    msg.Push((*it1)->resource.gems);
	    msg.Push((*it1)->resource.gold);
	    msg.Push(static_cast<u8>((*it1)->artifact));
	    msg.Push(static_cast<u8>((*it1)->quiet));

	    msg.Push(static_cast<u32>((*it1)->answers.size()));
	    std::vector<std::string>::const_iterator ita1 = (*it1)->answers.begin();
	    std::vector<std::string>::const_iterator ita2 = (*it1)->answers.end();
	    for(; ita1 != ita2; ++ita1) msg.Push(*ita1);

	    msg.Push((*it1)->message);
	}
    }

    msg.Push(static_cast<u16>(0xFFFF));
    return true;
}

void Game::IO::PackTile(QueueMessage & msg, const Maps::Tiles & tile)
{
    msg.Push(tile.tile_sprite_index);
    msg.Push(tile.tile_sprite_shape);
    msg.Push(tile.mp2_object);
    msg.Push(tile.quantity1);
    msg.Push(tile.quantity2);
    msg.Push(tile.quantity3);
    msg.Push(tile.quantity4);
    msg.Push(tile.fogs);
    msg.Push(tile.unused1);
    msg.Push(tile.unused2);
    msg.Push(tile.unused3);

    // addons 1
    msg.Push(static_cast<u8>(tile.addons_level1.size()));
    std::list<Maps::TilesAddon>::const_iterator it1 = tile.addons_level1.begin();
    std::list<Maps::TilesAddon>::const_iterator it2 = tile.addons_level1.end();
    for(; it1 != it2; ++it1)
    {
	const Maps::TilesAddon & addon = *it1;
	msg.Push(addon.level);
	msg.Push(addon.uniq);
	msg.Push(addon.object);
	msg.Push(addon.index);
    }

    // addons 2
    msg.Push(static_cast<u8>(tile.addons_level2.size()));
    it1 = tile.addons_level2.begin();
    it2 = tile.addons_level2.end();
    for(; it1 != it2; ++it1)
    {
	const Maps::TilesAddon & addon = *it1;
	msg.Push(addon.level);
	msg.Push(addon.uniq);
	msg.Push(addon.object);
	msg.Push(addon.index);
    }
}

void Game::IO::PackKingdom(QueueMessage & msg, const Kingdom & kingdom)
{
    msg.Push(static_cast<u8>(kingdom.color));
    msg.Push(static_cast<u8>(kingdom.control));

    msg.Push(kingdom.flags);
    msg.Push(kingdom.lost_town_days);
    msg.Push(static_cast<u16>(kingdom.ai_capital ? kingdom.ai_capital->GetIndex() : 0));

    // funds
    msg.Push(kingdom.resource.wood);
    msg.Push(kingdom.resource.mercury);
    msg.Push(kingdom.resource.ore);
    msg.Push(kingdom.resource.sulfur);
    msg.Push(kingdom.resource.crystal);
    msg.Push(kingdom.resource.gems);
    msg.Push(kingdom.resource.gold);

    // visit objects
    msg.Push(static_cast<u32>(kingdom.visit_object.size()));
    std::list<IndexObject>::const_iterator it1 = kingdom.visit_object.begin();
    std::list<IndexObject>::const_iterator it2 = kingdom.visit_object.end();
    for(; it1 != it2; ++it1)
    {
	msg.Push((*it1).first);
	msg.Push(static_cast<u8>((*it1).second));
    }

    // recruits
    msg.Push(static_cast<u8>(kingdom.recruits.GetID1()));
    msg.Push(static_cast<u8>(kingdom.recruits.GetID2()));

    // puzzle
    msg.Push(kingdom.puzzle_maps.to_string<char,std::char_traits<char>,std::allocator<char> >());

    // tents colors
    msg.Push(kingdom.visited_tents_colors);
}

void Game::IO::PackCastle(QueueMessage & msg, const Castle & castle)
{
    msg.Push(castle.GetCenter().x);
    msg.Push(castle.GetCenter().y);
    msg.Push(static_cast<u8>(castle.race));

    msg.Push(castle.modes);
    msg.Push(static_cast<u8>(castle.color));
    msg.Push(castle.name);
    msg.Push(castle.building);
	
    // mageguild
    {
	std::vector<Spell::spell_t>::const_iterator it1, it2;

	msg.Push(static_cast<u32>(castle.mageguild.general.spells.size()));

	it1 = castle.mageguild.general.spells.begin();
	it2 = castle.mageguild.general.spells.end();
	for(; it1 != it2; ++it1) msg.Push(static_cast<u8>(*it1));

	msg.Push(static_cast<u32>(castle.mageguild.library.spells.size()));

	it1 = castle.mageguild.library.spells.begin();
	it2 = castle.mageguild.library.spells.end();
	for(; it1 != it2; ++it1) msg.Push(static_cast<u8>(*it1));
    }

    // armies
    msg.Push(static_cast<u32>(castle.army.Size()));
    for(u32 jj = 0; jj < castle.army.Size(); ++jj)
    {
	const Army::Troop & troop = castle.army.At(jj);
	msg.Push(static_cast<u8>(troop()));
	msg.Push(troop.GetCount());
    }

    // dwelling
    msg.Push(static_cast<u32>(CASTLEMAXMONSTER));
    for(u32 jj = 0; jj < CASTLEMAXMONSTER; ++jj) msg.Push(castle.dwelling[jj]);
}

void Game::IO::PackHeroes(QueueMessage & msg, const Heroes & hero)
{
    msg.Push(static_cast<u8>(hero.portrait));
    msg.Push(static_cast<u8>(hero.race));

    msg.Push(hero.modes);
    msg.Push(static_cast<u8>(hero.color));
    msg.Push(hero.name);
    msg.Push(hero.attack);
    msg.Push(hero.defense);
    msg.Push(hero.knowledge);
    msg.Push(hero.power);
    msg.Push(hero.experience);
    msg.Push(hero.magic_point);
    msg.Push(hero.move_point);
    msg.Push(static_cast<u16>(hero.direction));
    msg.Push(hero.sprite_index);
    msg.Push(static_cast<u8>(hero.save_maps_object));
    msg.Push(hero.center.x);
    msg.Push(hero.center.y);

    // sec skills
    msg.Push(static_cast<u32>(hero.secondary_skills.size()));
    for(u32 jj = 0; jj < hero.secondary_skills.size(); ++jj)
    {
	const Skill::Secondary & sec = hero.secondary_skills[jj];
	msg.Push(static_cast<u8>(sec.Skill()));
	msg.Push(static_cast<u8>(sec.Level()));
    }

    // artifacts
    msg.Push(static_cast<u32>(hero.bag_artifacts.size()));
    for(u32 jj = 0; jj < hero.bag_artifacts.size(); ++jj)
    {
	msg.Push(static_cast<u8>(hero.bag_artifacts[jj].GetID()));
	msg.Push(hero.bag_artifacts[jj].GetExt());
    }

    // armies
    msg.Push(static_cast<u32>(hero.army.Size()));
    for(u32 jj = 0; jj < hero.army.Size(); ++jj)
    {
	const Army::Troop & troop = hero.army.At(jj);
	msg.Push(static_cast<u8>(troop()));
	msg.Push(troop.GetCount());
    }
	
    // spell book
    msg.Push(static_cast<u8>(hero.spell_book.active));
    {
	std::vector<Spell::spell_t>::const_iterator it1, it2;

	msg.Push(static_cast<u32>(hero.spell_book.spells.size()));

	it1 = hero.spell_book.spells.begin();
	it2 = hero.spell_book.spells.end();
	for(; it1 != it2; ++it1) msg.Push(static_cast<u8>(*it1));
    }
	
    // visit objects
    msg.Push(static_cast<u32>(hero.visit_object.size()));
    std::list<IndexObject>::const_iterator it1 = hero.visit_object.begin();
    std::list<IndexObject>::const_iterator it2 = hero.visit_object.end();
    for(; it1 != it2; ++it1)
    {
	msg.Push((*it1).first);
	msg.Push(static_cast<u8>((*it1).second));
    }

    // route path
    msg.Push(hero.path.dst);
    msg.Push(static_cast<u8>(hero.path.hide));
    msg.Push(static_cast<u32>(hero.path.size()));
    Route::Path::const_iterator ip1 = hero.path.begin();
    Route::Path::const_iterator ip2 = hero.path.end();
    for(; ip1 != ip2; ++ip1)
    {
	msg.Push(static_cast<u16>((*ip1).Direction()));
	msg.Push((*ip1).Penalty());
    }
}

bool Game::IO::LoadBIN(QueueMessage & msg)
{
    Settings & conf = Settings::Get();

    u8 byte8;
    u16 byte16;
    u32 byte32;
    u16 format;
    std::string str;

    // prepare World
    world.Reset();

    msg.Pop(byte16);
    if(byte16 != 0xFF01){ DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF01"); return false; }

    // format version
    msg.Pop(format);
    if(format > CURRENT_FORMAT_VERSION || format < LAST_FORMAT_VERSION)
    {
    	DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: unknown format: 0x" << std::hex << format);
    	return false;
    }
    else
    {
    	DEBUG(DBG_GAME , DBG_INFO, "Game::IO::LoadBIN: format: " << format);
    }


    // major version
    msg.Pop(byte8);
    // minor version
    msg.Pop(byte8);

    // svn
    msg.Pop(conf.svn_version);
    // time
    msg.Pop(byte32);
    // lang
    msg.Pop(str);
    if(str != "en" && str != conf.force_lang && !conf.Unicode())
    {
	std::string msg("This is an saved game is localized for lang = ");
	msg.append(str);
	msg.append(", and most of the messages will be displayed incorrectly.\n \n");
	msg.append("(tip: set unicode = on)");
	Dialog::Message("Warning!", msg, Font::BIG, Dialog::OK);
    }

    // maps
    msg.Pop(byte16);
    if(byte16 != 0xFF02) DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF02");

    if(FORMAT_VERSION_2031 <= format)
    {
	msg.Pop(conf.current_maps_file.size_w);
	msg.Pop(conf.current_maps_file.size_h);
    }
    else
    {
	msg.Pop(byte8); conf.current_maps_file.size_w = byte8;
	msg.Pop(byte8); conf.current_maps_file.size_h = byte8;
    }
    msg.Pop(conf.current_maps_file.file);
    msg.Pop(conf.current_maps_file.difficulty);
    msg.Pop(conf.current_maps_file.kingdom_colors);
    msg.Pop(conf.current_maps_file.human_colors);
    msg.Pop(conf.current_maps_file.computer_colors);
    msg.Pop(conf.current_maps_file.rnd_races);
    msg.Pop(conf.current_maps_file.conditions_wins);
    msg.Pop(conf.current_maps_file.wins1);
    msg.Pop(conf.current_maps_file.wins2);
    msg.Pop(conf.current_maps_file.wins3);
    msg.Pop(conf.current_maps_file.wins4);
    msg.Pop(conf.current_maps_file.conditions_loss);
    msg.Pop(conf.current_maps_file.loss1);
    msg.Pop(conf.current_maps_file.loss2);

    msg.Pop(byte16);
    if(byte16 != 0xFF03) DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF03");
    // races
    msg.Pop(byte32);
    for(u8 ii = 0; ii < byte32; ++ii)
    {
	msg.Pop(byte8);
	conf.current_maps_file.races[ii] = byte8;
    }
    // unions
    msg.Pop(byte32);
    for(u16 ii = 0; ii < byte32; ++ii)
    {
	msg.Pop(byte8);
	conf.current_maps_file.unions[ii] = byte8;
    }
    // maps name
    msg.Pop(conf.current_maps_file.name);
    // maps description
    msg.Pop(conf.current_maps_file.description);
    // game
    msg.Pop(byte16);
    if(byte16 != 0xFF04) DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF04");
    msg.Pop(byte8); conf.game_difficulty = Difficulty::Get(byte8);
    msg.Pop(byte8); conf.my_color = Color::Get(byte8);
    msg.Pop(byte8); conf.cur_color = Color::Get(byte8);
    msg.Pop(conf.game_type);
    msg.Pop(conf.players_colors);
    msg.Pop(conf.preferably_count_players);
#ifdef WITH_DEBUG
    msg.Pop(byte16);
#else
    msg.Pop(conf.debug);
#endif
    msg.Pop(byte32);
    // skip load interface options
    //conf.opt_game.ResetModes(MODES_ALL);
    //conf.opt_game.SetModes(byte32);
    msg.Pop(byte32);
    conf.opt_world.ResetModes(MODES_ALL);
    conf.opt_world.SetModes(byte32);
    msg.Pop(byte32);
    conf.opt_battle.ResetModes(MODES_ALL);
    conf.opt_battle.SetModes(byte32);

    // world
    msg.Pop(byte16);
    if(byte16 != 0xFF05) DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF05");
    msg.Pop(world.width);
    msg.Pop(world.height);

    if(FORMAT_VERSION_2031 <= format)
    {
	msg.Pop(world.ultimate_artifact);
    }
    else
    {
	msg.Pop(byte16); world.ultimate_artifact = byte16;
    }
    msg.Pop(world.uniq0);
    msg.Pop(byte8); world.week_name = Week::Get(byte8);
    msg.Pop(byte8); world.heroes_cond_wins = Heroes::ConvertID(byte8);
    msg.Pop(byte8); world.heroes_cond_loss = Heroes::ConvertID(byte8);
    msg.Pop(world.month);
    msg.Pop(world.week);
    msg.Pop(world.day);

    // tiles
    msg.Pop(byte16);
    if(byte16 != 0xFF06) DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF06");
    msg.Pop(byte32);
    world.vec_tiles.reserve(byte32);
    for(u32 maps_index = 0; maps_index < byte32; ++maps_index)
    {
	Maps::Tiles *tile = new Maps::Tiles(maps_index);
	UnpackTile(msg, *tile, format);
	world.vec_tiles.push_back(tile);
    }

    // heroes
    msg.Pop(byte16);
    if(byte16 != 0xFF07) DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF07");
    msg.Pop(byte32);
    world.vec_heroes.reserve(byte32);
    for(u32 ii = 0; ii < byte32; ++ii)
    {
	Heroes* hero = new Heroes();
	UnpackHeroes(msg, *hero, format);
	world.vec_heroes.push_back(hero);
    }

    // castles
    msg.Pop(byte16);
    if(byte16 != 0xFF08) DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF08");
    msg.Pop(byte32);
    world.vec_castles.reserve(byte32);
    for(u32 ii = 0; ii < byte32; ++ii)
    {
	Castle* castle = new Castle();
	UnpackCastle(msg, *castle, format);
	world.vec_castles.push_back(castle);
    }

    // kingdoms
    msg.Pop(byte16);
    if(byte16 != 0xFF09) DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF09");
    msg.Pop(byte32);
    world.vec_kingdoms.reserve(byte32);
    for(u32 ii = 0; ii < byte32; ++ii)
    {
	Kingdom* kingdom = new Kingdom();
	UnpackKingdom(msg, *kingdom, format);
	world.vec_kingdoms.push_back(kingdom);
    }

    // signs
    msg.Pop(byte16);
    if(byte16 != 0xFF0A) DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF0A");
    if(FORMAT_VERSION_2031 <= format)
    {
	msg.Pop(byte32);
	byte16 = byte32;
	world.map_sign.clear();
	for(u16 ii = 0; ii < byte16; ++ii)
	{
	    msg.Pop(byte32);
	    msg.Pop(str);
	    world.map_sign[byte32] = str;
	}
    }
    else
    {
	msg.Pop(byte32);
	world.map_sign.clear();
	for(u32 ii = 0; ii < byte32; ++ii)
	{
	    msg.Pop(byte16);
	    msg.Pop(str);
	    world.map_sign[byte16] = str;
	}
    }

    // captured object
    msg.Pop(byte16);
    if(byte16 != 0xFF0B) DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF0B");
    if(FORMAT_VERSION_2031 <= format)
    {
	msg.Pop(byte32);
	byte16 = byte32;
	world.map_captureobj.clear();
	for(u16 ii = 0; ii < byte16; ++ii)
	{
	    msg.Pop(byte32);
	    ObjectColor & value = world.map_captureobj[byte32];
	    msg.Pop(byte8); value.first = static_cast<MP2::object_t>(byte8);
	    msg.Pop(byte8); value.second = Color::Get(byte8);
	}
    }
    else
    {
	msg.Pop(byte32);
	world.map_captureobj.clear();
	for(u32 ii = 0; ii < byte32; ++ii)
	{
	    msg.Pop(byte16);
	    ObjectColor & value = world.map_captureobj[byte16];
	    msg.Pop(byte8); value.first = static_cast<MP2::object_t>(byte8);
	    msg.Pop(byte8); value.second = Color::Get(byte8);
	}
    }

    // rumors
    msg.Pop(byte16);
    if(byte16 != 0xFF0C) DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF0C");
    msg.Pop(byte32);
    world.vec_rumors.clear();
    world.vec_rumors.reserve(byte32);
    for(u32 ii = 0; ii < byte32; ++ii)
    {
	msg.Pop(str);
	world.vec_rumors.push_back(str);
    }

    // day events
    msg.Pop(byte16);
    if(byte16 != 0xFF0D) DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF0D");
    msg.Pop(byte32);
    world.vec_eventsday.clear();
    world.vec_eventsday.reserve(byte32);
    for(u32 ii = 0; ii < byte32; ++ii)
    {
	GameEvent::Day *event = new GameEvent::Day();
	u32 size;

	msg.Pop(size); event->resource.wood = size;
	msg.Pop(size); event->resource.mercury = size;
	msg.Pop(size); event->resource.ore = size;
	msg.Pop(size); event->resource.sulfur = size;
	msg.Pop(size); event->resource.crystal = size;
	msg.Pop(size); event->resource.gems = size;
	msg.Pop(size); event->resource.gold = size;
	msg.Pop(byte8);event->computer = byte8;
	msg.Pop(event->first);
	msg.Pop(event->subsequent);
	msg.Pop(event->colors);
	msg.Pop(event->message);

	world.vec_eventsday.push_back(event);
    }

    // coord events
    msg.Pop(byte16);
    if(byte16 != 0xFF0E) DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF0E");
    msg.Pop(byte32);
    world.vec_eventsmap.clear();
    world.vec_eventsmap.reserve(byte32);
    for(u32 ii = 0; ii < byte32; ++ii)
    {
	GameEvent::Coord *event = new GameEvent::Coord();
	u32 size;

	if(FORMAT_VERSION_2031 <= format)
	{
	    msg.Pop(size); event->index_map = size;
	}
	else
	{
	    msg.Pop(byte16); event->index_map = byte16;
	}
	msg.Pop(size); event->resource.wood = size;
	msg.Pop(size); event->resource.mercury = size;
	msg.Pop(size); event->resource.ore = size;
	msg.Pop(size); event->resource.sulfur = size;
	msg.Pop(size); event->resource.crystal = size;
	msg.Pop(size); event->resource.gems = size;
	msg.Pop(size); event->resource.gold = size;

	msg.Pop(byte8);event->artifact = Artifact::FromInt(byte8);
	msg.Pop(byte8);event->computer = byte8;
	msg.Pop(byte8);event->cancel = byte8;
	msg.Pop(event->colors);
	msg.Pop(event->message);

	world.vec_eventsmap.push_back(event);
    }
    
    // sphinx riddles
    msg.Pop(byte16);
    if(byte16 != 0xFF0F) DEBUG(DBG_GAME , DBG_WARN, "Game::IO::LoadBIN: 0xFF0F");
    msg.Pop(byte32);
    world.vec_riddles.clear();
    world.vec_riddles.reserve(byte32);
    for(u32 ii = 0; ii < byte32; ++ii)
    {
	GameEvent::Riddle *riddle = new GameEvent::Riddle();
	u32 size;
	
	if(FORMAT_VERSION_2031 <= format)
	{
	    msg.Pop(size); riddle->index_map = size;
	}
	else
	{
	    msg.Pop(byte16); riddle->index_map = byte16;
	}
	msg.Pop(size); riddle->resource.wood = size;
	msg.Pop(size); riddle->resource.mercury = size;
	msg.Pop(size); riddle->resource.ore = size;
	msg.Pop(size); riddle->resource.sulfur = size;
	msg.Pop(size); riddle->resource.crystal = size;
	msg.Pop(size); riddle->resource.gems = size;
	msg.Pop(size); riddle->resource.gold = size;
	msg.Pop(byte8);riddle->artifact = Artifact::FromInt(byte8);
	msg.Pop(byte8);riddle->quiet = byte8;

	msg.Pop(size);
	riddle->answers.reserve(size);
	for(u32 jj = 0; jj < size; ++jj)
	{
	    msg.Pop(str);
	    riddle->answers.push_back(str);
	}
	msg.Pop(riddle->message);

	world.vec_riddles.push_back(riddle);
    }

    msg.Pop(byte16);

    // sort castles to kingdoms
    std::vector<Castle *>::const_iterator itc1 = world.vec_castles.begin();
    std::vector<Castle *>::const_iterator itc2 = world.vec_castles.end();
    for(; itc1 != itc2; ++itc1)
        if(*itc1) world.GetKingdom((*itc1)->GetColor()).AddCastle(*itc1);

    // sort heroes to kingdoms
    std::vector<Heroes *>::const_iterator ith1 = world.vec_heroes.begin();
    std::vector<Heroes *>::const_iterator ith2 = world.vec_heroes.end();
    for(; ith1 != ith2; ++ith1)
        if(*ith1) world.GetKingdom((*ith1)->GetColor()).AddHeroes(*ith1);

    // regenerate puzzle surface
    Interface::GameArea::GenerateUltimateArtifactAreaSurface(world.ultimate_artifact, world.puzzle_surface);

    return byte16 == 0xFFFF;
}

void Game::IO::UnpackTile(QueueMessage & msg, Maps::Tiles & tile, u16 check_version)
{
    msg.Pop(tile.tile_sprite_index);
    msg.Pop(tile.tile_sprite_shape);
    msg.Pop(tile.mp2_object);
    msg.Pop(tile.quantity1);
    msg.Pop(tile.quantity2);
    msg.Pop(tile.quantity3);
    msg.Pop(tile.quantity4);
    msg.Pop(tile.fogs);
    msg.Pop(tile.unused1);
    if(FORMAT_VERSION_2031 <= check_version)
    {
	msg.Pop(tile.unused2);
	msg.Pop(tile.unused3);
    }

#ifdef WITH_DEBUG
    if(IS_DEVEL()) tile.fogs &= ~Settings::Get().MyColor();
#endif

    // addons 1
    u8 size;
    tile.addons_level1.clear();
    msg.Pop(size);
    for(u8 ii = 0; ii < size; ++ii)
    {
	Maps::TilesAddon addon;
	msg.Pop(addon.level);
	msg.Pop(addon.uniq);
	msg.Pop(addon.object);
	msg.Pop(addon.index);
	tile.addons_level1.push_back(addon);
    }

    // addons 2
    tile.addons_level2.clear();
    msg.Pop(size);
    for(u8 ii = 0; ii < size; ++ii)
    {
	Maps::TilesAddon addon;
	msg.Pop(addon.level);
	msg.Pop(addon.uniq);
	msg.Pop(addon.object);
	msg.Pop(addon.index);
	tile.addons_level2.push_back(addon);
    }
}

void Game::IO::UnpackKingdom(QueueMessage & msg, Kingdom & kingdom, u16 check_version)
{
    u8 byte8;
    u16 byte16;
    u32 byte32;

    msg.Pop(byte8);
    kingdom.color =Color::Get(byte8);

    msg.Pop(byte8);
    kingdom.control = Game::GetControl(byte8);

    msg.Pop(kingdom.flags);
    msg.Pop(kingdom.lost_town_days);
    msg.Pop(byte16);
    kingdom.ai_capital = byte16 ? world.GetCastle(byte16) : NULL;

    // funds
    msg.Pop(byte32); kingdom.resource.wood = byte32;
    msg.Pop(byte32); kingdom.resource.mercury = byte32;
    msg.Pop(byte32); kingdom.resource.ore = byte32;
    msg.Pop(byte32); kingdom.resource.sulfur = byte32;
    msg.Pop(byte32); kingdom.resource.crystal = byte32;
    msg.Pop(byte32); kingdom.resource.gems = byte32;
    msg.Pop(byte32); kingdom.resource.gold = byte32;

    // visit objects
    msg.Pop(byte32);
    kingdom.visit_object.clear();
    for(u32 jj = 0; jj < byte32; ++jj)
    {
	IndexObject io;
	if(FORMAT_VERSION_2031 <= check_version)
	{
	    msg.Pop(io.first);
	}
	else
	{
	    msg.Pop(byte16); io.first = byte16;
	}
	msg.Pop(byte8);
	io.second = static_cast<MP2::object_t>(byte8);
	kingdom.visit_object.push_back(io);
    }

    // recruits
    msg.Pop(byte8); kingdom.recruits.SetID1(byte8);
    msg.Pop(byte8); kingdom.recruits.SetID2(byte8);

    std::string str;

    // puzzle
    msg.Pop(str);
    kingdom.puzzle_maps = str.c_str();

    // visited tents
    msg.Pop(kingdom.visited_tents_colors);
}

void Game::IO::UnpackCastle(QueueMessage & msg, Castle & castle, u16 check_version)
{
    u8 byte8;
    u16 byte16;
    u32 byte32;

    if(FORMAT_VERSION_2031 <= check_version)
    {
	msg.Pop(castle.center.x);
	msg.Pop(castle.center.y);
    }
    else
    {
	msg.Pop(byte16); castle.center.x = byte16;
	msg.Pop(byte16); castle.center.y = byte16;
    }

    msg.Pop(byte8); castle.race = Race::Get(byte8);

    msg.Pop(castle.modes);
    msg.Pop(byte8); castle.color = Color::Get(byte8);
    msg.Pop(castle.name);
    msg.Pop(castle.building);

    // general
    msg.Pop(byte32);
    castle.mageguild.general.spells.clear();
    castle.mageguild.general.spells.reserve(byte32);
    for(u32 jj = 0; jj < byte32; ++jj)
    {
	msg.Pop(byte8);
	castle.mageguild.general.spells.push_back(Spell::FromInt(byte8));
    }
    // library
    msg.Pop(byte32);
    castle.mageguild.library.spells.clear();
    castle.mageguild.library.spells.reserve(byte32);
    for(u32 jj = 0; jj < byte32; ++jj)
    {
	msg.Pop(byte8);
	castle.mageguild.library.spells.push_back(Spell::FromInt(byte8));
    }

    // armies
    msg.Pop(byte32);
    for(u32 jj = 0; jj < castle.army.Size(); ++jj)
    {
	msg.Pop(byte8);
	castle.army.At(jj).SetMonster(Monster::FromInt(byte8));
        msg.Pop(byte32);
        castle.army.At(jj).SetCount(byte32);
    }
    // dwelling
    msg.Pop(byte32);
    for(u32 jj = 0; jj < CASTLEMAXMONSTER; ++jj) msg.Pop(castle.dwelling[jj]);
}

void Game::IO::UnpackHeroes(QueueMessage & msg, Heroes & hero, u16 check_version)
{
    u8 byte8;
    u16 byte16;
    u32 byte32;

    msg.Pop(byte8); hero.portrait = Heroes::ConvertID(byte8);
    msg.Pop(byte8); hero.race = Race::Get(byte8);

    msg.Pop(hero.modes);
    msg.Pop(byte8); hero.color = Color::Get(byte8);
    msg.Pop(hero.name);
    msg.Pop(hero.attack);
    msg.Pop(hero.defense);
    msg.Pop(hero.knowledge);
    msg.Pop(hero.power);
    msg.Pop(hero.experience);
    msg.Pop(hero.magic_point);
    msg.Pop(hero.move_point);
    msg.Pop(byte16); hero.direction = Direction::FromInt(byte16);
    msg.Pop(hero.sprite_index);
    msg.Pop(byte8); hero.save_maps_object = static_cast<MP2::object_t>(byte8);
    if(FORMAT_VERSION_2031 <= check_version)
    {
	msg.Pop(hero.center.x);
	msg.Pop(hero.center.y);
    }
    else
    {
	msg.Pop(byte16); hero.center.x = byte16;
	msg.Pop(byte16); hero.center.y = byte16;
    }

    // sec skills
    hero.secondary_skills.clear();
    hero.secondary_skills.reserve(HEROESMAXSKILL);
    msg.Pop(byte32);
    for(u32 jj = 0; jj < byte32; ++jj)
    {
	Skill::Secondary skill;
	msg.Pop(byte8); skill.SetSkill(Skill::Secondary::Skill(byte8));
	msg.Pop(byte8); skill.SetLevel(byte8);
	hero.secondary_skills.push_back(skill);
    }

    // artifacts
    std::fill(hero.bag_artifacts.begin(), hero.bag_artifacts.end(), Artifact::UNKNOWN);
    msg.Pop(byte32);
    for(u32 jj = 0; jj < hero.bag_artifacts.size(); ++jj)
    {
	msg.Pop(byte8);
	hero.bag_artifacts[jj].Set(Artifact::FromInt(byte8));

	msg.Pop(byte8);
	hero.bag_artifacts[jj].SetExt(byte8);
    }

    // armies
    msg.Pop(byte32);
    for(u32 jj = 0; jj < hero.army.Size(); ++jj)
    {
	msg.Pop(byte8);
	hero.army.At(jj).SetMonster(Monster::FromInt(byte8));
        msg.Pop(byte32);
        hero.army.At(jj).SetCount(byte32);
    }

    // spell book
    hero.spell_book.spells.clear();
    msg.Pop(byte8);
    hero.spell_book.active = byte8;

    msg.Pop(byte32);
    hero.spell_book.spells.reserve(byte32);
    for(u32 jj = 0; jj < byte32; ++jj)
    {
	msg.Pop(byte8);
	hero.spell_book.spells.push_back(Spell::FromInt(byte8));
    }

    // visit objects
    msg.Pop(byte32);
    hero.visit_object.clear();
    for(u32 jj = 0; jj < byte32; ++jj)
    {
	IndexObject io;
	if(FORMAT_VERSION_2031 <= check_version)
	{
	    msg.Pop(io.first);
	}
	else
	{
	    msg.Pop(byte16); io.first = byte16;
	}
	msg.Pop(byte8);
	io.second = static_cast<MP2::object_t>(byte8);
	hero.visit_object.push_back(io);
    }

    // route path
    if(FORMAT_VERSION_2031 <= check_version)
    {
	msg.Pop(hero.path.dst);
    }
    else
    {
	msg.Pop(byte16);
	hero.path.dst = byte16;
    }
    msg.Pop(byte8);
    hero.path.hide = byte8;
    msg.Pop(byte32);
    hero.path.clear();
    for(u32 jj = 0; jj < byte32; ++jj)
    {
	Route::Step step;
	msg.Pop(byte16);
	step.first = Direction::FromInt(byte16);
	msg.Pop(step.second);
	hero.path.push_back(step);
    }
}

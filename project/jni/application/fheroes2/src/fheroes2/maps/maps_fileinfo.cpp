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

#include <cstring>
#include <locale>
#include <algorithm>
#include <fstream>
#include "difficulty.h"
#include "color.h"
#include "race.h"
#include "world.h"
#include "settings.h"
#include "dir.h"
#include "maps_fileinfo.h"

#define LENGTHNAME		16
#define LENGTHDESCRIPTION	143

template <typename CharType>
bool AlphabeticalCompare(const std::basic_string<CharType> & lhs, const std::basic_string<CharType> & rhs)
{
    return std::use_facet< std::collate< CharType > >( std::locale() ).compare( lhs.data(), lhs.data() + lhs.size(),
		    rhs.data(), rhs.data() + rhs.size() ) == -1;
}

Race::race_t ByteToRace(u8 byte)
{
    switch(byte)
    {
	case 0x00:	return Race::KNGT;
	case 0x01:	return Race::BARB;
	case 0x02:	return Race::SORC;
	case 0x03:	return Race::WRLK;
	case 0x04:	return Race::WZRD;
	case 0x05:	return Race::NECR;
	case 0x06:	return Race::MULT;
	case 0x07:	return Race::RAND;

	default: 	break;
    }

    return Race::BOMG;
}

Maps::FileInfo::FileInfo() : difficulty(Difficulty::EASY),
    kingdom_colors(0), human_colors(0), computer_colors(0), rnd_races(0), localtime(0), with_heroes(false)
{
    for(u8 ii = 0; ii < KINGDOMMAX; ++ii)
    {
	races[ii] = Race::BOMG;
	unions[ii] = Color::GetFromIndex(ii);
    }
}

bool Maps::FileInfo::ReadSAV(const std::string & filename)
{
    return Game::LoadSAV2FileInfo(filename,  *this);
}

bool Maps::FileInfo::ReadMP2(const std::string & filename)
{
    if(filename.empty()) return false;

    std::ifstream fd(filename.c_str(), std::ios::binary);

    if(!fd.is_open())
    {
	DEBUG(DBG_GAME , DBG_WARN, "Maps::FileInfo::ReadMP2: file not found " << filename);
	return false;
    }

    file = filename;
    kingdom_colors = 0;
    human_colors = 0;
    computer_colors = 0;
    rnd_races = 0;
    localtime = 0;

    u8  byte8;
    u16 byte16;
    u32 byte32;

    // magic byte
    fd.read(reinterpret_cast<char *>(&byte32), sizeof(byte32));
    SwapLE32(byte32);

    if(byte32 != 0x0000005C)
    {
	DEBUG(DBG_GAME , DBG_WARN, "Maps::FileInfo::ReadMP2: incorrect maps file " << filename);
	fd.close();
	return false;
    }

    // level
    fd.read(reinterpret_cast<char *>(&byte16), sizeof(byte16));
    SwapLE16(byte16);

    switch(byte16)
    {
	case 0x00:
	    difficulty = Difficulty::EASY;
	    break;

	case 0x01:
	    difficulty = Difficulty::NORMAL;
	    break;

	case 0x02:
	    difficulty = Difficulty::HARD;
	    break;

	case 0x03:
	    difficulty = Difficulty::EXPERT;
	    break;

	default:
	    DEBUG(DBG_GAME , DBG_WARN, "Maps::FileInfo::ReadMP2: incorrect difficulty, maps: " << filename << ". Load EASY default.");
	break;
    }

    // width
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    size_w = byte8;

    // height
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    size_h = byte8;

    // kingdom color blue
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) kingdom_colors |= Color::BLUE;

    // kingdom color green
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) kingdom_colors |= Color::GREEN;

    // kingdom color red
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) kingdom_colors |= Color::RED;

    // kingdom color yellow
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) kingdom_colors |= Color::YELLOW;

    // kingdom color orange
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) kingdom_colors |= Color::ORANGE;

    // kingdom color purple
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) kingdom_colors |= Color::PURPLE;

    // allow color blue
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) human_colors |= Color::BLUE;

    // allow color green
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) human_colors |= Color::GREEN;

    // allow color red
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) human_colors |= Color::RED;

    // allow color yellow
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) human_colors |= Color::YELLOW;

    // allow color orange
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) human_colors |= Color::ORANGE;

    // allow color purple
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) human_colors |= Color::PURPLE;

    // rnd color blue
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) computer_colors |= Color::BLUE;

    // rnd color green
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) computer_colors |= Color::GREEN;

    // rnd color red
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) computer_colors |= Color::RED;

    // rnd color yellow
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) computer_colors |= Color::YELLOW;

    // rnd color orange
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) computer_colors |= Color::ORANGE;

    // rnd color purple
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    if(byte8) computer_colors |= Color::PURPLE;

    // kingdom count
    // fd.seekg(0x1A, std::ios_base::beg);
    // fd.read(&byte8, 1);

    // wins
    fd.seekg(0x1D, std::ios_base::beg);
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    conditions_wins = byte8;

    // data wins
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    wins1 = byte8;
    // data wins
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    wins2 = byte8;
    // data wins
    fd.read(reinterpret_cast<char *>(&byte16), 2);
    SwapLE16(byte16);
    wins3 = byte16;
    // data wins
    fd.seekg(0x2c, std::ios_base::beg);
    fd.read(reinterpret_cast<char *>(&byte16), 2);
    SwapLE16(byte16);
    wins4 = byte16;

    // loss
    fd.seekg(0x22, std::ios_base::beg);
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    conditions_loss = byte8;
    // data loss
    fd.read(reinterpret_cast<char *>(&byte16), 2);
    SwapLE16(byte16);
    loss1 = byte16;
    // data loss
    fd.seekg(0x2e, std::ios_base::beg);
    fd.read(reinterpret_cast<char *>(&byte16), 2);
    SwapLE16(byte16);
    loss2 = byte16;

    // start with hero
    fd.seekg(0x25, std::ios_base::beg);
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    with_heroes = 0 == byte8;

    // race color
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    races[0] = ByteToRace(byte8);
    if(Race::RAND == races[0]) rnd_races |= Color::BLUE;
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    races[1] = ByteToRace(byte8);
    if(Race::RAND == races[1]) rnd_races |= Color::GREEN;
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    races[2] = ByteToRace(byte8);
    if(Race::RAND == races[2]) rnd_races |= Color::RED;
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    races[3] = ByteToRace(byte8);
    if(Race::RAND == races[3]) rnd_races |= Color::YELLOW;
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    races[4] = ByteToRace(byte8);
    if(Race::RAND == races[4]) rnd_races |= Color::ORANGE;
    fd.read(reinterpret_cast<char *>(&byte8), 1);
    races[5] = ByteToRace(byte8);
    if(Race::RAND == races[5]) rnd_races |= Color::PURPLE;

    // name
    char bufname[LENGTHNAME];
    fd.seekg(0x3A, std::ios_base::beg);
    fd.read(bufname, LENGTHNAME);
    bufname[LENGTHNAME - 1] = 0;
    name = _(bufname);

    // description
    char bufdescription[LENGTHDESCRIPTION];
    fd.seekg(0x76, std::ios_base::beg);
    fd.read(bufdescription, LENGTHDESCRIPTION);
    bufdescription[LENGTHDESCRIPTION - 1] = 0;
    description = _(bufdescription);

    fd.close();
    
    //fill unions
    if(4 == conditions_wins)
    {
	u16 index = 0;
	u8 side1 = 0;
	u8 side2 = 0;

	for(Color::color_t cl = Color::BLUE; cl < Color::GRAY; ++cl, ++index) if(cl & kingdom_colors)
	{
	    if(index < wins3)
		side1 |= cl;
	    else
		side2 |= cl;
	}

	for(u8 ii = 0; ii < KINGDOMMAX; ++ii)
	{
	    Color::color_t cl = Color::GetFromIndex(ii);
	
	    if(side1 & cl)
		unions[ii] = side1;
	    else
	    if(side2 & cl)
		unions[ii] = side2;
	    else
		unions[ii] = cl;
	}
    }

    return true;
}

bool Maps::FileInfo::FileSorting(const FileInfo & fi1, const FileInfo & fi2)
{
    return AlphabeticalCompare(fi1.file, fi2.file);
}

bool Maps::FileInfo::NameSorting(const FileInfo & fi1, const FileInfo & fi2)
{
    return AlphabeticalCompare(fi1.name, fi2.name);
}

bool Maps::FileInfo::NameCompare(const FileInfo & fi1, const FileInfo & fi2)
{
    return fi1.name == fi2.name;
}

u8 Maps::FileInfo::KingdomRace(u8 color) const
{
    switch(color)
    {
        case Color::BLUE:       return races[0];
        case Color::GREEN:      return races[1];
	case Color::RED:        return races[2];
        case Color::YELLOW:     return races[3];
        case Color::ORANGE:     return races[4];
        case Color::PURPLE:     return races[5];
        default: break;
    }
    return 0;
}

u16 Maps::FileInfo::ConditionWins(void) const
{
    switch(conditions_wins)
    {
	case 0: return GameOver::WINS_ALL;
	case 1:	return wins2 ? GameOver::WINS_TOWN | GameOver::WINS_ALL : GameOver::WINS_TOWN;
	case 2:	return wins2 ? GameOver::WINS_HERO | GameOver::WINS_ALL : GameOver::WINS_HERO;
	case 3:	return wins2 ? GameOver::WINS_ARTIFACT | GameOver::WINS_ALL : GameOver::WINS_ARTIFACT;
	case 4:	return GameOver::WINS_SIDE;
	case 5:	return wins2 ? GameOver::WINS_GOLD | GameOver::WINS_ALL : GameOver::WINS_GOLD;
	default: break;
    }

    return GameOver::COND_NONE;
}

u16 Maps::FileInfo::ConditionLoss(void) const
{
    switch(conditions_loss)
    {
	case 0: return GameOver::LOSS_ALL;
        case 1:	return GameOver::LOSS_TOWN;
        case 2: return GameOver::LOSS_HERO;
        case 3: return GameOver::LOSS_TIME;
        default: break;
    }

    return GameOver::COND_NONE;
}

bool Maps::FileInfo::WinsCompAlsoWins(void) const
{
    return ((GameOver::WINS_TOWN | GameOver::WINS_GOLD) & ConditionWins()) && wins1;
}

bool Maps::FileInfo::WinsAllowNormalVictory(void) const
{
    return ((GameOver::WINS_TOWN | GameOver::WINS_ARTIFACT | GameOver::WINS_GOLD) & ConditionWins()) && wins2;
}

Artifact::artifact_t Maps::FileInfo::WinsFindArtifact(void) const
{
    return wins3 ? Artifact::FromInt(wins3 - 1) : Artifact::UNKNOWN;
}

bool Maps::FileInfo::WinsFindUltimateArtifact(void) const
{
    return 0 == wins3;
}

u32 Maps::FileInfo::WinsAccumulateGold(void) const
{
    return wins3 * 1000;
}

u16 Maps::FileInfo::WinsSidePart(void) const
{
    // FIX:: Maps::FileInfo::ConditionSidePart
    return wins3;
}

u32 Maps::FileInfo::WinsMapsIndexObject(void) const
{
    return wins4 * world.w() + wins3;
}

u32 Maps::FileInfo::LossMapsIndexObject(void) const
{
    return loss2 * world.w() + loss1;
}

u16 Maps::FileInfo::LossCountDays(void) const
{
    return loss1;
}

u8 Maps::FileInfo::HumanOnlyColors(void) const
{
    return human_colors & ~(human_colors & computer_colors);
}

u8 Maps::FileInfo::ComputerOnlyColors(void) const
{
    return computer_colors & ~(human_colors & computer_colors);
}

bool Maps::FileInfo::isAllowCountPlayers(u8 colors) const
{
    const u8 human_only = Color::Count(HumanOnlyColors());
    const u8 comp_human = Color::Count(human_colors & computer_colors);

    return human_only <= colors && colors <= human_only + comp_human;
}

bool Maps::FileInfo::isMultiPlayerMap(void) const
{
    return 1 < Color::Count(HumanOnlyColors());
}

void Maps::FileInfo::Dump(void) const
{
    VERBOSE("Maps::FileInfo::Dump: " << "file: " << file << ", name: " << name << ", kingdom colors: " << static_cast<int>(kingdom_colors) << \
	", human colors: " << static_cast<int>(human_colors) << ", computer colors: " << static_cast<int>(computer_colors) << ", rnd races: " << \
	static_cast<int>(rnd_races) << ", conditions wins: " << static_cast<int>(conditions_wins) << ", wins1: " << static_cast<int>(wins1) << \
	", wins2: " << static_cast<int>(wins2) << ", wins3: " << wins3 << ", wins4: " << wins4 << ", conditions loss: " << static_cast<int>(conditions_loss) << \
        ", loss1: " << loss1 << ", loss2: " << loss2);
}

bool PrepareMapsFileInfoList(MapsFileInfoList & lists, bool multi)
{
    const Settings & conf = Settings::Get();
    Dir dir;

    ListMapsDirectory::const_iterator it1 = conf.GetListMapsDirectory().begin();
    ListMapsDirectory::const_iterator it2 = conf.GetListMapsDirectory().end();

    for(; it1 != it2; ++it1)
    {
        dir.Read(*it1, ".mp2", false);
        dir.Read(conf.LocalPrefix() + SEPARATOR + *it1, ".mp2", false);
        // loyality version
        if(conf.PriceLoyaltyVersion())
        {
    	    dir.Read(*it1, ".mx2", false);
	    dir.Read(conf.LocalPrefix() + SEPARATOR + *it1, ".mx2", false);
	}
    }

    if(dir.empty()) return false;

    dir.sort();
    dir.unique();

    lists.reserve(dir.size());

    for(Dir::const_iterator itd = dir.begin(); itd != dir.end(); ++itd)
    {
	Maps::FileInfo fi;
	if(fi.ReadMP2(*itd)) lists.push_back(fi);
    }

    std::sort(lists.begin(), lists.end(), Maps::FileInfo::NameSorting);
    lists.resize(std::unique(lists.begin(), lists.end(), Maps::FileInfo::NameCompare) - lists.begin());

    if(multi == false)
    {
	MapsFileInfoList::iterator it = std::remove_if(lists.begin(), lists.end(), std::mem_fun_ref(&Maps::FileInfo::isMultiPlayerMap));
	if(it != lists.begin()) lists.resize(std::distance(lists.begin(), it));
    }

    // set preferably count filter
    if(conf.PreferablyCountPlayers())
    {

	MapsFileInfoList::iterator it = std::remove_if(lists.begin(), lists.end(), std::not1(std::bind2nd(std::mem_fun_ref(&Maps::FileInfo::isAllowCountPlayers), conf.PreferablyCountPlayers())));
	if(it != lists.begin()) lists.resize(std::distance(lists.begin(), it));
    }

    return lists.size();
}

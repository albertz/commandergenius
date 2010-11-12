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

#include <algorithm>
#include "color.h"
#include "dialog.h"
#include "settings.h"
#include "gameevent.h"

#define SIZEMESSAGE 400

GameEvent::Day::Day() : computer(false), first(MAXU16), subsequent(0), colors(0)
{
}

GameEvent::Day::Day(const void *ptr)
{
    const u8  *ptr8  = static_cast<const u8 *>(ptr);
    u16 byte16 = 0;
    u32 byte32 = 0;

    // id
    if(0x00 != *ptr8)
    {
	DEBUG(DBG_GAME , DBG_WARN, "GameEvent::Day: unknown magic id");
	return;
    }
    ++ptr8;

    // resource
    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.wood = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.mercury = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.ore = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.sulfur = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.crystal = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.gems = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.gold = byte32;

    // skip artifact
    byte16 = ReadLE16(ptr8);
    ++ptr8;
    ++ptr8;

    // allow computer
    byte16 = ReadLE16(ptr8);
    ++ptr8;
    ++ptr8;
    computer = byte16;

    // day of first occurent
    byte16 = ReadLE16(ptr8);
    ++ptr8;
    ++ptr8;
    first = byte16;

    // subsequent occurrences
    byte16 = ReadLE16(ptr8);
    ++ptr8;
    ++ptr8;
    subsequent = byte16;

    ptr8 += 6;

    colors = 0;
    
    // blue
    if(*ptr8) colors |= Color::BLUE;
    ++ptr8;

    // green
    if(*ptr8) colors |= Color::GREEN;
    ++ptr8;

    // red
    if(*ptr8) colors |= Color::RED;
    ++ptr8;

    // yellow
    if(*ptr8) colors |= Color::YELLOW;
    ++ptr8;

    // orange
    if(*ptr8) colors |= Color::ORANGE;
    ++ptr8;

    // purple
    if(*ptr8) colors |= Color::PURPLE;
    ++ptr8;

    // message
    message = std::string(_(reinterpret_cast<const char *>(ptr8)));

    //if(SIZEMESSAGE < message.size()) DEBUG(DBG_GAME , DBG_WARN, "GameEvent::Day: long message, incorrect block?");

    DEBUG(DBG_GAME , DBG_INFO, "GameEvent::Day: add: " << message);
}

GameEvent::Coord::Coord() : index_map(-1), artifact(Artifact::UNKNOWN), computer(false), cancel(true), colors(0)
{
}

GameEvent::Coord::Coord(s32 index, const void *ptr) : index_map(index)
{
    const u8  *ptr8  = static_cast<const u8 *>(ptr);
    u16 byte16 = 0;
    u32 byte32 = 0;

    // id
    if(0x01 != *ptr8)
    {
	DEBUG(DBG_GAME , DBG_WARN, "GameEvent::Coord: unknown magic id");
	return;
    }
    ++ptr8;

    // resource
    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.wood = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.mercury = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.ore = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.sulfur = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.crystal = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.gems = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.gold = byte32;

    // artifact
    byte16 = ReadLE16(ptr8);
    ++ptr8;
    ++ptr8;
    artifact = (0xffff != byte16 && Artifact::MAGIC_BOOK > byte16 ? Artifact::FromInt(byte16) : Artifact::UNKNOWN);

    // allow computer
    computer = *ptr8;
    ++ptr8;

    // cancel event after first visit
    cancel = *ptr8;
    ptr8 += 11;

    colors = 0;
    
    // blue
    if(*ptr8) colors |= Color::BLUE;
    ++ptr8;

    // green
    if(*ptr8) colors |= Color::GREEN;
    ++ptr8;

    // red
    if(*ptr8) colors |= Color::RED;
    ++ptr8;

    // yellow
    if(*ptr8) colors |= Color::YELLOW;
    ++ptr8;

    // orange
    if(*ptr8) colors |= Color::ORANGE;
    ++ptr8;

    // purple
    if(*ptr8) colors |= Color::PURPLE;
    ++ptr8;

    // message
    message = std::string(_(reinterpret_cast<const char *>(ptr8)));
    
    DEBUG(DBG_GAME , DBG_INFO, "GameEvent::Coord: add: " << message);
}

GameEvent::Riddle::Riddle() : index_map(-1), artifact(Artifact::UNKNOWN), quiet(true)
{
}

GameEvent::Riddle::Riddle(s32 index, const void *ptr) : index_map(index), quiet(true)
{
    const u8  *ptr8  = static_cast<const u8 *>(ptr);
    u16 byte16 = 0;
    u32 byte32 = 0;
    answers.reserve(9);

    // id
    if(0x00 != *ptr8)
    {
	DEBUG(DBG_GAME , DBG_WARN, "GameEvent::Day: unknown magic id");
	return;
    }
    ++ptr8;

    // resource
    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.wood = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.mercury = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.ore = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.sulfur = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.crystal = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.gems = byte32;

    byte32 = ReadLE32(ptr8);
    ptr8 += 4;;
    resource.gold = byte32;

    // artifact
    byte16 = ReadLE16(ptr8);
    ++ptr8;
    ++ptr8;
    artifact = (0xffff != byte16 && Artifact::MAGIC_BOOK > byte16 ? Artifact::FromInt(byte16) : Artifact::UNKNOWN);

    // count answers
    u8 count = *ptr8;
    ++ptr8;

    // answers
    for(u8 i = 0; i < 8; ++i)
    {
	std::string str(reinterpret_cast<const char *>(ptr8));
	String::Lower(str);
	if(count-- && str.size()){ answers.push_back(str); answers.push_back(_(reinterpret_cast<const char *>(ptr8))); };
	ptr8 += 13;
    }

    // message
    message = std::string(_(reinterpret_cast<const char *>(ptr8)));
    
    DEBUG(DBG_GAME , DBG_INFO, "GameEvent::Riddle: add: " << message);
}

bool GameEvent::Riddle::AnswerCorrect(const std::string & answer)
{
    return answers.end() != std::find(answers.begin(), answers.end(), answer);
}

bool GameEvent::Riddle::isValid(void) const
{
    return !quiet;
}

void GameEvent::Riddle::SetQuiet(void)
{
    quiet = true;
    artifact = Artifact::UNKNOWN;
    resource = Resource::funds_t();
}

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
#ifndef H2GAMEEVENT_H
#define H2GAMEEVENT_H

#include <vector>
#include <string>
#include "resource.h"
#include "artifact.h"
#include "game_io.h"
#include "gamedefs.h"

namespace GameEvent
{

class Day
{
public:
    Day();
    Day(const void *ptr);
    
    bool AllowComputer(void) const{ return computer; }
    const Resource::funds_t & GetResource(void) const{ return resource; }
    u16 GetFirst(void) const{ return first; }
    u16 GetSubsequent(void) const{ return subsequent; }
    u8  GetColors(void) const{ return colors; }
    const std::string & GetMessage(void) const{ return message; }

private:
    friend class Game::IO;

    Resource::funds_t resource;
    bool computer;
    u16 first;
    u16 subsequent;
    u8 colors;
    std::string message;
};

class Coord
{
public:
    Coord();
    Coord(s32 index, const void *ptr);

    bool AllowComputer(void) const{ return computer; }
    const Resource::funds_t & GetResource(void) const{ return resource; }
    s32 GetIndex(void) const{ return index_map; }
    u8  GetColors(void) const{ return colors; }
    const std::string & GetMessage(void) const{ return message; }
    Artifact::artifact_t GetArtifact(void) const {return artifact; }

private:
    friend class Game::IO;

    s32 index_map;
    Resource::funds_t resource;
    Artifact::artifact_t artifact;
    bool computer;
    bool cancel;
    u8 colors;
    std::string message;
};

class Riddle
{
public:
    Riddle();
    Riddle(s32 index, const void *ptr);

    const Resource::funds_t & GetResource(void) const{ return resource; }
    Artifact::artifact_t GetArtifact(void) const {return artifact; }
    bool AnswerCorrect(const std::string & answer);
    s32 GetIndex(void) const{ return index_map; }
    const std::string & GetMessage(void) const{ return message; }
    bool isValid(void) const;
    void SetQuiet(void);

private:
    friend class Game::IO;

    s32 index_map;
    Resource::funds_t resource;
    Artifact::artifact_t artifact;
    std::vector<std::string> answers;
    std::string message;
    bool quiet;
};

}

#endif

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
#ifndef H2TOOLS_H
#define H2TOOLS_H

#include <string>
#include "localevent.h"
#include "types.h"

namespace String
{
    void Trim(std::string &str);
    void Lower(std::string &str);
    void Upper(std::string &str);
    void AddInt(std::string &str, int value);
    int  ToInt(const std::string &str);
    bool Compare(const std::string &str1, const std::string &str2, bool sensitive = true);

    void Replace(std::string &, const char*, const char *);
    void Replace(std::string &, const char*, const std::string &);
    void Replace(std::string &, const char*, int);

    void AppendKey(std::string &, KeySym, u16);

    // from SDL_ttf
    void UTF8_to_UNICODE(u16 *unicode, const char *utf8, int len);
    void UNICODE_to_UTF8(std::string & utf8, const u16 *unicode, size_t len);

}

int Sign(int);
KeySym KeySymFromChar(char);
bool PressIntKey(u32 min, u32 max, u32 & result);

std::string GetDirname(const std::string &);
std::string GetBasename(const std::string &);

u32 GetMemoryUsage(void);

bool StoreMemToFile(const std::vector<u8> &, const std::string &);
bool StoreFileToMem(std::vector<u8> &, const std::string &);
bool FilePresent(const std::string &);

#endif

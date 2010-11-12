/***************************************************************************
 *   Copyright (C) 2010 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   Part of SDL++ Engine:                                                 *
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

#ifndef TINYCONFIG_H
#define TINYCONFIG_H

#include <utility>
#include <ostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <list>

namespace Tiny
{
    struct Value
    {
	Value();
	Value(int);
	Value(const char*);

	void operator= (int);
	void operator= (const char*);

	int ival;
	std::string sval;
    };

    struct Entry : public std::pair<std::string, Value>
    {
	Entry();
	Entry(const char*, const char*);
	Entry(const char*, int);

	const std::string & StrParams(void) const;
	int IntParams(void) const;
	
	bool IsKey(const char*) const;
	bool IsValue(const char*) const;
	bool IsValue(int) const;
    };

    std::ostream & operator<< (std::ostream &, const Entry &);

    typedef std::list<Entry> Entries;
    typedef std::list<Entry>::iterator EntryIterator;
    typedef std::list<Entry>::const_iterator EntryConstIterator;

    class Config
    {
    public:
	Config();
	~Config();

	bool Load(const char*);
	bool Save(const char*);
	void Dump(std::ostream &);
	void Clear(void);

	void SetSeparator(char);
	void SetComment(char);

	int IntParams(const char*) const;
	const char* StrParams(const char*) const;
	void GetParams(const char*, std::list<std::string> &) const;

    	void AddEntry(const char*, const char*, bool uniq = true);
	void AddEntry(const char*, int, bool uniq = true);

	const Entry* Find(const char*) const;
	const Entries & GetEntries(void) const;

    protected:
	EntryIterator FindEntry(std::string);
	EntryConstIterator FindEntry(std::string) const;

	char separator;
	char comment;
	Entries entries;
    };
}

#endif

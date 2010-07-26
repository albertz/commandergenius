/*
*	Copyright (C) 2005 Chai Braudo (braudo@users.sourceforge.net)
*
*	This file is part of Jooleem - http://sourceforge.net/projects/jooleem
*
*   Jooleem is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   Jooleem is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with Jooleem; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _FONTMANAGER_H_
#define _FONTMANAGER_H_

// A singleton font manager class.
// The user can load fonts into the manager. On loading, the user defines
// a unique string identifier that is associated with the font. The user can 
// then access the stored fonts using that ID.
//
// Example:
// FontManager::GetInstance()->AddFont("MainFont", "arial.ttf", 16);
// Font *font = FontManager::GetInstance()->GetFont("MainFont");

#include <map>
#include <string>
#include "../common/font.h"

using namespace std;

class FontManager
{
	private:
		map<string, Font*> m_map;	// Map string IDs to font objects

	public:
		Font *AddFont(string ID, string fileName, int size);
		Font *GetFont(string ID);
		
		static FontManager* GetInstance();
		void Cleanup();

	private:
		static FontManager* m_instance;
		FontManager(){};					
		FontManager(const FontManager&){};
		FontManager& operator= (const FontManager&){};
};

#endif


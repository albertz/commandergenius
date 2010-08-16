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

#include "../common/fontmanager.h"
#include "../common/trap.h"


FontManager* FontManager::m_instance = NULL;

// Returns an m_instance of the class. If no m_instance exists, the method 
// creates a new one.
FontManager* FontManager::GetInstance()
{
	if (m_instance == NULL)
	{
		{
			m_instance = new FontManager();
		}
	}

	return m_instance;
}

// Adds a new font to the manager.
// If a resource with the same ID exists, a pointer to it is returned.
// Otherwise, the method returns a pointer to the new font.
Font* FontManager::AddFont(string ID, string fileName, int size)
{
	// Make sure the ID does not already exist in the map:
	map<string, Font*>::iterator itr = m_map.find(ID);
	if (itr != m_map.end())
		return itr->second;

	Font *font;
	{
		font = new Font(fileName, size);
	}

	m_map.insert(pair<string, Font*>(ID, font));

	return font;
}


// Gets a font by its ID.
// If the ID does not exist, NULL is returned.
Font *FontManager::GetFont(string ID)
{
	map<string, Font*>::iterator itr = m_map.find(ID);
	if (itr == m_map.end())
		return NULL;

	return itr->second;
}
		

// Releases all the loaded fonts.
void FontManager::Cleanup()
{
	map<string, Font*>::iterator itr;
	for (itr = m_map.begin() ; itr != m_map.end() ; itr++)
		delete itr->second;

	m_map.clear();
}


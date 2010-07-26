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

#ifndef _SURFACEMANAGER_H_
#define _SURFACEMANAGER_H_

// A singleton SDL surface manager class.
// The user can use the class to load surfaces from image files. On loading, the 
// user defines a unique string identifier that is associated with the surface. The user
// can then access the stored surfaces using that ID.
//
// Example:
// SurfaceManager::GetInstance()->AddSurface("SmallButton", "small_button.png");
// SDL_Surface *surface = SurfaceManager::GetInstance()->GetSurface("SmallButton");

#include "SDL.h"
#include <map>
#include <string>

using namespace std;

class SurfaceManager
{
	private:
		map<string, SDL_Surface*> m_map;	// Map string IDs to font objects

	public:
		SDL_Surface *AddSurface(string ID, string fileName);
		SDL_Surface *AddSurface(string ID, SDL_Surface *surface);
		SDL_Surface *GetSurface(string ID);
		
		static SurfaceManager* GetInstance();
		void Cleanup();

		static void Blit(SDL_Surface *source, SDL_Surface *dest, float alpha, SDL_Rect *rect); // Improved blitting

	private:
		static SurfaceManager* m_instance;
		SurfaceManager(){};					
		SurfaceManager(const SurfaceManager&){};
		SurfaceManager& operator= (const SurfaceManager&){};
};

#endif


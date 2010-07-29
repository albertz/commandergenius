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

#include "../common/surfacemanager.h"
#include "../common/trap.h"
#include "SDL_image.h"

SurfaceManager* SurfaceManager::m_instance = NULL;

// Returns an m_instance of the class. If no m_instance exists, the method 
// creates a new one.
SurfaceManager* SurfaceManager::GetInstance()
{
	if (m_instance == NULL)
	{
		{
			m_instance = new SurfaceManager();
		}
	}

	return m_instance;
}


// Adds a new surface to the manager.
// If a resource with the same ID already exists, a pointer
// to it is returned. Otherwise, the method returns a pointer
// to the new surface.
SDL_Surface* SurfaceManager::AddSurface(string ID, string fileName)
{
	// Check whether the ID already exists in the map:
	map<string, SDL_Surface*>::iterator itr = m_map.find(ID);
	if (itr != m_map.end())
		itr->second;

	// Load the image:
	SDL_Surface *surface = IMG_Load(fileName.c_str());
	TRAP(surface == NULL, "SurfaceManager::AddSurface() - Could not open " << fileName); 
	SDL_Surface *surface2 = SDL_DisplayFormat(surface);
	SDL_FreeSurface(surface);
	surface = surface2;
	TRAP(surface == NULL, "SurfaceManager::AddSurface() - Could not convert to HW surface " << fileName); 

	// Convert it to the framebuffer's display format:
	//SDL_Surface *converted = SDL_DisplayFormatAlpha(surface);
	//SDL_FreeSurface(surface);

	m_map[ID] = surface;

	return surface;
}


// Adds a new surface to the manager.
// If a resource with the same ID exists, it is overwritten.
// The method returns a pointer to the new surface.
SDL_Surface* SurfaceManager::AddSurface(string ID, SDL_Surface *surface)
{
	// Check whether the ID already exists in the map:
	map<string, SDL_Surface*>::iterator itr = m_map.find(ID);
	if (itr != m_map.end())
		SDL_FreeSurface(itr->second);
	
	TRAP(surface == NULL, "SurfaceManager::AddSurface() - adding an NULL surface"); 

	m_map[ID] = surface;

	return surface;
}


// Gets a surface by its ID.
// If the ID does not exist, NULL is returned.
SDL_Surface *SurfaceManager::GetSurface(string ID)
{
	map<string, SDL_Surface*>::iterator itr = m_map.find(ID);
	if (itr == m_map.end())
		return NULL;

	return itr->second;
}
		

// Releases all the loaded surfaces.
void SurfaceManager::Cleanup()
{
	map<string, SDL_Surface*>::iterator itr;
	for (itr = m_map.begin() ; itr != m_map.end() ; itr++)
		SDL_FreeSurface(itr->second);
	
	m_map.clear();
}


// Blits the source surface onto the destination surface, while
// combining their alpha values. This allows to blit onto a
// transparent surface, which SDL_BlitSurface does not support.
// The user can also specify the source alpha value to create
// transparent text (0.0f = transparent, 1.0 = opaque).
// Bledning equation:
// result(r,g,b) = source(r,g,b) * source(a) + dest(r,g,b) * (1 - source(a))
// TO DO: Alpha should loook better. Review blending equation.
void SurfaceManager::Blit(SDL_Surface* source, SDL_Surface* dest, float srcAlpha, SDL_Rect *rect)
{
	if (source == NULL || dest == NULL || source->format->BitsPerPixel != 32
		|| source->format->BitsPerPixel != 32 || rect->x > dest->w || rect->y > dest->h)
		return;

	// Result, source and destination RGBA values:
	Uint8 rr, rg, rb, ra;
	Uint8 sr, sg, sb, sa;
	Uint8 dr, dg, db, da;

	// Lock both surfaces:
	if(SDL_MUSTLOCK(source)) 
		if(SDL_LockSurface(source) < 0) 
			return;

	if(SDL_MUSTLOCK(dest)) 
		if(SDL_LockSurface(dest) < 0) 
		{
			if SDL_MUSTLOCK(source)
				SDL_UnlockSurface(source);
			return;
		}

	// Make sure we stay in bounds:
	int maxX = (rect->x + source->w >= dest->w) ? dest->w - rect->x : source->w;
	int maxY = (rect->y + source->h >= dest->h) ? dest->h - rect->y : source->h;

	Uint8 *sourcePixel, *destPixel;
	float alpha;
	for (int x = 0 ; x < maxX ; x++)
		for (int y = 0 ; y < maxY ; y++)
		{
			// Get the source and destaination pixels:
			sourcePixel = (Uint8 *) source->pixels + y * source->pitch + x * source->format->BytesPerPixel;
			destPixel = (Uint8 *) dest->pixels + (y + rect->y) * dest->pitch + 
				(x + rect->x) * dest->format->BytesPerPixel;
			
			// Extract the pixels' RGBA components:
			SDL_GetRGBA(*(Uint32*)sourcePixel, source->format, &sr, &sg, &sb, &sa);
			SDL_GetRGBA(*(Uint32*)destPixel, dest->format, &dr, &dg, &db, &da);

			// Blend and write the result back to the destination pixel:
			alpha = sa / 255.0f;	// Convert from 0..255 to 0.0..1.0
			rr = (Uint8) (sr * alpha + dr * (1.0f - alpha));
			rg = (Uint8) (sg * alpha + dg * (1.0f - alpha));
			rb = (Uint8) (sb * alpha + db * (1.0f - alpha));
			ra = (Uint8) (sa * alpha + da * (1.0f - alpha));

			// A = A1 (1 - A2) + A2 A2
			//float A1 = sa / 255.0f;
			//float A2 = da / 255.0f;
			//ra = (Uint8) (A2 * (1.0f - A1) + (A1 * A1));

			
			*(Uint32 *)destPixel = SDL_MapRGBA(source->format, rr, rg, rb, (Uint8) (ra * srcAlpha));
			
		}

	if SDL_MUSTLOCK(source)
		SDL_UnlockSurface(source);
	
	if SDL_MUSTLOCK(dest)
		SDL_UnlockSurface(dest);
}


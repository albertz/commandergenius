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

#ifndef _EFFECTMANAGER_H_
#define _EFFECTMANAGER_H_

// A singleton effect manager class.
// The class holds a list of active effects. The user can add an effect
// to the list.
// Once an effect becomes inactive (or invisible), it is removed from
// the list.

#include "SDL.h"
#include <list>
#include <string>

#include "../common/font.h"
#include "../common/texteffect.h"

using namespace std;

class EffectManager
{
	private:
		list<TextEffect*> m_activeEffects;			// A list of active effects

	public:
		// Add effects:
		void Add(Sint16 x, Sint16 y, string text,	// Adds an effect
			Font *font, SDL_Color color, float initialAlpha,
			float velocity, Uint32 lifeTime, float gravity);
		void AddScoreEffect(SDL_Event *event);		// Adds a score effect
		void AddLevelEffect(int level);				// Adds a new level effect
		void AddPerfectEffect();					// Adds a perfect rectangle effect
		void AddGameOverEffect();					// Adds a game over effect

		bool Update();								// Updates the effects

		void Draw(SDL_Surface *screen);				// Draws the effects
		
		static EffectManager* GetInstance();
		void Init();
		void Cleanup();

	private:
		static EffectManager* m_instance;
		EffectManager(){};					
		EffectManager(const EffectManager&){};
		EffectManager& operator= (const EffectManager&){};

		bool m_initialized;	// Has the manager been initialized?
};

#endif


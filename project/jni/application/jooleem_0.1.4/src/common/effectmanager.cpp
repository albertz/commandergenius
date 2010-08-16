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

#include "../common/effectmanager.h"
#include "../common/fontmanager.h"
#include "../common/trap.h"
#include <sstream>

EffectManager* EffectManager::m_instance = NULL;

// Returns an m_instance of the class. If no m_instance exists, the method 
// creates a new one.
EffectManager* EffectManager::GetInstance()
{
	if (m_instance == NULL)
	{
		{
			m_instance = new EffectManager();
		}
	}

	return m_instance;
}


// Initialize the manager - load fonts.
void EffectManager::Init()
{
	m_initialized = true;
}


// Adds a score effect
void EffectManager::AddScoreEffect(SDL_Event* event)
{
	TRAP(m_initialized == false, "EffectManager::AddScoreEffect() - Manager has not been initialized");

	if (event == NULL)
		return;

	SDL_Color color = {255, 255, 255, 0};

	// Get the score and coordinates:
	Uint32 *score = (Uint32 *) event->user.data1;
	SDL_Rect *rect = (SDL_Rect *) event->user.data2;

	if (score == NULL || rect == NULL)
		return;

	// Create the string:
	ostringstream temp;
	temp << *score;
	
	// Add the effect:
	Add(rect->x + rect->w / 2, 
		rect->y + rect->h / 2,
		temp.str(),
		FontManager::GetInstance()->GetFont("ScoreEffect"),
		color,
		0.8f,
		0.0f,
		1500,
		0.5f);
}


// Adds a new level effect
void EffectManager::AddLevelEffect(int level)
{
	TRAP(m_initialized == false, "EffectManager::AddLevelEffect() - Manager has not been initialized");

	SDL_Color color = {255, 255, 255, 0};

	// Create the string:
	ostringstream temp;
	temp << "Level " << level;
	
	// Add the effect:
	Add(412,
		228,
		temp.str(),
		FontManager::GetInstance()->GetFont("LevelEffect"),
		color,
		0.8f,
		0.0f,
		2000,
		-0.3f);
}


// Adds a game over effect.
void EffectManager::AddGameOverEffect()
{
	TRAP(m_initialized == false, "EffectManager::AddGameOverEffect() - Manager has not been initialized");

	SDL_Color color = {255, 255, 255, 0};
	
	// Add the effect:
	Add(412,
		50,
		"Game Over",
		FontManager::GetInstance()->GetFont("GameOverEffect"),
		color,
		0.8f,
		0.0f,
		4000,
		-0.01f);
}


// Adds a perfect rectangle effect.
void EffectManager::AddPerfectEffect()
{
	TRAP(m_initialized == false, "EffectManager::AddLevelEffect() - Manager has not been initialized");

	SDL_Color color = {255, 255, 255, 0};
	
	// Add the effect:
	Add(412,
		300,
		"Perfect!",
		FontManager::GetInstance()->GetFont("ScoreEffect"),
		color,
		0.8f,
		0.0f,
		1500,
		0.5f);
}

// Adds an effect to the manager.
void EffectManager::Add(Sint16 x, Sint16 y, string text,
			Font *font, SDL_Color color, float initialAlpha,
			float velocity, Uint32 lifeTime, float gravity)
{
	TRAP(m_initialized == false, "EffectManager::AddScoreEffect() - Manager has not been initialized");

	{
		TextEffect *effect = new TextEffect(x, y, text, font, color, initialAlpha, velocity, lifeTime, gravity);

		// Add the effect to the list:
		m_activeEffects.push_back(effect);
	}

}


// Updates the effects.
// Returns true if a redraw is required.
bool EffectManager::Update()
{
	TRAP(m_initialized == false, "EffectManager::AddScoreEffect() - Manager has not been initialized");

	bool redraw = false;
	list<TextEffect *>::iterator itr;

	for (itr = m_activeEffects.begin() ; itr != m_activeEffects.end() ; itr++)
	{
		redraw |= (*itr)->Update();

		// Remove inactive effects from the list.
		if ( (*itr)->IsActive() == false)
		{
			delete (*itr);
			itr = m_activeEffects.erase(itr);
		}
	}

	return redraw;

}

// Draws the effects.
void EffectManager::Draw(SDL_Surface *screen)
{
	TRAP(m_initialized == false, "EffectManager::AddScoreEffect() - Manager has not been initialized");

	list<TextEffect *>::iterator itr;

	for (itr = m_activeEffects.begin() ; itr != m_activeEffects.end() ; itr++)
		(*itr)->Draw(screen);
}

// Clean up - free all the effects.
void EffectManager::Cleanup()
{
	TRAP(m_initialized == false, "EffectManager::AddScoreEffect() - Manager has not been initialized");

	list<TextEffect *>::iterator itr;

	for (itr = m_activeEffects.begin() ; itr != m_activeEffects.end() ; itr++)
		delete (*itr);
}



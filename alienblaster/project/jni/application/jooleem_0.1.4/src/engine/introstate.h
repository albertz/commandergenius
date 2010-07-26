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

#ifndef _INTROSTATE_H_
#define _INTROSTATE_H_

// Intro engine state. Singleton.
// Simulate a game.

#include "../engine/enginestate.h"
#include "../game/selection.h"
#include "../UI/uilabel.h"
#include "SDL.h"

class IntroState: public EngineState
{
	private:
		Selection m_selection;		// Selection for simulated game
		Uint32 m_iLastMove;			// Time of last move
		int m_iMarbleIndex;			// Index of marble within selection (0..3)
		static const int INTRO_DELAY = 750;		// Delay between simulated clicks

	public:
		void Init(Engine* engine);
		void Enter(Engine* engine);
		void Exit(Engine* engine);
		void HandleEvents(Engine* engine);	
		void Update(Engine* engine);	
		void Draw(Engine* engine);
		void Cleanup(Engine* engine);

		static IntroState* GetInstance();

	private:
		static IntroState* m_instance;
		IntroState(){};					
		IntroState(const IntroState&);
		IntroState& operator= (const IntroState&);

		// Handle user events:
		bool HandleUserEvents(Engine *engine, SDL_Event *event);
};


#endif


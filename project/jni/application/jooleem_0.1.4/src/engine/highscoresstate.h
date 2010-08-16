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

#ifndef _HIGHSCORESSTATE_H_
#define _HIGHSCORESSTATE_H_

// About screen engine state. Singleton.

#include "../engine/enginestate.h"
#include "../UI/highscoreswindow.h"

class HighScoresState: public EngineState
{
	private:
		HighScoresWindow *m_highScoresWindow;	// The high scores window

	public:
		void Init(Engine* engine);
		void Enter(Engine* engine);
		void Exit(Engine* engine);
		void HandleEvents(Engine* engine);	
		void Update(Engine* engine);	
		void Draw(Engine* engine);
		void Cleanup(Engine* engine);

		static HighScoresState* GetInstance();

	private:
		static HighScoresState* m_instance;
		HighScoresState(){};					
		HighScoresState(const HighScoresState&);
		HighScoresState& operator= (const HighScoresState&);

		void PopulateLabels(Engine* engine);	// Populates the labels making up the high scores table 
};

#endif


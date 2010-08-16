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

#ifndef _HIGHSCORESWINDOW_H_
#define _HIGHSCORESWINDOW_H_

// The high scores window.

#include "../UI/uiwindow.h"
#include "SDL.h"

class HighScoresWindow: public UIWindow
{
	public:

		enum Controls {	
			// Buttons:
			BTN_CLOSE,		// Close
			BTN_RESTART,	// Restart
			BTN_QUIT,		// Quit

			// Vectors (the names and scores are a vector of labels):
			VEC_INDEX,		// Scores index (1st place, 2nd...)
			VEC_NAMES,		// Player names
			VEC_SCORES,		// Scores

			// Labels:
			LBL_TITLE,		// Window title
		};

		// Constructor / destructor:
		HighScoresWindow(SDL_Surface *background);
		~HighScoresWindow();

	private:
		void InitControls();	// Initializes the window's controls, and adds them to the control collection
};

#endif


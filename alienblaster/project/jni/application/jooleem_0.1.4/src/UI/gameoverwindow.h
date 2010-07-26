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

#ifndef _GAMEOVERWINDOW_H_
#define _GAMEOVERWINDOW_H_

// This window is displayed when the game is over.
// It shows the final score and various game statitics.

#include "../UI/uiwindow.h"
#include "SDL.h"

class GameOverWindow: public UIWindow
{
	public:

		enum Controls {	
			// Buttons:
			BTN_OK,			// OK
			BTN_QUIT,		// Quit
			BTN_RESTART,	// Restart

			// Labels:
			LBL_TITLE,		// Window title
			LBL_CONGRATS,	// Congratulations message
			LBL_SCORE,		// Final score
			LBL_STATS_DESC,	// Statistics description
			LBL_STATS_VAL,	// Statistics values

			// Text box:
			TXT_NAME,		// Enter the player's name
		};

		// Constructor / destructor:
		GameOverWindow(SDL_Surface *background);
		~GameOverWindow();

	private:
		void InitControls();	// Initializes the window's controls, and adds them to the control collection
};

#endif


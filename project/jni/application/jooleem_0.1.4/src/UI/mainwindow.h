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

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

// The main window.

#include "../UI/uiwindow.h"
#include "SDL.h"

class MainWindow: public UIWindow
{
	public:

		enum Controls {	
			// Buttons:
			BTN_START,		// Start / restart
			BTN_PAUSE,		// Pause / resume
			BTN_QUIT,		// Quit
			BTN_HINT,		// Hint
			BTN_MUTE,		// Mute
			BTN_ABOUT,		// About
			BTN_HISCORES,	// High scores

			// Labels:
			LBL_SCORE,		// Game score
			LBL_LEVEL,		// Game level
			//LBL_TIMEBAR,	// The time bar

			// Windows:
			WND_ABOUT,		// About window
			WND_HISCORES,	// High scores window

			// Progress bar:
			PRG_TIMEBAR,	// Time bar
		};

		// Constructor / destructor:
		MainWindow(SDL_Surface *background);
		~MainWindow();

	private:
		void InitControls();	// Initializes the window's controls, and adds them to the control collection
};

#endif


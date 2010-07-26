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

#ifndef _QUITWINDOW_H_
#define _QUITWINDOW_H_

// The quit dialog window.

#include "../UI/uiwindow.h"
#include "SDL.h"

class QuitWindow: public UIWindow
{
	public:

		enum Controls {	
			// Buttons:
			BTN_QUIT,		// Close
			BTN_PLAY,		// Play

			// Labels:
			LBL_TITLE,		// Window title
			LBL_TEXT,		// Dialog text
		};

		// Constructor / destructor:
		QuitWindow(SDL_Surface *background);
		~QuitWindow();

	private:
		void InitControls();	// Initializes the window's controls, and adds them to the control collection
};


#endif


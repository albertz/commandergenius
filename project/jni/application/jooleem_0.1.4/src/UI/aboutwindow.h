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

#ifndef _ABOUTWINDOW_H_
#define _ABOUTWINDOW_H_

// The about window.

#include "../UI/uiwindow.h"
#include "SDL.h"

class AboutWindow: public UIWindow
{
	public:

		enum Controls {	
			// Buttons:
			BTN_LINK,		// Website link
			BTN_CLOSE,		// Close
			
			// Labels:
			LBL_TITLE,		// Window title
			LBL_LOGO,		// Logo
			LBL_APP,		// Application name and version
			LBL_CREDITS,	// Game credits
			LBL_LICENSE,	// License info
		};

		// Constructor / destructor:
		AboutWindow(SDL_Surface *background);
		~AboutWindow();

	private:
		void InitControls();	// Initializes the window's controls, and adds them to the control collection
};

#endif


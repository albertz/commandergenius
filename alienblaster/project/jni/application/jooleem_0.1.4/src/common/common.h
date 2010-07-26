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

#ifndef _COMMON_H_
#define _COMMON_H_

#include "SDL.h"

#include <string>
#include <ctime>
using namespace std;


// Application name, version and website:
static const string APP_NAME = "Jooleem";
static const string APP_VERSION = "0.1.4 (20050925)";
static const string APP_URL = "http://jooleem.sourceforge.net";


// Window:
static const string CAPTION = "Jooleem";						// Window caption
static const string ICON = "data/interface/icon.png";		// Window icon


// Screen dimensions:
static const int SCREEN_WIDTH = 640;
static const int SCREEN_HEIGHT = 480;

// Takes a screenshot.
// The file is saved as a Windows bitmap.
inline void ScreenShot(char* fileName = NULL)
{
	// If no file name is supplied, create one from the current time:
	if (!fileName)
	{
		char buf[128];
		sprintf(buf, "data/screenshots/%ld.bmp", time(NULL));
		SDL_SaveBMP(SDL_GetVideoSurface(), buf);
	}
	else
		SDL_SaveBMP(SDL_GetVideoSurface(), fileName);
}

#endif


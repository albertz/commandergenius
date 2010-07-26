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

/*
	Jooleem is a simple yet extremely addictive puzzle game. The player is
	presented with a 10x10 board of colored marbles, and must click on four
	marbles of the same color that form a rectangle. Time is constantly 
	running out, but forming rectangles earns the player more time, as well
	as points. The larger the rectangle, the more time and points are won.

	Inspired by Nick Kouvaris' ZNAX:
	http://www.freestuff.gr/lightforce/znax.html
*/

// Platform specific headers, to display a dialog box. See MsgBox().
#ifdef _WIN32
#include <windows.h>
#elif defined( __APPLE__ ) && defined( __MACH__ )
#include <Carbon/Carbon.h>
#endif

#include <iostream>
#include "engine/engine.h"
#include "SDL_main.h"


using namespace std;

void MsgBox(string message);	// Show a messagebox (cross platform)

int main(int argc, char* argv[])
{
	Engine engine;
	engine.Run();

	return 0;
}


// Shows an exception in an OS message box (cross platform).
// FIX: the message doesn't get here properly
void MsgBox(string message)
{

	// Prepare the error message:
	char buffer[4096];

	sprintf(buffer,
			"The game has encountered an unrecoverable error. Please send\n"
			"the error message below to braudo@users.sourceforge.net.\n\n%s",
			message.c_str());
	/*
	sprintf(buffer,
			"The game has encountered an unrecoverable error\n"
			"and will shut down. Downloading and installing\n"
			"the game again might solve the problem.\n\n"
			"If the problem persists, please contact\n"
			"braudo@users.sourceforge.net"
			);
			*/

// Windows:
#ifdef WIN32

	MessageBox(NULL, buffer, "Critical Error", MB_OK | MB_ICONERROR);


// Mac OS X:
#elif defined( __APPLE__ ) && defined( __MACH__ )
	// Reference:
	// http://developer.apple.com/documentation/Carbon/Conceptual/HandlingWindowsControls/hitb-wind_cont_tasks/chapter_3_section_4.html
  
	DialogRef theItem;
	DialogItemIndex itemIndex;
	
	// Create a string ref of the message:
	CFStringRef msgStrRef = CFStringCreateWithCString(NULL, message.c_str(), kCFStringEncodingASCII);

	// Create the dialog:
	CreateStandardAlert(kAlertStopAlert, CFSTR("Critical Error"), msgStrRef, NULL, &theItem);
	
	// Show it:
	RunStandardAlert(theItem, NULL, &itemIndex); 

	// Release the string:
	CFRelease(msgStrRef);

// Default to standard error:
#else
	// Note that only the basic error message is displayed here.
	cerr << message << endl;

#endif

}


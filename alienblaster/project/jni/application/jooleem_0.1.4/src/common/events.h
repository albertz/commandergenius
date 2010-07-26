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

#ifndef _EVENTS_H_
#define _EVENTS_H_

// User event codes.

#include "SDL_events.h"

enum Events 
	{ 
		EVENT_HINT = SDL_NUMEVENTS + 1,	// This ensures there are no collisions with the standard SDL events
		EVENT_PAUSE,				// Pause the game
		EVENT_RESUME,				// Resume the game
		EVENT_START,				// Start a new game
		EVENT_RESTART,				// Restart the game
		EVENT_SELECTION_ADD,		// A marble was added to the selection
		EVENT_SELECTION_CANCEL,		// The user cancelled the current selection
		EVENT_SELECTION_INVALID,	// The user selected an invalid marble
		EVENT_RECT_COMPLETE,		// Rectangle was completed
		EVENT_PERFECT,				// A perfect rectangle was complete
		EVENT_NEW_LEVEL,			// Level over, advanced to next level
		EVENT_TIME_LOW,				// Time is running low
		EVENT_TIME_OK,				// Time increased, and is over the low threshold
		EVENT_GAME_OVER,			// Game over
		EVENT_MUTE,					// Mute
		EVENT_UNMUTE,				// Unmute
		EVENT_ABOUT,				// Show about window
		EVENT_HIGH_SCORES,			// Show high scores window
		EVENT_CLOSE,				// Close the window 
		EVENT_REDRAW,				// Force a screen redraw
		EVENT_TRACK_OVER,			// A music track has ended
		EVENT_WEBSITE,				// Visit the website
	};

// Pushes a user event on to the SDL queue.
inline void PushUserEvent(Events eventCode, void *data1 = NULL, void *data2 = NULL)
{
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = eventCode;
	event.user.data1 = data1;
	event.user.data2 = data2;

	SDL_PushEvent(&event);
}

#endif


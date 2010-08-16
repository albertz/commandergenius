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

#ifndef _UIPROGRESS_H_
#define _UIPROGRESS_H_

// UI progress bar control.

#include "../UI/uicontrol.h"
#include "SDL.h"

class UIProgressBar: public UIControl
{
	private:
		// Surfaces:
		SDL_Surface *m_BGSurface;		// Background surface
		SDL_Surface *m_normalSurface;	// Normal foreground surface
		SDL_Surface *m_flashingSurface;	// Flashing foreground surface
		SDL_Surface *m_disabledSurface;	// Disabled foreground surface

		SDL_Rect m_clipRect;			// Clipping rectangle for the foreground surface

		float m_progress;				// Bar's progress, between 0.0f and 1.0f

		// Flash:
		bool m_flashingMode;			// Are we in flashing mode?
		bool m_isFlashing;				// Are we currently flashing?
		Uint32 m_flashStart;			// Time flashing has started

		// Button flash interval:
		static const int FLASH_INTERVAL = 500;

	public:
		UIProgressBar();
		~UIProgressBar(){};

		void Set(Uint16 x, Uint16 y, SDL_Surface* BGSurface,		// Sets location and surfaces 
			 SDL_Surface *normalSurface, SDL_Surface *flashingSurface, SDL_Surface *disabledSurface); 

		// Set and get the bar's progress:
		void SetProgress(float progress);
		float GetProgress(){return m_progress;}

		void Flash(bool flag);			// Starts or stops the bar's flashing

		bool Update();					// Updates the progress bar

		void Draw(SDL_Surface *screen);	// Draws the progress bar

		// Mouse and keyboard events are irrelevant to a progress bar, but required for polymorphism:
		bool MouseDown(Uint16 x, Uint16 y){return false;}		// Process mouse up events
		bool MouseUp(Uint16 x, Uint16 y){return false;}			// Process mouse up events
		bool Click(Uint16 x, Uint16 y){return false;}			// Process user clicks
		bool Hover(Uint16 x, Uint16 y){return false;}			// Process user mouse hover
		bool KeyDown(SDL_KeyboardEvent *event){return false;}	// Process key down events
};

#endif


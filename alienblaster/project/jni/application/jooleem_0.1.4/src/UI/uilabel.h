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

#ifndef _UILABEL_H_
#define _UILABEL_H_

// UI label control
// May contain text and/or a background image.
// The text is always centered in both dimensions.

#include "../UI/uicontrol.h"
#include "../common/font.h"
#include <string>
#include "SDL.h"

class UILabel: public UIControl
{
	private:
		SDL_Surface *m_BGSurface;		// Background surface
		SDL_Surface *m_TextSurface;		// Text surface
		SDL_Surface *m_CompSurface;		// The text composited onto the the background

		Font *m_font;					// Font used to render the label's text
		std::string m_Text;				// Label's text
		SDL_Color m_textColor;			// Label's Text color
		float m_alpha;					// Text's alpha value (0.0-1.0)

		SDL_Rect m_clipRect;			// Clipping rectangle for the label's surface

	public:
		UILabel();
		~UILabel();

		void Set(Uint16 x, Uint16 y, SDL_Surface* background,	// Set location, background, font & color
			 Font *font, SDL_Color textColor, float alpha = 1.0f); 

		void SetEnabled(bool enabled){};

		std::string GetText() const;				// Get the label's text
		void SetText(HAlign hAlign, const char *format, ...);	// Set the label's text (printf-like formatting)
		void SetText(HAlign hAlign, const std::string& text);	// Set the label's text

		void SetClipping(Sint16 w, Sint16 h);		// Sets the clipping rectangle

		void SetBackground(SDL_Surface *background);// Sets the background surface

		void ShowTimed(Uint32 time);				// Show the label for a set amount of time

		bool Update(){return false;}				// Update

		void Draw(SDL_Surface *screen);				// Draw the label

		// Mouse and keyboard events are irrelevant to a label, but required for polymorphism:
		bool MouseDown(Uint16 x, Uint16 y){return false;}		// Process mouse up events
		bool MouseUp(Uint16 x, Uint16 y){return false;}			// Process mouse up events
		bool Click(Uint16 x, Uint16 y){return false;}			// Process user clicks
		bool Hover(Uint16 x, Uint16 y){return false;}			// Process user mouse hover
		bool KeyDown(SDL_KeyboardEvent *event){return false;}

	private:
		SDL_TimerID m_timerID;									// Hide timer ID
		static Uint32 HideLabel(Uint32 interval, void* param);	// Hide label timer callback func.	
};

#endif


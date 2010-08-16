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

#ifndef _UITEXTBOX_H_
#define _UITEXTBOX_H_

// UI text box control

#include "../UI/uicontrol.h"
#include "../common/font.h"
#include <string>
#include <vector>
#include "SDL.h"

using namespace std;

class UITextBox: public UIControl
{
	private:
		SDL_Surface *m_BGSurface;		// Background surface
		SDL_Surface *m_textSurface;		// Text surface
		SDL_Surface *m_cursorSurface;	// Cursor surface

		string m_text;					// The box's text
		Font *m_font;					// Font used to render the text
		SDL_Color m_textColor;			// Text color

		Uint16 m_cursorIndex;			// Cursor position - as the index of the
										// character in the string. The cursor
										// is drawn BEFORE the character with 
										// the same index.
										// Use n+1 to indicate the position after
										// the last character.

		vector<Uint16> m_charOffset;	// A vector of character advance offset.
										// E.g. For the string "Hello", the value
										// of the second element will be the distance,
										// in pixels, from the begining of the rendered
										// text until the end of the character 'e'

		Uint16 m_textX, m_textY;		// Location of text area within the control

		bool m_blinking;				// Current blinking status
		static const int CURSOR_BLINK = 500;	// Cursor blink rate, in ms


	public:
		UITextBox();
		~UITextBox();

		void Set(Uint16 x, Uint16 y,				// Box location
				Uint16 textX, Uint16 textY,			// Text location relative to the box
				SDL_Surface* background, SDL_Surface *cursorSurface,	// Background and cursor surfaces
				Font *font, SDL_Color textColor);	// Font and text color 

		void SetEnabled(bool enabled){};

		void Clear();								// Clears the text

		string GetText() const;						// Gets the box's text
		void SetText(string text);					// Sets the box's text

		void SetBackground(SDL_Surface *background);// Sets the background surface

		bool Update();								// Updates the text box

		void Draw(SDL_Surface *screen);				// Draws the text box

		// Mouse and keyboard events:
		bool MouseDown(Uint16 x, Uint16 y);						// Processes mouse down events
		bool MouseUp(Uint16 x, Uint16 y){return false;}			// Processes mouse up events
		bool Click(Uint16 x, Uint16 y){return false;}			// Processes user clicks
		bool Hover(Uint16 x, Uint16 y){return false;}			// Processes user mouse hover
		bool KeyDown(SDL_KeyboardEvent *event);					// Processes keyboard clicks

	private:
		void Refresh();			// Refreshes the text box when changed
		void MeasureOffsets();	// Measure the characters' offset
};

#endif


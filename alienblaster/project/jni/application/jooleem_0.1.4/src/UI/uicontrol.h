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

#ifndef _UICONTROL_H_
#define _UICONTROL_H_

#include "SDL.h"

// Abstract base class for user interface controls (buttons, labels, text boxes)
class UIControl
{
	protected:
		Uint16 m_x, m_y;	// Location
		Uint16 m_w, m_h;	// Dimensions
		bool m_visible;		// Visibility flag
		bool m_enabled;		// Enabled flag

	public:
		UIControl():m_x(0), m_y(0), m_w(0), m_h(0), m_visible(true), m_enabled(true){};
		virtual ~UIControl(){};

		// Set the control's location:
		void SetLocation(Uint16 x, Uint16 y){m_x = x; m_y = y;}

		// Align the control, vertically and horizontally:
		enum HAlign { HALN_LEFT, HALN_CENTER, HALN_RIGHT};
		enum VAlign { VALN_TOP, VALN_MIDDLE, VALN_BOTTOM};
		void Align(HAlign hAlign, VAlign vAlign, Uint16 x, Uint16 y);
		
		// Visibility:
		void SetVisible(bool visible){m_visible = visible;}
		bool GetVisible(){return m_visible;}

		// Enable / disable:
		virtual void SetEnabled(bool enabled){m_enabled = enabled;}
		bool GetEnabled(){return m_enabled;}
		
		// Accessors:
		Uint16 GetX(){return m_x;}
		Uint16 GetY(){return m_y;}
		Uint16 GetWidth(){return m_w;}
		Uint16 GetHeight(){return m_h;}

		// Mouse events:
		virtual bool MouseDown(Uint16 x, Uint16 y) = 0;		// Process mouse up events
		virtual bool MouseUp(Uint16 x, Uint16 y) = 0;		// Process mouse up events
		virtual bool Click(Uint16 x, Uint16 y) = 0;			// Process user clicks
		virtual bool Hover(Uint16 x, Uint16 y) = 0;			// Process user mouse hover

		// Kyboard events:
		virtual bool KeyDown(SDL_KeyboardEvent *event) = 0;

		// Update:
		virtual bool Update() = 0;

		// Draw the control:
		virtual void Draw(SDL_Surface *screen) = 0;
};


#endif


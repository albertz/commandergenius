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

#ifndef _UIWINDOW_H_
#define _UIWINDOW_H_

// A base class for windows.
// The window is composed of a background and a collection of
// controls - labels, buttons or other windows.
// The class supports vectors of controls.

#include "../UI/uicontrol.h"
#include "../UI/uibutton.h"
#include "../UI/uilabel.h"
#include "../UI/uiprogress.h"

#include <map>
#include <vector>
#include "SDL.h"

using namespace std;

class UIWindow: public UIControl
{
	protected:
		SDL_Surface *m_background;			// Window background
		map<int, UIControl *> m_controls;	// Window controls
		map<int, vector<UIControl *> > m_controlsVectors;	// Window controls vectors
		
	public:
		UIWindow(SDL_Surface *background);
		virtual ~UIWindow();

		// Enable / Disbable the windows' controls:
		void SetEnabled(bool enabled);

		// Add and get a control:
		void AddControl(int ID, UIControl *control);
		void AddControlVector(int ID, vector<UIControl *> vec);
		UIControl *GetControl(int ID);
		vector<UIControl *> GetControlVector(int ID);

		// Accessors:
		UIButton *GetButton(int ID);
		UILabel *GetLabel(int ID);
		UIProgressBar *GetProgressBar(int ID);
		SDL_Surface *GetBackground(){return m_background;}


		// Mouse events:
		bool MouseDown(Uint16 x, Uint16 y);		// Process mouse up events
		bool MouseUp(Uint16 x, Uint16 y);		// Process mouse up events
		bool Click(Uint16 x, Uint16 y);			// Process user clicks
		bool Hover(Uint16 x, Uint16 y);			// Process user mouse hover

		// Key down:
		bool KeyDown(SDL_KeyboardEvent *event);

		// Update the window:
		bool Update();

		// Draw the window:
		void Draw(SDL_Surface *screen);
		void DrawBackground(SDL_Surface *screen);
		void DrawControls(SDL_Surface *screen);
};

#endif


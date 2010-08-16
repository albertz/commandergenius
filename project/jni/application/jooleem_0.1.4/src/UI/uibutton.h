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

#ifndef _UIBUTTON_H_
#define _UIBUTTON_H_

// User interface button class.
// A button has several modes (normal, hover etc.), and one
// or more user defined states.
// Once a button is clicked, it pushes an event defined by
// its current state.
// The class also supports tooltips.

#include "../UI/uicontrol.h"
#include "../common/font.h"

#include "SDL.h"

#include <string>
#include <map>

using namespace std;

// Tooltip related:
static const SDL_Color TOOLTIP_FG = {0, 0, 0, 0};		// Foreground color (text and border)
static const SDL_Color TOOLTIP_BG = {250, 250, 150, 0};	// Background color
static const Uint32 TOOLTIP_DELAY = 750;				// Time, in ms, until tooltip is shown

class UIButton: public UIControl
{
	public:
		// Button modes:
		enum ButtonMode {	MODE_NORMAL,
							MODE_DISABLED,
							MODE_HOVER,
							MODE_CLICKED,
							MODE_FLASH,
							MODE_MASK,
							MODE_MAX};

		// The button's state:
		typedef struct
		{
			SDL_Event event;				// The event to raise once clicked
			SDL_Surface *textSurface;		// Button text's surface
			SDL_Surface *toolTipSurface;	// Button's tooltip surface
		} State;

	private:
		string m_clickSoundID;				// Click sound ID

		// Modes:
		SDL_Surface *m_surfaces[MODE_MAX];	// A surface for every one of the modes
		ButtonMode m_currentMode;		// Current mode
		ButtonMode m_prevMode;			// Previous mode (to return to from hover)
		
		// Flash:
		bool m_isFlashing;				// Are we currently flashing?
		Uint32 m_flashStart;			// Time flashing has started
		Uint32 m_flashInterval;			// Flash interval

		// States:
		map<int, State> m_states;		// Map states to their names
		State m_currentState;			// Current state

		// Mask:
		Uint8* m_mask;					// Masks the button's clickable area
		Uint16 m_maskW, m_maskH;		// Mask's dimensions

		// Tooltip:
		bool m_showTooltip;				// Should the tooltip be shown?
		SDL_TimerID m_timerID;			// Tooltip timer ID
		typedef struct	// Passed to the timer callback fucntion
		{
			ButtonMode *currentMode;	// Current button mode
			bool *showTooltip;			// Should the tooltip be shown?
		} TooltipParam;
		TooltipParam m_tooltipParam;
	
	public:
		UIButton();
		~UIButton();

		// Set all of the button's properties:
		void Set(Uint16 x, Uint16 y, string clickSoundID, SDL_Surface* normal,
			SDL_Surface* disabled = NULL, SDL_Surface* hover = NULL,
			SDL_Surface* clicked = NULL, SDL_Surface* flash = NULL, 
			SDL_Surface* mask = NULL);

		bool IsInButton(Uint16 x, Uint16 y);	// Are the coordinates inside the button (use mask)?
		bool IsInRect(Uint16 x, Uint16 y);		// Are the coordinates inside the button?

		// Handle mouse down and up events:
		bool MouseDown(Uint16 x, Uint16 y);
		bool MouseUp(Uint16 x, Uint16 y);

		// Click the button:
		bool Click();							// Click the button
		bool Click(Uint16 x, Uint16 y);			// Click the button if the coords are valid

		// Hover over the button:
		bool Hover();
		bool Hover(Uint16 x, Uint16 y);

		// Disable the button:
		void Disable();
		void SetEnabled(bool enabled);			// Override the base class' method

		// Set the button to normal (enabled) mode:
		void Normal();
        
		// Set the button to flash mode:
		void Flash(Uint32 interval);

		// Add a state to the button:
		void AddState(int eventCode, Font *font, string text, SDL_Color color,
			Font *tooltipFont = NULL, string tooltipText = "");
		void AddState(SDL_Event event, Font *font, string text, SDL_Color color,
			Font *tooltipFont = NULL, string tooltipText = "");
		void AddState(int eventCode, SDL_Surface *surface, Font *tooltipFont = NULL, string tooltipText = "");
		void AddState(SDL_Event event, SDL_Surface *surface, Font *tooltipFont = NULL, string tooltipText = "");

		// Set the button's state:
		void SetState(int state);			

		// Set/Get the button's mode:
		void SetMode(ButtonMode mode);			
		ButtonMode GetMode(){return m_currentMode;}
		
		// Update the button:
		bool Update();

		// Draw the button:
		void Draw(SDL_Surface *screen);

		// Key down, unimplemented:
		virtual bool KeyDown(SDL_KeyboardEvent *event){return false;}

		// Button flash interval:
		static const int FLASH_INTERVAL = 500;

	protected:
		void MakeMask(SDL_Surface* maskSurface);		// Creates a mask from a surface
		SDL_Surface* CreateTooltip(Font *tooltipFont, string tooltipText);	// Creates a tooltip
		static Uint32 TriggerTooltip(Uint32 interval, void* param);	// Trigger tooltip timer callback func.
};

#endif


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

#include "../UI/uibutton.h"
#include "../common/trap.h"
#include "../common/events.h"
#include "../common/soundmanager.h"
#include "SDL_image.h"
#include <iostream>

using namespace std;


// Constructor - initialize the data members:
UIButton::UIButton()
{
		m_maskW = m_maskH = 0;
		m_clickSoundID = "";
		for (int i = 0 ; i < MODE_MAX ; ++i)
			m_surfaces[i] = NULL;
		m_currentMode = MODE_DISABLED;
		m_enabled = true;
		m_visible = true;
		m_mask = NULL;
		m_showTooltip = false;
		m_isFlashing = false;

		// Populate the tooltip callback timer parameter struct:
		m_tooltipParam.currentMode = &m_currentMode;
		m_tooltipParam.showTooltip = &m_showTooltip;
}


// Destructor - Free the tooltip surfaces memory for all the states:
UIButton::~UIButton()
{
	SDL_RemoveTimer(m_timerID);

	map<int, State>::iterator itr;
	for (itr = m_states.begin() ; itr != m_states.end() ; itr++)
	{
		if ((itr->second).toolTipSurface != NULL)
			SDL_FreeSurface((itr->second).toolTipSurface);
	}

	delete []m_mask;
}



// Set the button's location, click sounds, surfaces and mask:
void UIButton::Set(Uint16 x, Uint16 y, string clickSoundID, SDL_Surface* normal,
			SDL_Surface* disabled, SDL_Surface* hover, SDL_Surface* clicked,
			SDL_Surface* flash, SDL_Surface* mask)
{
	SetLocation(x, y);

	m_clickSoundID = clickSoundID;

	m_surfaces[MODE_NORMAL] = normal;
	m_surfaces[MODE_DISABLED] = disabled;
	m_surfaces[MODE_HOVER] = hover;
	m_surfaces[MODE_CLICKED] = clicked;
	m_surfaces[MODE_FLASH] = flash;

	if (mask != NULL)
	{
		m_w = (Sint16) mask->w;
		m_h = (Sint16) mask->h;
	}
	else
	{
		m_w = (Sint16) normal->w;
		m_h = (Sint16) normal->h;
	}

	MakeMask(mask);
}


// Handle a mouse down event:
bool UIButton::MouseDown(Uint16 x, Uint16 y)
{
	if (m_states.size() == 0 || m_visible == false || m_currentMode == MODE_DISABLED ||
		m_currentMode == MODE_CLICKED) 
		return false;

	if (IsInButton(x, y))
	{
		m_currentMode = MODE_CLICKED;
		return true;
	}
	else
		return false;
}


// Handle a mouse down event:
bool UIButton::MouseUp(Uint16 x, Uint16 y)
{
	if (m_states.size() == 0 || m_visible == false || m_currentMode == MODE_DISABLED 
		|| m_currentMode != MODE_CLICKED) 
		return false;

	if (IsInButton(x, y))
		return Click();
	else
	{
		m_currentMode = MODE_NORMAL;
		return true;
	}
}

// Are the coordinates inside the button (use mask)?
bool UIButton::IsInButton(Uint16 x, Uint16 y)
{
	Sint32 X = x - m_x;
	Sint32 Y = y - m_y;

	// If no mask is defined, do a simple boundry check:
	if (m_mask == NULL)
		return (X > 0 && Y > 0 && X <= m_surfaces[m_currentMode]->w && Y <= m_surfaces[m_currentMode]->h);

	// Otherwise, check the mask:
	if (X < 0 || Y < 0 || X >= m_maskW || Y >= m_maskH)
		return false;

	return ((m_mask[Y * m_maskW + X] == 0)? false : true);
}


// Are the coordinates inside the button?
bool UIButton::IsInRect(Uint16 x, Uint16 y)
{
	return (m_x <= x && m_x + m_w >= x && m_y <= y && m_w + m_h >= y);
}


// An unconditional click:
bool UIButton::Click()
{
	if (m_states.size() == 0 || m_visible == false || m_currentMode == MODE_DISABLED)
		return false;

	// Play the sound:
	if (m_clickSoundID != "")
		SoundManager::GetInstance()->PlaySound(m_clickSoundID);

	// Push the event into the even queue:
	SDL_PushEvent(&m_currentState.event);

	// Switch to clicked mode:
	m_currentMode = MODE_NORMAL;

	return true;
}

// Clicks the button if the coords are valid:
bool UIButton::Click(Uint16 x, Uint16 y)
{
	if (IsInButton(x, y)) 
		return Click();
	else
		return false;
}


// Switch to hover mode.
bool UIButton::Hover()
{	
	m_currentMode = MODE_HOVER;
	return true;
}


// Switch to hover mode, if the cursor is over the button.
bool UIButton::Hover(Uint16 x, Uint16 y)
{
	if (m_states.size() == 0 || m_visible == false || m_currentMode == MODE_DISABLED 
		|| m_currentMode == MODE_CLICKED || m_currentMode == MODE_FLASH)
		return false;

	if (IsInButton(x, y))
	{
		if (m_currentMode != MODE_HOVER)	// We just started hovering
		{
			m_currentMode = MODE_HOVER;

			// Set the tooltip callback timer:
			m_timerID = SDL_AddTimer(TOOLTIP_DELAY, TriggerTooltip, &m_tooltipParam);

			return true;
		}
	}
	else if (m_currentMode == MODE_HOVER)	// Restore to normal when no longer over button
	{
		m_currentMode = MODE_NORMAL;
		m_showTooltip = false;
		SDL_RemoveTimer(m_timerID);
		return true;
	}

	return false;
}


// Switch to disabled mode. 
// Clicks on the button will be ignored.
void UIButton::Disable()
{
	m_enabled = false;
	m_currentMode = MODE_DISABLED;
	m_showTooltip = false;
}


// Sets the button's enabled / disabled state.
// Overrides the base class' method, to make it is compatible
// with the button states.
void UIButton::SetEnabled(bool enabled)
{
	m_enabled = enabled;
	m_showTooltip = false;

	if (enabled)
		m_currentMode = MODE_NORMAL;
	else
		m_currentMode = MODE_DISABLED;
}


// Switch to flash mode.
// Set the button to flash every a_iInterval ms.
void UIButton::Flash(Uint32 interval)
{
	m_flashStart = SDL_GetTicks();
	m_flashInterval = interval;
	m_isFlashing = true;
	m_currentMode = MODE_FLASH;
}


// Switch to normal (enabled) mode.
void UIButton::Normal()
{
	m_currentMode = MODE_NORMAL;
	m_showTooltip = false;
}

 
// Adds a state - User event / Text:
void UIButton::AddState(int eventCode, Font *font, string text, SDL_Color color,
						Font *tooltipFont, string tooltipText)
{
	SDL_Event event;

	event.type = SDL_USEREVENT;
	event.user.code = eventCode;

	AddState(event, font, text, color, tooltipFont, tooltipText);
}


// Adds a state - SDL event / Text.
void UIButton::AddState(SDL_Event event, Font *font, string text, SDL_Color color, 
						Font *tooltipFont, string tooltipText)
{
	State state;
	
	// Render the text surface:
	if (font != NULL && text != "")
		state.textSurface = font->RenderPlain(Font::ALN_LEFT, color, text);

	// Create the tooltip:
	if (tooltipFont != NULL && tooltipText != "")
		state.toolTipSurface = CreateTooltip(tooltipFont, tooltipText);
	else
		state.toolTipSurface = NULL;

	// Set the event:
	state.event = event;

	// Add the state to the map. The name is the event type. For user events, 
	// the name is the event code.
	if (event.type == SDL_USEREVENT)
		m_states.insert(pair<int, State>(event.user.code, state)); 
	else
		m_states.insert(pair<int, State>(event.type, state)); 
	
	m_currentState = state;
}



// Adds a state - User event / Image:
void UIButton::AddState(int eventCode, SDL_Surface *surface, Font *tooltipFont, string tooltipText)
{
	SDL_Event event;

	event.type = SDL_USEREVENT;
	event.user.code = eventCode;

	AddState(event, surface, tooltipFont, tooltipText);
}


// Adds a state - SDL event / Image:
void UIButton::AddState(SDL_Event event, SDL_Surface *surface, Font *tooltipFont, string tooltipText)
{
	State state;
	
	state.textSurface = surface;
	
	// Set the event:
	state.event = event;

	// Create the tooltip:
	if (tooltipFont != NULL && tooltipText != "")
		state.toolTipSurface = CreateTooltip(tooltipFont, tooltipText);
	else
		state.toolTipSurface = NULL;

	// Add the state to the map. The name is the event type. For user events, 
	// the name is the event code.
	if (event.type == SDL_USEREVENT)
		m_states.insert(pair<int, State>(event.user.code, state)); 
	else
		m_states.insert(pair<int, State>(event.type, state)); 
	
	m_currentState = state;
}


// Sets the button's state.
void UIButton::SetState(int state)
{
	map<int, State>::iterator itr = m_states.find(state);
	if (itr != m_states.end())
		m_currentState = itr->second;
}


// Sets the button's mode:
void UIButton::SetMode(ButtonMode mode)
{
	m_currentMode = mode;
}



// Updates the button.
// Needed for flashing buttons.
// Return true if a redraw is needed.
bool UIButton::Update()
{
	if (m_enabled != true || m_currentMode != MODE_FLASH)
		return false;

	bool redraw = false;

	if	(((SDL_GetTicks() - m_flashStart) / m_flashInterval) % 2)	// Flashing
	{
		if (!m_isFlashing) // ... but was not flashing before
			redraw = true;
		m_isFlashing = true;
	}
	else // Not flashing
	{
		if (m_isFlashing) // ... but was flashing before
			redraw = true;

		m_isFlashing = false;
	}

	return redraw;
}

// Draws the button:
void UIButton::Draw(SDL_Surface *screen)
{	
	if (!m_visible)
		return;

	// Draw button:
	SDL_Rect rect = {m_x, m_y, 0, 0};
	
	// Draw the basic surface:
	if (m_currentMode == MODE_FLASH && m_isFlashing)
		SDL_BlitSurface(m_surfaces[MODE_NORMAL], NULL, screen, &rect);
	else	// Not flashing or between flashes
		SDL_BlitSurface(m_surfaces[m_currentMode], NULL, screen, &rect);

	// Draw text, centered horizontally and vertically:
	rect.x =	(Sint16) (m_x + (m_surfaces[m_currentMode]->w - m_currentState.textSurface->w) / 2);
	rect.y =	(Sint16) (m_y + (m_surfaces[m_currentMode]->h - m_currentState.textSurface->h) / 2);
	SDL_BlitSurface(m_currentState.textSurface, NULL, screen, &rect);

	// Draws the tooltip:
	if (m_showTooltip && m_currentState.toolTipSurface != NULL && m_currentMode != MODE_DISABLED)
	{
		rect.x = (Sint16)(m_x + m_w / 2 - m_currentState.toolTipSurface->w / 2);
		rect.y = (Sint16)(m_y + m_h);

		SDL_BlitSurface(m_currentState.toolTipSurface, NULL, screen, &rect);
	}
}


// Creates a mask from a surface.
// http://sdldoc.csn.ul.ie/guidevideo.php#AEN112
void UIButton::MakeMask(SDL_Surface* maskSurface)
{
	if (maskSurface == NULL)
		return;

	m_maskW = (Uint16) maskSurface->w;
	m_maskH = (Uint16) maskSurface->h;

	{
		m_mask = new Uint8[m_maskW * m_maskH];
	}

	// Lock the surface:
	if(SDL_MUSTLOCK(maskSurface)) {
		if(SDL_LockSurface(maskSurface) < 0) 
			return;
	}
		
	// Copy each pixel from the alpha or red channels to the mask:
	Uint32 pixel;	// Value of the current pixel
	Uint8 *pPixel;	// Pointer to the current pixel
	Uint8 g,b;
	for (Uint16 i = 0 ; i < m_maskW ; ++i)
		for (Uint16 j = 0 ; j < m_maskH ; ++j)
		{
			// 32 bit images - the mask is based on the alpha channel:
			if (maskSurface->format->BytesPerPixel == 4)
			{
				pPixel = (Uint8 *) maskSurface->pixels + j * maskSurface->pitch +
					i * maskSurface->format->BytesPerPixel;

				SDL_PixelFormat *fmt = maskSurface->format;
				pixel = *(Uint32 *)pPixel & fmt->Amask;		// Isolate alpha component
				pixel = pixel >> fmt->Ashift;				// Shift it down to 8-bit
				pixel = pixel << fmt->Aloss;				// Expand to a full 8-bit number 
				m_mask[j * m_maskW + i] = (Uint8) pixel;
			}

			// 24 bit images - the mask is based on the red channel:
			else if (maskSurface->format->BytesPerPixel == 3)
			{
				pPixel = (Uint8 *) maskSurface->pixels + j * maskSurface->pitch +
					i * maskSurface->format->BytesPerPixel;	

				if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
					pixel =  pPixel[0] << 16 | pPixel[1] << 8 | pPixel[2];
				else
					pixel = pPixel[0] | pPixel[1] << 8 | pPixel[2] << 16;

				SDL_GetRGB(pixel, maskSurface->format, &m_mask[j * m_maskW + i],
					&g, &b);
			}
		}

	// Unlock:
	if(SDL_MUSTLOCK(maskSurface)) 
		SDL_UnlockSurface(maskSurface);
}


// Creates a tooltip.
SDL_Surface* UIButton::CreateTooltip(Font *tooltipFont, string tooltipText)
{
	// Render the text on a colored background:
	SDL_Surface *shadedText = 
		tooltipFont->RenderShaded(Font::ALN_LEFT, TOOLTIP_FG, TOOLTIP_BG, tooltipText);

	// Get a pointer to the screen, so we can create surfaces with
	// the same pixel format:
	SDL_Surface* screen = SDL_GetVideoSurface();

	// Create a surface the will form the tooltip's background:
	SDL_Surface *background = SDL_CreateRGBSurface(screen->flags, 
		shadedText->w + 8, shadedText->h + 4,	// Add some spacing
		screen->format->BitsPerPixel,
		screen->format->Rmask,
		screen->format->Gmask,
		screen->format->Bmask, 0);

	TRAP(background == NULL, "UIButton::MakeTooltip - SDL_CreateRGBSurface failed");

	// Fill the surface with the background color:
	SDL_FillRect(background, NULL, SDL_MapRGB(background->format, TOOLTIP_BG.r, TOOLTIP_BG.g, TOOLTIP_BG.b));

	// Create a surface that will form the tooltip's border.
	SDL_Surface *border = SDL_CreateRGBSurface(screen->flags, 
		background->w + 2, background->h + 2,	// Add a 1px border
		screen->format->BitsPerPixel,
		screen->format->Rmask,
		screen->format->Gmask,
		screen->format->Bmask, 0);

	TRAP(border == NULL, "UIButton::MakeTooltip - SDL_CreateRGBSurface failed");

	// Fill the surface with the foreground color:
	SDL_FillRect(border, NULL, SDL_MapRGB(border->format, TOOLTIP_FG.r, TOOLTIP_FG.g, TOOLTIP_FG.b));

	// Blit the text on to the background surface:
	SDL_Rect rect = {4, 2, 0, 0};
	SDL_BlitSurface(shadedText, NULL, background, &rect);

	// Blit the background + text on the border surface:
	rect.x = rect.y = 1;
	SDL_BlitSurface(background, NULL, border, &rect);

	// The text surface is no longer needed:
	SDL_FreeSurface(shadedText);
	SDL_FreeSurface(background);

	return border;
}


// Trigger tooltip timer callback function.
// If by the time this is called the button is still in hover mode,
// then the tooltip should be shown.
Uint32 UIButton::TriggerTooltip(Uint32 interval, void* param)
{
	TooltipParam *tooltipParam = (TooltipParam *) param;

	if (tooltipParam == NULL)
		return 0;
		
	if (*(tooltipParam->currentMode) == UIButton::MODE_HOVER)
		*(tooltipParam->showTooltip) = true;

	// Force a redraw:
	PushUserEvent(EVENT_REDRAW);

	return 0;
}


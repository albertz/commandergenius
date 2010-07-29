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

#include "../UI/uiprogress.h"
#include "../common/trap.h"
#include "../common/events.h"

// Constructor
UIProgressBar::UIProgressBar():
m_BGSurface(NULL), m_normalSurface(NULL), m_flashingSurface(NULL), m_disabledSurface(NULL) 
{
	m_flashingMode = false;
	m_isFlashing = false;
	m_x = m_y = 0;
	m_progress = 0;
	m_clipRect.x = m_clipRect.y = m_clipRect.w = m_clipRect.h = 0;
}


// Sets location and surfaces 
void UIProgressBar::Set(Uint16 x, Uint16 y, SDL_Surface* BGSurface,
	SDL_Surface *normalSurface, SDL_Surface *flashingSurface, SDL_Surface *disabledSurface)
{
	m_x = x;
	m_y = y;

	m_BGSurface = BGSurface;
	m_normalSurface = normalSurface;
	m_flashingSurface = flashingSurface;
	m_disabledSurface = disabledSurface;

	if (normalSurface != NULL)
	{
		m_w = m_clipRect.w = (Uint16) normalSurface->w;
		m_h = m_clipRect.h = (Uint16) normalSurface->h;
	}
}


// Sets the bar's progress. Valid range is 0.0f - 1.0f.
void UIProgressBar::SetProgress(float progress)
{
	if (progress < 0.0f)
		m_progress = 0.0f;
	else if (progress > 1.0f)
		m_progress = 1.0f;
	else
		m_progress = progress;

	
	// Calculate the progress value in pixels:
	Uint16 progressInPixels = (Uint16) (m_progress * m_w); 

	// If the pixel value has changed, the bar needs to be redrawn:
	if (progressInPixels != m_clipRect.w)
	{
		m_clipRect.w = progressInPixels;
		PushUserEvent(EVENT_REDRAW);
	}
}


// Starts or stops the bar's flashing
void UIProgressBar::Flash(bool flag)
{
	// Ignore requests to start flashing while already flashing.
	if (flag == true && m_flashingMode == true)
		return;

	m_flashingMode = m_isFlashing = flag;
	m_flashStart = SDL_GetTicks();

	// Force redraw:
	PushUserEvent(EVENT_REDRAW);
}


// Updates the progress bar.
bool UIProgressBar::Update()
{
	if (m_enabled != true || m_flashingMode == false)
		return false;

	bool redraw = false;

	if	(((SDL_GetTicks() - m_flashStart) / FLASH_INTERVAL) % 2)	// Flashing
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

// Draws the progress bar.
void UIProgressBar::Draw(SDL_Surface *screen)
{
	if (!m_visible || screen == NULL || m_normalSurface == NULL || m_flashingSurface == NULL)
		return;

	
	SDL_Rect rect = {m_x, m_y, 0, 0};

	// Blit background:
	if (m_BGSurface)
		SDL_BlitSurface(m_BGSurface, NULL, screen, &rect);

	// Blit foreground:
	if (!m_enabled)
		SDL_BlitSurface(m_disabledSurface, &m_clipRect, screen, &rect);
	else if (m_isFlashing)
		SDL_BlitSurface(m_flashingSurface, &m_clipRect, screen, &rect);
	else
		SDL_BlitSurface(m_normalSurface, &m_clipRect, screen, &rect);
}

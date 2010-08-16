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

#include "uilabel.h"
#include <cstdlib>
#include <cstdarg>
#include "../common/trap.h"
#include "../common/events.h"

// Constructor
UILabel::UILabel():
m_BGSurface(NULL), m_TextSurface(NULL), m_CompSurface(NULL), m_font(NULL), m_Text("")
{
	m_clipRect.x = m_clipRect.y = m_clipRect.w = m_clipRect.h = 0;
}


// Destructor - free the SDL surfaces we created.
// Note that the background surface is not freed, since it was
// supplied to the object by the caller.
UILabel::~UILabel()
{
	if (m_TextSurface)
		SDL_FreeSurface(m_TextSurface);

	if (m_CompSurface)
		SDL_FreeSurface(m_CompSurface);

	SDL_RemoveTimer(m_timerID);
}

// Sets the label's location and background.
void UILabel::Set(Uint16 x, Uint16 y, SDL_Surface* background, Font* font, SDL_Color textColor, float alpha)
{
	UIControl::SetLocation(x, y);
    m_BGSurface = background;
	m_textColor = textColor;
	m_alpha = alpha;

	if (font != NULL)
		m_font = font;

	if (m_BGSurface != NULL)
	{
		m_w = m_clipRect.w = (Uint16) m_BGSurface->w;
		m_h = m_clipRect.h = (Uint16) m_BGSurface->h;
	}

	PushUserEvent(EVENT_REDRAW);
}


// Returns the label's text.
std::string UILabel::GetText() const
{
	return m_Text;
}


// Sets the label's text, using printf-like formatting.
// The alignment is only relevant to multi-line strings.
// The text surface itself is always centered in the 
// background surface, if once exists.
void UILabel::SetText(HAlign hAlign, const char *format, ...)
{
	// Format the text:
	char formatted[4096];		// Holds the text after formatting
	va_list	listOfArgs;			// Pointer to the list of arguments

	if (format == NULL)			// If there's no text, do nothing
	{
		*formatted = 0;
	}
	else						// Handle the variable-argument list
	{
		va_start(listOfArgs, format);
		vsprintf(formatted, format, listOfArgs);
		va_end(listOfArgs);
	}


	SetText(hAlign, std::string(formatted));
}

// Sets the lable's text.
void UILabel::SetText(HAlign hAlign, const std::string& text)
{
	m_Text = text;

	if (m_TextSurface != NULL)
		SDL_FreeSurface(m_TextSurface);

	if (m_Text == "" && m_font == NULL)
		return;
	
	if (hAlign == HALN_LEFT)
		m_TextSurface = m_font->RenderTransparent(Font::ALN_LEFT, m_textColor, m_alpha, text);
	else if (hAlign == HALN_CENTER)
		m_TextSurface = m_font->RenderTransparent(Font::ALN_CENTER, m_textColor, m_alpha, text);
	else
		m_TextSurface = m_font->RenderTransparent(Font::ALN_RIGHT, m_textColor, m_alpha, text);

	// If no background surface is defined, the label's dimensions are those of the text surface:
	if (m_BGSurface == NULL)
	{
		m_w = m_clipRect.w = (Uint16) m_TextSurface->w;
		m_h = m_clipRect.h = (Uint16) m_TextSurface->h;
	}

	PushUserEvent(EVENT_REDRAW);
}


// Sets the background surface.
void UILabel::SetBackground(SDL_Surface *background)
{
	m_BGSurface = background;

	if (m_BGSurface != NULL)
	{
		m_w = (Uint16) m_BGSurface->w;
		m_h = (Uint16) m_BGSurface->h;
	}
}


// Sets the clipping rectangle.
// Setting either dimension to a negative number cancels
// clipping of that dimension.
void UILabel::SetClipping(Sint16 w, Sint16 h)
{
	m_clipRect.w = (w < 0) ? m_w : w;
	m_clipRect.h = (h < 0) ? m_h : h;	
}

// Show the label for a set amount of time.
void UILabel::ShowTimed(Uint32 time)
{
	m_visible = true;

	// Remove any existing timers:
	SDL_RemoveTimer(m_timerID);

	// Set the tooltip callback timer:
	m_timerID = SDL_AddTimer(time, HideLabel, &m_visible);
}


// Draws the label.
// The method can use a clipping rectangle to clip the surface.
void UILabel::Draw(SDL_Surface *screen)
{
	if (m_visible == false)
		return;

	SDL_Rect rect = {0, 0, 0, 0};

	if (m_BGSurface != NULL)
	{
		rect.x = m_x;
		rect.y = m_y;
		
		SDL_BlitSurface(m_BGSurface, &m_clipRect, screen, &rect);
	}

	if (m_TextSurface != NULL)
	{
		rect.x = (Sint16) (m_x + (m_w - m_TextSurface->w) / 2);
		rect.y = (Sint16) (m_y + (m_h - m_TextSurface->h) / 2);

		SDL_BlitSurface(m_TextSurface, &m_clipRect, screen, &rect);
	}
}


// Hides the label.
// This is an SDL_timer callback function.
Uint32 UILabel::HideLabel(Uint32 interval, void* param)
{
	bool *visible = (bool *) param;
	*visible = false;

	// Force a redraw:
	PushUserEvent(EVENT_REDRAW);

	return 0;
}


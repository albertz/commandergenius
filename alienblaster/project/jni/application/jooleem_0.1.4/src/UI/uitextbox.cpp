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

#include "../UI/uitextbox.h"
#include "../common/trap.h"

// Constructor
UITextBox::UITextBox():
m_BGSurface(NULL), m_textSurface(NULL), m_cursorSurface(NULL), m_font(NULL), m_text("")
{
	m_cursorIndex = 0;
	m_blinking = true;
}


// Destructor - free the SDL surfaces we created.
// Note that the background and cursor surfaces are not freed, 
// since they were supplied to the object by the caller.
UITextBox::~UITextBox()
{
	if (m_textSurface)
		SDL_FreeSurface(m_textSurface);
}


// Sets the label's location and background.
void UITextBox::Set(Uint16 x, Uint16 y,				// Box location
				Uint16 textX, Uint16 textY,			// Text location relative to the box
				SDL_Surface* background, SDL_Surface *cursorSurface,	// background and cursor surfaces
				Font *font, SDL_Color textColor)	// Font and text color 
{
	UIControl::SetLocation(x, y);
	m_textX = textX;
	m_textY = textY;

    m_BGSurface = background;
	m_cursorSurface = cursorSurface;
	m_textColor = textColor;

	m_font = font;
}


// Clears the text.
void UITextBox::Clear()
{
	SetText("");
}


// Returns the label's text.
std::string UITextBox::GetText() const
{
	return m_text;
}


// Sets the box's text.
void UITextBox::SetText(string text)
{
	m_text = text;
	m_cursorIndex = (Uint16) m_text.length();
	Refresh();
}


// Update the cursor blink.
bool UITextBox::Update()
{
	if (m_enabled != true)
		return false;

	bool redraw = false;

	if	(SDL_GetTicks() % (CURSOR_BLINK * 2) < CURSOR_BLINK)	// Blink on
	{
		if (!m_blinking) // ... but was not flashing before
			redraw = true;
		m_blinking = true;
	}
	else // Not flashing
	{
		if (m_blinking) // ... but was flashing before
			redraw = true;
		m_blinking = false;
	}

	return redraw;
}

// Draws the text box.
void UITextBox::Draw(SDL_Surface *screen)
{
	if (m_visible == false)
		return;

	SDL_Rect rect = {0, 0, 0, 0};

	// Background:
	if (m_BGSurface != NULL)
	{
		rect.x = m_x;
		rect.y = m_y;
		
		SDL_BlitSurface(m_BGSurface, NULL, screen, &rect);
	}

	// Text:
	if (m_textSurface != NULL)
	{
		rect.x = m_textX + m_x;
		rect.y = m_textY + m_y;

		SDL_BlitSurface(m_textSurface, NULL, screen, &rect);
	}

	// Cursor:
	if (m_cursorSurface != NULL && m_blinking)
		//SDL_GetTicks() % (CURSOR_BLINK * 2) < CURSOR_BLINK)
	{
		rect.x = m_x + m_textX + (m_charOffset.empty() ? 0 : m_charOffset[m_cursorIndex]);
		rect.y = m_y + m_textY + 1;

		SDL_BlitSurface(m_cursorSurface, NULL, screen, &rect);
	}
}


// Process mouse down events.
// A click sets the cursor position.
bool UITextBox::MouseDown(Uint16 x, Uint16 y)
{
	// Check if the click in inside the text area:
	if (m_textSurface == NULL || x < m_x + m_textX || x > m_x + m_textX + m_textSurface->w
		|| y < m_y + m_textY || y > m_y + m_textY + m_textSurface->h)
		return false;

	// Scan the offset vector to see where to place the cursor:
	Uint16 A, B;

	for (Uint16 i = 0 ; i < (Uint16) m_charOffset.size() ; ++i)
	{
		if (i == 0)
			A = m_x + m_textX + m_charOffset[i];
		else
			A = (m_x + m_textX + m_charOffset[i - 1] + m_x + m_textX + m_charOffset[i]) / 2;

		if (i == (Uint16) m_charOffset.size())
			B = m_x + m_textX + m_charOffset[i];
		else
			B = (m_x + m_textX + m_charOffset[i] + m_x + m_textX + m_charOffset[i + 1]) / 2;

		if (x >= A && x < B)
			m_cursorIndex = i;
	}

	return true;
}


// Handle key down events.
bool UITextBox::KeyDown(SDL_KeyboardEvent *event)
{
	switch (event->keysym.sym)	// Get the key symbol
	{
		// Left arrow:
		case SDLK_LEFT:
			if (m_cursorIndex > 0)
				m_cursorIndex--;
			break;

		// Right arrow:
		case SDLK_RIGHT:
			if (m_cursorIndex < m_text.length())
				m_cursorIndex++;
			break;

		// Home:
		case SDLK_HOME:
			m_cursorIndex = 0;
			break;

		// End:
		case SDLK_END:
			m_cursorIndex = (Uint16) m_text.length();
			break;

		// Backspace:
		case SDLK_BACKSPACE:
			if (m_cursorIndex > 0)
			{	
				m_text.erase(m_cursorIndex - 1, 1);
				m_cursorIndex--;
				Refresh();
			}
			break;
			
		// Delete:
		case SDLK_DELETE:
			if (m_cursorIndex < m_text.length())
			{
				m_text.erase(m_cursorIndex, 1);
				Refresh();
			}
			break;

		// Characters:
		default:
		{
			// Make sure text doesn't graphically overflow the background image:
			if (m_textSurface != NULL && 
				m_textSurface->w >= (m_BGSurface->w - 3 * m_textX)) 
				break;

			char c = 0;

			// Convert the unicode value into ASCII:
			if ((event->keysym.unicode & 0xFF80) == 0)
			{
				c = (char)(event->keysym.unicode & 0x7F);
				
				if (c >= ' ' && c <= '~')	// Add only displayable chars
				{
					// Insert the character to the string at the current cursor position:
					m_text.insert(m_cursorIndex, string(1,c));
					m_cursorIndex++;
					Refresh();
				}
			}
			else	// International char - ignore
				return false;
				break;
		}
	}

	return true;
}


// Refreshes the text box when changed.
void UITextBox::Refresh()
{
	MeasureOffsets();

	SDL_FreeSurface(m_textSurface);
	m_textSurface = m_font->RenderPlain(Font::ALN_LEFT, m_textColor, m_text);
}


// Measure the characters' offset.
// The method populates the offset vector with the offsets
// of the string characters, in pixels, when rendered with
// the object's font.
void UITextBox::MeasureOffsets()
{
	m_charOffset.clear();

	m_charOffset.push_back(0);
	for (size_t i = 0 ; i < m_text.length() ; ++i)
	{
		Uint16 temp = m_font->GetGlyphAdvance((Uint16) m_text[i]);
		m_charOffset.push_back(temp + m_charOffset[i]);
	}
}


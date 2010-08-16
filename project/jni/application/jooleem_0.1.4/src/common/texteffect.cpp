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

#include "texteffect.h"
#include "../common/trap.h"
#include "../common/common.h"

// Constructor.
TextEffect::TextEffect(Sint16 x, Sint16 y, string text, Font *font, SDL_Color color, float initialAlpha,
			float velocity, Uint32 lifeTime, float gravity)
{
	m_x = x;
	m_y = y;
	m_text = text;
	m_font = font;
	m_color = color;
	
	if (initialAlpha > 1.0f || initialAlpha < 0.0f)
		m_alpha = 255;
	else
		m_alpha = (Uint8) (255 * initialAlpha);

	m_visible = (m_alpha > 0.0f);
	m_velocity = velocity;
	m_lifeTime = lifeTime;
	m_gravity = gravity;
	
	m_surface = NULL;

	Render(true);

	m_startTime = SDL_GetTicks();
}


// Destrcutor.
// Free the surface.
TextEffect::~TextEffect()
{
	if (m_surface)
		SDL_FreeSurface(m_surface);
}

// Updates the effect.
// Returns true if a redraw is required.
bool TextEffect::Update()
{
	if (m_visible == false)
		return false;

	bool redraw = false;

	// Calculate the alpha value, as a function of the time.
	// A signed int is used to prevent underflow (the effect will become opaque again).
	Sint16 alpha = 255 - (Sint16) (255 * ((SDL_GetTicks() - m_startTime) / (float) m_lifeTime));
	if (alpha < 0)
		alpha = 0;

	if (alpha != (Sint16) m_alpha)
	{
		m_alpha = (Uint8) alpha;
		redraw = true;

		//cerr << alpha << endl;

		// Check if the surface is now fully transparent:
		if (alpha == 0)
			m_visible = false;

		Render(false);	// The surface needs to be re-rendered with each change to its alpha
	}

	// Update the velocity, as a function of m_gravity and time:
	m_velocity += ((SDL_GetTicks() - m_startTime) / 1000.0f) * m_gravity;

	// Update the location:
	Sint16 y = m_y - (Sint16) m_velocity;
	
	if (y != m_y)
	{
		m_y = y;
		redraw = true;

		// Check if the surface went out of bounds:
		if (y + m_h < 0 || y > SCREEN_HEIGHT)
			m_visible = false;
	}

	return redraw;
}


// Draws the effect.
void TextEffect::Draw(SDL_Surface *screen)
{
	SDL_Rect rect;

	rect.x = m_x;
	rect.y = m_y;
		
	SDL_BlitSurface(m_surface, NULL, screen, &rect);
}


// Renders the surface.
void TextEffect::Render(bool center)
{
	if (m_surface)
		SDL_FreeSurface(m_surface);

	m_surface = m_font->RenderTransparent(Font::ALN_CENTER, m_color, m_alpha / 255.0f, m_text);

	m_w = (Sint16) m_surface->w;
	m_h = (Sint16) m_surface->h;

	// Center the text:
	if (center)
	{
		m_x -= m_w / 2;
		m_y -= m_h / 2;
	}
}


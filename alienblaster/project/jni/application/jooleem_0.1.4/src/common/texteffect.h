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

#ifndef _TEXTEFFECT_H_
#define _TEXTEFFECT_H_

// Text effect.
// Basically, a surface with a text message that can move and fade out.

#include "SDL.h"
#include <string>

#include "../common/font.h"

using namespace std;

class TextEffect
{
	private:
		string m_text;			// The effect's text
		SDL_Surface *m_surface;	// The current surface
		Font *m_font;			// Font used to render the text
		SDL_Color m_color;		// Font color

		Sint16 m_x, m_y, m_w, m_h;	// Location and dimensions
		Uint8 m_alpha;			// Current alpha value
		bool m_visible;			// Flag indicating the effect's visibilty
		
		Uint32 m_startTime;		// Starting time
		float m_velocity;		// Effect's current velocity (pixels per second)
		Uint32 m_lifeTime;		// The effect's lifetime, in ms
		float m_gravity;		// m_gravity factor (in pixels per second per second)

	public:

		// Constructor / destructor:
		TextEffect(Sint16 x, Sint16 y, string text, Font *font, SDL_Color color, float initialAlpha = 1.0f,
			float velocity = 0.0f, Uint32 lifeTime = 5000, float gravity = 0.5f);
		~TextEffect();

		bool IsActive(){return m_visible;}

		bool Update();					// Updates the effect

		void Draw(SDL_Surface *screen);	// Draws the effect

	private:
		void Render(bool center);		// Renders the surface

};


#endif


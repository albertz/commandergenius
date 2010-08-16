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

#ifndef _FONT_H_
#define _FONT_H_

// Font wrapper class.
// A font is defined as a face (e.g. Times New Roman) of a specific
// point size (e.g. 16).
// The class supports multi-line strings and text alignment.

#include "SDL.h"
#include "SDL_ttf.h"
#include <string>
#include <vector>

using namespace std;

class Font
{
	public:
		enum Alignment {ALN_LEFT, ALN_CENTER, ALN_RIGHT};		// Text alignment
		enum Style {STYLE_NORMAL, STYLE_BOLD, STYLE_ITALIC, STYLE_UNDERLINE};	// Font styles

	private:
		TTF_Font *m_font;									// The SDL_ttf font

	public:
		Font(string TTFfile, int size);
		~Font();

		// Renders plain text:
		SDL_Surface *RenderPlain(Font::Alignment alignment, SDL_Color color, string text);
		SDL_Surface *RenderPlain(Font::Alignment alignment, SDL_Color color, const char *format, ...);

		// Renders transparent text:
		SDL_Surface *RenderTransparent(Font::Alignment alignment, SDL_Color color, float alpha, string text);
		SDL_Surface *RenderTransparent(Font::Alignment alignment, SDL_Color color, float alpha, 
			const char *format, ...);

		// Renders shaded text (text on a colored background):
		SDL_Surface *RenderShaded(Font::Alignment alignment, SDL_Color color, SDL_Color BGcolor, string text);
		SDL_Surface *RenderShaded(Font::Alignment alignment, SDL_Color color, SDL_Color BGcolor, 
			const char *format, ...);

		// Gets the advance value of a glyph:
		Uint16 GetGlyphAdvance(Uint16 c); 

		// Sets the font style:
		void SetStyle(Style style);

	private:
		static const int MAX_TEXT_LEN = 4096;					// Maximum text length

		// Full control of text's appearance - color, transparency & background.
		// Used by the various Render* methods.
		SDL_Surface *Render(Font::Alignment alignment, SDL_Color color, SDL_Color BGcolor,
			float alpha, string text);
		SDL_Surface *Render(Font::Alignment alignment, SDL_Color color, SDL_Color BGcolor,
			float alpha, const char *format, ...);

		void SplitToLines(string text, vector<string>& lines);	// Splits the text into lines
};


#endif


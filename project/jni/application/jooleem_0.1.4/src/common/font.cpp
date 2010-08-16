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

#include "../common/font.h"
#include "../common/surfacemanager.h"

#include <cstdlib>
#include <cstdarg>
#include "../common/trap.h"


// Initializes a font object, using the argument 
// TTF file and point size.
Font::Font(string TTFfile, int size)
{
	m_font = TTF_OpenFont(TTFfile.c_str(), size);

	TRAP(m_font == NULL, "Font::Font() - Could not open " << TTFfile);
}

Font::~Font()
{
	TTF_CloseFont(m_font);
}


// Renders a text surface.
SDL_Surface *Font::Render(Font::Alignment alignment, SDL_Color color, SDL_Color BGcolor,
			float alpha, string text)
{
	TRAP(m_font == NULL, "Font::Render() - Font not initialized");

	if (text == "")
		return NULL;

	// Split the text into seperate lines:
	std::vector<std::string> lines;
	SplitToLines(text, lines);

	// Determine rendering mode: If the foreground and
	// background colors are identical, don't render the
	// background.
	bool shaded = (BGcolor.r == color.r && BGcolor.g == color.g && BGcolor.b == color.b) ? false : true;

	// Render each line to a surface, and remember the widest line:
	std::vector<SDL_Surface*> lineSurfaces;
	int maxLineWidth = 0;
	int combinedHeight = 0;
	for (size_t i = 0 ; i < lines.size() ; ++i)
	{
		SDL_Surface *temp;

		// Determine rendering mode, and render the line:
		if (shaded) // Renders with a colored background
		{
			SDL_Surface *temp1 = TTF_RenderText_Shaded(m_font, lines[i].c_str(), color, BGcolor);

			// The rendered surface must be converted to the display format, otherwise it
			// won't show up properly.
			temp = SDL_DisplayFormatAlpha(temp1);

			SDL_FreeSurface(temp1);
		}
		else		// Renders blended
			temp = TTF_RenderText_Blended(m_font, lines[i].c_str(), color);

		TRAP(temp == NULL, "Font::Render() - TTF_RenderText_Blended() failed");
		
		if (temp->w > maxLineWidth)
			maxLineWidth = temp->w; 

		combinedHeight += temp->h;

		lineSurfaces.push_back(temp);
	}

	// Calcaulate the line spacing (in pixels):
	// int lineSpacing = (TTF_FontLineSkip(m_font) - TTF_FontAscent(m_font)) / 2;
	// FIX:
	int lineSpacing = -3;

	// Create a new surface, to hold all the lines combined:
	SDL_Surface *textSurface = SDL_CreateRGBSurface(
		lineSurfaces[0]->flags, 
		maxLineWidth,
		(int) (combinedHeight + (lines.size() - 1) * lineSpacing),
		lineSurfaces[0]->format->BitsPerPixel,
		lineSurfaces[0]->format->Rmask,
		lineSurfaces[0]->format->Gmask,
		lineSurfaces[0]->format->Bmask,
		lineSurfaces[0]->format->Amask);

	TRAP(textSurface == NULL, "Font::Render() - SDL_CreateRGBSurface() failed");

	// Fill the surface with the background color, if applicable:
	if (shaded)
		SDL_FillRect(textSurface, NULL, 
		SDL_MapRGB(textSurface->format,	BGcolor.r, BGcolor.g, BGcolor.b));
	
	// Blit the lines to the new surface, freeing their surfaces as we go along:
	SDL_Rect rect = {0,0,0,0};
	SurfaceManager *sfcMgr = SurfaceManager::GetInstance();
	for (size_t i = 0 ; i < lineSurfaces.size() ; ++i)
	{
		// Horizontal alignment:
		switch (alignment)
		{
			case ALN_LEFT:
				rect.x = 0;
				break;
			case ALN_CENTER:
				rect.x = (Sint16)((maxLineWidth - lineSurfaces[i]->w) / 2);
				break;
			case ALN_RIGHT:
				rect.x = (Sint16)(maxLineWidth - lineSurfaces[i]->w);
				break;
			default:
				ERR("Font::Render() - Invalid alignment");
				break;
		}

		// Vertical placing:
		if (i == 0)
			rect.y = 0;
		else
		{
			rect.y = (Sint16)(i * (lineSurfaces[i - 1]->h + lineSpacing));
			SDL_FreeSurface(lineSurfaces[i - 1]);
		}
		
		sfcMgr->Blit(lineSurfaces[i], textSurface, alpha, &rect);
	}
	SDL_FreeSurface(lineSurfaces[lineSurfaces.size() - 1]);

	return textSurface;
}

SDL_Surface *Font::Render(Font::Alignment alignment, SDL_Color color, SDL_Color BGcolor,
			float alpha, const char *format, ...)
{
	// Format the text:
	char formatted[MAX_TEXT_LEN];	// Holds the text after formatting
	va_list	listOfArgs;				// Pointer to the list of arguments

	if (format == NULL)				// If there's no text, do nothing
	{
		*formatted = 0;
	}
	else						// Handle the variable-argument list
	{
		va_start(listOfArgs, format);
		vsprintf(formatted, format, listOfArgs);
		va_end(listOfArgs);
	}
 
	return Render(alignment, color, BGcolor, alpha, string(formatted));
}

// Renders transparent text.
SDL_Surface *Font::RenderTransparent(Font::Alignment alignment, SDL_Color color, float alpha, string text)
{
	return Render(alignment, color, color, alpha, text);
}

// Renders transparent text, with printf-like formatting.
SDL_Surface *Font::RenderTransparent(Font::Alignment alignment, SDL_Color color, float alpha, 
			const char *format, ...)
{
	// Format the text:
	char formatted[MAX_TEXT_LEN];	// Holds the text after formatting
	va_list	listOfArgs;				// Pointer to the list of arguments

	if (format == NULL)				// If there's no text, do nothing
	{
		*formatted = 0;
	}
	else						// Handle the variable-argument list
	{
		va_start(listOfArgs, format);
		vsprintf(formatted, format, listOfArgs);
		va_end(listOfArgs);
	}
 
	return Render(alignment, color, color, alpha, string(formatted));
}


// Renders a string 
SDL_Surface *Font::RenderPlain(Font::Alignment alignment, SDL_Color color, string text)
{
	return Render(alignment, color, color, 1.0f, text);
}

SDL_Surface *Font::RenderPlain(Font::Alignment alignment, SDL_Color color, const char *format, ...)
{
	// Format the text:
	char formatted[MAX_TEXT_LEN];	// Holds the text after formatting
	va_list	listOfArgs;				// Pointer to the list of arguments

	if (format == NULL)				// If there's no text, do nothing
	{
		*formatted = 0;
	}
	else							// Handle the variable-argument list
	{
		va_start(listOfArgs, format);
		vsprintf(formatted, format, listOfArgs);
		va_end(listOfArgs);
	}
 
	return Render(alignment, color, color, 1.0f, string(formatted));
}

// Renders shaded text (text on a colored background):
SDL_Surface *Font::RenderShaded(Font::Alignment alignment, SDL_Color color, SDL_Color BGcolor, string text)
{
	return Render(alignment, color, BGcolor, 1.0f, text);
}

SDL_Surface *Font::RenderShaded(Font::Alignment alignment, SDL_Color color, SDL_Color BGcolor, 
			const char *format, ...)
{
	// Format the text:
	char formatted[MAX_TEXT_LEN];	// Holds the text after formatting
	va_list	listOfArgs;				// Pointer to the list of arguments

	if (format == NULL)				// If there's no text, do nothing
	{
		*formatted = 0;
	}
	else							// Handle the variable-argument list
	{
		va_start(listOfArgs, format);
		vsprintf(formatted, format, listOfArgs);
		va_end(listOfArgs);
	}
 
	return Render(alignment, color, BGcolor, 1.0f, string(formatted));
}


// Gets the advance value of a glyph.
// The advance is the total horizontal space the
// rendered glyph occupies, including the spacing
// between it and the following glyph.
// See http://jcatki.no-ip.org/SDL_ttf/metrics.png
Uint16 Font::GetGlyphAdvance(Uint16 c)
{
	int ret = 0;

	TTF_GlyphMetrics(m_font, c, NULL, NULL, NULL, NULL, &ret);

	return (Uint16) ret;
}


// Sets the font style:
void Font::SetStyle(Style style)
{
	if (m_font == NULL)
		return;

	switch (style)
	{
		case STYLE_NORMAL:
			TTF_SetFontStyle(m_font, TTF_STYLE_NORMAL);
			break;
		case STYLE_BOLD:
			TTF_SetFontStyle(m_font, TTF_STYLE_BOLD);
			break;
		case STYLE_ITALIC:
			TTF_SetFontStyle(m_font, TTF_STYLE_ITALIC);
			break;
		case STYLE_UNDERLINE:
			TTF_SetFontStyle(m_font, TTF_STYLE_UNDERLINE);
			break;
		default:
			return;
	}
}


// Splits the text into lines. 
// The result is stored in a vector of strings.
void Font::SplitToLines(std::string text, std::vector<std::string>& lines)
{
	lines.empty();

	size_t nextNewLine = 0;
	size_t lastNewLine = 0;

	while ((nextNewLine = text.find('\n', lastNewLine)) != text.npos)
	{
		lines.push_back(text.substr(lastNewLine, nextNewLine - lastNewLine));
		lastNewLine = nextNewLine + 1;
	}

	lines.push_back(text.substr(lastNewLine, text.length() - lastNewLine));
}



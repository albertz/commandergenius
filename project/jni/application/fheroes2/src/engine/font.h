/***************************************************************************
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   Part of the Free Heroes2 Engine:                                      *
 *   http://sourceforge.net/projects/fheroes2                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef H2FONT_H
#define H2FONT_H

#ifdef WITH_TTF
#include <string>
#include "types.h"
#include "SDL_ttf.h"

class Surface;

namespace SDL
{
    class Font
    {
    public:
	enum render_t { SOLID, BLENDED };

	Font();
	~Font();

	static void Init(void);
	static void Quit(void);

	bool Open(const std::string &, u8);
	bool isValid(void) const;
	void SetStyle(u8);

	int Height(void) const;
	int Ascent(void) const;
	int Descent(void) const;
	int LineSkip(void) const;

	void RenderText(Surface &, const std::string &, const Colors &, render_t = SOLID);
	void RenderChar(Surface &, char, const Colors &, render_t = SOLID);
	void RenderUnicodeText(Surface &, const u16 *, const Colors &, render_t = SOLID);
	void RenderUnicodeChar(Surface &, u16, const Colors &, render_t = SOLID);

    private:
	TTF_Font *fnt;

	static bool init;
    };
}
#endif

#endif

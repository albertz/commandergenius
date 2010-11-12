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

#include "SDL.h"
#include "surface.h"
#include "error.h"
#include "palette_h2.h"
#include "palette.h"

#define PALETTE_SIZE 255

Palette::Palette()
{
    sdlpal = new SDL_Palette;
    sdlpal->ncolors = PALETTE_SIZE;
    sdlpal->colors  = new SDL_Color[PALETTE_SIZE];

    pal = new u32 [PALETTE_SIZE];

    Surface sfa;
    sfa.CreateSurface(1, 1, Surface::GetDefaultDepth(), SDL_SWSURFACE|SDL_SRCALPHA);
    const unsigned char *p = kb_pal;

    for(u16 ii = 0; ii < PALETTE_SIZE; ++ii)
    {
        sdlpal->colors[ii].r = *p++;
        sdlpal->colors[ii].g = *p++;
        sdlpal->colors[ii].b = *p++;

        sdlpal->colors[ii].r <<= 2;
        sdlpal->colors[ii].g <<= 2;
        sdlpal->colors[ii].b <<= 2;

        pal[ii] = SDL_MapRGBA(sfa.surface->format, sdlpal->colors[ii].r, sdlpal->colors[ii].g, sdlpal->colors[ii].b, 0xFF);
    }
}

Palette::~Palette()
{
    if(sdlpal)
    {
	if(sdlpal->colors) delete [] sdlpal->colors;
	delete sdlpal;
    }
    if(pal) delete [] pal;
}

Palette & Palette::Get(void)
{
    static Palette pal_cache;

    return pal_cache;
}

u16 Palette::Size(void) const
{
    return PALETTE_SIZE;
}

u32 Palette::GetColor(u16 index) const
{
    return index < PALETTE_SIZE ? pal[index] : 0;
}

const SDL_Palette * Palette::SDLPalette(void) const
{
    return sdlpal;
}

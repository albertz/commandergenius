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

#ifdef WITH_TTF

#include <iostream>
#include "font.h"
#include "engine.h"
#include "surface.h"
#include "SDL_ttf.h"

bool SDL::Font::init = false;

SDL::Font::Font() : fnt(NULL)
{
}

SDL::Font::~Font()
{
    if(fnt) TTF_CloseFont(fnt);
}

void SDL::Font::Init(void)
{
    if(0 != TTF_Init()) std::cerr << "Font::Init: error" << std::endl;
    else init = true;
}

void SDL::Font::Quit(void)
{
    TTF_Quit();
    init = false;
}

bool SDL::Font::isValid(void) const
{
    return fnt;
}

bool SDL::Font::Open(const std::string & filename, u8 size)
{
    if(init)
    {
	if(fnt) TTF_CloseFont(fnt);

	fnt = TTF_OpenFont(filename.c_str(), size);

	if(!fnt) std::cerr << "Font::Open: error open: " << filename << std::endl;
    }
    return fnt;
}

void SDL::Font::SetStyle(u8 style)
{
    if(fnt) TTF_SetFontStyle(fnt, style);
}

void SDL::Font::RenderText(Surface & dst, const std::string & msg, const Colors & clr, render_t render)
{
    if(dst.surface) Surface::FreeSurface(dst);
    if(fnt) switch(render)
    {
	case BLENDED:	dst.surface = TTF_RenderUTF8_Blended(fnt, msg.c_str(), clr);	break;
	default:	dst.surface = TTF_RenderUTF8_Solid(fnt, msg.c_str(), clr);	break;
    }
}

void SDL::Font::RenderChar(Surface & dst, char ch, const Colors & clr, render_t render)
{
    char buf[2] = { '\0', '\0' };
         buf[0] = ch;

    if(dst.surface) Surface::FreeSurface(dst);
    if(fnt) switch(render)
    {
	case BLENDED:	dst.surface = TTF_RenderUTF8_Blended(fnt, buf, clr);	break;
	default:	dst.surface = TTF_RenderUTF8_Solid(fnt, buf, clr);	break;
    }
}

void SDL::Font::RenderUnicodeText(Surface & dst, const u16 *msg, const Colors & clr, render_t render)
{
    if(dst.surface) Surface::FreeSurface(dst);
    if(fnt) switch(render)
    {
	case BLENDED:	dst.surface = TTF_RenderUNICODE_Blended(fnt, msg, clr);	break;
	default:	dst.surface = TTF_RenderUNICODE_Solid(fnt, msg, clr);	break;
    }
}

void SDL::Font::RenderUnicodeChar(Surface & dst, u16 ch, const Colors & clr, render_t render)
{
    u16 buf[2] = { L'\0', L'\0' };
        buf[0] = ch;

    if(dst.surface) Surface::FreeSurface(dst);
    if(fnt) switch(render)
    {
	case BLENDED:	dst.surface = TTF_RenderUNICODE_Blended(fnt, buf, clr);	break;
	default:	dst.surface = TTF_RenderUNICODE_Solid(fnt, buf, clr);	break;
    }
}

int SDL::Font::Height(void) const
{
    return fnt ? TTF_FontHeight(fnt) : 0;
}

int SDL::Font::Ascent(void) const
{
    return fnt ? TTF_FontAscent(fnt) : 0;
}

int SDL::Font::Descent(void) const
{
    return fnt ? TTF_FontDescent(fnt) : 0;
}

int SDL::Font::LineSkip(void) const
{
    return fnt ? TTF_FontLineSkip(fnt) : 0;
}

#endif

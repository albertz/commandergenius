/***************************************************************************
 *   Copyright (C) 2008 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#include <iostream>
#include <algorithm>
#include <string>
#include "rect.h"
#include "types.h"
#include "error.h"
#include "display.h"

UpdateRects::UpdateRects() : bits(NULL), len(0), bf(0), bw(0)
{
}

UpdateRects::~UpdateRects()
{
    delete [] bits;
}

void UpdateRects::SetVideoMode(u16 dw, u16 dh)
{
    if(dw < 640)
    {
	bw = 4;
	bf = 2;
    }
    else
    if(dw > 640)
    {
	bw = 16;
	bf = 4;
    }
    else
    {
	bw = 8;
	bf = 3;
    }

    // fix bw and bf
    while(((dw % bw) || (dh % bw)) && 1 < bf)
    {
	bw >>= 1;
	--bf;
    }

    len = (dw >> bf) * (dh >> bf);
    len = (len % 8 ? (len >> 3) + 1 : len >> 3);

    if(bits) delete [] bits;
    bits = new u8 [len];
    std::fill(bits, bits + len, 0);

    rects.reserve(len / 4);
}

size_t UpdateRects::Size(void) const
{
    return rects.size();
}

void UpdateRects::Clear(void)
{
    std::fill(bits, bits + len, 0);
    rects.clear();
}

SDL_Rect* UpdateRects::Data(void)
{
    return rects.size() ? &rects[0] : NULL;
}

void UpdateRects::PushRect(s16 px, s16 py, u16 pw, u16 ph)
{
    Display & display = Display::Get();

    if(0 != pw && 0 != ph &&
	px + pw > 0 && py + ph > 0 &&
	px < display.w() && py < display.h())
    {
	if(px < 0)
	{
	    pw += px;
	    px = 0;
	}

	if(py < 0)
	{
	    ph += py;
	    py = 0;
	}

	if(px + pw > display.w())
	    pw = display.w() - px;

	if(py + ph > display.h())
	    ph = display.h() - py;

	const u16 dw = display.w() >> bf;
	s16 xx, yy;

	for(yy = py; yy < py + ph; yy += bw)
	    for(xx = px; xx < px + pw; xx += bw)
		SetBit((yy >> bf) * dw + (xx >> bf), 1);

	yy = py + ph - 1;
	for(xx = px; xx < px + pw; xx += bw)
	    SetBit((yy >> bf) * dw + (xx >> bf), 1);

	xx = px + pw - 1;
	for(yy = py; yy < py + ph; yy += bw)
	    SetBit((yy >> bf) * dw + (xx >> bf), 1);

	yy = py + ph - 1;
	xx = px + pw - 1;
	SetBit((yy >> bf) * dw + (xx >> bf), 1);
    }
}

bool UpdateRects::BitsToRects(void)
{
    Display & display = Display::Get();

    const u16 dbf = display.w() >> bf;
    const size_t len2 = len << 3;
    size_t index;
    SDL_Rect rect;
    SDL_Rect* prt = NULL;

    for(index = 0; index < len2; ++index)
    {
	if(GetBit(index))
	{
    	    if(NULL != prt)
	    {
		if(static_cast<size_t>(rect.y) == (index / dbf) * bw)
		    rect.w += bw;
		else
		{
		    rects.push_back(*prt);
		    prt = NULL;
		}
    	    }

    	    if(NULL == prt)
    	    {
		rect.x = (index % dbf) * bw;
		rect.y = (index / dbf) * bw;
		rect.w = bw;
		rect.h = bw;
		prt = &rect;
    	    }
	}
	else
	{
    	    if(prt)
    	    {
		rects.push_back(*prt);
		prt = NULL;
    	    }
	}
    }

    if(prt)
    {
	rects.push_back(*prt);
	prt = NULL;
    }

    return rects.size();
}

void UpdateRects::SetBit(u32 index, bool value)
{
    if(value != GetBit(index))
	bits[index >> 3] ^= (1 << (index % 8));
}

bool UpdateRects::GetBit(u32 index) const
{
    return (bits[index >> 3] >> (index % 8));
}

Display::Display()
{
}

Display::~Display()
{
}

Display & Display::operator= (const Display & dp)
{
    surface = SDL_GetVideoSurface();

    return *this;
}

void Display::SetVideoMode(const u16 w, const u16 h, u32 flags)
{
    Display & display = Display::Get();

    if(display.isValid() && display.w() == w && display.h() == h) return;

    if(display.surface && (display.surface->flags & SDL_FULLSCREEN)) flags |= SDL_FULLSCREEN;
    display.surface = SDL_SetVideoMode(w, h, 16, flags);

    if(!display.surface)
	Error::Except("SDL_SetVideoMode: ", SDL_GetError());

    display.update_rects.SetVideoMode(display.w(), display.h());
}

/* flip */
void Display::Flip()
{
    Display & display = Display::Get();

    if(display.surface->flags & SDL_HWSURFACE)
	SDL_Flip(display.surface);
    else
    if(display.update_rects.BitsToRects())
    {
	SDL_UpdateRects(display.surface, display.update_rects.Size(), display.update_rects.Data());
	display.update_rects.Clear();
    }
}

/* full screen */
void Display::FullScreen(void)
{
    Display & display = Display::Get();

    SDL_WM_ToggleFullScreen(display.surface);
}

/* set caption main window */
void Display::SetCaption(const std::string & caption)
{
    SDL_WM_SetCaption(caption.c_str(), NULL);
}

/* set icons window */
void Display::SetIcons(const Surface & icons)
{
    SDL_WM_SetIcon(const_cast<SDL_Surface *>(icons.GetSurface()), NULL);
}

/* hide system cursor */
void Display::HideCursor(void)
{
    SDL_ShowCursor(SDL_DISABLE);
}

/* show system cursor */
void Display::ShowCursor(void)
{
    SDL_ShowCursor(SDL_ENABLE);
}

bool Display::Fade(u8 fadeTo)
{
    Display & display = Display::Get();
    u8 alpha = display.GetAlpha();

    if(alpha == fadeTo) return false;
    else
    if(alpha < fadeTo) return Rise(fadeTo);
    if(display.w() != 640 || display.h() != 480) return false;

    Surface temp(display);
    temp.SetDisplayFormat();
    temp.Blit(display);
    const u32 black = temp.MapRGB(0, 0, 0);

    while(alpha > fadeTo)
    {
	alpha -= alpha - 10 > fadeTo ? 10 : alpha - fadeTo;
	display.Fill(black);
	temp.SetAlpha(alpha);
	display.Blit(temp);
        display.Flip();
	DELAY(10);
    }

    return true;
}

bool Display::Rise(u8 riseTo)
{
    Display & display = Display::Get();
    u8 alpha = display.GetAlpha();

    if(alpha == riseTo) return false;
    else
    if(riseTo < alpha) return Fade(riseTo);
    if(display.w() != 640 || display.h() != 480) return false;

    Surface temp(display);
    temp.SetDisplayFormat();
    temp.Blit(display);
    const u32 black = temp.MapRGB(0, 0, 0);

    while(alpha < riseTo)
    {
	alpha += alpha + 10 < riseTo ? 10 : riseTo - alpha;
	display.Fill(black);
	temp.SetAlpha(alpha);
	display.Blit(temp);
        display.Flip();
	DELAY(10);
    }

    return true;
}

/* get video display */
Display & Display::Get(void)
{
    static Display inside;
    return inside;
}

int Display::GetMaxMode(Size & result, bool rotate)
{
    SDL_Rect** modes = SDL_ListModes(NULL, SDL_ANYFORMAT);

    if(modes == (SDL_Rect **) 0)
    {
        std::cerr <<  "Display::GetMaxMode: " << "no modes available" << std::endl;
	return 0;
    }
    else
    if(modes == (SDL_Rect **) -1)
    {
        //std::cout <<  "Display::GetMaxMode: " << "all modes available" << std::endl;
	return -1;
    }
    else
    {
	int max = 0;
	int cur = 0;

	for(int ii = 0; modes[ii]; ++ii)
	{
	    if(max < modes[ii]->w * modes[ii]->h)
	    {
		max = modes[ii]->w * modes[ii]->h;
		cur = ii;
	    }
	}

	result.w = modes[cur]->w;
	result.h = modes[cur]->h;

	if(rotate && result.w < result.h)
	{
	    cur = result.w;
	    result.w = result.h;
	    result.h = cur;
	}
    }
    return 1;
}

void Display::AddUpdateRect(s16 px, s16 py, u16 pw, u16 ph)
{
    if(0 == (surface->flags & SDL_HWSURFACE))
	update_rects.PushRect(px, py, pw, ph);
}

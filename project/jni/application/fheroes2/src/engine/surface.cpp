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

#include <algorithm>
#include <iostream>
#include <cstring>
#include "surface.h"
#include "palette.h"
#include "error.h"
#include "localevent.h"
#include "display.h"

#ifdef WITH_TTF
#include "SDL_ttf.h"
#endif

#ifdef WITH_IMAGE
#include "SDL_image.h"
#include "IMG_savepng.h"
#endif

static u8 default_depth = 16;
static SDL_Color* pal_colors = NULL;
#define pal_ncolors 255

void SDLFreeSurface(SDL_Surface *sf)
{
    if(sf)
    {
	// clear static palette
	if(sf->format && 8 == sf->format->BitsPerPixel && sf->format->palette && pal_colors == sf->format->palette->colors)
        {
    	    sf->format->palette->colors = NULL;
            sf->format->palette->ncolors = 0;
        }
	SDL_FreeSurface(sf);
    }
}

Surface::Surface() : surface(NULL)
{
}

Surface::Surface(const void* pixels, unsigned int width, unsigned int height, unsigned char bytes_per_pixel, bool alpha) : surface(NULL)
{
    Set(pixels, width, height, bytes_per_pixel, alpha);
}

Surface::Surface(u16 sw, u16 sh, u8 depth, u32 fl) : surface(NULL)
{
    Set(sw, sh, depth, fl);
}

Surface::Surface(u16 sw, u16 sh, bool alpha) : surface(NULL)
{
    Set(sw, sh, alpha);
}

Surface::Surface(const Surface & bs) : surface(NULL)
{
    Set(bs);
}

Surface::Surface(SDL_Surface* sf) : surface(NULL)
{
    Set(sf);
}

Surface::~Surface()
{
    if(! isDisplay())
	FreeSurface(*this);
}

/* operator = */
Surface & Surface::operator= (const Surface & bs)
{
    Set(bs);

    return *this;
}

void Surface::SetDefaultDepth(u8 depth)
{
    switch(depth)
    {
	case 8:
	case 16:
	case 24:
	case 32:
	    default_depth = depth;
	    break;

	default:
	    break;
    }
}

u8 Surface::GetDefaultDepth(void)
{
    return default_depth;
}

void Surface::Set(SDL_Surface* sf)
{
    FreeSurface(*this);

    surface = sf ? sf : NULL;
    LoadPalette();
}

void Surface::Set(const Surface & bs)
{
    FreeSurface(*this);

    if(bs.surface)
    {
	    surface = SDL_ConvertSurface(bs.surface, bs.surface->format, bs.surface->flags);
	    if(!surface) std::cerr << "Surface: copy constructor, error: " << SDL_GetError() << std::endl;
    }
}

void Surface::Set(u16 sw, u16 sh, bool alpha)
{
    FreeSurface(*this);

    CreateSurface(sw, sh, default_depth, 8 < default_depth && alpha ? SDL_SRCALPHA|SDL_SWSURFACE : SDL_SWSURFACE);
    LoadPalette();
}

void Surface::Set(u16 sw, u16 sh, u8 depth, u32 fl)
{
    FreeSurface(*this);

    CreateSurface(sw, sh, depth,  fl);
    LoadPalette();
}

void Surface::Set(const void* pixels, unsigned int width, unsigned int height, unsigned char bytes_per_pixel, bool alpha)
{
    FreeSurface(*this);

    switch(bytes_per_pixel)
    {
	case 1:
	    Set(width, height, 8, SDL_SWSURFACE);
	    Lock();
	    memcpy(surface->pixels, pixels, width * height);
	    Unlock();
	    break;

	default:
	{
	    u32 rmask = 0;
	    u32 gmask = 0;
	    u32 bmask = 0;
	    u32 amask = 0;

	    switch(bytes_per_pixel)
	    {
		case 4:
		    rmask = RMASK32;
		    gmask = GMASK32;
		    bmask = BMASK32;
		    amask = alpha ? AMASK32 : 0;
		    break;
		case 3:
		    rmask = alpha ? RMASK24 : RMASK32;
		    gmask = alpha ? GMASK24 : GMASK32;
		    bmask = alpha ? BMASK24 : BMASK32;
		    amask = alpha ? AMASK24 : 0;
		    break;
		case 2:
		    rmask = RMASK16;
		    gmask = GMASK16;
		    bmask = BMASK16;
		    amask = alpha ? AMASK16 : 0;
		    break;
		default: break;
	    }

	    surface = SDL_CreateRGBSurfaceFrom(const_cast<void *>(pixels), width, height, 8 * bytes_per_pixel, width * bytes_per_pixel,
		rmask, gmask, bmask, amask);
	}
	break;
    }
}

bool Surface::isDisplay(void) const
{
    return NULL != surface && Display::Get().surface == surface;
}

bool Surface::Load(const char* fn)
{
    FreeSurface(*this);

#ifdef WITH_IMAGE
    if(fn) surface = IMG_Load(fn);
#else
    if(fn) surface = SDL_LoadBMP(fn);
#endif
    return surface;
}

bool Surface::Load(const std::string & str)
{
    return Load(str.c_str());
}

bool Surface::Save(const char *fn) const
{
#ifdef WITH_IMAGE
    return !surface || !fn || IMG_SavePNG(fn, surface, -1) ? false : true;
#else
    return !surface || !fn || SDL_SaveBMP(surface, fn) ? false : true;
#endif
}

bool Surface::Save(const std::string & str) const
{
    return Save(str.c_str());
}

u16 Surface::w(void) const
{
    return surface ? surface->w : 0;
}

u16 Surface::h(void) const
{
    return surface ? surface->h : 0;
}

u8 Surface::depth(void) const
{
    return surface ? surface->format->BitsPerPixel : 0;
}

bool Surface::isAlpha(void) const
{
    return SDL_SRCALPHA & surface->flags;
}

u8 Surface::GetAlpha(void) const
{
#if SDL_VERSION_ATLEAST(1, 3, 0)
    u8 alpha = 0;
    if(surface) SDL_GetSurfaceAlphaMod(surface, &alpha);
    return alpha;
#else
    return surface->format->alpha;
#endif
}

u32 Surface::MapRGB(u8 r, u8 g, u8 b, u8 a) const
{
    return (SDL_SRCALPHA & surface->flags) ? SDL_MapRGBA(surface->format, r, g, b, a) : SDL_MapRGB(surface->format, r, g, b);
}

void Surface::GetRGB(u32 pixel, u8 *r, u8 *g, u8 *b, u8 *a) const
{
    return (SDL_SRCALPHA & surface->flags && a) ? SDL_GetRGBA(pixel, surface->format, r, g, b, a) : SDL_GetRGB(pixel, surface->format, r, g, b);
}

/* create new surface */
void Surface::CreateSurface(u16 sw, u16 sh, u8 dp, u32 fl)
{
    switch(dp)
    {
	case 32:
	    surface = SDL_CreateRGBSurface(fl, sw, sh, dp, RMASK32, GMASK32, BMASK32, (SDL_SRCALPHA & fl ? AMASK32 : 0));
	    break;
	case 24:
	    surface = SDL_CreateRGBSurface(fl, sw, sh, dp, RMASK24, GMASK24, BMASK24, (SDL_SRCALPHA & fl ? AMASK24 : 0));
	    break;
	case 16:
	    surface = SDL_CreateRGBSurface(fl, sw, sh, dp, RMASK16, GMASK16, BMASK16, (SDL_SRCALPHA & fl ? AMASK16 : 0));
	    break;
	default:
	    surface = SDL_CreateRGBSurface(fl, sw, sh, dp, 0, 0, 0, 0);
	    break;
    }

    if(!surface)
    {
	std::cerr << "w: " << sw << ", h: " << sh << std::endl;
	Error::Except("Surface::CreateSurface: empty surface, error:", SDL_GetError());
    }
}

void Surface::LoadPalette(void)
{
    // only 8bit color
    // load static palette (economize 1kb for each surface)
    if(surface && 8 == surface->format->BitsPerPixel)
    {
	if(!pal_colors)
	{
	    pal_colors = Palette::Get().SDLPalette()->colors;
	}

	if(surface->format->palette)
	{
    	    if(surface->format->palette->colors && pal_colors != surface->format->palette->colors) SDL_free(surface->format->palette->colors);
    	    surface->format->palette->colors = pal_colors;
    	    surface->format->palette->ncolors = pal_ncolors;
	}
    }
}

/* format surface */
void Surface::SetDisplayFormat(void)
{
    SDL_Surface *osurface = surface;
    surface = SDL_DisplayFormatAlpha(osurface);
    if(osurface) SDLFreeSurface(osurface);
}

u32 Surface::GetColor(u16 index) const
{
    if(! surface) return 0;

    return 8 == surface->format->BitsPerPixel ? index : Palette::Get().GetColor(index);
}

u32 Surface::GetColorKey(void) const
{
    if(! surface) return 0;
    return SDL_MapRGBA(surface->format, 0xFF, 0x00, 0xFF, 0);
}

/* set color key */
void Surface::SetColorKey(void)
{
    if(surface)
    {
	const u32 clkey = GetColorKey();
	Fill(clkey);
	SetColorKey(clkey);
    }
}

void Surface::SetColorKey(u8 r, u8 g, u8 b)
{
#ifdef _WIN32_WCE
    SDL_SetColorKey(surface, SDL_SRCCOLORKEY, MapRGB(r, g, b));
#else
    SDL_SetColorKey(surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, MapRGB(r, g, b));
#endif
}

void Surface::SetColorKey(u32 color)
{
#ifdef _WIN32_WCE
    SDL_SetColorKey(surface, SDL_SRCCOLORKEY, color);
#else
    SDL_SetColorKey(surface, SDL_SRCCOLORKEY | SDL_RLEACCEL, color);
#endif
}

/* draw u32 pixel */
void Surface::SetPixel4(u16 x, u16 y, u32 color)
{
    if(x > surface->w || y > surface->h) return;
    
    u32 *bufp = static_cast<u32 *>(surface->pixels) + y * surface->pitch / 4 + x;

    *bufp = color;
}

/* draw u24 pixel */
void Surface::SetPixel3(u16 x, u16 y, u32 color)
{
    if(x > surface->w || y > surface->h) return;

    u8 *bufp = static_cast<u8 *>(surface->pixels) + y * surface->pitch + x * 3; 

    if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
    {
        bufp[0] = color;
        bufp[1] = color >> 8;
        bufp[2] = color >> 16;
    }
    else
    { 
	bufp[2] = color;
	bufp[1] = color >> 8;
        bufp[0] = color >> 16;
    }
}

/* draw u16 pixel */
void Surface::SetPixel2(u16 x, u16 y, u32 color)
{
    if(x > surface->w || y > surface->h) return;
    
    u16 *bufp = static_cast<u16 *>(surface->pixels) + y * surface->pitch / 2 + x;

    *bufp = static_cast<u16>(color);
}

/* draw u8 pixel */
void Surface::SetPixel1(u16 x, u16 y, u32 color)
{
    if(x > surface->w || y > surface->h) return;

    u8 *bufp = static_cast<u8 *>(surface->pixels) + y * surface->pitch + x;

    *bufp = static_cast<u8>(color);
}

/* draw pixel */
void Surface::SetPixel(u16 x, u16 y, u32 color)
{
    switch(surface->format->BytesPerPixel)
    {
	case 1:	SetPixel1(x, y, color);	break;
	case 2:	SetPixel2(x, y, color);	break;
	case 3:	SetPixel3(x, y, color);	break;
	case 4:	SetPixel4(x, y, color);	break;
	default: break;
    }
    if(isDisplay()) Display::Get().AddUpdateRect(x, y, 1, 1);
}

u32 Surface::GetPixel4(u16 x, u16 y) const
{
    if(x > surface->w || y > surface->h) return 0;
    
    u32 *bufp = static_cast<u32 *>(surface->pixels) + y * surface->pitch / 4 + x;

    return *bufp;
}

u32 Surface::GetPixel3(u16 x, u16 y) const
{
    if(x > surface->w || y > surface->h) return 0;

    u8 *bufp = static_cast<u8 *>(surface->pixels) + y * surface->pitch + x * 3; 

    u32 color = 0;

    if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
    {
        color |= bufp[2];
        color <<= 8;
        color |= bufp[1];
        color <<= 8;
        color |= bufp[0];
    }
    else
    { 
        color |= bufp[0];
        color <<= 8;
        color |= bufp[1];
        color <<= 8;
        color |= bufp[2];
    }
    
    return color;
}

u32 Surface::GetPixel2(u16 x, u16 y) const
{
    if(x > surface->w || y > surface->h) return 0;
    u16 *bufp = static_cast<u16 *>(surface->pixels) + y * surface->pitch / 2 + x;

    return static_cast<u32>(*bufp);
}

u32 Surface::GetPixel1(u16 x, u16 y) const
{
    if(x > surface->w || y > surface->h) return 0;
    u8 *bufp = static_cast<u8 *>(surface->pixels) + y * surface->pitch + x;

    return static_cast<u32>(*bufp);
}

u32 Surface::GetPixel(u16 x, u16 y) const
{
    switch(surface->format->BytesPerPixel)
    {
	case 1:	return GetPixel1(x, y);
	case 2:	return GetPixel2(x, y);
	case 3:	return GetPixel3(x, y);
	case 4:	return GetPixel4(x, y);
	default: break;
    }
    
    return 0;
}

/* fill colors surface */
void Surface::Fill(u32 color)
{
    SDL_Rect dstrect = {0, 0, surface->w, surface->h};

    SDL_FillRect(surface, &dstrect, color);
    if(isDisplay()) Display::Get().AddUpdateRect(0, 0, surface->w, surface->h);
}

/* rect fill colors surface */
void Surface::FillRect(u32 color, const Rect & rect)
{
    SDL_Rect dstrect = {rect.x, rect.y, rect.w, rect.h};
    SDL_FillRect(surface, &dstrect, color);
    if(isDisplay()) Display::Get().AddUpdateRect(rect.x, rect.y, rect.w, rect.h);
}

/* blit */
void Surface::Blit(const Surface &src)
{
    SDL_BlitSurface(src.surface, NULL, surface, NULL);
    if(isDisplay()) Display::Get().AddUpdateRect(0, 0, src.w(), src.h());
}

/* blit */
void Surface::Blit(const Surface &src, s16 dst_ox, s16 dst_oy)
{
    SDL_Rect dstrect = {dst_ox, dst_oy, src.surface->w, src.surface->h};

    SDL_BlitSurface(src.surface, NULL, surface, &dstrect);
    if(isDisplay()) Display::Get().AddUpdateRect(dst_ox, dst_oy, src.surface->w, src.surface->h);
}

/* blit */
void Surface::Blit(const Surface &src, const Rect &src_rt, s16 dst_ox, s16 dst_oy)
{
    SDL_Rect dstrect = {dst_ox, dst_oy, src_rt.w, src_rt.h};
    SDL_Rect srcrect = {src_rt.x, src_rt.y, src_rt.w, src_rt.h};

    SDL_BlitSurface(src.surface, &srcrect, surface, &dstrect);
    if(isDisplay()) Display::Get().AddUpdateRect(dst_ox, dst_oy, src_rt.w, src_rt.h);
}

void Surface::Blit(const Surface &src, const Point &dst_pt)
{
    Blit(src, dst_pt.x, dst_pt.y);
}

void Surface::Blit(const Surface &src, const Rect &src_rt, const Point &dst_pt)
{
    Blit(src, src_rt, dst_pt.x, dst_pt.y);
}

void Surface::SetAlpha(u8 level)
{
#if SDL_VERSION_ATLEAST(1, 3, 0)
    if(surface)
	SDL_SetSurfaceAlphaMod(surface, level);
#else
    if(surface)
	SDL_SetAlpha(surface, SDL_SRCALPHA, level);
#endif
}

void Surface::ResetAlpha(void)
{
    if(!surface) return;
    SDL_SetAlpha(surface, 0, 255);
}

void Surface::Lock(void) const
{
    if(SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
}

void Surface::Unlock(void) const
{
    if(SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
}

void Surface::FreeSurface(Surface & sf)
{
    if(sf.surface)
    {
	SDLFreeSurface(sf.surface);
	sf.surface = NULL;
    }
}

const SDL_PixelFormat *Surface::GetPixelFormat(void) const
{
    return surface ? surface->format : NULL;
}

void Surface::ChangeColorIndex(u32 fc, u32 tc)
{
    if(!surface) return;

    if(8 != depth()) return ChangeColor(GetColor(fc), GetColor(tc));

    Lock();
    if(fc != tc)
    for(u16 y = 0; y < surface->h; ++y)
	for(u16 x = 0; x < surface->w; ++x)
	    if(fc == GetPixel(x, y)) SetPixel(x, y, tc);
    Unlock();
}

void Surface::ChangeColor(u32 fc, u32 tc)
{
    if(!surface) return;

    Lock();
    if(fc != tc)
    for(u16 y = 0; y < surface->h; ++y)
	for(u16 x = 0; x < surface->w; ++x)
	    if(fc == GetPixel(x, y)) SetPixel(x, y, tc);
    Unlock();
}

void Surface::GrayScale(void)
{
    if(!surface) return;

    u8 a, r, g, b, z;

    const u32 colkey = GetColorKey();
    u32 color = 0;

    Lock();
    for(u16 y = 0; y < surface->h; ++y)
	for(u16 x = 0; x < surface->w; ++x)
    {
	color = GetPixel(x, y);
	if(color == colkey) continue;
	GetRGB(color, &r, &g, &b, &a);
	z = static_cast<u8>(0.299 * r + 0.587 * g + 0.114 * b);
	r = z;
	g = z;
	b = z;
	SetPixel(x, y, MapRGB(r, g, b, a));
    }
    Unlock();
}

void Surface::Sepia(void)
{
    if(!surface) return;

    Lock();
    for(u16 x = 0; x < surface->w; x++)
        for(u16 y = 0; y < surface->h; y++)
        {
            u32 pixel = GetPixel(x, y);
            u8 r, g, b;
            GetRGB(pixel, &r, &g, &b);
    
            //Numbers derived from http://blogs.techrepublic.com.com/howdoi/?p=120
            #define CLAMP255(val) static_cast<u8>(std::min<u16>((val), 255))
            u8 outR = CLAMP255(static_cast<u16>(r * 0.693f + g * 0.769f + b * 0.189f));
            u8 outG = CLAMP255(static_cast<u16>(r * 0.449f + g * 0.686f + b * 0.168f));
            u8 outB = CLAMP255(static_cast<u16>(r * 0.272f + g * 0.534f + b * 0.131f));
            pixel = MapRGB(outR, outG, outB);
            SetPixel(x, y, pixel);
            #undef CLAMP255
        }
    Unlock();
}

void Surface::DrawLine(const Point & p1, const Point & p2, u32 c)
{
    DrawLine(p1.x, p1.y, p2.x, p2.y, c);
}

void Surface::DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u32 c)
{
    const u16 dx = std::abs(x2 - x1);
    const u16 dy = std::abs(y2 - y1);

    Lock();
    if(dx > dy)
    {
	s16 ns = std::div(dx, 2).quot;

	for(u16 i = 0; i <= dx; ++i)
	{
	    SetPixel(x1, y1, c);
	    x1 < x2 ? ++x1 : --x1;
	    ns -= dy;
	    if(ns < 0)
	    {
		y1 < y2 ? ++y1 : --y1;
		ns += dx;
	    }
	}
    }
    else
    {
	s16 ns = std::div(dy, 2).quot;

	for(u16 i = 0; i <= dy; ++i)
	{
	    SetPixel(x1, y1, c);
	    y1 < y2 ? ++y1 : --y1;
	    ns -= dx;
	    if(ns < 0)
	    {
		x1 < x2 ? ++x1 : --x1;
		ns += dy;
	    }
	}
    }
    Unlock();
}

void Surface::MakeStencil(Surface & dst, const Surface & src, u32 col)
{
    if(!src.surface) return;

    dst.Set(src.surface->w, src.surface->h);
    dst.SetColorKey();
    const u32 clkey = src.GetColorKey();
    u8 r, g, b, a;

    src.Lock();
    dst.Lock();
    for(u16 y = 0; y < src.surface->h; ++y)
        for(u16 x = 0; x < src.surface->w; ++x)
        {
            u32 pixel = src.GetPixel(x, y);
            if(clkey != pixel)
	    {
		if(src.isAlpha())
		{
		    src.GetRGB(pixel, &r, &g, &b, &a);
		    // skip shadow
		    if(a < 200) continue;
		}

                dst.SetPixel(x, y, col);
            }
        }
    dst.Unlock();
    src.Unlock();
}

void Surface::MakeContour(Surface & dst, const Surface & src, u32 col)
{
    if(!src.surface) return;

    dst.Set(src.surface->w + 2, src.surface->h + 2);
    dst.SetColorKey();

    Surface trf;
    u32 fake = src.MapRGB(0x00, 0xFF, 0xFF);

    MakeStencil(trf, src, fake);
    const u32 clkey = trf.GetColorKey();
    trf.Lock();
    dst.Lock();
    for(u16 y = 0; y < trf.h(); ++y)
        for(u16 x = 0; x < trf.w(); ++x)
        {
            if(fake == trf.GetPixel(x, y))
            {
                if(0 == x) dst.SetPixel(x, y, col);
                else if(trf.w() - 1 == x) dst.SetPixel(x + 1, y, col);
                else if(0 == y) dst.SetPixel(x, y, col);
                else if(trf.h() - 1 == y) dst.SetPixel(x, y + 1, col);
                else {
                    if(0 < x && clkey == trf.GetPixel(x - 1, y)) dst.SetPixel(x - 1, y, col);
                    if(trf.w() - 1 > x && clkey == trf.GetPixel(x + 1, y)) dst.SetPixel(x + 1, y, col);

                    if(0 < y && clkey == trf.GetPixel(x, y - 1)) dst.SetPixel(x, y - 1, col);
                    if(trf.h() - 1 > y && clkey == trf.GetPixel(x, y + 1)) dst.SetPixel(x, y + 1, col);
                }
            }
        }
    trf.Unlock();
    dst.Unlock();
}

void Surface::TILReflect(Surface & sf_dst, const Surface & sf_src, const u8 shape)
{
    // valid sf_src
    if(!sf_src.surface || sf_src.w() != sf_src.h())
    {
	std::cerr << "Surface::TILReflect: " << "incorrect size" << std::endl;
	return;
    }

    if(sf_src.depth() != 8)
    {
	std::cerr << "Surface::TILReflect: " << "incorrect depth, use only 8 bpp" << std::endl;
	return;
    }

    const u8 tile_width = sf_src.w();
    const u8 tile_height = sf_src.h();

    // valid sf_dst
    if(!sf_dst.surface || sf_dst.w() != tile_width || sf_dst.h() != tile_height)
    {
        sf_dst = Surface(tile_width, tile_height, 8, SWSURFACE);
    }

    const char* src = static_cast<const char*>(sf_src.surface->pixels);
    char* dst = static_cast<char*>(sf_dst.surface->pixels);

    s16 x, y;

    char * dst2 = NULL;

    sf_dst.Lock();

    // draw tiles
    switch(shape % 4)
    {
        // normal
	case 0:
	    std::memcpy(dst, src, tile_width * tile_height);
            break;

        // vertical reflect
        case 1:
	{
	    dst2 = dst + tile_width * (tile_height - 1);

	    for(int i = 0; i < tile_height; i++)
	    {
    		memcpy(dst2, src, tile_width);

    		src += tile_width;
    		dst2 -= tile_width;
	    }
	}
            break;

        // horizontal reflect
        case 2:
            for(y = 0; y < tile_height; ++y)
                for(x = tile_width - 1; x >= 0; --x)
                {
		    dst2 = dst + y * tile_width + x;
		    *dst2 = *src;
                    ++src;
                }
            break;

        // any variant
        case 3:
            for(y = tile_height - 1; y >= 0; --y)
                for( x = tile_width - 1; x >= 0; --x)
                {
		    dst2 = dst + y * tile_width + x;
		    *dst2 = *src;
                    ++src;
                }
            break;
    }

    sf_dst.Unlock();
}

u32 Surface::GetSize(void) const
{
    u32 res = 0;

    if(surface)
    {
	res = sizeof(SDL_Surface) + sizeof(SDL_PixelFormat) + surface->pitch * surface->h;

	if(surface->format->palette) res += sizeof(SDL_Palette) + surface->format->palette->ncolors * sizeof(SDL_Color);
    }

        return res;
}

u32 AVERAGE(SDL_PixelFormat* fm, u32 c1, u32 c2)
{
    if(c1 == c2) return c1;
    if(c1 == SDL_MapRGBA(fm, 0xFF, 0x00, 0xFF, 0)) c1 = 0;
    if(c2 == SDL_MapRGBA(fm, 0xFF, 0x00, 0xFF, 0)) c2 = 0;

#define avr(a, b) ((a + b) >> 1)
    u8 r1, g1, b1, a1;
    SDL_GetRGBA(c1, fm, &r1, &g1, &b1, &a1);
    u8 r2, g2, b2, a2;
    SDL_GetRGBA(c2, fm, &r2, &g2, &b2, &a2);
    return SDL_MapRGBA(fm, avr(r1, r2), avr(g1, g2), avr(b1, b2), avr(a1, a2));
}

/* scale surface */
void Surface::ScaleMinifyByTwo(Surface & sf_dst, const Surface & sf_src, bool event)
{
    if(!sf_src.isValid()) { std::cerr << "Surface::ScaleMinifyByTwo: " << "invalid surface" << std::endl; return; };
    u16 x, y, x2, y2;

    u8 mul = 2;
    u16 w = sf_src.w() / mul;
    u16 h = sf_src.h() / mul;

    if(2 > w || 2 > h){ std::cerr << "Surface::ScaleMinifyByTwo: " << "small size" << std::endl; return; };

    sf_dst.Set(w, h, sf_src.depth(), SWSURFACE);
    sf_dst.SetColorKey();
    sf_dst.Lock();
    sf_src.Lock();
    for(y = 0; y < h; y++)
    {
       y2 = mul * y;
       for(x = 0; x < w; x++)
       {
	    x2 = mul * x;
	    const u32 & p = AVERAGE(sf_src.surface->format, sf_src.GetPixel(x2, y2), sf_src.GetPixel(x2 + 1, y2));
	    const u32 & q = AVERAGE(sf_src.surface->format, sf_src.GetPixel(x2, y2 + 1), sf_src.GetPixel(x2 + 1, y2 + 1));
	    sf_dst.SetPixel(x, y, AVERAGE(sf_src.surface->format, p, q));
	    if(event) LocalEvent::Get().HandleEvents(false);
       }
    }
    sf_src.Unlock();
    sf_dst.Unlock();
}

void Surface::Swap(Surface & sf1, Surface & sf2)
{
    std::swap(sf1.surface, sf2.surface);
}

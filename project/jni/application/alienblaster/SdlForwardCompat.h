/*
	Compatibility wrapper to compile the same code on both SDL 1.2 and 1.3 without many #ifdefs
*/

#ifndef __SDL_FORWARD_COMPAT_H__
#define __SDL_FORWARD_COMPAT_H__

#include <SDL.h>
#include <SDL_video.h>
#include <SDL_version.h>

#ifndef __cplusplus
#error "This header is for C++ only, you're unlucky, sorry"
#endif

#if SDL_VERSION_ATLEAST(1,3,0)

struct SdlCompat_AcceleratedSurface
{
	SDL_Texture * t;
	SDL_Surface * s;
	int w, h;
	int flags;
	SDL_PixelFormat * format;
};

static inline SdlCompat_AcceleratedSurface * SdlCompat_CreateAcceleratedSurface(SDL_Surface * surface)
{
	SdlCompat_AcceleratedSurface * ret = new SdlCompat_AcceleratedSurface();
	// Allocate accelerated surface even if that means loss of color quality
	Uint32 format;

	format = SDL_PIXELFORMAT_RGB565;

	if( surface->flags & SDL_SRCCOLORKEY )
		format = SDL_PIXELFORMAT_RGBA5551;
	ret->s = SDL_ConvertSurface( surface, surface->format, 0 );
	ret->t = SDL_CreateTextureFromSurface(format, ret->s);

	ret->w = surface->w;
	ret->h = surface->h;
	ret->format = new SDL_PixelFormat();
	*(ret->format) = *(surface->format);
	
	if( ! ret->t )
	{
		SDL_SetError("SdlCompat_CreateAcceleratedSurface: Cannot allocate HW texture, W %d H %d format %x surface->flags %x", ret->w, ret->h, format, surface->flags );
		return ret;
	}

	ret->flags = surface->flags;
	if( surface->flags & SDL_SRCALPHA )
	{
		SDL_SetTextureBlendMode( ret->t, SDL_BLENDMODE_BLEND );
		Uint8 alpha = 128;
		if( SDL_GetSurfaceAlphaMod( surface, &alpha ) < 0 )
			alpha = 128;
		SDL_SetTextureAlphaMod( ret->t, alpha );
	}
	
	return ret;
};

static inline int SDL_BlitSurface( SdlCompat_AcceleratedSurface * src, SDL_Rect * srcR, SdlCompat_AcceleratedSurface * unused, SDL_Rect * destR )
{
	return SDL_RenderCopy(src->t, srcR, destR);
};

static inline void SDL_FreeSurface(SdlCompat_AcceleratedSurface * surface)
{
	SDL_DestroyTexture(surface->t);
	SDL_FreeSurface(surface->s);
	delete surface->format;
	delete surface;
};

static inline void SDL_FillRect( SdlCompat_AcceleratedSurface * unused, const SDL_Rect* rect, Uint32 color )
{
	Uint8 r, g, b, a;
	SDL_GetRGBA( color, SDL_GetVideoSurface()->format, &r, &g, &b, &a );
	SDL_SetRenderDrawColor(r, g, b, SDL_ALPHA_OPAQUE /* a */);
	SDL_RenderFillRect(rect);
};

static inline int SDL_Flip(SdlCompat_AcceleratedSurface * unused)
{
	SDL_RenderPresent();
	return 0;
};

static inline int SDL_SetAlpha(SdlCompat_AcceleratedSurface * surface, Uint32 flag, Uint8 alpha)
{
	if( ! (flag & SDL_SRCALPHA) )
		alpha = SDL_ALPHA_OPAQUE;
	return SDL_SetTextureAlphaMod(surface->t, alpha);
};

static inline void SdlCompat_ReloadSurfaceToVideoMemory(SdlCompat_AcceleratedSurface * surface)
{
	SDL_DestroyTexture(surface->t);
	
	// Allocate accelerated surface even if that means loss of color quality
	Uint32 format;
	format = SDL_PIXELFORMAT_RGB565;

	if( surface->flags & SDL_SRCCOLORKEY )
		format = SDL_PIXELFORMAT_RGBA5551;
	surface->t = SDL_CreateTextureFromSurface(format, surface->s);
	
	if( ! surface->t )
	{
		SDL_SetError("SdlCompat_CreateAcceleratedSurface: Cannot allocate HW texture, W %d H %d format %x surface->flags %x", surface->w, surface->h, format, surface->flags );
		return;
	}

	if( surface->flags & SDL_SRCALPHA )
	{
		SDL_SetTextureBlendMode( surface->t, SDL_BLENDMODE_BLEND );
		Uint8 alpha = 128;
		if( SDL_GetSurfaceAlphaMod( surface->s, &alpha ) < 0 )
			alpha = 128;
		SDL_SetTextureAlphaMod( surface->t, alpha );
	}
};

#else

typedef SDL_Surface SdlCompat_AcceleratedSurface;

static inline SdlCompat_AcceleratedSurface * SdlCompat_CreateAcceleratedSurface(SDL_Surface * surface)
{
	return SDL_ConvertSurface(surface, surface->format, surface->flags | SDL_HWSURFACE);
};

static inline void SdlCompat_ReloadSurfaceToVideoMemory(SDL_Surface * surface)
{
};

#endif

#endif


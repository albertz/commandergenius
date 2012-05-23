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
	int w, h;
	SDL_PixelFormat * format;
};

enum { SDL_SRCALPHA = 8, SDL_SRCCOLORKEY = 16 }; // Some dummy non-zero values

typedef SDL_Keycode SDLKey;

extern SDL_Renderer * SDL_global_renderer;

static inline SdlCompat_AcceleratedSurface * SdlCompat_CreateAcceleratedSurface(SDL_Surface * surface)
{
	SdlCompat_AcceleratedSurface * ret = new SdlCompat_AcceleratedSurface();
	// Allocate accelerated surface even if that means loss of color quality
	Uint32 format;
	Uint32 colorkey;
	Uint8 alpha;

	ret->w = surface->w;
	ret->h = surface->h;
	ret->format = new SDL_PixelFormat();
	memcpy(ret->format, surface->format, sizeof(SDL_PixelFormat));

	format = SDL_PIXELFORMAT_RGB565;
	if( SDL_GetColorKey(surface, &colorkey) == 0 )
	{
		format = SDL_PIXELFORMAT_RGBA4444;
	}

	ret->t = SDL_CreateTextureFromSurface(SDL_global_renderer, surface);
	
	if( ! ret->t )
	{
		SDL_SetError("SdlCompat_CreateAcceleratedSurface: Cannot allocate HW texture, W %d H %d format %x surface->flags %x", ret->w, ret->h, format, surface->flags );
		return ret;
	}

	SDL_SetTextureBlendMode( ret->t, SDL_BLENDMODE_BLEND );
	//SDL_SetTextureAlphaMod( ret->t, SDL_ALPHA_OPAQUE );
	SDL_SetTextureAlphaMod( ret->t, 128 );
	if( SDL_GetSurfaceAlphaMod(surface, &alpha) == 0 )
		SDL_SetTextureAlphaMod( ret->t, alpha );

	return ret;
};

static inline int SDL_BlitSurface( SdlCompat_AcceleratedSurface * src, SDL_Rect * srcR, SdlCompat_AcceleratedSurface * unused, SDL_Rect * destR )
{
	return SDL_RenderCopy(SDL_global_renderer, src->t, srcR, destR);
};

static inline void SDL_FreeSurface(SdlCompat_AcceleratedSurface * surface)
{
	SDL_DestroyTexture(surface->t);
	delete surface->format;
	delete surface;
};

static inline void SDL_FillRect( SdlCompat_AcceleratedSurface * unused, const SDL_Rect* rect, Uint32 color )
{
	Uint8 r = color & 0xff, g = (color >> 8) & 0xff, b = (color >> 16) & 0xff;
	SDL_SetRenderDrawColor(SDL_global_renderer, r, g, b, SDL_ALPHA_OPAQUE /* a */);
	SDL_RenderFillRect(SDL_global_renderer, rect);
};

static inline int SDL_Flip(SdlCompat_AcceleratedSurface * unused)
{
	SDL_RenderPresent(SDL_global_renderer);
	return 0;
};

static inline int SDL_SetAlpha(SdlCompat_AcceleratedSurface * surface, Uint32 flag, Uint8 alpha)
{
	if( ! flag )
		alpha = SDL_ALPHA_OPAQUE;
	return SDL_SetTextureAlphaMod(surface->t, alpha);
};

static inline void SdlCompat_ReloadSurfaceToVideoMemory(SdlCompat_AcceleratedSurface * surface, SDL_Surface * src)
{
	// Allocate accelerated surface even if that means loss of color quality
	Uint32 format;
	int access, w, h;
	SDL_QueryTexture(surface->t, &format, &access, &w, &h);
	
	int bpp;
	Uint32 r,g,b,a;
	SDL_PixelFormatEnumToMasks(format, &bpp, &r, &g, &b, &a);
	SDL_Surface * formatsurf = SDL_CreateRGBSurface(0, 1, 1, bpp, r, g, b, a);
	SDL_Surface * converted = SDL_ConvertSurface( src, formatsurf->format, 0 );

	SDL_LockSurface(converted);

	SDL_UpdateTexture( surface->t, NULL, converted->pixels, converted->pitch );
	SDL_UnlockSurface(converted);
	
	SDL_FreeSurface(converted);
	SDL_FreeSurface(formatsurf);
	
	if( src->flags & SDL_SRCALPHA )
	{
		SDL_SetTextureBlendMode( surface->t, SDL_BLENDMODE_BLEND );
		Uint8 alpha = 128;
		if( SDL_GetSurfaceAlphaMod( src, &alpha ) < 0 )
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

static inline void SdlCompat_ReloadSurfaceToVideoMemory(SDL_Surface * surface, SDL_Surface * src)
{
};

#endif

#endif


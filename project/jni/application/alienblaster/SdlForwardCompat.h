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

static inline SdlCompat_AcceleratedSurface * SdlCompat_CreateAcceleratedSurface(SDL_Surface * surface)
{
	SdlCompat_AcceleratedSurface * ret = new SdlCompat_AcceleratedSurface();
	// Allocate accelerated surface even if that means loss of color quality
	Uint32 format;

	ret->w = surface->w;
	ret->h = surface->h;
	ret->format = new SDL_PixelFormat();
	*(ret->format) = *(surface->format);

	format = SDL_PIXELFORMAT_RGB565;
	if( surface->flags & SDL_SRCCOLORKEY )
		format = SDL_PIXELFORMAT_RGBA4444;

	ret->t = SDL_CreateTextureFromSurface(format, surface);
	
	if( ! ret->t )
	{
		SDL_SetError("SdlCompat_CreateAcceleratedSurface: Cannot allocate HW texture, W %d H %d format %x surface->flags %x", ret->w, ret->h, format, surface->flags );
		return ret;
	}

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

static inline void SdlCompat_ReloadSurfaceToVideoMemory(SdlCompat_AcceleratedSurface * surface, SDL_Surface * src)
{
	// Allocate accelerated surface even if that means loss of color quality
	Uint32 format;
	format = SDL_PIXELFORMAT_RGB565;

	if( src->flags & SDL_SRCCOLORKEY )
		format = SDL_PIXELFORMAT_RGBA5551;
	//surface->t = SDL_CreateTextureFromSurface(format, src);
	
	int bpp;
	Uint32 r,g,b,a;
	SDL_PixelFormatEnumToMasks(format, &bpp, &r, &g, &b, &a);
	SDL_Surface * formatsurf = SDL_CreateRGBSurface(0, 1, 1, bpp, r, g, b, a);
	SDL_Surface * converted = SDL_ConvertSurface( src, formatsurf->format, 0 );

	SDL_LockSurface(converted);

	// debug
	/*
	for( int x=0; x<converted->w; x++ )
	for( int y=0; y<converted->h; y++ )
		*(Sint16 *) ( ((Uint8 *)converted->pixels) + y*converted->pitch + x*2 ) = y*4;
	*/
	// end debug

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


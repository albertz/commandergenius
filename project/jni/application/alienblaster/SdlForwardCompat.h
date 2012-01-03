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

extern SDL_Renderer * SDL_global_renderer;

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
	SDL_Surface * surface2 = surface;
	if( surface->flags & SDL_SRCCOLORKEY )
	{
		format = SDL_PIXELFORMAT_RGBA4444;
		//surface2 = SDL_ConvertSurfaceFormat(surface, format, 0); // Does not copy alpha
		int bpp;
		Uint32 r,g,b,a;
		SDL_PixelFormatEnumToMasks(format, &bpp, &r, &g, &b, &a);
		surface2 = SDL_CreateRGBSurface(0, surface->w, surface->h, bpp, r, g, b, a);
		SDL_FillRect(surface2, NULL, 0);
		SDL_BlitSurface(surface, NULL, surface2, NULL);
		// Fix the alpha channel, using ugly pixel access
		SDL_LockSurface(surface2);
		SDL_LockSurface(surface);
		for(int y = 0; y < surface->h; y++)
		for(int x = 0; x < surface->w; x++)
		{
			// Assume 24-bit or 32-bit surface
			Uint8 * pixel = ((Uint8 *) surface->pixels) + y*surface->pitch + x*surface->format->BytesPerPixel;
			if( pixel[0] == 255 && pixel[1] == 0 && pixel[2] == 255 )
				*(Uint16 *)(((Uint8 *) surface2->pixels) + y*surface2->pitch + x*surface2->format->BytesPerPixel) = 0;
		}
		SDL_UnlockSurface(surface);
		SDL_UnlockSurface(surface2);
	}

	ret->t = SDL_CreateTextureFromSurface(SDL_global_renderer, surface2);
	
	if(surface != surface2)
		SDL_FreeSurface(surface2);
	
	if( ! ret->t )
	{
		SDL_SetError("SdlCompat_CreateAcceleratedSurface: Cannot allocate HW texture, W %d H %d format %x surface->flags %x", ret->w, ret->h, format, surface->flags );
		return ret;
	}

	SDL_SetTextureBlendMode( ret->t, SDL_BLENDMODE_BLEND );
	//SDL_SetTextureAlphaMod( ret->t, SDL_ALPHA_OPAQUE );
	SDL_SetTextureAlphaMod( ret->t, 128 );
	if( surface->flags & SDL_SRCALPHA )
	{
		Uint8 alpha = 128;
		if( SDL_GetSurfaceAlphaMod( surface, &alpha ) < 0 )
			alpha = 128;
		SDL_SetTextureAlphaMod( ret->t, alpha );
	}

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


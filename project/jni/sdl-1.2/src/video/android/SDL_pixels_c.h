/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
/*
This source code is distibuted under ZLIB license, however when compiling with SDL 1.2,
which is licensed under LGPL, the resulting library, and all it's source code,
falls under "stronger" LGPL terms, so is this file.
If you compile this code with SDL 1.3 or newer, or use in some other way, the license stays ZLIB.
*/

#include "SDL_config.h"
#include "SDL_version.h"

/* Useful functions and variables from SDL_pixel.c */

#include "SDL_blit.h"

/* Pixel format functions */
extern SDL_PixelFormat *SDL_AllocFormat(int bpp,
                                        Uint32 Rmask, Uint32 Gmask,
                                        Uint32 Bmask, Uint32 Amask);
extern SDL_PixelFormat *SDL_InitFormat(SDL_PixelFormat * format, int bpp,
                                       Uint32 Rmask, Uint32 Gmask,
                                       Uint32 Bmask, Uint32 Amask);
extern void SDL_FormatChanged(SDL_Surface * surface);
extern void SDL_FreeFormat(SDL_PixelFormat * format);

/* Blit mapping functions */
extern SDL_BlitMap *SDL_AllocBlitMap(void);
extern void SDL_InvalidateMap(SDL_BlitMap * map);
extern int SDL_MapSurface(SDL_Surface * src, SDL_Surface * dst);
extern void SDL_FreeBlitMap(SDL_BlitMap * map);

/* Miscellaneous functions */
extern int SDL_CalculatePitch(SDL_Surface * surface);
extern void SDL_DitherColors(SDL_Color * colors, int bpp);
extern Uint8 SDL_FindColor(SDL_Palette * pal, Uint8 r, Uint8 g, Uint8 b);
extern void SDL_ApplyGamma(Uint16 * gamma, SDL_Color * colors,
                           SDL_Color * output, int ncolors);
/* vi: set ts=4 sw=4 expandtab: */

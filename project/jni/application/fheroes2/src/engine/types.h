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

#ifndef H2TYPES_H
#define H2TYPES_H

#include "SDL.h"

typedef Sint8		s8;
typedef Uint8		u8;
typedef Sint16		s16;
typedef Uint16		u16;
typedef Sint32		s32;
typedef Uint32		u32;

typedef SDL_Color Colors;

#define MAXU16   0xFFFF
#define MAXU32   0xFFFFFFFF

#if defined __SYMBIAN32__
#define MKDIR(X)    mkdir(X, S_IRWXU)
#define SEPARATOR       '\\'
#elif defined __WIN32__
#include <io.h>
#define MKDIR(X)    mkdir(X)
#define SEPARATOR       '\\'
#else
#define MKDIR(X)    mkdir(X, S_IRWXU)
#define SEPARATOR       '/'
#endif

#define DELAY(X)	SDL_Delay(X)

#if SDL_BYTEORDER == SDL_BIG_ENDIAN

#define RMASK16			0x0000f000
#define GMASK16			0x00000f00
#define BMASK16			0x000000f0
#define AMASK16			0x0000000f

#define RMASK24			0x00fc0000
#define GMASK24			0x0003f000
#define BMASK24			0x00000fc0
#define AMASK24			0x0000003f

#define RMASK32			0xff000000
#define GMASK32			0x00ff0000
#define BMASK32			0x0000ff00
#define AMASK32			0x000000ff

#else

#define RMASK16			0x0000000f
#define GMASK16			0x000000f0
#define BMASK16			0x00000f00
#define AMASK16			0x0000f000

#define RMASK24			0x0000003f
#define GMASK24			0x00000fc0
#define BMASK24			0x0003f000
#define AMASK24			0x00fc0000

#define RMASK32			0x000000ff
#define GMASK32			0x0000ff00
#define BMASK32			0x00ff0000
#define AMASK32			0xff000000

#endif

#define Swap16(X)	X=SDL_Swap16(X)
#define Swap32(X)	X=SDL_Swap32(X)
#define SwapLE16(X)	X=SDL_SwapLE16(X)
#define SwapLE32(X)	X=SDL_SwapLE32(X)
#define SwapBE16(X)	X=SDL_SwapBE16(X)
#define SwapBE32(X)	X=SDL_SwapBE32(X)

u32 ReadBE32(const u8 *p);
u32 ReadLE32(const u8 *p);
u16 ReadBE16(const u8 *p);
u16 ReadLE16(const u8 *p);

void WriteBE32(u8 *p, u32 x);
void WriteBE16(u8 *p, u16 x);
void WriteLE32(u8 *p, u32 x);
void WriteLE16(u8 *p, u16 x);

#if defined __SYMBIAN32__
#define PATH_MAX FILENAME_MAX
namespace std
{
    int c_abs(int x);
    float c_abs(float x);
    double c_abs(double x);
    int c_isspace(char c);

#define isspace(c) c_isspace(c)
#define abs(x) c_abs(x)
}
using namespace std;
#endif

#if defined __MINGW32CE__
#include <cstdlib>
#ifndef PATH_MAX
#define PATH_MAX 255
#endif
#define setlocale(x,y) 0
#define system(x) 0
#define putenv(x) SDL_putenv(x)
#define getenv(x) SDL_getenv(x)
#define getopt(x, y, z) -1
#define optarg 0
#else
#include <cstdlib>
#endif

#endif

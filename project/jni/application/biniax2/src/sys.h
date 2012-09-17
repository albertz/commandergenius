/******************************************************************************
BINIAX SYSTEM-RELATED DEFINITIONS
COPYRIGHT JORDAN TUZSUZOV, (C) 2005-2009

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

LICENSE ORIGIN : http://www.gzip.org/zlib/zlib_license.html

For complete product license refer to LICENSE.TXT file

******************************************************************************/

#ifndef _BNX_SYS_H
#define _BNX_SYS_H

/******************************************************************************
INCLUDES
******************************************************************************/

#include "inc.h"

/******************************************************************************
MACRO
******************************************************************************/

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define NORM16(X)    (X)
#define NORM32(X)    (X)
#else
#define NORM16(X)    SDL_Swap16(X)
#define NORM32(X)    SDL_Swap32(X)
#endif

/******************************************************************************
FUNCTIONS
******************************************************************************/

BNX_BOOL sysInit();

BNX_INT32 sysRand( BNX_INT32 max );

void sysRandInit( BNX_UINT32 init );

BNX_UINT32 sysGetTime();

char* sysGetFullFileName( char *file );

void sysUpdate();

/******************************************************************************
FILE FUNCIONS
******************************************************************************/

BNX_UINT32 sysGetFileLen( char *file );

void sysFPut8( BNX_UINT8 n, FILE *f );
void sysFPut16( BNX_UINT16 n, FILE *f );
void sysFPut32( BNX_UINT32 n, FILE *f );

BNX_UINT8 sysFGet8( FILE *f );
BNX_UINT16 sysFGet16( FILE *f );
BNX_UINT32 sysFGet32( FILE *f );

/* File allignemt macro */
#define sysFPut1byte(F) {sysFPut8(0,(F));}
#define sysFPut2byte(F) {sysFPut8(0,(F));sysFPut8(0,(F));}
#define sysFPut3byte(F) {sysFPut8(0,(F));sysFPut8(0,(F));sysFPut8(0,(F));}

#define sysFGet1byte(F) {sysFGet8(F);}
#define sysFGet2byte(F) {sysFGet8(F);sysFGet8(F);}
#define sysFGet3byte(F) {sysFGet8(F);sysFGet8(F);sysFGet8(F);}

#endif

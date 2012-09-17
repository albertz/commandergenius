/******************************************************************************
BINIAX SYSTEM-RELATED IMPLEMENTATIONS
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

/******************************************************************************
INCLUDES
******************************************************************************/

#include <stdlib.h>

#include "inc.h"

/******************************************************************************
FUNCTIONS
******************************************************************************/

BNX_BOOL sysInit()
{
	atexit( SDL_Quit );
	sysRandInit( sysGetTime() );

	return BNX_TRUE;
}

BNX_INT32 sysRand( BNX_INT32 max )
{
	return ( BNX_INT32 ) rand() % max;
}

void sysRandInit( BNX_UINT32 init )
{
	srand( init );
}

BNX_UINT32 sysGetTime()
{
	return ( BNX_UINT32 ) SDL_GetTicks();
}

char* sysGetFullFileName( char *file )
{
	return (char *) file;
}

void sysUpdate()
{
	SDL_Delay( 1 );
}

/******************************************************************************
FILE FUNCTIONS
******************************************************************************/

BNX_UINT32 sysGetFileLen( char *file )
{
	BNX_UINT32 len = 0;
	FILE *f;

	f = fopen( file, "rb" );
	if ( f == (FILE *) NULL )
		return 0;
	while( !feof( f ) )
	{
		fgetc( f );
		++len;
	}
	fclose( f );
	return len-1;
}

void sysFPut8( BNX_UINT8 n, FILE *f )
{
	fputc( n, f );
}

void sysFPut16( BNX_UINT16 n, FILE *f )
{
	fputc( (char) (n & 0xff), f );
	fputc( (char) (n >> 8), f );
}

void sysFPut32( BNX_UINT32 n, FILE *f )
{
	fputc( (char) (n & 0xff), f );
	fputc( (char) ((n >> 8) & 0xff), f );
	fputc( (char) ((n >> 16) & 0xff), f );
	fputc( (char) ((n >> 24) & 0xff), f );
}

BNX_UINT8 sysFGet8( FILE *f )
{
	BNX_INT8 n = 0;
	n = fgetc( f );
	return n;
}

BNX_UINT16 sysFGet16( FILE *f )
{
	BNX_UINT16 n = 0;
	n = (BNX_UINT16) fgetc( f );
	n |= (BNX_UINT16) fgetc( f ) << 8;
	return n;
}

BNX_UINT32 sysFGet32( FILE *f )
{
	BNX_UINT32 n = 0;
	n = (BNX_UINT32) fgetc( f );
	n |= (BNX_UINT32) fgetc( f ) << 8;
	n |= (BNX_UINT32) fgetc( f ) << 16;
	n |= (BNX_UINT32) fgetc( f ) << 24;
	return n;
}

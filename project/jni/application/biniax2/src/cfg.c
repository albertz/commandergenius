/******************************************************************************
BINIAX SETUP IMPLEMENTATIONS
COPYRIGHT JORDAN TUZSUZOV, (C) 2005-2007

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

#include "inc.h"


/******************************************************************************
FUNCTIONS
******************************************************************************/

#define _Cfg_Buffer 255
#define csConfigName "config.bnx2"

struct BNX_SETUP
{
	BNX_BOOL	sound;
	BNX_BOOL	music;
	BNX_BOOL	fullscreen;
	BNX_BOOL	touch;
} _Cfg;

BNX_BOOL cfgCheckRegistered( char * RegCode );

BNX_BOOL cfgInit()
{
	FILE	*f;
	char	buffer[ _Cfg_Buffer ];
	int		nTemp;

	_Cfg.sound = BNX_TRUE;
	_Cfg.music = BNX_TRUE;
	_Cfg.fullscreen = BNX_FALSE;
	_Cfg.touch = BNX_FALSE;

	f = fopen( sysGetFullFileName( csConfigName ), "rt" );
	if ( f == 0 )
	{
		return BNX_FALSE;
	}

	fgets( buffer, _Cfg_Buffer, f );
	sscanf( buffer, "SOUND=%d", &nTemp );
	_Cfg.sound = (BNX_BOOL) nTemp;

	fgets( buffer, _Cfg_Buffer, f );
	sscanf( buffer, "MUSIC=%d", &nTemp );
	_Cfg.music = (BNX_BOOL) nTemp;

	fgets( buffer, _Cfg_Buffer, f );
	sscanf( buffer, "FULLSCREEN=%d", &nTemp );
	_Cfg.fullscreen = (BNX_BOOL) nTemp;

	fgets( buffer, _Cfg_Buffer, f );
	sscanf( buffer, "TOUCH=%d", &nTemp );
	_Cfg.touch = (BNX_BOOL) nTemp;

	fclose( f );

	return BNX_TRUE;
}

BNX_BOOL cfgGetSound()
{
	return _Cfg.sound;
}

BNX_BOOL cfgGetMusic()
{
	return _Cfg.music;
}

BNX_BOOL cfgGetFullscreen()
{
	return _Cfg.fullscreen;
}

BNX_BOOL cfgGetTouch()
{
	return _Cfg.touch;
}

void cfgSetSound( BNX_BOOL v )
{
	_Cfg.sound = v;
}

void cfgSetMusic( BNX_BOOL v )
{
	_Cfg.music = v;
}

void cfgSetFullscreen( BNX_BOOL v )
{
	_Cfg.fullscreen = v;
}

void cfgSetTouch( BNX_BOOL v )
{
	_Cfg.touch = v;
}

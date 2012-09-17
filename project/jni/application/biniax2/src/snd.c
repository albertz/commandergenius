/******************************************************************************
BINIAX SOUND-RELATED IMPLEMENTATIONS
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
#include <SDL_mixer.h>

#include "inc.h"

/******************************************************************************
LOCALS
******************************************************************************/

BNX_SND _Snd;

/******************************************************************************
FUNCTIONS
******************************************************************************/

BNX_BOOL sndInit()
{
	BNX_INT32	audio_rate		= 44000;
	BNX_UINT16	audio_format	= AUDIO_S16;
	BNX_INT32	audio_channels	= 2;
	BNX_BOOL	loaded			= BNX_TRUE;

	if ( Mix_OpenAudio( audio_rate, audio_format, audio_channels, 1024 ) < 0 )
	{
		cfgSetMusic( BNX_FALSE );
		cfgSetSound( BNX_FALSE );
		return BNX_TRUE;
	}
	else
	{
		Mix_QuerySpec( &audio_rate, &audio_format, &audio_channels );
	}

	Mix_VolumeMusic( MIX_MAX_VOLUME >> 1 );

	_Snd.sounds[ 1 ] = Mix_LoadWAV( "data/sound/sfx1.wav" );
	_Snd.sounds[ 2 ] = Mix_LoadWAV( "data/sound/sfx2.wav" );
	_Snd.sounds[ 3 ] = Mix_LoadWAV( "data/sound/sfx3.wav" );
	_Snd.sounds[ 4 ] = Mix_LoadWAV( "data/sound/sfx4.wav" );
	_Snd.sounds[ 5 ] = Mix_LoadWAV( "data/sound/sfx5.wav" );

	_Snd.music[ 0 ] = Mix_LoadMUS( "data/music/biniax_common00.it" );
	_Snd.music[ 1 ] = Mix_LoadMUS( "data/music/biniax_common01.it" );
	_Snd.music[ 2 ] = Mix_LoadMUS( "data/music/biniax_common02.it" );
	_Snd.music[ 3 ] = Mix_LoadMUS( "data/music/biniax_common03.it" );
	_Snd.music[ 4 ] = Mix_LoadMUS( "data/music/biniax_common04.it" );
	_Snd.music[ 5 ] = Mix_LoadMUS( "data/music/biniax_common05.it" );
	_Snd.music[ 6 ] = Mix_LoadMUS( "data/music/biniax_common06.it" );
	_Snd.music[ 7 ] = Mix_LoadMUS( "data/music/biniax_common07.it" );

	return loaded;
}

void sndUpdate()
{
	return;
}

void sndPlay( BNX_GAME *game )
{
	BNX_UINT32 snd		= 0;
	BNX_UINT32 sndmask	= 1;

	if ( cfgGetSound() == BNX_FALSE )
	{
		return;
	}

	while ( game->sounds != cSndNone && snd <= cSndLast )
	{
		sndmask = 1 << snd;
		if ( (game->sounds & sndmask) != cSndNone )
		{
			if ( _Snd.sounds[ snd ] != NULL )
			{
				Mix_PlayChannel( snd % cSndChannels, _Snd.sounds[ snd ], 0 );
			}
		}
		game->sounds &= ~sndmask;
		snd ++;
	}
}

void sndPlayMusic( BNX_INT16 index )
{
	if ( cfgGetMusic() == BNX_FALSE )
	{
		return;
	}

	if ( index >= 0 && index < cSndMaxMusic )
	{
		_Snd.curMusic = index;
		Mix_PlayMusic( _Snd.music[ index ], 1 );
	}
}

void sndUpdateMusic( BNX_GAME *game, BNX_BOOL change )
{
	BNX_INT32	newMusic;

	if ( cfgGetMusic() == BNX_FALSE )
	{
		return;
	}

	if ( Mix_PlayingMusic() == 0 )
	{
		if ( change == BNX_TRUE )
		{
			do
			{
				newMusic = sysRand( cSndMaxMusic-1 );
			} while( _Snd.curMusic == newMusic );
			
			_Snd.curMusic = newMusic;
		}

		sndPlayMusic( _Snd.curMusic );
	}
}

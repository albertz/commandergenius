/******************************************************************************
BINIAX SOUND-RELATED DEFINITIONS
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

#ifndef _BNX_SND_H
#define _BNX_SND_H

/******************************************************************************
INCLUDES
******************************************************************************/

#include <SDL_mixer.h>

#include "inc.h"

/******************************************************************************
CONSTANTS
******************************************************************************/

#define cSndChannels	8
#define cSndMaxMusic	8
#define cSndMenu		7

enum _BNX_Sounds {
	cSndNone =	0,
	cSndTake,
	cSndFail,
	cSndScroll,
	cSndShake,
	cSndSweep,
	cSndLast,
};

/******************************************************************************
LOCAL SOUND DATA (WAV SAMPLES, ETC.)
******************************************************************************/

typedef struct BNX_SND {

	BNX_INT32	curMusic;
	Mix_Chunk	*sounds[ cSndLast ];
	Mix_Music	*music[ cSndMaxMusic ];

} BNX_SND;

/******************************************************************************
FUNCTIONS
******************************************************************************/

BNX_BOOL sndInit();

void sndUpdate();

void sndPlay( BNX_GAME *game );

void sndPlayMusic( BNX_INT16 index );

void sndUpdateMusic( BNX_GAME *game, BNX_BOOL change );

//void sndPlay( BNX_INT16 index );

#endif

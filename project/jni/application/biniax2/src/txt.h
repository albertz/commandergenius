/******************************************************************************
BINIAX 2 TEXT MESSAGES
COPYRIGHT JORDAN TUZSUZOV, (C) 2006-2007

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

#ifndef _BNX_TXT_H
#define _BNX_TXT_H

#include "game.h"

char *TXT_HofCase[] = {
	"BEST ARCADE SCORES",
	"BEST TACTIC SCORES"
};

char *TXT_MenuMain[ cMaxOptions ] = {
		"CONTINUE LAST GAME",
		"NEW ARCADE GAME",
		"NEW TACTIC GAME",
		"MULTIPLAYER",
		"HALL OF FAME",
		"HELP",
		"EXIT"
};

char *TXT_MenuMulti[ cMaxOptions ] = {
		"SAME MACHINE MULTIPLAYER",
		"START NETWORK GAME",
		"JOIN NETWORK GAME",
		"BACK"
};

char *TXT_Extras[ cMaxExtras ] = {
		"MINOR COMBO +5",
		"COMBO +10",
		"BIG COMBO +20",
		"MEGA COMBO +40",
		"FIELD MASTERED +100",
		"NEW BROOM",
};

#endif
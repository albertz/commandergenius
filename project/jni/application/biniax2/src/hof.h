/******************************************************************************
BINIAX HALL OF FAME DEFINITIONS
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

#ifndef _BNX_HOF_H
#define _BNX_HOF_H

/******************************************************************************
INCLUDES
******************************************************************************/

#include "inc.h"

#define cHofEntries		7
#define cHofNameLen		30
#define cHofInitScore	1000

/******************************************************************************
HALL OF FAME DATATYPES
******************************************************************************/
typedef struct BNX_HALL_ENTRY
{
	char		name[ cHofNameLen ];
	BNX_INT32	score;
} BNX_HALL_ENTRY;

typedef struct BNX_HALL
{
	BNX_HALL_ENTRY	arcade[ cHofEntries ];
	BNX_HALL_ENTRY	tactic[ cHofEntries ];
} BNX_HALL;

/******************************************************************************
FUNCTIONS
******************************************************************************/

BNX_BOOL hofInit();

BNX_BOOL hofSave();

void hofView();

BNX_BOOL hofEnter( BNX_GAME *game );

BNX_HALL *hofGet();

#endif

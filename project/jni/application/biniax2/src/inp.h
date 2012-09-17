/******************************************************************************
BINIAX INPUT-RELATED DEFINITIONS
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

#ifndef _BNX_INP_H
#define _BNX_INP_H

/******************************************************************************
INCLUDES
******************************************************************************/

#include "inc.h"

#define cInpKeyEnter		SDLK_RETURN
#define cInpKeyBackspace	SDLK_BACKSPACE

/******************************************************************************
LOCAL INPUT DATA (KEY FLAGS, POINTERS, ETC.)
******************************************************************************/

typedef struct BNX_INP
{

	BNX_BOOL	keyUp;
	BNX_BOOL	keyDown;
	BNX_BOOL	keyLeft;
	BNX_BOOL	keyRight;
	BNX_BOOL	keyAltUp;
	BNX_BOOL	keyAltDown;
	BNX_BOOL	keyAltLeft;
	BNX_BOOL	keyAltRight;
	BNX_BOOL	keyPageUp;
	BNX_BOOL	keyPageDown;
	BNX_BOOL	keyA;
	BNX_BOOL	keyB;
	BNX_BOOL	keyC;
	BNX_BOOL	keyQuit;
	BNX_BOOL	keyDel;

	BNX_BOOL	mousePress;
	BNX_INT16	mouseX;
	BNX_INT16	mouseY;

	char		letter;

	BNX_UINT32	moment;

} BNX_INP;

BNX_INP _Inp;

/******************************************************************************
FUNCTIONS
******************************************************************************/

BNX_BOOL inpInit();

void inpUpdate();

BNX_BOOL inpKeyLeft();
BNX_BOOL inpKeyRight();
BNX_BOOL inpKeyUp();
BNX_BOOL inpKeyDown();
BNX_BOOL inpKeyAltLeft();
BNX_BOOL inpKeyAltRight();
BNX_BOOL inpKeyAltUp();
BNX_BOOL inpKeyAltDown();
BNX_BOOL inpKeyPageUp();
BNX_BOOL inpKeyPageDown();
BNX_BOOL inpKeyA();
BNX_BOOL inpKeyB();
BNX_BOOL inpKeyC();
BNX_BOOL inpKeyDel();
BNX_BOOL inpExit();

char inpGetChar();

BNX_INP *inpDirect();


#endif

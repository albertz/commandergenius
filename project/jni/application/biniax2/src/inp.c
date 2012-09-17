/******************************************************************************
BINIAX INPUT-RELATED IMPLEMENTATIONS
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


BNX_BOOL inpInit()
{
	_Inp.keyUp		= BNX_FALSE;
	_Inp.keyDown	= BNX_FALSE;
	_Inp.keyLeft	= BNX_FALSE;
	_Inp.keyRight	= BNX_FALSE;
	_Inp.keyAltUp	= BNX_FALSE;
	_Inp.keyAltDown	= BNX_FALSE;
	_Inp.keyAltLeft	= BNX_FALSE;
	_Inp.keyAltRight= BNX_FALSE;
	_Inp.keyPageUp	= BNX_FALSE;
	_Inp.keyPageDown= BNX_FALSE;
	_Inp.keyA		= BNX_FALSE;
	_Inp.keyB		= BNX_FALSE;
	_Inp.keyQuit	= BNX_FALSE;
	_Inp.keyDel		= BNX_FALSE;

	_Inp.mousePress	= BNX_FALSE;
	_Inp.mouseX		= 0;
	_Inp.mouseY		= 0;

	_Inp.letter		= 0;

	return BNX_TRUE;
}

void inpUpdate()
{
    SDL_Event	event;
	int			x, y;

    while( SDL_PollEvent( &event ) ) 
	{
        switch( event.type ) 
		{
			case SDL_KEYDOWN:
				switch( event.key.keysym.sym ) 
				{
					case SDLK_SPACE : 
						_Inp.keyC		= BNX_TRUE;
						break;
					case SDLK_RETURN : 
						_Inp.keyA		= BNX_TRUE;
						break;
					case SDLK_ESCAPE : 
						_Inp.keyB		= BNX_TRUE;
						break;
					case SDLK_KP8:
					case SDLK_UP : 
						_Inp.keyUp		= BNX_TRUE;
						break;
					case SDLK_KP2:
					case SDLK_DOWN : 
						_Inp.keyDown	= BNX_TRUE;
						break;
					case SDLK_KP4:
					case SDLK_LEFT : 
						_Inp.keyLeft	= BNX_TRUE;
						break;
					case SDLK_KP6:
					case SDLK_RIGHT : 
						_Inp.keyRight	= BNX_TRUE;
						break;

					case SDLK_w : 
						_Inp.keyAltUp	= BNX_TRUE;
						break;
					case SDLK_s : 
						_Inp.keyAltDown	= BNX_TRUE;
						break;
					case SDLK_a : 
						_Inp.keyAltLeft	= BNX_TRUE;
						break;
					case SDLK_d : 
						_Inp.keyAltRight= BNX_TRUE;
						break;

					case SDLK_BACKSPACE :
					case SDLK_DELETE :
						_Inp.keyDel = BNX_TRUE;
						break;
					case SDLK_PAGEUP : 
						_Inp.keyPageUp	= BNX_TRUE;
						break;
					case SDLK_PAGEDOWN : 
						_Inp.keyPageDown= BNX_TRUE;
						break;
				}
				if ( event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z )
				{
					_Inp.letter = (event.key.keysym.sym - SDLK_a) + 'A';
				}
				else if ( event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_9 )
				{
					_Inp.letter = (event.key.keysym.sym - SDLK_0) + '0';
				}
				else if ( event.key.keysym.sym == SDLK_SPACE )
				{
					_Inp.letter = ' ';
				}
			break;

			case SDL_MOUSEBUTTONDOWN :
				_Inp.mousePress = BNX_TRUE;
			break;

			case SDL_QUIT:
				SDL_Quit();
				exit( 2 );
			break;

		}
	}
	
	SDL_GetMouseState( &x, &y );
	_Inp.mouseX = x;
	_Inp.mouseY = y;
}

BNX_BOOL inpKeyLeft()
{
	if ( _Inp.keyLeft == BNX_TRUE )
	{
		_Inp.keyLeft = BNX_FALSE;
		return BNX_TRUE;
	}

	return BNX_FALSE;
}

BNX_BOOL inpKeyRight()
{
	if ( _Inp.keyRight == BNX_TRUE )
	{
		_Inp.keyRight = BNX_FALSE;
		return BNX_TRUE;
	}

	return BNX_FALSE;
}

BNX_BOOL inpKeyUp()
{
	if ( _Inp.keyUp == BNX_TRUE )
	{
		_Inp.keyUp = BNX_FALSE;
		return BNX_TRUE;
	}

	return BNX_FALSE;
}

BNX_BOOL inpKeyDown()
{
	if ( _Inp.keyDown == BNX_TRUE )
	{
		_Inp.keyDown = BNX_FALSE;
		return BNX_TRUE;
	}

	return BNX_FALSE;
}

BNX_BOOL inpKeyAltLeft()
{
	if ( _Inp.keyAltLeft == BNX_TRUE )
	{
		_Inp.keyAltLeft = BNX_FALSE;
		return BNX_TRUE;
	}

	return BNX_FALSE;
}

BNX_BOOL inpKeyAltRight()
{
	if ( _Inp.keyAltRight == BNX_TRUE )
	{
		_Inp.keyAltRight = BNX_FALSE;
		return BNX_TRUE;
	}

	return BNX_FALSE;
}

BNX_BOOL inpKeyAltUp()
{
	if ( _Inp.keyAltUp == BNX_TRUE )
	{
		_Inp.keyAltUp = BNX_FALSE;
		return BNX_TRUE;
	}

	return BNX_FALSE;
}

BNX_BOOL inpKeyAltDown()
{
	if ( _Inp.keyAltDown == BNX_TRUE )
	{
		_Inp.keyAltDown = BNX_FALSE;
		return BNX_TRUE;
	}

	return BNX_FALSE;
}

BNX_BOOL inpKeyA()
{
	if ( _Inp.keyA == BNX_TRUE )
	{
		_Inp.keyA = BNX_FALSE;
		return BNX_TRUE;
	}

	return BNX_FALSE;
}

BNX_BOOL inpKeyB()
{
	if ( _Inp.keyB == BNX_TRUE )
	{
		_Inp.keyB = BNX_FALSE;
		return BNX_TRUE;
	}

	return BNX_FALSE;
}

BNX_BOOL inpKeyC()
{
	if ( _Inp.keyC == BNX_TRUE )
	{
		_Inp.keyC = BNX_FALSE;
		return BNX_TRUE;
	}

	return BNX_FALSE;
}

BNX_BOOL inpKeyPageUp()
{
	if ( _Inp.keyPageUp == BNX_TRUE )
	{
		_Inp.keyPageUp = BNX_FALSE;
		return BNX_TRUE;
	}

	return BNX_FALSE;
}

BNX_BOOL inpKeyPageDown()
{
	if ( _Inp.keyPageDown == BNX_TRUE )
	{
		_Inp.keyPageDown = BNX_FALSE;
		return BNX_TRUE;
	}

	return BNX_FALSE;
}

BNX_BOOL inpKeyDel()
{
	if ( _Inp.keyDel == BNX_TRUE )
	{
		_Inp.keyDel = BNX_FALSE;
		return BNX_TRUE;
	}

	return BNX_FALSE;
}

BNX_BOOL inpExit()
{
	return _Inp.keyQuit;
}

char inpGetChar()
{
	char ch = 0;
	if ( _Inp.letter > 0 )
	{
		ch = _Inp.letter;
		_Inp.letter = 0;
	}

	return ch;
}

BNX_INP *inpDirect()
{
	return &_Inp;
}
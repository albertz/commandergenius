/******************************************************************************
BINIAX ALL INCLUDES
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

#ifndef _BNX_INC_H
#define _BNX_INC_H

/******************************************************************************
INCLUDES
******************************************************************************/

#include "game.h"
#include "hof.h"

#define __WIN32

/* Desktop / SDL includes */
#ifdef __WIN32
#include "types.h"
#include "inp.h"
#include "gfx.h"
#include "snd.h"
#include "sys.h"
#include "cfg.h"
//#include "net.h"

#include <SDL.h>

#endif

/* WinCE / EasyCE includes */
#ifdef __WINCE
#include "wince/types.h"
#include "wince/inp.h"
#include "wince/gfx.h"
#include "wince/snd.h"
#include "wince/sys.h"
#endif

/* Symbain / Series60 includes */
#ifdef __SERIES60
#include "symbian/types.h"
#include "symbian/inp.h"
#include "symbian/gfx.h"
#include "symbian/snd.h"
#include "symbian/sys.h"
#endif


#endif

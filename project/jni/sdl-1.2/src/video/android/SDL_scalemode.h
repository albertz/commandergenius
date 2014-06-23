/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2012 Sam Lantinga <slouken@libsdl.org>

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
*/
/*
This source code is distibuted under ZLIB license, however when compiling with SDL 1.2,
which is licensed under LGPL, the resulting library, and all it's source code,
falls under "stronger" LGPL terms, so is this file.
If you compile this code with SDL 1.3 or newer, or use in some other way, the license stays ZLIB.
*/

/**
 *  \file SDL_scalemode.h
 *  
 *  Header file declaring the SDL_ScaleMode enumeration
 */

#ifndef _SDL_scalemode_h
#define _SDL_scalemode_h

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/**
 *  \brief The texture scale mode used in SDL_RenderCopy().
 */
typedef enum
{
    SDL_SCALEMODE_NONE = 0x00000000,     /**< No scaling, rectangles must
                                              match dimensions */
    
    SDL_SCALEMODE_FAST = 0x00000001,     /**< Point sampling or 
                                              equivalent algorithm */
    
    SDL_SCALEMODE_SLOW = 0x00000002,     /**< Linear filtering or 
                                              equivalent algorithm */
    
    SDL_SCALEMODE_BEST = 0x00000004      /**< Bicubic filtering or 
                                              equivalent algorithm */
} SDL_ScaleMode;


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#include "close_code.h"

#endif /* _SDL_video_h */

/* vi: set ts=4 sw=4 expandtab: */

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
#include "SDL_config.h"
#include "SDL_rect.h"

typedef struct SDL_DirtyRect
{
    SDL_Rect rect;
    struct SDL_DirtyRect *next;
} SDL_DirtyRect;

typedef struct SDL_DirtyRectList
{
    SDL_DirtyRect *list;
    SDL_DirtyRect *free;
} SDL_DirtyRectList;

extern void SDL_AddDirtyRect(SDL_DirtyRectList * list, const SDL_Rect * rect);
extern void SDL_ClearDirtyRects(SDL_DirtyRectList * list);
extern void SDL_FreeDirtyRects(SDL_DirtyRectList * list);

/* vi: set ts=4 sw=4 expandtab: */

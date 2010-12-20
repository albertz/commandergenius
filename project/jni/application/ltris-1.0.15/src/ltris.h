/***************************************************************************
                          defs.h  -  description
                             -------------------
    begin                : Tue Feb 29 2000
    copyright            : (C) 2000 by Michael Speck
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __LTRIS_H
#define __LTRIS_H

/*
====================================================================
Global includes.
====================================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL.h>
#ifdef SOUND
#include <SDL_mixer.h>
#include "audio.h"
#endif
#include "sdl.h"
#include "tools.h"
#include "config.h"

/* i18n */
#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#include "gettext.h"
#if ENABLE_NLS
#define _(str) gettext (str)
#else
#define _(str) (str)
#endif

enum { 
    /* maximum number of bowls */
    BOWL_COUNT = 3,

    /* number of buffered game blocks */
    NEXT_BLOCKS_CHUNK_SIZE = 1000,

    /* bowl defaults */
    BOWL_BLOCK_SIZE = 20,
    BOWL_WIDTH = 10,
    BOWL_HEIGHT = 20,
    BLOCK_COUNT = 7,
    BLOCK_TILE_COUNT = 10
};

/* block mask for the block types */
typedef struct {
    int rx, ry;
    int id;
    int mask[4][4][4];
} Block_Mask;

#endif

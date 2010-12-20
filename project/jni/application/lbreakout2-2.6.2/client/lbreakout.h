/***************************************************************************
                          lbreakout.h  -  description
                             -------------------
    begin                : Thu Sep 6 2001
    copyright            : (C) 2001 by Michael Speck
    email                : kulkanie@gmx.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __LBREAKOUT_H
#define __LBREAKOUT_H

//#define WITH_BUG_REPORT
#define GAME_DEBUG

/*
====================================================================
Global definitions for LBreakout and general system includes.
====================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include "../gui/stk.h"
#include "../common/tools.h"
#include "../common/list.h"
#include "../common/net.h"
#include "../common/messages.h"
#include "../game/gamedefs.h"
#include "misc.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795f
#endif

/* config directory name in home directory */
#ifdef _WIN32
#define CONFIG_DIR_NAME "lgames"
#else
#define CONFIG_DIR_NAME ".lgames"
#endif

/* i18n */
#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif
#include "../common/gettext.h"
#if ENABLE_NLS
#define _(str) gettext (str)
#else
#define _(str) (str)
#endif

/*
====================================================================
Alpha of shadow
====================================================================
*/
enum { SHADOW_ALPHA = 128 };
/*
====================================================================
Number of original backgrounds.
====================================================================
*/
enum { ORIG_BACK_COUNT = 6 };

/* used to compile net messages */
extern char msgbuf[MAX_MSG_SIZE];
extern int msglen;

#endif

// Copyright Michael Martin, 2004.

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _SETUPMENU_H
#define _SETUPMENU_H

#include "controls.h"

typedef enum {
	OPTVAL_DISABLED,
	OPTVAL_ENABLED
} OPT_ENABLABLE;

typedef enum {
	OPTVAL_PC,
	OPTVAL_3DO
} OPT_CONSOLETYPE;

typedef enum {
	OPTVAL_NO_SCALE,
	OPTVAL_BILINEAR_SCALE,
	OPTVAL_BIADAPT_SCALE,
	OPTVAL_BIADV_SCALE,
	OPTVAL_TRISCAN_SCALE,
	OPTVAL_HQXX_SCALE,
} OPT_SCALETYPE;

typedef enum {
	OPTVAL_320_240,
	OPTVAL_640_480,
	OPTVAL_800_600,
	OPTVAL_1024_768,
	OPTVAL_CUSTOM
} OPT_RESTYPE;

typedef enum {
	OPTVAL_PURE_IF_POSSIBLE,
	OPTVAL_ALWAYS_GL
} OPT_DRIVERTYPE;

typedef enum {
	OPTVAL_SILENCE,
	OPTVAL_MIXSDL,
	OPTVAL_OPENAL
} OPT_ADRIVERTYPE;

typedef enum {
	OPTVAL_LOW,
	OPTVAL_MEDIUM,
	OPTVAL_HIGH
} OPT_AQUALITYTYPE;

/* At the moment, CONTROL_TEMPLATE is directly in this structure.  If
 * CONTROL_TEMPLATE and the options available diverge, this will need
 * to change */
typedef struct globalopts_struct {
	OPT_SCALETYPE scaler;
	OPT_RESTYPE res;
	OPT_DRIVERTYPE driver;
	OPT_ADRIVERTYPE adriver;
	OPT_AQUALITYTYPE aquality;
	OPT_ENABLABLE fullscreen, subtitles, scanlines, fps, stereo, music3do, musicremix;
	OPT_CONSOLETYPE menu, text, cscan, scroll, intro, meleezoom, shield;
	CONTROL_TEMPLATE player1, player2;
	int speechvol, musicvol, sfxvol;
} GLOBALOPTS;

void SetupMenu (void);

void GetGlobalOptions (GLOBALOPTS *opts);
void SetGlobalOptions (GLOBALOPTS *opts);

#endif // _SETUPMENU_H

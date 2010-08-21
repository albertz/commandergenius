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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _INIT_H
#define _INIT_H

#include "libs/gfxlib.h"
#include "libs/reslib.h"

#define NUM_PLAYERS 2
#define NUM_SIDES 2

extern FRAME stars_in_space;

extern BOOLEAN InitSpace (void);
extern void UninitSpace (void);

extern SIZE InitShips (void);
extern void UninitShips (void);

extern BOOLEAN load_animation (FRAME *pixarray, RESOURCE big_res,
		RESOURCE med_res, RESOURCE sml_res);
extern BOOLEAN free_image (FRAME *pixarray);

#endif  /* _INIT_H */


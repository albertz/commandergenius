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

#ifndef _FMV_H
#define _FMV_H

#include "libs/compiler.h"
#include "libs/sndlib.h"
#include "libs/gfxlib.h"
#define WANT_SHIP_SPINS

extern void Logo (void);
extern void SplashScreen (void (* DoProcessing)(DWORD TimeOut));
extern void Introduction (void);
extern void Victory (void);
extern void DoShipSpin (COUNT index, MUSIC_REF hMusic);

extern BOOLEAN ShowPresentation (RESOURCE presentation);

#endif  /* _FMV_H */


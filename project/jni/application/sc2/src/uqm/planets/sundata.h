//Copyright Paul Reiche, Fred Ford. 1992-2002

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

#ifndef _SUNDATA_H
#define _SUNDATA_H

#include "plandata.h"
#include "libs/compiler.h"


/*------------------------------ Global Data ------------------------------ */

#define NUMBER_OF_SUN_SIZES (SUPER_GIANT_STAR - DWARF_STAR + 1)

#define DWARF_ENERGY 1
#define GIANT_ENERGY 5
#define SUPERGIANT_ENERGY 20

typedef struct
{
	BYTE StarSize;
	BYTE StarIntensity;
	UWORD StarEnergy;

	PLANET_INFO PlanetInfo;
} SYSTEM_INFO;
		
extern DWORD GenerateMineralDeposits (SYSTEM_INFO *SysInfoPtr,
		COUNT *pwhich_deposit);
extern DWORD GenerateLifeForms (SYSTEM_INFO *SysInfoPtr, COUNT *pwhich_life);

#define DWARF_ELEMENT_DENSITY  1
#define GIANT_ELEMENT_DENSITY 3
#define SUPERGIANT_ELEMENT_DENSITY 8

#define MAX_ELEMENT_DENSITY ((MAX_ELEMENT_UNITS * SUPERGIANT_ELEMENT_DENSITY) << 1)

extern DWORD DoPlanetaryAnalysis (SYSTEM_INFO *SysInfoPtr,
		PLANET_DESC *pPlanetDesc);

extern SYSTEM_INFO CurSysInfo;

#endif /* _SUNDATA_H */


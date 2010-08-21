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

#ifndef UQM_STATUS_H_INCL_
#define UQM_STATUS_H_INCL_

#include "races.h"
#include "libs/compiler.h"

#define CREW_XOFFS 4
#define ENERGY_XOFFS 52
#define GAUGE_YOFFS (SHIP_INFO_HEIGHT - 10)
#define UNIT_WIDTH 2
#define UNIT_HEIGHT 1
#define STAT_WIDTH (1 + UNIT_WIDTH + 1 + UNIT_WIDTH + 1)

#define SHIP_INFO_HEIGHT 65
#define CAPTAIN_XOFFS 4
#define CAPTAIN_YOFFS (SHIP_INFO_HEIGHT + 4)
#define CAPTAIN_WIDTH 55
#define CAPTAIN_HEIGHT 30
#define SHIP_STATUS_HEIGHT (STATUS_HEIGHT >> 1)
#define BAD_GUY_YOFFS 0
#define GOOD_GUY_YOFFS SHIP_STATUS_HEIGHT
#define STARCON_TEXT_HEIGHT 7
#define TINY_TEXT_HEIGHT 9

#define BATTLE_CREW_X 10
#define BATTLE_CREW_Y (64 - SAFE_Y)

extern COORD status_y_offsets[];

extern void InitStatusOffsets (void);

extern void DrawCrewFuelString (COORD y, SIZE state);
extern void ClearShipStatus (COORD y);
extern void OutlineShipStatus (COORD y);
extern void InitShipStatus (SHIP_INFO *ShipInfoPtr, STARSHIP *StarShipPtr,
		RECT *pClipRect);
			// StarShipPtr or pClipRect can be NULL
extern void DeltaStatistics (SHIP_INFO *ShipInfoPtr, COORD y_offs,
		SIZE crew_delta, SIZE energy_delta);
extern void DrawBattleCrewAmount (SHIP_INFO *ShipInfoPtr, COORD y_offs);

extern void DrawCaptainsWindow (STARSHIP *StarShipPtr);
extern BOOLEAN DeltaEnergy (ELEMENT *ElementPtr, SIZE energy_delta);
extern BOOLEAN DeltaCrew (ELEMENT *ElementPtr, SIZE crew_delta);

extern void PreProcessStatus (ELEMENT *ShipPtr);
extern void PostProcessStatus (ELEMENT *ShipPtr);

#endif /* UQM_STATUS_H_INCL_ */

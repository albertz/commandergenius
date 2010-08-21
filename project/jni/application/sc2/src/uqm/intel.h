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

#ifndef _INTEL_H
#define _INTEL_H

#include "battlecontrols.h"
#include "controls.h"
#include "element.h"
#include "races.h"

#define MANEUVERABILITY(pi) ((pi)->ManeuverabilityIndex)
#define WEAPON_RANGE(pi) ((pi)->WeaponRange)

#define WORLD_TO_TURN(d) ((d)>>6)

#define CLOSE_RANGE_WEAPON DISPLAY_TO_WORLD (50)
#define LONG_RANGE_WEAPON DISPLAY_TO_WORLD (1000)
#define FAST_SHIP 150
#define MEDIUM_SHIP 45
#define SLOW_SHIP 25

enum
{
	ENEMY_SHIP_INDEX = 0,
	CREW_OBJECT_INDEX,
	ENEMY_WEAPON_INDEX,
	GRAVITY_MASS_INDEX,
	FIRST_EMPTY_INDEX
};

extern BATTLE_INPUT_STATE computer_intelligence (
		ComputerInputContext *context, STARSHIP *StarShipPtr);
extern BATTLE_INPUT_STATE tactical_intelligence (
		ComputerInputContext *context, STARSHIP *StarShipPtr);
extern void ship_intelligence (ELEMENT *ShipPtr,
		EVALUATE_DESC *ObjectsOfConcern, COUNT ConcernCounter);
extern BOOLEAN ship_weapons (ELEMENT *ShipPtr, ELEMENT *OtherPtr,
		COUNT margin_of_error);

extern void Pursue (ELEMENT *ShipPtr, EVALUATE_DESC *EvalDescPtr);
extern void Entice (ELEMENT *ShipPtr, EVALUATE_DESC *EvalDescPtr);
extern void Avoid (ELEMENT *ShipPtr, EVALUATE_DESC *EvalDescPtr);
extern BOOLEAN TurnShip (ELEMENT *ShipPtr, COUNT angle);
extern BOOLEAN ThrustShip (ELEMENT *ShipPtr, COUNT angle);


#define HUMAN_CONTROL (BYTE)(1 << 0)
#define CYBORG_CONTROL (BYTE)(1 << 1)
		// The computer fights the battles.
#define PSYTRON_CONTROL (BYTE)(1 << 2)
		// The computer selects the ships to fight with.
#define NETWORK_CONTROL (BYTE)(1 << 3)
#define COMPUTER_CONTROL (CYBORG_CONTROL | PSYTRON_CONTROL)
#define CONTROL_MASK (HUMAN_CONTROL | COMPUTER_CONTROL | NETWORK_CONTROL)

#define STANDARD_RATING (BYTE)(1 << 4)
#define GOOD_RATING (BYTE)(1 << 5)
#define AWESOME_RATING (BYTE)(1 << 6)


#endif /* _INTEL_H */



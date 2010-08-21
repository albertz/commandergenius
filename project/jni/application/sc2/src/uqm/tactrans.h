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

#ifndef _TACTRANS_H
#define _TACTRANS_H

#include "libs/compiler.h"
#include "races.h"
#include "element.h"
#include "battlecontrols.h"

bool battleEndReadyHuman (HumanInputContext *context);
bool battleEndReadyComputer (ComputerInputContext *context);
#ifdef NETPLAY
bool battleEndReadyNetwork (NetworkInputContext *context);
#endif

extern void ship_transition (ELEMENT *ElementPtr);
extern BOOLEAN OpponentAlive (STARSHIP *TestStarShipPtr);
extern void new_ship (ELEMENT *ElementPtr);
extern void ship_death (ELEMENT *ShipPtr);
extern void spawn_ion_trail (ELEMENT *ElementPtr);
extern void flee_preprocess (ELEMENT *ElementPtr);

extern void StopDitty (void);
extern void ResetWinnerStarShip (void);

#endif  /* _TACTRANS_H */



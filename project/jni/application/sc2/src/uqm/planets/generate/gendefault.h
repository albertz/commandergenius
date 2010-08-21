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

#ifndef GENDEFAULT_H
#define GENDEFAULT_H

#include "types.h"
#include "../planets.h"
#include "libs/compiler.h"

bool GenerateDefault_initNpcs (SOLARSYS_STATE *solarSys);
bool GenerateDefault_reinitNpcs (SOLARSYS_STATE *solarSys);
bool GenerateDefault_uninitNpcs (SOLARSYS_STATE *solarSys);
bool GenerateDefault_generatePlanets (SOLARSYS_STATE *solarSys);
bool GenerateDefault_generateMoons (SOLARSYS_STATE *solarSys,
		PLANET_DESC *planet);
bool GenerateDefault_generateName (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
bool GenerateDefault_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
bool GenerateDefault_generateMinerals (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);
bool GenerateDefault_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);
bool GenerateDefault_generateLife (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);

extern const GenerateFunctions generateDefaultFunctions;

#endif  /* GENDEFAULT_H */


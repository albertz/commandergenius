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

#ifndef GENERATE_H
#define GENERATE_H

typedef struct GenerateFunctions GenerateFunctions;

#include "types.h"
#include "planets.h"
#include "libs/compiler.h"

/*
 * To do (for further cleanups):
 * - split off pickupMineral, pickupEnergy, pickupLife from Generate*
 * - split off generateOrbital in a calculation and an activation
 *   (graphics and music) part.
 * - make generateOrbital return a meaningful value, specifically, whether
 *   or not the player is going into orbit
 * - make generateMinerals et al return the last object number (or count
 *   if called for all objects) and take non-pointer whichNode
 * - for GenerateNameFunction, set the name in an argument, instead
 *   of in GLOBAL_SYS(PlanetName)
 * - make generateName work for moons
 * - add parameters to initNcs, reinitNpcs, and uninitNpcs, so that
 *   globals don't have to be used.
 * - Add a reference from each world to the solar system, so that most
 *   of these functions can do with one less argument.
 * - (maybe) don't directly call the generate functions via
 *   solarSys->genFuncs->..., but use a function for this, which first
 *   checks for solar system dependent handlers, and if this does not exist,
 *   or returns false, calls the default function.
 * - use isNodeRetrieved, setNodeRetrieved, and setNodeNotRetrieved instead
 *   of manually messing with bit shifts, etc.
 */

// Any of these functions returning true means that the action has been
// handled, and that the default function should not be called.
typedef bool (*InitNpcsFunction)(SOLARSYS_STATE *solarSys);
typedef bool (*ReinitNpcsFunction)(SOLARSYS_STATE *solarSys);
typedef bool (*UninitNpcsFunction)(SOLARSYS_STATE *solarSys);
typedef bool (*GeneratePlanetsFunction)(SOLARSYS_STATE *solarSys);
typedef bool (*GenerateMoonsFunction)(SOLARSYS_STATE *solarSys,
		PLANET_DESC *planet);
typedef bool (*GenerateOrbitalFunction)(SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
typedef bool (*GenerateNameFunction)(SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
typedef bool (*GenerateMineralsFunction)(SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);
typedef bool (*GenerateEnergyFunction)(SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);
typedef bool (*GenerateLifeFunction)(SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);


struct GenerateFunctions {
	InitNpcsFunction initNpcs;
			// Ships in the solar system, the first time it is accessed.
	ReinitNpcsFunction reinitNpcs;
			// Ships in the solar system, every next time it is accessed.
	UninitNpcsFunction uninitNpcs;
			// When leaving the solar system.
	GeneratePlanetsFunction generatePlanets;
			// Layout of planets within a solar system.
	GenerateMoonsFunction generateMoons;
			// Layout of moons around a planet.
	GenerateNameFunction generateName;
			// Name of a planet.
	GenerateOrbitalFunction generateOrbital;
			// Characteristics of words (planets and moons).
	GenerateMineralsFunction generateMinerals;
			// Minerals on the planet surface.
	GenerateEnergyFunction generateEnergy;
			// Energy sources on the planet surface.
	GenerateLifeFunction generateLife;
			// Bio on the planet surface.
};


#endif  /* GENERATE_H */


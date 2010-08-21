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

#include "genall.h"
#include "../planets.h"
#include "../../comm.h"

static bool GenerateSyreen_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateSyreen_generateMoons (SOLARSYS_STATE *solarSys,
		PLANET_DESC *planet);
static bool GenerateSyreen_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);


const GenerateFunctions generateSyreenFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateSyreen_generatePlanets,
	/* .generateMoons    = */ GenerateSyreen_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateSyreen_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateDefault_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
};


static bool
GenerateSyreen_generatePlanets (SOLARSYS_STATE *solarSys)
{
	GenerateDefault_generatePlanets (solarSys);

	solarSys->PlanetDesc[0].data_index = WATER_WORLD | PLANET_SHIELDED;
	solarSys->PlanetDesc[0].NumPlanets = 1;

	return true;
}

static bool
GenerateSyreen_generateMoons (SOLARSYS_STATE *solarSys, PLANET_DESC *planet)
{
	GenerateDefault_generateMoons (solarSys, planet);

	if (matchWorld (solarSys, planet, 0, MATCH_PLANET))
	{
		solarSys->MoonDesc[0].data_index = HIERARCHY_STARBASE;
		solarSys->MoonDesc[0].radius = MIN_MOON_RADIUS;
		solarSys->MoonDesc[0].location.x =
				COSINE (QUADRANT, solarSys->MoonDesc[0].radius);
		solarSys->MoonDesc[0].location.y =
				SINE (QUADRANT, solarSys->MoonDesc[0].radius);
	}

	return true;
}

static bool
GenerateSyreen_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		/* Syreen home planet */
		GenerateDefault_generateOrbital (solarSys, world);

		solarSys->SysInfo.PlanetInfo.SurfaceTemperature = 19;
		solarSys->SysInfo.PlanetInfo.Tectonics = 0;
		solarSys->SysInfo.PlanetInfo.Weather = 0;
		solarSys->SysInfo.PlanetInfo.AtmoDensity = EARTH_ATMOSPHERE * 9 / 10;
		return true;
	}

	if (matchWorld (solarSys, world, 0, 0))
	{
		/* Starbase */
		InitCommunication (SYREEN_CONVERSATION);
		return true;
	}

	GenerateDefault_generateOrbital (solarSys, world);

	return true;
}


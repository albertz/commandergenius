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
#include "../../build.h"
#include "../../globdata.h"
#include "../../grpinfo.h"
#include "../../state.h"
#include "libs/mathlib.h"


static bool GenerateColony_initNpcs (SOLARSYS_STATE *solarSys);
static bool GenerateColony_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateColony_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);


const GenerateFunctions generateColonyFunctions = {
	/* .initNpcs         = */ GenerateColony_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateColony_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateColony_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateDefault_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
};


static bool
GenerateColony_initNpcs (SOLARSYS_STATE *solarSys)
{
	HIPGROUP hGroup;

	GLOBAL (BattleGroupRef) = GET_GAME_STATE_32 (COLONY_GRPOFFS0);
	if (GLOBAL (BattleGroupRef) == 0)
	{
		CloneShipFragment (URQUAN_SHIP,
				&GLOBAL (npc_built_ship_q), 0);
		GLOBAL (BattleGroupRef) = PutGroupInfo (GROUPS_ADD_NEW, 1);
		ReinitQueue (&GLOBAL (npc_built_ship_q));
		SET_GAME_STATE_32 (COLONY_GRPOFFS0, GLOBAL (BattleGroupRef));
	}

	GenerateDefault_initNpcs (solarSys);

	if (GLOBAL (BattleGroupRef)
			&& (hGroup = GetHeadLink (&GLOBAL (ip_group_q))))
	{
		IP_GROUP *GroupPtr;

		GroupPtr = LockIpGroup (&GLOBAL (ip_group_q), hGroup);
		GroupPtr->task = IN_ORBIT;
		GroupPtr->sys_loc = 0 + 1; /* orbitting colony */
		GroupPtr->dest_loc = 0 + 1; /* orbitting colony */
		GroupPtr->loc.x = 0;
		GroupPtr->loc.y = 0;
		GroupPtr->group_counter = 0;
		UnlockIpGroup (&GLOBAL (ip_group_q), hGroup);
	}

	return true;
}

static bool
GenerateColony_generatePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT angle;
	PLANET_DESC *pMinPlanet;

	pMinPlanet = &solarSys->PlanetDesc[0];
	FillOrbits (solarSys, (BYTE)~0, pMinPlanet, FALSE);

	pMinPlanet->radius = EARTH_RADIUS * 115L / 100;
	angle = ARCTAN (pMinPlanet->location.x, pMinPlanet->location.y);
	pMinPlanet->location.x = COSINE (angle, pMinPlanet->radius);
	pMinPlanet->location.y = SINE (angle, pMinPlanet->radius);
	pMinPlanet->data_index = WATER_WORLD | PLANET_SHIELDED;

	return true;
}

static bool
GenerateColony_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		DoPlanetaryAnalysis (&solarSys->SysInfo, world);

		solarSys->SysInfo.PlanetInfo.AtmoDensity =
				EARTH_ATMOSPHERE * 98 / 100;
		solarSys->SysInfo.PlanetInfo.Weather = 0;
		solarSys->SysInfo.PlanetInfo.Tectonics = 0;
		solarSys->SysInfo.PlanetInfo.SurfaceTemperature = 28;

		LoadPlanet (NULL);

		return true;
	}

	GenerateDefault_generateOrbital (solarSys, world);

	return true;
}


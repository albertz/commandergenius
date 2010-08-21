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
#include "../lifeform.h"
#include "../planets.h"
#include "../../build.h"
#include "../../comm.h"
#include "../../globdata.h"
#include "../../ipdisp.h"
#include "../../nameref.h"
#include "../../state.h"
#include "libs/mathlib.h"


static bool GenerateSpathi_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateSpathi_generateMoons (SOLARSYS_STATE *solarSys,
		PLANET_DESC *planet);
static bool GenerateSpathi_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
static bool GenerateSpathi_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);
static bool GenerateSpathi_generateLife (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);


const GenerateFunctions generateSpathiFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateSpathi_generatePlanets,
	/* .generateMoons    = */ GenerateSpathi_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateSpathi_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateSpathi_generateEnergy,
	/* .generateLife     = */ GenerateSpathi_generateLife,
};


static bool
GenerateSpathi_generatePlanets (SOLARSYS_STATE *solarSys)
{
	PLANET_DESC *pMinPlanet;
	COUNT angle;

	pMinPlanet = &solarSys->PlanetDesc[0];
	solarSys->SunDesc[0].NumPlanets = 1;
	FillOrbits (solarSys,
			solarSys->SunDesc[0].NumPlanets, pMinPlanet, FALSE);

	pMinPlanet->radius = EARTH_RADIUS * 1150L / 100;
	angle = ARCTAN (pMinPlanet->location.x, pMinPlanet->location.y);
	pMinPlanet->location.x = COSINE (angle, pMinPlanet->radius);
	pMinPlanet->location.y = SINE (angle, pMinPlanet->radius);
	pMinPlanet->data_index = WATER_WORLD;
	if (GET_GAME_STATE (SPATHI_SHIELDED_SELVES))
		pMinPlanet->data_index |= PLANET_SHIELDED;
	pMinPlanet->NumPlanets = 1;

	return true;
}

static bool
GenerateSpathi_generateMoons (SOLARSYS_STATE *solarSys, PLANET_DESC *planet)
{
	COUNT angle;

	GenerateDefault_generateMoons (solarSys, planet);

	if (matchWorld (solarSys, planet, 0, MATCH_PLANET))
	{
#ifdef NOTYET
		utf8StringCopy (GLOBAL_SIS (PlanetName),
				sizeof (GLOBAL_SIS (PlanetName)),
				"Spathiwa");
#endif /* NOTYET */

		solarSys->MoonDesc[0].data_index = PELLUCID_WORLD;
		solarSys->MoonDesc[0].radius = MIN_MOON_RADIUS + MOON_DELTA;
		angle = NORMALIZE_ANGLE (LOWORD (TFB_Random ()));
		solarSys->MoonDesc[0].location.x =
				COSINE (angle, solarSys->MoonDesc[0].radius);
		solarSys->MoonDesc[0].location.y =
				SINE (angle, solarSys->MoonDesc[0].radius);
	}

	return true;
}

static bool
GenerateSpathi_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	DWORD rand_val;
	COUNT i;

	if (matchWorld (solarSys, world, 0, 0))
	{
		/* Spathiwa's moon */
		if (!GET_GAME_STATE (SPATHI_SHIELDED_SELVES)
				&& ActivateStarShip (SPATHI_SHIP, SPHERE_TRACKING))
		{
			NotifyOthers (SPATHI_SHIP, IPNL_ALL_CLEAR);
			PutGroupInfo (GROUPS_RANDOM, GROUP_SAVE_IP);
			ReinitQueue (&GLOBAL (ip_group_q));
			assert (CountLinks (&GLOBAL (npc_built_ship_q)) == 0);

			CloneShipFragment (SPATHI_SHIP, &GLOBAL (npc_built_ship_q),
					INFINITE_FLEET);

			SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 7);
			GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
			InitCommunication (SPATHI_CONVERSATION);

			if (!(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD)))
			{
				GLOBAL (CurrentActivity) &= ~START_INTERPLANETARY;
				ReinitQueue (&GLOBAL (npc_built_ship_q));
				GetGroupInfo (GROUPS_RANDOM, GROUP_LOAD_IP);
			}
			return true;
		}
		rand_val = DoPlanetaryAnalysis (&solarSys->SysInfo, world);

		solarSys->SysInfo.PlanetInfo.ScanSeed[BIOLOGICAL_SCAN] = rand_val;
		i = (COUNT)~0;
		rand_val = GenerateLifeForms (&solarSys->SysInfo, &i);

		solarSys->SysInfo.PlanetInfo.ScanSeed[MINERAL_SCAN] = rand_val;
		i = (COUNT)~0;
		GenerateMineralDeposits (&solarSys->SysInfo, &i);

		solarSys->SysInfo.PlanetInfo.ScanSeed[ENERGY_SCAN] = rand_val;

		solarSys->SysInfo.PlanetInfo.Weather = 0;
		solarSys->SysInfo.PlanetInfo.Tectonics = 0;
		solarSys->SysInfo.PlanetInfo.SurfaceTemperature = 28;
		if (!GET_GAME_STATE (UMGAH_BROADCASTERS))
		{
			LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
			solarSys->PlanetSideFrame[1] =
					CaptureDrawable (LoadGraphic (UMGAH_BCS_MASK_PMAP_ANIM));
			solarSys->SysInfo.PlanetInfo.DiscoveryString =
					CaptureStringTable (LoadStringTable (UMGAH_BCS_STRTAB));
		}
		LoadPlanet (NULL);
		return true;
	}
	else if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		/* visiting Spathiwa */
		rand_val = DoPlanetaryAnalysis (&solarSys->SysInfo, world);

		solarSys->SysInfo.PlanetInfo.ScanSeed[MINERAL_SCAN] = rand_val;
		i = (COUNT)~0;
		rand_val = GenerateMineralDeposits (&solarSys->SysInfo, &i);

		solarSys->SysInfo.PlanetInfo.ScanSeed[BIOLOGICAL_SCAN] = rand_val;

		solarSys->SysInfo.PlanetInfo.PlanetRadius = 120;
		solarSys->SysInfo.PlanetInfo.SurfaceGravity =
				CalcGravity (solarSys->SysInfo.PlanetInfo.PlanetDensity,
				solarSys->SysInfo.PlanetInfo.PlanetRadius);
		solarSys->SysInfo.PlanetInfo.Weather = 0;
		solarSys->SysInfo.PlanetInfo.Tectonics = 0;
		solarSys->SysInfo.PlanetInfo.SurfaceTemperature = 31;

		LoadPlanet (NULL);
		return true;
	}
	
	GenerateDefault_generateOrbital (solarSys, world);

	return true;
}

static bool
GenerateSpathi_generateEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT *whichNode)
{
	if (matchWorld (solarSys, world, 0, 0)
			&& !GET_GAME_STATE (UMGAH_BROADCASTERS))
	{
		DWORD rand_val;
		DWORD old_rand;

		old_rand = TFB_SeedRandom (
				solarSys->SysInfo.PlanetInfo.ScanSeed[ENERGY_SCAN]);

		rand_val = TFB_Random ();
		solarSys->SysInfo.PlanetInfo.CurPt.x =
				(LOBYTE (LOWORD (rand_val)) % (MAP_WIDTH - (8 << 1))) + 8;
		solarSys->SysInfo.PlanetInfo.CurPt.y =
				(HIBYTE (LOWORD (rand_val)) % (MAP_HEIGHT - (8 << 1))) + 8;
		solarSys->SysInfo.PlanetInfo.CurDensity = 0;
		solarSys->SysInfo.PlanetInfo.CurType = 0;
		if (!(solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
				& (1L << 0))
				&& *whichNode == (COUNT)~0)
			*whichNode = 1;
		else
		{
			*whichNode = 0;
			if (solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
					& (1L << 0))
			{
				SET_GAME_STATE (UMGAH_BROADCASTERS, 1);
				SET_GAME_STATE (UMGAH_BROADCASTERS_ON_SHIP, 1);
			}
		}

		TFB_SeedRandom (old_rand);
		return true;
	}

	*whichNode = 0;
	return true;
}

static bool
GenerateSpathi_generateLife (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT *whichNode)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET)
			&& !GET_GAME_STATE (SPATHI_SHIELDED_SELVES))
	{
		COUNT i;
		COUNT nodeI;
		DWORD old_rand;

		old_rand = TFB_SeedRandom (
				solarSys->SysInfo.PlanetInfo.ScanSeed[BIOLOGICAL_SCAN]);

		nodeI = 0;
		i = 0;
		do
		{
			DWORD rand_val = TFB_Random ();
			solarSys->SysInfo.PlanetInfo.CurPt.x =
					(LOBYTE (LOWORD (rand_val)) % (MAP_WIDTH - (8 << 1))) + 8;
			solarSys->SysInfo.PlanetInfo.CurPt.y =
					(HIBYTE (LOWORD (rand_val)) % (MAP_HEIGHT - (8 << 1))) + 8;
			solarSys->SysInfo.PlanetInfo.CurType = NUM_CREATURE_TYPES;
			if (nodeI >= *whichNode
					&& !(solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[BIOLOGICAL_SCAN]
					& (1L << i)))
				break;
			++nodeI;
		} while (++i < 32);
		*whichNode = nodeI;

		if (solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[BIOLOGICAL_SCAN])
		{
			SET_GAME_STATE (SPATHI_CREATURES_EXAMINED, 1);
			if (solarSys->SysInfo.
					PlanetInfo.ScanRetrieveMask[BIOLOGICAL_SCAN] == 0xFFFFFFFF)
				SET_GAME_STATE (SPATHI_CREATURES_ELIMINATED, 1);
		}

		TFB_SeedRandom (old_rand);
		return true;
	}

	*whichNode = 0;
	return true;
}


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
#include "../../comm.h"
#include "../../encount.h"
#include "../../globdata.h"
#include "../../ipdisp.h"
#include "../../nameref.h"
#include "../../setup.h"
#include "../../state.h"
#include "libs/mathlib.h"


static bool GenerateThraddash_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateThraddash_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
static bool GenerateThraddash_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);


const GenerateFunctions generateThraddashFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateThraddash_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateThraddash_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateThraddash_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
};


static bool
GenerateThraddash_generatePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT angle;

	GenerateDefault_generatePlanets (solarSys);

	if (CurStarDescPtr->Index == AQUA_HELIX_DEFINED)
	{
		solarSys->PlanetDesc[0].data_index = PRIMORDIAL_WORLD;
		solarSys->PlanetDesc[0].radius = EARTH_RADIUS * 65L / 100;
		angle = ARCTAN (solarSys->PlanetDesc[0].location.x,
				solarSys->PlanetDesc[0].location.y);
		solarSys->PlanetDesc[0].location.x =
				COSINE (angle, solarSys->PlanetDesc[0].radius);
		solarSys->PlanetDesc[0].location.y =
				SINE (angle, solarSys->PlanetDesc[0].radius);
	}
	else  /* CurStarDescPtr->Index == THRADD_DEFINED */
	{
		solarSys->PlanetDesc[0].data_index = WATER_WORLD;
		solarSys->PlanetDesc[0].NumPlanets = 0;
		solarSys->PlanetDesc[0].radius = EARTH_RADIUS * 98L / 100;
		angle = ARCTAN (solarSys->PlanetDesc[0].location.x,
				solarSys->PlanetDesc[0].location.y);
		solarSys->PlanetDesc[0].location.x =
				COSINE (angle, solarSys->PlanetDesc[0].radius);
		solarSys->PlanetDesc[0].location.y =
				SINE (angle, solarSys->PlanetDesc[0].radius);
	}
	return true;
}

static bool
GenerateThraddash_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		if (ActivateStarShip (THRADDASH_SHIP, SPHERE_TRACKING)
				&& (CurStarDescPtr->Index == THRADD_DEFINED
				|| (!GET_GAME_STATE (HELIX_UNPROTECTED)
				&& (BYTE)(GET_GAME_STATE (THRADD_MISSION) - 1) >= 3)))
		{
			NotifyOthers (THRADDASH_SHIP, IPNL_ALL_CLEAR);
			PutGroupInfo (GROUPS_RANDOM, GROUP_SAVE_IP);
			ReinitQueue (&GLOBAL (ip_group_q));
			assert (CountLinks (&GLOBAL (npc_built_ship_q)) == 0);

			CloneShipFragment (THRADDASH_SHIP, &GLOBAL (npc_built_ship_q),
					INFINITE_FLEET);

			GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
			if (CurStarDescPtr->Index == THRADD_DEFINED)
			{
				SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 7);
			}
			else
			{
				SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 6);
			}
			InitCommunication (THRADD_CONVERSATION);

			if (GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD))
				return true;

			GLOBAL (CurrentActivity) &= ~START_INTERPLANETARY;
			ReinitQueue (&GLOBAL (npc_built_ship_q));
			GetGroupInfo (GROUPS_RANDOM, GROUP_LOAD_IP);

			if (CurStarDescPtr->Index == THRADD_DEFINED
					|| (!GET_GAME_STATE (HELIX_UNPROTECTED)
					&& (BYTE)(GET_GAME_STATE (THRADD_MISSION) - 1) >= 3))
				return true;

			LockMutex (GraphicsLock);
			RepairSISBorder ();
			UnlockMutex (GraphicsLock);
		}

		if (CurStarDescPtr->Index == AQUA_HELIX_DEFINED
				&& !GET_GAME_STATE (AQUA_HELIX))
		{
			LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
			solarSys->PlanetSideFrame[1] =
					CaptureDrawable (LoadGraphic (AQUA_MASK_PMAP_ANIM));
			solarSys->SysInfo.PlanetInfo.DiscoveryString =
					CaptureStringTable (LoadStringTable (AQUA_STRTAB));
		}
		else if (CurStarDescPtr->Index == THRADD_DEFINED)
		{
			LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
			solarSys->PlanetSideFrame[1] =
					CaptureDrawable (LoadGraphic (RUINS_MASK_PMAP_ANIM));
			solarSys->SysInfo.PlanetInfo.DiscoveryString =
					CaptureStringTable (LoadStringTable (RUINS_STRTAB));
		}
	}

	GenerateDefault_generateOrbital (solarSys, world);
	return true;
}

static bool
GenerateThraddash_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		DWORD rand_val;
		DWORD old_rand;

		if (CurStarDescPtr->Index != AQUA_HELIX_DEFINED)
		{
			COUNT i;
			COUNT nodeI;

			old_rand = TFB_SeedRandom (
					solarSys->SysInfo.PlanetInfo.ScanSeed[ENERGY_SCAN]);

			nodeI = 0;
			i = 0;
			do
			{
				rand_val = TFB_Random ();
				solarSys->SysInfo.PlanetInfo.CurPt.x =
						(LOBYTE (LOWORD (rand_val)) % (MAP_WIDTH - (8 << 1))) + 8;
				solarSys->SysInfo.PlanetInfo.CurPt.y =
						(HIBYTE (LOWORD (rand_val)) % (MAP_HEIGHT - (8 << 1))) + 8;
				solarSys->SysInfo.PlanetInfo.CurType = 1;
				solarSys->SysInfo.PlanetInfo.CurDensity = 0;
				if (nodeI >= *whichNode
						&& !(solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
						& (1L << i)))
					break;
				++nodeI;
			} while (++i < 16);
			*whichNode = nodeI;

			TFB_SeedRandom (old_rand);
			return true;
		}

		if (!GET_GAME_STATE (AQUA_HELIX))
		{
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
					& (1L << 0)) && *whichNode == (COUNT)~0)
				*whichNode = 1;
			else
			{
				*whichNode = 0;
				if (solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
						& (1L << 0))
				{
					SET_GAME_STATE (HELIX_VISITS, 0);
					SET_GAME_STATE (AQUA_HELIX, 1);
					SET_GAME_STATE (AQUA_HELIX_ON_SHIP, 1);
					SET_GAME_STATE (HELIX_UNPROTECTED, 1);
				}
			}

			TFB_SeedRandom (old_rand);
			return true;
		}
	}

	*whichNode = 0;
	return true;
}


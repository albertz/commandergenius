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
#include "../lander.h"
#include "../lifeform.h"
#include "../planets.h"
#include "../../build.h"
#include "../../comm.h"
#include "../../encount.h"
#include "../../globdata.h"
#include "../../ipdisp.h"
#include "../../nameref.h"
#include "../../setup.h"
#include "../../sounds.h"
#include "../../state.h"
#include "libs/mathlib.h"


static bool GenerateVux_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateVux_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
static bool GenerateVux_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);
static bool GenerateVux_generateLife (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);


const GenerateFunctions generateVuxFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateVux_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateVux_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateVux_generateEnergy,
	/* .generateLife     = */ GenerateVux_generateLife,
};


static bool
GenerateVux_generatePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT angle;

	GenerateDefault_generatePlanets (solarSys);

	if (CurStarDescPtr->Index == MAIDENS_DEFINED)
	{
		GenerateDefault_generatePlanets (solarSys);
				// XXX: this is the second time that this function is
				// called. Is it safe to remove one, or does this change
				// the RNG so that the outcome is different?
		solarSys->PlanetDesc[0].data_index = REDUX_WORLD;
		solarSys->PlanetDesc[0].radius = EARTH_RADIUS * 212L / 100;
		angle = ARCTAN (solarSys->PlanetDesc[0].location.x,
				solarSys->PlanetDesc[0].location.y);
		solarSys->PlanetDesc[0].location.x =
				COSINE (angle, solarSys->PlanetDesc[0].radius);
		solarSys->PlanetDesc[0].location.y =
				SINE (angle, solarSys->PlanetDesc[0].radius);
	}
	else
	{
		if (CurStarDescPtr->Index == VUX_DEFINED)
		{
			solarSys->PlanetDesc[0].data_index = REDUX_WORLD;
			solarSys->PlanetDesc[0].NumPlanets = 1;
			solarSys->PlanetDesc[0].radius = EARTH_RADIUS * 42L / 100;
			angle = HALF_CIRCLE + OCTANT;
		}
		else /* if (CurStarDescPtr->Index == VUX_BEAST_DEFINED) */
		{
			memmove (&solarSys->PlanetDesc[1], &solarSys->PlanetDesc[0],
					sizeof (solarSys->PlanetDesc[0])
					* solarSys->SunDesc[0].NumPlanets);
			++solarSys->SunDesc[0].NumPlanets;

			angle = HALF_CIRCLE - OCTANT;
			solarSys->PlanetDesc[0].data_index = WATER_WORLD;
			solarSys->PlanetDesc[0].radius = EARTH_RADIUS * 110L / 100;
			solarSys->PlanetDesc[0].NumPlanets = 0;
		}

		solarSys->PlanetDesc[0].location.x =
				COSINE (angle, solarSys->PlanetDesc[0].radius);
		solarSys->PlanetDesc[0].location.y =
				SINE (angle, solarSys->PlanetDesc[0].radius);
		solarSys->PlanetDesc[0].rand_seed = MAKE_DWORD (
				solarSys->PlanetDesc[0].location.x,
				solarSys->PlanetDesc[0].location.y);
	}
	return true;
}

static bool
GenerateVux_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if ((matchWorld (solarSys, world, 0, MATCH_PLANET)
			&& (CurStarDescPtr->Index == VUX_DEFINED
			|| (CurStarDescPtr->Index == MAIDENS_DEFINED
			&& !GET_GAME_STATE (ZEX_IS_DEAD))))
			&& ActivateStarShip (VUX_SHIP, SPHERE_TRACKING))
	{
		NotifyOthers (VUX_SHIP, IPNL_ALL_CLEAR);
		PutGroupInfo (GROUPS_RANDOM, GROUP_SAVE_IP);
		ReinitQueue (&GLOBAL (ip_group_q));
		assert (CountLinks (&GLOBAL (npc_built_ship_q)) == 0);

		CloneShipFragment (VUX_SHIP,
				&GLOBAL (npc_built_ship_q), INFINITE_FLEET);
		if (CurStarDescPtr->Index == VUX_DEFINED)
		{
			SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 7);
		}
		else
		{
			SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 6);
		}

		GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
		InitCommunication (VUX_CONVERSATION);

		if (GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD))
			return true;

		{
			GLOBAL (CurrentActivity) &= ~START_INTERPLANETARY;
			ReinitQueue (&GLOBAL (npc_built_ship_q));
			GetGroupInfo (GROUPS_RANDOM, GROUP_LOAD_IP);

			if (CurStarDescPtr->Index == VUX_DEFINED
					|| !GET_GAME_STATE (ZEX_IS_DEAD))
				return true;

			LockMutex (GraphicsLock);
			RepairSISBorder ();
			UnlockMutex (GraphicsLock);
		}
	}

	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		if (CurStarDescPtr->Index == MAIDENS_DEFINED)
		{
			if (!GET_GAME_STATE (SHOFIXTI_MAIDENS))
			{
				LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
				solarSys->PlanetSideFrame[1] = CaptureDrawable (
						LoadGraphic (MAIDENS_MASK_PMAP_ANIM));
				solarSys->SysInfo.PlanetInfo.DiscoveryString =
						CaptureStringTable (
						LoadStringTable (MAIDENS_STRTAB));
			}
		}
		else if (CurStarDescPtr->Index == VUX_BEAST_DEFINED)
		{
			if (!GET_GAME_STATE (VUX_BEAST))
			{
				LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
				solarSys->PlanetSideFrame[1] = 0;
				solarSys->SysInfo.PlanetInfo.DiscoveryString =
						CaptureStringTable (
						LoadStringTable (BEAST_STRTAB));
			}
		}
		else
		{
			LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
			solarSys->PlanetSideFrame[1] =
					CaptureDrawable (LoadGraphic (RUINS_MASK_PMAP_ANIM));
			solarSys->SysInfo.PlanetInfo.DiscoveryString =
					CaptureStringTable (LoadStringTable (RUINS_STRTAB));
		}
	}

	GenerateDefault_generateOrbital (solarSys, world);

	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		solarSys->SysInfo.PlanetInfo.Weather = 2;
		solarSys->SysInfo.PlanetInfo.Tectonics = 0;
	}

	return true;
}

static bool
GenerateVux_generateEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT *whichNode)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET)
			&& CurStarDescPtr->Index != VUX_BEAST_DEFINED)
	{
		if (CurStarDescPtr->Index == MAIDENS_DEFINED
				&& !GET_GAME_STATE (SHOFIXTI_MAIDENS))
		{
			solarSys->SysInfo.PlanetInfo.CurPt.x = MAP_WIDTH / 3;
			solarSys->SysInfo.PlanetInfo.CurPt.y = MAP_HEIGHT * 5 / 8;
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
					SET_GAME_STATE (SHOFIXTI_MAIDENS, 1);
					SET_GAME_STATE (MAIDENS_ON_SHIP, 1);
				}
			}
			return true;
		}

		if (CurStarDescPtr->Index == VUX_DEFINED)
		{
			COUNT i;
			COUNT nodeI;
			DWORD rand_val;
			DWORD old_rand;

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
	}

	*whichNode = 0;
	return true;
}

static bool
GenerateVux_generateLife (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT *whichNode)
{
	if (CurStarDescPtr->Index == MAIDENS_DEFINED
			&& matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		COUNT i;
		DWORD old_rand;

		old_rand = TFB_SeedRandom (
				solarSys->SysInfo.PlanetInfo.ScanSeed[BIOLOGICAL_SCAN]);

		i = 0;
		do
		{
			DWORD rand_val = TFB_Random ();
			solarSys->SysInfo.PlanetInfo.CurPt.x =
					(LOBYTE (LOWORD (rand_val)) % (MAP_WIDTH - (8 << 1))) + 8;
			solarSys->SysInfo.PlanetInfo.CurPt.y =
					(HIBYTE (LOWORD (rand_val)) % (MAP_HEIGHT - (8 << 1))) + 8;
			if (i < 4)
				solarSys->SysInfo.PlanetInfo.CurType = 9;
			else if (i < 8)
				solarSys->SysInfo.PlanetInfo.CurType = 14;
			else /* if (i < 12) */
				solarSys->SysInfo.PlanetInfo.CurType = 18;
			if (i >= *whichNode
					&& !(solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[BIOLOGICAL_SCAN]
					& (1L << i)))
				break;
		} while (++i < 12);
		*whichNode = i;

		TFB_SeedRandom (old_rand);
		return true;
	}

	if (CurStarDescPtr->Index == VUX_BEAST_DEFINED
			&& matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		COUNT i;
		DWORD old_rand;

		old_rand = TFB_SeedRandom (
				solarSys->SysInfo.PlanetInfo.ScanSeed[BIOLOGICAL_SCAN]);

		i = 0;
		do
		{
			DWORD rand_val = TFB_Random ();
			solarSys->SysInfo.PlanetInfo.CurPt.x =
					(LOBYTE (LOWORD (rand_val)) % (MAP_WIDTH - (8 << 1))) + 8;
			solarSys->SysInfo.PlanetInfo.CurPt.y =
					(HIBYTE (LOWORD (rand_val)) % (MAP_HEIGHT - (8 << 1))) + 8;
			if (i == 0)
				solarSys->SysInfo.PlanetInfo.CurType = NUM_CREATURE_TYPES + 2;
			else if (i <= 5)
					/* {SPEED_MOTIONLESS | DANGER_NORMAL, MAKE_BYTE (5, 3)}, */
				solarSys->SysInfo.PlanetInfo.CurType = 3;
			else /* if (i <= 10) */
					/* {BEHAVIOR_UNPREDICTABLE | SPEED_SLOW | DANGER_NORMAL, MAKE_BYTE (3, 8)}, */
				solarSys->SysInfo.PlanetInfo.CurType = 8;
			if (i >= *whichNode
					&& !(solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[BIOLOGICAL_SCAN]
					& (1L << i)))
				break;
			else if (i == 0
					&& (solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[BIOLOGICAL_SCAN]
					& (1L << i))
					&& !GET_GAME_STATE (VUX_BEAST))
			{
				UnbatchGraphics ();
				DoDiscoveryReport (MenuSounds);
				BatchGraphics ();
				SetLanderTakeoff ();

				SET_GAME_STATE (VUX_BEAST, 1);
				SET_GAME_STATE (VUX_BEAST_ON_SHIP, 1);
			}
		} while (++i < 11);
		*whichNode = i;

		TFB_SeedRandom (old_rand);
		return true;
	}

	GenerateDefault_generateLife (solarSys, world, whichNode);
	return true;
}


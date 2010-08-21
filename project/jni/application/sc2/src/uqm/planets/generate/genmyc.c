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


static bool GenerateMycon_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateMycon_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
static bool GenerateMycon_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);
static bool GenerateMycon_generateLife (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);


const GenerateFunctions generateMyconFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateMycon_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateMycon_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateMycon_generateEnergy,
	/* .generateLife     = */ GenerateMycon_generateLife,
};


static bool
GenerateMycon_generatePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT angle;

	GenerateDefault_generatePlanets (solarSys);

	solarSys->PlanetDesc[0].data_index = SHATTERED_WORLD;
	solarSys->PlanetDesc[0].radius = EARTH_RADIUS * 80L / 100;
	if (solarSys->PlanetDesc[0].NumPlanets > 2)
		solarSys->PlanetDesc[0].NumPlanets = 2;
	angle = ARCTAN (
			solarSys->PlanetDesc[0].location.x,
			solarSys->PlanetDesc[0].location.y);
	solarSys->PlanetDesc[0].location.x =
			COSINE (angle, solarSys->PlanetDesc[0].radius);
	solarSys->PlanetDesc[0].location.y =
			SINE (angle, solarSys->PlanetDesc[0].radius);

	return true;
}

static bool
GenerateMycon_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		if ((CurStarDescPtr->Index == MYCON_DEFINED
				|| CurStarDescPtr->Index == SUN_DEVICE_DEFINED)
				&& ActivateStarShip (MYCON_SHIP, SPHERE_TRACKING))
		{
			if (CurStarDescPtr->Index == MYCON_DEFINED
					|| !GET_GAME_STATE (SUN_DEVICE_UNGUARDED))
			{
				NotifyOthers (MYCON_SHIP, IPNL_ALL_CLEAR);
				PutGroupInfo (GROUPS_RANDOM, GROUP_SAVE_IP);
				ReinitQueue (&GLOBAL (ip_group_q));
				assert (CountLinks (&GLOBAL (npc_built_ship_q)) == 0);

				if (CurStarDescPtr->Index == MYCON_DEFINED
						|| !GET_GAME_STATE (MYCON_FELL_FOR_AMBUSH))
				{
					CloneShipFragment (MYCON_SHIP,
							&GLOBAL (npc_built_ship_q), INFINITE_FLEET);
				}
				else
				{
					COUNT i;

					for (i = 0; i < 5; ++i)
						CloneShipFragment (MYCON_SHIP,
								&GLOBAL (npc_built_ship_q), 0);
				}

				GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
				if (CurStarDescPtr->Index == MYCON_DEFINED)
				{
					SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 7);
				}
				else
				{
					SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 6);
				}
				InitCommunication (MYCON_CONVERSATION);

				if (GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD))
					return true;

				{
					BOOLEAN MyconSurvivors;

					MyconSurvivors =
							GetHeadLink (&GLOBAL (npc_built_ship_q)) != 0;

					GLOBAL (CurrentActivity) &= ~START_INTERPLANETARY;
					ReinitQueue (&GLOBAL (npc_built_ship_q));
					GetGroupInfo (GROUPS_RANDOM, GROUP_LOAD_IP);

					if (MyconSurvivors)
						return true;

					SET_GAME_STATE (SUN_DEVICE_UNGUARDED, 1);
					LockMutex (GraphicsLock);
					RepairSISBorder ();
					UnlockMutex (GraphicsLock);
				}
			}
		}

		switch (CurStarDescPtr->Index)
		{
			case SUN_DEVICE_DEFINED:
				if (!GET_GAME_STATE (SUN_DEVICE))
				{
					LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
					solarSys->PlanetSideFrame[1] =
							CaptureDrawable (
									LoadGraphic (SUN_DEVICE_MASK_PMAP_ANIM));
					solarSys->SysInfo.PlanetInfo.DiscoveryString =
							CaptureStringTable (
									LoadStringTable (SUN_DEVICE_STRTAB));
				}
				break;
			case EGG_CASE0_DEFINED:
			case EGG_CASE1_DEFINED:
			case EGG_CASE2_DEFINED:
				if (GET_GAME_STATE (KNOW_ABOUT_SHATTERED) == 0)
					SET_GAME_STATE (KNOW_ABOUT_SHATTERED, 1);

				if (!(solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
						& (1L << 0)))
				{
					LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
					solarSys->PlanetSideFrame[1] =
							CaptureDrawable (
									LoadGraphic (EGG_CASE_MASK_PMAP_ANIM));
					solarSys->SysInfo.PlanetInfo.DiscoveryString =
							CaptureStringTable (
									LoadStringTable (EGG_CASE_STRTAB));
				}
				break;
		}
	}

	GenerateDefault_generateOrbital (solarSys, world);
	return true;
}

static bool
GenerateMycon_generateEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT *whichNode)
{
	if (CurStarDescPtr->Index != MYCON_DEFINED
		&& matchWorld (solarSys, world, 0, MATCH_PLANET))
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
		if (CurStarDescPtr->Index == SUN_DEVICE_DEFINED)
		{
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
					SET_GAME_STATE (SUN_DEVICE, 1);
					SET_GAME_STATE (SUN_DEVICE_ON_SHIP, 1);
					SET_GAME_STATE (MYCON_VISITS, 0);
				}
			}
		}
		else
		{
			if (!(solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
					& (1L << 0))
					&& *whichNode == (COUNT)~0)
				*whichNode = 1;
			else
			{
				*whichNode = 0;
				if ((solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
						& (1L << 0))
						&& solarSys->SysInfo.PlanetInfo.DiscoveryString)
				{
					switch (CurStarDescPtr->Index)
					{
						case EGG_CASE0_DEFINED:
							SET_GAME_STATE (EGG_CASE0_ON_SHIP, 1);
							break;
						case EGG_CASE1_DEFINED:
							SET_GAME_STATE (EGG_CASE1_ON_SHIP, 1);
							break;
						case EGG_CASE2_DEFINED:
							SET_GAME_STATE (EGG_CASE2_ON_SHIP, 1);
							break;
					}
				}
			}
		}

		TFB_SeedRandom (old_rand);
		return true;
	}

	*whichNode = 0;
	return true;
}

static bool
GenerateMycon_generateLife (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT *whichNode)
{
	*whichNode = 0;
	(void) solarSys;
	(void) world;
	return true;
}


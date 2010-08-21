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
#include "../planets.h"
#include "../../globdata.h"
#include "../../nameref.h"
#include "../../resinst.h"
#include "libs/mathlib.h"


static bool GenerateVault_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
static bool GenerateVault_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);


const GenerateFunctions generateVaultFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateDefault_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateVault_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateVault_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
};


static bool
GenerateVault_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, 0, 0))
	{
		LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
		solarSys->PlanetSideFrame[1] =
				CaptureDrawable (LoadGraphic (VAULT_MASK_PMAP_ANIM));
		solarSys->SysInfo.PlanetInfo.DiscoveryString =
				CaptureStringTable (LoadStringTable (VAULT_STRTAB));
		if (GET_GAME_STATE (SHIP_VAULT_UNLOCKED))
		{
			solarSys->SysInfo.PlanetInfo.DiscoveryString =
					SetAbsStringTableIndex (
					solarSys->SysInfo.PlanetInfo.DiscoveryString, 2);
		}
		else if (GET_GAME_STATE (SYREEN_SHUTTLE_ON_SHIP))
		{
			solarSys->SysInfo.PlanetInfo.DiscoveryString =
					SetAbsStringTableIndex (
					solarSys->SysInfo.PlanetInfo.DiscoveryString, 1);
		}
	}

	GenerateDefault_generateOrbital (solarSys, world);
	return true;
}

static bool
GenerateVault_generateEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT *whichNode)
{
	if (matchWorld (solarSys, world, 0, 0))
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
		if (!GET_GAME_STATE (SHIP_VAULT_UNLOCKED))
			solarSys->SysInfo.PlanetInfo.CurType = 0;
		else
			solarSys->SysInfo.PlanetInfo.CurType = 1;
		*whichNode = 1;

		if (solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
				& (1L << 0))
		{
			solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
					&= ~(1L << 0);
			if (GET_GAME_STATE (SYREEN_SHUTTLE_ON_SHIP))
			{
				SetLanderTakeoff ();

				SET_GAME_STATE (SHIP_VAULT_UNLOCKED, 1);
				SET_GAME_STATE (SYREEN_SHUTTLE_ON_SHIP, 0);
				SET_GAME_STATE (SYREEN_HOME_VISITS, 0);
			}
			else if (!GET_GAME_STATE (KNOW_SYREEN_VAULT))
			{
				SET_GAME_STATE (KNOW_SYREEN_VAULT, 1);
				SET_GAME_STATE (SYREEN_HOME_VISITS, 0);
			}
		}

		TFB_SeedRandom (old_rand);
		return true;
	}

	*whichNode = 0;
	return true;
}


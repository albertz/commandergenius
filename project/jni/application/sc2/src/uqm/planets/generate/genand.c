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
#include "../../sounds.h"
#include "libs/mathlib.h"


static bool GenerateAndrosynth_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateAndrosynth_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
static bool GenerateAndrosynth_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);


const GenerateFunctions generateAndrosynthFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateAndrosynth_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateAndrosynth_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateAndrosynth_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
};


static bool
GenerateAndrosynth_generatePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT angle;

	GenerateDefault_generatePlanets (solarSys);

	solarSys->PlanetDesc[1].data_index = TELLURIC_WORLD;
	solarSys->PlanetDesc[1].radius = EARTH_RADIUS * 204L / 100;
	angle = ARCTAN (solarSys->PlanetDesc[1].location.x,
			solarSys->PlanetDesc[1].location.y);
	solarSys->PlanetDesc[1].location.x =
			COSINE (angle, solarSys->PlanetDesc[1].radius);
	solarSys->PlanetDesc[1].location.y =
			SINE (angle, solarSys->PlanetDesc[1].radius);

	return true;
}

static bool
GenerateAndrosynth_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, 1, MATCH_PLANET))
	{
		UWORD retval;

		LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
		solarSys->PlanetSideFrame[1] =
				CaptureDrawable (LoadGraphic (RUINS_MASK_PMAP_ANIM));
		solarSys->SysInfo.PlanetInfo.DiscoveryString =
				CaptureStringTable (
				LoadStringTable (ANDROSYNTH_RUINS_STRTAB));
		retval = HIWORD (
				solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]);
		while (retval)
		{
			if (retval & 1)
			{
				solarSys->SysInfo.PlanetInfo.DiscoveryString =
						SetRelStringTableIndex (
						solarSys->SysInfo.PlanetInfo.DiscoveryString, 1);
				if (GetStringTableIndex (
						solarSys->SysInfo.PlanetInfo.DiscoveryString) == 0)
				{
					DestroyStringTable (ReleaseStringTable (
							solarSys->SysInfo.PlanetInfo.DiscoveryString));
					solarSys->SysInfo.PlanetInfo.DiscoveryString = 0;
				}
			}

			retval >>= 1;
		}
	}

	GenerateDefault_generateOrbital (solarSys, world);

	if (matchWorld (solarSys, world, 1, MATCH_PLANET))
	{
		solarSys->SysInfo.PlanetInfo.AtmoDensity =
				EARTH_ATMOSPHERE * 144 / 100;
		solarSys->SysInfo.PlanetInfo.SurfaceTemperature = 28;
		solarSys->SysInfo.PlanetInfo.Weather = 1;
		solarSys->SysInfo.PlanetInfo.Tectonics = 1;
	}

	return true;
}

static bool
GenerateAndrosynth_generateEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT *whichNode)
{
	DWORD rand_val;
	DWORD old_rand;

	if (matchWorld (solarSys, world, 1, MATCH_PLANET))
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
			solarSys->SysInfo.PlanetInfo.CurType = 0;
			solarSys->SysInfo.PlanetInfo.CurDensity = 0;

			if (solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
					& (1L << i))
			{
				solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
						&= ~(1L << i);
				if (!(solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
						& (1L << (i + 16))))
				{
					SET_GAME_STATE (PLANETARY_CHANGE, 1);

					solarSys->SysInfo.PlanetInfo.ScanRetrieveMask[ENERGY_SCAN]
							|= (1L << (i + 16));
					if (solarSys->SysInfo.PlanetInfo.DiscoveryString)
					{
						UnbatchGraphics ();
						DoDiscoveryReport (MenuSounds);
						BatchGraphics ();
						solarSys->SysInfo.PlanetInfo.DiscoveryString =
								SetRelStringTableIndex (
								solarSys->SysInfo.PlanetInfo.DiscoveryString,
								1);
					}
				}
			}

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

	*whichNode = 0;
	return true;
}


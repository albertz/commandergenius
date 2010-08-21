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


static bool GenerateTalkingPet_generatePlanets (SOLARSYS_STATE *solarSys);
static bool GenerateTalkingPet_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
static bool GenerateTalkingPet_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);

static void ZapToUrquanEncounter (void);


const GenerateFunctions generateTalkingPetFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateTalkingPet_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateTalkingPet_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateTalkingPet_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
};


static bool
GenerateTalkingPet_generatePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT angle;

	GenerateDefault_generatePlanets (solarSys);

	solarSys->PlanetDesc[0].data_index = TELLURIC_WORLD;
	solarSys->PlanetDesc[0].radius = EARTH_RADIUS * 204L / 100;
	angle = ARCTAN (solarSys->PlanetDesc[0].location.x,
			solarSys->PlanetDesc[0].location.y);
	solarSys->PlanetDesc[0].location.x =
			COSINE (angle, solarSys->PlanetDesc[0].radius);
	solarSys->PlanetDesc[0].location.y =
			SINE (angle, solarSys->PlanetDesc[0].radius);

	return true;
}

static bool
GenerateTalkingPet_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET)
			&& (GET_GAME_STATE (UMGAH_ZOMBIE_BLOBBIES)
			|| !GET_GAME_STATE (TALKING_PET)
			|| ActivateStarShip (UMGAH_SHIP, SPHERE_TRACKING)))
	{
		NotifyOthers (UMGAH_SHIP, IPNL_ALL_CLEAR);
		PutGroupInfo (GROUPS_RANDOM, GROUP_SAVE_IP);
		ReinitQueue (&GLOBAL (ip_group_q));
		assert (CountLinks (&GLOBAL (npc_built_ship_q)) == 0);

		if (ActivateStarShip (UMGAH_SHIP, SPHERE_TRACKING))
		{
			GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
			SET_GAME_STATE (GLOBAL_FLAGS_AND_DATA, 1 << 7);
			if (!GET_GAME_STATE (UMGAH_ZOMBIE_BLOBBIES))
			{
				CloneShipFragment (UMGAH_SHIP,
						&GLOBAL (npc_built_ship_q), INFINITE_FLEET);
				InitCommunication (UMGAH_CONVERSATION);
			}
			else
			{
				COUNT i;

				for (i = 0; i < 10; ++i)
				{
					CloneShipFragment (UMGAH_SHIP,
							&GLOBAL (npc_built_ship_q), 0);
				}
				InitCommunication (TALKING_PET_CONVERSATION);
			}
		}

		if (!(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD)))
		{
			BOOLEAN UmgahSurvivors;

			UmgahSurvivors = GetHeadLink (
					&GLOBAL (npc_built_ship_q)) != 0;
			GLOBAL (CurrentActivity) &= ~START_INTERPLANETARY;

			if (GET_GAME_STATE (PLAYER_HYPNOTIZED))
				ZapToUrquanEncounter ();
			else if (GET_GAME_STATE (UMGAH_ZOMBIE_BLOBBIES)
					&& !UmgahSurvivors)
			{
				// Defeated the zombie fleet.
				InitCommunication (TALKING_PET_CONVERSATION);
			}
			else if (!(ActivateStarShip (UMGAH_SHIP, SPHERE_TRACKING)))
			{
				// The Kohr-Ah have destroyed the Umgah, but the
				// talking pet survived.
				InitCommunication (TALKING_PET_CONVERSATION);
			}

			ReinitQueue (&GLOBAL (npc_built_ship_q));
			GetGroupInfo (GROUPS_RANDOM, GROUP_LOAD_IP);
		}

		return true;
	}

	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		LoadStdLanderFont (&solarSys->SysInfo.PlanetInfo);
		solarSys->PlanetSideFrame[1] =
				CaptureDrawable (LoadGraphic (RUINS_MASK_PMAP_ANIM));
		solarSys->SysInfo.PlanetInfo.DiscoveryString =
				CaptureStringTable (LoadStringTable (RUINS_STRTAB));
	}

	GenerateDefault_generateOrbital (solarSys, world);

	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
		solarSys->SysInfo.PlanetInfo.Weather = 0;

	return true;
}

static bool
GenerateTalkingPet_generateEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT *whichNode)
{
	if (matchWorld (solarSys, world, 0, MATCH_PLANET))
	{
		COUNT i;
		COUNT nodeI;
		DWORD rand_val;
		COUNT old_rand;

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

	*whichNode = 0;
	return true;
}

static void
ZapToUrquanEncounter (void)
{
	HENCOUNTER hEncounter;

	if ((hEncounter = AllocEncounter ()) || (hEncounter = GetHeadEncounter ()))
	{
		SIZE dx, dy;
		ENCOUNTER *EncounterPtr;
		HFLEETINFO hStarShip;
		FLEET_INFO *TemplatePtr;
		BRIEF_SHIP_INFO *BSIPtr;

		LockEncounter (hEncounter, &EncounterPtr);

		if (hEncounter == GetHeadEncounter ())
			RemoveEncounter (hEncounter);
		memset (EncounterPtr, 0, sizeof (*EncounterPtr));

		InsertEncounter (hEncounter, GetHeadEncounter ());

		hStarShip = GetStarShipFromIndex (&GLOBAL (avail_race_q), URQUAN_SHIP);
		TemplatePtr = LockFleetInfo (&GLOBAL (avail_race_q), hStarShip);
		EncounterPtr->origin = TemplatePtr->loc;
		EncounterPtr->radius = TemplatePtr->actual_strength;
		EncounterPtr->SD.Type = URQUAN_SHIP;
		EncounterPtr->SD.Index = MAKE_BYTE (1, 0) | ONE_SHOT_ENCOUNTER;
		BSIPtr = &EncounterPtr->ShipList[0];
		BSIPtr->race_id = URQUAN_SHIP;
		BSIPtr->crew_level = TemplatePtr->crew_level;
		BSIPtr->max_crew = TemplatePtr->max_crew;
		BSIPtr->max_energy = TemplatePtr->max_energy;
		EncounterPtr->SD.star_pt.x = 5288;
		EncounterPtr->SD.star_pt.y = 4892;
		EncounterPtr->log_x = UNIVERSE_TO_LOGX (EncounterPtr->SD.star_pt.x);
		EncounterPtr->log_y = UNIVERSE_TO_LOGY (EncounterPtr->SD.star_pt.y);
		GLOBAL_SIS (log_x) = EncounterPtr->log_x;
		GLOBAL_SIS (log_y) = EncounterPtr->log_y;
		UnlockFleetInfo (&GLOBAL (avail_race_q), hStarShip);

		{
#define LOST_DAYS 15
			SleepThreadUntil (FadeScreen (FadeAllToBlack, ONE_SECOND * 2));
			LockMutex (GraphicsLock);
			MoveGameClockDays (LOST_DAYS);
			UnlockMutex (GraphicsLock);
		}

		GLOBAL (CurrentActivity) = MAKE_WORD (IN_HYPERSPACE, 0) | START_ENCOUNTER;

		dx = CurStarDescPtr->star_pt.x - EncounterPtr->SD.star_pt.x;
		dy = CurStarDescPtr->star_pt.y - EncounterPtr->SD.star_pt.y;
		dx = (SIZE)square_root ((long)dx * dx + (long)dy * dy)
				+ (FUEL_TANK_SCALE >> 1);

		LockMutex (GraphicsLock);
		DeltaSISGauges (0, -dx, 0);
		if (GLOBAL_SIS (FuelOnBoard) < 5 * FUEL_TANK_SCALE)
		{
			dx = ((5 + ((COUNT)TFB_Random () % 5)) * FUEL_TANK_SCALE)
					- (SIZE)GLOBAL_SIS (FuelOnBoard);
			DeltaSISGauges (0, dx, 0);
		}
		DrawSISMessage (NULL);
		DrawHyperCoords (EncounterPtr->SD.star_pt);
		UnlockMutex (GraphicsLock);

		UnlockEncounter (hEncounter);
	}
}


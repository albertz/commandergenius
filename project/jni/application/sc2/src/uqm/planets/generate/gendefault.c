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
#include "../../encount.h"
#include "../../gamestr.h"
#include "../../globdata.h"
#include "../../grpinfo.h"
#include "../../races.h"
#include "../../state.h"
#include "libs/mathlib.h"


bool GenerateDefault_initNpcs (SOLARSYS_STATE *solarSys);
bool GenerateDefault_reinitNpcs (SOLARSYS_STATE *solarSys);
bool GenerateDefault_uninitNpcs (SOLARSYS_STATE *solarSys);
bool GenerateDefault_generatePlanets (SOLARSYS_STATE *solarSys);
bool GenerateDefault_generateMoons (SOLARSYS_STATE *solarSys,
		PLANET_DESC *planet);
bool GenerateDefault_generateName (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
bool GenerateDefault_generateOrbital (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world);
bool GenerateDefault_generateMinerals (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);
bool GenerateDefault_generateEnergy (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);
bool GenerateDefault_generateLife (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode);

static void GeneratePlanets (SOLARSYS_STATE *system);
static void check_yehat_rebellion (void);


const GenerateFunctions generateDefaultFunctions = {
	/* .initNpcs         = */ GenerateDefault_initNpcs,
	/* .reinitNpcs       = */ GenerateDefault_reinitNpcs,
	/* .uninitNpcs       = */ GenerateDefault_uninitNpcs,
	/* .generatePlanets  = */ GenerateDefault_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateDefault_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateDefault_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
};


bool
GenerateDefault_initNpcs (SOLARSYS_STATE *solarSys)
{
	if (!GetGroupInfo (GLOBAL (BattleGroupRef), GROUP_INIT_IP))
	{
		GLOBAL (BattleGroupRef) = 0;
		BuildGroups ();
	}

	(void) solarSys;
	return true;
}

bool
GenerateDefault_reinitNpcs (SOLARSYS_STATE *solarSys)
{
	GetGroupInfo (GROUPS_RANDOM, GROUP_LOAD_IP);
	// This is not a great place to do the Yehat rebellion check, but
	// since you can start the rebellion in any star system (not just
	// the Homeworld), I could not find a better place for it.
	// At least it is better than where it was originally.
	check_yehat_rebellion ();

	(void) solarSys;
	return true;
}

bool
GenerateDefault_uninitNpcs (SOLARSYS_STATE *solarSys)
{
	PutGroupInfo (GROUPS_RANDOM, GROUP_SAVE_IP);
	ReinitQueue (&GLOBAL (npc_built_ship_q));
	ReinitQueue (&GLOBAL (ip_group_q));

	(void) solarSys;
	return true;
}

bool
GenerateDefault_generatePlanets (SOLARSYS_STATE *solarSys)
{
	FillOrbits (solarSys, (BYTE)~0, solarSys->PlanetDesc, FALSE);
	GeneratePlanets (solarSys);
	return true;
}

bool
GenerateDefault_generateMoons (SOLARSYS_STATE *solarSys, PLANET_DESC *planet)
{
	FillOrbits (solarSys, planet->NumPlanets, solarSys->MoonDesc, FALSE);
	return true;
}

bool
GenerateDefault_generateName (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	COUNT i = planetIndex (solarSys, world);
	utf8StringCopy (GLOBAL_SIS (PlanetName), sizeof (GLOBAL_SIS (PlanetName)),
			GAME_STRING (PLANET_NUMBER_BASE + (9 + 7) + i));
	SET_GAME_STATE (BATTLE_PLANET, world->data_index);

	return true;
}

bool
GenerateDefault_generateOrbital (SOLARSYS_STATE *solarSys, PLANET_DESC *world)
{
	COUNT i;
	DWORD rand_val;
	SYSTEM_INFO *sysInfo;

#ifdef DEBUG_SOLARSYS
	if (worldIsPlanet (solarSys, world))
	{
		log_add (log_Debug, "Planet index = %d",
				planetIndex (solarSys, world));
	}
	else
	{
		log_add (log_Debug, "Planet index = %d, Moon index = %d",
				planetIndex (solarSys, world),
				moonIndex (solarSys, world));
	}
#endif /* DEBUG_SOLARSYS */

	sysInfo = &solarSys->SysInfo;

	rand_val = DoPlanetaryAnalysis (sysInfo, world);

	sysInfo->PlanetInfo.ScanSeed[BIOLOGICAL_SCAN] = rand_val;
	i = (COUNT)~0;
	rand_val = GenerateLifeForms (sysInfo, &i);

	sysInfo->PlanetInfo.ScanSeed[MINERAL_SCAN] = rand_val;
	i = (COUNT)~0;
	GenerateMineralDeposits (sysInfo, &i);

	sysInfo->PlanetInfo.ScanSeed[ENERGY_SCAN] = rand_val;
	LoadPlanet (NULL);

	return true;
}

bool
GenerateDefault_generateMinerals (SOLARSYS_STATE *solarSys,
		PLANET_DESC *world, COUNT *whichNode)
{
	GenerateMineralDeposits (&solarSys->SysInfo, whichNode);
	(void) world;
	return true;
}

bool
GenerateDefault_generateEnergy (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT *whichNode)
{
	*whichNode = 0;
	(void) solarSys;
	(void) world;
	return true;
}

bool
GenerateDefault_generateLife (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT *whichNode)
{
	GenerateLifeForms (&solarSys->SysInfo, whichNode);
	(void) world;
	return true;
}


// NB. This function modifies the RNG state.
static void
GeneratePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT i;
	PLANET_DESC *planet;

	for (i = solarSys->SunDesc[0].NumPlanets,
			planet = &solarSys->PlanetDesc[0]; i; --i, ++planet)
	{
		DWORD rand_val;
		BYTE byte_val;
		BYTE num_moons;
		BYTE type;

		rand_val = TFB_Random ();
		byte_val = LOBYTE (rand_val);

		num_moons = 0;
		type = PlanData[planet->data_index & ~PLANET_SHIELDED].Type;
		switch (PLANSIZE (type))
		{
			case LARGE_ROCKY_WORLD:
				if (byte_val < 0x00FF * 25 / 100)
				{
					if (byte_val < 0x00FF * 5 / 100)
						++num_moons;
					++num_moons;
				}
				break;
			case GAS_GIANT:
				if (byte_val < 0x00FF * 90 / 100)
				{
					if (byte_val < 0x00FF * 75 / 100)
					{
						if (byte_val < 0x00FF * 50 / 100)
						{
							if (byte_val < 0x00FF * 25 / 100)
								++num_moons;
							++num_moons;
						}
						++num_moons;
					}
					++num_moons;
				}
				break;
		}
		planet->NumPlanets = num_moons;
	}
}

static void
check_yehat_rebellion (void)
{
	HIPGROUP hGroup, hNextGroup;

	// XXX: Is there a better way to do this? I could not find one.
	//   When you talk to a Yehat ship (YEHAT_SHIP) and start the rebellion,
	//   there is no battle following the comm. There is *never* a battle in
	//   an encounter with Rebels, but the group race_id (YEHAT_REBEL_SHIP)
	//   is different from Royalists (YEHAT_SHIP). There is *always* a battle
	//   in an encounter with Royalists.
	// TRANSLATION: "If the civil war has not started yet, or the player
	//   battled a ship -- bail."
	if (!GET_GAME_STATE (YEHAT_CIVIL_WAR) || EncounterRace >= 0)
		return; // not this time

	// Send Yehat groups to flee the system, but only if the player
	// has actually talked to a ship.
	for (hGroup = GetHeadLink (&GLOBAL (ip_group_q)); hGroup;
			hGroup = hNextGroup)
	{
		IP_GROUP *GroupPtr = LockIpGroup (&GLOBAL (ip_group_q), hGroup);
		hNextGroup = _GetSuccLink (GroupPtr);
		// IGNORE_FLAGSHIP was set in ipdisp.c:ip_group_collision()
		// during a collision with the flagship.
		if (GroupPtr->race_id == YEHAT_SHIP
				&& (GroupPtr->task & IGNORE_FLAGSHIP))
		{
			GroupPtr->task &= REFORM_GROUP;
			GroupPtr->task |= FLEE | IGNORE_FLAGSHIP;
			GroupPtr->dest_loc = 0;
		}
		UnlockIpGroup (&GLOBAL (ip_group_q), hGroup);
	}
}



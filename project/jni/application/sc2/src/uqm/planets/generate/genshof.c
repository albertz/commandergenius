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
#include "../../build.h"
#include "../../globdata.h"
#include "../../grpinfo.h"
#include "../../state.h"
#include "../planets.h"


static bool GenerateShofixti_initNpcs (SOLARSYS_STATE *solarSys);
static bool GenerateShofixti_reinitNpcs (SOLARSYS_STATE *solarSys);
static bool GenerateShofixti_uninitNpcs (SOLARSYS_STATE *solarSys);
static bool GenerateShofixti_generatePlanets (SOLARSYS_STATE *solarSys);

static void check_old_shofixti (void);


const GenerateFunctions generateShofixtiFunctions = {
	/* .initNpcs         = */ GenerateShofixti_initNpcs,
	/* .reinitNpcs       = */ GenerateShofixti_reinitNpcs,
	/* .uninitNpcs       = */ GenerateShofixti_uninitNpcs,
	/* .generatePlanets  = */ GenerateShofixti_generatePlanets,
	/* .generateMoons    = */ GenerateDefault_generateMoons,
	/* .generateName     = */ GenerateDefault_generateName,
	/* .generateOrbital  = */ GenerateDefault_generateOrbital,
	/* .generateMinerals = */ GenerateDefault_generateMinerals,
	/* .generateEnergy   = */ GenerateDefault_generateEnergy,
	/* .generateLife     = */ GenerateDefault_generateLife,
};


static bool
GenerateShofixti_initNpcs (SOLARSYS_STATE *solarSys)
{
	if (!GET_GAME_STATE (SHOFIXTI_RECRUITED)
			&& (!GET_GAME_STATE (SHOFIXTI_KIA)
			|| (!GET_GAME_STATE (SHOFIXTI_BRO_KIA)
			&& GET_GAME_STATE (MAIDENS_ON_SHIP))))
	{
		GLOBAL (BattleGroupRef) = GET_GAME_STATE_32 (SHOFIXTI_GRPOFFS0);
		if (GLOBAL (BattleGroupRef) == 0
				|| !GetGroupInfo (GLOBAL (BattleGroupRef), GROUP_INIT_IP))
		{
			HSHIPFRAG hStarShip;

			if (GLOBAL (BattleGroupRef) == 0)
				GLOBAL (BattleGroupRef) = ~0L;

			hStarShip = CloneShipFragment (SHOFIXTI_SHIP,
					&GLOBAL (npc_built_ship_q), 1);
			if (hStarShip)
			{	/* Set old Shofixti name; his brother if Tanaka died */
				SHIP_FRAGMENT *FragPtr = LockShipFrag (
						&GLOBAL (npc_built_ship_q), hStarShip);
				/* Name Tanaka or Katana (+1) */
				FragPtr->captains_name_index =
						NAME_OFFSET + NUM_CAPTAINS_NAMES +
						(GET_GAME_STATE (SHOFIXTI_KIA) & 1);
				UnlockShipFrag (&GLOBAL (npc_built_ship_q), hStarShip);
			}

			GLOBAL (BattleGroupRef) = PutGroupInfo (
					GLOBAL (BattleGroupRef), 1);
			ReinitQueue (&GLOBAL (npc_built_ship_q));
			SET_GAME_STATE_32 (SHOFIXTI_GRPOFFS0, GLOBAL (BattleGroupRef));
		}
	}

	// This was originally a fallthrough to REINIT_NPCS.
	// XXX: is the call to check_old_shofixti() needed?
	GenerateDefault_initNpcs (solarSys);
	check_old_shofixti ();

	return true;
}

static bool
GenerateShofixti_reinitNpcs (SOLARSYS_STATE *solarSys)
{
	GenerateDefault_reinitNpcs (solarSys);
	check_old_shofixti ();

	(void) solarSys;
	return true;
}

static bool
GenerateShofixti_uninitNpcs (SOLARSYS_STATE *solarSys)
{
	if (GLOBAL (BattleGroupRef)
			&& !GET_GAME_STATE (SHOFIXTI_RECRUITED)
			&& GetHeadLink (&GLOBAL (ip_group_q)) == 0)
	{
		if (!GET_GAME_STATE (SHOFIXTI_KIA))
		{
			SET_GAME_STATE (SHOFIXTI_KIA, 1);
			SET_GAME_STATE (SHOFIXTI_VISITS, 0);
		}
		else if (GET_GAME_STATE (MAIDENS_ON_SHIP))
		{
			SET_GAME_STATE (SHOFIXTI_BRO_KIA, 1);
		}
	}
	
	GenerateDefault_uninitNpcs (solarSys);
	return true;
}

static bool
GenerateShofixti_generatePlanets (SOLARSYS_STATE *solarSys)
{
	COUNT i;

#define NUM_PLANETS 6
	solarSys->SunDesc[0].NumPlanets = NUM_PLANETS;
	for (i = 0; i < NUM_PLANETS; ++i)
	{
		PLANET_DESC *pCurDesc = &solarSys->PlanetDesc[i];

		pCurDesc->NumPlanets = 0;
		if (i < (NUM_PLANETS >> 1))
			pCurDesc->data_index = SELENIC_WORLD;
		else
			pCurDesc->data_index = METAL_WORLD;
	}

	FillOrbits (solarSys, NUM_PLANETS, solarSys->PlanetDesc, TRUE);

	return true;
}


static void
check_old_shofixti (void)
{
	HIPGROUP hGroup;
	IP_GROUP *GroupPtr;

	if (!GLOBAL (BattleGroupRef))
		return; // nothing to check

	hGroup = GetHeadLink (&GLOBAL (ip_group_q));
	if (!hGroup)
		return; // still nothing to check

	GroupPtr = LockIpGroup (&GLOBAL (ip_group_q), hGroup);
	// REFORM_GROUP was set in ipdisp.c:ip_group_collision()
	// during a collision with the flagship.
	if (GroupPtr->race_id == SHOFIXTI_SHIP
			&& (GroupPtr->task & REFORM_GROUP)
			&& GET_GAME_STATE (SHOFIXTI_RECRUITED))
	{
		GroupPtr->task = FLEE | IGNORE_FLAGSHIP | REFORM_GROUP;
		GroupPtr->dest_loc = 0;
	}
	UnlockIpGroup (&GLOBAL (ip_group_q), hGroup);
}


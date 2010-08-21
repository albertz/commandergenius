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
#include "../../encount.h"
#include "../../globdata.h"
#include "../../state.h"
#include "libs/log.h"


static bool GenerateMelnorme_initNpcs (SOLARSYS_STATE *solarSys);

static int SelectMelnormeRefVar (void);
static DWORD GetMelnormeRef (void);
static void SetMelnormeRef (DWORD Ref);


const GenerateFunctions generateMelnormeFunctions = {
	/* .initNpcs         = */ GenerateMelnorme_initNpcs,
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


static bool
GenerateMelnorme_initNpcs (SOLARSYS_STATE *solarSys)
{
	GLOBAL (BattleGroupRef) = GetMelnormeRef ();
	if (GLOBAL (BattleGroupRef) == 0)
	{
		CloneShipFragment (MELNORME_SHIP, &GLOBAL (npc_built_ship_q), 0);
		GLOBAL (BattleGroupRef) = PutGroupInfo (GROUPS_ADD_NEW, 1);
		ReinitQueue (&GLOBAL (npc_built_ship_q));
		SetMelnormeRef (GLOBAL (BattleGroupRef));
	}

	GenerateDefault_initNpcs (solarSys);

	return true;
}


static int
SelectMelnormeRefVar (void)
{
	switch (CurStarDescPtr->Index)
	{
		case MELNORME0_DEFINED: return MELNORME0_GRPOFFS0;
		case MELNORME1_DEFINED: return MELNORME1_GRPOFFS0;
		case MELNORME2_DEFINED: return MELNORME2_GRPOFFS0;
		case MELNORME3_DEFINED: return MELNORME3_GRPOFFS0;
		case MELNORME4_DEFINED: return MELNORME4_GRPOFFS0;
		case MELNORME5_DEFINED: return MELNORME5_GRPOFFS0;
		case MELNORME6_DEFINED: return MELNORME6_GRPOFFS0;
		case MELNORME7_DEFINED: return MELNORME7_GRPOFFS0;
		case MELNORME8_DEFINED: return MELNORME8_GRPOFFS0;
		default:
			return -1;
	}
}

static DWORD
GetMelnormeRef (void)
{
	int RefVar = SelectMelnormeRefVar ();
	if (RefVar < 0)
	{
		log_add (log_Warning, "GetMelnormeRef(): reference unknown");
		return 0;
	}

	return GET_GAME_STATE_32 (RefVar);
}

static void
SetMelnormeRef (DWORD Ref)
{
	int RefVar = SelectMelnormeRefVar ();
	if (RefVar < 0)
	{
		log_add (log_Warning, "SetMelnormeRef(): reference unknown");
		return;
	}

	SET_GAME_STATE_32 (RefVar, Ref);
}


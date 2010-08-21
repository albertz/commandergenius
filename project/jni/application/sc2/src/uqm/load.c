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

#include <assert.h>

#include "load.h"

#include "build.h"
#include "libs/declib.h"
#include "encount.h"
#include "libs/file.h"
#include "globdata.h"
#include "load.h"
#include "options.h"
#include "setup.h"
#include "state.h"
#include "grpinfo.h"

#include "libs/tasklib.h"
#include "libs/log.h"
#include "libs/misc.h"

//#define DEBUG_LOAD

ACTIVITY NextActivity;

// XXX: these should handle endian conversions later
static inline COUNT
cread_8 (DECODE_REF fh, BYTE *v)
{
	BYTE t;
	if (!v) /* read value ignored */
		v = &t;
	return cread (v, 1, 1, fh);
}

static inline COUNT
cread_16 (DECODE_REF fh, UWORD *v)
{
	UWORD t;
	if (!v) /* read value ignored */
		v = &t;
	return cread (v, 2, 1, fh);
}

static inline COUNT
cread_16s (DECODE_REF fh, SWORD *v)
{
	UWORD t;
	COUNT ret;
	// value was converted to unsigned when saved
	ret = cread_16 (fh, &t);
	// unsigned to signed conversion
	if (v)
		*v = t;
	return ret;
}

static inline COUNT
cread_32 (DECODE_REF fh, DWORD *v)
{
	DWORD t;
	if (!v) /* read value ignored */
		v = &t;
	return cread (v, 4, 1, fh);
}

static inline COUNT
cread_32s (DECODE_REF fh, SDWORD *v)
{
	DWORD t;
	COUNT ret;
	// value was converted to unsigned when saved
	ret = cread_32 (fh, &t);
	// unsigned to signed conversion
	if (v)
		*v = t;
	return ret;
}

static inline COUNT
cread_ptr (DECODE_REF fh)
{
	DWORD t;
	return cread_32 (fh, &t); /* ptrs are useless in saves */
}

static inline COUNT
cread_a8 (DECODE_REF fh, BYTE *ar, COUNT count)
{
	assert (ar != NULL);
	return cread (ar, 1, count, fh) == count;
}

static inline COUNT
read_8 (void *fp, BYTE *v)
{
	BYTE t;
	if (!v) /* read value ignored */
		v = &t;
	return ReadResFile (v, 1, 1, fp);
}

static inline COUNT
read_16 (void *fp, UWORD *v)
{
	UWORD t;
	if (!v) /* read value ignored */
		v = &t;
	return ReadResFile (v, 2, 1, fp);
}

static inline COUNT
read_32 (void *fp, DWORD *v)
{
	DWORD t;
	if (!v) /* read value ignored */
		v = &t;
	return ReadResFile (v, 4, 1, fp);
}

static inline COUNT
read_32s (void *fp, SDWORD *v)
{
	DWORD t;
	COUNT ret;
	// value was converted to unsigned when saved
	ret = read_32 (fp, &t);
	// unsigned to signed conversion
	if (v)
		*v = t;
	return ret;
}

static inline COUNT
read_ptr (void *fp)
{
	DWORD t;
	return read_32 (fp, &t); /* ptrs are useless in saves */
}

static inline COUNT
read_a8 (void *fp, BYTE *ar, COUNT count)
{
	assert (ar != NULL);
	return ReadResFile (ar, 1, count, fp) == count;
}

static inline COUNT
read_str (void *fp, char *str, COUNT count)
{
	// no type conversion needed for strings
	return read_a8 (fp, (BYTE *)str, count);
}

static inline COUNT
read_a16 (void *fp, UWORD *ar, COUNT count)
{
	assert (ar != NULL);

	for ( ; count > 0; --count, ++ar)
	{
		if (read_16 (fp, ar) != 1)
			return 0;
	}
	return 1;
}

static void
LoadEmptyQueue (DECODE_REF fh)
{
	COUNT num_links;

	cread_16 (fh, &num_links);
	if (num_links)
	{
		log_add (log_Error, "LoadEmptyQueue(): BUG: the queue is not empty!");
#ifdef DEBUG
		explode ();
#endif
	}
}

static void
LoadShipQueue (DECODE_REF fh, QUEUE *pQueue)
{
	COUNT num_links;

	cread_16 (fh, &num_links);

	while (num_links--)
	{
		HSHIPFRAG hStarShip;
		SHIP_FRAGMENT *FragPtr;
		COUNT Index;
		BYTE tmpb;

		cread_16 (fh, &Index);

		hStarShip = CloneShipFragment (Index, pQueue, 0);
		FragPtr = LockShipFrag (pQueue, hStarShip);

		// Read SHIP_FRAGMENT elements
		cread_16 (fh, NULL); /* unused: was which_side */
		cread_8  (fh, &FragPtr->captains_name_index);
		cread_8  (fh, NULL); /* padding */
		cread_16 (fh, NULL); /* unused: was ship_flags */
		cread_8  (fh, &FragPtr->race_id);
		cread_8  (fh, &FragPtr->index);
		// XXX: reading crew as BYTE to maintain savegame compatibility
		cread_8  (fh, &tmpb);
		FragPtr->crew_level = tmpb;
		cread_8  (fh, &tmpb);
		FragPtr->max_crew = tmpb;
		cread_8  (fh, &FragPtr->energy_level);
		cread_8  (fh, &FragPtr->max_energy);
		cread_16 (fh, NULL); /* unused; was loc.x */
		cread_16 (fh, NULL); /* unused; was loc.y */

		UnlockShipFrag (pQueue, hStarShip);
	}
}

static void
LoadRaceQueue (DECODE_REF fh, QUEUE *pQueue)
{
	COUNT num_links;

	cread_16 (fh, &num_links);

	while (num_links--)
	{
		HFLEETINFO hStarShip;
		FLEET_INFO *FleetPtr;
		COUNT Index;
		BYTE tmpb;

		cread_16 (fh, &Index);

		hStarShip = GetStarShipFromIndex (pQueue, Index);
		FleetPtr = LockFleetInfo (pQueue, hStarShip);

		// Read FLEET_INFO elements
		cread_16 (fh, &FleetPtr->allied_state);
		cread_8  (fh, &FleetPtr->days_left);
		cread_8  (fh, &FleetPtr->growth_fract);
		cread_8  (fh, &tmpb);
		FleetPtr->crew_level = tmpb;
		cread_8  (fh, &tmpb);
		FleetPtr->max_crew = tmpb;
		cread_8  (fh, &FleetPtr->growth);
		cread_8  (fh, &FleetPtr->max_energy);
		cread_16s(fh, &FleetPtr->loc.x);
		cread_16s(fh, &FleetPtr->loc.y);

		cread_16 (fh, &FleetPtr->actual_strength);
		cread_16 (fh, &FleetPtr->known_strength);
		cread_16s(fh, &FleetPtr->known_loc.x);
		cread_16s(fh, &FleetPtr->known_loc.y);
		cread_8  (fh, &FleetPtr->growth_err_term);
		cread_8  (fh, &FleetPtr->func_index);
		cread_16s(fh, &FleetPtr->dest_loc.x);
		cread_16s(fh, &FleetPtr->dest_loc.y);
		cread_16 (fh, NULL); /* alignment padding */

		UnlockFleetInfo (pQueue, hStarShip);
	}
}

static void
LoadGroupQueue (DECODE_REF fh, QUEUE *pQueue)
{
	COUNT num_links;

	cread_16 (fh, &num_links);

	while (num_links--)
	{
		HIPGROUP hGroup;
		IP_GROUP *GroupPtr;
		BYTE tmpb;

		cread_16 (fh, NULL); /* unused; was race_id */

		hGroup = BuildGroup (pQueue, 0);
		GroupPtr = LockIpGroup (pQueue, hGroup);

		cread_16 (fh, NULL); /* unused; was which_side */
		cread_8  (fh, NULL); /* unused; was captains_name_index */
		cread_8  (fh, NULL); /* padding; for savegame compat */
		cread_16 (fh, &GroupPtr->group_counter);
		cread_8  (fh, &GroupPtr->race_id);
		cread_8  (fh, &tmpb); /* was var2 */
		GroupPtr->sys_loc = LONIBBLE (tmpb);
		GroupPtr->task = HINIBBLE (tmpb);
		cread_8  (fh, &GroupPtr->in_system); /* was crew_level */
		cread_8  (fh, NULL); /* unused; was max_crew */
		cread_8  (fh, &tmpb); /* was energy_level */
		GroupPtr->dest_loc = LONIBBLE (tmpb);
		GroupPtr->orbit_pos = HINIBBLE (tmpb);
		cread_8  (fh, &GroupPtr->group_id); /* was max_energy */
		cread_16s(fh, &GroupPtr->loc.x);
		cread_16s(fh, &GroupPtr->loc.y);

		UnlockIpGroup (pQueue, hGroup);
	}
}

static void
LoadEncounter (ENCOUNTER *EncounterPtr, DECODE_REF fh)
{
	COUNT i;

	cread_ptr (fh); /* useless ptr; HENCOUNTER pred */
	EncounterPtr->pred = 0;
	cread_ptr (fh); /* useless ptr; HENCOUNTER succ */
	EncounterPtr->succ = 0;
	cread_ptr (fh); /* useless ptr; HELEMENT hElement */
	EncounterPtr->hElement = 0;
	cread_16s (fh, &EncounterPtr->transition_state);
	cread_16s (fh, &EncounterPtr->origin.x);
	cread_16s (fh, &EncounterPtr->origin.y);
	cread_16  (fh, &EncounterPtr->radius);
	// STAR_DESC fields
	cread_16s (fh, &EncounterPtr->SD.star_pt.x);
	cread_16s (fh, &EncounterPtr->SD.star_pt.y);
	cread_8   (fh, &EncounterPtr->SD.Type);
	cread_8   (fh, &EncounterPtr->SD.Index);
	cread_16  (fh, NULL); /* alignment padding */

	// Load each entry in the BRIEF_SHIP_INFO array
	for (i = 0; i < MAX_HYPER_SHIPS; i++)
	{
		BRIEF_SHIP_INFO *ShipInfo = &EncounterPtr->ShipList[i];
		BYTE tmpb;

		cread_16  (fh, NULL); /* useless; was SHIP_INFO.ship_flags */
		cread_8   (fh, &ShipInfo->race_id);
		cread_8   (fh, NULL); /* useless; was SHIP_INFO.var2 */
		// XXX: reading crew as BYTE to maintain savegame compatibility
		cread_8   (fh, &tmpb);
		ShipInfo->crew_level = tmpb;
		cread_8   (fh, &tmpb);
		ShipInfo->max_crew = tmpb;
		cread_8   (fh, NULL); /* useless; was SHIP_INFO.energy_level */
		cread_8   (fh, &ShipInfo->max_energy);
		cread_16  (fh, NULL); /* useless; was SHIP_INFO.loc.x */
		cread_16  (fh, NULL); /* useless; was SHIP_INFO.loc.y */
		cread_32  (fh, NULL); /* useless val; STRING race_strings */
		cread_ptr (fh); /* useless ptr; FRAME icons */
		cread_ptr (fh); /* useless ptr; FRAME melee_icon */
	}
	
	// Load the stuff after the BRIEF_SHIP_INFO array
	cread_32s (fh, &EncounterPtr->log_x);
	cread_32s (fh, &EncounterPtr->log_y);
}

static void
LoadEvent (EVENT *EventPtr, DECODE_REF fh)
{
	cread_ptr (fh); /* useless ptr; HEVENT pred */
	EventPtr->pred = 0;
	cread_ptr (fh); /* useless ptr; HEVENT succ */
	EventPtr->succ = 0;
	cread_8   (fh, &EventPtr->day_index);
	cread_8   (fh, &EventPtr->month_index);
	cread_16  (fh, &EventPtr->year_index);
	cread_8   (fh, &EventPtr->func_index);
	cread_8   (fh, NULL); /* padding */
	cread_16  (fh, NULL); /* padding */
}

static void
DummyLoadQueue (QUEUE *QueuePtr, DECODE_REF fh)
{
	/* QUEUE should never actually be loaded since it contains
	 * purely internal representation and the lists
	 * involved are actually loaded separately */
	(void)QueuePtr; /* silence compiler */

	/* QUEUE format with QUEUE_TABLE defined -- UQM default */
	cread_ptr (fh); /* HLINK head */
	cread_ptr (fh); /* HLINK tail */
	cread_ptr (fh); /* BYTE* pq_tab */
	cread_ptr (fh); /* HLINK free_list */
	cread_16  (fh, NULL); /* MEM_HANDLE hq_tab */
	cread_16  (fh, NULL); /* COUNT object_size */
	cread_8   (fh, NULL); /* BYTE num_objects */
	
	cread_8   (fh, NULL); /* padding */
	cread_16  (fh, NULL); /* padding */
}

static void
LoadClockState (CLOCK_STATE *ClockPtr, DECODE_REF fh)
{
	cread_8   (fh, &ClockPtr->day_index);
	cread_8   (fh, &ClockPtr->month_index);
	cread_16  (fh, &ClockPtr->year_index);
	cread_16s (fh, &ClockPtr->tick_count);
	cread_16s (fh, &ClockPtr->day_in_ticks);
	cread_ptr (fh); /* not loading ptr; Semaphore clock_sem */
	cread_ptr (fh); /* not loading ptr; Task clock_task */
	cread_32  (fh, NULL); /* not loading; DWORD TimeCounter */

	DummyLoadQueue (&ClockPtr->event_q, fh);
}

static void
LoadGameState (GAME_STATE *GSPtr, DECODE_REF fh)
{
	BYTE dummy8;

	cread_8   (fh, &dummy8); /* obsolete */
	cread_8   (fh, &GSPtr->glob_flags);
	cread_8   (fh, &GSPtr->CrewCost);
	cread_8   (fh, &GSPtr->FuelCost);
	cread_a8  (fh, GSPtr->ModuleCost, NUM_MODULES);
	cread_a8  (fh, GSPtr->ElementWorth, NUM_ELEMENT_CATEGORIES);
	cread_ptr (fh); /* not loading ptr; PRIMITIVE *DisplayArray */
	cread_16  (fh, &GSPtr->CurrentActivity);
	
	cread_16  (fh, NULL); /* CLOCK_STATE alignment padding */
	LoadClockState (&GSPtr->GameClock, fh);

	cread_16s (fh, &GSPtr->autopilot.x);
	cread_16s (fh, &GSPtr->autopilot.y);
	cread_16s (fh, &GSPtr->ip_location.x);
	cread_16s (fh, &GSPtr->ip_location.y);
	/* STAMP ShipStamp */
	cread_16s (fh, &GSPtr->ShipStamp.origin.x);
	cread_16s (fh, &GSPtr->ShipStamp.origin.y);
	cread_16  (fh, &GSPtr->ShipFacing);
	cread_8   (fh, &GSPtr->ip_planet);
	cread_8   (fh, &GSPtr->in_orbit);

	/* VELOCITY_DESC velocity */
	cread_16  (fh, &GSPtr->velocity.TravelAngle);
	cread_16s (fh, &GSPtr->velocity.vector.width);
	cread_16s (fh, &GSPtr->velocity.vector.height);
	cread_16s (fh, &GSPtr->velocity.fract.width);
	cread_16s (fh, &GSPtr->velocity.fract.height);
	cread_16s (fh, &GSPtr->velocity.error.width);
	cread_16s (fh, &GSPtr->velocity.error.height);
	cread_16s (fh, &GSPtr->velocity.incr.width);
	cread_16s (fh, &GSPtr->velocity.incr.height);
	cread_16  (fh, NULL); /* VELOCITY_DESC padding */

	cread_32  (fh, &GSPtr->BattleGroupRef);
	
	DummyLoadQueue (&GSPtr->avail_race_q, fh);
	DummyLoadQueue (&GSPtr->npc_built_ship_q, fh);
	// Not loading ip_group_q, was not there originally
	DummyLoadQueue (&GSPtr->encounter_q, fh);
	DummyLoadQueue (&GSPtr->built_ship_q, fh);

	cread_a8  (fh, GSPtr->GameState, sizeof (GSPtr->GameState));

	assert (sizeof (GSPtr->GameState) % 4 == 3);
	cread_8  (fh, NULL); /* GAME_STATE alignment padding */
}

static BOOLEAN
LoadSisState (SIS_STATE *SSPtr, void *fp)
{
	if (
			read_32s (fp, &SSPtr->log_x) != 1 ||
			read_32s (fp, &SSPtr->log_y) != 1 ||
			read_32  (fp, &SSPtr->ResUnits) != 1 ||
			read_32  (fp, &SSPtr->FuelOnBoard) != 1 ||
			read_16  (fp, &SSPtr->CrewEnlisted) != 1 ||
			read_16  (fp, &SSPtr->TotalElementMass) != 1 ||
			read_16  (fp, &SSPtr->TotalBioMass) != 1 ||
			read_a8  (fp, SSPtr->ModuleSlots, NUM_MODULE_SLOTS) != 1 ||
			read_a8  (fp, SSPtr->DriveSlots, NUM_DRIVE_SLOTS) != 1 ||
			read_a8  (fp, SSPtr->JetSlots, NUM_JET_SLOTS) != 1 ||
			read_8   (fp, &SSPtr->NumLanders) != 1 ||
			read_a16 (fp, SSPtr->ElementAmounts, NUM_ELEMENT_CATEGORIES) != 1 ||

			read_str (fp, SSPtr->ShipName, SIS_NAME_SIZE) != 1 ||
			read_str (fp, SSPtr->CommanderName, SIS_NAME_SIZE) != 1 ||
			read_str (fp, SSPtr->PlanetName, SIS_NAME_SIZE) != 1 ||

			read_16  (fp, NULL) != 1 /* padding */
		)
		return FALSE;
	else
		return TRUE;
}

static BOOLEAN
LoadSummary (SUMMARY_DESC *SummPtr, void *fp)
{
	if (!LoadSisState (&SummPtr->SS, fp))
		return FALSE;

	if (
			read_8  (fp, &SummPtr->Activity) != 1 ||
			read_8  (fp, &SummPtr->Flags) != 1 ||
			read_8  (fp, &SummPtr->day_index) != 1 ||
			read_8  (fp, &SummPtr->month_index) != 1 ||
			read_16 (fp, &SummPtr->year_index) != 1 ||
			read_8  (fp, &SummPtr->MCreditLo) != 1 ||
			read_8  (fp, &SummPtr->MCreditHi) != 1 ||
			read_8  (fp, &SummPtr->NumShips) != 1 ||
			read_8  (fp, &SummPtr->NumDevices) != 1 ||
			read_a8 (fp, SummPtr->ShipList, MAX_BUILT_SHIPS) != 1 ||
			read_a8 (fp, SummPtr->DeviceList, MAX_EXCLUSIVE_DEVICES) != 1 ||

			read_16  (fp, NULL) != 1 /* padding */
		)
		return FALSE;
	else
		return TRUE;
}

static void
LoadStarDesc (STAR_DESC *SDPtr, DECODE_REF fh)
{
	cread_16s(fh, &SDPtr->star_pt.x);
	cread_16s(fh, &SDPtr->star_pt.y);
	cread_8  (fh, &SDPtr->Type);
	cread_8  (fh, &SDPtr->Index);
	cread_8  (fh, &SDPtr->Prefix);
	cread_8  (fh, &SDPtr->Postfix);
}

BOOLEAN
LoadGame (COUNT which_game, SUMMARY_DESC *SummPtr)
{
	uio_Stream *in_fp;
	char file[PATH_MAX];
	char buf[256];
	SUMMARY_DESC loc_sd;
	GAME_STATE_FILE *fp;
	DECODE_REF fh;
	COUNT num_links;
	STAR_DESC SD;
	ACTIVITY Activity;

	sprintf (file, "starcon2.%02u", which_game);
	in_fp = res_OpenResFile (saveDir, file, "rb");
	if (!in_fp)
		return FALSE;

	if (!LoadSummary (&loc_sd, in_fp))
	{
		log_add (log_Error, "Warning: Savegame is corrupt");
		res_CloseResFile (in_fp);
		return FALSE;
	}

	if (!SummPtr)
	{
		SummPtr = &loc_sd;
	}
	else
	{	// only need summary for displaying to user
		memcpy (SummPtr, &loc_sd, sizeof (*SummPtr));
		res_CloseResFile (in_fp);
		return TRUE;
	}

	// Crude check for big-endian/little-endian incompatibilities.
	// year_index is suitable as it's a multi-byte value within
	// a specific recognisable range.
	if (SummPtr->year_index < START_YEAR ||
			SummPtr->year_index >= START_YEAR +
			YEARS_TO_KOHRAH_VICTORY + 1 /* Utwig intervention */ +
			1 /* time to destroy all races, plenty */ +
			25 /* for cheaters */)
	{
		log_add (log_Error, "Warning: Savegame corrupt or from "
				"an incompatible platform.");
		res_CloseResFile (in_fp);
		return FALSE;
	}

	GlobData.SIS_state = SummPtr->SS;

	if ((fh = copen (in_fp, FILE_STREAM, STREAM_READ)) == 0)
	{
		res_CloseResFile (in_fp);
		return FALSE;
	}

	ReinitQueue (&GLOBAL (GameClock.event_q));
	ReinitQueue (&GLOBAL (encounter_q));
	ReinitQueue (&GLOBAL (ip_group_q));
	ReinitQueue (&GLOBAL (npc_built_ship_q));
	ReinitQueue (&GLOBAL (built_ship_q));

	memset (&GLOBAL (GameState[0]), 0, sizeof (GLOBAL (GameState)));
	Activity = GLOBAL (CurrentActivity);
	LoadGameState (&GlobData.Game_state, fh);
	NextActivity = GLOBAL (CurrentActivity);
	GLOBAL (CurrentActivity) = Activity;

	LoadRaceQueue (fh, &GLOBAL (avail_race_q));
	// START_INTERPLANETARY is only set when saving from Homeworld
	//   encounter screen. When the game is loaded, the
	//   GenerateOrbitalFunction for the current star system will
	//   create the encounter anew and populate the npc queue.
	if (!(NextActivity & START_INTERPLANETARY))
	{
		if (NextActivity & START_ENCOUNTER)
			LoadShipQueue (fh, &GLOBAL (npc_built_ship_q));
		else if (LOBYTE (NextActivity) == IN_INTERPLANETARY)
			// XXX: Technically, this queue does not need to be
			//   saved/loaded at all. IP groups will be reloaded
			//   from group state files. But the original code did,
			//   and so will we until we can prove we do not need to.
			LoadGroupQueue (fh, &GLOBAL (ip_group_q));
		else
			// XXX: The empty queue read is only needed to maintain
			//   the savegame compatibility
			LoadEmptyQueue (fh);
	}
	LoadShipQueue (fh, &GLOBAL (built_ship_q));

	// Load the game events (compressed)
	cread_16 (fh, &num_links);
	{
#ifdef DEBUG_LOAD
		log_add (log_Debug, "EVENTS:");
#endif /* DEBUG_LOAD */
		while (num_links--)
		{
			HEVENT hEvent;
			EVENT *EventPtr;

			hEvent = AllocEvent ();
			LockEvent (hEvent, &EventPtr);

			LoadEvent (EventPtr, fh);

#ifdef DEBUG_LOAD
		log_add (log_Debug, "\t%u/%u/%u -- %u",
				EventPtr->month_index,
				EventPtr->day_index,
				EventPtr->year_index,
				EventPtr->func_index);
#endif /* DEBUG_LOAD */
			UnlockEvent (hEvent);
			PutEvent (hEvent);
		}
	}

	// Load the encounters (black globes in HS/QS (compressed))
	cread_16 (fh, &num_links);
	{
		while (num_links--)
		{
			HENCOUNTER hEncounter;
			ENCOUNTER *EncounterPtr;

			hEncounter = AllocEncounter ();
			LockEncounter (hEncounter, &EncounterPtr);

			LoadEncounter (EncounterPtr, fh);

			UnlockEncounter (hEncounter);
			PutEncounter (hEncounter);
		}
	}

	// Copy the star info file from the compressed stream
	fp = OpenStateFile (STARINFO_FILE, "wb");
	if (fp)
	{
		DWORD flen;

		cread_32 (fh, &flen);
		while (flen)
		{
			COUNT num_bytes;

			num_bytes = flen >= sizeof (buf) ? sizeof (buf) : (COUNT)flen;
			cread (buf, num_bytes, 1, fh);
			WriteStateFile (buf, num_bytes, 1, fp);

			flen -= num_bytes;
		}
		CloseStateFile (fp);
	}

	// Copy the defined groupinfo file from the compressed stream
	fp = OpenStateFile (DEFGRPINFO_FILE, "wb");
	if (fp)
	{
		DWORD flen;

		cread_32 (fh, &flen);
		while (flen)
		{
			COUNT num_bytes;

			num_bytes = flen >= sizeof (buf) ? sizeof (buf) : (COUNT)flen;
			cread (buf, num_bytes, 1, fh);
			WriteStateFile (buf, num_bytes, 1, fp);

			flen -= num_bytes;
		}
		CloseStateFile (fp);
	}

	// Copy the random groupinfo file from the compressed stream
	fp = OpenStateFile (RANDGRPINFO_FILE, "wb");
	if (fp)
	{
		DWORD flen;

		cread_32 (fh, &flen);
		while (flen)
		{
			COUNT num_bytes;

			num_bytes = flen >= sizeof (buf) ? sizeof (buf) : (COUNT)flen;
			cread (buf, num_bytes, 1, fh);
			WriteStateFile (buf, num_bytes, 1, fp);

			flen -= num_bytes;
		}
		CloseStateFile (fp);
	}

	LoadStarDesc (&SD, fh);

	cclose (fh);
	res_CloseResFile (in_fp);

	EncounterGroup = 0;
	EncounterRace = -1;

	ReinitQueue (&race_q[0]);
	ReinitQueue (&race_q[1]);
	CurStarDescPtr = FindStar (NULL, &SD.star_pt, 0, 0);
	if (!(NextActivity & START_ENCOUNTER)
			&& LOBYTE (NextActivity) == IN_INTERPLANETARY)
		NextActivity |= START_INTERPLANETARY;

	return TRUE;
}



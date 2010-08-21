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

#include "../build.h"
#include "../colors.h"
#include "../controls.h"
#include "../races.h"
#include "../units.h"
#include "../sis.h"
#include "../shipcont.h"
#include "../setup.h"
#include "../sounds.h"
#include "port.h"
#include "libs/gfxlib.h"
#include "libs/tasklib.h"

#include <stdlib.h>

// Ship icon positions in status display around the flagship
static const POINT ship_pos[MAX_BUILT_SHIPS] =
{
	SUPPORT_SHIP_PTS
};

typedef struct
{
	// Ship icon positions split into (lower half) left and right (upper)
	// and sorted in the Y coord. These are used for navigation around the
	// escort positions.
	POINT shipPos[MAX_BUILT_SHIPS];
	COUNT count;
			// Number of ships
	
	POINT curShipPt;
			// Location of the currently selected escort
	FRAME curShipFrame;
			// Icon of the currently selected escort
	bool modifyingCrew;
			// true when in crew modification "sub-menu". This is simple
			// enough that it does not require a real sub-menu.
} ROSTER_STATE;

static SHIP_FRAGMENT* LockSupportShip (ROSTER_STATE *, HSHIPFRAG *phFrag);

static void
drawSupportShip (ROSTER_STATE *rosterState, bool filled)
{
	STAMP s;

	if (!rosterState->curShipFrame)
		return;

	s.origin = rosterState->curShipPt;
	s.frame = rosterState->curShipFrame;
	
	if (filled)
		DrawFilledStamp (&s);
	else
		DrawStamp (&s);
}

static void
getSupportShipIcon (ROSTER_STATE *rosterState)
{
	HSHIPFRAG hShipFrag;
	SHIP_FRAGMENT *ShipFragPtr;

	rosterState->curShipFrame = NULL;
	ShipFragPtr = LockSupportShip (rosterState, &hShipFrag);
	if (!ShipFragPtr)
		return;

	rosterState->curShipFrame = ShipFragPtr->icons;
	UnlockShipFrag (&GLOBAL (built_ship_q), hShipFrag);
}

static void
flashSupportShip (ROSTER_STATE *rosterState)
{
	static Color c = BUILD_COLOR (MAKE_RGB15_INIT (0x1F, 0x00, 0x00), 0x24);
	static TimeCount NextTime = 0;

	if (GetTimeCounter () >= NextTime)
	{
		NextTime = GetTimeCounter () + (ONE_SECOND / 15);
		
		/* The commented code out code is the old code before the switch
		 * to 24-bits colors. The current code produces very slightly
		 * different colors due to rounding errors, but the old code wasn't
		 * original anyhow, and you can't tell the difference visually.
		 * - SvdB
		if (c >= BUILD_COLOR (MAKE_RGB15 (0x1F, 0x19, 0x19), 0x24))
			c = BUILD_COLOR (MAKE_RGB15 (0x1F, 0x00, 0x00), 0x24);
		else
			c += BUILD_COLOR (MAKE_RGB15 (0x00, 0x02, 0x02), 0x00);
		*/

		if (c.g >= CC5TO8 (0x19))
		{
			c = BUILD_COLOR (MAKE_RGB15 (0x1F, 0x00, 0x00), 0x24);
		}
		else
		{
			c.g += CC5TO8 (0x02);
			c.b += CC5TO8 (0x02);
		}
		SetContextForeGroundColor (c);

		drawSupportShip (rosterState, TRUE);
	}
}

static SHIP_FRAGMENT *
LockSupportShip (ROSTER_STATE *rosterState, HSHIPFRAG *phFrag)
{
	const POINT *pship_pos;
	HSHIPFRAG hStarShip, hNextShip;

	// Lookup the current escort's location in the unsorted points list
	// to find the original escort index
	for (hStarShip = GetHeadLink (&GLOBAL (built_ship_q)),
			pship_pos = ship_pos;
			hStarShip; hStarShip = hNextShip, ++pship_pos)
	{
		SHIP_FRAGMENT *StarShipPtr;

		StarShipPtr = LockShipFrag (&GLOBAL (built_ship_q), hStarShip);

		if (pointsEqual (*pship_pos, rosterState->curShipPt))
		{
			*phFrag = hStarShip;
			return StarShipPtr;
		}

		hNextShip = _GetSuccLink (StarShipPtr);
		UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);
	}

	return NULL;
}

static void
flashSupportShipCrew (void)
{
	RECT r;

	SetContext (StatusContext);
	GetStatusMessageRect (&r);
	SetFlashRect (&r);
}

static BOOLEAN
DeltaSupportCrew (ROSTER_STATE *rosterState, SIZE crew_delta)
{
	BOOLEAN ret = FALSE;
	UNICODE buf[40];
	HFLEETINFO hTemplate;
	HSHIPFRAG hShipFrag;
	SHIP_FRAGMENT *StarShipPtr;
	FLEET_INFO *TemplatePtr;

	StarShipPtr = LockSupportShip (rosterState, &hShipFrag);
	if (!StarShipPtr)
		return FALSE;

	hTemplate = GetStarShipFromIndex (&GLOBAL (avail_race_q),
			StarShipPtr->race_id);
	TemplatePtr = LockFleetInfo (&GLOBAL (avail_race_q), hTemplate);

	StarShipPtr->crew_level += crew_delta;

	if (StarShipPtr->crew_level == 0)
		StarShipPtr->crew_level = 1;
	else if (StarShipPtr->crew_level > TemplatePtr->crew_level &&
			crew_delta > 0)
		StarShipPtr->crew_level -= crew_delta;
	else
	{
		if (StarShipPtr->crew_level >= TemplatePtr->crew_level)
			sprintf (buf, "%u", StarShipPtr->crew_level);
		else
			sprintf (buf, "%u/%u",
					StarShipPtr->crew_level,
					TemplatePtr->crew_level);

		DrawStatusMessage (buf);
		DeltaSISGauges (-crew_delta, 0, 0);
		if (crew_delta)
		{
			flashSupportShipCrew ();
		}
		ret = TRUE;
	}

	UnlockFleetInfo (&GLOBAL (avail_race_q), hTemplate);
	UnlockShipFrag (&GLOBAL (built_ship_q), hShipFrag);

	return ret;
}

static void
drawModifiedSupportShip (ROSTER_STATE *rosterState)
{
	SetContext (StatusContext);
	SetContextForeGroundColor (ROSTER_MODIFY_SHIP_COLOR);
	drawSupportShip (rosterState, TRUE);
}

static void
selectSupportShip (ROSTER_STATE *rosterState, COUNT shipIndex)
{
	rosterState->curShipPt = rosterState->shipPos[shipIndex];
	getSupportShipIcon (rosterState);
	DeltaSupportCrew (rosterState, 0);
}

static BOOLEAN
DoModifyRoster (MENU_STATE *pMS)
{
	ROSTER_STATE *rosterState = pMS->privData;
	BOOLEAN select, cancel, up, down, horiz;

	if (GLOBAL (CurrentActivity) & CHECK_ABORT)
		return FALSE;

	select = PulsedInputState.menu[KEY_MENU_SELECT];
	cancel = PulsedInputState.menu[KEY_MENU_CANCEL];
	up = PulsedInputState.menu[KEY_MENU_UP];
	down = PulsedInputState.menu[KEY_MENU_DOWN];
	// Left or right produces the same effect because there are 2 columns
	horiz = PulsedInputState.menu[KEY_MENU_LEFT] ||
			PulsedInputState.menu[KEY_MENU_RIGHT];

	if (cancel && !rosterState->modifyingCrew)
	{
		return FALSE;
	}
	else if (select || cancel)
	{
		LockMutex (GraphicsLock);
		rosterState->modifyingCrew ^= true;
		if (!rosterState->modifyingCrew)
		{
			SetFlashRect (NULL);
			SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);
		}
		else
		{
			drawModifiedSupportShip (rosterState);
			flashSupportShipCrew ();
			SetMenuSounds (MENU_SOUND_UP | MENU_SOUND_DOWN,
					MENU_SOUND_SELECT | MENU_SOUND_CANCEL);
		}
		UnlockMutex (GraphicsLock);
	}
	else if (rosterState->modifyingCrew)
	{
		SIZE delta = 0;
		BOOLEAN failed = FALSE;

		if (up)
		{
			if (GLOBAL_SIS (CrewEnlisted))
				delta = 1;
			else
				failed = TRUE;
		}
		else if (down)
		{
			if (GLOBAL_SIS (CrewEnlisted) < GetCrewPodCapacity ())
				delta = -1;
			else
				failed = TRUE;
		}
		
		if (delta != 0)
		{
			LockMutex (GraphicsLock);
			failed = !DeltaSupportCrew (rosterState, delta);
			UnlockMutex (GraphicsLock);
		}

		if (failed)
		{	// not enough room or crew
			PlayMenuSound (MENU_SOUND_FAILURE);
		}
	}
	else
	{
		COUNT NewState;
		POINT *pship_pos = rosterState->shipPos;
		COUNT top_right = (rosterState->count + 1) >> 1;

		NewState = pMS->CurState;
		
		if (rosterState->count < 2)
		{
			// no navigation allowed
		}
		else if (horiz)
		{
			if (NewState == top_right - 1)
				NewState = rosterState->count - 1;
			else if (NewState >= top_right)
			{
				NewState -= top_right;
				if (pship_pos[NewState].y < pship_pos[pMS->CurState].y)
					++NewState;
			}
			else
			{
				NewState += top_right;
				if (NewState != top_right
						&& pship_pos[NewState].y > pship_pos[pMS->CurState].y)
					--NewState;
			}
		}
		else if (down)
		{
			++NewState;
			if (NewState == rosterState->count)
				NewState = top_right;
			else if (NewState == top_right)
				NewState = 0;
		}
		else if (up)
		{
			if (NewState == 0)
				NewState = top_right - 1;
			else if (NewState == top_right)
				NewState = rosterState->count - 1;
			else
				--NewState;
		}

		LockMutex (GraphicsLock);
		BatchGraphics ();
		SetContext (StatusContext);

		if (NewState != pMS->CurState)
		{
			// Draw the previous escort in unselected state
			drawSupportShip (rosterState, FALSE);
			// Select the new one
			selectSupportShip (rosterState, NewState);
			pMS->CurState = NewState;
		}

		flashSupportShip (rosterState);

		UnbatchGraphics ();
		UnlockMutex (GraphicsLock);
	}

	SleepThread (ONE_SECOND / 30);

	return TRUE;
}

static int
compShipPos (const void *ptr1, const void *ptr2)
{
	const POINT *pt1 = (const POINT *) ptr1;
	const POINT *pt2 = (const POINT *) ptr2;

	// Ships on the left in the lower half
	if (pt1->x < pt2->x)
		return -1;
	else if (pt1->x > pt2->x)
		return 1;

	// and ordered on Y
	if (pt1->y < pt2->y)
		return -1;
	else if (pt1->y > pt2->y)
		return 1;
	else
		return 0;
}

BOOLEAN
RosterMenu (void)
{
	MENU_STATE MenuState;
	ROSTER_STATE RosterState;

	memset (&MenuState, 0, sizeof MenuState);
	MenuState.privData = &RosterState;

	memset (&RosterState, 0, sizeof RosterState);
	
	RosterState.count = CountLinks (&GLOBAL (built_ship_q));
	if (!RosterState.count)
		return FALSE;

	// Get the escort positions we will use and sort on X then Y
	assert (sizeof (RosterState.shipPos) == sizeof (ship_pos));
	memcpy (RosterState.shipPos, ship_pos, sizeof (ship_pos));
	qsort (RosterState.shipPos, RosterState.count,
			sizeof (RosterState.shipPos[0]), compShipPos);

	LockMutex (GraphicsLock);
	SetContext (StatusContext);
	selectSupportShip (&RosterState, MenuState.CurState);
	UnlockMutex (GraphicsLock);

	SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);

	MenuState.InputFunc = DoModifyRoster;
	DoInput (&MenuState, TRUE);

	LockMutex (GraphicsLock);
	SetContext (StatusContext);
	// unselect the last ship
	drawSupportShip (&RosterState, FALSE);
	DrawStatusMessage (NULL);
	UnlockMutex (GraphicsLock);

	return TRUE;
}


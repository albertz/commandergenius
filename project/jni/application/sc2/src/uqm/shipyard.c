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

#include "build.h"
#include "colors.h"
#include "controls.h"
#include "menustat.h"
#include "fmv.h"
#include "gameopt.h"
#include "gamestr.h"
#include "supermelee/melee.h"
#include "master.h"
#include "options.h"
#include "races.h"
#include "nameref.h"
#include "resinst.h"
#include "settings.h"
#include "starbase.h"
#include "setup.h"
#include "sis.h"
#include "units.h"
#include "sounds.h"
#include "libs/graphics/gfx_common.h"
#include "libs/inplib.h"


#ifdef USE_3DO_HANGAR
// 3DO 4x3 hangar layout
#	define HANGAR_SHIPS_ROW  4
#	define HANGAR_Y          64
#	define HANGAR_DY         44

static const COORD hangar_x_coords[HANGAR_SHIPS_ROW] =
{
	19, 60, 116, 157
};

#else // use PC hangar
// modified PC 6x2 hangar layout
#	define HANGAR_SHIPS_ROW  6
#	define HANGAR_Y          88
#	define HANGAR_DY         84

static const COORD hangar_x_coords[HANGAR_SHIPS_ROW] =
{
	0, 38, 76,  131, 169, 207
};
#endif // USE_3DO_HANGAR

#define HANGAR_SHIPS      12
#define HANGAR_ROWS       (HANGAR_SHIPS / HANGAR_SHIPS_ROW)
#define HANGAR_ANIM_RATE  15 // fps

enum
{
	SHIPYARD_CREW,
	SHIPYARD_SAVELOAD,
	SHIPYARD_EXIT
};

static void
animatePowerLines (MENU_STATE *pMS)
{
	static STAMP s; 
	static COLORMAP ColorMap;
	static TimeCount NextTime = 0;
	TimeCount Now = GetTimeCounter ();

	if (pMS)
	{	// Init animation
		s.origin.x = 0;
		s.origin.y = 0;
		s.frame = SetAbsFrameIndex (pMS->ModuleFrame, 24);
		ColorMap = SetAbsColorMapIndex (pMS->CurString, 0);
	}

	if (Now >= NextTime || pMS)
	{
		NextTime = Now + (ONE_SECOND / HANGAR_ANIM_RATE);

		SetColorMap (GetColorMapAddress (ColorMap));
		DrawStamp (&s);
		// Advance colomap cycle
		ColorMap = SetRelColorMapIndex (ColorMap, 1);
	}
}

static void
on_input_frame (void)
{
	CONTEXT oldContext;

	LockMutex (GraphicsLock);
	oldContext = SetContext (SpaceContext);
	animatePowerLines (NULL);
	SetContext (oldContext);
	UnlockMutex (GraphicsLock);
}

#ifdef WANT_SHIP_SPINS
static void
SpinStarShip (MENU_STATE *pMS, HFLEETINFO hStarShip)
{
	int Index;
	FLEET_INFO *FleetPtr;

	FleetPtr = LockFleetInfo (&GLOBAL (avail_race_q), hStarShip);
	Index = FindMasterShipIndex (FleetPtr->SpeciesID);
	UnlockFleetInfo (&GLOBAL (avail_race_q), hStarShip);
				
	if (Index >= 0 && Index < NUM_MELEE_SHIPS)
	{
		UnlockMutex (GraphicsLock);
		DoShipSpin (Index, pMS->hMusic);
		LockMutex (GraphicsLock);
	}
}
#endif

// Count the ships which can be built by the player.
static COUNT
GetAvailableRaceCount (void)
{
	COUNT Index;
	HFLEETINFO hStarShip, hNextShip;

	Index = 0;
	for (hStarShip = GetHeadLink (&GLOBAL (avail_race_q));
			hStarShip; hStarShip = hNextShip)
	{
		FLEET_INFO *FleetPtr;

		FleetPtr = LockFleetInfo (&GLOBAL (avail_race_q), hStarShip);
		if (FleetPtr->allied_state == GOOD_GUY)
			++Index;

		hNextShip = _GetSuccLink (FleetPtr);
		UnlockFleetInfo (&GLOBAL (avail_race_q), hStarShip);
	}

	return Index;
}

static HFLEETINFO
GetAvailableRaceFromIndex (BYTE Index)
{
	HFLEETINFO hStarShip, hNextShip;

	for (hStarShip = GetHeadLink (&GLOBAL (avail_race_q));
			hStarShip; hStarShip = hNextShip)
	{
		FLEET_INFO *FleetPtr;

		FleetPtr = LockFleetInfo (&GLOBAL (avail_race_q), hStarShip);
		if (FleetPtr->allied_state == GOOD_GUY && Index-- == 0)
		{
			UnlockFleetInfo (&GLOBAL (avail_race_q), hStarShip);
			return hStarShip;
		}

		hNextShip = _GetSuccLink (FleetPtr);
		UnlockFleetInfo (&GLOBAL (avail_race_q), hStarShip);
	}

	return 0;
}

static void
DrawRaceStrings (MENU_STATE *pMS, BYTE NewRaceItem)
{
	RECT r;
	STAMP s;
	CONTEXT OldContext;
	
	LockMutex (GraphicsLock);

	OldContext = SetContext (StatusContext);
	GetContextClipRect (&r);
	s.origin.x = RADAR_X - r.corner.x;
	s.origin.y = RADAR_Y - r.corner.y;
	r.corner.x = s.origin.x - 1;
	r.corner.y = s.origin.y - 11;
	r.extent.width = RADAR_WIDTH + 2;
	r.extent.height = 11;
	BatchGraphics ();
	ClearSISRect (CLEAR_SIS_RADAR);
	SetContextForeGroundColor (
			BUILD_COLOR (MAKE_RGB15 (0x0A, 0x0A, 0x0A), 0x08));
	DrawFilledRectangle (&r);
	r.corner = s.origin;
	r.extent.width = RADAR_WIDTH;
	r.extent.height = RADAR_HEIGHT;
	SetContextForeGroundColor (
			BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x00), 0x00));
	DrawFilledRectangle (&r);
	if (NewRaceItem != (BYTE)~0)
	{
		TEXT t;
		HFLEETINFO hStarShip;
		FLEET_INFO *FleetPtr;
		UNICODE buf[30];
		COUNT ShipCost[] =
		{
			RACE_SHIP_COST
		};

		hStarShip = GetAvailableRaceFromIndex (NewRaceItem);
		NewRaceItem = GetIndexFromStarShip (&GLOBAL (avail_race_q),
				hStarShip);
		s.frame = SetAbsFrameIndex (pMS->ModuleFrame, 3 + NewRaceItem);
		DrawStamp (&s);
		FleetPtr = LockFleetInfo (&GLOBAL (avail_race_q), hStarShip);
		s.frame = FleetPtr->melee_icon;
		UnlockFleetInfo (&GLOBAL (avail_race_q), hStarShip);

		t.baseline.x = s.origin.x + RADAR_WIDTH - 2;
		t.baseline.y = s.origin.y + RADAR_HEIGHT - 2;
		s.origin.x += (RADAR_WIDTH >> 1);
		s.origin.y += (RADAR_HEIGHT >> 1);
		DrawStamp (&s);
		t.align = ALIGN_RIGHT;
		t.CharCount = (COUNT)~0;
		t.pStr = buf;
		sprintf (buf, "%u", ShipCost[NewRaceItem]);
		SetContextFont (TinyFont);
		SetContextForeGroundColor (
				BUILD_COLOR (MAKE_RGB15 (0x00, 0x1F, 0x00), 0x02));
		font_DrawText (&t);
	}
	UnbatchGraphics ();
	SetContext (OldContext);

	// Flash the ship purchase menu even when optMenu == OPT_PC
	SetFlashRect (SFR_MENU_ANY);
	UnlockMutex (GraphicsLock);
}

#define SHIP_WIN_WIDTH 34
#define SHIP_WIN_HEIGHT (SHIP_WIN_WIDTH + 6)
#define SHIP_WIN_FRAMES ((SHIP_WIN_WIDTH >> 1) + 1)

static void
ShowShipCrew (SHIP_FRAGMENT *StarShipPtr, RECT *pRect)
{
	RECT r;
	TEXT t;
	UNICODE buf[80];
	HFLEETINFO hTemplate;
	FLEET_INFO *TemplatePtr;

	hTemplate = GetStarShipFromIndex (&GLOBAL (avail_race_q),
			StarShipPtr->race_id);
	TemplatePtr = LockFleetInfo (&GLOBAL (avail_race_q), hTemplate);
	if (StarShipPtr->crew_level >= TemplatePtr->crew_level)
		sprintf (buf, "%u", StarShipPtr->crew_level);
	else if (StarShipPtr->crew_level == 0)
		// XXX: "SCRAP" needs to be moved to starcon.txt
		utf8StringCopy (buf, sizeof (buf), "SCRAP");
	else
		sprintf (buf, "%u/%u",
				StarShipPtr->crew_level,
				TemplatePtr->crew_level);
	UnlockFleetInfo (&GLOBAL (avail_race_q), hTemplate);

	r = *pRect;
	t.baseline.x = r.corner.x + (r.extent.width >> 1);
	t.baseline.y = r.corner.y + r.extent.height - 1;
	t.align = ALIGN_CENTER;
	t.pStr = buf;
	t.CharCount = (COUNT)~0;
	if (r.corner.y)
	{
		r.corner.y = t.baseline.y - 6;
		r.extent.width = SHIP_WIN_WIDTH;
		r.extent.height = 6;
		SetContextForeGroundColor (BLACK_COLOR);
		DrawFilledRectangle (&r);
	}
	SetContextForeGroundColor ((StarShipPtr->crew_level != 0) ?
			(BUILD_COLOR (MAKE_RGB15 (0x00, 0x14, 0x00), 0x02)):
			(BUILD_COLOR (MAKE_RGB15 (0x12, 0x00, 0x00), 0x2B)));
	font_DrawText (&t);
}

static void
ShowCombatShip (MENU_STATE *pMS, COUNT which_window,
		SHIP_FRAGMENT *YankedStarShipPtr)
{
	COUNT i, num_ships;
	HSHIPFRAG hStarShip, hNextShip;
	SHIP_FRAGMENT *StarShipPtr;
	struct
	{
		SHIP_FRAGMENT *StarShipPtr;
		POINT finished_s;
		STAMP ship_s, lfdoor_s, rtdoor_s;
	} ship_win_info[MAX_BUILT_SHIPS], *pship_win_info;

	num_ships = 1;
	pship_win_info = &ship_win_info[0];
	if (YankedStarShipPtr)
	{
		pship_win_info->StarShipPtr = YankedStarShipPtr;

		pship_win_info->lfdoor_s.origin.x = -(SHIP_WIN_WIDTH >> 1);
		pship_win_info->rtdoor_s.origin.x = (SHIP_WIN_WIDTH >> 1);
		pship_win_info->lfdoor_s.origin.y = 0;
		pship_win_info->rtdoor_s.origin.y = 0;
		pship_win_info->lfdoor_s.frame = IncFrameIndex (pMS->ModuleFrame);
		pship_win_info->rtdoor_s.frame =
				IncFrameIndex (pship_win_info->lfdoor_s.frame);

		pship_win_info->ship_s.origin.x = (SHIP_WIN_WIDTH >> 1) + 1;
		pship_win_info->ship_s.origin.y = (SHIP_WIN_WIDTH >> 1);
		pship_win_info->ship_s.frame = YankedStarShipPtr->melee_icon;

		pship_win_info->finished_s.x = hangar_x_coords[
				which_window % HANGAR_SHIPS_ROW];
		pship_win_info->finished_s.y = HANGAR_Y + (HANGAR_DY *
				(which_window / HANGAR_SHIPS_ROW));
	}
	else
	{
		if (which_window == (COUNT)~0)
		{
			hStarShip = GetHeadLink (&GLOBAL (built_ship_q));
			num_ships = CountLinks (&GLOBAL (built_ship_q));
		}
		else
		{
			HSHIPFRAG hTailShip;

			hTailShip = GetTailLink (&GLOBAL (built_ship_q));
			RemoveQueue (&GLOBAL (built_ship_q), hTailShip);

			hStarShip = GetHeadLink (&GLOBAL (built_ship_q));
			while (hStarShip)
			{
				StarShipPtr = LockShipFrag (&GLOBAL (built_ship_q), hStarShip);
				if (StarShipPtr->index > which_window)
				{
					UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);
					break;
				}
				hNextShip = _GetSuccLink (StarShipPtr);
				UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);

				hStarShip = hNextShip;
			}
			InsertQueue (&GLOBAL (built_ship_q), hTailShip, hStarShip);

			hStarShip = hTailShip;
			StarShipPtr = LockShipFrag (&GLOBAL (built_ship_q), hStarShip);
			StarShipPtr->index = which_window;
			UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);
		}

		for (i = 0; i < num_ships; ++i)
		{
			StarShipPtr = LockShipFrag (&GLOBAL (built_ship_q), hStarShip);
			hNextShip = _GetSuccLink (StarShipPtr);

			pship_win_info->StarShipPtr = StarShipPtr;

			pship_win_info->lfdoor_s.origin.x = -1;
			pship_win_info->rtdoor_s.origin.x = 1;
			pship_win_info->lfdoor_s.origin.y = 0;
			pship_win_info->rtdoor_s.origin.y = 0;
			pship_win_info->lfdoor_s.frame = IncFrameIndex (pMS->ModuleFrame);
			pship_win_info->rtdoor_s.frame =
					IncFrameIndex (pship_win_info->lfdoor_s.frame);

			pship_win_info->ship_s.origin.x = (SHIP_WIN_WIDTH >> 1) + 1;
			pship_win_info->ship_s.origin.y = (SHIP_WIN_WIDTH >> 1);
			pship_win_info->ship_s.frame = StarShipPtr->melee_icon;

			which_window = StarShipPtr->index;
			pship_win_info->finished_s.x = hangar_x_coords[
					which_window % HANGAR_SHIPS_ROW];
			pship_win_info->finished_s.y = HANGAR_Y + (HANGAR_DY *
					(which_window / HANGAR_SHIPS_ROW));
			++pship_win_info;

			UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);
			hStarShip = hNextShip;
		}
	}

	if (num_ships)
	{
		BOOLEAN AllDoorsFinished;
		DWORD TimeIn;
		RECT r;
		CONTEXT OldContext;
		RECT OldClipRect;
		int j;

		AllDoorsFinished = FALSE;
		r.corner.x = r.corner.y = 0;
		r.extent.width = SHIP_WIN_WIDTH;
		r.extent.height = SHIP_WIN_HEIGHT;
		FlushInput ();
		TimeIn = GetTimeCounter ();

		for (j = 0; (j < SHIP_WIN_FRAMES) && !AllDoorsFinished; j++)
		{
			SleepThreadUntil (TimeIn + ONE_SECOND / 24);
			TimeIn = GetTimeCounter ();
			if (AnyButtonPress (FALSE))
			{
				if (YankedStarShipPtr != 0)
				{	// Fully close the doors
					ship_win_info[0].lfdoor_s.origin.x = 0;
					ship_win_info[0].rtdoor_s.origin.x = 0;
				}
				AllDoorsFinished = TRUE;
			}

			LockMutex (GraphicsLock);
			OldContext = SetContext (SpaceContext);
			GetContextClipRect (&OldClipRect);
			SetContextBackGroundColor (BLACK_COLOR);

			BatchGraphics ();
			pship_win_info = &ship_win_info[0];
			for (i = 0; i < num_ships; ++i)
			{
				{
					RECT ClipRect;

					ClipRect.corner.x = SIS_ORG_X +
							pship_win_info->finished_s.x;
					ClipRect.corner.y = SIS_ORG_Y +
							pship_win_info->finished_s.y;
					ClipRect.extent.width = SHIP_WIN_WIDTH;
					ClipRect.extent.height = SHIP_WIN_HEIGHT;
					SetContextClipRect (&ClipRect);
					
					ClearDrawable ();
					DrawStamp (&pship_win_info->ship_s);
					ShowShipCrew (pship_win_info->StarShipPtr, &r);
					if (!AllDoorsFinished || YankedStarShipPtr)
					{
						DrawStamp (&pship_win_info->lfdoor_s);
						DrawStamp (&pship_win_info->rtdoor_s);
						if (YankedStarShipPtr)
						{	// Close the doors
							++pship_win_info->lfdoor_s.origin.x;
							--pship_win_info->rtdoor_s.origin.x;
						}
						else
						{	// Open the doors
							--pship_win_info->lfdoor_s.origin.x;
							++pship_win_info->rtdoor_s.origin.x;
						}
					}
				}
				++pship_win_info;
			}

			SetContextClipRect (&OldClipRect);
#ifndef USE_3DO_HANGAR
			animatePowerLines (NULL);
#endif
			UnbatchGraphics ();
			SetContext (OldContext);
			UnlockMutex (GraphicsLock);
		}
	}
}

static void
CrewTransaction (SIZE crew_delta)
{
	if (crew_delta)
	{
		SIZE crew_bought;

		crew_bought = (SIZE)MAKE_WORD (
				GET_GAME_STATE (CREW_PURCHASED0),
				GET_GAME_STATE (CREW_PURCHASED1)) + crew_delta;
		if (crew_bought < 0)
		{
			if (crew_delta < 0)
				crew_bought = 0;
			else
				crew_bought = 0x7FFF;
		}
		else if (crew_delta > 0)
		{
			if (crew_bought >= CREW_EXPENSE_THRESHOLD
					&& crew_bought - crew_delta < CREW_EXPENSE_THRESHOLD)
			{
				GLOBAL (CrewCost) += 2;

				UnlockMutex (GraphicsLock);
				DrawMenuStateStrings (PM_CREW, SHIPYARD_CREW);
				LockMutex (GraphicsLock);
			}
		}
		else
		{
			if (crew_bought < CREW_EXPENSE_THRESHOLD
					&& crew_bought - crew_delta >= CREW_EXPENSE_THRESHOLD)
			{
				GLOBAL (CrewCost) -= 2;

				UnlockMutex (GraphicsLock);
				DrawMenuStateStrings (PM_CREW, SHIPYARD_CREW);
				LockMutex (GraphicsLock);
			}
		}
		if (ActivateStarShip (SHOFIXTI_SHIP, CHECK_ALLIANCE) != GOOD_GUY)
		{
			SET_GAME_STATE (CREW_PURCHASED0, LOBYTE (crew_bought));
			SET_GAME_STATE (CREW_PURCHASED1, HIBYTE (crew_bought));
		}
	}
}

/* in this routine, the least significant byte of pMS->CurState is used
 * to store the current selected ship index
 * a special case for the row is hi-nibble == -1 (0xf), which specifies
 * SIS as the selected ship
 * some bitwise math is still done to scroll through ships, for it to work
 * ships per row number must divide 0xf0 without remainder
 */
static BOOLEAN
DoModifyShips (MENU_STATE *pMS)
{
#define MODIFY_CREW_FLAG (1 << 8)
	BOOLEAN select, cancel;
#ifdef WANT_SHIP_SPINS
	BOOLEAN special;

	special = PulsedInputState.menu[KEY_MENU_SPECIAL];
#endif /* WANT_SHIP_SPINS */
	select = PulsedInputState.menu[KEY_MENU_SELECT];
	cancel = PulsedInputState.menu[KEY_MENU_CANCEL];

	if (GLOBAL (CurrentActivity) & CHECK_ABORT)
	{
		pMS->InputFunc = DoShipyard;
		return TRUE;
	}

	if (!pMS->Initialized)
	{
		pMS->InputFunc = DoModifyShips;
		pMS->Initialized = TRUE;
		pMS->CurState = MAKE_BYTE (0, 0xF);
		pMS->delta_item = 0;

		LockMutex (GraphicsLock);
		SetContext (SpaceContext);
		goto ChangeFlashRect;
	}
	else
	{
		SBYTE dx = 0;
		SBYTE dy = 0;
		BYTE NewState;

		if (!(pMS->delta_item & MODIFY_CREW_FLAG))
		{
			SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);
		}

		if (PulsedInputState.menu[KEY_MENU_RIGHT]) dx = 1;
		if (PulsedInputState.menu[KEY_MENU_LEFT]) dx = -1;
		if (PulsedInputState.menu[KEY_MENU_UP]) dy = -1;
		if (PulsedInputState.menu[KEY_MENU_DOWN]) dy = 1;
		NewState = pMS->CurState;
		if (pMS->delta_item & MODIFY_CREW_FLAG)
		{
		}
		else if (dy)
		{
			if (HINIBBLE (NewState))
				NewState = pMS->CurState % HANGAR_SHIPS_ROW;
			else
				NewState = (unsigned char)(pMS->CurState + HANGAR_SHIPS_ROW);

			NewState += dy * HANGAR_SHIPS_ROW;
			if (NewState / HANGAR_SHIPS_ROW > 0
					&& NewState / HANGAR_SHIPS_ROW <= HANGAR_ROWS)
				NewState -= HANGAR_SHIPS_ROW;
			else if (NewState / HANGAR_SHIPS_ROW > HANGAR_ROWS + 1)
				/* negative number - select last row */
				NewState = pMS->CurState % HANGAR_SHIPS_ROW
						+ HANGAR_SHIPS_ROW * (HANGAR_ROWS - 1);
			else
				// select SIS
				NewState = MAKE_BYTE (pMS->CurState, 0xF);
		}
		else if (dx && !HINIBBLE (NewState))
		{
			NewState = NewState % HANGAR_SHIPS_ROW;
			if ((dx += NewState) < 0)
				NewState = (BYTE)(pMS->CurState + (HANGAR_SHIPS_ROW - 1));
			else if (dx > HANGAR_SHIPS_ROW - 1)
				NewState = (BYTE)(pMS->CurState - (HANGAR_SHIPS_ROW - 1));
			else
				NewState = (BYTE)(pMS->CurState - NewState + dx);
		}

		if (select || cancel
#ifdef WANT_SHIP_SPINS
				|| special
#endif
				|| NewState != pMS->CurState
				|| ((pMS->delta_item & MODIFY_CREW_FLAG) && (dx || dy)))
		{
			HSHIPFRAG hStarShip, hNextShip;
			SHIP_FRAGMENT *StarShipPtr;
			RECT r;

			for (hStarShip = GetHeadLink (&GLOBAL (built_ship_q));
					hStarShip; hStarShip = hNextShip)
			{
				StarShipPtr = LockShipFrag (&GLOBAL (built_ship_q), hStarShip);

				if (StarShipPtr->index == pMS->CurState)
				{
					UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);
					break;
				}

				hNextShip = _GetSuccLink (StarShipPtr);
				UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);
			}
			if ((pMS->delta_item & MODIFY_CREW_FLAG) && (hStarShip))
			{
				SetMenuSounds (MENU_SOUND_UP | MENU_SOUND_DOWN,
						MENU_SOUND_SELECT | MENU_SOUND_CANCEL);
			}
			else
			{
				SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);
			}
	
			LockMutex (GraphicsLock);

#ifdef WANT_SHIP_SPINS
			if (special)
			{
				HFLEETINFO hSpinShip = 0;
				
				if ((special && (((hStarShip == 0
						   && HINIBBLE (pMS->CurState) == 0)
						  && (pMS->delta_item & MODIFY_CREW_FLAG))
						 || ((hStarShip != 0 &&
						      HINIBBLE (pMS->CurState) == 0)
						     && !(pMS->delta_item & MODIFY_CREW_FLAG))))
				    && (hStarShip
					|| (HINIBBLE (pMS->CurState) == 0
					    && (hSpinShip = GetAvailableRaceFromIndex (
							LOBYTE (pMS->delta_item))))))
				{
					CONTEXT OldContext;
					RECT OldClipRect;

					if (!hSpinShip)
					{	/* Get fleet info from selected escort */
						SHIP_FRAGMENT *FragPtr = LockShipFrag (
								&GLOBAL (built_ship_q), hStarShip);
						hSpinShip = GetStarShipFromIndex (
								&GLOBAL (avail_race_q), FragPtr->race_id);
						UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);
					}
					
					SetFlashRect (NULL);

					OldContext = SetContext (ScreenContext);
					GetContextClipRect (&OldClipRect);

					SpinStarShip (pMS, hSpinShip);

					SetContextClipRect (&OldClipRect);
					SetContext (OldContext);

					if (hStarShip)
						goto ChangeFlashRect;

					SetFlashRect (SFR_MENU_3DO);
				}
			}
			else
#endif
			if (select || ((pMS->delta_item & MODIFY_CREW_FLAG)
					&& (dx || dy || cancel)))
			{
				COUNT ShipCost[] =
				{
					RACE_SHIP_COST
				};

				if (hStarShip == 0 && HINIBBLE (pMS->CurState) == 0)
				{
					COUNT Index;

// SetFlashRect (NULL);
					UnlockMutex (GraphicsLock);
					if (!(pMS->delta_item & MODIFY_CREW_FLAG))
					{
						pMS->delta_item = MODIFY_CREW_FLAG;
						DrawRaceStrings (pMS, 0);
						return TRUE;
					}
					else if (cancel)
					{
						pMS->delta_item ^= MODIFY_CREW_FLAG;
						LockMutex (GraphicsLock);
						SetFlashRect (SFR_MENU_3DO);
						UnlockMutex (GraphicsLock);
						DrawMenuStateStrings (PM_CREW, SHIPYARD_CREW);
						SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);
					}
					else if (select)
					{
						Index = GetIndexFromStarShip (&GLOBAL (avail_race_q),
								GetAvailableRaceFromIndex (
								LOBYTE (pMS->delta_item)));

						if (GLOBAL_SIS (ResUnits) >= (DWORD)ShipCost[Index]
								&& CloneShipFragment (Index,
								&GLOBAL (built_ship_q), 1))
						{
							ShowCombatShip (pMS, pMS->CurState, NULL);
							//Reset flash rectangle
							LockMutex (GraphicsLock);
							SetFlashRect (SFR_MENU_3DO);
							UnlockMutex (GraphicsLock);
							DrawMenuStateStrings (PM_CREW, SHIPYARD_CREW);

							LockMutex (GraphicsLock);
							DeltaSISGauges (UNDEFINED_DELTA, UNDEFINED_DELTA,
									-((int)ShipCost[Index]));
							r.corner.x = pMS->flash_rect0.corner.x;
							r.corner.y = pMS->flash_rect0.corner.y
									+ pMS->flash_rect0.extent.height - 6;
							r.extent.width = SHIP_WIN_WIDTH;
							r.extent.height = 5;
							SetContext (SpaceContext);
							SetFlashRect (&r);
							UnlockMutex (GraphicsLock);
						}
						else
						{	// not enough RUs to build
							PlayMenuSound (MENU_SOUND_FAILURE);
						}
						SetMenuSounds (MENU_SOUND_UP | MENU_SOUND_DOWN,
								MENU_SOUND_SELECT | MENU_SOUND_CANCEL);
							
						return TRUE;
					}
					else
					{
						Index = GetAvailableRaceCount ();
						NewState = LOBYTE (pMS->delta_item);
						if (dx < 0 || dy < 0)
						{
							if (NewState-- == 0)
								NewState = Index - 1;
						}
						else if (dx > 0 || dy > 0)
						{
							if (++NewState == Index)
								NewState = 0;
						}
						
						if (NewState != LOBYTE (pMS->delta_item))
						{
							DrawRaceStrings (pMS, NewState);
							pMS->delta_item = NewState | MODIFY_CREW_FLAG;
						}
						
						return TRUE;
					}
					LockMutex (GraphicsLock);
					goto ChangeFlashRect;
				}
				else if (select || cancel)
				{
					if ((pMS->delta_item & MODIFY_CREW_FLAG)
							&& hStarShip != 0)
					{
						StarShipPtr = LockShipFrag (&GLOBAL (built_ship_q),
								hStarShip);
						if (StarShipPtr->crew_level == 0)
						{
							SetFlashRect (NULL);
							UnlockMutex (GraphicsLock);
							ShowCombatShip (pMS, pMS->CurState, StarShipPtr);
							LockMutex (GraphicsLock);
							UnlockShipFrag (&GLOBAL (built_ship_q),
									hStarShip);
							RemoveQueue (&GLOBAL (built_ship_q), hStarShip);
							FreeShipFrag (&GLOBAL (built_ship_q), hStarShip);
							// refresh SIS display
							DeltaSISGauges (UNDEFINED_DELTA, UNDEFINED_DELTA,
									UNDEFINED_DELTA);
							r.corner.x = pMS->flash_rect0.corner.x;
							r.corner.y = pMS->flash_rect0.corner.y;
							r.extent.width = SHIP_WIN_WIDTH;
							r.extent.height = SHIP_WIN_HEIGHT;
							SetContext (SpaceContext);
							SetFlashRect (&r);
						}
						else
						{
							UnlockShipFrag (&GLOBAL (built_ship_q),
									hStarShip);
						}
					}
					
					if (!(pMS->delta_item ^= MODIFY_CREW_FLAG))
					{
						goto ChangeFlashRect;
					}
					else if (hStarShip == 0)
					{
						SetContext (StatusContext);
						GetGaugeRect (&r, TRUE);
						SetFlashRect (&r);
						SetContext (SpaceContext);
						SetMenuSounds (MENU_SOUND_UP | MENU_SOUND_DOWN,
								MENU_SOUND_SELECT | MENU_SOUND_CANCEL);
					}
					else
					{
						r.corner.x = pMS->flash_rect0.corner.x;
						r.corner.y = pMS->flash_rect0.corner.y
								+ pMS->flash_rect0.extent.height - 6;
						r.extent.width = SHIP_WIN_WIDTH;
						r.extent.height = 5;
						SetContext (SpaceContext);
						SetFlashRect (&r);
						SetMenuSounds (MENU_SOUND_UP | MENU_SOUND_DOWN,
								MENU_SOUND_SELECT | MENU_SOUND_CANCEL);
					}
				}
				else if (pMS->delta_item & MODIFY_CREW_FLAG)
				{
					SIZE crew_delta, crew_bought;

					if (hStarShip)
						StarShipPtr = LockShipFrag (&GLOBAL (built_ship_q),
								hStarShip);
					else
						StarShipPtr = NULL;  // Keeping compiler quiet.

					SetMenuSounds (MENU_SOUND_UP | MENU_SOUND_DOWN,
							MENU_SOUND_SELECT | MENU_SOUND_CANCEL);
					crew_delta = 0;
					if (dy < 0)
					{
						if (hStarShip == 0)
						{
							if (GetCPodCapacity (&r.corner) > GetCrewCount ()
									&& GLOBAL_SIS (ResUnits) >=
									(DWORD)GLOBAL (CrewCost))
							{
								DrawPoint (&r.corner);
								DeltaSISGauges (1, 0, -GLOBAL (CrewCost));
								crew_delta = 1;

								SetContext (StatusContext);
								GetGaugeRect (&r, TRUE);
								SetFlashRect (&r);
								SetContext (SpaceContext);
							}
							else
							{	// at capacity or not enough RUs
								PlayMenuSound (MENU_SOUND_FAILURE);
							}
						}
						else
						{
							HFLEETINFO hTemplate;
							FLEET_INFO *TemplatePtr;

							hTemplate = GetStarShipFromIndex (
									&GLOBAL (avail_race_q),
									StarShipPtr->race_id);
							TemplatePtr = LockFleetInfo (
									&GLOBAL (avail_race_q), hTemplate);
							if (GLOBAL_SIS (ResUnits) >=
									(DWORD)GLOBAL (CrewCost)
									&& StarShipPtr->crew_level <
									StarShipPtr->max_crew &&
									StarShipPtr->crew_level <
									TemplatePtr->crew_level)
							{
								if (StarShipPtr->crew_level > 0)
									DeltaSISGauges (0, 0, -GLOBAL (CrewCost));
								else
									DeltaSISGauges (0, 0, -(COUNT)ShipCost[
											StarShipPtr->race_id]);
								++StarShipPtr->crew_level;
								crew_delta = 1;
								ShowShipCrew (StarShipPtr, &pMS->flash_rect0);
								r.corner.x = pMS->flash_rect0.corner.x;
								r.corner.y = pMS->flash_rect0.corner.y
										+ pMS->flash_rect0.extent.height - 6;
								r.extent.width = SHIP_WIN_WIDTH;
								r.extent.height = 5;
								SetContext (SpaceContext);
								SetFlashRect (&r);
							}
							else
							{	// at capacity or not enough RUs
								PlayMenuSound (MENU_SOUND_FAILURE);
							}
							UnlockFleetInfo (&GLOBAL (avail_race_q),
									hTemplate);
						}
					}
					else if (dy > 0)
					{
						crew_bought = (SIZE)MAKE_WORD (
								GET_GAME_STATE (CREW_PURCHASED0),
								GET_GAME_STATE (CREW_PURCHASED1));
						if (hStarShip == 0)
						{
							if (GetCrewCount ())
							{
								DeltaSISGauges (-1, 0, GLOBAL (CrewCost)
										- (crew_bought ==
										CREW_EXPENSE_THRESHOLD ? 2 : 0));
								crew_delta = -1;

								GetCPodCapacity (&r.corner);
								SetContextForeGroundColor (BLACK_COLOR);
								DrawPoint (&r.corner);

								SetContext (StatusContext);
								GetGaugeRect (&r, TRUE);
								SetFlashRect (&r);
								SetContext (SpaceContext);
							}
							else
							{	// no crew to dismiss
								PlayMenuSound (MENU_SOUND_FAILURE);
							}
						}
						else
						{
							if (StarShipPtr->crew_level > 0)
							{
								if (StarShipPtr->crew_level > 1)
									DeltaSISGauges (0, 0, GLOBAL (CrewCost)
											- (crew_bought ==
											CREW_EXPENSE_THRESHOLD ? 2 : 0));
								else
									DeltaSISGauges (0, 0, (COUNT)ShipCost[
											StarShipPtr->race_id]);
								crew_delta = -1;
								--StarShipPtr->crew_level;
							}
							else
							{	// no crew to dismiss
								PlayMenuSound (MENU_SOUND_FAILURE);
							}
							ShowShipCrew (StarShipPtr, &pMS->flash_rect0);
							r.corner.x = pMS->flash_rect0.corner.x;
							r.corner.y = pMS->flash_rect0.corner.y
									+ pMS->flash_rect0.extent.height - 6;
							r.extent.width = SHIP_WIN_WIDTH;
							r.extent.height = 5;
							SetContext (SpaceContext);
							SetFlashRect (&r);
						}
					}

					if (hStarShip)
					{
						UnlockShipFrag (&GLOBAL (built_ship_q), hStarShip);
						
						// clear out the bought ship index
						// so that flash rects work correctly
						pMS->delta_item &= MODIFY_CREW_FLAG;
					}
					CrewTransaction (crew_delta);
				}
			}
			else if (cancel)
			{
				UnlockMutex (GraphicsLock);

				pMS->InputFunc = DoShipyard;
				pMS->CurState = SHIPYARD_CREW;
				DrawMenuStateStrings (PM_CREW, pMS->CurState);
				LockMutex (GraphicsLock);
				SetFlashRect (SFR_MENU_3DO);
				UnlockMutex (GraphicsLock);

				return TRUE;
			}
			else
			{
				pMS->CurState = NewState;

ChangeFlashRect:
				if (HINIBBLE (pMS->CurState))
				{
					pMS->flash_rect0.corner.x =
							pMS->flash_rect0.corner.y = 0;
					pMS->flash_rect0.extent.width = SIS_SCREEN_WIDTH;
					pMS->flash_rect0.extent.height = 61;
				}
				else
				{
					pMS->flash_rect0.corner.x = hangar_x_coords[
							pMS->CurState % HANGAR_SHIPS_ROW];
					pMS->flash_rect0.corner.y = HANGAR_Y + (HANGAR_DY *
							(pMS->CurState / HANGAR_SHIPS_ROW));
					pMS->flash_rect0.extent.width = SHIP_WIN_WIDTH;
					pMS->flash_rect0.extent.height = SHIP_WIN_HEIGHT;
				}
				SetFlashRect (&pMS->flash_rect0);
			}
			UnlockMutex (GraphicsLock);
		}
	}

	SleepThread (ONE_SECOND / 30);

	return TRUE;
}

static void
DrawBluePrint (MENU_STATE *pMS)
{
	COUNT num_frames;
	STAMP s;
	FRAME ModuleFrame;

	ModuleFrame = CaptureDrawable (LoadGraphic (SISBLU_MASK_ANIM));

	s.origin.x = 0;
	s.origin.y = 0;
	s.frame = DecFrameIndex (ModuleFrame);
	SetContextForeGroundColor (
			BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x16), 0x01));
	DrawFilledStamp (&s);

	for (num_frames = 0; num_frames < NUM_DRIVE_SLOTS; ++num_frames)
	{
		DrawShipPiece (ModuleFrame, GLOBAL_SIS (DriveSlots[num_frames]),
				num_frames, TRUE);
	}
	for (num_frames = 0; num_frames < NUM_JET_SLOTS; ++num_frames)
	{
		DrawShipPiece (ModuleFrame, GLOBAL_SIS (JetSlots[num_frames]),
				num_frames, TRUE);
	}
	for (num_frames = 0; num_frames < NUM_MODULE_SLOTS; ++num_frames)
	{
		BYTE which_piece;

		which_piece = GLOBAL_SIS (ModuleSlots[num_frames]);

		if (!(pMS->CurState == SHIPYARD && which_piece == CREW_POD))
			DrawShipPiece (ModuleFrame, which_piece, num_frames, TRUE);
	}

	SetContextForeGroundColor (
			BUILD_COLOR (MAKE_RGB15 (0x0A, 0x0A, 0x1F), 0x09));
	for (num_frames = 0; num_frames < NUM_MODULE_SLOTS; ++num_frames)
	{
		BYTE which_piece;

		which_piece = GLOBAL_SIS (ModuleSlots[num_frames]);
		if (pMS->CurState == SHIPYARD && which_piece == CREW_POD)
			DrawShipPiece (ModuleFrame, which_piece, num_frames, TRUE);
	}

	{
		num_frames = GLOBAL_SIS (CrewEnlisted);
		GLOBAL_SIS (CrewEnlisted) = 0;

		while (num_frames--)
		{
			POINT pt;

			GetCPodCapacity (&pt);
			DrawPoint (&pt);

			++GLOBAL_SIS (CrewEnlisted);
		}
	}
	{
		RECT r;

		num_frames = GLOBAL_SIS (TotalElementMass);
		GLOBAL_SIS (TotalElementMass) = 0;

		r.extent.width = 9;
		r.extent.height = 1;
		while (num_frames)
		{
			COUNT m;

			m = num_frames < SBAY_MASS_PER_ROW ?
					num_frames : SBAY_MASS_PER_ROW;
			GLOBAL_SIS (TotalElementMass) += m;
			GetSBayCapacity (&r.corner);
			DrawFilledRectangle (&r);
			num_frames -= m;
		}
	}
	if (GLOBAL_SIS (FuelOnBoard) > FUEL_RESERVE)
	{
		DWORD FuelVolume;
		RECT r;

		FuelVolume = GLOBAL_SIS (FuelOnBoard) - FUEL_RESERVE;
		GLOBAL_SIS (FuelOnBoard) = FUEL_RESERVE;

		r.extent.width = 3;
		r.extent.height = 1;
		while (FuelVolume)
		{
			COUNT m;

			GetFTankCapacity (&r.corner);
			DrawPoint (&r.corner);
			r.corner.x += r.extent.width + 1;
			DrawPoint (&r.corner);
			r.corner.x -= r.extent.width;
			SetContextForeGroundColor (
					SetContextBackGroundColor (BLACK_COLOR));
			DrawFilledRectangle (&r);
			m = FuelVolume < FUEL_VOLUME_PER_ROW ?
					(COUNT)FuelVolume : FUEL_VOLUME_PER_ROW;
			GLOBAL_SIS (FuelOnBoard) += m;
			FuelVolume -= m;
		}
	}

	DestroyDrawable (ReleaseDrawable (ModuleFrame));
}

BOOLEAN
DoShipyard (MENU_STATE *pMS)
{
	BOOLEAN select, cancel;
	if (GLOBAL (CurrentActivity) & CHECK_ABORT)
		goto ExitShipyard;

	select = PulsedInputState.menu[KEY_MENU_SELECT];
	cancel = PulsedInputState.menu[KEY_MENU_CANCEL];

	SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);
	if (!pMS->Initialized)
	{
		pMS->InputFunc = DoShipyard;

		{
			STAMP s;
			RECT r, old_r;

			pMS->ModuleFrame = CaptureDrawable (
					LoadGraphic (SHIPYARD_PMAP_ANIM));

			pMS->CurString = CaptureColorMap (
					LoadColorMap (HANGAR_COLOR_TAB));

			pMS->hMusic = LoadMusic (SHIPYARD_MUSIC);

			LockMutex (GraphicsLock);
			SetTransitionSource (NULL);
			BatchGraphics ();
			DrawSISFrame ();
			DrawSISMessage (GAME_STRING (STARBASE_STRING_BASE + 3));
			DrawSISTitle (GAME_STRING (STARBASE_STRING_BASE));
			SetContext (SpaceContext);
			DrawBluePrint (pMS);
			UnlockMutex (GraphicsLock);

			pMS->CurState = SHIPYARD_CREW;
			DrawMenuStateStrings (PM_CREW, pMS->CurState);

			LockMutex (GraphicsLock);
			SetContext (SpaceContext);
			s.origin.x = 0;
			s.origin.y = 0;
			s.frame = SetAbsFrameIndex (pMS->ModuleFrame, 0);
#ifdef USE_3DO_HANGAR
			DrawStamp (&s);
#else // PC hangar
			// the PC ship dock needs to overwrite the border
			// expand the clipping rect by 1 pixel
			GetContextClipRect (&old_r);
			r = old_r;
			r.corner.x--;
			r.extent.width += 2;
			r.extent.height += 1;
			SetContextClipRect (&r);
			DrawStamp (&s);
			SetContextClipRect (&old_r);
			animatePowerLines (pMS);
#endif // USE_3DO_HANGAR
			
			SetContextFont (TinyFont);

			ScreenTransition (3, NULL);
			UnbatchGraphics ();
			UnlockMutex (GraphicsLock);

			PlayMusic (pMS->hMusic, TRUE, 1);

			ShowCombatShip (pMS, (COUNT)~0, NULL);

			SetInputCallback (on_input_frame);

			LockMutex (GraphicsLock);
			SetFlashRect (SFR_MENU_3DO);
			UnlockMutex (GraphicsLock);
		}

		pMS->Initialized = TRUE;
	}
	else if (cancel || (select && pMS->CurState == SHIPYARD_EXIT))
	{
ExitShipyard:
		SetInputCallback (NULL);

		LockMutex (GraphicsLock);
		DestroyDrawable (ReleaseDrawable (pMS->ModuleFrame));
		pMS->ModuleFrame = 0;
		DestroyColorMap (ReleaseColorMap (pMS->CurString));
		pMS->CurString = 0;
		UnlockMutex (GraphicsLock);

		return FALSE;
	}
	else if (select)
	{
		if (pMS->CurState != SHIPYARD_SAVELOAD)
		{
			pMS->Initialized = FALSE;
			DoModifyShips (pMS);
		}
		else
		{
			// Clearing FlashRect is not necessary
			if (!GameOptions ())
				goto ExitShipyard;
			DrawMenuStateStrings (PM_CREW, pMS->CurState);
			LockMutex (GraphicsLock);
			SetFlashRect (SFR_MENU_3DO);
			UnlockMutex (GraphicsLock);
		}
	}
	else
	{
		DoMenuChooser (pMS, PM_CREW);
	}

	return TRUE;
}


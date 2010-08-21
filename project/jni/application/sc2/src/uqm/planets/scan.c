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

#include "lander.h"
#include "lifeform.h"
#include "scan.h"
#include "../build.h"
#include "../colors.h"
#include "../cons_res.h"
#include "../controls.h"
#include "../menustat.h"
#include "../encount.h"
		// for EncounterGroup
#include "../gamestr.h"
#include "../nameref.h"
#include "../resinst.h"
#include "../settings.h"
#include "../load.h"
#include "../util.h"
#include "../process.h"
#include "../setup.h"
#include "../sounds.h"
#include "../state.h"
#include "../sis.h"
#include "options.h"
#include "libs/graphics/gfx_common.h"
#include "libs/graphics/drawable.h"
#include "libs/inplib.h"
#include "libs/mathlib.h"

extern FRAME SpaceJunkFrame;

// define SPIN_ON_SCAN to allow the planet to spin 
// while scaning  is going on
#undef SPIN_ON_SCAN

#define FLASH_INDEX 105

static CONTEXT ScanContext;

static POINT planetLoc;
static RECT cursorRect;
static FRAME eraseFrame;

// ScanSystem() menu items
// The first three are from enum PlanetScanTypes in planets.h
enum ScanMenuItems
{
	EXIT_SCAN = NUM_SCAN_TYPES,
	AUTO_SCAN,
	DISPATCH_SHUTTLE,
};


void
RepairBackRect (RECT *pRect)
{
	RECT new_r, old_r;

	GetContextClipRect (&old_r);
	new_r.corner.x = pRect->corner.x + old_r.corner.x;
	new_r.corner.y = pRect->corner.y + old_r.corner.y;
	new_r.extent = pRect->extent;

	new_r.extent.height += new_r.corner.y & 1;
	new_r.corner.y &= ~1;
	DrawFromExtraScreen (&new_r);
}

static void
EraseCoarseScan (void)
{
	LockMutex (GraphicsLock);
	SetContext (PlanetContext);
	
	BatchGraphics ();
	DrawStarBackGround ();
	DrawDefaultPlanetSphere ();
	UnbatchGraphics ();
	
	UnlockMutex (GraphicsLock);
}

static void
PrintScanTitlePC (TEXT *t, RECT *r, const char *txt, int xpos)
{
	t->baseline.x = xpos;
	SetContextForeGroundColor (SCAN_PC_TITLE_COLOR);
	t->pStr = txt;
	t->CharCount = (COUNT)~0;
	font_DrawText (t);
	TextRect (t, r, NULL);
	t->baseline.x += r->extent.width;
	SetContextForeGroundColor (SCAN_INFO_COLOR);
}

static void
MakeScanValue (UNICODE *buf, long val, const UNICODE *extra)
{
	if (val >= 10 * 100)
	{	// 1 decimal place
		sprintf (buf, "%ld.%ld%s", val / 100, (val / 10) % 10, extra);
	}
	else
	{	// 2 decimal places
		sprintf (buf, "%ld.%02ld%s", val / 100, val % 100, extra);
	}
}

static void
GetPlanetTitle (UNICODE *buf, COUNT bufsize)
{
	int val;
	UNICODE *named = GetNamedPlanetaryBody ();
	if (named)
	{
		utf8StringCopy (buf, bufsize, named);
		return;
	}

	// Unnamed body, use world type
	val = pSolarSysState->pOrbitalDesc->data_index & ~PLANET_SHIELDED;
	if (val >= FIRST_GAS_GIANT)
	{
		sprintf (buf, "%s", GAME_STRING (SCAN_STRING_BASE + 4 + 51));
					// Gas Giant
	}
	else
	{
		sprintf (buf, "%s %s",
				GAME_STRING (SCAN_STRING_BASE + 4 + val),
				GAME_STRING (SCAN_STRING_BASE + 4 + 50));
						// World
	}
}

static void
PrintCoarseScanPC (void)
{
#define SCAN_LEADING_PC 14
	SDWORD val;
	TEXT t;
	RECT r;
	UNICODE buf[200];

	GetPlanetTitle (buf, sizeof (buf));

	LockMutex (GraphicsLock);
	SetContext (PlanetContext);

	t.align = ALIGN_CENTER;
	t.baseline.x = SIS_SCREEN_WIDTH >> 1;
	t.baseline.y = 13;
	t.pStr = buf;
	t.CharCount = (COUNT)~0;

	SetContextForeGroundColor (SCAN_PC_TITLE_COLOR);
	SetContextFont (MicroFont);
	font_DrawText (&t);

	SetContextFont (TinyFont);
	UnlockMutex (GraphicsLock);

#define LEFT_SIDE_BASELINE_X_PC 5
#define RIGHT_SIDE_BASELINE_X_PC (SIS_SCREEN_WIDTH - 75)
#define SCAN_BASELINE_Y_PC 40

	t.baseline.y = SCAN_BASELINE_Y_PC;
	t.align = ALIGN_LEFT;

	LockMutex (GraphicsLock);
	PrintScanTitlePC (&t, &r, GAME_STRING (ORBITSCAN_STRING_BASE),
			LEFT_SIDE_BASELINE_X_PC); // "Orbit: "
	val = ((pSolarSysState->SysInfo.PlanetInfo.PlanetToSunDist * 100L
			+ (EARTH_RADIUS >> 1)) / EARTH_RADIUS);
	MakeScanValue (buf, val,
			GAME_STRING (ORBITSCAN_STRING_BASE + 1)); // " a.u."
	t.pStr = buf;
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING_PC;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	PrintScanTitlePC (&t, &r, GAME_STRING (ORBITSCAN_STRING_BASE + 2),
			LEFT_SIDE_BASELINE_X_PC); // "Atmo: "
	if (pSolarSysState->SysInfo.PlanetInfo.AtmoDensity == GAS_GIANT_ATMOSPHERE)
		utf8StringCopy (buf, sizeof (buf),
				GAME_STRING (ORBITSCAN_STRING_BASE + 3)); // "Super Thick"
	else if (pSolarSysState->SysInfo.PlanetInfo.AtmoDensity == 0)
		utf8StringCopy (buf, sizeof (buf),
				GAME_STRING (ORBITSCAN_STRING_BASE + 4)); // "Vacuum"
	else
	{
		val = (pSolarSysState->SysInfo.PlanetInfo.AtmoDensity * 100
			+ (EARTH_ATMOSPHERE >> 1)) / EARTH_ATMOSPHERE;
		MakeScanValue (buf, val,
				GAME_STRING (ORBITSCAN_STRING_BASE + 5)); // " atm"
	}
	t.pStr = buf;
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING_PC;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	PrintScanTitlePC (&t, &r, GAME_STRING (ORBITSCAN_STRING_BASE + 6),
			LEFT_SIDE_BASELINE_X_PC); // "Temp: "
	sprintf (buf, "%d" STR_DEGREE_SIGN " c",
			pSolarSysState->SysInfo.PlanetInfo.SurfaceTemperature);
	t.pStr = buf;
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING_PC;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	PrintScanTitlePC (&t, &r, GAME_STRING (ORBITSCAN_STRING_BASE + 7),
			LEFT_SIDE_BASELINE_X_PC); // "Weather: "
	if (pSolarSysState->SysInfo.PlanetInfo.AtmoDensity == 0)
		t.pStr = GAME_STRING (ORBITSCAN_STRING_BASE + 8); // "None"
	else
	{
		sprintf (buf, "%s %u",
				GAME_STRING (ORBITSCAN_STRING_BASE + 9), // "Class"
				pSolarSysState->SysInfo.PlanetInfo.Weather + 1);
		t.pStr = buf;
	}
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING_PC;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	PrintScanTitlePC (&t, &r, GAME_STRING (ORBITSCAN_STRING_BASE + 10),
			LEFT_SIDE_BASELINE_X_PC); // "Tectonics: "
	if (PLANSIZE (pSolarSysState->SysInfo.PlanetInfo.PlanDataPtr->Type) ==
			GAS_GIANT)
		t.pStr = GAME_STRING (ORBITSCAN_STRING_BASE + 8); // "None"
	else
	{
		sprintf (buf, "%s %u",
				GAME_STRING (ORBITSCAN_STRING_BASE + 9), // "Class"
				pSolarSysState->SysInfo.PlanetInfo.Tectonics + 1);
		t.pStr = buf;
	}
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	UnlockMutex (GraphicsLock);

	t.baseline.y = SCAN_BASELINE_Y_PC;

	LockMutex (GraphicsLock);
	PrintScanTitlePC (&t, &r, GAME_STRING (ORBITSCAN_STRING_BASE + 11),
			RIGHT_SIDE_BASELINE_X_PC); // "Mass: "
	val = pSolarSysState->SysInfo.PlanetInfo.PlanetRadius;
	val = ((DWORD) val * (DWORD) val * (DWORD) val / 100L
			* pSolarSysState->SysInfo.PlanetInfo.PlanetDensity
			+ ((100L * 100L) >> 1)) / (100L * 100L);
	if (val == 0)
		val = 1;
	MakeScanValue (buf, val,
			GAME_STRING (ORBITSCAN_STRING_BASE + 12)); // " e.s."
	t.pStr = buf;
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING_PC;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	PrintScanTitlePC (&t, &r, GAME_STRING (ORBITSCAN_STRING_BASE + 13),
			RIGHT_SIDE_BASELINE_X_PC); // "Radius: "
	val = pSolarSysState->SysInfo.PlanetInfo.PlanetRadius;
	MakeScanValue (buf, val,
			GAME_STRING (ORBITSCAN_STRING_BASE + 12)); // " e.s."
	t.pStr = buf;
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING_PC;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	PrintScanTitlePC (&t, &r, GAME_STRING (ORBITSCAN_STRING_BASE + 14),
			RIGHT_SIDE_BASELINE_X_PC); // "Gravity: "
	val = pSolarSysState->SysInfo.PlanetInfo.SurfaceGravity;
	if (val == 0)
		val = 1;
	MakeScanValue (buf, val,
			GAME_STRING (ORBITSCAN_STRING_BASE + 15)); // " g."
	t.pStr = buf;
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING_PC;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	PrintScanTitlePC (&t, &r, GAME_STRING (ORBITSCAN_STRING_BASE + 16),
			RIGHT_SIDE_BASELINE_X_PC); // "Day: "
	val = (SDWORD)pSolarSysState->SysInfo.PlanetInfo.RotationPeriod
			* 10 / 24;
	MakeScanValue (buf, val,
			GAME_STRING (ORBITSCAN_STRING_BASE + 17)); // " days"
	t.pStr = buf;
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING_PC;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	PrintScanTitlePC (&t, &r, GAME_STRING (ORBITSCAN_STRING_BASE + 18),
			RIGHT_SIDE_BASELINE_X_PC); // "Tilt: "
	val = pSolarSysState->SysInfo.PlanetInfo.AxialTilt;
	if (val < 0)
		val = -val;
	t.pStr = buf;
	sprintf (buf, "%d" STR_DEGREE_SIGN, val);
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	UnlockMutex (GraphicsLock);
}

static void
PrintCoarseScan3DO (void)
{
#define SCAN_LEADING 19
	SDWORD val;
	TEXT t;
	STAMP s;
	UNICODE buf[200];

	GetPlanetTitle (buf, sizeof (buf));

	LockMutex (GraphicsLock);
	SetContext (PlanetContext);

	t.align = ALIGN_CENTER;
	t.baseline.x = SIS_SCREEN_WIDTH >> 1;
	t.baseline.y = 13;
	t.pStr = buf;
	t.CharCount = (COUNT)~0;

	SetContextForeGroundColor (SCAN_INFO_COLOR);
	SetContextFont (MicroFont);
	font_DrawText (&t);

	s.origin.x = s.origin.y = 0;
	s.origin.x = 16 - SAFE_X;
	s.frame = SetAbsFrameIndex (SpaceJunkFrame, 20);
	DrawStamp (&s);

	UnlockMutex (GraphicsLock);

#define LEFT_SIDE_BASELINE_X (27 + (16 - SAFE_X))
#define RIGHT_SIDE_BASELINE_X (SIS_SCREEN_WIDTH - LEFT_SIDE_BASELINE_X)
#define SCAN_BASELINE_Y 25

	t.baseline.x = LEFT_SIDE_BASELINE_X;
	t.baseline.y = SCAN_BASELINE_Y;
	t.align = ALIGN_LEFT;

	LockMutex (GraphicsLock);
	t.pStr = buf;
	val = ((pSolarSysState->SysInfo.PlanetInfo.PlanetToSunDist * 100L
			+ (EARTH_RADIUS >> 1)) / EARTH_RADIUS);
	MakeScanValue (buf, val, STR_EARTH_SIGN);
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	t.pStr = buf;
	if (pSolarSysState->SysInfo.PlanetInfo.AtmoDensity == GAS_GIANT_ATMOSPHERE)
		strcpy (buf, STR_INFINITY_SIGN);
	else
	{
		val = (pSolarSysState->SysInfo.PlanetInfo.AtmoDensity * 100
				+ (EARTH_ATMOSPHERE >> 1)) / EARTH_ATMOSPHERE;
		MakeScanValue (buf, val, STR_EARTH_SIGN);
	}
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	t.pStr = buf;
	sprintf (buf, "%d" STR_DEGREE_SIGN,
			pSolarSysState->SysInfo.PlanetInfo.SurfaceTemperature);
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	t.pStr = buf;
	sprintf (buf, "<%u>", pSolarSysState->SysInfo.PlanetInfo.AtmoDensity == 0
			? 0 : (pSolarSysState->SysInfo.PlanetInfo.Weather + 1));
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	t.pStr = buf;
	sprintf (buf, "<%u>",
			PLANSIZE (
			pSolarSysState->SysInfo.PlanetInfo.PlanDataPtr->Type
			) == GAS_GIANT
			? 0 : (pSolarSysState->SysInfo.PlanetInfo.Tectonics + 1));
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	UnlockMutex (GraphicsLock);

	t.baseline.x = RIGHT_SIDE_BASELINE_X;
	t.baseline.y = SCAN_BASELINE_Y;
	t.align = ALIGN_RIGHT;

	LockMutex (GraphicsLock);
	t.pStr = buf;
	val = pSolarSysState->SysInfo.PlanetInfo.PlanetRadius;
	val = ((DWORD) val * (DWORD) val * (DWORD) val / 100L
			* pSolarSysState->SysInfo.PlanetInfo.PlanetDensity
			+ ((100L * 100L) >> 1)) / (100L * 100L);
	if (val == 0)
		val = 1;
	MakeScanValue (buf, val, STR_EARTH_SIGN);
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	t.pStr = buf;
	val = pSolarSysState->SysInfo.PlanetInfo.PlanetRadius;
	MakeScanValue (buf, val, STR_EARTH_SIGN);

	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	t.pStr = buf;
	val = pSolarSysState->SysInfo.PlanetInfo.SurfaceGravity;
	if (val == 0)
		val = 1;
	MakeScanValue (buf, val, STR_EARTH_SIGN);
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	t.pStr = buf;
	val = pSolarSysState->SysInfo.PlanetInfo.AxialTilt;
	if (val < 0)
		val = -val;
	sprintf (buf, "%d" STR_DEGREE_SIGN, val);
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	t.baseline.y += SCAN_LEADING;
	UnlockMutex (GraphicsLock);

	LockMutex (GraphicsLock);
	t.pStr = buf;
	val = (SDWORD)pSolarSysState->SysInfo.PlanetInfo.RotationPeriod
			* 10 / 24;
	MakeScanValue (buf, val, STR_EARTH_SIGN);
	t.CharCount = (COUNT)~0;
	font_DrawText (&t);
	UnlockMutex (GraphicsLock);
}

static void
initPlanetLocationImage (void)
{
	FRAME cursorFrame;

	// Get the cursor image
	cursorFrame = SetAbsFrameIndex (MiscDataFrame, FLASH_INDEX);
	cursorRect.extent = GetFrameBounds (cursorFrame);
}

static void
savePlanetLocationImage (void)
{
	RECT r;
	FRAME cursorFrame = SetAbsFrameIndex (MiscDataFrame, FLASH_INDEX);
	HOT_SPOT hs = GetFrameHot (cursorFrame);

	DestroyDrawable (ReleaseDrawable (eraseFrame));

	r = cursorRect;
	r.corner.x -= hs.x;
	r.corner.y -= hs.y;
	eraseFrame = CaptureDrawable (CopyContextRect (&r));
	SetFrameHot (eraseFrame, hs);
}

static void
restorePlanetLocationImage (void)
{
	STAMP s;

	s.origin = cursorRect.corner;
	s.frame = eraseFrame; // saved image
	DrawStamp (&s);
}

static void
drawPlanetCursor (BOOLEAN filled)
{
	STAMP s;

	s.origin = cursorRect.corner;
	s.frame = SetAbsFrameIndex (MiscDataFrame, FLASH_INDEX);
	if (filled)
		DrawFilledStamp (&s);
	else
		DrawStamp (&s);
}

static void
setPlanetCursorLoc (POINT new_pt)
{
	new_pt.x >>= MAG_SHIFT;
	new_pt.y >>= MAG_SHIFT;
	cursorRect.corner = new_pt;
}

static void
setPlanetLoc (POINT new_pt, BOOLEAN restoreOld)
{
	planetLoc = new_pt;

	SetContext (ScanContext);
	if (restoreOld)
		restorePlanetLocationImage ();
	setPlanetCursorLoc (new_pt);
	savePlanetLocationImage ();
}

static void
flashPlanetLocation (void)
{
#define FLASH_FRAME_DELAY  (ONE_SECOND / 16)
	static BYTE c = 0x00;
	static int val = -2;
	static POINT prevPt;
	static TimeCount NextTime = 0;
	BOOLEAN locChanged;
	TimeCount Now = GetTimeCounter ();

	locChanged = prevPt.x != cursorRect.corner.x
				|| prevPt.y != cursorRect.corner.y;

	if (!locChanged && Now < NextTime)
		return; // nothing to do

	if (locChanged)
	{	// Reset the flashing cycle
		c = 0x00;
		val = -2;
		prevPt = cursorRect.corner;
		
		NextTime = Now + FLASH_FRAME_DELAY;
	}
	else
	{	// Continue the flashing cycle
		if (c == 0x00 || c == 0x1A)
			val = -val;
		c += val;

		if (Now - NextTime > FLASH_FRAME_DELAY)
			NextTime = Now + FLASH_FRAME_DELAY; // missed timing by too much
		else
			NextTime += FLASH_FRAME_DELAY; // stable frame rate
	}

	SetContext (ScanContext);
	SetContextForeGroundColor (BUILD_COLOR (MAKE_RGB15 (c, c, c), c));
	drawPlanetCursor (TRUE);
}

void
RedrawSurfaceScan (const POINT *newLoc)
{
	CONTEXT OldContext;

	OldContext = SetContext (ScanContext);

	BatchGraphics ();
	DrawPlanet (0, BLACK_COLOR);
	DrawScannedObjects (TRUE);
	if (newLoc)
	{
		setPlanetLoc (*newLoc, FALSE);
	 	drawPlanetCursor (FALSE);
	}
	UnbatchGraphics ();

	SetContext (OldContext);
}

static COUNT
getLandingFuelNeeded (void)
{
	COUNT fuel;

	fuel = pSolarSysState->SysInfo.PlanetInfo.SurfaceGravity << 1;
	if (fuel > 3 * FUEL_TANK_SCALE)
		fuel = 3 * FUEL_TANK_SCALE;

	return fuel;
}

static void
spawnFwiffo (void)
{
	HSHIPFRAG hStarShip;

	EncounterGroup = 0;
	PutGroupInfo (GROUPS_RANDOM, GROUP_SAVE_IP);
	ReinitQueue (&GLOBAL (ip_group_q));
	assert (CountLinks (&GLOBAL (npc_built_ship_q)) == 0);

	hStarShip = CloneShipFragment (SPATHI_SHIP,
			&GLOBAL (npc_built_ship_q), 1);
	if (hStarShip)
	{
		SHIP_FRAGMENT *StarShipPtr;

		StarShipPtr = LockShipFrag (&GLOBAL (npc_built_ship_q),
				hStarShip);
		// Name Fwiffo
		StarShipPtr->captains_name_index = NAME_OFFSET +
				NUM_CAPTAINS_NAMES;
		UnlockShipFrag (&GLOBAL (npc_built_ship_q), hStarShip);
	}
}

// Returns TRUE if the parent menu should remain
static BOOLEAN
DispatchLander (void)
{
	InputFrameCallback *oldCallback;
	SIZE landingFuel = getLandingFuelNeeded ();

	EraseCoarseScan ();

	// Deactivate planet rotation callback
	oldCallback = SetInputCallback (NULL);

	LockMutex (GraphicsLock);
	DeltaSISGauges (0, -landingFuel, 0);
	SetContext (ScanContext);
	drawPlanetCursor (FALSE);
	UnlockMutex (GraphicsLock);

	PlanetSide (planetLoc);
	if (GLOBAL (CurrentActivity) & CHECK_ABORT)
		return FALSE;

	if (GET_GAME_STATE (FOUND_PLUTO_SPATHI) == 1)
	{
		/* Create Fwiffo group and go into comm with it */
		spawnFwiffo ();

		NextActivity |= CHECK_LOAD; /* fake a load game */
		GLOBAL (CurrentActivity) |= START_ENCOUNTER;
		SaveSolarSysLocation ();

		return FALSE;
	}

	if (optWhichCoarseScan == OPT_PC)
		PrintCoarseScanPC ();
	else
		PrintCoarseScan3DO ();

	// Reactivate planet rotation callback
	SetInputCallback (oldCallback);

	return TRUE;
}

typedef struct
{
	bool success;
			// true when player selected a location
} PICK_PLANET_STATE;

static BOOLEAN
DoPickPlanetSide (MENU_STATE *pMS)
{
	PICK_PLANET_STATE *pickState = pMS->privData;
	DWORD TimeIn = GetTimeCounter ();
	BOOLEAN select, cancel;

	select = PulsedInputState.menu[KEY_MENU_SELECT];
	cancel = PulsedInputState.menu[KEY_MENU_CANCEL];
	
	if (GLOBAL (CurrentActivity) & CHECK_ABORT)
	{
		pickState->success = false;
		return FALSE;
	}

	if (cancel)
	{
		pickState->success = false;
		return FALSE;
	}
	else if (select)
	{
		pickState->success = true;
		return FALSE;
	}
	else
	{
		SIZE dx = 0;
		SIZE dy = 0;
		POINT new_pt;

		new_pt = planetLoc;

		if (CurrentInputState.menu[KEY_MENU_LEFT])
			dx = -1;
		if (CurrentInputState.menu[KEY_MENU_RIGHT])
			dx = 1;
		if (CurrentInputState.menu[KEY_MENU_UP])
			dy = -1;
		if (CurrentInputState.menu[KEY_MENU_DOWN])
			dy = 1;

		LockMutex (GraphicsLock);
		BatchGraphics ();

		dx = dx << MAG_SHIFT;
		if (dx)
		{
			new_pt.x += dx;
			if (new_pt.x < 0)
				new_pt.x += (MAP_WIDTH << MAG_SHIFT);
			else if (new_pt.x >= (MAP_WIDTH << MAG_SHIFT))
				new_pt.x -= (MAP_WIDTH << MAG_SHIFT);
		}
		dy = dy << MAG_SHIFT;
		if (dy)
		{
			new_pt.y += dy;
			if (new_pt.y < 0 || new_pt.y >= (MAP_HEIGHT << MAG_SHIFT))
				new_pt.y = planetLoc.y;
		}

		if (!pointsEqual (new_pt, planetLoc))
		{
			setPlanetLoc (new_pt, TRUE);
		}

		flashPlanetLocation ();

		UnbatchGraphics ();
		UnlockMutex (GraphicsLock);

		SleepThreadUntil (TimeIn + ONE_SECOND / 40);
	}

	return TRUE;
}

static void
drawLandingFuelUsage (COUNT fuel)
{
	UNICODE buf[100];

	sprintf (buf, "%s%1.1f",
			GAME_STRING (NAVIGATION_STRING_BASE + 5),
			(float) fuel / FUEL_TANK_SCALE);
	DrawStatusMessage (buf);
}

static void
eraseLandingFuelUsage (void)
{
	LockMutex (GraphicsLock);
	DrawStatusMessage (NULL);
	UnlockMutex (GraphicsLock);
}

static BOOLEAN
PickPlanetSide (void)
{
	MENU_STATE MenuState;
	PICK_PLANET_STATE PickState;
	COUNT fuel = getLandingFuelNeeded ();
	BOOLEAN retval = TRUE;

	memset (&MenuState, 0, sizeof MenuState);
	MenuState.privData = &PickState;

	LockMutex (GraphicsLock);
	ClearSISRect (CLEAR_SIS_RADAR);
	SetContext (ScanContext);
	BatchGraphics ();
	DrawPlanet (0, BLACK_COLOR);
	DrawScannedObjects (FALSE);
	UnbatchGraphics ();

	drawLandingFuelUsage (fuel);
	// Set the current flash location
	setPlanetCursorLoc (planetLoc);
	savePlanetLocationImage ();
	UnlockMutex (GraphicsLock);

	InitLander (0);

	SetMenuSounds (MENU_SOUND_NONE, MENU_SOUND_SELECT);

	PickState.success = false;
	MenuState.InputFunc = DoPickPlanetSide;
	DoInput (&MenuState, TRUE);

	eraseLandingFuelUsage ();
	if (PickState.success)
	{	// player chose a location
		retval = DispatchLander ();
	}
	else
	{	// player bailed out
		LockMutex (GraphicsLock);
		restorePlanetLocationImage ();
		UnlockMutex (GraphicsLock);
	}

	SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);

	return retval;
}

#define NUM_FLASH_COLORS 8

static void
DrawScannedStuff (COUNT y, COUNT scan)
{
	HELEMENT hElement, hNextElement;
	Color OldColor;

	OldColor = SetContextForeGroundColor (BLACK_COLOR);

	for (hElement = GetHeadElement (); hElement; hElement = hNextElement)
	{
		ELEMENT *ElementPtr;
		SIZE dy;
		STAMP s;
		
		LockElement (hElement, &ElementPtr);
		hNextElement = GetSuccElement (ElementPtr);

		dy = y - ElementPtr->current.location.y;
		if (LOBYTE (ElementPtr->scan_node) != scan || dy < 0)
		{	// node of wrong type, or not time for it yet
			UnlockElement (hElement);
			continue;
		}

		// XXX: flag this as 'found' scanned object
		ElementPtr->state_flags |= APPEARING;

		s.origin = ElementPtr->current.location;
		
		if (dy >= NUM_FLASH_COLORS)
		{	// flashing done for this node, draw normal
			s.frame = ElementPtr->next.image.frame;
			DrawStamp (&s);
		}
		else
		{
			BYTE grad;
			Color c = WHITE_COLOR;
			COUNT nodeSize;
			
			// mineral -- white --> turquoise?? (contrasts with red)
			// energy -- white --> red (contrasts with white)
			// bio -- white --> violet (contrasts with green)
			grad = 0xff - 0xff * dy / (NUM_FLASH_COLORS - 1);
			switch (scan)
			{
				case MINERAL_SCAN:
					c.r = grad;
					break;
				case ENERGY_SCAN:
					c.g = grad;
					c.b = grad;
					break;
				case BIOLOGICAL_SCAN:
					c.g = grad;
					break;
			}
			
			SetContextForeGroundColor (c);
			
			// flash the node from the smallest size to node size
			// Get the node size for mineral, or number of transitions
			// for other scan types (was set by GeneratePlanetSide())
			nodeSize = GetFrameIndex (ElementPtr->next.image.frame)
					- GetFrameIndex (ElementPtr->current.image.frame);
			if (dy > nodeSize)
				dy = nodeSize;
			
			s.frame = SetRelFrameIndex (ElementPtr->current.image.frame, dy);
			DrawFilledStamp (&s);
		}

		UnlockElement (hElement);
	}
	
	SetContextForeGroundColor (OldColor);
}

bool
callGenerateForScanType (SOLARSYS_STATE *solarSys, PLANET_DESC *world,
		COUNT *node, BYTE scanType)
{
	switch (scanType)
	{
		case MINERAL_SCAN:
			return (*solarSys->genFuncs->generateMinerals) (
					solarSys, world, node);
		case ENERGY_SCAN:
			return (*solarSys->genFuncs->generateEnergy) (
					solarSys, world, node);
		case BIOLOGICAL_SCAN:
			return (*solarSys->genFuncs->generateLife) (
					solarSys, world, node);
	}

	assert (false);
	return false;
}

static void
ScanPlanet (COUNT scanType)
{
#define SCAN_DURATION   (ONE_SECOND * 7 / 4)
// NUM_FLASH_COLORS for flashing blips; 1 for the final frame
#define SCAN_LINES      (MAP_HEIGHT + NUM_FLASH_COLORS + 1)
#define SCAN_LINE_WAIT  (SCAN_DURATION / SCAN_LINES)

	COUNT startScan, endScan;
	COUNT scan;
	RECT r;
	static const Color textColors[] =
	{
		SCAN_MINERAL_TEXT_COLOR,
		SCAN_ENERGY_TEXT_COLOR,
		SCAN_BIOLOGICAL_TEXT_COLOR,
	};
	static const Color tintColors[] =
	{
		SCAN_MINERAL_TINT_COLOR,
		SCAN_ENERGY_TINT_COLOR,
		SCAN_BIOLOGICAL_TINT_COLOR,
	};

	if (scanType == AUTO_SCAN)
	{
		startScan = MINERAL_SCAN;
		endScan = BIOLOGICAL_SCAN;
	}
	else
	{
		startScan = scanType;
		endScan = scanType;
	}

	for (scan = startScan; scan <= endScan; ++scan)
	{
		TEXT t;
		SWORD i;
		Color tintColor;
				// Alpha value will be ignored.
		TimeCount TimeOut;

		t.baseline.x = SIS_SCREEN_WIDTH >> 1;
		t.baseline.y = SIS_SCREEN_HEIGHT - MAP_HEIGHT - 7;
		t.align = ALIGN_CENTER;
		t.CharCount = (COUNT)~0;

		pSolarSysState->CurNode = (COUNT)~0;
		callGenerateForScanType (pSolarSysState,
				pSolarSysState->pOrbitalDesc, &pSolarSysState->CurNode,
				scan);
		t.pStr = GAME_STRING (SCAN_STRING_BASE + scan);

		LockMutex (GraphicsLock);
		SetContext (PlanetContext);
		r.corner.x = 0;
		r.corner.y = t.baseline.y - 10;
		r.extent.width = SIS_SCREEN_WIDTH;
		r.extent.height = t.baseline.y - r.corner.y + 1;
		// XXX: I do not know why we are repairing it here, as there
		//   should not be anything drawn over the stars at the moment
		RepairBackRect (&r);

		SetContextFont (MicroFont);
		SetContextForeGroundColor (textColors[scan]);
		font_DrawText (&t);

		SetContext (ScanContext);
		UnlockMutex (GraphicsLock);

		// Draw a virgin surface
		LockMutex (GraphicsLock);
		BatchGraphics ();
		DrawPlanet (0, BLACK_COLOR);
		UnbatchGraphics ();
		UnlockMutex (GraphicsLock);

		tintColor = tintColors[scan];

		// Draw the scan slowly line by line
		TimeOut = GetTimeCounter ();
		for (i = 0; i < SCAN_LINES; i++)
		{
			TimeOut += SCAN_LINE_WAIT;
			if (WaitForAnyButtonUntil (TRUE, TimeOut, FALSE))
				break;

			LockMutex (GraphicsLock);
			BatchGraphics ();
			DrawPlanet (i, tintColor);
			DrawScannedStuff (i, scan);
			UnbatchGraphics ();
#ifdef SPIN_ON_SCAN
			RotatePlanetSphere (TRUE);
#endif
			UnlockMutex (GraphicsLock);
		}

		if (i < SCAN_LINES)
		{	// Aborted by a keypress; draw in finished state
			LockMutex (GraphicsLock);
			BatchGraphics ();
			DrawPlanet (SCAN_LINES - 1, tintColor);
			DrawScannedStuff (SCAN_LINES - 1, scan);
			UnbatchGraphics ();
			UnlockMutex (GraphicsLock);
		}
	}

	LockMutex (GraphicsLock);
	SetContext (PlanetContext);
	RepairBackRect (&r);

	SetContext (ScanContext);
	if (scanType == AUTO_SCAN)
	{	// clear the last scan
		DrawPlanet (0, BLACK_COLOR);
		DrawScannedObjects (FALSE);
	}

	UnlockMutex (GraphicsLock);
	FlushInput ();
}

static BOOLEAN
DoScan (MENU_STATE *pMS)
{
	BOOLEAN select, cancel;

	select = PulsedInputState.menu[KEY_MENU_SELECT];
	cancel = PulsedInputState.menu[KEY_MENU_CANCEL];
	
	if (GLOBAL (CurrentActivity) & CHECK_ABORT)
		return FALSE;

	if (cancel || (select && pMS->CurState == EXIT_SCAN))
	{
		return FALSE;
	}
	else if (select)
	{
		if (pMS->CurState == DISPATCH_SHUTTLE)
		{
			COUNT fuel_required;

			if ((pSolarSysState->pOrbitalDesc->data_index & PLANET_SHIELDED)
					|| (pSolarSysState->SysInfo.PlanetInfo.AtmoDensity ==
						GAS_GIANT_ATMOSPHERE))
			{	// cannot dispatch to shielded planets or gas giants
				PlayMenuSound (MENU_SOUND_FAILURE);
				return TRUE;
			}

			fuel_required = getLandingFuelNeeded ();
			if (GLOBAL_SIS (FuelOnBoard) < fuel_required
					|| GLOBAL_SIS (NumLanders) == 0
					|| GLOBAL_SIS (CrewEnlisted) == 0)
			{
				PlayMenuSound (MENU_SOUND_FAILURE);
				return TRUE;
			}

			LockMutex (GraphicsLock);
			SetFlashRect (NULL);
			UnlockMutex (GraphicsLock);

			if (!PickPlanetSide ())
				return FALSE;

			DrawMenuStateStrings (PM_MIN_SCAN, pMS->CurState);
			LockMutex (GraphicsLock);
			SetFlashRect (SFR_MENU_3DO);
			UnlockMutex (GraphicsLock);

			return TRUE;
		}

		// Various scans
		if (pSolarSysState->pOrbitalDesc->data_index & PLANET_SHIELDED)
		{	// cannot scan shielded planets
			PlayMenuSound (MENU_SOUND_FAILURE);
			return TRUE;
		}

		ScanPlanet (pMS->CurState);
		if (pMS->CurState == AUTO_SCAN)
		{
			pMS->CurState = DISPATCH_SHUTTLE;
			DrawMenuStateStrings (PM_MIN_SCAN, pMS->CurState);
		}
	}
	else if (optWhichMenu == OPT_PC ||
			(!(pSolarSysState->pOrbitalDesc->data_index & PLANET_SHIELDED)
			&& pSolarSysState->SysInfo.PlanetInfo.AtmoDensity !=
				GAS_GIANT_ATMOSPHERE))
	{
		DoMenuChooser (pMS, PM_MIN_SCAN);
	}

	return TRUE;
}

static CONTEXT
CreateScanContext (void)
{
	CONTEXT oldContext;
	CONTEXT context;
	RECT r;

	// ScanContext rect is relative to SpaceContext
	oldContext = SetContext (SpaceContext);
	GetContextClipRect (&r);

	context = CreateContext ("ScanContext");
	SetContext (context);
	SetContextFGFrame (Screen);
	r.corner.x += r.extent.width - MAP_WIDTH;
	r.corner.y += r.extent.height - MAP_HEIGHT;
	r.extent.width = MAP_WIDTH;
	r.extent.height = MAP_HEIGHT;
	SetContextClipRect (&r);

	SetContext (oldContext);

	return context;
}

CONTEXT
GetScanContext (BOOLEAN *owner)
{
	// TODO: Make CONTEXT ref-counted
	if (ScanContext)
	{
		if (owner)
			*owner = FALSE;
	}
	else
	{
		if (owner)
			*owner = TRUE;
		ScanContext = CreateScanContext ();
	}
	return ScanContext;
}

void
DestroyScanContext (void)
{
	if (ScanContext)
	{
		DestroyContext (ScanContext);
		ScanContext = NULL;
	}
}

void
ScanSystem (void)
{
	MENU_STATE MenuState;

	memset (&MenuState, 0, sizeof MenuState);

	LockMutex (GraphicsLock);
	GetScanContext (NULL);
	UnlockMutex (GraphicsLock);

	if (optWhichMenu == OPT_3DO &&
			((pSolarSysState->pOrbitalDesc->data_index & PLANET_SHIELDED)
			|| pSolarSysState->SysInfo.PlanetInfo.AtmoDensity ==
				GAS_GIANT_ATMOSPHERE))
	{
		MenuState.CurState = EXIT_SCAN;
	}
	else
	{
		MenuState.CurState = AUTO_SCAN;
		planetLoc.x = (MAP_WIDTH >> 1) << MAG_SHIFT;
		planetLoc.y = (MAP_HEIGHT >> 1) << MAG_SHIFT;

		LockMutex (GraphicsLock);
		initPlanetLocationImage ();
		SetContext (ScanContext);
		DrawScannedObjects (FALSE);
		UnlockMutex (GraphicsLock);
	}

	DrawMenuStateStrings (PM_MIN_SCAN, MenuState.CurState);
	LockMutex (GraphicsLock);
	SetFlashRect (SFR_MENU_3DO);
	UnlockMutex (GraphicsLock);

	if (optWhichCoarseScan == OPT_PC)
		PrintCoarseScanPC ();
	else
		PrintCoarseScan3DO ();

	SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);

	MenuState.InputFunc = DoScan;
	DoInput (&MenuState, FALSE);

	LockMutex (GraphicsLock);
	SetFlashRect (NULL);
	UnlockMutex (GraphicsLock);

	// cleanup scan graphics
	LockMutex (GraphicsLock);
	BatchGraphics ();
	SetContext (ScanContext);
	DrawPlanet (0, BLACK_COLOR);
	EraseCoarseScan ();
	UnbatchGraphics ();
	UnlockMutex (GraphicsLock);

	DestroyDrawable (ReleaseDrawable (eraseFrame));
	eraseFrame = NULL;
}

static void
generateBioNode (SOLARSYS_STATE *system, ELEMENT *NodeElementPtr,
		BYTE *life_init_tab)
{
	COUNT i;
	COUNT creatureType;

	creatureType = system->SysInfo.PlanetInfo.CurType;

	if (CreatureData[creatureType].Attributes & SPEED_MASK)
	{
		// Place moving creatures at a random location.
		i = (COUNT)TFB_Random ();
		NodeElementPtr->current.location.x =
				(LOBYTE (i) % (MAP_WIDTH - (8 << 1))) + 8;
		NodeElementPtr->current.location.y =
				(HIBYTE (i) % (MAP_HEIGHT - (8 << 1))) + 8;
	}

	if (system->PlanetSideFrame[0] == 0)
		system->PlanetSideFrame[0] =
				CaptureDrawable (LoadGraphic (CANNISTER_MASK_PMAP_ANIM));

	for (i = 0; i < MAX_LIFE_VARIATION
			&& life_init_tab[i] != (BYTE)(creatureType + 1);
			++i)
	{
		if (life_init_tab[i] != 0)
			continue;

		life_init_tab[i] = (BYTE)creatureType + 1;

		system->PlanetSideFrame[i + 3] = load_life_form (creatureType);
		break;
	}

	NodeElementPtr->mass_points = (BYTE)creatureType;
	NodeElementPtr->hit_points = HINIBBLE (
			CreatureData[creatureType].ValueAndHitPoints);
	DisplayArray[NodeElementPtr->PrimIndex].
			Object.Stamp.frame = SetAbsFrameIndex (
			system->PlanetSideFrame[i + 3], (COUNT)TFB_Random ());
}

void
GeneratePlanetSide (void)
{
	SIZE scan;
	BYTE life_init_tab[MAX_LIFE_VARIATION];
			// life_init_tab is filled with the creature types of already
			// selected creatures. If an entry is 0, none has been selected
			// yet, otherwise, it is 1 more than the creature type.

	InitDisplayList ();
	if (pSolarSysState->pOrbitalDesc->data_index & PLANET_SHIELDED)
		return;

	memset (life_init_tab, 0, sizeof life_init_tab);

	for (scan = BIOLOGICAL_SCAN; scan >= MINERAL_SCAN; --scan)
	{
		COUNT num_nodes;
		FRAME f;

		f = SetAbsFrameIndex (MiscDataFrame,
				NUM_SCANDOT_TRANSITIONS * (scan - ENERGY_SCAN));

		pSolarSysState->CurNode = (COUNT)~0;
		callGenerateForScanType (pSolarSysState,
				pSolarSysState->pOrbitalDesc, &pSolarSysState->CurNode, scan);

		num_nodes = pSolarSysState->CurNode;
		while (num_nodes--)
		{
			HELEMENT hNodeElement;
			ELEMENT *NodeElementPtr;

			if (isNodeRetrieved (&pSolarSysState->SysInfo.PlanetInfo,
					scan, num_nodes))
				continue;

			hNodeElement = AllocElement ();
			if (!hNodeElement)
				continue;

			LockElement (hNodeElement, &NodeElementPtr);

			pSolarSysState->CurNode = num_nodes;
			callGenerateForScanType (pSolarSysState,
					pSolarSysState->pOrbitalDesc, &pSolarSysState->CurNode,
					scan);

			NodeElementPtr->scan_node = MAKE_WORD (scan, num_nodes + 1);
			NodeElementPtr->playerNr = PS_NON_PLAYER;
			NodeElementPtr->current.location.x =
					pSolarSysState->SysInfo.PlanetInfo.CurPt.x;
			NodeElementPtr->current.location.y =
					pSolarSysState->SysInfo.PlanetInfo.CurPt.y;

			SetPrimType (&DisplayArray[NodeElementPtr->PrimIndex], STAMP_PRIM);
			if (scan == MINERAL_SCAN)
			{
				COUNT EType;

				EType = pSolarSysState->SysInfo.PlanetInfo.CurType;
				NodeElementPtr->turn_wait = (BYTE)EType;
				NodeElementPtr->mass_points = HIBYTE (
						pSolarSysState->SysInfo.PlanetInfo.CurDensity);
				NodeElementPtr->current.image.frame = SetAbsFrameIndex (
						MiscDataFrame, (NUM_SCANDOT_TRANSITIONS * 2)
						+ ElementCategory (EType) * 5);
				NodeElementPtr->next.image.frame = SetRelFrameIndex (
						NodeElementPtr->current.image.frame, LOBYTE (
						pSolarSysState->SysInfo.PlanetInfo.CurDensity) + 1);
				DisplayArray[NodeElementPtr->PrimIndex].Object.Stamp.frame =
						IncFrameIndex (NodeElementPtr->next.image.frame);
			}
			else  /* (scan == BIOLOGICAL_SCAN || scan == ENERGY_SCAN) */
			{
				NodeElementPtr->current.image.frame = f;
				NodeElementPtr->next.image.frame = SetRelFrameIndex (
						f, NUM_SCANDOT_TRANSITIONS - 1);
				NodeElementPtr->turn_wait = MAKE_BYTE (4, 4);
				NodeElementPtr->preprocess_func = object_animation;
				if (scan == ENERGY_SCAN)
				{
					if (pSolarSysState->SysInfo.PlanetInfo.CurType == 1)
					{
						NodeElementPtr->mass_points = 0;
					}
					else if (pSolarSysState->SysInfo.PlanetInfo.CurType == 2)
					{
						// Special case: Fwiffo
						NodeElementPtr->mass_points = 1;
					}
					else
						NodeElementPtr->mass_points = MAX_SCROUNGED;
					DisplayArray[NodeElementPtr->PrimIndex].Object.Stamp.frame =
							pSolarSysState->PlanetSideFrame[1];
				}
				else /* (scan == BIOLOGICAL_SCAN) */
				{
					generateBioNode (pSolarSysState, NodeElementPtr,
							life_init_tab);
				}
			}

			NodeElementPtr->next.location.x =
					NodeElementPtr->current.location.x << MAG_SHIFT;
			NodeElementPtr->next.location.y =
					NodeElementPtr->current.location.y << MAG_SHIFT;
			UnlockElement (hNodeElement);

			PutElement (hNodeElement);
		}
	}
}

bool
isNodeRetrieved (PLANET_INFO *planetInfo, BYTE scanType, BYTE nodeNr)
{
	return (planetInfo->ScanRetrieveMask[scanType] & ((DWORD) 1 << nodeNr))
			!= 0;
}

void
setNodeRetrieved (PLANET_INFO *planetInfo, BYTE scanType, BYTE nodeNr)
{
	planetInfo->ScanRetrieveMask[scanType] |= ((DWORD) 1 << nodeNr);
}

void
setNodeNotRetrieved (PLANET_INFO *planetInfo, BYTE scanType, BYTE nodeNr)
{
	planetInfo->ScanRetrieveMask[scanType] &= ~((DWORD) 1 << nodeNr);
}


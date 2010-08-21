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
#include "../colors.h"
#include "../controls.h"
#include "../menustat.h"
		// for DrawMenuStateStrings()
#include "../encount.h"
#include "../races.h"
#include "../gamestr.h"
#include "../gendef.h"
#include "../globdata.h"
#include "../sis.h"
#include "../init.h"
#include "../shipcont.h"
#include "../gameopt.h"
#include "../nameref.h"
#include "../resinst.h"
#include "../settings.h"
#include "../ipdisp.h"
#include "../grpinfo.h"
#include "../process.h"
#include "../load.h"
#include "../setup.h"
#include "../sounds.h"
#include "../state.h"
#include "../uqmdebug.h"
#include "options.h"
#include "libs/graphics/gfx_common.h"
#include "libs/mathlib.h"
#include "libs/log.h"
#include "libs/misc.h"


//#define DEBUG_SOLARSYS
//#define SMOOTH_SYSTEM_ZOOM  1

#define IP_FRAME_RATE  (ONE_SECOND / 30)

static BOOLEAN DoIpFlight (SOLARSYS_STATE *pSS);
static void DrawSystem (SIZE radius, BOOLEAN IsInnerSystem);
static FRAME CreateStarBackGround (void);
static void DrawInnerSystem (void);
static void DrawOuterSystem (void);
static void ValidateOrbits (void);

// SolarSysMenu() items
enum SolarSysMenuMenuItems
{
	// XXX: Must match the enum in menustat.h
	STARMAP = 1,
	EQUIP_DEVICE,
	CARGO,
	ROSTER,
	GAME_MENU,
	NAVIGATION,
};


SOLARSYS_STATE *pSolarSysState;
FRAME SISIPFrame;
FRAME SunFrame;
FRAME OrbitalFrame;
FRAME SpaceJunkFrame;
COLORMAP OrbitalCMap;
COLORMAP SunCMap;
MUSIC_REF SpaceMusic;

SIZE EncounterRace;
BYTE EncounterGroup;
		// last encountered group info

static FRAME StarsFrame;
		// prepared star-field graphic
static FRAME SolarSysFrame;
		// saved solar system view graphic

static RECT scaleRect;
		// system zooms in when the flagship enters this rect


#define DISPLAY_TO_LOC  (DISPLAY_FACTOR >> 1)

POINT
locationToDisplay (POINT pt, SIZE scaleRadius)
{
	POINT out;

	out.x = (SIS_SCREEN_WIDTH >> 1)
			+ (long)pt.x * DISPLAY_TO_LOC / scaleRadius;
	out.y = (SIS_SCREEN_HEIGHT >> 1)
			+ (long)pt.y * DISPLAY_TO_LOC / scaleRadius;

	return out;
}

POINT
displayToLocation (POINT pt, SIZE scaleRadius)
{
	POINT out;

	out.x = ((long)pt.x - (SIS_SCREEN_WIDTH >> 1))
			* scaleRadius / DISPLAY_TO_LOC;
	out.y = ((long)pt.y - (SIS_SCREEN_HEIGHT >> 1))
			* scaleRadius / DISPLAY_TO_LOC;

	return out;
}

POINT
planetOuterLocation (COUNT planetI)
{
	SIZE scaleRadius = pSolarSysState->SunDesc[0].radius;
	return displayToLocation (pSolarSysState->PlanetDesc[planetI].image.origin,
			scaleRadius);
}

bool
worldIsPlanet (const SOLARSYS_STATE *solarSys, const PLANET_DESC *world)
{
	return world->pPrevDesc == solarSys->SunDesc;
}

bool
worldIsMoon (const SOLARSYS_STATE *solarSys, const PLANET_DESC *world)
{
	return world->pPrevDesc != solarSys->SunDesc;
}

// Returns the planet index of the world. If the world is a moon, then
// this is the index of the planet it is orbiting.
COUNT
planetIndex (const SOLARSYS_STATE *solarSys, const PLANET_DESC *world)
{
	const PLANET_DESC *planet = worldIsPlanet (solarSys, world) ?
			world : world->pPrevDesc;
	return planet - solarSys->PlanetDesc;
}

COUNT
moonIndex (const SOLARSYS_STATE *solarSys, const PLANET_DESC *moon)
{
	assert (!worldIsPlanet (solarSys, moon));
	return moon - solarSys->MoonDesc;
}

// Test whether 'world' is the planetI-th planet, and if moonI is not
// set to MATCH_PLANET, also whether 'world' is the moonI-th moon.
bool
matchWorld (const SOLARSYS_STATE *solarSys, const PLANET_DESC *world,
		BYTE planetI, BYTE moonI)
{
	// Check whether we have the right planet.
	if (planetIndex (solarSys, world) != planetI)
		return false;

	if (moonI == MATCH_PLANET)
	{
		// Only test whether we are at the planet.
		if (!worldIsPlanet (solarSys, world))
			return false;
	}
	else
	{
		// Test whether the moon matches too
		if (!worldIsMoon (solarSys, world))
			return false;

		if (moonIndex (solarSys, world) != moonI)
			return false;
	}

	return true;
}

bool
playerInSolarSystem (void)
{
	return pSolarSysState != NULL;
}

bool
playerInPlanetOrbit (void)
{
	return playerInSolarSystem () && pSolarSysState->InOrbit;
}

bool
playerInInnerSystem (void)
{
	assert (playerInSolarSystem ());
	assert (pSolarSysState->pBaseDesc == pSolarSysState->PlanetDesc
			|| pSolarSysState->pBaseDesc == pSolarSysState->MoonDesc);
	return pSolarSysState->pBaseDesc != pSolarSysState->PlanetDesc;
}

static void
GenerateMoons (SOLARSYS_STATE *system, PLANET_DESC *planet)
{
	COUNT i;
	COUNT facing;
	PLANET_DESC *pMoonDesc;
	DWORD old_seed;

	old_seed = TFB_SeedRandom (planet->rand_seed);

	(*system->genFuncs->generateName) (system, planet);
	(*system->genFuncs->generateMoons) (system, planet);

	facing = NORMALIZE_FACING (ANGLE_TO_FACING (
			ARCTAN (planet->location.x, planet->location.y)));
	for (i = 0, pMoonDesc = &system->MoonDesc[0];
			i < MAX_MOONS; ++i, ++pMoonDesc)
	{
		pMoonDesc->pPrevDesc = planet;
		if (i >= planet->NumPlanets)
			continue;
		
		pMoonDesc->temp_color = planet->temp_color;
	}

	TFB_SeedRandom (old_seed);
}

void
FreeIPData (void)
{
	DestroyDrawable (ReleaseDrawable (SISIPFrame));
	SISIPFrame = 0;
	DestroyDrawable (ReleaseDrawable (SunFrame));
	SunFrame = 0;
	DestroyColorMap (ReleaseColorMap (SunCMap));
	SunCMap = 0;
	DestroyColorMap (ReleaseColorMap (OrbitalCMap));
	OrbitalCMap = 0;
	DestroyDrawable (ReleaseDrawable (OrbitalFrame));
	OrbitalFrame = 0;
	DestroyDrawable (ReleaseDrawable (SpaceJunkFrame));
	SpaceJunkFrame = 0;
	DestroyMusic (SpaceMusic);
	SpaceMusic = 0;
}

void
LoadIPData (void)
{
	if (SpaceJunkFrame == 0)
	{
		SpaceJunkFrame = CaptureDrawable (
				LoadGraphic (IPBKGND_MASK_PMAP_ANIM));
		SISIPFrame = CaptureDrawable (LoadGraphic (SISIP_MASK_PMAP_ANIM));

		OrbitalCMap = CaptureColorMap (LoadColorMap (ORBPLAN_COLOR_MAP));
		OrbitalFrame = CaptureDrawable (
				LoadGraphic (ORBPLAN_MASK_PMAP_ANIM));
		SunCMap = CaptureColorMap (LoadColorMap (IPSUN_COLOR_MAP));
		SunFrame = CaptureDrawable (LoadGraphic (SUN_MASK_PMAP_ANIM));

		SpaceMusic = LoadMusic (IP_MUSIC);
	}
}
	

static void
sortPlanetPositions (void)
{
	COUNT i;
	SIZE sort_array[MAX_PLANETS + 1];

	// When this part is done, sort_array will contain the indices to
	// all planets, sorted on their y position.
	// The sun itself, which has its data located at
	// pSolarSysState->PlanetDesc[-1], is included in this array.
	// Very ugly stuff, but it's correct.

	// Initialise sort_array.
	for (i = 0; i <= pSolarSysState->SunDesc[0].NumPlanets; ++i)
		sort_array[i] = i - 1;

	// Sort sort_array, based on the positions of the planets/sun.
	for (i = 0; i <= pSolarSysState->SunDesc[0].NumPlanets; ++i)
	{
		COUNT j;

		for (j = pSolarSysState->SunDesc[0].NumPlanets; j > i; --j)
		{
			SIZE real_i, real_j;

			real_i = sort_array[i];
			real_j = sort_array[j];
			if (pSolarSysState->PlanetDesc[real_i].image.origin.y >
					pSolarSysState->PlanetDesc[real_j].image.origin.y)
			{
				SIZE temp;

				temp = sort_array[i];
				sort_array[i] = sort_array[j];
				sort_array[j] = temp;
			}
		}
	}

	// Put the results of the sorting in the solar system structure.
	pSolarSysState->FirstPlanetIndex = sort_array[0];
	pSolarSysState->LastPlanetIndex =
			sort_array[pSolarSysState->SunDesc[0].NumPlanets];
	for (i = 0; i <= pSolarSysState->SunDesc[0].NumPlanets; ++i) {
		PLANET_DESC *planet = &pSolarSysState->PlanetDesc[sort_array[i]];
		planet->NextIndex = sort_array[i + 1];
	}
}

static void
initSolarSysSISCharacteristics (void)
{
	BYTE i;
	BYTE num_thrusters;

	num_thrusters = 0;
	for (i = 0; i < NUM_DRIVE_SLOTS; ++i)
	{
		if (GLOBAL_SIS (DriveSlots[i]) == FUSION_THRUSTER)
			++num_thrusters;
	}
	pSolarSysState->max_ship_speed = (BYTE)(
			(num_thrusters + 5) * IP_SHIP_THRUST_INCREMENT);

	pSolarSysState->turn_wait = IP_SHIP_TURN_WAIT;
	for (i = 0; i < NUM_JET_SLOTS; ++i)
	{
		if (GLOBAL_SIS (JetSlots[i]) == TURNING_JETS)
			pSolarSysState->turn_wait -= IP_SHIP_TURN_DECREMENT;
	}
}

static DWORD
seedRandomForSolarSys (void)
{
	return TFB_SeedRandom (MAKE_DWORD (CurStarDescPtr->star_pt.x,
			CurStarDescPtr->star_pt.y));
}

// Returns an orbital PLANET_DESC when player is in orbit
static PLANET_DESC *
LoadSolarSys (void)
{
	COUNT i;
	PLANET_DESC *orbital = NULL;
	PLANET_DESC *pCurDesc;
	DWORD old_seed;
#define NUM_TEMP_RANGES 5
	static const Color temp_color_array[NUM_TEMP_RANGES] =
	{
		BUILD_COLOR (MAKE_RGB15_INIT (0x00, 0x00, 0x0E), 0x54),
		BUILD_COLOR (MAKE_RGB15_INIT (0x00, 0x06, 0x08), 0x62),
		BUILD_COLOR (MAKE_RGB15_INIT (0x00, 0x0B, 0x00), 0x6D),
		BUILD_COLOR (MAKE_RGB15_INIT (0x0F, 0x00, 0x00), 0x2D),
		BUILD_COLOR (MAKE_RGB15_INIT (0x0F, 0x08, 0x00), 0x75),
	};

	old_seed = seedRandomForSolarSys ();

	SunFrame = SetAbsFrameIndex (SunFrame, STAR_TYPE (CurStarDescPtr->Type));

	pCurDesc = &pSolarSysState->SunDesc[0];
	pCurDesc->pPrevDesc = 0;
	pCurDesc->rand_seed = TFB_Random ();

	pCurDesc->data_index = STAR_TYPE (CurStarDescPtr->Type);
	pCurDesc->location.x = 0;
	pCurDesc->location.y = 0;
	pCurDesc->image.origin = pCurDesc->location;
	pCurDesc->image.frame = SunFrame;

	(*pSolarSysState->genFuncs->generatePlanets) (pSolarSysState);
	if (GET_GAME_STATE (PLANETARY_CHANGE))
	{
		PutPlanetInfo ();
		SET_GAME_STATE (PLANETARY_CHANGE, 0);
	}

	for (i = 0, pCurDesc = pSolarSysState->PlanetDesc;
			i < MAX_PLANETS; ++i, ++pCurDesc)
	{
		pCurDesc->pPrevDesc = &pSolarSysState->SunDesc[0];
		pCurDesc->image.origin = pCurDesc->location;
		if (i >= pSolarSysState->SunDesc[0].NumPlanets)
		{
			pCurDesc->image.frame = 0;
		}
		else
		{
			COUNT index;
			SYSTEM_INFO SysInfo;

			DoPlanetaryAnalysis (&SysInfo, pCurDesc);
			index = (SysInfo.PlanetInfo.SurfaceTemperature + 250) / 100;
			if (index >= NUM_TEMP_RANGES)
				index = NUM_TEMP_RANGES - 1;
			pCurDesc->temp_color = temp_color_array[index];
		}
	}

	sortPlanetPositions ();

	if (!GLOBAL (ip_planet))
	{	// Outer system
		pSolarSysState->pBaseDesc = pSolarSysState->PlanetDesc;
		pSolarSysState->pOrbitalDesc = NULL;
	}
	else
	{	// Inner system
		pSolarSysState->SunDesc[0].location = GLOBAL (ip_location);
		GLOBAL (ip_location) = displayToLocation (
				GLOBAL (ShipStamp.origin), MAX_ZOOM_RADIUS);

		i = GLOBAL (ip_planet) - 1;
		pSolarSysState->pOrbitalDesc = &pSolarSysState->PlanetDesc[i];
		GenerateMoons (pSolarSysState, pSolarSysState->pOrbitalDesc);
		pSolarSysState->pBaseDesc = pSolarSysState->MoonDesc;

		SET_GAME_STATE (PLANETARY_LANDING, 0);
	}

	initSolarSysSISCharacteristics ();

	if (GLOBAL (in_orbit))
	{	// Only when loading a game into orbital
		i = GLOBAL (in_orbit) - 1;
		if (i == 0)
		{	// Orbiting the planet itself
			orbital = pSolarSysState->pBaseDesc->pPrevDesc;
		}
		else
		{	// Orbiting a moon
			// -1 because planet itself is 1, and moons have to be 1-based
			i -= 1;
			orbital = &pSolarSysState->MoonDesc[i];
		}
		GLOBAL (ip_location) = pSolarSysState->SunDesc[0].location;
		GLOBAL (in_orbit) = 0;
	}
	else
	{
		i = GLOBAL (ShipFacing);
		// XXX: Solar system reentry test depends on ShipFacing != 0
		if (i == 0)
			++i;

		GLOBAL (ShipStamp.frame) = SetAbsFrameIndex (SISIPFrame, i - 1);
	}

	// Restore RNG state:
	TFB_SeedRandom (old_seed);

	return orbital;
}

static void
saveNonOrbitalLocation (void)
{
	// XXX: Solar system reentry test depends on ShipFacing != 0
	GLOBAL (ShipFacing) = GetFrameIndex (GLOBAL (ShipStamp.frame)) + 1;
	GLOBAL (in_orbit) = 0;
	if (!playerInInnerSystem ())
	{
		GLOBAL (ip_planet) = 0;
	}
	else
	{
		// ip_planet is 1-based because code tests for ip_planet!=0
		GLOBAL (ip_planet) = 1 + planetIndex (pSolarSysState,
				pSolarSysState->pOrbitalDesc);
		GLOBAL (ip_location) = pSolarSysState->SunDesc[0].location;
	}
}

static void
FreeSolarSys (void)
{
	if (pSolarSysState->InIpFlight)
	{
		pSolarSysState->InIpFlight = FALSE;
		
		if (!(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD)))
			saveNonOrbitalLocation ();
	}

	DestroyDrawable (ReleaseDrawable (SolarSysFrame));
	SolarSysFrame = NULL;

	StopMusic ();

//    FreeIPData ();
}

static FRAME
getCollisionFrame (PLANET_DESC *planet, COUNT WaitPlanet)
{
	if (pSolarSysState->WaitIntersect != (COUNT)~0
			&& pSolarSysState->WaitIntersect != WaitPlanet)
	{	// New collisions are with a single point (center of planet)
		return DecFrameIndex (stars_in_space);
	}
	else
	{	// Existing collisions are cleared only once the ship does not
		// intersect anymore with a full planet image
		return planet->image.frame;
	}
}

// Returns the planet with which the flagship is colliding
static PLANET_DESC *
CheckIntersect (BOOLEAN just_checking)
{
	COUNT i;
	PLANET_DESC *pCurDesc;
	INTERSECT_CONTROL ShipIntersect, PlanetIntersect;
	COUNT NewWaitPlanet;
	BYTE PlanetOffset, MoonOffset;

	// Check collisions with the system center object
	// This may be the planet in inner view, or the sun
	pCurDesc = pSolarSysState->pBaseDesc->pPrevDesc;
	PlanetOffset = pCurDesc - pSolarSysState->PlanetDesc + 1;
	MoonOffset = 1; // the planet itself

	ShipIntersect.IntersectStamp.origin = GLOBAL (ShipStamp.origin);
	ShipIntersect.EndPoint = ShipIntersect.IntersectStamp.origin;
	ShipIntersect.IntersectStamp.frame = GLOBAL (ShipStamp.frame);

	PlanetIntersect.IntersectStamp.origin.x = SIS_SCREEN_WIDTH >> 1;
	PlanetIntersect.IntersectStamp.origin.y = SIS_SCREEN_HEIGHT >> 1;
	PlanetIntersect.EndPoint = PlanetIntersect.IntersectStamp.origin;

	PlanetIntersect.IntersectStamp.frame = getCollisionFrame (pCurDesc,
			MAKE_WORD (PlanetOffset, MoonOffset));

	// Start with no collisions
	NewWaitPlanet = 0;

	if (pCurDesc != pSolarSysState->SunDesc /* can't intersect with sun */
			&& DrawablesIntersect (&ShipIntersect,
			&PlanetIntersect, MAX_TIME_VALUE))
	{
#ifdef DEBUG_SOLARSYS
		log_add (log_Debug, "0: Planet %d, Moon %d", PlanetOffset,
				MoonOffset);
#endif /* DEBUG_SOLARSYS */
		NewWaitPlanet = MAKE_WORD (PlanetOffset, MoonOffset);
		if (pSolarSysState->WaitIntersect != (COUNT)~0
				&& pSolarSysState->WaitIntersect != NewWaitPlanet)
		{
			pSolarSysState->WaitIntersect = NewWaitPlanet;
#ifdef DEBUG_SOLARSYS
			log_add (log_Debug, "Star index = %d, Planet index = %d, <%d, %d>",
					CurStarDescPtr - star_array,
					pCurDesc - pSolarSysState->PlanetDesc,
					pSolarSysState->SunDesc[0].location.x,
					pSolarSysState->SunDesc[0].location.y);
#endif /* DEBUG_SOLARSYS */
			return pCurDesc;
		}
	}

	for (i = pCurDesc->NumPlanets,
			pCurDesc = pSolarSysState->pBaseDesc; i; --i, ++pCurDesc)
	{
		PlanetIntersect.IntersectStamp.origin = pCurDesc->image.origin;
		PlanetIntersect.EndPoint = PlanetIntersect.IntersectStamp.origin;
		if (playerInInnerSystem ())
		{
			PlanetOffset = pCurDesc->pPrevDesc -
					pSolarSysState->PlanetDesc;
			MoonOffset = pCurDesc - pSolarSysState->MoonDesc + 2;
		}
		else
		{
			PlanetOffset = pCurDesc - pSolarSysState->PlanetDesc;
			MoonOffset = 0;
		}
		++PlanetOffset;
		PlanetIntersect.IntersectStamp.frame = getCollisionFrame (pCurDesc,
				MAKE_WORD (PlanetOffset, MoonOffset));

		if (DrawablesIntersect (&ShipIntersect,
				&PlanetIntersect, MAX_TIME_VALUE))
		{
#ifdef DEBUG_SOLARSYS
			log_add (log_Debug, "1: Planet %d, Moon %d", PlanetOffset,
					MoonOffset);
#endif /* DEBUG_SOLARSYS */
			NewWaitPlanet = MAKE_WORD (PlanetOffset, MoonOffset);
			
			if (pSolarSysState->WaitIntersect == (COUNT)~0)
			{	// All collisions disallowed, but the ship is still colliding
				// with something. Collisions will remain disabled.
				break;
			}
			else if (pSolarSysState->WaitIntersect == NewWaitPlanet)
			{	// Existing and continued collision -- ignore
				continue;
			}
			
			if (playerInInnerSystem ())
			{	// Collision in the inner system (starts orbital)
				pSolarSysState->WaitIntersect = NewWaitPlanet;
			}
			else
			{	// Going into an inner system
				// So there is now no existing collision
				if (!just_checking)
					pSolarSysState->WaitIntersect = 0;
			}
			return pCurDesc;
		}
	}

	// This records the planet/moon with which the ship just collided
	// It may be a previously existing collision also (the value won't change)
	// If all collisions were disabled, this will reenable then once the ship
	// stops colliding with any planets
	if (pSolarSysState->WaitIntersect != (COUNT)~0 || NewWaitPlanet == 0)
		pSolarSysState->WaitIntersect = NewWaitPlanet;

	return NULL;
}

static void
GetOrbitRect (RECT *pRect, COORD dx, COORD dy, SIZE radius,
		int xnumer, int ynumer, int denom)
{
	pRect->corner.x = (SIS_SCREEN_WIDTH >> 1) + (long)-dx * xnumer / denom;
	pRect->corner.y = (SIS_SCREEN_HEIGHT >> 1) + (long)-dy * ynumer / denom;
	pRect->extent.width = (long)radius * (xnumer << 1) / denom;
	pRect->extent.height = pRect->extent.width >> 1;
}

static void
GetPlanetOrbitRect (RECT *r, PLANET_DESC *planet, int sizeNumer,
		int dyNumer, int denom)
{
	COORD dx, dy;

	dx = planet->radius;
	dy = planet->radius;
	if (sizeNumer > DISPLAY_FACTOR)
	{
		dx = dx + planet->location.x;
		dy = (dy + planet->location.y) << 1;
	}
	GetOrbitRect (r, dx, dy, planet->radius, sizeNumer, dyNumer, denom);
}

static void
ValidateOrbit (PLANET_DESC *planet, int sizeNumer, int dyNumer, int denom)
{
	COUNT index;

	if (sizeNumer <= DISPLAY_FACTOR)
	{	// All planets in outer view, and moons in inner
		RECT r;

		GetPlanetOrbitRect (&r, planet, sizeNumer, dyNumer, denom);

		// Calculate the location of the planet's image
		r.corner.x += (r.extent.width >> 1);
		r.corner.y += (r.extent.height >> 1);
		r.corner.x += (long)planet->location.x * sizeNumer / denom;
		// Ellipse function always has coefficients a^2 = 2 * b^2
		r.corner.y += (long)planet->location.y * (sizeNumer / 2) / denom;

		planet->image.origin = r.corner;
	}

	// Calculate the size and lighting angle of planet's image and
	// set the image that will be drawn
	index = planet->data_index & ~WORLD_TYPE_SPECIAL;
	if (index < NUMBER_OF_PLANET_TYPES)
	{	// The world is a normal planetary body (planet or moon)
		BYTE Type;
		COUNT Size;
		COUNT angle;

		Type = PlanData[index].Type;
		Size = PLANSIZE (Type);
		if (sizeNumer > DISPLAY_FACTOR)
		{
			Size += 3;
		}
		else if (worldIsMoon (pSolarSysState, planet))
		{
			Size += 2;
		}
		else if (denom <= (MAX_ZOOM_RADIUS >> 2))
		{
			++Size;
			if (denom == MIN_ZOOM_RADIUS)
				++Size;
		}
		
		if (worldIsPlanet (pSolarSysState, planet))
		{	// Planet
			angle = ARCTAN (planet->location.x, planet->location.y);
		}
		else
		{	// Moon
			angle = ARCTAN (planet->pPrevDesc->location.x,
					planet->pPrevDesc->location.y);
		}
		planet->image.frame =	SetAbsFrameIndex (OrbitalFrame,
				(Size << FACING_SHIFT) + NORMALIZE_FACING (
				ANGLE_TO_FACING (angle)));
	}
	else if (planet->data_index == HIERARCHY_STARBASE)
	{
		planet->image.frame = SetAbsFrameIndex (SpaceJunkFrame, 16);
	}
	else if (planet->data_index == SA_MATRA)
	{
		planet->image.frame = SetAbsFrameIndex (SpaceJunkFrame, 19);
	}
}

static void
DrawOrbit (PLANET_DESC *planet, int sizeNumer, int dyNumer, int denom)
{
	RECT r;

	GetPlanetOrbitRect (&r, planet, sizeNumer, dyNumer, denom);

	SetContextForeGroundColor (planet->temp_color);
	DrawOval (&r, 1);
}

static SIZE
FindRadius (POINT shipLoc, SIZE fromRadius)
{
	SIZE nextRadius;
	POINT displayLoc;

	do
	{
		fromRadius >>= 1;
		if (fromRadius > MIN_ZOOM_RADIUS)
			nextRadius = fromRadius >> 1;
		else
			nextRadius = 0; // scaleRect will be nul

		GetOrbitRect (&scaleRect, nextRadius, nextRadius, nextRadius,
				DISPLAY_FACTOR, DISPLAY_FACTOR >> 2, fromRadius);
		displayLoc = locationToDisplay (shipLoc, fromRadius);
	
	} while (pointWithinRect (scaleRect, displayLoc));

	return fromRadius;
}

static UWORD
flagship_inertial_thrust (COUNT CurrentAngle)
{
	BYTE max_speed;
	SIZE cur_delta_x, cur_delta_y;
	COUNT TravelAngle;
	VELOCITY_DESC *VelocityPtr;

	max_speed = pSolarSysState->max_ship_speed;
	VelocityPtr = &GLOBAL (velocity);
	GetCurrentVelocityComponents (VelocityPtr, &cur_delta_x, &cur_delta_y);
	TravelAngle = GetVelocityTravelAngle (VelocityPtr);
	if (TravelAngle == CurrentAngle
			&& cur_delta_x == COSINE (CurrentAngle, max_speed)
			&& cur_delta_y == SINE (CurrentAngle, max_speed))
		return (SHIP_AT_MAX_SPEED);
	else
	{
		SIZE delta_x, delta_y;
		DWORD desired_speed;

		delta_x = cur_delta_x
				+ COSINE (CurrentAngle, IP_SHIP_THRUST_INCREMENT);
		delta_y = cur_delta_y
				+ SINE (CurrentAngle, IP_SHIP_THRUST_INCREMENT);
		desired_speed = (DWORD) ((long) delta_x * delta_x)
				+ (DWORD) ((long) delta_y * delta_y);
		if (desired_speed <= (DWORD) ((UWORD) max_speed * max_speed))
			SetVelocityComponents (VelocityPtr, delta_x, delta_y);
		else if (TravelAngle == CurrentAngle)
		{
			SetVelocityComponents (VelocityPtr,
					COSINE (CurrentAngle, max_speed),
					SINE (CurrentAngle, max_speed));
			return (SHIP_AT_MAX_SPEED);
		}
		else
		{
			VELOCITY_DESC v;

			v = *VelocityPtr;

			DeltaVelocityComponents (&v,
					COSINE (CurrentAngle, IP_SHIP_THRUST_INCREMENT >> 1)
					- COSINE (TravelAngle, IP_SHIP_THRUST_INCREMENT),
					SINE (CurrentAngle, IP_SHIP_THRUST_INCREMENT >> 1)
					- SINE (TravelAngle, IP_SHIP_THRUST_INCREMENT));
			GetCurrentVelocityComponents (&v, &cur_delta_x, &cur_delta_y);
			desired_speed =
					(DWORD) ((long) cur_delta_x * cur_delta_x)
					+ (DWORD) ((long) cur_delta_y * cur_delta_y);
			if (desired_speed > (DWORD) ((UWORD) max_speed * max_speed))
			{
				SetVelocityComponents (VelocityPtr,
						COSINE (CurrentAngle, max_speed),
						SINE (CurrentAngle, max_speed));
				return (SHIP_AT_MAX_SPEED);
			}

			*VelocityPtr = v;
		}

		return 0;
	}
}

static void
ProcessShipControls (void)
{
	COUNT index;
	SIZE delta_x, delta_y;

	if (CurrentInputState.key[PlayerControls[0]][KEY_UP])
		delta_y = -1;
	else
		delta_y = 0;

	delta_x = 0;
	if (CurrentInputState.key[PlayerControls[0]][KEY_LEFT])
		delta_x -= 1;
	if (CurrentInputState.key[PlayerControls[0]][KEY_RIGHT])
		delta_x += 1;
		
	if (delta_x || delta_y < 0)
	{
		GLOBAL (autopilot.x) = ~0;
		GLOBAL (autopilot.y) = ~0;
	}
	else if (GLOBAL (autopilot.x) != ~0 && GLOBAL (autopilot.y) != ~0)
		delta_y = -1;
	else
		delta_y = 0;

	index = GetFrameIndex (GLOBAL (ShipStamp.frame));
	if (pSolarSysState->turn_counter)
		--pSolarSysState->turn_counter;
	else if (delta_x)
	{
		if (delta_x < 0)
			index = NORMALIZE_FACING (index - 1);
		else
			index = NORMALIZE_FACING (index + 1);

		GLOBAL (ShipStamp.frame) =
				SetAbsFrameIndex (GLOBAL (ShipStamp.frame), index);

		pSolarSysState->turn_counter = pSolarSysState->turn_wait;
	}
	if (pSolarSysState->thrust_counter)
		--pSolarSysState->thrust_counter;
	else if (delta_y < 0)
	{
#define THRUST_WAIT 1
		flagship_inertial_thrust (FACING_TO_ANGLE (index));

		pSolarSysState->thrust_counter = THRUST_WAIT;
	}
}

static void
enterInnerSystem (PLANET_DESC *planet)
{
#define INNER_ENTRY_DISTANCE  (MIN_MOON_RADIUS + ((MAX_MOONS - 1) \
		* MOON_DELTA) + (MOON_DELTA / 4))
	COUNT angle;

	// Calculate the inner system entry location and facing
	angle = FACING_TO_ANGLE (GetFrameIndex (GLOBAL (ShipStamp.frame)))
			+ HALF_CIRCLE;
	GLOBAL (ShipStamp.origin.x) = (SIS_SCREEN_WIDTH >> 1)
			+ COSINE (angle, INNER_ENTRY_DISTANCE);
	GLOBAL (ShipStamp.origin.y) = (SIS_SCREEN_HEIGHT >> 1)
			+ SINE (angle, INNER_ENTRY_DISTANCE);
	if (GLOBAL (ShipStamp.origin.y) < 0)
		GLOBAL (ShipStamp.origin.y) = 1;
	else if (GLOBAL (ShipStamp.origin.y) >= SIS_SCREEN_HEIGHT)
		GLOBAL (ShipStamp.origin.y) =
				(SIS_SCREEN_HEIGHT - 1) - 1;

	GLOBAL (ip_location) = displayToLocation (
			GLOBAL (ShipStamp.origin), MAX_ZOOM_RADIUS);
	
	pSolarSysState->SunDesc[0].location =
			planetOuterLocation (planetIndex (pSolarSysState, planet));
	ZeroVelocityComponents (&GLOBAL (velocity));

	GenerateMoons (pSolarSysState, planet);
	pSolarSysState->pBaseDesc = pSolarSysState->MoonDesc;
	pSolarSysState->pOrbitalDesc = planet;
}

static void
leaveInnerSystem (PLANET_DESC *planet)
{
	COUNT outerPlanetWait;

	pSolarSysState->pBaseDesc = pSolarSysState->PlanetDesc;
	pSolarSysState->pOrbitalDesc = NULL;

	outerPlanetWait = MAKE_WORD (planet - pSolarSysState->PlanetDesc + 1, 0);
	GLOBAL (ip_location) = pSolarSysState->SunDesc[0].location;
	XFormIPLoc (&GLOBAL (ip_location), &GLOBAL (ShipStamp.origin), TRUE);
	ZeroVelocityComponents (&GLOBAL (velocity));

	// Now the ship is in outer system (as per game logic)

	pSolarSysState->WaitIntersect = outerPlanetWait;
	// See if we also intersect with another planet, and if we do,
	// disable collisions comletely until we stop intersecting
	// with any planet at all.
	CheckIntersect (TRUE);
	if (pSolarSysState->WaitIntersect != outerPlanetWait)
		pSolarSysState->WaitIntersect = (COUNT)~0;
}

static void
enterOrbital (PLANET_DESC *planet)
{
	ZeroVelocityComponents (&GLOBAL (velocity));
	pSolarSysState->pOrbitalDesc = planet;
	pSolarSysState->InOrbit = TRUE;
}

static BOOLEAN
CheckShipLocation (SIZE *newRadius)
{
	SIZE radius;

	radius = pSolarSysState->SunDesc[0].radius;
	*newRadius = pSolarSysState->SunDesc[0].radius;
	
	if (GLOBAL (ShipStamp.origin.x) < 0
			|| GLOBAL (ShipStamp.origin.x) >= SIS_SCREEN_WIDTH
			|| GLOBAL (ShipStamp.origin.y) < 0
			|| GLOBAL (ShipStamp.origin.y) >= SIS_SCREEN_HEIGHT)
	{
		// The ship leaves the screen.
		if (!playerInInnerSystem ())
		{	// Outer zoom-out transition
			if (radius == MAX_ZOOM_RADIUS)
			{
				// The ship leaves IP.
				GLOBAL (CurrentActivity) |= END_INTERPLANETARY;
				return FALSE; // no location change
			}

			*newRadius = FindRadius (GLOBAL (ip_location),
					MAX_ZOOM_RADIUS << 1);
		}
		else
		{
			leaveInnerSystem (pSolarSysState->pOrbitalDesc);
		}
		
		return TRUE;
	}

	if (!playerInInnerSystem ()
			&& pointWithinRect (scaleRect, GLOBAL (ShipStamp.origin)))
	{	// Outer zoom-in transition
		*newRadius = FindRadius (GLOBAL (ip_location), radius);
		return TRUE;
	}

	if (GLOBAL (autopilot.x) == ~0 && GLOBAL (autopilot.y) == ~0)
	{	// Not on autopilot -- may collide with a planet
		PLANET_DESC *planet = CheckIntersect (FALSE);
		if (planet)
		{	// Collision with a planet
			if (playerInInnerSystem ())
			{	// Entering planet orbit (scans, etc.)
				enterOrbital (planet);
				return FALSE; // no location change
			}
			else
			{	// Transition to inner system
				enterInnerSystem (planet);
				return TRUE;
			}
		}
	}

	return FALSE; // no location change
}

static void
DrawSystemTransition (BOOLEAN inner)
{
	SetTransitionSource (NULL);
	BatchGraphics ();
	if (inner)
		DrawInnerSystem ();
	else
		DrawOuterSystem ();
	RedrawQueue (FALSE);
	ScreenTransition (3, NULL);
	UnbatchGraphics ();
}

static void
TransitionSystemIn (void)
{
	LockMutex (GraphicsLock);
	SetContext (SpaceContext);
	DrawSystemTransition (playerInInnerSystem ());
	UnlockMutex (GraphicsLock);
}

static void
ScaleSystem (SIZE new_radius)
{
#ifdef SMOOTH_SYSTEM_ZOOM
	// XXX: This appears to have been an attempt to zoom the system view
	//   in a different way. This code zooms gradually instead of
	//   doing a crossfade from one zoom level to the other.
	// TODO: Do not loop here, and instead increment the zoom level
	//   in IP_frame() with a function drawing the new zoom. The ship
	//   controls are not handled in the loop, and the flagship
	//   can collide with a group while zooming, and that is not handled
	//   100% correctly.
#define NUM_STEPS 10
	COUNT i;
	SIZE old_radius;
	SIZE d, step;

	old_radius = pSolarSysState->SunDesc[0].radius;

	assert (old_radius != 0);
	assert (old_radius != new_radius);

	d = new_radius - old_radius;
	step = d / NUM_STEPS;

	for (i = 0; i < NUM_STEPS - 1; ++i)
	{
		pSolarSysState->SunDesc[0].radius += step;
		XFormIPLoc (&GLOBAL (ip_location), &GLOBAL (ShipStamp.origin), TRUE);

		BatchGraphics ();
		DrawOuterSystem ();
		RedrawQueue (FALSE);
		UnbatchGraphics ();

		SleepThread (ONE_SECOND / 30);
	}
	
	// Final zoom step
	pSolarSysState->SunDesc[0].radius = new_radius;
	XFormIPLoc (&GLOBAL (ip_location), &GLOBAL (ShipStamp.origin), TRUE);
	
	BatchGraphics ();
	DrawOuterSystem ();
	RedrawQueue (FALSE);
	UnbatchGraphics ();
	
#else // !SMOOTH_SYSTEM_ZOOM
	RECT r;

	pSolarSysState->SunDesc[0].radius = new_radius;
	XFormIPLoc (&GLOBAL (ip_location), &GLOBAL (ShipStamp.origin), TRUE);

	GetContextClipRect (&r);
	SetTransitionSource (&r);
	BatchGraphics ();
	DrawOuterSystem ();
	RedrawQueue (FALSE);
	ScreenTransition (3, &r);
	UnbatchGraphics ();
#endif // SMOOTH_SYSTEM_ZOOM
}

static void
RestoreSystemView (void)
{
	STAMP s;

	s.origin.x = 0;
	s.origin.y = 0;
	s.frame = SolarSysFrame;
	DrawStamp (&s);
}

// Normally called by DoIpFlight() to process a frame
static void
IP_frame (void)
{
	BOOLEAN locChange;
	SIZE newRadius;

	LockMutex (GraphicsLock);
	SetContext (SpaceContext);

	GameClockTick ();
	ProcessShipControls ();
	
	locChange = CheckShipLocation (&newRadius);
	if (locChange)
	{
		if (playerInInnerSystem ())
		{	// Entering inner system
			DrawSystemTransition (TRUE);
		}
		else if (pSolarSysState->SunDesc[0].radius == newRadius)
		{	// Leaving inner system to outer
			DrawSystemTransition (FALSE);
		}
		else
		{	// Zooming outer system
			ScaleSystem (newRadius);
		}
	}
	else
	{	// Just flying around, minding own business..
		BatchGraphics ();
		RestoreSystemView ();
		RedrawQueue (FALSE);
		DrawAutoPilotMessage (FALSE);
		UnbatchGraphics ();
	}
	
	UnlockMutex (GraphicsLock);
}

static BOOLEAN
CheckZoomLevel (void)
{
	BOOLEAN InnerSystem;
	POINT shipLoc;

	InnerSystem = playerInInnerSystem ();
	if (InnerSystem)
		shipLoc = pSolarSysState->SunDesc[0].location;
	else
		shipLoc = GLOBAL (ip_location);

	pSolarSysState->SunDesc[0].radius = FindRadius (shipLoc,
			MAX_ZOOM_RADIUS << 1);
	if (!InnerSystem)
	{	// Update ship stamp since the radius probably changed
		XFormIPLoc (&shipLoc, &GLOBAL (ShipStamp.origin), TRUE);
	}

	return InnerSystem;
}

static void
ValidateOrbits (void)
{
	COUNT i;
	PLANET_DESC *planet;

	for (i = pSolarSysState->SunDesc[0].NumPlanets,
			planet = &pSolarSysState->PlanetDesc[0]; i; --i, ++planet)
	{
		ValidateOrbit (planet, DISPLAY_FACTOR, DISPLAY_FACTOR / 4,
				pSolarSysState->SunDesc[0].radius);
	}
}

static void
ValidateInnerOrbits (void)
{
	COUNT i;
	PLANET_DESC *planet;

	assert (playerInInnerSystem ());

	planet = pSolarSysState->pOrbitalDesc;
	ValidateOrbit (planet, DISPLAY_FACTOR * 4, DISPLAY_FACTOR,
			planet->radius);

	for (i = 0; i < planet->NumPlanets; ++i)
	{
		PLANET_DESC *moon = &pSolarSysState->MoonDesc[i];
		ValidateOrbit (moon, 2, 1, 2);
	}
}

static void
DrawInnerSystem (void)
{
	ValidateInnerOrbits ();
	DrawSystem (pSolarSysState->pOrbitalDesc->radius, TRUE);
	DrawSISTitle (GLOBAL_SIS (PlanetName));
}

static void
DrawOuterSystem (void)
{
	ValidateOrbits ();
	DrawSystem (pSolarSysState->SunDesc[0].radius, FALSE);
	DrawHyperCoords (CurStarDescPtr->star_pt);
}

static void
ResetSolarSys (void)
{
	// Originally there was a flash_task test here, however, I found no cases
	// where flash_task could be set at the time of call. The test was
	// probably needed on 3DO when IP_frame() was a task.
	assert (!pSolarSysState->InIpFlight);

	DrawMenuStateStrings (PM_STARMAP, -(PM_NAVIGATE - PM_SCAN));

	InitDisplayList ();
	// This also spawns the flagship element
	DoMissions ();

	// Figure out and note which planet/moon we just left, if any
	CheckIntersect (TRUE);
	
	pSolarSysState->InIpFlight = TRUE;

	// Do not start playing the music if we entered the solarsys only
	// to load a game (load invoked from Main menu)
	// XXX: This is quite hacky
	if (!PLRPlaying ((MUSIC_REF)~0) &&
			(LastActivity != CHECK_LOAD || NextActivity))
	{
		PlayMusic (SpaceMusic, TRUE, 1);
	}
}

static void
EnterPlanetOrbit (void)
{
	if (pSolarSysState->InIpFlight)
	{	// This means we hit a planet in IP flight; not a Load into orbit
		FreeSolarSys ();

		if (worldIsMoon (pSolarSysState, pSolarSysState->pOrbitalDesc))
		{	// Moon -- use its origin
			// XXX: The conversion functions do not error-correct, so the
			//   point we set here will change once flag_ship_preprocess()
			//   in ipdisp.c starts over again.
			GLOBAL (ShipStamp.origin) =
					pSolarSysState->pOrbitalDesc->image.origin;
		}
		else
		{	// Planet -- its origin is for the outer view, so use mid-screen
			GLOBAL (ShipStamp.origin.x) = SIS_SCREEN_WIDTH >> 1;
			GLOBAL (ShipStamp.origin.y) = SIS_SCREEN_HEIGHT >> 1;
		}
	}

	GetPlanetInfo ();
	(*pSolarSysState->genFuncs->generateOrbital) (pSolarSysState,
			pSolarSysState->pOrbitalDesc);
	LastActivity &= ~(CHECK_LOAD | CHECK_RESTART);
	if ((GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD |
			START_ENCOUNTER)) || GLOBAL_SIS (CrewEnlisted) == (COUNT)~0
			|| GET_GAME_STATE (CHMMR_BOMB_STATE) == 2)
		return;

	// Implement a to-do in generate.h for a better test
	if (pSolarSysState->TopoFrame)
	{	// We've entered orbit; LoadPlanet() called planet surface-gen code
		PlanetOrbitMenu ();
		FreePlanet ();
	}
	// Otherwise, generateOrbital function started a homeworld conversation,
	// and we did not get to the planet no matter what.

	// START_ENCOUNTER could be set by Devices menu a number of ways:
	// Talking Pet, Sun Device or a Caster over Chmmr, or
	// a Caster for Ilwrath
	// Could also have blown self up with Utwig Bomb
	if (!(GLOBAL (CurrentActivity) & (START_ENCOUNTER |
			CHECK_ABORT | CHECK_LOAD))
			&& GLOBAL_SIS (CrewEnlisted) != (COUNT)~0)
	{	// Reload the system and return to the inner view
		PLANET_DESC *orbital = LoadSolarSys ();
		assert (!orbital);
		CheckZoomLevel ();
		ValidateOrbits ();
		ValidateInnerOrbits ();
		ResetSolarSys ();

		LockMutex (GraphicsLock);
		RepairSISBorder ();
		UnlockMutex (GraphicsLock);
		TransitionSystemIn ();
	}
}

static void
InitSolarSys (void)
{
	BOOLEAN InnerSystem;
	BOOLEAN Reentry;
	PLANET_DESC *orbital;

	LockMutex (GraphicsLock);

	LoadIPData ();
	LoadLanderData ();
	UnlockMutex (GraphicsLock);

	Reentry = (GLOBAL (ShipFacing) != 0);
	if (!Reentry)
	{
		GLOBAL (autopilot.x) = ~0;
		GLOBAL (autopilot.y) = ~0;

		GLOBAL (ShipStamp.origin.x) = SIS_SCREEN_WIDTH >> 1;
		GLOBAL (ShipStamp.origin.y) = SIS_SCREEN_HEIGHT - 2;
		
		GLOBAL (ip_location) = displayToLocation (GLOBAL (ShipStamp.origin),
				MAX_ZOOM_RADIUS);
	}

	LockMutex (GraphicsLock);

	StarsFrame = CreateStarBackGround ();
	
	SetContext (SpaceContext);
	SetContextFGFrame (Screen);
	SetContextBackGroundColor (BLACK_COLOR);
	
	UnlockMutex (GraphicsLock);

	orbital = LoadSolarSys ();
	InnerSystem = CheckZoomLevel ();
	ValidateOrbits ();
	if (InnerSystem)
		ValidateInnerOrbits ();

	if (Reentry)
	{
		(*pSolarSysState->genFuncs->reinitNpcs) (pSolarSysState);
	}
	else
	{
		EncounterRace = -1;
		EncounterGroup = 0;
		GLOBAL (BattleGroupRef) = 0;
		ReinitQueue (&GLOBAL (ip_group_q));
		ReinitQueue (&GLOBAL (npc_built_ship_q));
		(*pSolarSysState->genFuncs->initNpcs) (pSolarSysState);
	}

	if (orbital)
	{
		enterOrbital (orbital);
	}
	else
	{	// Draw the borders, the system (inner or outer) and fade/transition
		LockMutex (GraphicsLock);
		SetContext (SpaceContext);

		SetTransitionSource (NULL);
		BatchGraphics ();

		DrawSISFrame ();
		DrawSISMessage (NULL);

		ResetSolarSys ();

		if (LastActivity == (CHECK_LOAD | CHECK_RESTART))
		{	// Starting a new game, NOT from load!
			// We have to fade the screen in from intro or menu
			DrawOuterSystem ();
			RedrawQueue (FALSE);
			UnbatchGraphics ();
			FadeScreen (FadeAllToColor, ONE_SECOND / 2);

			LastActivity = 0;
		}
		else if (LastActivity == CHECK_LOAD && !NextActivity)
		{	// Called just to load a game; invoked from Main menu
			// No point in drawing anything
			UnbatchGraphics ();
		}
		else
		{	// Entered a new system, or loaded into inner or outer
			if (InnerSystem)
				DrawInnerSystem ();
			else
				DrawOuterSystem ();
			RedrawQueue (FALSE);
			ScreenTransition (3, NULL);
			UnbatchGraphics ();

			LastActivity &= ~CHECK_LOAD;
		}
		
		UnlockMutex (GraphicsLock);
	}
}

static void
endInterPlanetary (void)
{
	GLOBAL (CurrentActivity) &= ~END_INTERPLANETARY;
	
	if (!(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD)))
	{
		// These are game state changing ops and so cannot be
		// called once another game has been loaded!
		(*pSolarSysState->genFuncs->uninitNpcs) (pSolarSysState);
		SET_GAME_STATE (USED_BROADCASTER, 0);
	}
}

// Find the closest planet to a point, in interplanetary.
static PLANET_DESC *
closestPlanetInterPlanetary (const POINT *point)
{
	BYTE i;
	BYTE numPlanets;
	DWORD bestDistSquared;
	PLANET_DESC *bestPlanet = NULL;

	assert(pSolarSysState != NULL);

	numPlanets = pSolarSysState->SunDesc[0].NumPlanets;

	bestDistSquared = (DWORD) -1;  // Maximum value of DWORD.
	for (i = 0; i < numPlanets; i++)
	{
		PLANET_DESC *planet = &pSolarSysState->PlanetDesc[i];

		SIZE dx = point->x - planet->image.origin.x;
		SIZE dy = point->y - planet->image.origin.y;

		DWORD distSquared = (DWORD) ((long) dx * dx + (long) dy * dy);
		if (distSquared < bestDistSquared)
		{
			bestDistSquared = distSquared;
			bestPlanet = planet;
		}
	}

	return bestPlanet;
}

static void
UninitSolarSys (void)
{
	FreeSolarSys ();

//FreeLanderData ();
//FreeIPData ();

	DestroyDrawable (ReleaseDrawable (StarsFrame));
	StarsFrame = NULL;

	if (GLOBAL (CurrentActivity) & END_INTERPLANETARY)
	{
		endInterPlanetary ();
		return;
	}

	if ((GLOBAL (CurrentActivity) & START_ENCOUNTER) && EncounterGroup)
	{
		GetGroupInfo (GLOBAL (BattleGroupRef), EncounterGroup);
		// Generate the encounter location name based on the closest planet

		if (GLOBAL (ip_planet) == 0)
		{
			PLANET_DESC *planet =
					closestPlanetInterPlanetary (&GLOBAL (ShipStamp.origin));

			(*pSolarSysState->genFuncs->generateName) (
					pSolarSysState, planet);
		}
	}
}

static void
CalcSunSize (PLANET_DESC *pSunDesc, SIZE radius)
{
	SIZE index = 0;

	if (radius <= (MAX_ZOOM_RADIUS >> 1))
	{
		++index;
		if (radius <= (MAX_ZOOM_RADIUS >> 2))
			++index;
	}

	pSunDesc->image.origin.x = SIS_SCREEN_WIDTH >> 1;
	pSunDesc->image.origin.y = SIS_SCREEN_HEIGHT >> 1;
	pSunDesc->image.frame = SetRelFrameIndex (SunFrame, index);
}

static void
SetPlanetColorMap (PLANET_DESC *planet)
{
	COUNT index = planet->data_index & ~WORLD_TYPE_SPECIAL;
	assert (index < NUMBER_OF_PLANET_TYPES);
	SetColorMap (GetColorMapAddress (SetAbsColorMapIndex (OrbitalCMap,
			PLANCOLOR (PlanData[index].Type))));
}

static void
DrawInnerPlanets (PLANET_DESC *planet)
{
	STAMP s;
	COUNT i;
	PLANET_DESC *moon;

	// Draw the planet image
	SetPlanetColorMap (planet);
	s.origin.x = SIS_SCREEN_WIDTH >> 1;
	s.origin.y = SIS_SCREEN_HEIGHT >> 1;
	s.frame = planet->image.frame;

	i = planet->data_index & ~WORLD_TYPE_SPECIAL;
	if (i < NUMBER_OF_PLANET_TYPES
			&& (planet->data_index & PLANET_SHIELDED))
	{	// Shielded world looks "shielded" in inner view
		s.frame = SetAbsFrameIndex (SpaceJunkFrame, 17);
	}
	DrawStamp (&s);

	// Draw the moon images
	for (i = planet->NumPlanets, moon = pSolarSysState->MoonDesc;
			i; --i, ++moon)
	{
		if (!(moon->data_index & WORLD_TYPE_SPECIAL))
			SetPlanetColorMap (moon);
		DrawStamp (&moon->image);
	}
}

static void
DrawSystem (SIZE radius, BOOLEAN IsInnerSystem)
{
	BYTE i;
	PLANET_DESC *pCurDesc;
	PLANET_DESC *pBaseDesc;
	CONTEXT oldContext;
	STAMP s;

	if (!SolarSysFrame)
	{	// Create the saved view graphic
		RECT clipRect;

		GetContextClipRect (&clipRect);
		SolarSysFrame = CaptureDrawable (CreateDrawable (WANT_PIXMAP,
				clipRect.extent.width, clipRect.extent.height, 1));
	}

	oldContext = SetContext (OffScreenContext);
	SetContextFGFrame (SolarSysFrame);
	SetContextClipRect (NULL);

	DrawStarBackGround ();

	pBaseDesc = pSolarSysState->pBaseDesc;
	if (IsInnerSystem)
	{	// Draw the inner system view *planet's* orbit segment
		pCurDesc = pSolarSysState->pOrbitalDesc;
		DrawOrbit (pCurDesc, DISPLAY_FACTOR * 4, DISPLAY_FACTOR, radius);
	}

	// Draw the planet orbits or moon orbits
	for (i = pBaseDesc->pPrevDesc->NumPlanets, pCurDesc = pBaseDesc;
			i; --i, ++pCurDesc)
	{
		if (IsInnerSystem)
			DrawOrbit (pCurDesc, 2, 1, 2);
		else
			DrawOrbit (pCurDesc, DISPLAY_FACTOR, DISPLAY_FACTOR / 4,
					radius);
	}

	if (IsInnerSystem)
	{	// Draw the inner system view
		DrawInnerPlanets (pSolarSysState->pOrbitalDesc);
	}
	else
	{	// Draw the outer system view
		SIZE index;

		CalcSunSize (&pSolarSysState->SunDesc[0], radius);

		index = pSolarSysState->FirstPlanetIndex;
		for (;;)
		{
			pCurDesc = &pSolarSysState->PlanetDesc[index];
			if (pCurDesc == &pSolarSysState->SunDesc[0])
			{	// It's a sun
				SetColorMap (GetColorMapAddress (SetAbsColorMapIndex (
						SunCMap, STAR_COLOR (CurStarDescPtr->Type))));
			}
			else
			{	// It's a planet
				SetPlanetColorMap (pCurDesc);
			}
			DrawStamp (&pCurDesc->image);

			if (index == pSolarSysState->LastPlanetIndex)
				break;
			index = pCurDesc->NextIndex;
		}
	}

	SetContext (oldContext);

	// Draw the now-saved view graphic
	s.origin.x = 0;
	s.origin.y = 0;
	s.frame = SolarSysFrame;
	DrawStamp (&s);
}

void
DrawStarBackGround (void)
{
	STAMP s;
	
	s.origin.x = 0;
	s.origin.y = 0;
	s.frame = StarsFrame;
	DrawStamp (&s);
}

static FRAME
CreateStarBackGround (void)
{
	COUNT i, j;
	DWORD rand_val;
	STAMP s;
	DWORD old_seed;
	CONTEXT oldContext;
	RECT clipRect;
	FRAME frame;

	oldContext = SetContext (SpaceContext);
	GetContextClipRect (&clipRect);

	// Prepare a pre-drawn stars frame for this system
	frame = CaptureDrawable (CreateDrawable (WANT_PIXMAP,
			clipRect.extent.width, clipRect.extent.height, 1));
	SetContext (OffScreenContext);
	SetContextFGFrame (frame);
	SetContextClipRect (NULL);
	SetContextBackGroundColor (BLACK_COLOR);

	ClearDrawable ();

	old_seed = seedRandomForSolarSys ();

#define NUM_DIM_PIECES 8
	s.frame = SpaceJunkFrame;
	for (i = 0; i < NUM_DIM_PIECES; ++i)
	{
#define NUM_DIM_DRAWN 5
		for (j = 0; j < NUM_DIM_DRAWN; ++j)
		{
			rand_val = TFB_Random ();
			s.origin.x = LOWORD (rand_val) % SIS_SCREEN_WIDTH;
			s.origin.y = HIWORD (rand_val) % SIS_SCREEN_HEIGHT;

			DrawStamp (&s);
		}
		s.frame = IncFrameIndex (s.frame);
	}
#define NUM_BRT_PIECES 8
	for (i = 0; i < NUM_BRT_PIECES; ++i)
	{
#define NUM_BRT_DRAWN 30
		for (j = 0; j < NUM_BRT_DRAWN; ++j)
		{
			rand_val = TFB_Random ();
			s.origin.x = LOWORD (rand_val) % SIS_SCREEN_WIDTH;
			s.origin.y = HIWORD (rand_val) % SIS_SCREEN_HEIGHT;

			DrawStamp (&s);
		}
		s.frame = IncFrameIndex (s.frame);
	}

	TFB_SeedRandom (old_seed);

	SetContext (oldContext);

	return frame;
}

void
XFormIPLoc (POINT *pIn, POINT *pOut, BOOLEAN ToDisplay)
{
	if (ToDisplay)
		*pOut = locationToDisplay (*pIn, pSolarSysState->SunDesc[0].radius);
	else
		*pOut = displayToLocation (*pIn, pSolarSysState->SunDesc[0].radius);
}

void
ExploreSolarSys (void)
{
	SOLARSYS_STATE SolarSysState;
	
	if (CurStarDescPtr == 0)
	{
		POINT universe;

		universe.x = LOGX_TO_UNIVERSE (GLOBAL_SIS (log_x));
		universe.y = LOGY_TO_UNIVERSE (GLOBAL_SIS (log_y));
		CurStarDescPtr = FindStar (0, &universe, 1, 1);
		if (!CurStarDescPtr)
		{
			log_add (log_Fatal, "ExploreSolarSys(): do not know where you are!");
			explode ();
		}
	}
	GLOBAL_SIS (log_x) = UNIVERSE_TO_LOGX (CurStarDescPtr->star_pt.x);
	GLOBAL_SIS (log_y) = UNIVERSE_TO_LOGY (CurStarDescPtr->star_pt.y);

	pSolarSysState = &SolarSysState;

	memset (pSolarSysState, 0, sizeof (*pSolarSysState));

	SolarSysState.genFuncs = getGenerateFunctions (CurStarDescPtr->Index);

	InitSolarSys ();
	SetMenuSounds (MENU_SOUND_NONE, MENU_SOUND_NONE);
	SolarSysState.InputFunc = DoIpFlight;
	DoInput (&SolarSysState, FALSE);
	UninitSolarSys ();
	pSolarSysState = 0;
}

UNICODE *
GetNamedPlanetaryBody (void)
{
	if (!CurStarDescPtr || !playerInSolarSystem () || !playerInInnerSystem ())
		return NULL; // Not inside an inner system, so no name

	assert (pSolarSysState->pOrbitalDesc != NULL);

	if (CurStarDescPtr->Index == SOL_DEFINED)
	{	// Planets and moons in Sol
		int planet;
		int moon;

		planet = planetIndex (pSolarSysState, pSolarSysState->pOrbitalDesc);

		if (worldIsPlanet (pSolarSysState, pSolarSysState->pOrbitalDesc))
		{	// A planet
			return GAME_STRING (PLANET_NUMBER_BASE + planet);
		}

		// Moons
		moon = moonIndex (pSolarSysState, pSolarSysState->pOrbitalDesc);
		switch (planet)
		{
			case 2: // Earth
				switch (moon)
				{
					case 0: // Starbase
						return GAME_STRING (STARBASE_STRING_BASE + 0);
					case 1: // Luna
						return GAME_STRING (PLANET_NUMBER_BASE + 9);
				}
				break;
			case 4: // Jupiter
				switch (moon)
				{
					case 0: // Io
						return GAME_STRING (PLANET_NUMBER_BASE + 10);
					case 1: // Europa
						return GAME_STRING (PLANET_NUMBER_BASE + 11);
					case 2: // Ganymede
						return GAME_STRING (PLANET_NUMBER_BASE + 12);
					case 3: // Callisto
						return GAME_STRING (PLANET_NUMBER_BASE + 13);
				}
				break;
			case 5: // Saturn
				if (moon == 0) // Titan
					return GAME_STRING (PLANET_NUMBER_BASE + 14);
				break;
			case 7: // Neptune
				if (moon == 0) // Triton
					return GAME_STRING (PLANET_NUMBER_BASE + 15);
				break;
		}
	}
	else if (CurStarDescPtr->Index == SPATHI_DEFINED)
	{
		if (matchWorld (pSolarSysState, pSolarSysState->pOrbitalDesc,
				0, MATCH_PLANET))
		{
#ifdef NOTYET
			return "Spathiwa";
#endif // NOTYET
		}
	}
	else if (CurStarDescPtr->Index == SAMATRA_DEFINED)
	{
		if (matchWorld (pSolarSysState, pSolarSysState->pOrbitalDesc, 4, 0))
		{	// Sa-Matra
			return GAME_STRING (PLANET_NUMBER_BASE + 32);
		}
	}

	return NULL;
}

void
GetPlanetOrMoonName (UNICODE *buf, COUNT bufsize)
{
	UNICODE *named;
	int moon;
	int i;

	named = GetNamedPlanetaryBody ();
	if (named)
	{
		utf8StringCopy (buf, bufsize, named);
		return;
	}
		
	// Either not named or we already have a name
	utf8StringCopy (buf, bufsize, GLOBAL_SIS (PlanetName));

	if (!playerInSolarSystem () || !playerInInnerSystem () ||
			worldIsPlanet (pSolarSysState, pSolarSysState->pOrbitalDesc))
	{	// Outer or inner system or orbiting a planet
		return;
	}

	// Orbiting an unnamed moon
	i = strlen (buf);
	buf += i;
	bufsize -= i;
	moon = moonIndex (pSolarSysState, pSolarSysState->pOrbitalDesc);
	if (bufsize >= 3)
	{
		snprintf (buf, bufsize, "-%c", 'A' + moon);
		buf[bufsize - 1] = '\0';
	}
}

void
SaveSolarSysLocation (void)
{
	assert (playerInSolarSystem ());

	// This is a two-stage saving procedure
	// Stage 1: called when saving from inner/outer view
	// Stage 2: called when saving from orbital

	if (!playerInPlanetOrbit ())
	{
		saveNonOrbitalLocation ();
	}
	else
	{	// In orbit around a planet.
		BYTE moon;

		// Update the starinfo.dat file if necessary.
		if (GET_GAME_STATE (PLANETARY_CHANGE))
		{
			PutPlanetInfo ();
			SET_GAME_STATE (PLANETARY_CHANGE, 0);
		}

		// GLOBAL (ip_planet) is already set
		assert (GLOBAL (ip_planet) != 0);

		// has to be at least 1 because code tests for in_orbit!=0
		moon = 1; /* the planet itself */
		if (worldIsMoon (pSolarSysState, pSolarSysState->pOrbitalDesc))
		{
			moon += moonIndex (pSolarSysState, pSolarSysState->pOrbitalDesc);
			// +1 because moons have to be 1-based
			moon += 1;
		}
		GLOBAL (in_orbit) = moon;
	}
}

static BOOLEAN
DoSolarSysMenu (MENU_STATE *pMS)
{
	BOOLEAN select = PulsedInputState.menu[KEY_MENU_SELECT];
	BOOLEAN handled;

	if ((GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD))
			|| GLOBAL_SIS (CrewEnlisted) == (COUNT)~0)
		return FALSE;

	handled = DoMenuChooser (pMS, PM_STARMAP);
	if (handled)
		return TRUE;

	if (LastActivity == CHECK_LOAD)
		select = TRUE; // Selected LOAD from main menu

	if (!select)
		return TRUE;

	LockMutex (GraphicsLock);
	SetFlashRect (NULL);
	UnlockMutex (GraphicsLock);

	switch (pMS->CurState)
	{
		case EQUIP_DEVICE:
			select = DevicesMenu ();
			if (GLOBAL (CurrentActivity) & START_ENCOUNTER)
			{	// Invoked Talking Pet or a Caster for Ilwrath
				// Going into conversation
				return FALSE;
			}
			break;
		case CARGO:
			CargoMenu ();
			break;
		case ROSTER:
			select = RosterMenu ();
			break;
		case GAME_MENU:
			if (!GameOptions ())
				return FALSE; // abort or load
			break;
		case STARMAP:
			StarMap ();
			if (GLOBAL (CurrentActivity) & CHECK_ABORT)
				return FALSE;

			TransitionSystemIn ();
			// Fall through !!!
		case NAVIGATION:
			return FALSE;
	}

	if (!(GLOBAL (CurrentActivity) & CHECK_ABORT))
	{
		if (select)
		{	// 3DO menu jumps to NAVIGATE after a successful submenu run
			if (optWhichMenu != OPT_PC)
				pMS->CurState = NAVIGATION;
			DrawMenuStateStrings (PM_STARMAP, pMS->CurState);
		}
		LockMutex (GraphicsLock);
		SetFlashRect (SFR_MENU_3DO);
		UnlockMutex (GraphicsLock);
	}

	return TRUE;
}

static void
SolarSysMenu (void)
{
	MENU_STATE MenuState;

	memset (&MenuState, 0, sizeof MenuState);

	if (LastActivity == CHECK_LOAD)
	{	// Selected LOAD from main menu
		MenuState.CurState = GAME_MENU;
	}
	else
	{
		DrawMenuStateStrings (PM_STARMAP, STARMAP);
		MenuState.CurState = STARMAP;
	}

	LockMutex (GraphicsLock);
	DrawStatusMessage (NULL);
	SetFlashRect (SFR_MENU_3DO);
	UnlockMutex (GraphicsLock);

	SetMenuSounds (MENU_SOUND_ARROWS, MENU_SOUND_SELECT);
	MenuState.InputFunc = DoSolarSysMenu;
	DoInput (&MenuState, TRUE);

	DrawMenuStateStrings (PM_STARMAP, -NAVIGATION);
}

static BOOLEAN
DoIpFlight (SOLARSYS_STATE *pSS)
{
	static TimeCount NextTime;
	BOOLEAN cancel = PulsedInputState.menu[KEY_MENU_CANCEL];

	if (pSS->InOrbit)
	{	// CheckShipLocation() or InitSolarSys() sent us to orbital
		EnterPlanetOrbit ();
		SetMenuSounds (MENU_SOUND_NONE, MENU_SOUND_NONE);
		pSS->InOrbit = FALSE;
	}
	else if (cancel || LastActivity == CHECK_LOAD)
	{
		SolarSysMenu ();
		SetMenuSounds (MENU_SOUND_NONE, MENU_SOUND_NONE);
	}
	else
	{
		assert (pSS->InIpFlight);
		IP_frame ();
		SleepThreadUntil (NextTime);
		NextTime = GetTimeCounter () + IP_FRAME_RATE;
	}

	return (!(GLOBAL (CurrentActivity)
			& (START_ENCOUNTER | END_INTERPLANETARY
			| CHECK_ABORT | CHECK_LOAD))
			&& GLOBAL_SIS (CrewEnlisted) != (COUNT)~0);
}

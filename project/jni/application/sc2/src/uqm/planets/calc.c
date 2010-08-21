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

/* ----------------------------- INCLUDES ---------------------------- */
#include "../encount.h"
#include "libs/mathlib.h"
#include "libs/log.h"
/* -------------------------------- DATA -------------------------------- */

/* -------------------------------- CODE -------------------------------- */

//#define DEBUG_PLANET_CALC

#define CalcMass(b) CalcFromBase (b, b)
#define CalcRadius(b) CalcFromBase (b, ((b) >> 1) + 1)

#define LOW_TEMP 0
#define MED_TEMP 500
#define HIGH_TEMP 1500
#define LOW_TEMP_BONUS 10
#define MED_TEMP_BONUS 25
#define HIGH_TEMP_BONUS 50
#define MAX_TECTONICS 255
#ifdef OLD
#define CalcTectonics(b,t) (CalcFromBase(b, 3 << 5) \
										+ (UWORD)((t) < LOW_TEMP ? 0 : \
										((t) < MED_TEMP ? LOW_TEMP_BONUS : \
										((t) < HIGH_TEMP ? MED_TEMP_BONUS : \
										HIGH_TEMP_BONUS))))
#else /* OLD */
#define CalcTectonics(b,t) CalcFromBase(b, 3 << 5)
#endif /* OLD */

enum
{
	RED_SUN_INTENSITY = 0,
	ORANGE_SUN_INTENSITY,
	YELLOW_SUN_INTENSITY,
	GREEN_SUN_INTENSITY,
	BLUE_SUN_INTENSITY,
	WHITE_SUN_INTENSITY
};

static void
CalcSysInfo (SYSTEM_INFO *SysInfoPtr)
{
	SysInfoPtr->StarSize = pSolarSysState->SunDesc[0].data_index;
	switch (STAR_COLOR (CurStarDescPtr->Type))
	{
		case BLUE_BODY:
			SysInfoPtr->StarIntensity = BLUE_SUN_INTENSITY;
			break;
		case GREEN_BODY:
			SysInfoPtr->StarIntensity = GREEN_SUN_INTENSITY;
			break;
		case ORANGE_BODY:
			SysInfoPtr->StarIntensity = ORANGE_SUN_INTENSITY;
			break;
		case RED_BODY:
			SysInfoPtr->StarIntensity = RED_SUN_INTENSITY;
			break;
		case WHITE_BODY:
			SysInfoPtr->StarIntensity = WHITE_SUN_INTENSITY;
			break;
		case YELLOW_BODY:
			SysInfoPtr->StarIntensity = YELLOW_SUN_INTENSITY;
			break;
	}
	
	switch (STAR_TYPE (CurStarDescPtr->Type))
	{
		case DWARF_STAR:
			SysInfoPtr->StarEnergy =
					(SysInfoPtr->StarIntensity + 1) * DWARF_ENERGY;
			break;
		case GIANT_STAR:
			SysInfoPtr->StarEnergy =
					(SysInfoPtr->StarIntensity + 1) * GIANT_ENERGY;
			break;
		case SUPER_GIANT_STAR:
			SysInfoPtr->StarEnergy =
					(SysInfoPtr->StarIntensity + 1) * SUPERGIANT_ENERGY;
			break;
	}
}

static UWORD
GeneratePlanetComposition (PLANET_INFO *PlanetInfoPtr, SIZE SurfaceTemp,
		SIZE radius)
{
	if (PLANSIZE (PlanetInfoPtr->PlanDataPtr->Type) == GAS_GIANT)
	{
		PlanetInfoPtr->Weather = 7 << 5;
		return (GAS_GIANT_ATMOSPHERE);
	}
	else
	{
		BYTE range;
		UWORD atmo;

		PlanetInfoPtr->Weather = 0;
		atmo = 0;
		if ((range = HINIBBLE (PlanetInfoPtr->PlanDataPtr->AtmoAndDensity)) <= HEAVY)
		{
			if (SurfaceTemp < COLD_THRESHOLD)
				--range;
			else if (SurfaceTemp > HOT_THRESHOLD)
				++range;

			if (range <= HEAVY + 1)
			{
				switch (range)
				{
					case LIGHT:
						atmo = THIN_ATMOSPHERE;
						PlanetInfoPtr->Weather = 1 << 5;
						break;
					case MEDIUM:
						atmo = NORMAL_ATMOSPHERE;
						PlanetInfoPtr->Weather = 2 << 5;
						break;
					case HEAVY:
						atmo = THICK_ATMOSPHERE;
						PlanetInfoPtr->Weather = 4 << 5;
						break;
					default:
						atmo = SUPER_THICK_ATMOSPHERE;
						PlanetInfoPtr->Weather = 6 << 5;
						break;
				}

				radius /= EARTH_RADIUS;
				if (radius < 2)
					PlanetInfoPtr->Weather += 1 << 5;
				else if (radius > 10)
					PlanetInfoPtr->Weather -= 1 << 5;
				atmo = CalcFromBase (atmo, (atmo >> 1) + 1);
			}
		}

		return (atmo);
	}
}

// This function is called both when the solar system is generated,
// and when planetary orbit is entered.
// In the former case, the if() block will not be executed,
// which means that the temperature calculated in that case will be
// slightly lower. The result is that the orbits may not always
// have the colour you'd expect based on the true temperature.
// (eg. Beta Corvi I). I don't know what the idea behind this is,
// but the if statement must be there for a reason. -- SvdB
static SIZE
CalcTemp (SYSTEM_INFO *SysInfoPtr, SIZE radius)
{
#define GENERIC_ALBEDO 33 /* In %, 0=black, 100 is reflective */
#define ADJUST_FOR_KELVIN 273
#define PLANET_TEMP_CONSTANT 277L
	DWORD alb;
	SIZE centigrade, bonus;

	alb = 100 - GENERIC_ALBEDO;
	alb = square_root (square_root (alb * 100 * 10000))
			* PLANET_TEMP_CONSTANT * SysInfoPtr->StarEnergy
			/ ((YELLOW_SUN_INTENSITY + 1) * DWARF_ENERGY);

	centigrade = (SIZE)(alb / square_root (radius * 10000L / EARTH_RADIUS))
			- ADJUST_FOR_KELVIN;

	bonus = 0;
	if (SysInfoPtr == &pSolarSysState->SysInfo
			&& HINIBBLE (SysInfoPtr->PlanetInfo.PlanDataPtr->AtmoAndDensity) <= HEAVY)
	{
#define COLD_BONUS 20
#define HOT_BONUS 200
		if (centigrade >= HOT_THRESHOLD)
			bonus = HOT_BONUS;
		else if (centigrade >= COLD_THRESHOLD)
			bonus = COLD_BONUS;

		bonus <<= HINIBBLE (SysInfoPtr->PlanetInfo.PlanDataPtr->AtmoAndDensity);
		bonus = CalcFromBase (bonus, (bonus >> 1) + 1);
	}

	return (centigrade + bonus);
}

static COUNT
CalcRotation (PLANET_INFO *PlanetInfoPtr)
{
	if (PLANSIZE (PlanetInfoPtr->PlanDataPtr->Type) == GAS_GIANT)
		return ((COUNT)CalcFromBase (80, 80));
	else if (((BYTE)TFB_Random () % 10) == 0)
		return ((COUNT)CalcFromBase ((UWORD)50 * 240, (UWORD)200 * 240));
	else
		return ((COUNT)CalcFromBase (150, 150));
}

static SIZE
CalcTilt (void)
{ /* Calculate Axial Tilt */
	SIZE tilt;
	BYTE  i;

#define NUM_TOSSES 10
#define TILT_RANGE 180
	tilt = -(TILT_RANGE / 2);
	i = NUM_TOSSES;
	do /* Using added Randomom values to give bell curve */
	{
		tilt += (UWORD)TFB_Random () % ((TILT_RANGE / NUM_TOSSES) + 1);
	} while (--i);

	return (tilt);
}

// NB. Returns the RNG to the state it found it in.
DWORD
DoPlanetaryAnalysis (SYSTEM_INFO *SysInfoPtr, PLANET_DESC *pPlanetDesc)
{
	DWORD old_seed;

	if (pPlanetDesc->data_index == HIERARCHY_STARBASE)
		return (0);

	old_seed = TFB_SeedRandom (pPlanetDesc->rand_seed);

	CalcSysInfo (SysInfoPtr);

#ifdef DEBUG_PLANET_CALC
	{
		BYTE ColorClass[6][8] = {
				"Red",
				"Orange",
				"Yellow",
				"Green",
				"Blue",
				"White",
				};
		BYTE SizeName[3][12] = {
				"Dwarf",
				"Giant",
				"Supergiant",
				};

		log_add (log_Debug, "%s %s",
			ColorClass[SysInfoPtr->StarIntensity],
			SizeName[SysInfoPtr->StarSize]);
		log_add (log_Debug, "Stellar Energy: %d (sol = 3)",
				SysInfoPtr->StarEnergy);
	}
#endif /* DEBUG_PLANET_CALC */

	{
		SIZE radius;

		SysInfoPtr->PlanetInfo.PlanDataPtr =
				&PlanData[pPlanetDesc->data_index & ~PLANET_SHIELDED];

		if (pPlanetDesc->pPrevDesc == pSolarSysState->SunDesc)
			radius = pPlanetDesc->radius;
		else
			radius = pPlanetDesc->pPrevDesc->radius;
		SysInfoPtr->PlanetInfo.PlanetToSunDist = radius;

		SysInfoPtr->PlanetInfo.SurfaceTemperature =
				CalcTemp (SysInfoPtr, radius);
		switch (LONIBBLE (SysInfoPtr->PlanetInfo.PlanDataPtr->AtmoAndDensity))
		{
			case GAS_DENSITY:
				SysInfoPtr->PlanetInfo.PlanetDensity = 20;
				break;
			case LIGHT_DENSITY:
				SysInfoPtr->PlanetInfo.PlanetDensity = 33;
				break;
			case LOW_DENSITY:
				SysInfoPtr->PlanetInfo.PlanetDensity = 60;
				break;
			case NORMAL_DENSITY:
				SysInfoPtr->PlanetInfo.PlanetDensity = 100;
				break;
			case HIGH_DENSITY:
				SysInfoPtr->PlanetInfo.PlanetDensity = 150;
				break;
			case SUPER_DENSITY:
				SysInfoPtr->PlanetInfo.PlanetDensity = 200;
				break;
		}
		SysInfoPtr->PlanetInfo.PlanetDensity +=
				(SysInfoPtr->PlanetInfo.PlanetDensity / 20)
				- ((COUNT)TFB_Random ()
				% (SysInfoPtr->PlanetInfo.PlanetDensity / 10));

		switch (PLANSIZE (SysInfoPtr->PlanetInfo.PlanDataPtr->Type))
		{
			case SMALL_ROCKY_WORLD:
#define SMALL_RADIUS 25
				SysInfoPtr->PlanetInfo.PlanetRadius = CalcRadius (SMALL_RADIUS);
				break;
			case LARGE_ROCKY_WORLD:
#define LARGE_RADIUS 75
				SysInfoPtr->PlanetInfo.PlanetRadius = CalcRadius (LARGE_RADIUS);
				break;
			case GAS_GIANT:
#define MIN_GAS_RADIUS 300
#define MAX_GAS_RADIUS 1500
				SysInfoPtr->PlanetInfo.PlanetRadius =
						CalcFromBase (MIN_GAS_RADIUS, MAX_GAS_RADIUS - MIN_GAS_RADIUS);
				break;
		}

		SysInfoPtr->PlanetInfo.RotationPeriod = CalcRotation (&SysInfoPtr->PlanetInfo);
		SysInfoPtr->PlanetInfo.SurfaceGravity =
				CalcGravity (SysInfoPtr->PlanetInfo.PlanetDensity,
				SysInfoPtr->PlanetInfo.PlanetRadius);
		SysInfoPtr->PlanetInfo.AxialTilt = CalcTilt ();
		if ((SysInfoPtr->PlanetInfo.Tectonics =
				CalcTectonics (SysInfoPtr->PlanetInfo.PlanDataPtr->BaseTectonics,
				SysInfoPtr->PlanetInfo.SurfaceTemperature)) > MAX_TECTONICS)
			SysInfoPtr->PlanetInfo.Tectonics = MAX_TECTONICS;

		SysInfoPtr->PlanetInfo.AtmoDensity =
				GeneratePlanetComposition (&SysInfoPtr->PlanetInfo,
				SysInfoPtr->PlanetInfo.SurfaceTemperature, radius);

		SysInfoPtr->PlanetInfo.Tectonics >>= 5;
		SysInfoPtr->PlanetInfo.Weather >>= 5;

#ifdef DEBUG_PLANET_CALC
		radius = (SIZE)((DWORD)UNSCALE_RADIUS (radius) * 100 / UNSCALE_RADIUS (EARTH_RADIUS));
		log_add (log_Debug, "\tOrbital Distance   : %d.%02d AU", radius / 100, radius % 100);
		//log_add (log_Debug, "\tPlanetary Mass : %d.%02d Earth masses",
		// SysInfoPtr->PlanetInfo.PlanetMass / 100,
		// SysInfoPtr->PlanetInfo.PlanetMass % 100);
		log_add (log_Debug, "\tPlanetary Radius   : %d.%02d Earth radii",
				SysInfoPtr->PlanetInfo.PlanetRadius / 100,
				SysInfoPtr->PlanetInfo.PlanetRadius % 100);
		log_add (log_Debug, "\tSurface Gravity: %d.%02d gravities",
				SysInfoPtr->PlanetInfo.SurfaceGravity / 100,
				SysInfoPtr->PlanetInfo.SurfaceGravity % 100);
		log_add (log_Debug, "\tSurface Temperature: %d degrees C",
				SysInfoPtr->PlanetInfo.SurfaceTemperature );
		log_add (log_Debug, "\tAxial Tilt : %d degrees",
				abs (SysInfoPtr->PlanetInfo.AxialTilt));
		log_add (log_Debug, "\tTectonics : Class %u",
				SysInfoPtr->PlanetInfo.Tectonics + 1);
		log_add (log_Debug, "\tAtmospheric Density: %u.%02u",
				SysInfoPtr->PlanetInfo.AtmoDensity / EARTH_ATMOSPHERE,
				(SysInfoPtr->PlanetInfo.AtmoDensity * 100 / EARTH_ATMOSPHERE) % 100);
		if (SysInfoPtr->PlanetInfo.AtmoDensity == 0)
		{
			log_add (log_Debug, "\tAtmosphere: (Vacuum)");
		}
		else if (SysInfoPtr->PlanetInfo.AtmoDensity < THIN_ATMOSPHERE)
		{
			log_add (log_Debug, "\tAtmosphere: (Thin)");
		}
		else if (SysInfoPtr->PlanetInfo.AtmoDensity < NORMAL_ATMOSPHERE)
		{
			log_add (log_Debug, "\tAtmosphere: (Normal)");
		}
		else if (SysInfoPtr->PlanetInfo.AtmoDensity < THICK_ATMOSPHERE)
		{
			log_add (log_Debug, "\tAtmosphere: (Thick)");
		}
		else if (SysInfoPtr->PlanetInfo.AtmoDensity < SUPER_THICK_ATMOSPHERE)
		{
			log_add (log_Debug, "\tAtmosphere: (Super thick)");
		}
		else
		{
			log_add (log_Debug, "\tAtmosphere: (Gas Giant)");
		}

		log_add (log_Debug, "\tWeather   : Class %u",
				SysInfoPtr->PlanetInfo.Weather + 1);

		if (SysInfoPtr->PlanetInfo.RotationPeriod >= 480)
		{
			log_add (log_Debug, "\tLength of day  : %d.%d Earth days",
					SysInfoPtr->PlanetInfo.RotationPeriod / 240,
					SysInfoPtr->PlanetInfo.RotationPeriod % 240);
		}
		else
		{
			log_add (log_Debug, "\tLength of day  : %d.%d Earth hours",
					SysInfoPtr->PlanetInfo.RotationPeriod / 10,
					SysInfoPtr->PlanetInfo.RotationPeriod % 10);
		}
#endif /* DEBUG_PLANET_CALC */
	}

	return (TFB_SeedRandom (old_seed));
}


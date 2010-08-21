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

#include "../commall.h"
#include "resinst.h"
#include "strings.h"

#include "uqm/gameev.h"
#include "uqm/setup.h"
		// for GraphicsLock
#include "uqm/shipcont.h"
#include "libs/inplib.h"
#include "libs/mathlib.h"

#include "uqm/hyper.h"
			// for SOL_X/SOL_Y
#include "uqm/planets/planets.h"
		// for xxx_DISASTER
#include "uqm/sis.h"


#define NUM_HISTORY_ITEMS 9
#define NUM_EVENT_ITEMS 8
#define NUM_ALIEN_RACE_ITEMS 16
#define NUM_TECH_ITEMS 13

static NUMBER_SPEECH_DESC melnorme_numbers_english;

static LOCDATA melnorme_desc =
{
	NULL, /* init_encounter_func */
	NULL, /* post_encounter_func */
	NULL, /* uninit_encounter_func */
	MELNORME_PMAP_ANIM, /* AlienFrame */
	MELNORME_FONT, /* AlienFont */
	WHITE_COLOR_INIT, /* AlienTextFColor */
	BLACK_COLOR_INIT, /* AlienTextBColor */
	{0, 0}, /* AlienTextBaseline */
	0, /* SIS_TEXT_WIDTH - 16, */ /* AlienTextWidth */
	ALIGN_CENTER, /* AlienTextAlign */
	VALIGN_TOP, /* AlienTextValign */
	MELNORME_COLOR_MAP, /* AlienColorMap */
	MELNORME_MUSIC, /* AlienSong */
	NULL_RESOURCE, /* AlienAltSong */
	0, /* AlienSongFlags */
	MELNORME_CONVERSATION_PHRASES, /* PlayerPhrases */
	4, /* NumAnimations */
	{ /* AlienAmbientArray (ambient animations) */
		{
			6, /* StartIndex */
			5, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			ONE_SECOND / 12, 0, /* FrameRate */
			ONE_SECOND * 4, ONE_SECOND * 4,/* RestartRate */
			(1 << 1), /* BlockMask */
		},
		{
			11, /* StartIndex */
			9, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			ONE_SECOND / 20, 0, /* FrameRate */
			ONE_SECOND * 4, ONE_SECOND * 4,/* RestartRate */
			(1 << 0), /* BlockMask */
		},
		{
			20, /* StartIndex */
			2, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			ONE_SECOND / 10, ONE_SECOND / 15, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
			0, /* BlockMask */
		},
		{
			22, /* StartIndex */
			2, /* NumFrames */
			YOYO_ANIM, /* AnimFlags */
			ONE_SECOND / 10, ONE_SECOND / 15, /* FrameRate */
			ONE_SECOND, ONE_SECOND * 3, /* RestartRate */
			0, /* BlockMask */
		},
	},
	{ /* AlienTransitionDesc */
		0, /* StartIndex */
		0, /* NumFrames */
		0, /* AnimFlags */
		0, 0, /* FrameRate */
		0, 0, /* RestartRate */
		0, /* BlockMask */
	},
	{ /* AlienTalkDesc */
		1, /* StartIndex */
		5, /* NumFrames */
		0, /* AnimFlags */
		ONE_SECOND / 15, 0, /* FrameRate */
		ONE_SECOND / 12, 0, /* RestartRate */
		0, /* BlockMask */
	},
	&melnorme_numbers_english, /* AlienNumberSpeech - default */
	/* Filler for loaded resources */
	NULL, NULL, NULL,
	NULL,
	NULL,
};

static COUNT melnorme_digit_names[] =
{
	ENUMERATE_ZERO,
	ENUMERATE_ONE,
	ENUMERATE_TWO,
	ENUMERATE_THREE,
	ENUMERATE_FOUR,
	ENUMERATE_FIVE,
	ENUMERATE_SIX,
	ENUMERATE_SEVEN,
	ENUMERATE_EIGHT,
	ENUMERATE_NINE
};

static COUNT melnorme_teen_names[] =
{
	ENUMERATE_TEN,
	ENUMERATE_ELEVEN,
	ENUMERATE_TWELVE,
	ENUMERATE_THIRTEEN,
	ENUMERATE_FOURTEEN,
	ENUMERATE_FIFTEEN,
	ENUMERATE_SIXTEEN,
	ENUMERATE_SEVENTEEN,
	ENUMERATE_EIGHTEEN,
	ENUMERATE_NINETEEN
};

static COUNT melnorme_tens_names[] =
{
	0, /* invalid */
	0, /* skip digit */
	ENUMERATE_TWENTY,
	ENUMERATE_THIRTY,
	ENUMERATE_FOURTY,
	ENUMERATE_FIFTY,
	ENUMERATE_SIXTY,
	ENUMERATE_SEVENTY,
	ENUMERATE_EIGHTY,
	ENUMERATE_NINETY
};

static NUMBER_SPEECH_DESC melnorme_numbers_english =
{
	5, /* NumDigits */
	{
		{ /* 1000-999999 */
			1000, /* Divider */
			0, /* Subtrahend */
			NULL, /* StrDigits - recurse */
			NULL, /* Names - not used */
			ENUMERATE_THOUSAND /* CommonIndex */
		},
		{ /* 100-999 */
			100, /* Divider */
			0, /* Subtrahend */
			melnorme_digit_names, /* StrDigits */
			NULL, /* Names - not used */
			ENUMERATE_HUNDRED /* CommonIndex */
		},
		{ /* 20-99 */
			10, /* Divider */
			0, /* Subtrahend */
			melnorme_tens_names, /* StrDigits */
			NULL, /* Names - not used */
			0 /* CommonIndex - not used */
		},
		{ /* 10-19 */
			1, /* Divider */
			10, /* Subtrahend */
			melnorme_teen_names, /* StrDigits */
			NULL, /* Names - not used */
			0 /* CommonIndex - not used */
		},
		{ /* 0-9 */
			1, /* Divider */
			0, /* Subtrahend */
			melnorme_digit_names, /* StrDigits */
			NULL, /* Names - not used */
			0 /* CommonIndex - not used */
		}
	}
};

static StatMsgMode prevMsgMode;

static void DoFirstMeeting (RESPONSE_REF R);

static COUNT
ShipWorth (void)
{
	BYTE i;
	SBYTE crew_pods;
	COUNT worth;

	worth = GLOBAL_SIS (NumLanders)
			* GLOBAL (ModuleCost[PLANET_LANDER]);
	for (i = 0; i < NUM_DRIVE_SLOTS; ++i)
	{
		if (GLOBAL_SIS (DriveSlots[i]) < EMPTY_SLOT)
			worth += GLOBAL (ModuleCost[FUSION_THRUSTER]);
	}
	for (i = 0; i < NUM_JET_SLOTS; ++i)
	{
		if (GLOBAL_SIS (JetSlots[i]) < EMPTY_SLOT)
			worth += GLOBAL (ModuleCost[TURNING_JETS]);
	}

	crew_pods = -(SBYTE)(
			(GLOBAL_SIS (CrewEnlisted) + CREW_POD_CAPACITY - 1)
			/ CREW_POD_CAPACITY
			);
	for (i = 0; i < NUM_MODULE_SLOTS; ++i)
	{
		BYTE which_module;

		which_module = GLOBAL_SIS (ModuleSlots[i]);
		if (which_module < BOMB_MODULE_0
				&& (which_module != CREW_POD || ++crew_pods > 0))
		{
			worth += GLOBAL (ModuleCost[which_module]);
		}
	}

	return (worth);
}

static COUNT rescue_fuel;
static SIS_STATE SIS_copy;

static BOOLEAN
StripShip (COUNT fuel_required)
{
	BYTE i, which_module;
	SBYTE crew_pods;

	SET_GAME_STATE (MELNORME_RESCUE_REFUSED, 0);

	crew_pods = -(SBYTE)(
			(GLOBAL_SIS (CrewEnlisted) + CREW_POD_CAPACITY - 1)
			/ CREW_POD_CAPACITY
			);
	if (fuel_required == 0)
	{
		GlobData.SIS_state = SIS_copy;
		LockMutex (GraphicsLock);
		DeltaSISGauges (UNDEFINED_DELTA, rescue_fuel, UNDEFINED_DELTA);
		UnlockMutex (GraphicsLock);
	}
	else if (fuel_required == (COUNT)~0)
	{
		GLOBAL_SIS (NumLanders) = 0;
		for (i = 0; i < NUM_DRIVE_SLOTS; ++i)
			GLOBAL_SIS (DriveSlots[i]) = EMPTY_SLOT + 0;
		for (i = 0; i < NUM_JET_SLOTS; ++i)
			GLOBAL_SIS (JetSlots[i]) = EMPTY_SLOT + 1;
		if (GLOBAL_SIS (FuelOnBoard) > FUEL_RESERVE)
			GLOBAL_SIS (FuelOnBoard) = FUEL_RESERVE;
		GLOBAL_SIS (TotalBioMass) = 0;
		GLOBAL_SIS (TotalElementMass) = 0;
		for (i = 0; i < NUM_ELEMENT_CATEGORIES; ++i)
			GLOBAL_SIS (ElementAmounts[i]) = 0;
		for (i = 0; i < NUM_MODULE_SLOTS; ++i)
		{
			which_module = GLOBAL_SIS (ModuleSlots[i]);
			if (which_module < BOMB_MODULE_0
					&& (which_module != CREW_POD
					|| ++crew_pods > 0))
				GLOBAL_SIS (ModuleSlots[i]) = EMPTY_SLOT + 2;
		}

		LockMutex (GraphicsLock);
		DeltaSISGauges (UNDEFINED_DELTA, UNDEFINED_DELTA, UNDEFINED_DELTA);
		UnlockMutex (GraphicsLock);
	}
	else if (fuel_required)
	{
		SBYTE bays;
		BYTE num_searches, beg_mod, end_mod;
		COUNT worth, total;
		BYTE module_count[BOMB_MODULE_0];
		BYTE slot;
		DWORD capacity;

		SIS_copy = GlobData.SIS_state;
		for (i = PLANET_LANDER; i < BOMB_MODULE_0; ++i)
			module_count[i] = 0;

		capacity = FUEL_RESERVE;
		slot = NUM_MODULE_SLOTS - 1;
		do
		{
			if (SIS_copy.ModuleSlots[slot] == FUEL_TANK
					|| SIS_copy.ModuleSlots[slot] == HIGHEFF_FUELSYS)
			{
				COUNT volume;

				volume = SIS_copy.ModuleSlots[slot] == FUEL_TANK
						? FUEL_TANK_CAPACITY : HEFUEL_TANK_CAPACITY;
				capacity += volume;
			}
		} while (slot--);
		if (fuel_required > capacity)
			fuel_required = capacity;

		bays = -(SBYTE)(
				(SIS_copy.TotalElementMass + STORAGE_BAY_CAPACITY - 1)
				/ STORAGE_BAY_CAPACITY
				);
		for (i = 0; i < NUM_MODULE_SLOTS; ++i)
		{
			which_module = SIS_copy.ModuleSlots[i];
			if (which_module == CREW_POD)
				++crew_pods;
			else if (which_module == STORAGE_BAY)
				++bays;
		}

		worth = fuel_required / FUEL_TANK_SCALE;
		total = 0;
		num_searches = 0;
		beg_mod = end_mod = (BYTE)~0;
		while (total < worth && ShipWorth () && ++num_searches)
		{
			DWORD rand_val;

			rand_val = TFB_Random ();
			switch (which_module = LOBYTE (LOWORD (rand_val)) % (CREW_POD + 1))
			{
				case PLANET_LANDER:
					if (SIS_copy.NumLanders == 0)
						continue;
					--SIS_copy.NumLanders;
					break;
				case FUSION_THRUSTER:
					for (i = 0; i < NUM_DRIVE_SLOTS; ++i)
					{
						if (SIS_copy.DriveSlots[i] < EMPTY_SLOT)
							break;
					}
					if (i == NUM_DRIVE_SLOTS)
						continue;
					SIS_copy.DriveSlots[i] = EMPTY_SLOT + 0;
					break;
				case TURNING_JETS:
					for (i = 0; i < NUM_JET_SLOTS; ++i)
					{
						if (SIS_copy.JetSlots[i] < EMPTY_SLOT)
							break;
					}
					if (i == NUM_JET_SLOTS)
						continue;
					SIS_copy.JetSlots[i] = EMPTY_SLOT + 1;
					break;
				case CREW_POD:
					i = HIBYTE (LOWORD (rand_val)) % NUM_MODULE_SLOTS;
					which_module = SIS_copy.ModuleSlots[i];
					if (which_module >= BOMB_MODULE_0
							|| which_module == FUEL_TANK
							|| which_module == HIGHEFF_FUELSYS
							|| (which_module == STORAGE_BAY
							&& module_count[STORAGE_BAY] >= bays)
							|| (which_module == CREW_POD
							&& module_count[CREW_POD] >= crew_pods))
						continue;
					SIS_copy.ModuleSlots[i] = EMPTY_SLOT + 2;
					break;
			}

			if (beg_mod == (BYTE)~0)
				beg_mod = end_mod = which_module;
			else if (which_module > end_mod)
				end_mod = which_module;
			++module_count[which_module];
			total += GLOBAL (ModuleCost[which_module]);
		}

		if (total == 0)
		{
			NPCPhrase (CHARITY);
			LockMutex (GraphicsLock);
			DeltaSISGauges (0, fuel_required, 0);
			UnlockMutex (GraphicsLock);
			return (FALSE);
		}
		else
		{
			NPCPhrase (RESCUE_OFFER);
			rescue_fuel = fuel_required;
			if (rescue_fuel == capacity)
				NPCPhrase (RESCUE_TANKS);
			else
				NPCPhrase (RESCUE_HOME);
			for (i = PLANET_LANDER; i < BOMB_MODULE_0; ++i)
			{
				if (module_count[i])
				{
					RESPONSE_REF pStr = 0;

					switch (i)
					{
						case PLANET_LANDER:
							pStr = LANDERS;
							break;
						case FUSION_THRUSTER:
							pStr = THRUSTERS;
							break;
						case TURNING_JETS:
							pStr = JETS;
							break;
						case CREW_POD:
							pStr = PODS;
							break;
						case STORAGE_BAY:
							pStr = BAYS;
							break;
						case DYNAMO_UNIT:
							pStr = DYNAMOS;
							break;
						case SHIVA_FURNACE:
							pStr = FURNACES;
							break;
						case GUN_WEAPON:
							pStr = GUNS;
							break;
						case BLASTER_WEAPON:
							pStr = BLASTERS;
							break;
						case CANNON_WEAPON:
							pStr = CANNONS;
							break;
						case TRACKING_SYSTEM:
							pStr = TRACKERS;
							break;
						case ANTIMISSILE_DEFENSE:
							pStr = DEFENSES;
							break;
						default:
							assert (0 && "Unknown module");
					}

					if (i == end_mod && i != beg_mod)
						NPCPhrase (END_LIST_WITH_AND);
					NPCPhrase (ENUMERATE_ONE + (module_count[i] - 1));
					NPCPhrase (pStr);
				}
			}
		}
	}

	return (TRUE);
}

static void
ExitConversation (RESPONSE_REF R)
{
	if (PLAYER_SAID (R, no_trade_now))
		NPCPhrase (OK_NO_TRADE_NOW_BYE);
	else if (PLAYER_SAID (R, youre_on))
	{
		NPCPhrase (YOU_GIVE_US_NO_CHOICE);

		SET_GAME_STATE (MELNORME_ANGER, 1);
		SET_GAME_STATE (BATTLE_SEGUE, 1);
	}
	else if (PLAYER_SAID (R, so_we_can_attack))
	{
		NPCPhrase (DECEITFUL_HUMAN);

		SET_GAME_STATE (MELNORME_ANGER, 2);
		SET_GAME_STATE (BATTLE_SEGUE, 1);
	}
	else if (PLAYER_SAID (R, bye_melnorme_slightly_angry))
		NPCPhrase (MELNORME_SLIGHTLY_ANGRY_GOODBYE);
	else if (PLAYER_SAID (R, ok_strip_me))
	{
		if (ShipWorth () < 4000 / MODULE_COST_SCALE)
				/* is ship worth stripping */
			NPCPhrase (NOT_WORTH_STRIPPING);
		else
		{
			SET_GAME_STATE (MELNORME_ANGER, 0);

			StripShip ((COUNT)~0);
			NPCPhrase (FAIR_JUSTICE);
		}
	}
	else if (PLAYER_SAID (R, fight_some_more))
	{
		NPCPhrase (OK_FIGHT_SOME_MORE);

		SET_GAME_STATE (MELNORME_ANGER, 3);
		SET_GAME_STATE (BATTLE_SEGUE, 1);
	}
	else if (PLAYER_SAID (R, bye_melnorme_pissed_off))
		NPCPhrase (MELNORME_PISSED_OFF_GOODBYE);
	else if (PLAYER_SAID (R, well_if_thats_the_way_you_feel))
	{
		NPCPhrase (WE_FIGHT_AGAIN);

		SET_GAME_STATE (BATTLE_SEGUE, 1);
	}
	else if (PLAYER_SAID (R, you_hate_us_so_we_go_away))
		NPCPhrase (HATE_YOU_GOODBYE);
	else if (PLAYER_SAID (R, take_it))
	{
		StripShip (0);
		NPCPhrase (HAPPY_TO_HAVE_RESCUED);
	}
	else if (PLAYER_SAID (R, leave_it))
	{
		SET_GAME_STATE (MELNORME_RESCUE_REFUSED, 1);
		NPCPhrase (MAYBE_SEE_YOU_LATER);
	}
	else if (PLAYER_SAID (R, no_help))
	{
		SET_GAME_STATE (MELNORME_RESCUE_REFUSED, 1);
		NPCPhrase (GOODBYE_AND_GOODLUCK);
	}
	else if (PLAYER_SAID (R, no_changed_mind))
	{
		NPCPhrase (GOODBYE_AND_GOODLUCK_AGAIN);
	}
	else if (PLAYER_SAID (R, be_leaving_now)
			|| PLAYER_SAID (R, goodbye))
	{
		NPCPhrase (FRIENDLY_GOODBYE);
	}
}

static void
DoRescue (RESPONSE_REF R)
{
	SIZE dx, dy;
	COUNT fuel_required;

	(void) R;  // ignored
	dx = LOGX_TO_UNIVERSE (GLOBAL_SIS (log_x))
			- SOL_X;
	dy = LOGY_TO_UNIVERSE (GLOBAL_SIS (log_y))
			- SOL_Y;
	fuel_required = square_root (
			(DWORD)((long)dx * dx + (long)dy * dy)
			) + (2 * FUEL_TANK_SCALE);

	if (StripShip (fuel_required))
	{
		Response (take_it, ExitConversation);
		Response (leave_it, ExitConversation);
	}
}

static COUNT
DeltaCredit (SIZE delta_credit)
{
	COUNT Credit;

	Credit = MAKE_WORD (
			GET_GAME_STATE (MELNORME_CREDIT0),
			GET_GAME_STATE (MELNORME_CREDIT1)
			);
	if ((int)delta_credit >= 0 || ((int)(-delta_credit) <= (int)(Credit)))
	{
		Credit += delta_credit;
		SET_GAME_STATE (MELNORME_CREDIT0, LOBYTE (Credit));
		SET_GAME_STATE (MELNORME_CREDIT1, HIBYTE (Credit));
		LockMutex (GraphicsLock);
		DrawStatusMessage (NULL);
		UnlockMutex (GraphicsLock);
	}
	else
	{
		NPCPhrase (NEED_MORE_CREDIT0);
		NPCPhrase (delta_credit + (int)Credit);
		NPCPhrase (NEED_MORE_CREDIT1);
	}
	
	return (Credit);
}

static void
CurrentEvents (void)
{
	BYTE stack;

	stack = GET_GAME_STATE (MELNORME_EVENTS_INFO_STACK);
	switch (stack++)
	{
		case 0:
			NPCPhrase (OK_BUY_EVENT_1);
			break;
		case 1:
			NPCPhrase (OK_BUY_EVENT_2);
			break;
		case 2:
			NPCPhrase (OK_BUY_EVENT_3);
			break;
		case 3:
			NPCPhrase (OK_BUY_EVENT_4);
			break;
		case 4:
			NPCPhrase (OK_BUY_EVENT_5);
			break;
		case 5:
			NPCPhrase (OK_BUY_EVENT_6);
			break;
		case 6:
			NPCPhrase (OK_BUY_EVENT_7);
			break;
		case 7:
			NPCPhrase (OK_BUY_EVENT_8);
			break;
	}
	SET_GAME_STATE (MELNORME_EVENTS_INFO_STACK, stack);
}

static void
AlienRaces (void)
{
	BYTE stack;

	stack = GET_GAME_STATE (MELNORME_ALIEN_INFO_STACK);
	switch (stack++)
	{
		case 0:
			NPCPhrase (OK_BUY_ALIEN_RACE_1);
			break;
		case 1:
			NPCPhrase (OK_BUY_ALIEN_RACE_2);
			break;
		case 2:
			NPCPhrase (OK_BUY_ALIEN_RACE_3);
			break;
		case 3:
			NPCPhrase (OK_BUY_ALIEN_RACE_4);
			break;
		case 4:
			NPCPhrase (OK_BUY_ALIEN_RACE_5);
			break;
		case 5:
			NPCPhrase (OK_BUY_ALIEN_RACE_6);
			break;
		case 6:
			NPCPhrase (OK_BUY_ALIEN_RACE_7);
			break;
		case 7:
			NPCPhrase (OK_BUY_ALIEN_RACE_8);
			break;
		case 8:
			NPCPhrase (OK_BUY_ALIEN_RACE_9);
			break;
		case 9:
			NPCPhrase (OK_BUY_ALIEN_RACE_10);
			break;
		case 10:
			NPCPhrase (OK_BUY_ALIEN_RACE_11);
			break;
		case 11:
			NPCPhrase (OK_BUY_ALIEN_RACE_12);
			break;
		case 12:
			NPCPhrase (OK_BUY_ALIEN_RACE_13);
			break;
		case 13:
			NPCPhrase (OK_BUY_ALIEN_RACE_14);
			if (!GET_GAME_STATE (FOUND_PLUTO_SPATHI))
			{
				SET_GAME_STATE (KNOW_SPATHI_PASSWORD, 1);
				SET_GAME_STATE (SPATHI_HOME_VISITS, 7);
			}
			break;
		case 14:
			NPCPhrase (OK_BUY_ALIEN_RACE_15);
			if (GET_GAME_STATE (KNOW_ABOUT_SHATTERED) < 2)
			{
				SET_GAME_STATE (KNOW_ABOUT_SHATTERED, 2);
			}
			SET_GAME_STATE (KNOW_SYREEN_WORLD_SHATTERED, 1);
			break;
		case 15:
			NPCPhrase (OK_BUY_ALIEN_RACE_16);
			break;
	}
	SET_GAME_STATE (MELNORME_ALIEN_INFO_STACK, stack);
}

static void
History (void)
{
	BYTE stack;

	stack = GET_GAME_STATE (MELNORME_HISTORY_INFO_STACK);
	switch (stack++)
	{
		case 0:
			NPCPhrase (OK_BUY_HISTORY_1);
			break;
		case 1:
			NPCPhrase (OK_BUY_HISTORY_2);
			break;
		case 2:
			NPCPhrase (OK_BUY_HISTORY_3);
			break;
		case 3:
			NPCPhrase (OK_BUY_HISTORY_4);
			break;
		case 4:
			NPCPhrase (OK_BUY_HISTORY_5);
			break;
		case 5:
			NPCPhrase (OK_BUY_HISTORY_6);
			break;
		case 6:
			NPCPhrase (OK_BUY_HISTORY_7);
			break;
		case 7:
			NPCPhrase (OK_BUY_HISTORY_8);
			break;
		case 8:
			NPCPhrase (OK_BUY_HISTORY_9);
			break;
	}
	SET_GAME_STATE (MELNORME_HISTORY_INFO_STACK, stack);
}

static void NatureOfConversation (RESPONSE_REF R);

static BYTE AskedToBuy;

static void
DoBuy (RESPONSE_REF R)
{
	COUNT credit;
	SIZE needed_credit;
	BYTE slot;
	DWORD capacity;

	credit = MAKE_WORD (
			GET_GAME_STATE (MELNORME_CREDIT0),
			GET_GAME_STATE (MELNORME_CREDIT1)
			);

	capacity = FUEL_RESERVE;
	slot = NUM_MODULE_SLOTS - 1;
	do
	{
		if (GLOBAL_SIS (ModuleSlots[slot]) == FUEL_TANK
				|| GLOBAL_SIS (ModuleSlots[slot]) == HIGHEFF_FUELSYS)
		{
			COUNT volume;

			volume = GLOBAL_SIS (ModuleSlots[slot]) == FUEL_TANK
					? FUEL_TANK_CAPACITY : HEFUEL_TANK_CAPACITY;
			capacity += volume;
		}
	} while (slot--);

	if (credit == 0)
	{
		AskedToBuy = TRUE;
		NPCPhrase (NEED_CREDIT);

		NatureOfConversation (R);
	}
	else if (PLAYER_SAID (R, buy_fuel)
			|| PLAYER_SAID (R, buy_1_fuel)
			|| PLAYER_SAID (R, buy_5_fuel)
			|| PLAYER_SAID (R, buy_10_fuel)
			|| PLAYER_SAID (R, buy_25_fuel)
			|| PLAYER_SAID (R, fill_me_up))
	{
		needed_credit = 0;
		if (PLAYER_SAID (R, buy_1_fuel))
			needed_credit = 1;
		else if (PLAYER_SAID (R, buy_5_fuel))
			needed_credit = 5;
		else if (PLAYER_SAID (R, buy_10_fuel))
			needed_credit = 10;
		else if (PLAYER_SAID (R, buy_25_fuel))
			needed_credit = 25;
		else if (PLAYER_SAID (R, fill_me_up))
			needed_credit = (capacity - GLOBAL_SIS (FuelOnBoard)
					+ FUEL_TANK_SCALE - 1)
				/ FUEL_TANK_SCALE;

		if (needed_credit == 0)
		{
			if (!GET_GAME_STATE (MELNORME_FUEL_PROCEDURE))
			{
				NPCPhrase (BUY_FUEL_INTRO);
				SET_GAME_STATE (MELNORME_FUEL_PROCEDURE, 1);
			}
		}
		else
		{
			if (GLOBAL_SIS (FuelOnBoard) / FUEL_TANK_SCALE
					+ needed_credit > capacity / FUEL_TANK_SCALE)
			{
				NPCPhrase (NO_ROOM_FOR_FUEL);
				goto TryFuelAgain;
			}

			if ((int)(needed_credit * (BIO_CREDIT_VALUE / 2)) <= (int)credit)
			{
				DWORD f;

				NPCPhrase (GOT_FUEL);

				f = (DWORD)needed_credit * FUEL_TANK_SCALE;
				LockMutex (GraphicsLock);
				while (f > 0x3FFFL)
				{
					DeltaSISGauges (0, 0x3FFF, 0);
					f -= 0x3FFF;
				}
				DeltaSISGauges (0, (SIZE)f, 0);
				UnlockMutex (GraphicsLock);
			}
			needed_credit *= (BIO_CREDIT_VALUE / 2);
		}
		if (needed_credit)
		{
			DeltaCredit (-needed_credit);
			if (GLOBAL_SIS (FuelOnBoard) >= capacity)
				goto BuyBuyBuy;
		}
TryFuelAgain:
		NPCPhrase (HOW_MUCH_FUEL);

		Response (buy_1_fuel, DoBuy);
		Response (buy_5_fuel, DoBuy);
		Response (buy_10_fuel, DoBuy);
		Response (buy_25_fuel, DoBuy);
		Response (fill_me_up, DoBuy);
		Response (done_buying_fuel, DoBuy);
	}
	else if (PLAYER_SAID (R, buy_technology)
			|| PLAYER_SAID (R, buy_new_tech))
	{
		BYTE stack;

		needed_credit = 0;
		if (PLAYER_SAID (R, buy_technology))
		{
			if (!GET_GAME_STATE (MELNORME_TECH_PROCEDURE))
			{
				NPCPhrase (BUY_NEW_TECH_INTRO);
				SET_GAME_STATE (MELNORME_TECH_PROCEDURE, 1);
			}
			stack = 0;
		}
		else
		{
			RESPONSE_REF pStr = 0;

			stack = GET_GAME_STATE (MELNORME_TECH_STACK);
			switch (stack)
			{
				case 0:
					pStr = OK_BUY_NEW_TECH_1;
					needed_credit = 75 * BIO_CREDIT_VALUE;
					break;
				case 1:
					pStr = OK_BUY_NEW_TECH_2;
					needed_credit = 75 * BIO_CREDIT_VALUE;
					break;
				case 2:
					pStr = OK_BUY_NEW_TECH_3;
					needed_credit = 75 * BIO_CREDIT_VALUE;
					break;
				case 3:
					pStr = OK_BUY_NEW_TECH_4;
					needed_credit = 75 * BIO_CREDIT_VALUE;
					break;
				case 4:
					pStr = OK_BUY_NEW_TECH_5;
					needed_credit = 75 * BIO_CREDIT_VALUE;
					break;
				case 5:
					pStr = OK_BUY_NEW_TECH_6;
					needed_credit = 75 * BIO_CREDIT_VALUE;
					break;
				case 6:
					pStr = OK_BUY_NEW_TECH_7;
					needed_credit = 75 * BIO_CREDIT_VALUE;
					break;
				case 7:
					pStr = OK_BUY_NEW_TECH_8;
					needed_credit = 75 * BIO_CREDIT_VALUE;
					break;
				case 8:
					pStr = OK_BUY_NEW_TECH_9;
					needed_credit = 75 * BIO_CREDIT_VALUE;
					break;
				case 9:
					pStr = OK_BUY_NEW_TECH_10;
					needed_credit = 75 * BIO_CREDIT_VALUE;
					break;
				case 10:
					pStr = OK_BUY_NEW_TECH_11;
					needed_credit = 75 * BIO_CREDIT_VALUE;
					break;
				case 11:
					pStr = OK_BUY_NEW_TECH_12;
					needed_credit = 75 * BIO_CREDIT_VALUE;
					break;
				case 12:
					pStr = OK_BUY_NEW_TECH_13;
					needed_credit = 75 * BIO_CREDIT_VALUE;
					break;
				default:
					assert (0 && "Unknown tech");
			}
			if ((int)needed_credit > (int)credit)
			{
				DeltaCredit (-needed_credit);
				goto BuyBuyBuy;
			}
			else
			{
				++stack;
				NPCPhrase (pStr);
				DeltaCredit (-needed_credit);
			}
		}

		switch (stack)
		{
			case 0:
				if (GLOBAL (ModuleCost[BLASTER_WEAPON]) == 0)
				{
					NPCPhrase (NEW_TECH_1);
					break;
				}
				++stack;
			case 1:
				GLOBAL (ModuleCost[BLASTER_WEAPON]) =
						4000 / MODULE_COST_SCALE;
				if (!GET_GAME_STATE (IMPROVED_LANDER_SPEED))
				{
					NPCPhrase (NEW_TECH_2);
					break;
				}
				++stack;
			case 2:
				SET_GAME_STATE (IMPROVED_LANDER_SPEED, 1);
				if (GLOBAL (ModuleCost[ANTIMISSILE_DEFENSE]) == 0)
				{
					NPCPhrase (NEW_TECH_3);
					break;
				}
				++stack;
			case 3:
				GLOBAL (ModuleCost[ANTIMISSILE_DEFENSE]) =
						4000 / MODULE_COST_SCALE;
				if (!(GET_GAME_STATE (LANDER_SHIELDS)
						& (1 << BIOLOGICAL_DISASTER)))
				{
					NPCPhrase (NEW_TECH_4);
					break;
				}
				++stack;
			case 4:
				credit = GET_GAME_STATE (LANDER_SHIELDS)
						| (1 << BIOLOGICAL_DISASTER);
				SET_GAME_STATE (LANDER_SHIELDS, credit);
				if (!GET_GAME_STATE (IMPROVED_LANDER_CARGO))
				{
					NPCPhrase (NEW_TECH_5);
					break;
				}
				++stack;
			case 5:
				SET_GAME_STATE (IMPROVED_LANDER_CARGO, 1);
				if (GLOBAL (ModuleCost[HIGHEFF_FUELSYS]) == 0)
				{
					NPCPhrase (NEW_TECH_6);
					break;
				}
				++stack;
			case 6:
				GLOBAL (ModuleCost[HIGHEFF_FUELSYS]) =
						1000 / MODULE_COST_SCALE;
				if (!GET_GAME_STATE (IMPROVED_LANDER_SHOT))
				{
					NPCPhrase (NEW_TECH_7);
					break;
				}
				++stack;
			case 7:
				SET_GAME_STATE (IMPROVED_LANDER_SHOT, 1);
				if (!(GET_GAME_STATE (LANDER_SHIELDS)
						& (1 << EARTHQUAKE_DISASTER)))
				{
					NPCPhrase (NEW_TECH_8);
					break;
				}
				++stack;
			case 8:
				credit = GET_GAME_STATE (LANDER_SHIELDS)
						| (1 << EARTHQUAKE_DISASTER);
				SET_GAME_STATE (LANDER_SHIELDS, credit);
				if (GLOBAL (ModuleCost[TRACKING_SYSTEM]) == 0)
				{
					NPCPhrase (NEW_TECH_9);
					break;
				}
				++stack;
			case 9:
				GLOBAL (ModuleCost[TRACKING_SYSTEM]) =
						5000 / MODULE_COST_SCALE;
				if (!(GET_GAME_STATE (LANDER_SHIELDS)
						& (1 << LIGHTNING_DISASTER)))
				{
					NPCPhrase (NEW_TECH_10);
					break;
				}
				++stack;
			case 10:
				credit = GET_GAME_STATE (LANDER_SHIELDS)
						| (1 << LIGHTNING_DISASTER);
				SET_GAME_STATE (LANDER_SHIELDS, credit);
				if (!(GET_GAME_STATE (LANDER_SHIELDS)
						& (1 << LAVASPOT_DISASTER)))
				{
					NPCPhrase (NEW_TECH_11);
					break;
				}
				++stack;
			case 11:
				credit = GET_GAME_STATE (LANDER_SHIELDS)
						| (1 << LAVASPOT_DISASTER);
				SET_GAME_STATE (LANDER_SHIELDS, credit);
				if (GLOBAL (ModuleCost[CANNON_WEAPON]) == 0)
				{
					NPCPhrase (NEW_TECH_12);
					break;
				}
				++stack;
			case 12:
				GLOBAL (ModuleCost[CANNON_WEAPON]) =
						6000 / MODULE_COST_SCALE;
				if (GLOBAL (ModuleCost[SHIVA_FURNACE]) == 0)
				{
					NPCPhrase (NEW_TECH_13);
					break;
				}
				++stack;
			case 13:
				GLOBAL (ModuleCost[SHIVA_FURNACE]) =
						4000 / MODULE_COST_SCALE;
				NPCPhrase (NEW_TECH_ALL_GONE);
				SET_GAME_STATE (MELNORME_TECH_STACK, stack);
				goto BuyBuyBuy;
		}
		SET_GAME_STATE (MELNORME_TECH_STACK, stack);

		Response (buy_new_tech, DoBuy);
		Response (no_buy_new_tech, DoBuy);
	}
	else if (PLAYER_SAID (R, buy_info)
			|| PLAYER_SAID (R, buy_current_events)
			|| PLAYER_SAID (R, buy_alien_races)
			|| PLAYER_SAID (R, buy_history))
	{
		needed_credit = 0;
		if (PLAYER_SAID (R, buy_info))
		{
			if (GET_GAME_STATE (MELNORME_INFO_PROCEDURE))
				NPCPhrase (OK_BUY_INFO);
			else
			{
				NPCPhrase (BUY_INFO_INTRO);
				SET_GAME_STATE (MELNORME_INFO_PROCEDURE, 1);
			}
		}
		else
		{
#define INFO_COST 75
			needed_credit = INFO_COST;
			if ((int)credit >= (int)needed_credit)
			{
				if (PLAYER_SAID (R, buy_current_events))
					CurrentEvents ();
				else if (PLAYER_SAID (R, buy_alien_races))
					AlienRaces ();
				else /* if (R == buy_history) */
					History ();
			}

			DeltaCredit (-needed_credit);
			if (GET_GAME_STATE (MELNORME_EVENTS_INFO_STACK) < NUM_EVENT_ITEMS
					 || GET_GAME_STATE (MELNORME_ALIEN_INFO_STACK) < NUM_ALIEN_RACE_ITEMS
					 || GET_GAME_STATE (MELNORME_HISTORY_INFO_STACK) < NUM_HISTORY_ITEMS)
			{
			}
			else
			{
				NPCPhrase (INFO_ALL_GONE);
				goto BuyBuyBuy;
			}
		}

		if (GET_GAME_STATE (MELNORME_EVENTS_INFO_STACK) < NUM_EVENT_ITEMS)
			Response (buy_current_events, DoBuy);
		if (GET_GAME_STATE (MELNORME_ALIEN_INFO_STACK) < NUM_ALIEN_RACE_ITEMS)
			Response (buy_alien_races, DoBuy);
		if (GET_GAME_STATE (MELNORME_HISTORY_INFO_STACK) < NUM_HISTORY_ITEMS)
			Response (buy_history, DoBuy);
		Response (done_buying_info, DoBuy);
	}
	else
	{
		if (PLAYER_SAID (R, done_buying_fuel))
			NPCPhrase (OK_DONE_BUYING_FUEL);
		else if (PLAYER_SAID (R, no_buy_new_tech))
			NPCPhrase (OK_NO_BUY_NEW_TECH);
		else if (PLAYER_SAID (R, done_buying_info))
			NPCPhrase (OK_DONE_BUYING_INFO);
		else
			NPCPhrase (WHAT_TO_BUY);

BuyBuyBuy:
		if (GLOBAL_SIS (FuelOnBoard) < capacity)
			Response (buy_fuel, DoBuy);
		if (GET_GAME_STATE (MELNORME_TECH_STACK) < NUM_TECH_ITEMS)
			Response (buy_technology, DoBuy);
		if (GET_GAME_STATE (MELNORME_ALIEN_INFO_STACK) < NUM_ALIEN_RACE_ITEMS
				|| GET_GAME_STATE (MELNORME_HISTORY_INFO_STACK) < NUM_HISTORY_ITEMS
				|| GET_GAME_STATE (MELNORME_EVENTS_INFO_STACK) < NUM_EVENT_ITEMS)
			Response (buy_info, DoBuy);
		Response (done_buying, NatureOfConversation);
		Response (be_leaving_now, ExitConversation);
	}
}

static void
DoSell (RESPONSE_REF R)
{
	BYTE num_new_rainbows;
	UWORD rainbow_mask;
	SIZE added_credit;
	int what_to_sell_queued = 0;

	rainbow_mask = MAKE_WORD (
			GET_GAME_STATE (RAINBOW_WORLD0),
			GET_GAME_STATE (RAINBOW_WORLD1)
			);
	num_new_rainbows = (BYTE)(-GET_GAME_STATE (MELNORME_RAINBOW_COUNT));
	while (rainbow_mask)
	{
		if (rainbow_mask & 1)
			++num_new_rainbows;

		rainbow_mask >>= 1;
	}

	if (!PLAYER_SAID (R, sell))
	{
		if (PLAYER_SAID (R, sell_life_data))
		{
			DWORD TimeIn;

			added_credit = GLOBAL_SIS (TotalBioMass) * BIO_CREDIT_VALUE;

			NPCPhrase (SOLD_LIFE_DATA1);
			NPCPhrase (-(int)GLOBAL_SIS (TotalBioMass));
			NPCPhrase (SOLD_LIFE_DATA2);
			NPCPhrase (-(int)added_credit);
			NPCPhrase (SOLD_LIFE_DATA3);
			// queue WHAT_TO_SELL before talk-segue
			if (num_new_rainbows)
			{
				NPCPhrase (WHAT_TO_SELL);
				what_to_sell_queued = 1;
			}
			AlienTalkSegue (1);

			DrawCargoStrings ((BYTE)~0, (BYTE)~0);
			SleepThread (ONE_SECOND / 2);
			TimeIn = GetTimeCounter ();
			DrawCargoStrings (
					(BYTE)NUM_ELEMENT_CATEGORIES,
					(BYTE)NUM_ELEMENT_CATEGORIES
					);
			do
			{
				TimeIn = GetTimeCounter ();
				if (AnyButtonPress (TRUE))
				{
					DeltaCredit (GLOBAL_SIS (TotalBioMass) * BIO_CREDIT_VALUE);
					GLOBAL_SIS (TotalBioMass) = 0;
				}
				else
				{
					--GLOBAL_SIS (TotalBioMass);
					DeltaCredit (BIO_CREDIT_VALUE);
				}
				DrawCargoStrings (
						(BYTE)NUM_ELEMENT_CATEGORIES,
						(BYTE)NUM_ELEMENT_CATEGORIES
						);
			} while (GLOBAL_SIS (TotalBioMass));
			SleepThread (ONE_SECOND / 2);

			LockMutex (GraphicsLock);
			ClearSISRect (DRAW_SIS_DISPLAY);
			UnlockMutex (GraphicsLock);
		}
		else /* if (R == sell_rainbow_locations) */
		{
			added_credit = num_new_rainbows * (250 * BIO_CREDIT_VALUE);

			NPCPhrase (SOLD_RAINBOW_LOCATIONS1);
			NPCPhrase (-(int)num_new_rainbows);
			NPCPhrase (SOLD_RAINBOW_LOCATIONS2);
			NPCPhrase (-(int)added_credit);
			NPCPhrase (SOLD_RAINBOW_LOCATIONS3);

			num_new_rainbows += GET_GAME_STATE (MELNORME_RAINBOW_COUNT);
			SET_GAME_STATE (MELNORME_RAINBOW_COUNT, num_new_rainbows);
			num_new_rainbows = 0;

			DeltaCredit (added_credit);
		}
		
		AskedToBuy = FALSE;
	}

	if (GLOBAL_SIS (TotalBioMass) || num_new_rainbows)
	{
		if (!what_to_sell_queued)
			NPCPhrase (WHAT_TO_SELL);

		if (GLOBAL_SIS (TotalBioMass))
			Response (sell_life_data, DoSell);
		if (num_new_rainbows)
			Response (sell_rainbow_locations, DoSell);
		Response (done_selling, NatureOfConversation);
	}
	else
	{
		if (PLAYER_SAID (R, sell))
			NPCPhrase (NOTHING_TO_SELL);
		DISABLE_PHRASE (sell);

		NatureOfConversation (R);
	}
}

static void
NatureOfConversation (RESPONSE_REF R)
{
	BYTE num_new_rainbows;
	UWORD rainbow_mask;
	COUNT Credit;

	if (PLAYER_SAID (R, get_on_with_business))
	{
		SET_GAME_STATE (MELNORME_YACK_STACK2, 5);
		R = 0;
	}

	Credit = DeltaCredit (0);
	if (R == 0)
	{
		BYTE stack;

		stack = (BYTE)(GET_GAME_STATE (MELNORME_YACK_STACK2) - 5);
		switch (stack++)
		{
			case 0:
				NPCPhrase (HELLO_AND_DOWN_TO_BUSINESS_1);
				break;
			case 1:
				NPCPhrase (HELLO_AND_DOWN_TO_BUSINESS_2);
				break;
			case 2:
				NPCPhrase (HELLO_AND_DOWN_TO_BUSINESS_3);
				break;
			case 3:
				NPCPhrase (HELLO_AND_DOWN_TO_BUSINESS_4);
				break;
			case 4:
				NPCPhrase (HELLO_AND_DOWN_TO_BUSINESS_5);
				break;
			case 5:
				NPCPhrase (HELLO_AND_DOWN_TO_BUSINESS_6);
				break;
			case 6:
				NPCPhrase (HELLO_AND_DOWN_TO_BUSINESS_7);
				break;
			case 7:
				NPCPhrase (HELLO_AND_DOWN_TO_BUSINESS_8);
				break;
			case 8:
				NPCPhrase (HELLO_AND_DOWN_TO_BUSINESS_9);
				break;
			default:
				--stack;
				NPCPhrase (HELLO_AND_DOWN_TO_BUSINESS_10);
				break;
		}
		SET_GAME_STATE (MELNORME_YACK_STACK2, stack + 5);
	}

	rainbow_mask = MAKE_WORD (
			GET_GAME_STATE (RAINBOW_WORLD0),
			GET_GAME_STATE (RAINBOW_WORLD1)
			);
	num_new_rainbows = (BYTE)(-GET_GAME_STATE (MELNORME_RAINBOW_COUNT));
	while (rainbow_mask)
	{
		if (rainbow_mask & 1)
			++num_new_rainbows;

		rainbow_mask >>= 1;
	}

	if (GLOBAL_SIS (FuelOnBoard) > 0
			|| GLOBAL_SIS (TotalBioMass)
			|| Credit
			|| num_new_rainbows)
	{
		if (!GET_GAME_STATE (TRADED_WITH_MELNORME))
		{
			SET_GAME_STATE (TRADED_WITH_MELNORME, 1);

			NPCPhrase (TRADING_INFO);
		}

		if (R == 0)
		{
				/* Melnorme reports any news and turns purple */
			NPCPhrase (BUY_OR_SELL);
			AlienTalkSegue(1);
			XFormColorMap (GetColorMapAddress (
					SetAbsColorMapIndex (CommData.AlienColorMap, 1)
					), ONE_SECOND / 2);
			AlienTalkSegue((COUNT)~0);
		}
		else if (PLAYER_SAID (R, why_turned_purple))
		{
			SET_GAME_STATE (WHY_MELNORME_PURPLE, 1);

			NPCPhrase (TURNED_PURPLE_BECAUSE);
		}
		else if (PLAYER_SAID (R, done_selling))
		{
			NPCPhrase (OK_DONE_SELLING);
		}
		else if (PLAYER_SAID (R, done_buying))
		{
			NPCPhrase (OK_DONE_BUYING);
		}

		if (!GET_GAME_STATE (WHY_MELNORME_PURPLE))
		{
			Response (why_turned_purple, NatureOfConversation);
		}
		if (!AskedToBuy)
			Response (buy, DoBuy);
		if (PHRASE_ENABLED (sell))
			Response (sell, DoSell);
		Response (goodbye, ExitConversation);
	}
	else /* needs to be rescued */
	{
		if (GET_GAME_STATE (MELNORME_RESCUE_REFUSED))
		{
			NPCPhrase (CHANGED_MIND);

			Response (yes_changed_mind, DoRescue);
			Response (no_changed_mind, ExitConversation);
		}
		else
		{
			BYTE num_rescues;

			num_rescues = GET_GAME_STATE (MELNORME_RESCUE_COUNT);
			switch (num_rescues)
			{
				case 0:
					NPCPhrase (RESCUE_EXPLANATION);
					break;
				case 1:
					NPCPhrase (RESCUE_AGAIN_1);
					break;
				case 2:
					NPCPhrase (RESCUE_AGAIN_2);
					break;
				case 3:
					NPCPhrase (RESCUE_AGAIN_3);
					break;
				case 4:
					NPCPhrase (RESCUE_AGAIN_4);
					break;
				case 5:
					NPCPhrase (RESCUE_AGAIN_5);
					break;
			 }

			if (num_rescues < 5)
			{
				++num_rescues;
				SET_GAME_STATE (MELNORME_RESCUE_COUNT, num_rescues);
			}

			NPCPhrase (SHOULD_WE_HELP_YOU);

			Response (yes_help, DoRescue);
			Response (no_help, ExitConversation);
		}
	}
}

static BYTE local_stack0, local_stack1;

static void
DoBluster (RESPONSE_REF R)
{
	if (PLAYER_SAID (R, trade_is_for_the_weak))
	{
		XFormColorMap (GetColorMapAddress (
				SetAbsColorMapIndex (CommData.AlienColorMap, 2)
				), ONE_SECOND / 2);

		SET_GAME_STATE (MELNORME_YACK_STACK2, 4);
		NPCPhrase (WERE_NOT_AFRAID);
	}
	else if (PLAYER_SAID (R, why_blue_light))
	{
		SET_GAME_STATE (WHY_MELNORME_BLUE, 1);

		NPCPhrase (BLUE_IS_MAD);
	}
	else if (PLAYER_SAID (R, we_strong_1))
	{
		local_stack0 = 1;
		NPCPhrase (YOU_NOT_STRONG_1);
	}
	else if (PLAYER_SAID (R, we_strong_2))
	{
		local_stack0 = 2;
		NPCPhrase (YOU_NOT_STRONG_2);
	}
	else if (PLAYER_SAID (R, we_strong_3))
	{
		local_stack0 = 3;
		NPCPhrase (YOU_NOT_STRONG_3);
	}
	else if (PLAYER_SAID (R, just_testing))
	{
		local_stack1 = 1;
		NPCPhrase (REALLY_TESTING);
	}

	if (!GET_GAME_STATE (WHY_MELNORME_BLUE))
		Response (why_blue_light, DoBluster);
	switch (local_stack0)
	{
		case 0:
			Response (we_strong_1, DoBluster);
			break;
		case 1:
			Response (we_strong_2, DoBluster);
			break;
		case 2:
			Response (we_strong_3, DoBluster);
			break;
	}
	switch (local_stack1)
	{
		case 0:
			Response (just_testing, DoBluster);
			break;
		case 1:
		{
			Response (yes_really_testing, DoFirstMeeting);
			break;
		}
	}
	Response (youre_on, ExitConversation);
}

static void
yack0_respond (void)
{

	switch (GET_GAME_STATE (MELNORME_YACK_STACK0))
	{
		case 0:
		{
			UNICODE buf[ALLIANCE_NAME_BUFSIZE];

			GetAllianceName (buf, name_1);
			construct_response (
					shared_phrase_buf,
					we_are_from_alliance0,
					buf,
					(RESPONSE_REF)-1);
			DoResponsePhrase (we_are_from_alliance0, DoFirstMeeting, shared_phrase_buf);
			break;
		}
		case 1:
			Response (how_know, DoFirstMeeting);
			break;
	}
}

static void
yack1_respond (void)
{
	switch (GET_GAME_STATE (MELNORME_YACK_STACK1))
	{
		case 0:
			Response (what_about_yourselves, DoFirstMeeting);
			break;
		case 1:
			Response (what_factors, DoFirstMeeting);
		case 2:
			Response (get_on_with_business, NatureOfConversation);
			break;
	}
}

static void
yack2_respond (void)
{
	switch (GET_GAME_STATE (MELNORME_YACK_STACK2))
	{
		case 0:
			Response (what_about_universe, DoFirstMeeting);
			break;
		case 1:
			Response (giving_is_good_1, DoFirstMeeting);
			break;
		case 2:
			Response (giving_is_good_2, DoFirstMeeting);
			break;
		case 3:
			Response (trade_is_for_the_weak, DoBluster);
			break;
	}
}

static void
DoFirstMeeting (RESPONSE_REF R)
{
	BYTE last_stack = 0;
	PVOIDFUNC temp_func, stack_func[] =
	{
		yack0_respond,
		yack1_respond,
		yack2_respond,
	};

	if (R == 0)
	{
		BYTE business_count;

		business_count = GET_GAME_STATE (MELNORME_BUSINESS_COUNT);
		switch (business_count++)
		{
			case 0:
				NPCPhrase (HELLO_NOW_DOWN_TO_BUSINESS_1);
				break;
			case 1:
				NPCPhrase (HELLO_NOW_DOWN_TO_BUSINESS_2);
				break;
			case 2:
				NPCPhrase (HELLO_NOW_DOWN_TO_BUSINESS_3);
				--business_count;
				break;
		}
		SET_GAME_STATE (MELNORME_BUSINESS_COUNT, business_count);
	}
	else if (PLAYER_SAID (R, we_are_from_alliance0))
	{
		SET_GAME_STATE (MELNORME_YACK_STACK0, 1);
		NPCPhrase (KNOW_OF_YOU);
	}
	else if (PLAYER_SAID (R, how_know))
	{
		SET_GAME_STATE (MELNORME_YACK_STACK0, 2);
		NPCPhrase (KNOW_BECAUSE);
	}
	else if (PLAYER_SAID (R, what_about_yourselves))
	{
		last_stack = 1;
		SET_GAME_STATE (MELNORME_YACK_STACK1, 1);
		NPCPhrase (NO_TALK_ABOUT_OURSELVES);
	}
	else if (PLAYER_SAID (R, what_factors))
	{
		last_stack = 1;
		SET_GAME_STATE (MELNORME_YACK_STACK1, 2);
		NPCPhrase (FACTORS_ARE);
	}
	else if (PLAYER_SAID (R, what_about_universe))
	{
		last_stack = 2;
		SET_GAME_STATE (MELNORME_YACK_STACK2, 1);
		NPCPhrase (NO_FREE_LUNCH);
	}
	else if (PLAYER_SAID (R, giving_is_good_1))
	{
		last_stack = 2;
		SET_GAME_STATE (MELNORME_YACK_STACK2, 2);
		NPCPhrase (GIVING_IS_BAD_1);
	}
	else if (PLAYER_SAID (R, giving_is_good_2))
	{
		last_stack = 2;
		SET_GAME_STATE (MELNORME_YACK_STACK2, 3);
		NPCPhrase (GIVING_IS_BAD_2);
	}
	else if (PLAYER_SAID (R, yes_really_testing))
	{
		XFormColorMap (GetColorMapAddress (
				SetAbsColorMapIndex (CommData.AlienColorMap, 0)
				), ONE_SECOND / 2);

		NPCPhrase (TEST_RESULTS);
	}
	else if (PLAYER_SAID (R, we_apologize))
	{
		SET_GAME_STATE (MELNORME_ANGER, 0);
		XFormColorMap (GetColorMapAddress (
				SetAbsColorMapIndex (CommData.AlienColorMap, 0)
				), ONE_SECOND / 2);

		NPCPhrase (APOLOGY_ACCEPTED);
	}

	temp_func = stack_func[0];
	stack_func[0] = stack_func[last_stack];
	stack_func[last_stack] = temp_func;
	(*stack_func[0]) ();
	(*stack_func[1]) ();
	(*stack_func[2]) ();
	Response (no_trade_now, ExitConversation);
}

static void
DoMelnormeMiffed (RESPONSE_REF R)
{
	if (R == 0)
	{
		BYTE miffed_count;

		miffed_count = GET_GAME_STATE (MELNORME_MIFFED_COUNT);
		switch (miffed_count++)
		{
			case 0:
				NPCPhrase (HELLO_SLIGHTLY_ANGRY_1);
				break;
			case 1:
				NPCPhrase (HELLO_SLIGHTLY_ANGRY_2);
				break;
			default:
				--miffed_count;
				NPCPhrase (HELLO_SLIGHTLY_ANGRY_3);
				break;
		}
		SET_GAME_STATE (MELNORME_MIFFED_COUNT, miffed_count);

		XFormColorMap (GetColorMapAddress (
				SetAbsColorMapIndex (CommData.AlienColorMap, 2)
				), ONE_SECOND / 2);
	}
	else if (PLAYER_SAID (R, explore_relationship))
	{
		SET_GAME_STATE (MELNORME_YACK_STACK3, 1);

		NPCPhrase (EXAMPLE_OF_RELATIONSHIP);
	}
	else if (PLAYER_SAID (R, excuse_1))
	{
		SET_GAME_STATE (MELNORME_YACK_STACK3, 2);

		NPCPhrase (NO_EXCUSE_1);
	}
	else if (PLAYER_SAID (R, excuse_2))
	{
		SET_GAME_STATE (MELNORME_YACK_STACK3, 3);

		NPCPhrase (NO_EXCUSE_2);
	}
	else if (PLAYER_SAID (R, excuse_3))
	{
		SET_GAME_STATE (MELNORME_YACK_STACK3, 4);

		NPCPhrase (NO_EXCUSE_3);
	}

	switch (GET_GAME_STATE (MELNORME_YACK_STACK3))
	{
		case 0:
			Response (explore_relationship, DoMelnormeMiffed);
			break;
		case 1:
			Response (excuse_1, DoMelnormeMiffed);
			break;
		case 2:
			Response (excuse_2, DoMelnormeMiffed);
			break;
		case 3:
			Response (excuse_3, DoMelnormeMiffed);
			break;
	}
	Response (we_apologize, DoFirstMeeting);
	Response (so_we_can_attack, ExitConversation);
	Response (bye_melnorme_slightly_angry, ExitConversation);
}

static void
DoMelnormePissed (RESPONSE_REF R)
{
	if (R == 0)
	{
		BYTE pissed_count;

		pissed_count = GET_GAME_STATE (MELNORME_PISSED_COUNT);
		switch (pissed_count++)
		{
			case 0:
				NPCPhrase (HELLO_PISSED_OFF_1);
				break;
			case 1:
				NPCPhrase (HELLO_PISSED_OFF_2);
				break;
			default:
				--pissed_count;
				NPCPhrase (HELLO_PISSED_OFF_3);
				break;
		}
		SET_GAME_STATE (MELNORME_PISSED_COUNT, pissed_count);

		XFormColorMap (GetColorMapAddress (
				SetAbsColorMapIndex (CommData.AlienColorMap, 2)
				), ONE_SECOND / 2);
	}
	else if (PLAYER_SAID (R, beg_forgiveness))
	{
		SET_GAME_STATE (MELNORME_YACK_STACK4, 1);

		NPCPhrase (LOTS_TO_MAKE_UP_FOR);
	}
	else if (PLAYER_SAID (R, you_are_so_right))
	{
		SET_GAME_STATE (MELNORME_YACK_STACK4, 2);

		NPCPhrase (ONE_LAST_CHANCE);
	}

	switch (GET_GAME_STATE (MELNORME_YACK_STACK4))
	{
		case 0:
			Response (beg_forgiveness, DoMelnormePissed);
			break;
		case 1:
			Response (you_are_so_right, DoMelnormePissed);
			break;
		case 2:
			Response (ok_strip_me, ExitConversation);
			break;
	}
	Response (fight_some_more, ExitConversation);
	Response (bye_melnorme_pissed_off, ExitConversation);
}

static void
DoMelnormeHate (RESPONSE_REF R)
{
	BYTE hate_count;

	(void) R;  // ignored
	hate_count = GET_GAME_STATE (MELNORME_HATE_COUNT);
	switch (hate_count++)
	{
		case 0:
			NPCPhrase (HELLO_HATE_YOU_1);
			break;
		case 1:
			NPCPhrase (HELLO_HATE_YOU_2);
			break;
		default:
			--hate_count;
			NPCPhrase (HELLO_HATE_YOU_3);
			break;
	}
	SET_GAME_STATE (MELNORME_HATE_COUNT, hate_count);

	XFormColorMap (GetColorMapAddress (
			SetAbsColorMapIndex (CommData.AlienColorMap, 2)
			), ONE_SECOND / 2);

	Response (well_if_thats_the_way_you_feel, ExitConversation);
	Response (you_hate_us_so_we_go_away, ExitConversation);
}

static void
Intro (void)
{
	prevMsgMode = SetStatusMessageMode (SMM_CREDITS);

	if (GET_GAME_STATE (MET_MELNORME) == 0)
	{
		SET_GAME_STATE (MET_MELNORME, 1);
		DoFirstMeeting (0);
	}
	else
	{
		switch (GET_GAME_STATE (MELNORME_ANGER))
		{
			case 0:
				if (GET_GAME_STATE (MELNORME_YACK_STACK2) <= 5)
					DoFirstMeeting (0);
				else
					NatureOfConversation (0);
				break;
			case 1:
				DoMelnormeMiffed (0);
				break;
			case 2:
				DoMelnormePissed (0);
				break;
			default:
				DoMelnormeHate (0);
				break;
		}
	}
}

static COUNT
uninit_melnorme (void)
{
	return 0;
}

static void
post_melnorme_enc (void)
{
	LockMutex (GraphicsLock);
	if (prevMsgMode != SMM_UNDEFINED)
		SetStatusMessageMode (prevMsgMode);
	DrawStatusMessage (NULL);
	UnlockMutex (GraphicsLock);
}

LOCDATA*
init_melnorme_comm (void)
{
	LOCDATA *retval;

	melnorme_desc.init_encounter_func = Intro;
	melnorme_desc.post_encounter_func = post_melnorme_enc;
	melnorme_desc.uninit_encounter_func = uninit_melnorme;

	melnorme_desc.AlienTextBaseline.x = TEXT_X_OFFS + (SIS_TEXT_WIDTH >> 1);
	melnorme_desc.AlienTextBaseline.y = 0;
	melnorme_desc.AlienTextWidth = SIS_TEXT_WIDTH - 16;

	local_stack0 = 0;
	local_stack1 = 0;

	prevMsgMode = SMM_UNDEFINED;

	SET_GAME_STATE (BATTLE_SEGUE, 0);
	AskedToBuy = FALSE;
	retval = &melnorme_desc;

	return (retval);
}

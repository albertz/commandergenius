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

#include "restart.h"

#include "colors.h"
#include "controls.h"
#include "credits.h"
// XXX: for star_array[]
#include "encount.h"
#include "fmv.h"
#include "menustat.h"
#include "gamestr.h"
#include "globdata.h"
#include "intel.h"
#include "supermelee/melee.h"
#include "resinst.h"
#include "nameref.h"
#include "settings.h"
#include "load.h"
#include "setup.h"
#include "sounds.h"
#include "setupmenu.h"
#include "util.h"
#include "starcon.h"
#include "uqmversion.h"
#include "libs/graphics/gfx_common.h"
#include "libs/inplib.h"


// TODO: This entire module fails to uphold the GraphicsLock semantics
//   This either has to be fixed, or GraphicsLock completely ignored,
//   or will become irrelevant if GraphicsLock completely removed.

enum
{
	START_NEW_GAME = 0,
	LOAD_SAVED_GAME,
	PLAY_SUPER_MELEE,
	SETUP_GAME,
	QUIT_GAME
};

// Draw the full restart menu. Nothing is done with selections.
static void
DrawRestartMenuGraphic (MENU_STATE *pMS)
{
	RECT r;
	STAMP s;
	TEXT t;
	UNICODE buf[64];

	s.frame = CaptureDrawable (LoadGraphic (RESTART_PMAP_ANIM));
	pMS->CurFrame = s.frame;
	GetFrameRect (s.frame, &r);
	s.origin.x = (SCREEN_WIDTH - r.extent.width) >> 1;
	s.origin.y = (SCREEN_HEIGHT - r.extent.height) >> 1;
	
	SetContextBackGroundColor (BLACK_COLOR);
	BatchGraphics ();
	ClearDrawable ();
	FlushColorXForms ();
	LockMutex (GraphicsLock);
	DrawStamp (&s);

	// Put the version number in the bottom right corner.
	SetContextFont (TinyFont);
	t.pStr = buf;
	t.baseline.x = SCREEN_WIDTH - 3;
	t.baseline.y = SCREEN_HEIGHT - 2;
	t.align = ALIGN_RIGHT;
	t.CharCount = (COUNT)~0;
	sprintf (buf, "v%d.%d.%d%s", UQM_MAJOR_VERSION, UQM_MINOR_VERSION,
			UQM_PATCH_VERSION, UQM_EXTRA_VERSION);
	SetContextForeGroundColor (WHITE_COLOR);
	font_DrawText (&t);

	UnlockMutex (GraphicsLock);
	UnbatchGraphics ();
}

static void
DrawRestartMenu (MENU_STATE *pMS, BYTE NewState, FRAME f)
{
	POINT origin;
	origin.x = 0;
	origin.y = 0;
	Flash_setOverlay(pMS->flashContext,
			&origin, SetAbsFrameIndex (f, NewState + 1));
}

static BOOLEAN
DoRestart (MENU_STATE *pMS)
{
	static TimeCount LastInputTime;
	static TimeCount InactTimeOut;
	TimeCount TimeIn = GetTimeCounter ();

	/* Cancel any presses of the Pause key. */
	GamePaused = FALSE;

	if (pMS->Initialized)
		Flash_process(pMS->flashContext);

	if (!pMS->Initialized)
	{
		if (pMS->hMusic)
		{
			StopMusic ();
			DestroyMusic (pMS->hMusic);
			pMS->hMusic = 0;
		}
		pMS->hMusic = LoadMusic (MAINMENU_MUSIC);
		InactTimeOut = (pMS->hMusic ? 120 : 20) * ONE_SECOND;
		pMS->flashContext = Flash_createOverlay (ScreenContext,
				NULL, NULL);
		Flash_setMergeFactors (pMS->flashContext, -3, 3, 16);
		Flash_setSpeed (pMS->flashContext, (6 * ONE_SECOND) / 16, 0,
				(6 * ONE_SECOND) / 16, 0);
		Flash_setFrameTime (pMS->flashContext, ONE_SECOND / 16);
		Flash_setState(pMS->flashContext, FlashState_fadeIn,
				(3 * ONE_SECOND) / 16);
		DrawRestartMenu (pMS, pMS->CurState, pMS->CurFrame);
		Flash_start (pMS->flashContext);
		PlayMusic (pMS->hMusic, TRUE, 1);
		LastInputTime = GetTimeCounter ();
		pMS->Initialized = TRUE;

		SleepThreadUntil (FadeScreen (FadeAllToColor, ONE_SECOND / 2));
	}
	else if (GLOBAL (CurrentActivity) & CHECK_ABORT)
	{
		return FALSE;
	}
	else if (PulsedInputState.menu[KEY_MENU_SELECT])
	{
		switch (pMS->CurState)
		{
			case LOAD_SAVED_GAME:
				LastActivity = CHECK_LOAD;
				GLOBAL (CurrentActivity) = IN_INTERPLANETARY;
				break;
			case START_NEW_GAME:
				LastActivity = CHECK_LOAD | CHECK_RESTART;
				GLOBAL (CurrentActivity) = IN_INTERPLANETARY;
				break;
			case PLAY_SUPER_MELEE:
				GLOBAL (CurrentActivity) = SUPER_MELEE;
				break;
			case SETUP_GAME:
				Flash_pause(pMS->flashContext);
				Flash_setState(pMS->flashContext, FlashState_fadeIn,
						(3 * ONE_SECOND) / 16);
				SetupMenu ();
				SetMenuSounds (MENU_SOUND_UP | MENU_SOUND_DOWN,
						MENU_SOUND_SELECT);
				LastInputTime = GetTimeCounter ();
				SetTransitionSource (NULL);
				BatchGraphics ();
				DrawRestartMenuGraphic (pMS);
				ScreenTransition (3, NULL);
				DrawRestartMenu (pMS, pMS->CurState, pMS->CurFrame);
				Flash_continue(pMS->flashContext);
				UnbatchGraphics ();
				return TRUE;
			case QUIT_GAME:
				SleepThreadUntil (FadeScreen (FadeAllToBlack, ONE_SECOND / 2));
				GLOBAL (CurrentActivity) = CHECK_ABORT;
				break;
		}

		Flash_pause(pMS->flashContext);

		return FALSE;
	}
	else if (PulsedInputState.menu[KEY_MENU_UP] ||
			PulsedInputState.menu[KEY_MENU_DOWN])
	{
		BYTE NewState;

		NewState = pMS->CurState;
		if (PulsedInputState.menu[KEY_MENU_UP])
		{
			if (NewState == START_NEW_GAME)
				NewState = QUIT_GAME;
			else
				--NewState;
		}
		else if (PulsedInputState.menu[KEY_MENU_DOWN])
		{
			if (NewState == QUIT_GAME)
				NewState = START_NEW_GAME;
			else
				++NewState;
		}
		if (NewState != pMS->CurState)
		{
			BatchGraphics ();
			DrawRestartMenu (pMS, NewState, pMS->CurFrame);
			UnbatchGraphics ();
			pMS->CurState = NewState;
		}

		LastInputTime = GetTimeCounter ();
	}
	else if (PulsedInputState.menu[KEY_MENU_LEFT] ||
			PulsedInputState.menu[KEY_MENU_RIGHT])
	{	// Does nothing, but counts as input for timeout purposes
		LastInputTime = GetTimeCounter ();
	}
	else if (MouseButtonDown)
	{
		Flash_pause(pMS->flashContext);
		DoPopupWindow (GAME_STRING (MAINMENU_STRING_BASE + 54));
				// Mouse not supported message
		SetMenuSounds (MENU_SOUND_UP | MENU_SOUND_DOWN, MENU_SOUND_SELECT);	
		SetTransitionSource (NULL);
		BatchGraphics ();
		DrawRestartMenuGraphic (pMS);
		DrawRestartMenu (pMS, pMS->CurState, pMS->CurFrame);
		ScreenTransition (3, NULL);
		UnbatchGraphics ();
		Flash_continue(pMS->flashContext);

		LastInputTime = GetTimeCounter ();
	}
	else
	{	// No input received, check if timed out
		if (GetTimeCounter () - LastInputTime > InactTimeOut)
		{
			SleepThreadUntil (FadeMusic (0, ONE_SECOND));
			StopMusic ();
			FadeMusic (NORMAL_VOLUME, 0);

			GLOBAL (CurrentActivity) = (ACTIVITY)~0;
			return FALSE;
		}
	}

	SleepThreadUntil (TimeIn + ONE_SECOND / 30);

	return TRUE;
}

static BOOLEAN
RestartMenu (MENU_STATE *pMS)
{
	TimeCount TimeOut;

	ReinitQueue (&race_q[0]);
	ReinitQueue (&race_q[1]);

	SetContext (ScreenContext);

	GLOBAL (CurrentActivity) |= CHECK_ABORT;
	if (GLOBAL_SIS (CrewEnlisted) == (COUNT)~0
			&& GET_GAME_STATE (UTWIG_BOMB_ON_SHIP)
			&& !GET_GAME_STATE (UTWIG_BOMB))
	{	// player blew himself up with Utwig bomb
		SET_GAME_STATE (UTWIG_BOMB_ON_SHIP, 0);

		SleepThreadUntil (FadeScreen (FadeAllToWhite, ONE_SECOND / 8)
				+ ONE_SECOND / 60);
		SetContextBackGroundColor (
				BUILD_COLOR (MAKE_RGB15 (0x1F, 0x1F, 0x1F), 0x0F));
		ClearDrawable ();
		FlushColorXForms ();

		TimeOut = ONE_SECOND / 8;
	}
	else
	{
		TimeOut = ONE_SECOND / 2;

		if (LOBYTE (LastActivity) == WON_LAST_BATTLE)
		{
			GLOBAL (CurrentActivity) = WON_LAST_BATTLE;
			Victory ();
			Credits (TRUE);

			FreeGameData ();
			
			GLOBAL (CurrentActivity) = CHECK_ABORT;
		}
	}

	LastActivity = 0;
	NextActivity = 0;

	// TODO: This fade is not always necessary, especially after a splash
	//   screen. It only makes a user wait.
	SleepThreadUntil (FadeScreen (FadeAllToBlack, TimeOut));
	if (TimeOut == ONE_SECOND / 8)
		SleepThread (ONE_SECOND * 3);
	DrawRestartMenuGraphic (pMS);
	GLOBAL (CurrentActivity) &= ~CHECK_ABORT;
	SetMenuSounds (MENU_SOUND_UP | MENU_SOUND_DOWN, MENU_SOUND_SELECT);
	SetDefaultMenuRepeatDelay ();
	DoInput (pMS, TRUE);
	
	StopMusic ();
	if (pMS->hMusic)
	{
		DestroyMusic (pMS->hMusic);
		pMS->hMusic = 0;
	}

	Flash_terminate (pMS->flashContext);
	DestroyDrawable (ReleaseDrawable (pMS->CurFrame));

	if (GLOBAL (CurrentActivity) == (ACTIVITY)~0)
		return (FALSE); // timed out

	if (GLOBAL (CurrentActivity) & CHECK_ABORT)
		return (FALSE); // quit

	TimeOut = FadeScreen (FadeAllToBlack, ONE_SECOND / 2);
	
	SleepThreadUntil (TimeOut);
	FlushColorXForms ();

	SeedRandomNumbers ();

	return (LOBYTE (GLOBAL (CurrentActivity)) != SUPER_MELEE);
}

static BOOLEAN
TryStartGame (void)
{
	MENU_STATE MenuState;

	LastActivity = GLOBAL (CurrentActivity);
	GLOBAL (CurrentActivity) = 0;

	memset (&MenuState, 0, sizeof (MenuState));
	MenuState.InputFunc = DoRestart;

	while (!RestartMenu (&MenuState))
	{	// spin until a game is started or loaded
		if (LOBYTE (GLOBAL (CurrentActivity)) == SUPER_MELEE &&
				!(GLOBAL (CurrentActivity) & CHECK_ABORT))
		{
			FreeGameData ();
			Melee ();
			MenuState.Initialized = FALSE;
		}
		else if (GLOBAL (CurrentActivity) == (ACTIVITY)~0)
		{	// timed out
			SleepThreadUntil (FadeScreen (FadeAllToBlack, ONE_SECOND / 2));
			return (FALSE);
		}
		else if (GLOBAL (CurrentActivity) & CHECK_ABORT)
		{	// quit
			return (FALSE);
		}
	}

	return TRUE;
}

BOOLEAN
StartGame (void)
{
	do
	{
		while (!TryStartGame ())
		{
			if (GLOBAL (CurrentActivity) == (ACTIVITY)~0)
			{	// timed out
				GLOBAL (CurrentActivity) = 0;
				SplashScreen (0);
				Credits (FALSE);
			}

			if (GLOBAL (CurrentActivity) & CHECK_ABORT)
				return (FALSE); // quit
		}

		if (LastActivity & CHECK_RESTART)
		{	// starting a new game
			Introduction ();
		}
	
	} while (GLOBAL (CurrentActivity) & CHECK_ABORT);

	{
		extern STAR_DESC starmap_array[];
		extern const BYTE element_array[];
		extern const PlanetFrame planet_array[];

		star_array = starmap_array;
		Elements = element_array;
		PlanData = planet_array;
	}

	PlayerControl[0] = HUMAN_CONTROL | STANDARD_RATING;
	PlayerControl[1] = COMPUTER_CONTROL | AWESOME_RATING;

	return (TRUE);
}


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

#include <stdlib.h>

#include "comm.h"
#include "battle.h"
#include "fmv.h"
#include "gameev.h"
#include "types.h"
#include "globdata.h"
#include "load.h"
#include "resinst.h"
#include "restart.h"
#include "starbase.h"
#include "setup.h"
#include "master.h"
#include "controls.h"
#include "starcon.h"
#include "clock.h"
		// for GameClockTick()
#include "hyper.h"
		// for SeedUniverse()
#include "planets/planets.h"
		// for ExploreSolarSys()
#include "uqmdebug.h"
#include "libs/tasklib.h"
#include "libs/log.h"
#include "libs/gfxlib.h"
#include "libs/graphics/gfx_common.h"
#include "libs/graphics/tfb_draw.h"
#include "libs/misc.h"

#include "uqmversion.h"
#include "options.h"

volatile int MainExited = FALSE;

// Open or close the periodically occuring QuasiSpace portal.
// It changes the appearant portal size when necessary.
static void
checkArilouGate (void)
{
	BYTE counter;

	counter = GET_GAME_STATE (ARILOU_SPACE_COUNTER);
	if (GET_GAME_STATE (ARILOU_SPACE) == OPENING)
	{	// The portal is opening or fully open
		if (counter < 9)
			++counter;
	}
	else
	{	// The portal is closing or fully closed
		if (counter > 0)
			--counter;
	}
	SET_GAME_STATE (ARILOU_SPACE_COUNTER, counter);
}

// Battle frame callback function.
// Called with GraphicsLock held
static void
on_battle_frame (void)
{
	GameClockTick ();
	checkArilouGate ();

	if (!(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD)))
		SeedUniverse ();

	DrawAutoPilotMessage (FALSE);
}

static void
BackgroundInitKernel (DWORD TimeOut)
{
	LoadMasterShipList (TaskSwitch);
	TaskSwitch ();
	InitGameKernel ();

	while ((GetTimeCounter () <= TimeOut) &&
	       !(GLOBAL (CurrentActivity) & CHECK_ABORT))
	{
		UpdateInputState ();
		TaskSwitch ();
	}
}

void
SignalStopMainThread (void)
{
	GamePaused = FALSE;
	GLOBAL (CurrentActivity) |= CHECK_ABORT;
	TaskSwitch ();
}

void
ProcessUtilityKeys (void)
{
	if (ImmediateInputState.menu[KEY_ABORT])
	{
		log_showBox (false, false);
		exit (EXIT_SUCCESS);
	}
	
	if (ImmediateInputState.menu[KEY_FULLSCREEN])
	{
		int flags = GfxFlags ^ TFB_GFXFLAGS_FULLSCREEN;
		// clear ImmediateInputState so we don't repeat this next frame
		FlushInput ();
		TFB_DrawScreen_ReinitVideo (GraphicsDriver, flags, ScreenWidthActual,
				ScreenHeightActual);
	}

#if defined(DEBUG) || defined(USE_DEBUG_KEY)
	if (ImmediateInputState.menu[KEY_DEBUG])
	{
		// clear ImmediateInputState so we don't repeat this next frame
		FlushInput ();
		debugKeyPressed ();
	}
#endif  /* DEBUG */
}

/* TODO: Remove these declarations once threading is gone. */
extern int snddriver, soundflags;

int
Starcon2Main (void *threadArg)
{
#if CREATE_JOURNAL
{
int ac = argc;
char **av = argv;

while (--ac > 0)
{
	++av;
	if ((*av)[0] == '-')
	{
		switch ((*av)[1])
		{
#if CREATE_JOURNAL
			case 'j':
				++create_journal;
				break;
#endif //CREATE_JOURNAL
		}
	}
}
}
#endif // CREATE_JOURNAL

	{
		/* TODO: Put initAudio back in main where it belongs once threading
		 *       is gone.
		 */
		extern sint32 initAudio (sint32 driver, sint32 flags);
		initAudio (snddriver, soundflags);
	}

	if (!LoadKernel (0,0))
	{
		log_add (log_Fatal, "\n  *** FATAL ERROR: Could not load basic content ***\n\nUQM requires at least the base content pack to run properly.");
		log_add (log_Fatal, "This file is typically called uqm-%d.%d.0.uqm.  UQM was expecting it", UQM_MAJOR_VERSION, UQM_MINOR_VERSION);
		log_add (log_Fatal, "in the %s/packages directory.", baseContentPath);
		log_add (log_Fatal, "Either your installation did not install the content pack at all, or it\ninstalled it in a different directory.\n\nFix your installation and rerun UQM.\n\n  *******************\n");
		exit (EXIT_FAILURE);
	}
	log_add (log_Info, "We've loaded the Kernel");

	Logo ();
	
	GLOBAL (CurrentActivity) = 0;
	// show splash and init the kernel in the meantime
	SplashScreen (BackgroundInitKernel);

//	OpenJournal ();
	while (StartGame ())
	{
		// Initialise a new game
		if (!SetPlayerInputAll ()) {
			log_add (log_Fatal, "Could not set player input.");
			explode ();  // Does not return;
		}
		InitGameStructures ();
		InitGameClock ();
		AddInitialGameEvents();

		do
		{
#ifdef DEBUG
			if (debugHook != NULL)
			{
				void (*saveDebugHook) (void);
				saveDebugHook = debugHook;
				debugHook = NULL;
						// No further debugHook calls unless the called
						// function resets debugHook.
				(*saveDebugHook) ();
				continue;
			}
#endif
			SetStatusMessageMode (SMM_DEFAULT);

			if (!((GLOBAL (CurrentActivity) | NextActivity) & CHECK_LOAD))
				ZeroVelocityComponents (&GLOBAL (velocity));
					// not going into talking pet conversation
			else if (GLOBAL (CurrentActivity) & CHECK_LOAD)
				GLOBAL (CurrentActivity) = NextActivity;

			if ((GLOBAL (CurrentActivity) & START_ENCOUNTER)
					|| GET_GAME_STATE (CHMMR_BOMB_STATE) == 2)
			{
				if (GET_GAME_STATE (CHMMR_BOMB_STATE) == 2
						&& !GET_GAME_STATE (STARBASE_AVAILABLE))
				{	/* BGD mode */
					InstallBombAtEarth ();
				}
				else if (GET_GAME_STATE (GLOBAL_FLAGS_AND_DATA) == (BYTE)~0
						|| GET_GAME_STATE (CHMMR_BOMB_STATE) == 2)
				{
					GLOBAL (CurrentActivity) |= START_ENCOUNTER;
					VisitStarBase ();
				}
				else
				{
					GLOBAL (CurrentActivity) |= START_ENCOUNTER;
					RaceCommunication ();
				}

				if (!(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD)))
				{
					GLOBAL (CurrentActivity) &= ~START_ENCOUNTER;
					if (LOBYTE (GLOBAL (CurrentActivity)) == IN_INTERPLANETARY)
						GLOBAL (CurrentActivity) |= START_INTERPLANETARY;
				}
			}
			else if (GLOBAL (CurrentActivity) & START_INTERPLANETARY)
			{
				GLOBAL (CurrentActivity) = MAKE_WORD (IN_INTERPLANETARY, 0);

				DrawAutoPilotMessage (TRUE);
				SetGameClockRate (INTERPLANETARY_CLOCK_RATE);
				ExploreSolarSys ();
			}
			else
			{
				// Entering HyperSpace or QuasiSpace.
				GLOBAL (CurrentActivity) = MAKE_WORD (IN_HYPERSPACE, 0);

				DrawAutoPilotMessage (TRUE);
				SetGameClockRate (HYPERSPACE_CLOCK_RATE);
				Battle (&on_battle_frame);
			}

			LockMutex (GraphicsLock);
			SetFlashRect (NULL);
			UnlockMutex (GraphicsLock);

			LastActivity = GLOBAL (CurrentActivity);

			if (!(GLOBAL (CurrentActivity) & (CHECK_ABORT | CHECK_LOAD))
					&& (LOBYTE (GLOBAL (CurrentActivity)) == WON_LAST_BATTLE
							// if died for some reason
					|| GLOBAL_SIS (CrewEnlisted) == (COUNT)~0))
			{
				if (GET_GAME_STATE (KOHR_AH_KILLED_ALL))
					InitCommunication (BLACKURQ_CONVERSATION);
						// surrendered to Ur-Quan
				else if (GLOBAL (CurrentActivity) & CHECK_RESTART)
					GLOBAL (CurrentActivity) &= ~CHECK_RESTART;
				break;
			}
		} while (!(GLOBAL (CurrentActivity) & CHECK_ABORT));

		StopSound ();
		UninitGameClock ();
		UninitGameStructures ();
		ClearPlayerInputAll ();
	}
//	CloseJournal ();

	UninitGameKernel ();
	FreeMasterShipList ();
	FreeKernel ();

	MainExited = TRUE;

	(void) threadArg;  /* Satisfying compiler (unused parameter) */
	return 0;
}


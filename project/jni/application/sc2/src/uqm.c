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

#ifdef HAVE_UNISTD_H
#	include <unistd.h>
#endif

#ifdef HAVE_GETOPT_LONG
#	include <getopt.h>
#else
#	include "getopt/getopt.h"
#endif

#include <stdarg.h>
#include "libs/graphics/gfx_common.h"
#include "libs/graphics/cmap.h"
#include "libs/sound/sound.h"
#include "libs/input/input_common.h"
#include "libs/inplib.h"
#include "libs/tasklib.h"
#include "uqm/controls.h"
#include "uqm/battle.h"
		// For BATTLE_FRAME_RATE
#include "libs/file.h"
#include "types.h"
#include "port.h"
#include "libs/memlib.h"
#include "libs/platform.h"
#include "libs/log.h"
#include "options.h"
#include "uqmversion.h"
#include "uqm/comm.h"
#ifdef NETPLAY
#	include "libs/callback.h"
#	include "libs/alarm.h"
#	include "libs/net.h"
#	include "uqm/supermelee/netplay/netoptions.h"
#	include "uqm/supermelee/netplay/netplay.h"
#endif
#include "uqm/setup.h"
#include "uqm/starcon.h"


#if defined (GFXMODULE_SDL)
#	include SDL_INCLUDE(SDL.h)
			// Including this is actually necessary on OSX.
#endif

struct bool_option
{
	bool value;
	bool set;
};

struct int_option
{
	int value;
	bool set;
};

struct float_option
{
	float value;
	bool set;
};

struct options_struct
{
#define DECL_CONFIG_OPTION(type, name) \
	struct type##_option name

#define DECL_CONFIG_OPTION2(type, name, val1, val2) \
	struct { type val1; type val2; bool set; } name

	// Commandline-only options
	const char *logFile;
	enum {
		runMode_normal,
		runMode_usage,
		runMode_version,
	} runMode;

	const char *configDir;
	const char *contentDir;
	const char *addonDir;
	const char **addons;
	int numAddons;
	
	// Commandline and user config options
	DECL_CONFIG_OPTION(bool, opengl);
	DECL_CONFIG_OPTION2(int, resolution, width, height);
	DECL_CONFIG_OPTION(bool, fullscreen);
	DECL_CONFIG_OPTION(bool, scanlines);
	DECL_CONFIG_OPTION(int, scaler);
	DECL_CONFIG_OPTION(bool, showFps);
	DECL_CONFIG_OPTION(bool, keepAspectRatio);
	DECL_CONFIG_OPTION(float, gamma);
	DECL_CONFIG_OPTION(int, soundDriver);
	DECL_CONFIG_OPTION(int, soundQuality);
	DECL_CONFIG_OPTION(bool, use3doMusic);
	DECL_CONFIG_OPTION(bool, useRemixMusic);
	DECL_CONFIG_OPTION(int, whichCoarseScan);
	DECL_CONFIG_OPTION(int, whichMenu);
	DECL_CONFIG_OPTION(int, whichFonts);
	DECL_CONFIG_OPTION(int, whichIntro);
	DECL_CONFIG_OPTION(int, whichShield);
	DECL_CONFIG_OPTION(int, smoothScroll);
	DECL_CONFIG_OPTION(int, meleeScale);
	DECL_CONFIG_OPTION(bool, subtitles);
	DECL_CONFIG_OPTION(bool, stereoSFX);
	DECL_CONFIG_OPTION(float, musicVolumeScale);
	DECL_CONFIG_OPTION(float, sfxVolumeScale);
	DECL_CONFIG_OPTION(float, speechVolumeScale);
	DECL_CONFIG_OPTION(bool, safeMode);

#define INIT_CONFIG_OPTION(name, val) \
	{ val, false }

#define INIT_CONFIG_OPTION2(name, val1, val2) \
	{ val1, val2, false }
};

struct option_list_value
{
	const char *str;
	int value;
};

static const struct option_list_value scalerList[] = 
{
	{"bilinear", TFB_GFXFLAGS_SCALE_BILINEAR},
	{"biadapt",  TFB_GFXFLAGS_SCALE_BIADAPT},
	{"biadv",    TFB_GFXFLAGS_SCALE_BIADAPTADV},
	{"triscan",  TFB_GFXFLAGS_SCALE_TRISCAN},
	{"hq",       TFB_GFXFLAGS_SCALE_HQXX},
	{"none",     0},
	{"no",       0}, /* uqm.cfg value */
	{NULL, 0}
};

static const struct option_list_value meleeScaleList[] = 
{
	{"smooth",   TFB_SCALE_TRILINEAR},
	{"3do",      TFB_SCALE_TRILINEAR},
	{"step",     TFB_SCALE_STEP},
	{"pc",       TFB_SCALE_STEP},
	{"bilinear", TFB_SCALE_BILINEAR},
	{NULL, 0}
};

static const struct option_list_value audioDriverList[] = 
{
	{"openal",  audio_DRIVER_OPENAL},
	{"mixsdl",  audio_DRIVER_MIXSDL},
	{"none",    audio_DRIVER_NOSOUND},
	{"nosound", audio_DRIVER_NOSOUND},
	{NULL, 0}
};

static const struct option_list_value audioQualityList[] = 
{
	{"low",    audio_QUALITY_LOW},
	{"medium", audio_QUALITY_MEDIUM},
	{"high",   audio_QUALITY_HIGH},
	{NULL, 0}
};

static const struct option_list_value choiceList[] = 
{
	{"pc",  OPT_PC},
	{"3do", OPT_3DO},
	{NULL, 0}
};

static const struct option_list_value accelList[] = 
{
	{"mmx",    PLATFORM_MMX},
	{"sse",    PLATFORM_SSE},
	{"3dnow",  PLATFORM_3DNOW},
	{"none",   PLATFORM_C},
	{"detect", PLATFORM_NULL},
	{NULL, 0}
};

// Looks up the given string value in the given list and passes
// the associated int value back. returns true if value was found.
// The list is terminated by a NULL 'str' value.
static bool lookupOptionValue (const struct option_list_value *list,
		const char *strval, int *ret);

// Error message buffer used for when we cannot use logging facility yet
static char errBuffer[512];

static void saveError (const char *fmt, ...)
		PRINTF_FUNCTION(1, 2);

static int parseOptions (int argc, char *argv[],
		struct options_struct *options);
static void getUserConfigOptions (struct options_struct *options);
static void usage (FILE *out, const struct options_struct *defaultOptions);
static int parseIntOption (const char *str, int *result,
		const char *optName);
static int parseFloatOption (const char *str, float *f,
		const char *optName);
static void parseIntVolume (int intVol, float *vol);
static int InvalidArgument (const char *supplied, const char *opt_name);
static const char *choiceOptString (const struct int_option *option);
static const char *boolOptString (const struct bool_option *option);
static const char *boolNotOptString (const struct bool_option *option);

int
main (int argc, char *argv[])
{
	struct options_struct options = {
		/* .logFile = */            NULL,
		/* .runMode = */            runMode_normal,
		/* .configDir = */          NULL,
		/* .contentDir = */         NULL,
		/* .addonDir = */           NULL,
		/* .addons = */             NULL,
		/* .numAddons = */          0,

		INIT_CONFIG_OPTION(  opengl,            false ),
		INIT_CONFIG_OPTION2( resolution,        640, 480 ),
		INIT_CONFIG_OPTION(  fullscreen,        false ),
		INIT_CONFIG_OPTION(  scanlines,         false ),
		INIT_CONFIG_OPTION(  scaler,            0 ),
		INIT_CONFIG_OPTION(  showFps,           false ),
		INIT_CONFIG_OPTION(  keepAspectRatio,   false ),
		INIT_CONFIG_OPTION(  gamma,             0.0f ),
		INIT_CONFIG_OPTION(  soundDriver,       audio_DRIVER_MIXSDL ),
		INIT_CONFIG_OPTION(  soundQuality,      audio_QUALITY_MEDIUM ),
		INIT_CONFIG_OPTION(  use3doMusic,       true ),
		INIT_CONFIG_OPTION(  useRemixMusic,     false ),
		INIT_CONFIG_OPTION(  whichCoarseScan,   OPT_PC ),
		INIT_CONFIG_OPTION(  whichMenu,         OPT_PC ),
		INIT_CONFIG_OPTION(  whichFonts,        OPT_PC ),
		INIT_CONFIG_OPTION(  whichIntro,        OPT_PC ),
		INIT_CONFIG_OPTION(  whichShield,       OPT_PC ),
		INIT_CONFIG_OPTION(  smoothScroll,      OPT_PC ),
		INIT_CONFIG_OPTION(  meleeScale,        TFB_SCALE_TRILINEAR ),
		INIT_CONFIG_OPTION(  subtitles,         true ),
		INIT_CONFIG_OPTION(  stereoSFX,         false ),
		INIT_CONFIG_OPTION(  musicVolumeScale,  1.0f ),
		INIT_CONFIG_OPTION(  sfxVolumeScale,    1.0f ),
		INIT_CONFIG_OPTION(  speechVolumeScale, 1.0f ),
		INIT_CONFIG_OPTION(  safeMode,          false ),
	};
	struct options_struct defaults = options;
	int optionsResult;
	int gfxDriver;
	int gfxFlags;
	int i;

	// NOTE: we cannot use the logging facility yet because we may have to
	//   log to a file, and we'll only get the log file name after parsing
	//   the options.
	optionsResult = parseOptions (argc, argv, &options);

	log_init (15);

	if (options.logFile != NULL)
	{
		int i;
		freopen (options.logFile, "w", stderr);
#ifdef UNBUFFERED_LOGFILE
		setbuf (stderr, NULL);
#endif
		for (i = 0; i < argc; ++i)
			log_add (log_User, "argv[%d] = [%s]", i, argv[i]);
	}

	if (options.runMode == runMode_version)
	{
 		printf ("%d.%d.%d%s\n", UQM_MAJOR_VERSION, UQM_MINOR_VERSION,
				UQM_PATCH_VERSION, UQM_EXTRA_VERSION);
		log_showBox (false, false);
		return EXIT_SUCCESS;
	}
	
	log_add (log_User, "The Ur-Quan Masters v%d.%d.%d%s (compiled %s %s)\n"
	        "This software comes with ABSOLUTELY NO WARRANTY;\n"
			"for details see the included 'COPYING' file.\n",
			UQM_MAJOR_VERSION, UQM_MINOR_VERSION,
			UQM_PATCH_VERSION, UQM_EXTRA_VERSION,
			__DATE__, __TIME__);
#ifdef NETPLAY
	log_add (log_User, "Netplay protocol version %d.%d. Netplay opponent "
			"must have UQM %d.%d.%d or later.",
			NETPLAY_PROTOCOL_VERSION_MAJOR, NETPLAY_PROTOCOL_VERSION_MINOR,
			NETPLAY_MIN_UQM_VERSION_MAJOR, NETPLAY_MIN_UQM_VERSION_MINOR,
			NETPLAY_MIN_UQM_VERSION_PATCH);
#endif

	if (errBuffer[0] != '\0')
	{	// Have some saved error to log
		log_add (log_Error, "%s", errBuffer);
		errBuffer[0] = '\0';
	}

	if (options.runMode == runMode_usage)
	{
		usage (stdout, &defaults);
		log_showBox (true, false);
		return EXIT_SUCCESS;
	}

	if (optionsResult != EXIT_SUCCESS)
	{	// Options parsing failed. Oh, well.
		log_add (log_Fatal, "Run with -h to see the allowed arguments.");
		return optionsResult;
	}

	TFB_PreInit ();
	mem_init ();
	InitThreadSystem ();
	log_initThreads ();
	initIO ();
	prepareConfigDir (options.configDir);

	PlayerControls[0] = CONTROL_TEMPLATE_KB_1;
	PlayerControls[1] = CONTROL_TEMPLATE_JOY_1;

	// Fill in the options struct based on uqm.cfg
	if (!options.safeMode.value)
	{
		LoadResourceIndex (configDir, "uqm.cfg", "config.");
		getUserConfigOptions (&options);
	}

	{	/* remove old control template names */
		int i;

		for (i = 0; i < 6; ++i)
		{
			char cfgkey[64];

			snprintf(cfgkey, sizeof(cfgkey), "config.keys.%d.name", i + 1);
			cfgkey[sizeof(cfgkey) - 1] = '\0';

			res_Remove (cfgkey);
		}
	}

	/* TODO: Once threading is gone, these become local variables
	   again.  In the meantime, they must be global so that
	   initAudio (in StarCon2Main) can see them.  initAudio needed
	   to be moved there because calling AssignTask in the main
	   thread doesn't work */
	snddriver = options.soundDriver.value;
	soundflags = options.soundQuality.value;

	// Fill in global variables:
	opt3doMusic = options.use3doMusic.value;
	optRemixMusic = options.useRemixMusic.value;
	optWhichCoarseScan = options.whichCoarseScan.value;
	optWhichMenu = options.whichMenu.value;
	optWhichFonts = options.whichFonts.value;
	optWhichIntro = options.whichIntro.value;
	optWhichShield = options.whichShield.value;
	optSmoothScroll = options.smoothScroll.value;
	optMeleeScale = options.meleeScale.value;
	optKeepAspectRatio = options.keepAspectRatio.value;
	optSubtitles = options.subtitles.value;
	optStereoSFX = options.stereoSFX.value;
	musicVolumeScale = options.musicVolumeScale.value;
	sfxVolumeScale = options.sfxVolumeScale.value;
	speechVolumeScale = options.speechVolumeScale.value;
	optAddons = options.addons;

	prepareContentDir (options.contentDir, options.addonDir, argv[0]);
	prepareMeleeDir ();
	prepareSaveDir ();
	prepareShadowAddons (options.addons);
#if 0
	initTempDir ();
#endif

	InitTimeSystem ();
	InitTaskSystem ();

#ifdef NETPLAY
	Network_init ();
	Alarm_init ();
	Callback_init ();
	NetManager_init ();
#endif

	GraphicsLock = CreateMutex ("Graphics",
			SYNC_CLASS_TOPLEVEL | SYNC_CLASS_VIDEO);

	gfxDriver = options.opengl.value ?
			TFB_GFXDRIVER_SDL_OPENGL : TFB_GFXDRIVER_SDL_PURE;
	gfxFlags = options.scaler.value;
	if (options.fullscreen.value)
		gfxFlags |= TFB_GFXFLAGS_FULLSCREEN;
	if (options.scanlines.value)
		gfxFlags |= TFB_GFXFLAGS_SCANLINES;
	if (options.showFps.value)
		gfxFlags |= TFB_GFXFLAGS_SHOWFPS;
	TFB_InitGraphics (gfxDriver, gfxFlags, options.resolution.width,
			options.resolution.height);
	if (options.gamma.set)
		TFB_SetGamma (options.gamma.value);
	InitColorMaps ();
	init_communication ();
	/* TODO: Once threading is gone, restore initAudio here.
	   initAudio calls AssignTask, which currently blocks on
	   ProcessThreadLifecycles... */
	// initAudio (snddriver, soundflags);
	// Make sure that the compiler treats multidim arrays the way we expect
	assert (sizeof (int [NUM_TEMPLATES * NUM_KEYS]) ==
			sizeof (int [NUM_TEMPLATES][NUM_KEYS]));
	TFB_SetInputVectors (ImmediateInputState.menu, NUM_MENU_KEYS,
			(volatile int *)ImmediateInputState.key, NUM_TEMPLATES, NUM_KEYS);
	TFB_InitInput (TFB_INPUTDRIVER_SDL, 0);

	StartThread (Starcon2Main, NULL, 1024, "Starcon2Main");

	for (i = 0; i < 2000 && !MainExited; )
	{
		if (QuitPosted)
		{	/* Try to stop the main thread, but limited number of times */
			SignalStopMainThread ();
			++i;
		}
		else if (!GameActive)
		{	// Throttle down the main loop when game is inactive
			SleepThread (ONE_SECOND / 4);
		}

		TFB_ProcessEvents ();
		ProcessUtilityKeys ();
		ProcessThreadLifecycles ();
		TFB_FlushGraphics ();
	}

	/* Currently, we use atexit() callbacks everywhere, so we
	 *   cannot simply call unInitAudio() and the like, because other
	 *   tasks might still be using it */
	if (MainExited)
	{
		// Not yet: TFB_UninitInput ();
		unInitAudio ();
		uninit_communication ();
		UninitColorMaps ();
		// Not yet: TFB_UninitGraphics ();

#ifdef NETPLAY
		NetManager_uninit ();
		Alarm_uninit ();
		Network_uninit ();
#endif

		// Not yet: CleanupTaskSystem ();
		UnInitTimeSystem ();
#if 0
		unInitTempDir ();
#endif
		uninitIO ();
		UnInitThreadSystem ();
		mem_uninit ();
	}

	log_showBox (false, false);
	
	return EXIT_SUCCESS;
}

static void
saveErrorV (const char *fmt, va_list list)
{
	int len = strlen (errBuffer);
	int left = sizeof (errBuffer) - len;
	if (len > 0 && left > 0)
	{	// Already something there
		errBuffer[len] = '\n';
		++len;
		--left;
	}
	vsnprintf (errBuffer + len, left, fmt, list);
	errBuffer[sizeof (errBuffer) - 1] = '\0';
}

static void
saveError (const char *fmt, ...)
{
	va_list list;

	va_start (list, fmt);
	saveErrorV (fmt, list);
	va_end (list);
}


static bool
lookupOptionValue (const struct option_list_value *list,
		const char *strval, int *ret)
{
	if (!list)
		return false;

	// The list is terminated by a NULL 'str' value.
	while (list->str && strcmp (strval, list->str) != 0)
		++list;
	if (!list->str)
		return false;

	*ret = list->value;
	return true;
}

static void
getBoolConfigValue (struct bool_option *option, const char *config_val)
{
	if (option->set || !res_IsBoolean (config_val))
		return;

	option->value = res_GetBoolean (config_val);
	option->set = true;
}

static void
getBoolConfigValueXlat (struct int_option *option, const char *config_val,
		int true_val, int false_val)
{
	if (option->set || !res_IsBoolean (config_val))
		return;

	option->value = res_GetBoolean (config_val) ? true_val : false_val;
	option->set = true;
}

static void
getVolumeConfigValue (struct float_option *option, const char *config_val)
{
	if (option->set || !res_IsInteger (config_val))
		return;

	parseIntVolume (res_GetInteger (config_val), &option->value);
	option->set = true;
}

static bool
getListConfigValue (struct int_option *option, const char *config_val,
		const struct option_list_value *list)
{
	const char *strval;
	bool found;

	if (option->set || !res_IsString (config_val) || !list)
		return false;

	strval = res_GetString (config_val);
	found = lookupOptionValue (list, strval, &option->value);
	option->set = found;

	return found;
}

static void
getUserConfigOptions (struct options_struct *options)
{
	// Most of the user config options are only applied if they
	// have not already been set (i.e. on the commandline)

	if (res_IsInteger ("config.reswidth") && res_IsInteger ("config.resheight")
			&& !options->resolution.set)
	{
		options->resolution.width = res_GetInteger ("config.reswidth");
		options->resolution.height = res_GetInteger ("config.resheight");
		options->resolution.set = true;
	}

	if (res_IsBoolean ("config.alwaysgl") && !options->opengl.set)
	{	// config.alwaysgl is processed differently than others
		// Only set when it's 'true'
		if (res_GetBoolean ("config.alwaysgl"))
		{
			options->opengl.value = true;
			options->opengl.set = true;
		}
	}
	getBoolConfigValue (&options->opengl, "config.usegl");

	getListConfigValue (&options->scaler, "config.scaler", scalerList);

	getBoolConfigValue (&options->fullscreen, "config.fullscreen");
	getBoolConfigValue (&options->scanlines, "config.scanlines");
	getBoolConfigValue (&options->showFps, "config.showfps");
	getBoolConfigValue (&options->keepAspectRatio, "config.keepaspectratio");

	getBoolConfigValue (&options->subtitles, "config.subtitles");
	
	getBoolConfigValueXlat (&options->whichMenu, "config.textmenu",
			OPT_PC, OPT_3DO);
	getBoolConfigValueXlat (&options->whichFonts, "config.textgradients",
			OPT_PC, OPT_3DO);
	getBoolConfigValueXlat (&options->whichCoarseScan, "config.iconicscan",
			OPT_3DO, OPT_PC);
	getBoolConfigValueXlat (&options->smoothScroll, "config.smoothscroll",
			OPT_3DO, OPT_PC);
	getBoolConfigValueXlat (&options->whichShield, "config.pulseshield",
			OPT_3DO, OPT_PC);
	getBoolConfigValueXlat (&options->whichIntro, "config.3domovies",
			OPT_3DO, OPT_PC);

	getBoolConfigValue (&options->use3doMusic, "config.3domusic");
	getBoolConfigValue (&options->useRemixMusic, "config.remixmusic");

	getBoolConfigValueXlat (&options->meleeScale, "config.smoothmelee",
			TFB_SCALE_TRILINEAR, TFB_SCALE_STEP);

	if (getListConfigValue (&options->soundDriver, "config.audiodriver",
			audioDriverList))
	{
		// XXX: I don't know if we should turn speech off in this case.
		//   This affects which version of the alien script will be used.
		if (options->soundDriver.value == audio_DRIVER_NOSOUND)
			options->speechVolumeScale.value = 0.0f;
	}
	
	getListConfigValue (&options->soundQuality, "config.audioquality",
			audioQualityList);
	getBoolConfigValue (&options->stereoSFX, "config.positionalsfx");
	getVolumeConfigValue (&options->musicVolumeScale, "config.musicvol");
	getVolumeConfigValue (&options->sfxVolumeScale, "config.sfxvol");
	getVolumeConfigValue (&options->speechVolumeScale, "config.speechvol");
	
	if (res_IsInteger ("config.player1control"))
	{
		PlayerControls[0] = res_GetInteger ("config.player1control");
		/* This is an unsigned, so no < 0 check is necessary */
		if (PlayerControls[0] >= NUM_TEMPLATES)
		{
			log_add (log_Error, "Illegal control template '%d' for Player "
					"One.", PlayerControls[0]);
			PlayerControls[0] = CONTROL_TEMPLATE_KB_1;
		}
	}
	
	if (res_IsInteger ("config.player2control"))
	{
		/* This is an unsigned, so no < 0 check is necessary */
		PlayerControls[1] = res_GetInteger ("config.player2control");
		if (PlayerControls[1] >= NUM_TEMPLATES)
		{
			log_add (log_Error, "Illegal control template '%d' for Player "
					"Two.", PlayerControls[1]);
			PlayerControls[1] = CONTROL_TEMPLATE_JOY_1;
		}
	}
}

enum
{
	CSCAN_OPT = 1000,
	MENU_OPT,
	FONT_OPT,
	SHIELD_OPT,
	SCROLL_OPT,
	SOUND_OPT,
	STEREOSFX_OPT,
	ADDON_OPT,
	ADDONDIR_OPT,
	ACCEL_OPT,
	SAFEMODE_OPT,
#ifdef NETPLAY
	NETHOST1_OPT,
	NETPORT1_OPT,
	NETHOST2_OPT,
	NETPORT2_OPT,
	NETDELAY_OPT,
#endif
};

static const char *optString = "+r:foc:b:spC:n:?hM:S:T:m:q:ug:l:i:vwxk";
static struct option longOptions[] = 
{
	{"res", 1, NULL, 'r'},
	{"fullscreen", 0, NULL, 'f'},
	{"opengl", 0, NULL, 'o'},
	{"scale", 1, NULL, 'c'},
	{"meleezoom", 1, NULL, 'b'},
	{"scanlines", 0, NULL, 's'},
	{"fps", 0, NULL, 'p'},
	{"configdir", 1, NULL, 'C'},
	{"contentdir", 1, NULL, 'n'},
	{"help", 0, NULL, 'h'},
	{"musicvol", 1, NULL, 'M'},
	{"sfxvol", 1, NULL, 'S'},
	{"speechvol", 1, NULL, 'T'},
	{"audioquality", 1, NULL, 'q'},
	{"nosubtitles", 0, NULL, 'u'},
	{"gamma", 1, NULL, 'g'},
	{"logfile", 1, NULL, 'l'},
	{"intro", 1, NULL, 'i'},
	{"version", 0, NULL, 'v'},
	{"windowed", 0, NULL, 'w'},
	{"nogl", 0, NULL, 'x'},
	{"keepaspectratio", 0, NULL, 'k'},

	//  options with no short equivalent:
	{"cscan", 1, NULL, CSCAN_OPT},
	{"menu", 1, NULL, MENU_OPT},
	{"font", 1, NULL, FONT_OPT},
	{"shield", 1, NULL, SHIELD_OPT},
	{"scroll", 1, NULL, SCROLL_OPT},
	{"sound", 1, NULL, SOUND_OPT},
	{"stereosfx", 0, NULL, STEREOSFX_OPT},
	{"addon", 1, NULL, ADDON_OPT},
	{"addondir", 1, NULL, ADDONDIR_OPT},
	{"accel", 1, NULL, ACCEL_OPT},
	{"safe", 0, NULL, SAFEMODE_OPT},
#ifdef NETPLAY
	{"nethost1", 1, NULL, NETHOST1_OPT},
	{"netport1", 1, NULL, NETPORT1_OPT},
	{"nethost2", 1, NULL, NETHOST2_OPT},
	{"netport2", 1, NULL, NETPORT2_OPT},
	{"netdelay", 1, NULL, NETDELAY_OPT},
#endif
	{0, 0, 0, 0}
};

static inline void
setBoolOption (struct bool_option *option, bool value)
{
	option->value = value;
	option->set = true;
}

static bool
setFloatOption (struct float_option *option, const char *strval,
		const char *optName)
{
	if (parseFloatOption (strval, &option->value, optName) != 0)
		return false;
	option->set = true;
	return true;
}

// returns true is value was found and set successfully
static bool
setListOption (struct int_option *option, const char *strval,
		const struct option_list_value *list)
{
	bool found;

	if (!list)
		return false; // not found

	found = lookupOptionValue (list, strval, &option->value);
	option->set = found;

	return found;
}

static inline bool
setChoiceOption (struct int_option *option, const char *strval)
{
	return setListOption (option, strval, choiceList);
}

static bool
setVolumeOption (struct float_option *option, const char *strval,
		const char *optName)
{
	int intVol;
	
	if (parseIntOption (strval, &intVol, optName) != 0)
		return false;
	parseIntVolume (intVol, &option->value);
	option->set = true;
	return true;
}

static int
parseOptions (int argc, char *argv[], struct options_struct *options)
{
	int optionIndex;
	bool badArg = false;

	opterr = 0;

	options->addons = HMalloc (1 * sizeof (const char *));
	options->addons[0] = NULL;
	options->numAddons = 0;

	if (argc == 0)
	{
		saveError ("Error: Bad command line.");
		return EXIT_FAILURE;
	}

	while (!badArg)
	{
		int c;
		optionIndex = -1;
		c = getopt_long (argc, argv, optString, longOptions, &optionIndex);
		if (c == -1)
			break;

		switch (c)
		{
			case '?':
				if (optopt != '?')
				{
					saveError ("\nInvalid option or its argument");
					badArg = true;
					break;
				}
				// fall through
			case 'h':
				options->runMode = runMode_usage;
				return EXIT_SUCCESS;
			case 'v':
				options->runMode = runMode_version;
				return EXIT_SUCCESS;
			case 'r':
			{
				int width, height;
				if (sscanf (optarg, "%dx%d", &width, &height) != 2)
				{
					saveError ("Error: invalid argument specified "
							"as resolution.");
					badArg = true;
					break;
				}
				options->resolution.width = width;
				options->resolution.height = height;
				options->resolution.set = true;
				break;
			}
			case 'f':
				setBoolOption (&options->fullscreen, true);
				break;
			case 'w':
				setBoolOption (&options->fullscreen, false);
				break;
			case 'o':
				setBoolOption (&options->opengl, true);
				break;
			case 'x':
				setBoolOption (&options->opengl, false);
				break;
			case 'k':
				setBoolOption (&options->keepAspectRatio, true);
				break;
			case 'c':
				if (!setListOption (&options->scaler, optarg, scalerList))
				{
					InvalidArgument (optarg, "--scale or -c");
					badArg = true;
				}
				break;
			case 'b':
				if (!setListOption (&options->meleeScale, optarg,
						meleeScaleList))
				{
					InvalidArgument (optarg, "--meleezoom or -b");
					badArg = true;
				}
				break;
			case 's':
				setBoolOption (&options->scanlines, true);
				break;
			case 'p':
				setBoolOption (&options->showFps, true);
				break;
			case 'n':
				options->contentDir = optarg;
				break;
			case 'M':
				if (!setVolumeOption (&options->musicVolumeScale, optarg,
						"music volume"))
				{
					badArg = true;
				}
				break;
			case 'S':
				if (!setVolumeOption (&options->sfxVolumeScale, optarg,
						"sfx volume"))
				{
					badArg = true;
				}
				break;
			case 'T':
				if (!setVolumeOption (&options->speechVolumeScale, optarg,
						"speech volume"))
				{
					badArg = true;
				}
				break;
			case 'q':
				if (!setListOption (&options->soundQuality, optarg,
						audioQualityList))
				{
					InvalidArgument (optarg, "--audioquality or -q");
					badArg = true;
				}
				break;
			case 'u':
				setBoolOption (&options->subtitles, false);
				break;
			case 'g':
				if (!setFloatOption (&options->gamma, optarg,
						"gamma correction"))
				{
					badArg = true;
				}
				break;
			case 'l':
				options->logFile = optarg;
				break;
			case 'C':
				options->configDir = optarg;
				break;			
			case 'i':
				if (!setChoiceOption (&options->whichIntro, optarg))
				{
					InvalidArgument (optarg, "--intro or -i");
					badArg = true;
				}
				break;
			case CSCAN_OPT:
				if (!setChoiceOption (&options->whichCoarseScan, optarg))
				{
					InvalidArgument (optarg, "--cscan");
					badArg = true;
				}
				break;
			case MENU_OPT:
				if (!setChoiceOption (&options->whichMenu, optarg))
				{
					InvalidArgument (optarg, "--menu");
					badArg = true;
				}
				break;
			case FONT_OPT:
				if (!setChoiceOption (&options->whichFonts, optarg))
				{
					InvalidArgument (optarg, "--font");
					badArg = true;
				}
				break;
			case SHIELD_OPT:
				if (!setChoiceOption (&options->whichShield, optarg))
				{
					InvalidArgument (optarg, "--shield");
					badArg = true;
				}
				break;
			case SCROLL_OPT:
				if (!setChoiceOption (&options->smoothScroll, optarg))
				{
					InvalidArgument (optarg, "--scroll");
					badArg = true;
				}
				break;
			case SOUND_OPT:
				if (setListOption (&options->soundDriver, optarg,
						audioDriverList))
				{
					// XXX: I don't know if we should turn speech off in
					//   this case. This affects which version of the alien
					//   script will be used.
					if (options->soundDriver.value == audio_DRIVER_NOSOUND)
						options->speechVolumeScale.value = 0.0f;
				}
				else
				{
					InvalidArgument (optarg, "--sound");
					badArg = true;
				}
				break;
			case STEREOSFX_OPT:
				setBoolOption (&options->stereoSFX, true);
				break;
			case ADDON_OPT:
				options->numAddons++;
				options->addons = HRealloc ((void *) options->addons,
						(options->numAddons + 1) * sizeof (const char *));
				options->addons[options->numAddons - 1] = optarg;
				options->addons[options->numAddons] = NULL;
				break;
			case ADDONDIR_OPT:
				options->addonDir = optarg;
				break;
			case ACCEL_OPT:
			{
				int value;
				if (lookupOptionValue (accelList, optarg, &value))
				{
					force_platform = value;
				}
				else
				{
					InvalidArgument (optarg, "--accel");
					badArg = true;
				}
				break;
			}
	                case SAFEMODE_OPT:
				setBoolOption (&options->safeMode, true);
				break;
#ifdef NETPLAY
			case NETHOST1_OPT:
				netplayOptions.peer[0].isServer = false;
				netplayOptions.peer[0].host = optarg;
				break;
			case NETPORT1_OPT:
				netplayOptions.peer[0].port = optarg;
				break;
			case NETHOST2_OPT:
				netplayOptions.peer[1].isServer = false;
				netplayOptions.peer[1].host = optarg;
				break;
			case NETPORT2_OPT:
				netplayOptions.peer[1].port = optarg;
				break;
			case NETDELAY_OPT:
			{
				int temp;
				if (parseIntOption (optarg, &temp, "network input delay")
						== -1)
				{
					badArg = true;
					break;
				}
				netplayOptions.inputDelay = temp;

				if (netplayOptions.inputDelay > BATTLE_FRAME_RATE)
				{
					saveError ("Network input delay is absurdly large.");
					badArg = true;
				}
				break;
			}
#endif
			default:
				saveError ("Error: Unknown option '%s'",
						optionIndex < 0 ? "<unknown>" :
						longOptions[optionIndex].name);
				badArg = true;
				break;
		}
	}

	if (!badArg && optind != argc)
	{
		saveError ("\nError: Extra arguments found on the command line.");
		badArg = true;
	}

	return badArg ? EXIT_FAILURE : EXIT_SUCCESS;
}

static void
parseIntVolume (int intVol, float *vol)
{
	if (intVol < 0)
	{
		*vol = 0.0f;
		return;
	}

	if (intVol > 100)
	{
		*vol = 1.0f;
		return;
	}

	*vol = intVol / 100.0f;
	return;
}

static int
parseIntOption (const char *str, int *result, const char *optName)
{
	char *endPtr;
	int temp;

	if (str[0] == '\0')
	{
		saveError ("Error: Invalid value for '%s'.", optName);
		return -1;
	}
	temp = (int) strtol (str, &endPtr, 10);
	if (*endPtr != '\0')
	{
		saveError ("Error: Junk characters in argument '%s'.", optName);
		return -1;
	}

	*result = temp;
	return 0;
}

static int
parseFloatOption (const char *str, float *f, const char *optName)
{
	char *endPtr;
	float temp;

	if (str[0] == '\0')
	{
		saveError ("Error: Invalid value for '%s'.", optName);
		return -1;
	}
	temp = (float) strtod (str, &endPtr);
	if (*endPtr != '\0')
	{
		saveError ("Error: Junk characters in argument '%s'.", optName);
		return -1;
	}

	*f = temp;
	return 0;
}

static void
usage (FILE *out, const struct options_struct *defaults)
{
	FILE *old = log_setOutput (out);
	log_captureLines (LOG_CAPTURE_ALL);
	
	log_add (log_User, "Options:");
	log_add (log_User, "  -r, --res=WIDTHxHEIGHT (default 640x480, bigger "
			"works only with --opengl)");
	log_add (log_User, "  -f, --fullscreen (default %s)",
			boolOptString (&defaults->fullscreen));
	log_add (log_User, "  -w, --windowed (default %s)",
			boolNotOptString (&defaults->fullscreen));
	log_add (log_User, "  -o, --opengl (default %s)",
			boolOptString (&defaults->opengl));
	log_add (log_User, "  -x, --nogl (default %s)",
			boolNotOptString (&defaults->opengl));
	log_add (log_User, "  -k, --keepaspectratio (default %s)",
			boolOptString (&defaults->keepAspectRatio));
	log_add (log_User, "  -c, --scale=MODE (bilinear, biadapt, biadv, "
			"triscan, hq or none (default) )");
	log_add (log_User, "  -b, --meleezoom=MODE (step, aka pc, or smooth, "
			"aka 3do; default is 3do)");
	log_add (log_User, "  -s, --scanlines (default %s)",
			boolOptString (&defaults->scanlines));
	log_add (log_User, "  -p, --fps (default %s)",
			boolOptString (&defaults->showFps));
	log_add (log_User, "  -g, --gamma=CORRECTIONVALUE (default 1.0, which "
			"causes no change)");
	log_add (log_User, "  -C, --configdir=CONFIGDIR");
	log_add (log_User, "  -n, --contentdir=CONTENTDIR");
	log_add (log_User, "  -M, --musicvol=VOLUME (0-100, default 100)");
	log_add (log_User, "  -S, --sfxvol=VOLUME (0-100, default 100)");
	log_add (log_User, "  -T, --speechvol=VOLUME (0-100, default 100)");
	log_add (log_User, "  -q, --audioquality=QUALITY (high, medium or low, "
			"default medium)");
	log_add (log_User, "  -u, --nosubtitles");
	log_add (log_User, "  -l, --logfile=FILE (sends console output to "
			"logfile FILE)");
	log_add (log_User, "  --addon ADDON (using a specific addon; "
			"may be specified multiple times)");
	log_add (log_User, "  --addondir=ADDONDIR (directory where addons "
			"reside)");
	log_add (log_User, "  --sound=DRIVER (openal, mixsdl, none; default "
			"mixsdl)");
	log_add (log_User, "  --stereosfx (enables positional sound effects, "
			"currently only for openal)");
	log_add (log_User, "  --safe (start in safe mode)");
#ifdef NETPLAY
	log_add (log_User, "  --nethostN=HOSTNAME (server to connect to for "
			"player N (1=bottom, 2=top)");
	log_add (log_User, "  --netportN=PORT (port to connect to/listen on for "
			"player N (1=bottom, 2=top)");
	log_add (log_User, "  --netdelay=FRAMES (number of frames to "
			"buffer/delay network input for");
#endif
	log_add (log_User, "The following options can take either '3do' or 'pc' "
			"as an option:");
	log_add (log_User, "  -i, --intro : Intro/ending version (default %s)",
			choiceOptString (&defaults->whichIntro));
	log_add (log_User, "  --cscan     : coarse-scan display, pc=text, "
			"3do=hieroglyphs (default %s)",
			choiceOptString (&defaults->whichCoarseScan));
	log_add (log_User, "  --menu      : menu type, pc=text, 3do=graphical "
			"(default %s)", choiceOptString (&defaults->whichMenu));
	log_add (log_User, "  --font      : font types and colors (default %s)",
			choiceOptString (&defaults->whichFonts));
	log_add (log_User, "  --shield    : slave shield type; pc=static, "
			"3do=throbbing (default %s)",
			choiceOptString (&defaults->whichShield));
	log_add (log_User, "  --scroll    : ff/frev during comm.  pc=per-page, "
			"3do=smooth (default %s)",
			choiceOptString (&defaults->smoothScroll));
	log_setOutput (old);
}

static int
InvalidArgument (const char *supplied, const char *opt_name)
{
	saveError ("Invalid argument '%s' to option %s.", supplied, opt_name);
	return EXIT_FAILURE;
}

static const char *
choiceOptString (const struct int_option *option)
{
	switch (option->value)
	{
		case OPT_3DO:
			return "3do";
		case OPT_PC:
			return "pc";
		default:  /* 0 */
			return "none";
	}
}

static const char *
boolOptString (const struct bool_option *option)
{
	return option->value ? "on" : "off";
}

static const char *
boolNotOptString (const struct bool_option *option)
{
	return option->value ? "off" : "on";
}

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <errno.h>
#include <SDL/SDL.h>
#include <SDL/SDL_screenkeyboard.h>
#include <SDL/SDL_android.h>
#include <android/log.h>

#include "gfx.h"

extern int android_main( int argc, char *argv[], char *envp[] );

static void retryLaunchWithDifferentPort(void);
static void showError(void);
static void setupEnv(void);
static char port[16] = ":0";

int main( int argc, char* argv[] )
{
	int i;
	char screenres[128] = "640x480x24";
	char clientcmd[PATH_MAX] = "";
	char * cmd = "";
	char fontpath[PATH_MAX] = "";
	char* args[64] = {
		"XSDL",
		port,
		"-nolock",
		"-noreset",
		"-nopn",
		"-nolisten",
		"unix",
		"-fp",
		fontpath,
		"-screen",
		screenres,
	};
	int argnum = 11;
	char * envp[] = { NULL };
	int printHelp = 1;
	int screenResOverride = 0;
	int screenButtons = 0;
	int warnDiskSpaceMb = 0;
	int builtinKeyboard = 0;
	
	int resolutionW = atoi(getenv("DISPLAY_RESOLUTION_WIDTH"));
	int resolutionH = atoi(getenv("DISPLAY_RESOLUTION_HEIGHT"));
	int displayW = atoi(getenv("DISPLAY_WIDTH_MM"));
	int displayH = atoi(getenv("DISPLAY_HEIGHT_MM"));

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "Actual video resolution %d/%dx%d/%d", resolutionW, displayW, resolutionH, displayH);
	setupEnv();

	SDL_ANDROID_SetScreenKeyboardShown(0);

	XSDL_initSDL();

	while( argc > 1 )
	{
		if( argv[1][0] == ':')
		{
			strcpy(port, argv[1]);
		}
		else if( strcmp(argv[1], "-nohelp") == 0 )
		{
			printHelp = 0;
		}
		else if( strcmp(argv[1], "-screen") == 0 )
		{
			screenResOverride = 1;
			argc--;
			argv++;
			strcpy(screenres, argv[1]);
		}
		else if( strcmp(argv[1], "-screenbuttons") == 0 )
		{
			screenButtons = 1;
		}
		else if( strcmp(argv[1], "-warndiskspacemb") == 0 && argc > 2 )
		{
			warnDiskSpaceMb = atoi(argv[2]);
			argc--;
			argv++;
		}
		else
		{
			args[argnum] = argv[1];
			argnum++;
		}
		argc--;
		argv++;
	}

	if (getenv("SDL_RESTART_PARAMS") && getenv("SDL_RESTART_PARAMS")[0])
		strcpy(port, getenv("SDL_RESTART_PARAMS"));

	snprintf(fontpath, sizeof(fontpath),
						"%s/img/usr/share/fonts/X11/misc,"
						"%s/img/usr/share/fonts/X11/Type1,"
						"%s/img/usr/share/fonts/X11/100dpi,"
						"%s/img/usr/share/fonts/X11/75dpi,"
						"%s/img/usr/share/fonts/X11/cyrillic,"
						"%s/usr/share/fonts",
						getenv("SECURE_STORAGE_DIR"),
						getenv("SECURE_STORAGE_DIR"),
						getenv("SECURE_STORAGE_DIR"),
						getenv("SECURE_STORAGE_DIR"),
						getenv("SECURE_STORAGE_DIR"),
						getenv("SECURE_STORAGE_DIR") );

	XSDL_unpackFiles(warnDiskSpaceMb);

	if( !screenResOverride )
	{
		XSDL_showConfigMenu(&resolutionW, &displayW, &resolutionH, &displayH, &builtinKeyboard, &screenButtons);
		sprintf( screenres, "%d/%dx%d/%dx%d", resolutionW, displayW, resolutionH, displayH, SDL_GetVideoInfo()->vfmt->BitsPerPixel );
	}

	XSDL_generateBackground( port, printHelp, resolutionW, resolutionH );

	XSDL_deinitSDL();

	if( printHelp )
	{
		sprintf( clientcmd, "%s/usr/bin/xhost + ; %s/usr/bin/xli -onroot -center %s/background.bmp",
			getenv("SECURE_STORAGE_DIR"), getenv("SECURE_STORAGE_DIR"), getenv("UNSECURE_STORAGE_DIR") );
		args[argnum] = "-exec";
		args[argnum+1] = clientcmd;
		argnum += 2;
	}

	SDL_ANDROID_SetScreenKeyboardShown( 1 );

	if( screenButtons && !SDL_ANDROID_GetScreenKeyboardRedefinedByUser() )
	{
		// Button coords are handled inside xserver/hw/kdrive/sdl/sdl.c
		SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, 1);
	}
	else
	{
		SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, 0);
		SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, 0);
		SDL_ANDROID_SetScreenKeyboardButtonShown(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, 0);
	}

	{
		char s[16];
		sprintf(s, "%d", builtinKeyboard);
		setenv("XSDL_BUILTIN_KEYBOARD", s, 1);
	}

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "XSDL video resolution %s, args:", screenres);
	for( i = 0; i < argnum; i++ )
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "> %s", args[i]);

	// We should never quit. If that happens, then the server did not start - try with different port number.
	atexit( &retryLaunchWithDifferentPort );

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "XSDL chdir to: %s", getenv("SECURE_STORAGE_DIR"));
	chdir( getenv("SECURE_STORAGE_DIR") ); // Megahack: change /proc/self/cwd to the X.org data dir, and use /proc/self/cwd path in libX11

	android_main( argnum, args, envp ); // Should never exit on success, if we want to terminate we kill ourselves

	return 0;
}

void setupEnv(void)
{
	uid_t uid = geteuid();
	struct passwd * pwd;
	char buf[PATH_MAX];
	errno = 0;
	pwd = getpwuid(uid);
	if( !pwd )
	{
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "Cannot determine user name for ID %d: %s", uid, strerror(errno));
		return;
	}
	sprintf( buf, "%d", uid );
	__android_log_print(ANDROID_LOG_INFO, "XSDL", "User %s ID %s", pwd->pw_name, buf);
	setenv("USER_ID", buf, 1);
	setenv("USER", pwd->pw_name, 1);
	sprintf( buf, "%s/usr/share/X11/XErrorDB", getenv("SECURE_STORAGE_DIR") );
	setenv("XERRORDB", buf, 1);
	sprintf( buf, "%s/usr/lib/X11/XKeysymDB", getenv("SECURE_STORAGE_DIR") );
	setenv("XKEYSYMDB", buf, 1);
	sprintf( buf, "%s/usr/share/X11/locale", getenv("SECURE_STORAGE_DIR") );
	setenv("XLOCALEDIR", buf, 1);
	sprintf( buf, "%s/usr/share/X11/locale", getenv("SECURE_STORAGE_DIR") );
}

void retryLaunchWithDifferentPort(void)
{
	int portNum = atoi(port + 1);
	if (portNum > 10)
	{
		// Server was ultimately unable to start - show error and exit
		XSDL_initSDL();
		XSDL_showServerLaunchErrorMessage();
		XSDL_deinitSDL();
		return;
	}
	sprintf(port, ":%d", portNum + 1);
	__android_log_print(ANDROID_LOG_INFO, "XSDL", "XSDL launch failed, retrying with new display number %s", port);
	SDL_ANDROID_RestartMyself(port);
}

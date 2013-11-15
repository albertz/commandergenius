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
#include <android/log.h>

#include "gfx.h"

extern int android_main( int argc, char *argv[], char *envp[] );

static void setupEnv(void);
static void showError(void);

int main( int argc, char* argv[] )
{
	int i;
	char screenres[128] = "640x480x24";
	char clientcmd[PATH_MAX*3] = "";
	char port[16] = ":1111";
	char * cmd = "";
	char* args[] = {
		"XSDL",
		port,
		"-nolock",
		"-noreset",
		"-screen",
		screenres,
		"-exec",
		clientcmd
	};
	enum { ARGNUM = 8 };
	char * envp[] = { NULL };
	int printHelp = 1;
	
	int resolutionW = atoi(getenv("DISPLAY_RESOLUTION_WIDTH"));
	int resolutionH = atoi(getenv("DISPLAY_RESOLUTION_HEIGHT"));
	int displayW = atoi(getenv("DISPLAY_WIDTH_MM"));
	int displayH = atoi(getenv("DISPLAY_HEIGHT_MM"));

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "Actual video resolution %d/%dx%d/%d", resolutionW, displayW, resolutionH, displayH);
	setupEnv();

	XSDL_initSDL();
	
	XSDL_unpackFiles();

	XSDL_showConfigMenu(&resolutionW, &displayW, &resolutionH, &displayH);

	for(i = 0; i < 1024; i++)
	{
		int s = socket(AF_INET, SOCK_STREAM, 0);
		if( s >= 0 )
		{
			struct sockaddr_in addr;

			memset(&addr, 0, sizeof(addr));

			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = INADDR_ANY;
			addr.sin_port = htons(6000 + i);

			if( bind (s, (struct sockaddr *) &addr, sizeof(addr) ) != 0 )
			{
				__android_log_print(ANDROID_LOG_INFO, "XSDL", "TCP port %d already used, trying next one: %s", 6000 + i, strerror(errno));
				close(s);
				continue;
			}
			close(s);
		}

		// Cannot create socket with non-existing path, but Xserver code somehow does that
		/*
		s = socket(AF_UNIX, SOCK_STREAM, 0);
		if( s >= 0 )
		{
			struct sockaddr_un addr;

			memset(&addr, 0, sizeof(addr));

			addr.sun_family = AF_UNIX;
			sprintf(addr.sun_path, "/tmp/.X11-unix/X%d", i);
			if( bind(s, (struct sockaddr *) &addr, strlen(addr.sun_path) + sizeof(addr.sun_family)) != 0 )
			{
				__android_log_print(ANDROID_LOG_INFO, "XSDL", "UNIX path %s already used, trying next one: %s", addr.sun_path, strerror(errno));
				close(s);
				continue;
			}
			close(s);
		}
		*/
		FILE * ff = fopen("/proc/net/unix", "rb");
		if( ff )
		{
			char buf[512], name[512];
			int found = 0;
			sprintf(name, "/tmp/.X11-unix/X%d", i);
			while( fgets(buf, sizeof(buf), ff) )
			{
				if( strstr(buf, name) != NULL )
				{
					__android_log_print(ANDROID_LOG_INFO, "XSDL", "UNIX path %s already used, trying next one", name);
					found = 1;
					break;
				}
			}
			fclose(ff);
			if( found )
				continue;
		}

		sprintf( port, ":%d", i );
		break;
	}

	if( argc > 1 && strcmp(argv[1], "-nohelp") == 0 )
	{
		printHelp = 0;
		argc--;
		argv++;
	}

	XSDL_generateBackground(port, printHelp);

	XSDL_deinitSDL();

	sprintf( screenres, "%d/%dx%d/%dx%d", resolutionW, displayW, resolutionH, displayH, SDL_GetVideoInfo()->vfmt->BitsPerPixel );

	if( argc > 1 )
	{
		for( ; argc > 1; argc--, argv++ )
		{
			strcat(clientcmd, " ");
			strcat(clientcmd, argv[1]);
		}
		strcat(clientcmd, " 2>&1");
	}
	else
	{
		sprintf( clientcmd, "%s/usr/bin/xhost + ; %s/usr/bin/xli -onroot -fillscreen background.bmp ;",
			getenv("SECURE_STORAGE_DIR"), getenv("SECURE_STORAGE_DIR") );
	}

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "XSDL video resolution %s, args:", screenres);
	for( i = 0; i < ARGNUM; i++ )
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "> %s", args[i]);

	// We should never quit. If that happens, then the server did not start - show error.
	atexit(&showError);
	return android_main( ARGNUM, args, envp );
}

void setupEnv(void)
{
	uid_t uid = geteuid();
	struct passwd * pwd;
	char buf[32];
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
}

void showError(void)
{
	XSDL_initSDL();
	XSDL_showServerLaunchErrorMessage();
	XSDL_deinitSDL();
}

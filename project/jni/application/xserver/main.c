#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <SDL/SDL.h>
#include <android/log.h>

#include "gfx.h"

extern int android_main( int argc, char *argv[], char *envp[] );

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

	XSDL_initSDL();
	
	XSDL_unpackFiles();

	XSDL_showConfigMenu(&resolutionW, &displayW, &resolutionH, &displayH);

	int s = socket(AF_INET, SOCK_STREAM, 0);
	if( s >= 0 )
	{
		for(i = 0; i < 1024; i++)
		{
			struct sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = INADDR_ANY;
			addr.sin_port = htons(6000 + i);

			if( bind (s, (struct sockaddr *) &addr, sizeof(addr) ) < 0 )
				continue;
			sprintf( port, ":%d", i );
		}
		close(s);
	}

	if( argc > 1 && strcmp(argv[1], "-nohelp") == 0 )
	{
		printHelp = 0;
		argc--;
		argv++;
	}

	XSDL_generateBackground(port, printHelp);

	XSDL_deinitSDL();

	sprintf( screenres, "%d/%dx%d/%dx%d", resolutionW, displayW, resolutionH, displayH, 24 );

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

	return android_main( ARGNUM, args, envp );
}

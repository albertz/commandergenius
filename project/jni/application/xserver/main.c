#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <SDL/SDL.h>
#include <android/log.h>

#include "gfx.h"

extern int android_main( int argc, char *argv[], char *envp[] );

int main( int argc, char* argv[] )
{
	char screenres[128] = "640x480x24";
	char clientcmd[PATH_MAX*2] = "xhost +";
	char * cmd = "";
	char* args[] = {
		"XSDL",
		":1111",
		"-nolock",
		"-noreset",
		"-screen",
		screenres,
		"-exec",
		clientcmd
	};
	char * envp[] = { NULL };
	
	int resolutionW = atoi(getenv("DISPLAY_RESOLUTION_WIDTH"));
	int resolutionH = atoi(getenv("DISPLAY_RESOLUTION_HEIGHT"));
	int displayW = atoi(getenv("DISPLAY_WIDTH_MM"));
	int displayH = atoi(getenv("DISPLAY_HEIGHT_MM"));

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "Actual video resolution %d/%dx%d/%d", resolutionW, displayW, resolutionH, displayH);

	XSDL_initSDL();
	
	XSDL_unpackFiles();

	XSDL_showConfigMenu(&resolutionW, &displayW, &resolutionH, &displayH);

	XSDL_deinitSDL();

	sprintf( screenres, "%d/%dx%d/%dx%d", resolutionW, displayW, resolutionH, displayH, 24 );

	if( argc >= 2 )
		cmd = argv[2];
	sprintf( clientcmd, "%s/usr/bin/xhost + ; %s",
		getenv("SECURE_STORAGE_DIR"), cmd );

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "XSDL video resolution %s", screenres);

	return android_main( 8, args, envp );
}

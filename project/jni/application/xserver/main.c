#include <stdlib.h>
#include <SDL/SDL.h>

extern int android_main(int argc, char *argv[], char *envp[]);

int main(int argc, char* argv[])
{
	char screenres[64] = "640x480x24";
	char* args[] = {
		"XSDL",
		":1111",
		"-3button",
		"-screen",
		screenres
	};
	char * envp[] = { NULL };
	
	sprintf("%sx%sx%d", getenv("DISPLAY_RESOLUTION_WIDTH"), getenv("DISPLAY_RESOLUTION_HEIGHT"), 24);
	
	return android_main(5, args, envp);
}

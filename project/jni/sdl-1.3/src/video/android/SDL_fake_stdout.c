/*
Writing to stdout will crash your program on some particular Android tablets.
Although this is the device bug I've put a workaround here,
it is heavily dependent on NDK internals and is not portable in any way.
*/

#ifndef _SDL_fake_stdout_h
#define _SDL_fake_stdout_h
#include <stdio.h>

extern FILE __SDL_fake_stdout[];

FILE __SDL_fake_stdout[3];

void SDL_ANDROID_initFakeStdout()
{
	FILE * ff = NULL;
	__SDL_fake_stdout[0] = * fopen("/dev/null", "r");
	__SDL_fake_stdout[1] = * fopen("/dev/null", "w");
	__SDL_fake_stdout[2] = * fopen("/dev/null", "w");
}

#endif

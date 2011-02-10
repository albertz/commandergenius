/*
Writing to stdout will crash your program on some particular Android tablets.
Although this is the device bug I've put a workaround here,
it is heavily dependent on NDK internals and is not portable in any way.
*/

#include <stdio.h>
#include <android/log.h>

extern FILE __SDL_fake_stdout[];
extern int __SDL_android_printf(const char * fmt, ...);

FILE __SDL_fake_stdout[3];

extern void SDL_ANDROID_initFakeStdout()
{
	FILE * ff = NULL;
	__SDL_fake_stdout[0] = * fopen("/dev/null", "r");
	__SDL_fake_stdout[1] = * fopen("/dev/null", "w");
	__SDL_fake_stdout[2] = * fopen("/dev/null", "w");
}

int __SDL_android_printf(const char * fmt, ...)
{
	int return_value;
	char buff[1024];
	va_list ap;
	va_start(ap, fmt);
	/*
	int characters = vfprintf(stdout, fmt, ap); // get buffer size
	if(characters<0) return;
	char* buff = new char[characters+1];
	return_value = vsprintf(buff, fmt, ap);
	*/
	return_value = vsnprintf(buff, sizeof(buff), fmt, ap);
	va_end(ap);
	__android_log_print(ANDROID_LOG_INFO, "libSDL", buff);
	//delete buff;
	return return_value;
}

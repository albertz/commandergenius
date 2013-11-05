#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <android/log.h>

#include "gfx.h"

static TTF_Font* sFont;
static int unpackProgressMb;
static int unpackProgressMbTotal = 1;
static int unpackFinished = 0;

static void renderString(const char *c, int x, int y);
static void renderStringColor(const char *c, int x, int y, int r, int g, int b);

static void * unpackFilesThread(void * unused);
static void showErrorMessage(const char *msg);

void * unpackFilesThread(void * unused)
{
	char fname[PATH_MAX*2];
	strcpy( fname, getenv("SECURE_STORAGE_DIR") );
	strcat( fname, "/usr/bin/xkbcomp" );
	struct stat st;
	if( stat( fname, &st ) == 0 )
	{
		unpackFinished = 1;
		return 1;
	}

	strcpy( fname, getenv("SECURE_STORAGE_DIR") );
	strcat( fname, "/busybox" );
	FILE * ff = fopen("busybox", "rb");
	FILE * fo = fopen(fname, "wb");
	if( !ff || !fo )
	{
		unpackFinished = 1;
		return 0;
	}

	for(;;)
	{
		char buf[2048];
		int cnt = fread( buf, 1, sizeof(buf), ff );
		if( cnt < 0 )
		{
			unpackFinished = 1;
			return 1;
		}
		fwrite( buf, 1, cnt, fo );
		if( cnt < sizeof(buf) )
			break;
	}

	fclose(ff);
	fclose(fo);

	if( chmod(fname, 0755) != 0 )
	{
		unpackFinished = 1;
		return 0;
	}

	if( stat( "data.tar.gz", &st ) == 0 )
		unpackProgressMbTotal = st.st_size / 1024 / 1024;
	else
		unpackProgressMbTotal = 1;

	unpackProgressMb = 0;

	ff = fopen("data.tar.gz", "rb");
	strcat(fname, " tar xz -C ");
	strcat(fname, getenv("SECURE_STORAGE_DIR"));
	fo = popen(fname, "w");
	if( !ff || !fo )
	{
		unpackFinished = 1;
		return 0;
	}

	int unpackProgressKb = 0;
	for(;;)
	{
		char buf[1024 * 8];
		int cnt = fread( buf, 1, sizeof(buf), ff );
		if( cnt < 0 )
		{
			unpackFinished = 1;
			return 1;
		}
		fwrite( buf, 1, cnt, fo );
		if( cnt < sizeof(buf) )
			break;
		unpackProgressKb += 8;
		if( unpackProgressKb >= 1024 )
		{
			unpackProgressKb = 0;
			unpackProgressMb++;
		}
	}

	fclose(ff);
	if( pclose(fo) != 0 )
	{
		unpackFinished = 1;
		return 0;
	}

	remove("data.tar.gz");

	unpackFinished = 1;
	return 1;
}

void XSDL_unpackFiles()
{
	pthread_t thread_id;
	void * status;
	pthread_create(&thread_id, NULL, &unpackFilesThread, NULL);

	while (!unpackFinished)
	{
		SDL_Delay(400);
		SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
		char s[128];
		sprintf(s, "Unpacking data: %d/%d Mb, %d%%", unpackProgressMb, unpackProgressMbTotal, unpackProgressMb * 100 / unpackProgressMbTotal);
		renderString(s, VID_X/2, VID_Y/3);
		renderString("You may put this app to background while it's unpacking", VID_X/2, VID_Y*2/3);
		SDL_Flip(SDL_GetVideoSurface());
	}

	pthread_join(thread_id, &status);
	if( status == 0 )
	{
		showErrorMessage("Cannot unpack data files, please reinstall the app");
		exit(1);
	}
}

void XSDL_showConfigMenu(int * resolutionW, int * displayW, int * resolutionH, int * displayH)
{
	int x, y, i, ii;
	SDL_Event event;
	int res = -1, dpi = -1;
	char native[32] = "0x0";
	float dpiScale = 1.0f;

	const char * resStr[] = {
		native, "1920x1080", "1280x960", "1280x720",
		"1024x768", "800x600", "800x480", "640x480"
	};
	const int resVal[][2] = {
		{*resolutionW, *resolutionH}, {1920,1080}, {1280,960}, {1280,720},
		{1024,768}, {800,600}, {800,480}, {640,480}
	};

	const char * fontsStr[] = {
		"x2.5", "x2", "x1.7", "x1.5",
		"x1.3", "x1", "x0.9", "x0.8",
		"x0.7", "x0.6", "x0.5", "x0.4"
	};
	const float fontsVal[] = {
		2.5f, 2.0f, 1.7f, 1.5f,
		1.3f, 1.0f, 0.9f, 0.8f,
		0.7f, 0.6f, 0.5f, 0.4f
	};

	sprintf(native, "%dx%d native", resVal[0][0], resVal[0][1]);

	while ( res < 0 )
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_UNDO)
					return;
				break;
				case SDL_MOUSEBUTTONUP:
				{
					SDL_GetMouseState(&x, &y);
					i = (y / (VID_Y/2));
					ii = (x / (VID_X/4));
					res = i * 4 + ii;
				}
				break;
			}
		}
		SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
		renderString("Select display resolution", VID_X/2, VID_Y/2);
		for(i = 0; i < 2; i++)
		for(ii = 0; ii < 4; ii++)
			renderString(resStr[i*4+ii], VID_X/8 + (ii*VID_X/4), VID_Y/4 + (i*VID_Y/2));
		SDL_GetMouseState(&x, &y);
		renderString("X", x, y);
		SDL_Delay(200);
		SDL_Flip(SDL_GetVideoSurface());
	}
	dpiScale = (float)resVal[res][0] / (float)*resolutionW;
	*resolutionW = resVal[res][0];
	*resolutionH = resVal[res][1];
	while ( dpi < 0 )
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_UNDO)
					return;
				break;
				case SDL_MOUSEBUTTONUP:
				{
					SDL_GetMouseState(&x, &y);
					i = (y / (VID_Y/3));
					ii = (x / (VID_X/4));
					dpi = i * 4 + ii;
				}
				break;
			}
		}
		SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
		renderString("Select font scale", VID_X/2, VID_Y/3);
		for(i = 0; i < 3; i++)
		for(ii = 0; ii < 4; ii++)
			renderString(fontsStr[i*4+ii], VID_X/8 + (ii*VID_X/4), VID_Y/6 + (i*VID_Y/3));
		SDL_GetMouseState(&x, &y);
		renderString("X", x, y);
		SDL_Delay(200);
		SDL_Flip(SDL_GetVideoSurface());
	}
	*displayW = *displayW * (dpiScale / fontsVal[dpi]);
	*displayH = *displayH * (dpiScale / fontsVal[dpi]);
}

void showErrorMessage(const char *msg)
{
	SDL_Event event;
	SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
	renderString(msg, VID_X/2, VID_Y/2);
	SDL_Flip(SDL_GetVideoSurface());
	while (1)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_UNDO)
					return;
				break;
			}
		}
	}
}

void XSDL_initSDL()
{
	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetVideoMode(VID_X, VID_Y, 24, SDL_SWSURFACE);
	TTF_Init();
	sFont = TTF_OpenFont("DroidSansMono.ttf", 10);
	if (!sFont)
	{
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "Error: cannot open font file, please reinstall the app");
		exit(1);
	}
}

void XSDL_deinitSDL()
{
	TTF_CloseFont(sFont);
	sFont = NULL;
	TTF_Quit();
	// Do NOT call SDL_Quit(), it crashes!
}

void renderStringColor(const char *c, int x, int y, int r, int g, int b)
{
	SDL_Color fColor = {r, g, b};
	SDL_Rect fontRect = {0, 0, 0, 0};
	SDL_Surface* fontSurface = TTF_RenderUTF8_Solid(sFont, c, fColor);
	fontRect.w = fontSurface->w;
	fontRect.h = fontSurface->h;
	fontRect.x = x - fontRect.w / 2;
	fontRect.y = y - fontRect.h / 2;
	SDL_BlitSurface(fontSurface, NULL, SDL_GetVideoSurface(), &fontRect);
	SDL_FreeSurface(fontSurface);
}

void renderString(const char *c, int x, int y)
{
	renderStringColor(c, x, y, 255, 255, 255);
}


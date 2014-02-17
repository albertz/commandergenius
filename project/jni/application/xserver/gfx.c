#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
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
static void renderStringColor(const char *c, int x, int y, int r, int g, int b, SDL_Surface * surf);
static void renderStringScaled(const char *c, int size, int x, int y, int r, int g, int b, SDL_Surface * surf);

static void * unpackFilesThread(void * unused);
static void showErrorMessage(const char *msg);

void * unpackFilesThread(void * unused)
{
	char fname[PATH_MAX*2];
	strcpy( fname, getenv("SECURE_STORAGE_DIR") );
	strcat( fname, "/usr/lib/xorg/protocol.txt" );
	struct stat st;
	if( stat( fname, &st ) == 0 )
	{
		unpackFinished = 1;
		return (void *)1;
	}

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "Unpacking data");

	if( stat( "data.tar.gz", &st ) == 0 )
		unpackProgressMbTotal = st.st_size / 1024 / 1024;
	else
		unpackProgressMbTotal = 1;

	unpackProgressMb = 0;

	FILE * ff = fopen("data.tar.gz", "rb");
	strcpy( fname, getenv("SECURE_STORAGE_DIR") );
	strcat( fname, "/busybox" );
	strcat( fname, " tar xz -C " );
	strcat( fname, getenv("SECURE_STORAGE_DIR") );
	FILE * fo = popen(fname, "w");
	if( !ff || !fo )
	{
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "Error extracting data");
		unpackFinished = 1;
		return (void *)0;
	}

	int unpackProgressKb = 0;
	for(;;)
	{
		char buf[1024 * 8];
		int cnt = fread( buf, 1, sizeof(buf), ff );
		if( cnt < 0 )
		{
			__android_log_print(ANDROID_LOG_INFO, "XSDL", "Error extracting data");
			unpackFinished = 1;
			return (void *)1;
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
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "Error extracting data");
		unpackFinished = 1;
		return (void *)0;
	}

	remove("data.tar.gz");

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "Extracting data finished");

	strcpy( fname, getenv("SECURE_STORAGE_DIR") );
	strcat( fname, "/postinstall.sh" );
	if( stat( fname, &st ) != 0 )
	{
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "No postinstall script");
		unpackFinished = 1;
		return (void *)1;
	}

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "Running postinstall scipt");

	fo = popen(fname, "r");
	if( !fo )
	{
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "ERROR: Cannot launch postinstall scipt");
		unpackFinished = 1;
		return (void *)0;
	}
	for(;;)
	{
		char buf[1024];
		if( !fgets(buf, sizeof(buf), fo) )
			break;
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "> %s", buf);
	}

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "Postinstall scipt exited with status %d", pclose(fo));

	unpackFinished = 1;
	return (void *)1;
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
		sprintf(s, "Unpacking data: %d/%d Mb, %d%%", unpackProgressMb, unpackProgressMbTotal, unpackProgressMb * 100 / (unpackProgressMbTotal > 0 ? unpackProgressMbTotal : 1));
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
	char native[32] = "0x0", native78[32], native68[32], native58[32], native48[32];
	int vertical = SDL_ListModes(NULL, 0)[0]->w < SDL_ListModes(NULL, 0)[0]->h;
	char cfgpath[PATH_MAX];
	FILE * cfgfile;

	if( vertical )
	{
		x = *resolutionW;
		*resolutionW = *resolutionH;
		*resolutionH = x;
		x = *displayW;
		*displayW = *displayH;
		*displayH = x;
	}

	const char * resStr[] = {
		native, native78, native68, native58,
		native48, "1280x1024", "1280x960", "1280x720",
		"1024x768", "800x600", "800x480", "640x480"
	};
	const int resVal[][2] = {
		{*resolutionW, *resolutionH},
		{*resolutionW * 7 / 8, *resolutionH * 7 / 8},
		{*resolutionW * 6 / 8, *resolutionH * 6 / 8},
		{*resolutionW * 5 / 8, *resolutionH * 5 / 8},
		{*resolutionW * 4 / 8, *resolutionH * 4 / 8},
		{1280,1024}, {1280,960}, {1280,720},
		{1024,768}, {800,600}, {800,480}, {640,480}
	};

	const char * fontsStr[] = {
		"X2.5", "X2", "X1.7", "X1.5",
		"X1.3", "X1", "X0.9", "X0.8",
		"X0.7", "X0.6", "X0.5", "X0.4",
		"X0.3", "X0.2", "X0.15", "X0.1"
	};
	const float fontsVal[] = {
		2.5f, 2.0f, 1.7f, 1.5f,
		1.3f, 1.0f, 0.9f, 0.8f,
		0.7f, 0.6f, 0.5f, 0.4f,
		0.3f, 0.2f, 0.15f, 0.1f
	};

	sprintf(native, "%dx%d", resVal[0][0], resVal[0][1]);
	sprintf(native78, "%dx%d", resVal[1][0], resVal[1][1]);
	sprintf(native68, "%dx%d", resVal[2][0], resVal[2][1]);
	sprintf(native58, "%dx%d", resVal[3][0], resVal[3][1]);
	sprintf(native48, "%dx%d", resVal[4][0], resVal[4][1]);

	int savedRes = 0;
	int savedDpi = 8;

	sprintf(cfgpath, "%s/.xsdl.cfg", getenv("SECURE_STORAGE_DIR"));
	cfgfile = fopen(cfgpath, "r");
	if( cfgfile )
	{
		fscanf(cfgfile, "%d %d", &savedRes, &savedDpi);
		fclose(cfgfile);
	}

	int counter = 3000, config = 0;
	Uint32 curtime = SDL_GetTicks();
	while ( counter > 0 && !config )
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_HELP)
						counter = 0;
				break;
				case SDL_MOUSEBUTTONUP:
					config = 1;
				break;
			}
		}
		SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
		y = VID_Y/3;
		renderString("Tap screen to change resolution and font scale", VID_X/2, y);
		char buf[100];
		y += 30;
		sprintf(buf, "Resolution: %s", resStr[savedRes]);
		renderString(buf, VID_X/2, y);
		y += 30;
		sprintf(buf, "Font scale: %s", fontsStr[savedDpi]);
		renderString(buf, VID_X/2, y);
		y += 40;
		sprintf(buf, "Starting in %d seconds", counter / 1000 + 1);
		renderString(buf, VID_X/2, y);
		SDL_Delay(100);
		SDL_Flip(SDL_GetVideoSurface());
		counter -= SDL_GetTicks() - curtime;
		curtime = SDL_GetTicks();
	}

	if( !config )
	{
		res = savedRes;
		dpi = savedDpi;
	}

	SDL_Joystick * j0 = SDL_JoystickOpen(0);

	while ( res < 0 )
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_HELP)
						return;
				break;
				case SDL_MOUSEBUTTONUP:
				{
					//SDL_GetMouseState(&x, &y);
					if( vertical )
					{
						int z = x;
						x = y;
						y = z;
					}
					i = (y / (VID_Y/2));
					ii = (x / (VID_X/4));
					res = i * 4 + ii;
					__android_log_print(ANDROID_LOG_INFO, "XSDL", "Screen coords %d %d res %d\n", x, y, res);
				}
				break;
				case SDL_JOYBALLMOTION:
					x = event.jball.xrel;
					y = event.jball.yrel;
				break;
			}
		}

		//__android_log_print(ANDROID_LOG_INFO, "XSDL", "Screen coords %d %d\n", x, y, res);
		SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
		renderString("Select display resolution", VID_X/2, VID_Y/3);
		for(i = 0; i < 3; i++)
		for(ii = 0; ii < 4; ii++)
		{
			if( vertical )
				renderString(resStr[i*4+ii], VID_Y/6 + (i*VID_Y/3), VID_X/8 + (ii*VID_X/4));
			else
				renderString(resStr[i*4+ii], VID_X/8 + (ii*VID_X/4), VID_Y/6 + (i*VID_Y/3));
			if( i == 0 && ii == 0 && !vertical )
				renderString("native", VID_X/8, VID_Y/6 + VID_Y/12);
		}
		//SDL_GetMouseState(&x, &y);
		//renderString("X", x, y);
		SDL_Delay(100);
		SDL_Flip(SDL_GetVideoSurface());
	}
	if( vertical )
	{
		*resolutionH = resVal[res][0];
		*resolutionW = resVal[res][1];
	}
	else
	{
		*resolutionW = resVal[res][0];
		*resolutionH = resVal[res][1];
	}
	while ( dpi < 0 )
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_HELP)
						return;
				break;
				case SDL_MOUSEBUTTONUP:
				{
					//SDL_GetMouseState(&x, &y);
					if( vertical )
					{
						int z = x;
						x = y;
						y = z;
					}
					i = (y / (VID_Y/4));
					ii = (x / (VID_X/4));
					dpi = i * 4 + ii;
					__android_log_print(ANDROID_LOG_INFO, "XSDL", "Screen coords %d %d dpi %d\n", x, y, res);
				}
				break;
				case SDL_JOYBALLMOTION:
					x = event.jball.xrel;
					y = event.jball.yrel;
				break;
			}
		}
		SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
		renderString("Select font scale", VID_X/2, VID_Y/2);
		for(i = 0; i < 4; i++)
		for(ii = 0; ii < 4; ii++)
		{
			int scale = (float)(*displayH) * (float)(*resolutionH) / 3500.0f * fontsVal[i*4+ii];
			if( vertical )
				renderStringScaled(fontsStr[i*4+ii], scale, VID_Y/8 + (i*VID_Y/4), VID_X/8 + (ii*VID_X/4), 255, 255, 255, SDL_GetVideoSurface());
			else
				renderStringScaled(fontsStr[i*4+ii], scale, VID_X/8 + (ii*VID_X/4), VID_Y/8 + (i*VID_Y/4), 255, 255, 255, SDL_GetVideoSurface());
		}
		//SDL_GetMouseState(&x, &y);
		//renderString("X", x, y);
		SDL_Delay(100);
		SDL_Flip(SDL_GetVideoSurface());
	}
	*displayW = *displayW / fontsVal[dpi];
	*displayH = *displayH / fontsVal[dpi];

	SDL_JoystickClose(j0);

	if( config )
	{
		cfgfile = fopen(cfgpath, "w");
		if( cfgfile )
		{
			fprintf(cfgfile, "%d %d\n", res, dpi);
			fclose(cfgfile);
		}
	}
}

void XSDL_generateBackground(const char * port, int showHelp, int resolutionW, int resolutionH)
{
	int sd, addr, ifc_num, i;
    struct ifconf ifc;
    struct ifreq ifr[20];
    SDL_Surface * surf;
    int y = resolutionH * 1 / 3;
    char msg[128];

	if (resolutionH > resolutionW)
		resolutionH = resolutionW;

	if( !showHelp )
	{
		surf = SDL_CreateRGBSurface(SDL_SWSURFACE, 16, 16, 24, 0x0000ff, 0x00ff00, 0xff0000, 0);
		SDL_FillRect(surf, NULL, 0x00002f);
		SDL_SaveBMP(surf, "background.bmp");
		SDL_FreeSurface(surf);
		return;
	}

	surf = SDL_CreateRGBSurface(SDL_SWSURFACE, resolutionW, resolutionH, 24, 0x0000ff, 0x00ff00, 0xff0000, 0);
	SDL_FillRect(surf, NULL, 0x00002f);

	renderStringScaled("Launch these commands on your Linux PC:", 12 * resolutionH / VID_Y, resolutionW/2, y, 255, 255, 255, surf);
	y += resolutionH * 30 / VID_Y;

    sd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sd > 0)
    {
        ifc.ifc_len = sizeof(ifr);
        ifc.ifc_ifcu.ifcu_buf = (caddr_t)ifr;

        if (ioctl(sd, SIOCGIFCONF, &ifc) == 0)
        {
            ifc_num = ifc.ifc_len / sizeof(struct ifreq);
            __android_log_print(ANDROID_LOG_INFO, "XSDL", "%d network interfaces found", ifc_num);

            for (i = 0; i < ifc_num; ++i)
            {
                int addr = 0;
                char saddr[32];
                if (ifr[i].ifr_addr.sa_family != AF_INET)
                    continue;

                if (ioctl(sd, SIOCGIFADDR, &ifr[i]) == 0)
                    addr = ((struct sockaddr_in *)(&ifr[i].ifr_addr))->sin_addr.s_addr;
                if (addr == 0)
                    continue;
                sprintf (saddr, "%d.%d.%d.%d", (addr & 0xFF), (addr >> 8 & 0xFF), (addr >> 16 & 0xFF), (addr >> 24 & 0xFF));
                __android_log_print(ANDROID_LOG_INFO, "XSDL", "interface: %s address: %s\n", ifr[i].ifr_name, saddr);
                if (strcmp(saddr, "127.0.0.1") == 0)
                    continue;
                sprintf (msg, "env DISPLAY=%s%s metacity &", saddr, port);
                renderStringScaled(msg, 12 * resolutionH / VID_Y, resolutionW/2, y, 255, 255, 255, surf);
                y += resolutionH * 15 / VID_Y;
                sprintf (msg, "env DISPLAY=%s%s gimp", saddr, port);
                renderStringScaled(msg, 12 * resolutionH / VID_Y, resolutionW/2, y, 255, 255, 255, surf);
                y += resolutionH * 20 / VID_Y;
            }
        }

        close(sd);
    }

    y += resolutionH * 10 / VID_Y;
    sprintf (msg, "To tunnel X over SSH, forward port %d", atoi(port+1) + 6000);
    renderStringScaled(msg, 12 * resolutionH / VID_Y, resolutionW/2, y, 255, 255, 255, surf);
    y += resolutionH * 15 / VID_Y;
    sprintf (msg, "in your SSH client");
    renderStringScaled(msg, 12 * resolutionH / VID_Y, resolutionW/2, y, 255, 255, 255, surf);

	SDL_SaveBMP(surf, "background.bmp");
	SDL_FreeSurface(surf);
}

void XSDL_showServerLaunchErrorMessage()
{
	showErrorMessage(	"Error: X server failed to launch,\n"
						"because of stale Unix socket with non-existing path.\n\n"
						"Power off your device and power it on,\n"
						"and everything will work again.");
}

void showErrorMessage(const char *msg)
{
	SDL_Event event;
	const char * s;
	int y = VID_Y/3;
	SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
	for( s = msg; s && s[0]; s = strchr(s, '\n'), s += (s ? 1 : 0), y += 30 )
	{
		const char * s1 = strchr(s, '\n');
		int len = s1 ? s1 - s : strlen(s);
		char buf[512];
		strncpy(buf, s, len);
		buf[len] = 0;
		if( len > 0 )
			renderString(buf, VID_X/2, y);
	}
	SDL_Flip(SDL_GetVideoSurface());
	while (1)
	{
		while (SDL_WaitEvent(&event))
		{
			switch (event.type)
			{
				case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_HELP)
					return;
				break;
			}
		}
	}
}

void XSDL_initSDL()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

	__android_log_print(ANDROID_LOG_INFO, "XSDL", "Current video mode: %d %d", SDL_ListModes(NULL, 0)[0]->w, SDL_ListModes(NULL, 0)[0]->h);

	if( SDL_ListModes(NULL, 0)[0]->w > SDL_ListModes(NULL, 0)[0]->h )
		SDL_SetVideoMode(VID_X, VID_Y, 0, SDL_SWSURFACE);
	else
		SDL_SetVideoMode(VID_Y, VID_X, 0, SDL_SWSURFACE);
	TTF_Init();
	sFont = TTF_OpenFont("DroidSansMono.ttf", 14);
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

void renderStringColor(const char *c, int x, int y, int r, int g, int b, SDL_Surface * surf)
{
	SDL_Color fColor = {r, g, b};
	SDL_Rect fontRect = {0, 0, 0, 0};
	SDL_Surface* fontSurface = TTF_RenderUTF8_Solid(sFont, c, fColor);
	fontRect.w = fontSurface->w;
	fontRect.h = fontSurface->h;
	fontRect.x = x - fontRect.w / 2;
	fontRect.y = y - fontRect.h / 2;
	SDL_BlitSurface(fontSurface, NULL, surf, &fontRect);
	SDL_FreeSurface(fontSurface);
}

void renderString(const char *c, int x, int y)
{
	renderStringColor(c, x, y, 255, 255, 255, SDL_GetVideoSurface());
}

void renderStringScaled(const char *c, int size, int x, int y, int r, int g, int b, SDL_Surface * surf)
{
	SDL_Color fColor = {r, g, b};
	SDL_Rect fontRect = {0, 0, 0, 0};
	TTF_Font* font = TTF_OpenFont("DroidSansMono.ttf", size);
	SDL_Surface* fontSurface = TTF_RenderUTF8_Solid(font, c, fColor);
	TTF_CloseFont(font);
	fontRect.w = fontSurface->w;
	fontRect.h = fontSurface->h;
	fontRect.x = x - fontRect.w / 2;
	fontRect.y = y - fontRect.h / 2;
	SDL_BlitSurface(fontSurface, NULL, surf, &fontRect);
	SDL_FreeSurface(fontSurface);
}

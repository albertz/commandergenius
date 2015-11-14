/*
 * "Ballfield"
 *
 *   (C) David Olofson <david@olofson.net>, 2002, 2003
 *
 * This software is released under the terms of the GPL.
 *
 * Contact author for permission if you want to use this
 * software, or work derived from it, under other terms.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <android/log.h>
#include <wchar.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_screenkeyboard.h>

#define fprintf(X, ...) __android_log_print(ANDROID_LOG_INFO, "Ballfield", __VA_ARGS__)
#define printf(...) __android_log_print(ANDROID_LOG_INFO, "Ballfield", __VA_ARGS__)

/*----------------------------------------------------------
	Definitions...
----------------------------------------------------------*/

#define	SCREEN_W	1280
#define	SCREEN_H	800



/*----------------------------------------------------------
	General tool functions
----------------------------------------------------------*/

/*
 * Bump areas of low and high alpha to 0% or 100%
 * respectively, just in case the graphics contains
 * "alpha noise".
 */
SDL_Surface *clean_alpha(SDL_Surface *s)
{
	SDL_Surface *work;
	SDL_Rect r;
	Uint32 *pixels;
	int pp;
	int x, y;
	
	work = SDL_CreateRGBSurface(SDL_SWSURFACE, s->w, s->h,
			32, 0xff000000, 0x00ff0000, 0x0000ff00,
			0x000000ff);
	if(!work)
		return NULL;

	r.x = r.y = 0;
	r.w = s->w;
	r.h = s->h;
	if(SDL_BlitSurface(s, &r, work, NULL) < 0)
	{
		SDL_FreeSurface(work);
		return NULL;
	}

	SDL_LockSurface(work);
	pixels = (Uint32 *)work->pixels;
	pp = work->pitch / sizeof(Uint32);
	for(y = 0; y < work->h; ++y)
		for(x = 0; x < work->w; ++x)
		{
			Uint32 pix = pixels[y*pp + x];
			switch((pix & 0xff) >> 4)
			{
			  case 0:
				pix = 0x00000000;
			  	break;
			  default:
			  	break;
			  case 15:
				pix |= 0xff;
			  	break;
			}
			pixels[y*pp + x] = pix;
		}
	SDL_UnlockSurface(work);

	return work;
}


/*
 * Load and convert an antialiazed, zoomed set of sprites.
 */
SDL_Surface *load_zoomed(char *name, int alpha)
{
	SDL_Surface *sprites;
	SDL_Surface *temp = IMG_Load(name);
	if(!temp)
		return NULL;

	/*
	sprites = temp;
	SDL_SetAlpha(sprites, 0, 255);
	temp = clean_alpha(sprites);
	SDL_FreeSurface(sprites);
	*/
	if(!temp)
	{
		fprintf(stderr, "Could not clean alpha!\n");
		return NULL;
	}

	if(alpha)
	{
		SDL_SetAlpha(temp, 0, SDL_ALPHA_OPAQUE);
		sprites = SDL_DisplayFormatAlpha(temp);
	}
	else
	{
		SDL_SetColorKey(temp, SDL_SRCCOLORKEY,
				SDL_MapRGB(temp->format, 0, 0, 0));
		sprites = SDL_DisplayFormat(temp);
	}
	SDL_FreeSurface(temp);

	return sprites;
}


void print_num(SDL_Surface *dst, SDL_Surface *font, int x, int y, float value)
{
	char buf[16];
	int val = (int)(value * 10.0);
	int pos, p = 0;
	SDL_Rect from;

	/* Sign */
	if(val < 0)
	{
		buf[p++] = 10;
		val = -val;
	}

	/* Integer part */
	pos = 10000000;
	while(pos > 1)
	{
		int num = val / pos;
		val -= num * pos;
		pos /= 10;
		if(p || num)
			buf[p++] = num;
	}

	/* Decimals */
	if(val / pos)
	{
		buf[p++] = 11;
		while(pos > 0)
		{
			int num = val / pos;
			val -= num * pos;
			pos /= 10;
			buf[p++] = num;
		}
	}

	/* Render! */
	from.y = 0;
	from.w = 7;
	from.h = 10;
	for(pos = 0; pos < p; ++pos)
	{
		SDL_Rect to;
		to.x = x + pos * 7;
		to.y = y;
		from.x = buf[pos] * 7;
		SDL_BlitSurface(font, &from, dst, &to);
	}
}

/*
 * Draw tiled background image with offset.
 */
void tiled_back(SDL_Surface *back, SDL_Surface *screen, int xo, int yo)
{
	SDL_Rect r;
	xo %= back->w/8;
	yo %= back->h/8;
	r.x = xo - back->w/2 + screen->w/2;
	r.y = yo - back->h/2 + screen->h/2;
	r.w = back->w;
	r.h = back->h;
	SDL_BlitSurface(back, NULL, screen, &r);
}

void print_num_hex(SDL_Surface *dst, SDL_Surface *font, int x, int y, unsigned val)
{
	char buf[8];
	int pos, p = 0;
	SDL_Rect from;
	
	//val = htonl(val); // Big-endian

	/* Render! */
	from.y = 0;
	from.w = 7;
	from.h = 10;
	for(pos = 0; pos < 8; ++pos)
	{
		SDL_Rect to;
		to.x = 8 * 7 - (x + pos * 7); // Little-endian number wrapped backwards
		to.y = y;
		from.x = ( ( val >> (pos * 4) ) & 0xf ) * 7;
		SDL_BlitSurface(font, &from, dst, &to);
	}
}


/*----------------------------------------------------------
	main()
----------------------------------------------------------*/

int main(int argc, char* argv[])
{
	SDL_Surface	*screen;
	SDL_Surface	*temp_image;
	SDL_Surface	*back, *logo, *font, *font_hex;
	SDL_Event	event;
	int		bpp = 16,
			flags = SDL_HWSURFACE,
			alpha = 1;
	int		x_offs = 0, y_offs = 0;
	long		tick,
			last_tick,
			last_avg_tick;
	double		t = 0;
	float		dt;
	int		i;
	float		fps = 0.0;
	int		fps_count = 0;
	int		fps_start = 0;
	float		x_speed, y_speed, z_speed;
	enum { MAX_POINTERS = 16 };
	// some random colors
	int colors[MAX_POINTERS] = { 0xaaaaaa, 0xffffff, 0x888888, 0xcccccc, 0x666666, 0x999999, 0xdddddd, 0xeeeeee, 0xaaaaaa, 0xffffff, 0x888888, 0xcccccc, 0x666666, 0x999999, 0xdddddd, 0xeeeeee };
	struct TouchPointer_t { int x; int y; int pressure; int pressed; } touchPointers[MAX_POINTERS];
	int accel[5], screenjoy[4], gamepads[4][8];
	SDL_Surface	*mouse[4];
	int screenKeyboardShown = 0;


	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_Joystick * joysticks[6];
	for( i = 0; i < 6; i++ )
		joysticks[i] = SDL_JoystickOpen(i);

	atexit(SDL_Quit);

	screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, bpp, flags);
	if(!screen)
	{
		fprintf(stderr, "Failed to open screen!\n");
		exit(-1);
	}

	SDL_WM_SetCaption("Ballfield", "Ballfield");
	if(flags & SDL_FULLSCREEN)
		SDL_ShowCursor(0);

	/*
	 * Load background image
	 */
	temp_image = IMG_Load("maxresdefault.jpg");
	if(!temp_image)
	{
		fprintf(stderr, "Could not load background!\n");
		exit(-1);
	}
	back = SDL_DisplayFormat(temp_image);
	SDL_FreeSurface(temp_image);

	/*
	 * Load logo
	 */
	temp_image = SDL_LoadBMP("logo.bmp");
	if(!temp_image)
	{
		fprintf(stderr, "Could not load logo!\n");
		exit(-1);
	}
	SDL_SetColorKey(temp_image, SDL_SRCCOLORKEY,
			SDL_MapRGB(temp_image->format, 255, 0, 255));
	logo = SDL_DisplayFormat(temp_image);
	SDL_FreeSurface(temp_image);

	/*
	 * Load font
	 */
	temp_image = SDL_LoadBMP("font7x10.bmp");
	if(!temp_image)
	{
		fprintf(stderr, "Could not load font!\n");
		exit(-1);
	}
	SDL_SetColorKey(temp_image, SDL_SRCCOLORKEY,
			SDL_MapRGB(temp_image->format, 255, 0, 255));
	font = SDL_DisplayFormat(temp_image);
	SDL_FreeSurface(temp_image);

	temp_image = SDL_LoadBMP("font7x10-hex.bmp");
	if(!temp_image)
	{
		fprintf(stderr, "Could not load hex font!\n");
		exit(-1);
	}
	SDL_SetColorKey(temp_image, SDL_SRCCOLORKEY,
			SDL_MapRGB(temp_image->format, 255, 0, 255));
	font_hex = SDL_DisplayFormat(temp_image);
	SDL_FreeSurface(temp_image);

	for(i = 0; i < 4; i++)
	{
		char name[32];
		sprintf(name, "mouse%d.png", i);
		temp_image = IMG_Load(name);
		if(!temp_image)
		{
			fprintf(stderr, "Could not load %s!\n", name);
			exit(-1);
		}
		//mouse[i] = SDL_DisplayFormat(temp_image);
		//SDL_FreeSurface(temp_image);
		mouse[i] = temp_image; // Keep alpha
	}

	last_avg_tick = last_tick = SDL_GetTicks();

	float gyroX = SCREEN_W/2, gyroY = SCREEN_H/2;

	while(1)
	{
		SDL_Rect r;

		/* Timing */
		tick = SDL_GetTicks();
		dt = (tick - last_tick) * 0.001f;
		last_tick = tick;
		
		if( bpp == 32 )
			SDL_FillRect(screen, NULL, 0); // Clear alpha channel

		/* Background image */
		tiled_back(back, screen, x_offs>>11, y_offs>>11);

		/* FPS counter */
		if(tick > fps_start + 1000)
		{
			fps = (float)fps_count * 1000.0 / (tick - fps_start);
			fps_count = 0;
			fps_start = tick;
		}

		print_num(screen, font, screen->w-37, screen->h-12, fps);
		++fps_count;


		r.x = gyroX;
		r.y = gyroY;
		r.w = mouse[0]->w;
		r.h = mouse[0]->h;
		r.x -= r.w/2;
		r.y -= r.h/2;
		SDL_BlitSurface(mouse[0], NULL, screen, &r);

		SDL_Flip(SDL_GetVideoSurface());

		SDL_Event evt;
		while( SDL_PollEvent(&evt) )
		{
			if(evt.type == SDL_KEYUP || evt.type == SDL_KEYDOWN)
			{
				__android_log_print(ANDROID_LOG_INFO, "Ballfield", "SDL key event: evt %s state %s key %4d %12s scancode %4d mod %2d unicode %d", evt.type == SDL_KEYUP ? "UP  " : "DOWN" , evt.key.state == SDL_PRESSED ? "PRESSED " : "RELEASED", (int)evt.key.keysym.sym, SDL_GetKeyName(evt.key.keysym.sym), (int)evt.key.keysym.scancode, (int)evt.key.keysym.mod, (int)evt.key.keysym.unicode);
				if(evt.key.keysym.sym == SDLK_ESCAPE)
					return 0;
			}
			if(evt.type == SDL_MOUSEBUTTONUP || evt.type == SDL_MOUSEBUTTONDOWN)
			{
				__android_log_print(ANDROID_LOG_INFO, "Ballfield", "SDL mouse button event: evt %s state %s button %d coords %d:%d", evt.type == SDL_MOUSEBUTTONUP ? "UP  " : "DOWN" , evt.button.state == SDL_PRESSED ? "PRESSED " : "RELEASED", (int)evt.button.button, (int)evt.button.x, (int)evt.button.y);
				gyroX = SCREEN_W/2;
				gyroY = SCREEN_H/2;
			}
			if(evt.type == SDL_VIDEORESIZE)
				__android_log_print(ANDROID_LOG_INFO, "Ballfield", "SDL resize event: %d x %d", evt.resize.w, evt.resize.h);
			if(evt.type == SDL_ACTIVEEVENT)
				__android_log_print(ANDROID_LOG_INFO, "Ballfield", "======= SDL active event: gain %d state %d", evt.active.gain, evt.active.state);
			// Android-specific events - accelerometer, multitoush, and on-screen joystick
			if( evt.type == SDL_JOYAXISMOTION )
			{
				if(evt.jaxis.which == 1 && evt.jaxis.axis == 2)
					gyroX += evt.jaxis.value / 50.0f;
				if(evt.jaxis.which == 1 && evt.jaxis.axis == 3)
					gyroY -= evt.jaxis.value / 50.0f;
				if(gyroX < 0)
					gyroX = 0;
				if(gyroX > SCREEN_W)
					gyroX = SCREEN_W;
				if(gyroY < 0)
					gyroY = 0;
				if(gyroY > SCREEN_H)
					gyroY = SCREEN_H;
			}
		}

		/* Animate */
		/*
		x_speed = 500.0 * sin(t * 0.37);
		y_speed = 500.0 * sin(t * 0.53);
		z_speed = 400.0 * sin(t * 0.21);
		x_offs -= x_speed;
		y_offs -= y_speed;
		*/

		t += dt;
	}

	SDL_FreeSurface(back);
	SDL_FreeSurface(logo);
	SDL_FreeSurface(font);
	return 0;
}

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

#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <android/log.h>

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"


/*----------------------------------------------------------
	Definitions...
----------------------------------------------------------*/
#define	SCREEN_W	320
#define	SCREEN_H	240

#define	BALLS	300

#define	COLORS	2

typedef struct
{
	Sint32	x, y, z;	/* Position */
	Uint32	c;		/* Color */
} point_t;


/*
 * Ballfield
 */
typedef struct
{
	point_t		points[BALLS];
	SDL_Rect	*frames;
	SDL_Surface	*gfx[COLORS];
	int		use_alpha;
} ballfield_t;


/*
 * Size of the biggest ball image in pixels
 *
 * Balls are scaled down and *packed*, one pixel
 * smaller for each frame down to 1x1. The actual
 * image width is (obviously...) the same as the
 * width of the first frame.
 */
#define	BALL_W	32
#define	BALL_H	32




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
	ballfield_t functions
----------------------------------------------------------*/

ballfield_t *ballfield_init(void)
{
	int i;
	ballfield_t *bf = (ballfield_t *)calloc(sizeof(ballfield_t), 1);
	if(!bf)
		return NULL;
	for(i = 0; i < BALLS; ++i)
	{
		bf->points[i].x = rand() % 0x20000;
		bf->points[i].y = rand() % 0x20000;
		bf->points[i].z = 0x20000 * i / BALLS;
		if(rand() % 100 > 80)
			bf->points[i].c = 1;
		else
			bf->points[i].c = 0;
	}
	return bf;
}


void ballfield_free(ballfield_t *bf)
{
	int i;
	for(i = 0; i < COLORS; ++i)
		SDL_FreeSurface(bf->gfx[i]);
}


static int ballfield_init_frames(ballfield_t *bf)
{
	int i, j;
	/*
	 * Set up source rects for all frames
	 */
	bf->frames = (SDL_Rect *)calloc(sizeof(SDL_Rect), bf->gfx[0]->w);
	if(!bf->frames)
	{
		fprintf(stderr, "No memory for frame rects!\n");
		return -1;
	}
	for(j = 0, i = 0; i < bf->gfx[0]->w; ++i)
	{
		bf->frames[i].x = 0;
		bf->frames[i].y = j;
		bf->frames[i].w = bf->gfx[0]->w - i;
		bf->frames[i].h = bf->gfx[0]->w - i;
		j += bf->gfx[0]->w - i;
	}
	return 0;
}


int ballfield_load_gfx(ballfield_t *bf, char *name, unsigned int color)
{
	if(color >= COLORS)
		return -1;

	bf->gfx[color] = load_zoomed(name, bf->use_alpha);
	if(!bf->gfx[color])
		return -2;

	if(!bf->frames)
		return ballfield_init_frames(bf);

	return 0;
}


void ballfield_move(ballfield_t *bf, Sint32 dx, Sint32 dy, Sint32 dz)
{
	int i;
	for(i = 0; i < BALLS; ++i)
	{
		bf->points[i].x += dx;
		bf->points[i].x &= 0x1ffff;
		bf->points[i].y += dy;
		bf->points[i].y &= 0x1ffff;
		bf->points[i].z += dz;
		bf->points[i].z &= 0x1ffff;
	}
}


void ballfield_render(ballfield_t *bf, SDL_Surface *screen)
{
	int i, j, z;

	/* 
	 * Find the ball with the highest Z.
	 */
	z = 0;
	j = 0;
	for(i = 0; i < BALLS; ++i)
	{
		if(bf->points[i].z > z)
		{
			j = i;
			z = bf->points[i].z;
		}
	}

	/* 
	 * Render all balls in back->front order.
	 */
	for(i = 0; i < BALLS; ++i)
	{
		SDL_Rect r;
		int f;
		z = bf->points[j].z;
		z += 50;
		f = ((bf->frames[0].w << 12) + 100000) / z;
		f = bf->frames[0].w - f;
		if(f < 0)
			f = 0;
		else if(f > bf->frames[0].w - 1)
			f = bf->frames[0].w - 1;
		z >>= 7;
		z += 1;
		r.x = (bf->points[j].x - 0x10000) / z;
		r.y = (bf->points[j].y - 0x10000) / z;
		r.x += (screen->w - bf->frames[f].w) >> 1;
		r.y += (screen->h - bf->frames[f].h) >> 1;
		SDL_BlitSurface(bf->gfx[bf->points[j].c],
			&bf->frames[f], screen, &r);
		if(--j < 0)
			j = BALLS - 1;
	}
}



/*----------------------------------------------------------
	Other rendering functions
----------------------------------------------------------*/

/*
 * Draw tiled background image with offset.
 */
void tiled_back(SDL_Surface *back, SDL_Surface *screen, int xo, int yo)
{
	/*
	int x, y;
	SDL_Rect r;
	if(xo < 0)
		xo += back->w*(-xo/back->w + 1);
	if(yo < 0)
		yo += back->h*(-yo/back->h + 1);
	xo %= back->w;
	yo %= back->h;
	for(y = -yo; y < screen->h; y += back->h)
		for(x = -xo; x < screen->w; x += back->w)
		{
			r.x = x;
			r.y = y;
			SDL_BlitSurface(back, NULL, screen, &r);
		}
	*/
	SDL_Rect r;
	xo %= back->w/8;
	yo %= back->h/8;
	r.x = xo - back->w/2 + screen->w/2;
	r.y = yo - back->h/2 + screen->h/2;
	r.w = back->w;
	r.h = back->h;
	SDL_BlitSurface(back, NULL, screen, &r);
}

#pragma GCC push_options
#pragma GCC optimize ("O0")
extern "C" unsigned misaligned_mem_access(unsigned value, unsigned shift);

unsigned misaligned_mem_access(unsigned value, unsigned shift)
{
    volatile unsigned *iptr = NULL;
    volatile char *cptr = NULL;
    volatile unsigned ret = 0;
 
#if defined(__GNUC__)
# if defined(__i386__)
    /* Enable Alignment Checking on x86 */
    __asm__("pushf\norl $0x40000,(%esp)\npopf");
# elif defined(__x86_64__) 
     /* Enable Alignment Checking on x86_64 */
    __asm__("pushf\norl $0x40000,(%rsp)\npopf");
# endif
#endif

    /* malloc() always provides aligned memory */
    cptr = (volatile char *)malloc(sizeof(unsigned) + 10);

    /* Increment the pointer by one, making it misaligned */
    iptr = (volatile unsigned *) (cptr + shift);

    /* Dereference it as an int pointer, causing an unaligned access */
    /* GCC usually tries to optimize this, thus our test succeeds when it should fail, if we remove "volatile" specifiers */
    *iptr = value;
    //memcpy( &ret, iptr, sizeof(unsigned) );
    ret = *iptr;
    /*
    *((volatile char *)(&ret) + 0) = cptr[shift+0];
    *((volatile char *)(&ret) + 1) = cptr[shift+1];
    *((volatile char *)(&ret) + 2) = cptr[shift+2];
    *((volatile char *)(&ret) + 3) = cptr[shift+3];
    */
    free((void *)cptr);

    return ret;
}
#pragma GCC pop_options

/*----------------------------------------------------------
	main()
----------------------------------------------------------*/

extern "C" void unaligned_test(unsigned * data, unsigned * target);
extern "C" unsigned val0, val1, val2, val3, val4;

unsigned val0 = 0x01234567, val1, val2, val3;


int main(int argc, char* argv[])
{
	ballfield_t	*balls;
	SDL_Surface	*screen;
	SDL_Surface	*temp_image;
	SDL_Surface	*back, *logo, *font, *font_hex;
	SDL_Event	event;
	int		bpp = 16,
			flags = 0,
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
	Mix_Music *music = NULL;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);

	atexit(SDL_Quit);

	if(Mix_OpenAudio(44100, AUDIO_S16, 2, 8192))
	{
		fprintf(stderr, "Failed to open audio\n");
		exit(1);
	}
	if((music = Mix_LoadMUS("theday.xm")) == NULL)
	{
		fprintf(stderr, "Failed to load music file theday.xm\n");
		exit(1);
	}
	Mix_PlayMusic(music, -1);


	screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, bpp, flags);
	if(!screen)
	{
		fprintf(stderr, "Failed to open screen!\n");
		exit(-1);
	}

	SDL_WM_SetCaption("Ballfield", "Ballfield");
	if(flags & SDL_FULLSCREEN)
		SDL_ShowCursor(0);

	balls = ballfield_init();
	if(!balls)
	{
		fprintf(stderr, "Failed to create ballfield!\n");
		exit(-1);
	}

	/*
	 * Load and prepare balls...
	 */
	balls->use_alpha = alpha;
	if( ballfield_load_gfx(balls, "blueball.png", 0)
				||
			ballfield_load_gfx(balls, "redball.png", 1) )
	{
		fprintf(stderr, "Could not load balls!\n");
		exit(-1);
	}

	/*
	 * Load background image
	 */
	temp_image = IMG_Load("sun.gif");
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

	last_avg_tick = last_tick = SDL_GetTicks();
	
	enum { MAX_POINTERS = 16, PTR_PRESSED = 4 };
	int touchPointers[MAX_POINTERS][5];
	
	memset(touchPointers, 0, sizeof(touchPointers));
	SDL_Joystick * joysticks[MAX_POINTERS+1];
	for(i=0; i<MAX_POINTERS; i++)
		joysticks[i] = SDL_JoystickOpen(i);

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

		/* Ballfield */
		ballfield_render(balls, screen);

		/* Logo */
		r.x = 2;
		r.y = 2;
		SDL_BlitSurface(logo, NULL, screen, &r);

		/* FPS counter */
		if(tick > fps_start + 1000)
		{
			fps = (float)fps_count * 1000.0 / (tick - fps_start);
			fps_count = 0;
			fps_start = tick;
			
			*((unsigned char *)(&val0) + 0) += 1;
			*((unsigned char *)(&val0) + 1) += 1;
			*((unsigned char *)(&val0) + 2) += 1;
			*((unsigned char *)(&val0) + 3) += 1;
		}
		// MISALIGNED MEMORY ACCESS HERE! However all the devices that I have won't report it and won't send a signal or write to the /proc/kmsg,
		// despite the /proc/cpu/alignment flag set.
		val1 = misaligned_mem_access(val0, 1);
		val2 = misaligned_mem_access(val0, 2);
		val3 = misaligned_mem_access(val0, 3);
		/*
		print_num_hex(screen, font_hex, 0, 40, val0);
		print_num_hex(screen, font_hex, 0, 60, val1);
		print_num_hex(screen, font_hex, 0, 80, val2);
		print_num_hex(screen, font_hex, 0, 100, val3);
		*/

		print_num(screen, font, screen->w-37, screen->h-12, fps);
		++fps_count;

		for(i=0; i<MAX_POINTERS; i++)
		{
			if( !touchPointers[i][PTR_PRESSED] )
				continue;
			r.x = touchPointers[i][0];
			r.y = touchPointers[i][1];
			r.w = 80 + touchPointers[i][2] / 10; // Pressure
			r.h = 80 + touchPointers[i][3] / 10; // Touch point size
			r.x -= r.w/2;
			r.y -= r.h/2;
			SDL_FillRect(screen, &r, 0xaaaaaa);
			print_num(screen, font, r.x, r.y, i+1);
		}
		int mx, my;
		int b = SDL_GetMouseState(&mx, &my);
		//__android_log_print(ANDROID_LOG_INFO, "Ballfield", "Mouse buttons: %d", b);
		Uint32 color = 0xff;
		if( b )
		{
			color = 0;
			if( b & SDL_BUTTON_LMASK )
				color |= 0xf000;
			if( b & SDL_BUTTON_RMASK )
				color |= 0x1f0;
			if( b & SDL_BUTTON_MMASK )
				color |= 0x1f;
		}
		r.x = mx;
		r.y = my;
		r.w = 30;
		r.h = 30;
		r.x -= r.w/2;
		r.y -= r.h/2;
		SDL_FillRect(screen, &r, color);

		SDL_Flip(SDL_GetVideoSurface());
		SDL_Event evt;
		while( SDL_PollEvent(&evt) )
		{
			if(evt.type == SDL_KEYUP || evt.type == SDL_KEYDOWN)
			{
				__android_log_print(ANDROID_LOG_INFO, "Ballfield", "SDL key event: evt %s state %s key %d scancode %d mod %d unicode %d", evt.type == SDL_KEYUP ? "UP  " : "DOWN" , evt.key.state == SDL_PRESSED ? "PRESSED " : "RELEASED", (int)evt.key.keysym.sym, (int)evt.key.keysym.scancode, (int)evt.key.keysym.mod, (int)evt.key.keysym.unicode);
				if(evt.key.keysym.sym == SDLK_ESCAPE)
					return 0;
			}
			if(evt.type == SDL_VIDEORESIZE)
				__android_log_print(ANDROID_LOG_INFO, "Ballfield", "SDL resize event: %d x %d", evt.resize.w, evt.resize.h);
			if(evt.type == SDL_ACTIVEEVENT)
				__android_log_print(ANDROID_LOG_INFO, "Ballfield", "======= SDL active event: gain %d state %d", evt.active.gain, evt.active.state);
			/*
			if( evt.type == SDL_ACTIVEEVENT && evt.active.gain == 0 && evt.active.state & SDL_APPACTIVE )
			{
				// We've lost GL context, we are not allowed to do any GFX output here, or app will crash!
				while( 1 )
				{
					SDL_PollEvent(&evt);
					if( evt.type == SDL_ACTIVEEVENT && evt.active.gain && evt.active.state & SDL_APPACTIVE )
					{
						__android_log_print(ANDROID_LOG_INFO, "Ballfield", "======= SDL active event: gain %d state %d", evt.active.gain, evt.active.state);
						SDL_Flip(SDL_GetVideoSurface()); // One SDL_Flip() call is required here to restore OpenGL context
						// Re-load all textures, matrixes and all other GL states if we're in SDL+OpenGL mode
						// Re-load all images to SDL_Texture if we're using it
						// Now we can draw
						break;
					}
					// Process network stuff, maybe play some sounds using SDL_ANDROID_PauseAudioPlayback() / SDL_ANDROID_ResumeAudioPlayback()
					SDL_Delay(300);
					__android_log_print(ANDROID_LOG_INFO, "Ballfield", "Waiting");
				}
			}
			*/
			if( evt.type == SDL_JOYAXISMOTION )
			{
				if( evt.jaxis.which == 0 ) // 0 = The accelerometer
					continue;
				int joyid = evt.jaxis.which - 1;
				touchPointers[joyid][evt.jaxis.axis] = evt.jaxis.value; // Axis 0 and 1 are coordinates, 2 and 3 are pressure and touch point radius
			}
			if( evt.type == SDL_JOYBUTTONDOWN || evt.type == SDL_JOYBUTTONUP )
			{
				if( evt.jbutton.which == 0 ) // 0 = The accelerometer
					continue;
				int joyid = evt.jbutton.which - 1;
				touchPointers[joyid][PTR_PRESSED] = (evt.jbutton.state == SDL_PRESSED);
			}
		}

		/* Animate */
		x_speed = 500.0 * sin(t * 0.37);
		y_speed = 500.0 * sin(t * 0.53);
		z_speed = 400.0 * sin(t * 0.21);

		ballfield_move(balls, x_speed, y_speed, z_speed);
		x_offs -= x_speed;
		y_offs -= y_speed;

		t += dt;
	}

	ballfield_free(balls);
	SDL_FreeSurface(back);
	SDL_FreeSurface(logo);
	SDL_FreeSurface(font);
	std::ostringstream os;
	os << "lalala" << std::endl << "more text" << std::endl;
	std::cout << os.str() << std::endl << "text text" << std::endl;
	exit(0);
}

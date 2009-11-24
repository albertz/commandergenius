/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

/* Dummy SDL video driver implementation; this is just enough to make an
 *  SDL-based application THINK it's got a working video driver, for
 *  applications that call SDL_Init(SDL_INIT_VIDEO) when they don't need it,
 *  and also for use as a collection of stubs when porting SDL to a new
 *  platform for which you haven't yet written a valid video driver.
 *
 * This is also a great way to determine bottlenecks: if you think that SDL
 *  is a performance problem for a given platform, enable this driver, and
 *  then see if your application runs faster without video overhead.
 *
 * Initial work by Ryan C. Gordon (icculus@icculus.org). A good portion
 *  of this was cut-and-pasted from Stephane Peter's work in the AAlib
 *  SDL video driver.  Renamed to "ANDROID" by Sam Lantinga.
 */

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_androidvideo.h"

#include <jni.h>
#include <android/log.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>
#include <math.h>


#define ANDROIDVID_DRIVER_NAME "android"

/* Initialization/Query functions */
static int ANDROID_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Rect **ANDROID_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags);
static SDL_Surface *ANDROID_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int ANDROID_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void ANDROID_VideoQuit(_THIS);

/* Hardware surface functions */
static int ANDROID_AllocHWSurface(_THIS, SDL_Surface *surface);
static int ANDROID_LockHWSurface(_THIS, SDL_Surface *surface);
static void ANDROID_UnlockHWSurface(_THIS, SDL_Surface *surface);
static void ANDROID_FreeHWSurface(_THIS, SDL_Surface *surface);

/* etc. */
static void ANDROID_UpdateRects(_THIS, int numrects, SDL_Rect *rects);


static int sWindowWidth  = 320;
static int sWindowHeight = 480;
static SDL_sem * WaitForNativeRender = NULL;
// Pointer to in-memory video surface
static int memX = 0;
static int memY = 0;
static void * memBuffer = NULL;
static SDL_Thread * mainThread = NULL;

static SDLKey keymap[KEYCODE_LAST+1];

/* ANDROID driver bootstrap functions */

static int ANDROID_Available(void)
{
	/*
	const char *envr = SDL_getenv("SDL_VIDEODRIVER");
	if ((envr) && (SDL_strcmp(envr, ANDROIDVID_DRIVER_NAME) == 0)) {
		return(1);
	}

	return(0);
	*/
	return 1;
}

static void ANDROID_DeleteDevice(SDL_VideoDevice *device)
{
	SDL_free(device->hidden);
	SDL_free(device);
}

static SDL_VideoDevice *ANDROID_CreateDevice(int devindex)
{
	SDL_VideoDevice *device;

	/* Initialize all variables that we clean on shutdown */
	device = (SDL_VideoDevice *)SDL_malloc(sizeof(SDL_VideoDevice));
	if ( device ) {
		SDL_memset(device, 0, (sizeof *device));
		device->hidden = (struct SDL_PrivateVideoData *)
				SDL_malloc((sizeof *device->hidden));
	}
	if ( (device == NULL) || (device->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( device ) {
			SDL_free(device);
		}
		return(0);
	}
	SDL_memset(device->hidden, 0, (sizeof *device->hidden));

	/* Set the function pointers */
	device->VideoInit = ANDROID_VideoInit;
	device->ListModes = ANDROID_ListModes;
	device->SetVideoMode = ANDROID_SetVideoMode;
	device->CreateYUVOverlay = NULL;
	device->SetColors = ANDROID_SetColors;
	device->UpdateRects = ANDROID_UpdateRects;
	device->VideoQuit = ANDROID_VideoQuit;
	device->AllocHWSurface = ANDROID_AllocHWSurface;
	device->CheckHWBlit = NULL;
	device->FillHWRect = NULL;
	device->SetHWColorKey = NULL;
	device->SetHWAlpha = NULL;
	device->LockHWSurface = ANDROID_LockHWSurface;
	device->UnlockHWSurface = ANDROID_UnlockHWSurface;
	device->FlipHWSurface = NULL;
	device->FreeHWSurface = ANDROID_FreeHWSurface;
	device->SetCaption = NULL;
	device->SetIcon = NULL;
	device->IconifyWindow = NULL;
	device->GrabInput = NULL;
	device->GetWMInfo = NULL;
	device->InitOSKeymap = ANDROID_InitOSKeymap;
	device->PumpEvents = ANDROID_PumpEvents;

	device->free = ANDROID_DeleteDevice;

	return device;
}

VideoBootStrap ANDROID_bootstrap = {
	ANDROIDVID_DRIVER_NAME, "SDL android video driver",
	ANDROID_Available, ANDROID_CreateDevice
};


int ANDROID_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
	int i;
	/* Determine the screen depth (use default 16-bit depth) */
	/* we change this during the SDL_SetVideoMode implementation... */
	vformat->BitsPerPixel = 16;
	vformat->BytesPerPixel = 2;

	for ( i=0; i<SDL_NUMMODES; ++i ) {
		SDL_modelist[i] = SDL_malloc(sizeof(SDL_Rect));
		SDL_modelist[i]->x = SDL_modelist[i]->y = 0;
	}
	/* Modes sorted largest to smallest */
	SDL_modelist[0]->w = sWindowWidth; SDL_modelist[0]->h = sWindowHeight;
	SDL_modelist[1]->w = 320; SDL_modelist[1]->h = 240; // Always available on any screen and any orientation
	SDL_modelist[2]->w = 320; SDL_modelist[2]->h = 200; // Always available on any screen and any orientation
	SDL_modelist[3] = NULL;

	WaitForNativeRender = SDL_CreateSemaphore(0);
	/* We're done! */
	return(0);
}

SDL_Rect **ANDROID_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
	if(format->BitsPerPixel != 16)
		return NULL;
	return SDL_modelist;
}

SDL_Surface *ANDROID_SetVideoMode(_THIS, SDL_Surface *current,
				int width, int height, int bpp, Uint32 flags)
{
	if ( this->hidden->buffer ) {
		SDL_free( this->hidden->buffer );
	}

	this->hidden->buffer = SDL_malloc(width * height * (bpp / 8));
	if ( ! this->hidden->buffer ) {
		SDL_SetError("Couldn't allocate buffer for requested mode");
		return(NULL);
	}
	memX = width;
	memY = height;
	memBuffer = this->hidden->buffer;

/* 	printf("Setting mode %dx%d\n", width, height); */

	SDL_memset(this->hidden->buffer, 0, width * height * (bpp / 8));

	/* Allocate the new pixel format for the screen */
	if ( ! SDL_ReallocFormat(current, bpp, 0, 0, 0, 0) ) {
		SDL_free(this->hidden->buffer);
		this->hidden->buffer = NULL;
		SDL_SetError("Couldn't allocate new pixel format for requested mode");
		return(NULL);
	}

	/* Set up the new mode framebuffer */
	current->flags = flags & SDL_FULLSCREEN;
	this->hidden->w = current->w = width;
	this->hidden->h = current->h = height;
	current->pitch = current->w * (bpp / 8);
	current->pixels = this->hidden->buffer;
	
	/* We're done */
	return(current);
}

/* We don't actually allow hardware surfaces other than the main one */
static int ANDROID_AllocHWSurface(_THIS, SDL_Surface *surface)
{
	return(-1);
}
static void ANDROID_FreeHWSurface(_THIS, SDL_Surface *surface)
{
	return;
}

/* We need to wait for vertical retrace on page flipped displays */
static int ANDROID_LockHWSurface(_THIS, SDL_Surface *surface)
{
	return(0);
}

static void ANDROID_UnlockHWSurface(_THIS, SDL_Surface *surface)
{
	return;
}

static void ANDROID_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
	/* do nothing. */
}

int ANDROID_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	/* do nothing of note. */
	return(1);
}

/* Note:  If we are terminated, this could be called in the middle of
   another SDL video routine -- notably UpdateRects.
*/
void ANDROID_VideoQuit(_THIS)
{
	memX = 0;
	memY = 0;
	memBuffer = NULL;
	int i;
	
	if (this->screen->pixels != NULL)
	{
		SDL_free(this->screen->pixels);
		this->screen->pixels = NULL;
	}
	/* Free video mode lists */
	for ( i=0; i<SDL_NUMMODES; ++i ) {
		if ( SDL_modelist[i] != NULL ) {
			SDL_free(SDL_modelist[i]);
			SDL_modelist[i] = NULL;
		}
	}
	SDL_DestroySemaphore( WaitForNativeRender );
	WaitForNativeRender = NULL;
}

void ANDROID_PumpEvents(_THIS)
{
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Frame is ready to render");
	SDL_SemPost(WaitForNativeRender);
}

/* JNI-C++ wrapper stuff */

extern int main( int argc, char ** argv );
static int SDLCALL MainThreadWrapper(void * dummy)
{
	int argc = 1;
	char * argv[] = { "demo" };
	return main( argc, argv );
};

#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)

extern void
JAVA_EXPORT_NAME(DemoRenderer_nativeInit) ( JNIEnv*  env, jobject  thiz )
{
	mainThread = SDL_CreateThread( MainThreadWrapper, NULL );
}

extern void
JAVA_EXPORT_NAME(DemoRenderer_nativeResize) ( JNIEnv*  env, jobject  thiz, jint w, jint h )
{
    sWindowWidth  = w;
    sWindowHeight = h;
    __android_log_print(ANDROID_LOG_INFO, "libSDL", "resize w=%d h=%d", w, h);
}

/* Call to finalize the graphics state */
extern void
JAVA_EXPORT_NAME(DemoRenderer_nativeDone) ( JNIEnv*  env, jobject  thiz )
{
	if( mainThread )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "quitting...");
		SDL_PrivateQuit();
		SDL_WaitThread(mainThread, NULL);
		mainThread = NULL;
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "quit OK");
	}
}

enum MOUSE_ACTION { MOUSE_DOWN = 0, MOUSE_UP=1, MOUSE_MOVE=2 };

extern void
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeMouse) ( JNIEnv*  env, jobject  thiz, jint x, jint y, jint action )
{
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "mouse event %i at (%03i, %03i)", action, x, y);
	if( action == MOUSE_DOWN || action == MOUSE_UP )
		SDL_PrivateMouseButton( (action == MOUSE_DOWN) ? SDL_PRESSED : SDL_RELEASED, 1, x, y );
	if( action == MOUSE_MOVE )
		SDL_PrivateMouseMotion(0, 0, x, y);
}

static SDL_keysym *TranslateKey(int scancode, SDL_keysym *keysym)
{
	/* Sanity check */
	if ( scancode >= SDL_arraysize(keymap) )
		scancode = KEYCODE_UNKNOWN;

	/* Set the keysym information */
	keysym->scancode = scancode;
	keysym->sym = keymap[scancode];
	keysym->mod = KMOD_NONE;

	/* If UNICODE is on, get the UNICODE value for the key */
	keysym->unicode = 0;
	if ( SDL_TranslateUNICODE ) {
		/* Populate the unicode field with the ASCII value */
		keysym->unicode = scancode;
	}
	return(keysym);
}

void
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeKey) ( JNIEnv*  env, jobject  thiz, jint key, jint action )
{
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "key event %i %s", key, action ? "down" : "up");
	SDL_keysym keysym;
	SDL_PrivateKeyboard( action ? SDL_PRESSED : SDL_RELEASED, TranslateKey(key, &keysym) );
}

gluPerspectivef(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
{
   GLfloat xmin, xmax, ymin, ymax;

   ymax = zNear * tan(fovy * M_PI / 360.0f);
   ymin = -ymax;
   xmin = ymin * aspect;
   xmax = ymax * aspect;
   glFrustumf(xmin, xmax, ymin, ymax, zNear, zFar);
};

/* Call to render the next GL frame */
extern void
JAVA_EXPORT_NAME(DemoRenderer_nativeRender) ( JNIEnv*  env, jobject  thiz )
{
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "rendering frame...");
	
	static float bounceColor = 0.0f;
	static float bounceDir = 1.0f;

	static GLfloat vertexData [] = {
	-0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f
	};
	static GLfloat textureData [] = {
	 0.0f, 0.0f,
	 1.0f, 0.0f,
	 0.0f, 1.0f,
	 1.0f, 1.0f,
	};
	
	char tmp[512];
	
	GLuint texture[1];

	/*
	// Show that we're doing something (just flash the screen)
	bounceColor += 0.005f * bounceDir;
	if( bounceColor >= 1.0f )
		bounceDir = -1.0f;
	if( bounceColor <= 0.0f )
		bounceDir = 1.0f;
	//glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearColor(bounceColor, bounceColor, bounceColor, 0.5f);

	glClear(GL_COLOR_BUFFER_BIT); // Clear Screen Buffer (TODO: comment this out when whole thing start working)
	*/

	if( WaitForNativeRender && memBuffer )
	{
		if( SDL_SemWaitTimeout( WaitForNativeRender, 200 ) == 0 )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Rendering frame");
		}
		else
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Frame skipped");
		}

		// ----- Fail code starts here -----

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DITHER);
		glDisable(GL_MULTISAMPLE);

		glEnable(GL_TEXTURE_2D);

		if( glGetError() != GL_NO_ERROR )
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "glGenTextures() error");

		glGenTextures(1, texture);
		if( glGetError() != GL_NO_ERROR )
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "glGenTextures() error");

		glBindTexture(GL_TEXTURE_2D, texture[0]);
		if( glGetError() != GL_NO_ERROR )
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "glBindTexture() error");

		/*
		glActiveTexture(texture[0]);
		if( glGetError() != GL_NO_ERROR )
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "glActiveTexture() error");

		glClientActiveTexture(texture[0]);
		if( glGetError() != GL_NO_ERROR )
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "glClientActiveTexture() error");
		*/

		//glTexImage2D(GL_TEXTURE_2D, 0, 3, memX, memY, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, memBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, memBuffer);

		int errorNum = glGetError();
		if( errorNum != GL_NO_ERROR )
		{
			sprintf(tmp, "glTexImage2D() error: %i", errorNum);
			__android_log_print(ANDROID_LOG_INFO, "libSDL", tmp);
		}

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

		if( glGetError() != GL_NO_ERROR )
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "glTexParameteri() error");

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		if( glGetError() != GL_NO_ERROR )
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "glEnableClientState() error");


		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspectivef( 90.0f, (float)memX / (float)memY, 0.1f, 100.0f);
	
    	glMatrixMode( GL_MODELVIEW );
    	glLoadIdentity();
		glTranslatef( 0.0f, 0.0f, -1.0f );

		glVertexPointer(3, GL_FLOAT, 0, vertexData);

		if( glGetError() != GL_NO_ERROR )
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "glVertexPointer() error");

		glTexCoordPointer(2, GL_FLOAT, 0, textureData);

		if( glGetError() != GL_NO_ERROR )
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "glTexCoordPointer() error");

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		if( glGetError() != GL_NO_ERROR )
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "glDrawArrays() error");

		//glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		//glDisableClientState(GL_VERTEX_ARRAY);

		glDeleteTextures(1, texture);
		if( glGetError() != GL_NO_ERROR )
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "glDeleteTextures() error");

		// ----- Fail code ends here - it just doesn't output anything -----

	}
	
}

void ANDROID_InitOSKeymap(_THIS)
{
  int i;
	
  /* Initialize the DirectFB key translation table */
  for (i=0; i<SDL_arraysize(keymap); ++i)
    keymap[i] = SDLK_UNKNOWN;

  keymap[KEYCODE_CALL] = SDLK_RCTRL;
  keymap[KEYCODE_ENDCALL] = SDLK_RSHIFT;

  keymap[KEYCODE_UNKNOWN] = SDLK_UNKNOWN;
  keymap[KEYCODE_HOME] = SDLK_HOME;
  keymap[KEYCODE_BACK] = SDLK_ESCAPE;
  keymap[KEYCODE_0] = SDLK_0;
  keymap[KEYCODE_1] = SDLK_1;
  keymap[KEYCODE_2] = SDLK_2;
  keymap[KEYCODE_3] = SDLK_3;
  keymap[KEYCODE_4] = SDLK_4;
  keymap[KEYCODE_5] = SDLK_5;
  keymap[KEYCODE_6] = SDLK_6;
  keymap[KEYCODE_7] = SDLK_7;
  keymap[KEYCODE_8] = SDLK_8;
  keymap[KEYCODE_9] = SDLK_9;
  keymap[KEYCODE_STAR] = SDLK_ASTERISK;
  keymap[KEYCODE_POUND] = SDLK_DOLLAR;
  keymap[KEYCODE_DPAD_UP] = SDLK_UP;
  keymap[KEYCODE_DPAD_DOWN] = SDLK_DOWN;
  keymap[KEYCODE_DPAD_LEFT] = SDLK_LEFT;
  keymap[KEYCODE_DPAD_RIGHT] = SDLK_RIGHT;
  keymap[KEYCODE_DPAD_CENTER] = SDLK_RETURN;
  keymap[KEYCODE_VOLUME_UP] = SDLK_PAGEUP;
  keymap[KEYCODE_VOLUME_DOWN] = SDLK_PAGEDOWN;
  keymap[KEYCODE_CLEAR] = SDLK_CLEAR;
  keymap[KEYCODE_A] = SDLK_a;
  keymap[KEYCODE_B] = SDLK_b;
  keymap[KEYCODE_C] = SDLK_c;
  keymap[KEYCODE_D] = SDLK_d;
  keymap[KEYCODE_E] = SDLK_e;
  keymap[KEYCODE_F] = SDLK_f;
  keymap[KEYCODE_G] = SDLK_g;
  keymap[KEYCODE_H] = SDLK_h;
  keymap[KEYCODE_I] = SDLK_i;
  keymap[KEYCODE_J] = SDLK_j;
  keymap[KEYCODE_K] = SDLK_k;
  keymap[KEYCODE_L] = SDLK_l;
  keymap[KEYCODE_M] = SDLK_m;
  keymap[KEYCODE_N] = SDLK_n;
  keymap[KEYCODE_O] = SDLK_o;
  keymap[KEYCODE_P] = SDLK_p;
  keymap[KEYCODE_Q] = SDLK_q;
  keymap[KEYCODE_R] = SDLK_r;
  keymap[KEYCODE_S] = SDLK_s;
  keymap[KEYCODE_T] = SDLK_t;
  keymap[KEYCODE_U] = SDLK_u;
  keymap[KEYCODE_V] = SDLK_v;
  keymap[KEYCODE_W] = SDLK_w;
  keymap[KEYCODE_X] = SDLK_x;
  keymap[KEYCODE_Y] = SDLK_y;
  keymap[KEYCODE_Z] = SDLK_z;
  keymap[KEYCODE_COMMA] = SDLK_COMMA;
  keymap[KEYCODE_PERIOD] = SDLK_PERIOD;
  keymap[KEYCODE_TAB] = SDLK_TAB;
  keymap[KEYCODE_SPACE] = SDLK_SPACE;
  keymap[KEYCODE_ENTER] = SDLK_RETURN;
  keymap[KEYCODE_DEL] = SDLK_DELETE;
  keymap[KEYCODE_GRAVE] = SDLK_BACKQUOTE;
  keymap[KEYCODE_MINUS] = SDLK_MINUS;
  keymap[KEYCODE_EQUALS] = SDLK_EQUALS;
  keymap[KEYCODE_LEFT_BRACKET] = SDLK_LEFTBRACKET;
  keymap[KEYCODE_RIGHT_BRACKET] = SDLK_RIGHTBRACKET;
  keymap[KEYCODE_BACKSLASH] = SDLK_BACKSLASH;
  keymap[KEYCODE_SEMICOLON] = SDLK_SEMICOLON;
  keymap[KEYCODE_APOSTROPHE] = SDLK_QUOTE;
  keymap[KEYCODE_SLASH] = SDLK_SLASH;
  keymap[KEYCODE_AT] = SDLK_AT;

  keymap[KEYCODE_POWER] = SDLK_POWER;

  keymap[KEYCODE_PLUS] = SDLK_PLUS;

  keymap[KEYCODE_MENU] = SDLK_MENU;

  /*

  keymap[KEYCODE_SYM] = SDLK_SYM;
  keymap[KEYCODE_NUM] = SDLK_NUM;

  keymap[KEYCODE_CAMERA] = SDLK_CAMERA;

  keymap[KEYCODE_SOFT_LEFT] = SDLK_SOFT_LEFT;
  keymap[KEYCODE_SOFT_RIGHT] = SDLK_SOFT_RIGHT;

  keymap[KEYCODE_CALL] = SDLK_CALL;
  keymap[KEYCODE_ENDCALL] = SDLK_ENDCALL;

  keymap[KEYCODE_ALT_LEFT] = SDLK_ALT_LEFT;
  keymap[KEYCODE_ALT_RIGHT] = SDLK_ALT_RIGHT;
  keymap[KEYCODE_SHIFT_LEFT] = SDLK_SHIFT_LEFT;
  keymap[KEYCODE_SHIFT_RIGHT] = SDLK_SHIFT_RIGHT;

  keymap[KEYCODE_EXPLORER] = SDLK_EXPLORER;
  keymap[KEYCODE_ENVELOPE] = SDLK_ENVELOPE;
  keymap[KEYCODE_HEADSETHOOK] = SDLK_HEADSETHOOK;
  keymap[KEYCODE_FOCUS] = SDLK_FOCUS;
  keymap[KEYCODE_NOTIFICATION] = SDLK_NOTIFICATION;
  keymap[KEYCODE_SEARCH] = SDLK_SEARCH;
  keymap[KEYCODE_MEDIA_PLAY_PAUSE=] = SDLK_MEDIA_PLAY_PAUSE=;
  keymap[KEYCODE_MEDIA_STOP] = SDLK_MEDIA_STOP;
  keymap[KEYCODE_MEDIA_NEXT] = SDLK_MEDIA_NEXT;
  keymap[KEYCODE_MEDIA_PREVIOUS] = SDLK_MEDIA_PREVIOUS;
  keymap[KEYCODE_MEDIA_REWIND] = SDLK_MEDIA_REWIND;
  keymap[KEYCODE_MEDIA_FAST_FORWARD] = SDLK_MEDIA_FAST_FORWARD;
  keymap[KEYCODE_MUTE] = SDLK_MUTE;
  */

}


void CrossProd(float x1, float y1, float z1, float x2, float y2, float z2, float res[3])
{
res[0] = y1*z2 - y2*z1;
res[1] = x2*z1 - x1*z2;
res[2] = x1*y2 - x2*y1;
} 

// Stolen from http://www.khronos.org/message_boards/viewtopic.php?t=541
void gluLookAtf(float eyeX, float eyeY, float eyeZ, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ)
{
// i am not using here proper implementation for vectors.
// if you want, you can replace the arrays with your own
// vector types
float f[3];

// calculating the viewing vector
f[0] = lookAtX - eyeX;
f[1] = lookAtY - eyeY;
f[2] = lookAtZ - eyeZ;

float fMag = sqrtf(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);
float upMag = sqrtf(upX*upX + upY*upY + upZ*upZ);

// normalizing the viewing vector
if( fMag != 0)
{
f[0] = f[0]/fMag;
f[1] = f[1]/fMag;
f[2] = f[2]/fMag;
}

// normalising the up vector. no need for this here if you have your
// up vector already normalised, which is mostly the case.
if( upMag != 0 )
{
upX = upX/upMag;
upY = upY/upMag;
upZ = upZ/upMag;
}

float s[3], u[3];

CrossProd(f[0], f[1], f[2], upX, upY, upZ, s);
CrossProd(s[0], s[1], s[2], f[0], f[1], f[2], u);

float M[]=
{
s[0], u[0], -f[0], 0,
s[1], u[1], -f[1], 0,
s[2], u[2], -f[2], 0,
0, 0, 0, 1
};

glMultMatrixf(M);
glTranslatef (-eyeX, -eyeY, -eyeZ);
};


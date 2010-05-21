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

#include <jni.h>
#include <android/log.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <string.h> // for memset()

#include "SDL_config.h"

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_androidvideo.h"


/* Initialization/Query functions */
static int ANDROID_VideoInit(_THIS);
static int ANDROID_GetDisplayBounds(_THIS, SDL_VideoDisplay * display, SDL_Rect * rect);
static void ANDROID_GetDisplayModes(_THIS, SDL_VideoDisplay * display);
static int ANDROID_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode);
static void ANDROID_VideoQuit(_THIS);

static void ANDROID_GL_SwapBuffers(_THIS, SDL_Window * window);
// Stubs
static SDL_GLContext ANDROID_GL_CreateContext(_THIS, SDL_Window * window);
static int ANDROID_GL_MakeCurrent (_THIS, SDL_Window * window, SDL_GLContext context);
static void ANDROID_GL_DeleteContext (_THIS, SDL_GLContext context);


// The device screen dimensions to draw on
static int sWindowWidth  = 320;
static int sWindowHeight = 480;

// Extremely wicked JNI environment to call Java functions from C code
static JNIEnv* JavaEnv = NULL;
static jclass JavaRendererClass = NULL;
static jobject JavaRenderer = NULL;
static jmethodID JavaSwapBuffers = NULL;


static int CallJavaSwapBuffers();
static void SdlGlRenderInit();


/* ANDROID driver bootstrap functions */

static int ANDROID_Available(void)
{
	return 1;
}

static void ANDROID_DeleteDevice(SDL_VideoDevice *device)
{
	SDL_free(device);
}

static SDL_VideoDevice *ANDROID_CreateDevice(int devindex)
{
	SDL_VideoDevice *device;

	/* Initialize all variables that we clean on shutdown */
	device = (SDL_VideoDevice *)SDL_malloc(sizeof(SDL_VideoDevice));
	if ( device ) {
		SDL_memset(device, 0, sizeof (*device));
	}
	if ( (device == NULL) ) {
		SDL_OutOfMemory();
		if ( device ) {
			SDL_free(device);
		}
		return(0);
	}

	/* Set the function pointers */
	device->VideoInit = ANDROID_VideoInit;
	device->GetDisplayBounds = ANDROID_GetDisplayBounds;
	device->GetDisplayModes = ANDROID_GetDisplayModes;
	device->SetDisplayMode = ANDROID_SetDisplayMode;
	device->PumpEvents = ANDROID_PumpEvents;
	device->VideoQuit = ANDROID_VideoQuit;
	device->free = ANDROID_DeleteDevice;
	
	device->GL_SwapWindow = ANDROID_GL_SwapBuffers;
	device->GL_CreateContext = ANDROID_GL_CreateContext;
	device->GL_MakeCurrent = ANDROID_GL_MakeCurrent;
	device->GL_DeleteContext = ANDROID_GL_DeleteContext;

	return device;
}

VideoBootStrap ANDROID_bootstrap = {
	"android", "SDL Android video driver",
	ANDROID_Available, ANDROID_CreateDevice
};


int ANDROID_VideoInit(_THIS)
{
	SDL_VideoDisplay display;
	SDL_DisplayMode mode;

	mode.w = sWindowWidth;
	mode.h = sWindowHeight;
	mode.refresh_rate = 0;
	mode.format = SDL_PIXELFORMAT_RGB565;
	mode.driverdata = NULL;

	SDL_zero(display);
	display.desktop_mode = mode;
	display.current_mode = mode;
	display.driverdata = NULL;
	SDL_AddVideoDisplay(&display);

	return 1;
}


void ANDROID_GetDisplayModes(_THIS, SDL_VideoDisplay * display)
{
	SDL_DisplayMode mode;
	mode.w = sWindowWidth;
	mode.h = sWindowHeight;
	mode.refresh_rate = 0;
	mode.format = SDL_PIXELFORMAT_RGB565;
	mode.driverdata = NULL;
	SDL_AddDisplayMode(display, &mode);

	/*
	struct compatModes_t { int x, int y } compatModes[] =
	{ {800, 600}, {640, 480}, {320, 240}, {320, 200} };
	
	for(int i = 0; i < sizeof(compatModes) / sizeof(compatModes[0]); i++)
		if( sWindowWidth >= compatModes[i].x && sWindowHeight >= compatModes[i].y )
		{
			mode.w = compatModes[i].x;
			mode.h = compatModes[i].y;
			SDL_AddDisplayMode(display, &mode);
		}
	*/
}

int ANDROID_GetDisplayBounds(_THIS, SDL_VideoDisplay * display, SDL_Rect * rect)
{
	rect->w = sWindowWidth;
	rect->h = sWindowHeight;
	return 1;
};

int ANDROID_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode)
{
	return 1;
};


/* Note:  If we are terminated, this could be called in the middle of
   another SDL video routine -- notably UpdateRects.
*/
void ANDROID_VideoQuit(_THIS)
{
}

void ANDROID_PumpEvents(_THIS)
{
}

void ANDROID_GL_SwapBuffers(_THIS, SDL_Window * window)
{
	CallJavaSwapBuffers();
};

SDL_GLContext ANDROID_GL_CreateContext(_THIS, SDL_Window * window)
{
	return (SDL_GLContext)1;
};
int ANDROID_GL_MakeCurrent (_THIS, SDL_Window * window, SDL_GLContext context)
{
	return 1;
};
void ANDROID_GL_DeleteContext (_THIS, SDL_GLContext context)
{
};

/* JNI-C++ wrapper stuff */

#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)

extern void
JAVA_EXPORT_NAME(DemoRenderer_nativeResize) ( JNIEnv*  env, jobject  thiz, jint w, jint h )
{
    sWindowWidth  = w;
    sWindowHeight = h;
    __android_log_print(ANDROID_LOG_INFO, "libSDL", "Physical screen resolution is %dx%d", w, h);
}

/* Call to finalize the graphics state */
extern void
JAVA_EXPORT_NAME(DemoRenderer_nativeDone) ( JNIEnv*  env, jobject  thiz )
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "quitting...");
	SDL_SendQuit();
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "quit OK");
}
void
JAVA_EXPORT_NAME(DemoRenderer_nativeInitJavaCallbacks) ( JNIEnv*  env, jobject thiz )
{
	char classPath[1024];
	JavaEnv = env;
	JavaRenderer = thiz;
	
	JavaRendererClass = (*JavaEnv)->GetObjectClass(JavaEnv, thiz);
	JavaSwapBuffers = (*JavaEnv)->GetMethodID(JavaEnv, JavaRendererClass, "swapBuffers", "()I");
	
	ANDROID_InitOSKeymap();
	
}

int CallJavaSwapBuffers()
{
	return (*JavaEnv)->CallIntMethod( JavaEnv, JavaRenderer, JavaSwapBuffers );
}





/* Stuff from SDL 1.2 */

/*


// Pointer to in-memory video surface
static int memX = 0;
static int memY = 0;
// In-memory surfaces
static void * memBuffer1 = NULL;
static void * memBuffer2 = NULL;
static void * memBuffer = NULL;
static int sdl_opengl = 0;
// Some wicked GLES stuff
static GLuint texture = 0;


SDL_Surface *ANDROID_SetVideoMode(_THIS, SDL_Surface *current,
				int width, int height, int bpp, Uint32 flags)
{
    __android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_SetVideoMode(): application requested mode %dx%d", width, height);

	if ( memBuffer1 )
		SDL_free( memBuffer1 );
	if ( memBuffer2 )
		SDL_free( memBuffer2 );

	memBuffer = memBuffer1 = memBuffer2 = NULL;

	sdl_opengl = (flags & SDL_OPENGL) ? 1 : 0;

	memX = width;
	memY = height;
	
	if( ! sdl_opengl )
	{
		memBuffer1 = SDL_malloc(memX * memY * (bpp / 8));
		if ( ! memBuffer1 ) {
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "Couldn't allocate buffer for requested mode");
			SDL_SetError("Couldn't allocate buffer for requested mode");
			return(NULL);
		}
		SDL_memset(memBuffer1, 0, memX * memY * (bpp / 8));

		if( flags & SDL_DOUBLEBUF )
		{
			memBuffer2 = SDL_malloc(memX * memY * (bpp / 8));
			if ( ! memBuffer2 ) {
				__android_log_print(ANDROID_LOG_INFO, "libSDL", "Couldn't allocate buffer for requested mode");
				SDL_SetError("Couldn't allocate buffer for requested mode");
				return(NULL);
			}
			SDL_memset(memBuffer2, 0, memX * memY * (bpp / 8));
		}
		memBuffer = memBuffer1;
	}

	if ( ! SDL_ReallocFormat(current, bpp, 0, 0, 0, 0) ) {
		if(memBuffer)
			SDL_free(memBuffer);
		memBuffer = NULL;
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Couldn't allocate new pixel format for requested mode");
		SDL_SetError("Couldn't allocate new pixel format for requested mode");
		return(NULL);
	}

	current->flags = (flags & SDL_FULLSCREEN) | (flags & SDL_DOUBLEBUF) | (flags & SDL_OPENGL);
	current->w = width;
	current->h = height;
	current->pitch = memX * (bpp / 8);
	current->pixels = memBuffer;
	
	SdlGlRenderInit();

	return(current);
}

SDL_Rect **ANDROID_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
	if(format->BitsPerPixel != 16)
		return NULL;
	return SDL_modelist;
}


static int ANDROID_FlipHWSurface(_THIS, SDL_Surface *surface)
{
	if( ! sdl_opengl )
	{
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, memX, memY, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, memBuffer);
		if( sWindowHeight < memY || sWindowWidth < memX )
			glDrawTexiOES(0, 0, 1, sWindowWidth, sWindowHeight);  // Larger than screen - shrink to fit
		else
			glDrawTexiOES(0, sWindowHeight-memY, 1, memX, memY);  // Smaller than screen - do not scale, it's faster that way

		if( surface->flags & SDL_DOUBLEBUF )
		{
			if( memBuffer == memBuffer1 )
				memBuffer = memBuffer2;
			else
				memBuffer = memBuffer1;
			surface->pixels = memBuffer;
		}
	}

	CallJavaSwapBuffers();

	processAndroidTrackballKeyDelays( -1, 0 );

	SDL_Delay(10);
	
	return(0);
};


int ANDROID_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	return(1);
}

// TODO: use OpenGL textures here
static int ANDROID_AllocHWSurface(_THIS, SDL_Surface *surface)
{
	return(-1);
}
static void ANDROID_FreeHWSurface(_THIS, SDL_Surface *surface)
{
	return;
}

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
	ANDROID_FlipHWSurface(this, SDL_VideoSurface);
}

void SdlGlRenderInit()
{
	// Set up an array of values to use as the sprite vertices.
	static GLfloat vertices[] =
	{
		0, 0,
		1, 0,
		0, 1,
		1, 1,
	};
	
	// Set up an array of values for the texture coordinates.
	static GLfloat texcoords[] =
	{
		0, 0,
		1, 0,
		0, 1,
		1, 1,
	};
	
	static GLint texcoordsCrop[] =
	{
		0, 0, 0, 0,
	};
	
	static float clearColor = 0.0f;
	static int clearColorDir = 1;
	int textX, textY;
	void * memBufferTemp;
	
	if( !sdl_opengl && memBuffer )
	{
			// Texture sizes should be 2^n
			textX = memX;
			textY = memY;

			if( textX <= 256 )
				textX = 256;
			else if( textX <= 512 )
				textX = 512;
			else
				textX = 1024;

			if( textY <= 256 )
				textY = 256;
			else if( textY <= 512 )
				textY = 512;
			else
				textY = 1024;

			glViewport(0, 0, textX, textY);

			glClearColor(0,0,0,0);
			// Set projection
			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			#if defined(GL_VERSION_ES_CM_1_0)
				#define glOrtho glOrthof
			#endif
			glOrtho( 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f );

			// Now Initialize modelview matrix
			glMatrixMode( GL_MODELVIEW );
			glLoadIdentity();
			
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DITHER);
			glDisable(GL_MULTISAMPLE);

			glEnable(GL_TEXTURE_2D);
			
			glGenTextures(1, &texture);

			glBindTexture(GL_TEXTURE_2D, texture);
	
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			void * textBuffer = SDL_malloc( textX*textY*2 );
			SDL_memset( textBuffer, 0, textX*textY*2 );
			
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textX, textY, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, textBuffer);

			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			glVertexPointer(2, GL_FLOAT, 0, vertices);
			glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			texcoordsCrop[0] = 0;
			texcoordsCrop[1] = memY;
			texcoordsCrop[2] = memX;
			texcoordsCrop[3] = -memY;

			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, texcoordsCrop);
			
			glFinish();
			
			SDL_free( textBuffer );
	}
}

// Stubs to get rid of crashing in OpenGL mode
// The implementation dependent data for the window manager cursor
struct WMcursor {
    int unused ;
};

void ANDROID_FreeWMCursor(_THIS, WMcursor *cursor) {
    SDL_free (cursor);
    return;
}
WMcursor * ANDROID_CreateWMCursor(_THIS, Uint8 *data, Uint8 *mask, int w, int h, int hot_x, int hot_y) {
    WMcursor * cursor;
    cursor = (WMcursor *) SDL_malloc (sizeof (WMcursor)) ;
    if (cursor == NULL) {
        SDL_OutOfMemory () ;
        return NULL ;
    }
    return cursor;
}
int ANDROID_ShowWMCursor(_THIS, WMcursor *cursor) {
    return 1;
}
void ANDROID_WarpWMCursor(_THIS, Uint16 x, Uint16 y) { }
void ANDROID_MoveWMCursor(_THIS, int x, int y) { }

static void ANDROID_UpdateRects(_THIS, int numrects, SDL_Rect *rects);

*/

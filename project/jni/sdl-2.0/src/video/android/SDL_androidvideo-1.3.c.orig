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
#include <dlfcn.h>

#include "SDL_config.h"

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"
#include "SDL_touch.h"
#include "../../events/SDL_touch_c.h"

#include "SDL_androidvideo.h"
#include "SDL_screenkeyboard.h"
#include "../../render/SDL_sysrender.h"


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
static void* ANDROID_GL_GetProcAddress(_THIS, const char *proc);
static int Android_GL_LoadLibrary(_THIS, const char *path);
static void Android_GL_UnloadLibrary(_THIS);
static void ANDROID_PumpEvents(_THIS);

static int ANDROID_CreateWindow(_THIS, SDL_Window * window);
static void ANDROID_DestroyWindow(_THIS, SDL_Window * window);

static void ANDROID_StartTextInput(_THIS);

void * glLibraryHandle = NULL;
void * gl2LibraryHandle = NULL;

static Uint32 SDL_VideoThreadID = 0;
int SDL_ANDROID_InsideVideoThread()
{
	return SDL_VideoThreadID == SDL_ThreadID();
}

/* ANDROID driver bootstrap functions */

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
	
	device->GL_SwapWindow = ANDROID_GL_SwapBuffers;
	device->GL_CreateContext = ANDROID_GL_CreateContext;
	device->GL_MakeCurrent = ANDROID_GL_MakeCurrent;
	device->GL_DeleteContext = ANDROID_GL_DeleteContext;
    device->GL_LoadLibrary = Android_GL_LoadLibrary;
    device->GL_GetProcAddress = ANDROID_GL_GetProcAddress;
    device->GL_UnloadLibrary = Android_GL_UnloadLibrary;
	
	device->CreateWindow = ANDROID_CreateWindow;
	device->DestroyWindow = ANDROID_DestroyWindow;
	
	device->StartTextInput = ANDROID_StartTextInput;
	
	device->free = ANDROID_DeleteDevice;

	glLibraryHandle = dlopen("libGLESv1_CM.so", RTLD_NOW | RTLD_GLOBAL);
	if(SDL_ANDROID_UseGles2)
	{
		gl2LibraryHandle = dlopen("libGLESv2.so", RTLD_NOW | RTLD_GLOBAL);
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Loading libGLESv2.so: %p", gl2LibraryHandle);
	}

	return device;
}

static int
ANDROID_Available(void)
{
	return 1;
}

VideoBootStrap Android_bootstrap = {
	"android", "SDL Android video driver",
	ANDROID_Available, ANDROID_CreateDevice
};


int ANDROID_VideoInit(_THIS)
{
	SDL_VideoDisplay display;
	SDL_DisplayMode mode;
	mode.w = SDL_ANDROID_sWindowWidth;
	mode.h = SDL_ANDROID_sWindowHeight;
	mode.refresh_rate = 0;
	mode.format = SDL_ANDROID_BITSPERPIXEL == 16 ? SDL_PIXELFORMAT_RGB565 : SDL_ANDROID_BITSPERPIXEL == 32 ? SDL_PIXELFORMAT_ABGR8888 : SDL_PIXELFORMAT_RGB24;
	mode.driverdata = NULL;

    if (SDL_AddBasicVideoDisplay(&mode) < 0) {
        return -1;
    }

    SDL_zero(mode);
    SDL_AddDisplayMode(&_this->displays[0], &mode);
	
	SDL_VideoThreadID = SDL_ThreadID();

	return 1;
}


void ANDROID_GetDisplayModes(_THIS, SDL_VideoDisplay * display)
{
	SDL_DisplayMode mode;
	mode.w = SDL_ANDROID_sWindowWidth;
	mode.h = SDL_ANDROID_sWindowHeight;
	mode.refresh_rate = 0;
	mode.format = SDL_ANDROID_BITSPERPIXEL == 16 ? SDL_PIXELFORMAT_RGB565 : SDL_ANDROID_BITSPERPIXEL == 32 ? SDL_PIXELFORMAT_ABGR8888 : SDL_PIXELFORMAT_RGB24;
	mode.driverdata = NULL;
	SDL_AddDisplayMode(display, &mode);
	//mode.w = 640;
	//mode.h = 480;
	//SDL_AddDisplayMode(display, &mode);
}

int ANDROID_GetDisplayBounds(_THIS, SDL_VideoDisplay * display, SDL_Rect * rect)
{
	rect->w = SDL_ANDROID_sWindowWidth;
	rect->h = SDL_ANDROID_sWindowHeight;
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
	SDL_ANDROID_PumpEvents();
}

void ANDROID_GL_SwapBuffers(_THIS, SDL_Window * window)
{
	if( SDL_ANDROID_InsideVideoThread() )
		SDL_ANDROID_CallJavaSwapBuffers();
	// SDL: 1.3 does not clear framebuffer before drawing
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Restore blend mode messed up by screen keyboard
	glColor4f(1, 1, 1, 1);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

extern SDL_Window * ANDROID_CurrentWindow;
SDL_Window * ANDROID_CurrentWindow = NULL;
int ANDROID_CreateWindow(_THIS, SDL_Window * window)
{
	ANDROID_CurrentWindow = window;

    window->x = 0;
    window->y = 0;
    window->w = SDL_ANDROID_sWindowWidth;
    window->h = SDL_ANDROID_sWindowHeight;

    window->flags &= ~SDL_WINDOW_RESIZABLE;     /* window is NEVER resizeable */
    window->flags |= SDL_WINDOW_FULLSCREEN;     /* window is always fullscreen */
    window->flags &= ~SDL_WINDOW_HIDDEN;
    window->flags |= SDL_WINDOW_SHOWN;          /* only one window on Android */
    window->flags |= SDL_WINDOW_INPUT_FOCUS;    /* always has input focus */    

	__android_log_print(ANDROID_LOG_INFO, "libSDL",  "ANDROID_CreateWindow(): ANDROID_CurrentWindow %p", ANDROID_CurrentWindow);

	//SDL_SetTouchFocus(0, window);
};
void ANDROID_DestroyWindow(_THIS, SDL_Window * window)
{
	ANDROID_CurrentWindow = NULL;
};

void ANDROID_StartTextInput(_THIS)
{
	SDL_ANDROID_ToggleScreenKeyboardTextInput("");
}


static void* ANDROID_GL_GetProcAddress(_THIS, const char *proc)
{
	void * func = dlsym(glLibraryHandle, proc);
	if(!func && gl2LibraryHandle)
		func = dlsym(gl2LibraryHandle, proc);
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROID_GL_GetProcAddress(\"%s\"): %p", proc, func);
	return func;
};
int Android_GL_LoadLibrary(_THIS, const char *path)
{
    return 0;
}
void Android_GL_UnloadLibrary(_THIS)
{
}

// TODO: fix that
void SDL_ANDROID_VideoContextLost()
{
	SDL_Renderer * renderer = NULL;
	if(ANDROID_CurrentWindow)
		renderer = SDL_GetRenderer(ANDROID_CurrentWindow);

	if( ! renderer || ! renderer->textures || SDL_GetWindowFlags(ANDROID_CurrentWindow) & SDL_WINDOW_OPENGL )
		return;

	__android_log_print(ANDROID_LOG_INFO, "libSDL",  "SDL_ANDROID_VideoContextLost()");
	/*
	SDL_Texture * tex = renderer->textures;
	while( tex )
	{
		renderer->DestroyTexture( renderer, tex );
		tex = tex->next;
	}
	*/
}

extern void GLES_ResetState(SDL_Renderer *renderer);
extern int GLES_ReinitTextureAndroid(SDL_Renderer * renderer, SDL_Texture * texture, void * data);

void SDL_ANDROID_VideoContextRecreated()
{
	int numTextures = 0;
	SDL_Texture * tex;
	__android_log_print(ANDROID_LOG_INFO, "libSDL",  "SDL_ANDROID_VideoContextRecreated(): enter, ANDROID_CurrentWindow %p", ANDROID_CurrentWindow);
	SDL_Renderer * renderer = NULL;
	if(ANDROID_CurrentWindow)
		renderer = SDL_GetRenderer(ANDROID_CurrentWindow);

	if( ! renderer )
		return;

	//if( renderer->UpdateViewport ) // Re-apply glOrtho() and blend modes
	//	renderer->UpdateViewport(renderer);
	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);
	if(strcmp(info.name, "opengles") == 0) {
		GLES_ResetState(renderer);
		tex = renderer->textures;
		while( tex )
		{
			GLES_ReinitTextureAndroid( renderer, tex, tex->driverdata );
			tex = tex->next;
			numTextures++;
		}
	}

	//SDL_GetWindowSurface(ANDROID_CurrentWindow);
	__android_log_print(ANDROID_LOG_INFO, "libSDL",  "SDL_ANDROID_VideoContextRecreated(): re-created %d textures", numTextures);
}

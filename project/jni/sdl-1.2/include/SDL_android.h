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

#ifndef _SDL_android_h
#define _SDL_android_h

#include "SDL_video.h"
#include "SDL_screenkeyboard.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/* 
Sets callbacks to be called when OS decides to put application to background, and restored to foreground.
*/
typedef void ( * SDL_ANDROID_ApplicationPutToBackgroundCallback_t ) (void);

extern DECLSPEC int SDLCALL SDL_ANDROID_SetApplicationPutToBackgroundCallback(
		SDL_ANDROID_ApplicationPutToBackgroundCallback_t appPutToBackground,
		SDL_ANDROID_ApplicationPutToBackgroundCallback_t appRestored );

/* Use these functions instead of setting volume to 0, that will save CPU and battery on device */
extern DECLSPEC int SDLCALL SDL_ANDROID_PauseAudioPlayback(void);
extern DECLSPEC int SDLCALL SDL_ANDROID_ResumeAudioPlayback(void);

/* Get the advertisement size, position and visibility */
extern DECLSPEC int SDLCALL SDL_ANDROID_GetAdvertisementParams(int * visible, SDL_Rect * position);
/* Control the advertisement placement */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetAdvertisementVisible(int visible);
/* Control the advertisement visibility */
extern DECLSPEC int SDLCALL SDL_ANDROID_SetAdvertisementPosition(int left, int top);

#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif

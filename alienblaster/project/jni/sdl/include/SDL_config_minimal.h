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

#ifndef _SDL_config_minimal_h
#define _SDL_config_minimal_h

#include "SDL_platform.h"

/* This is the minimal configuration that can be used to build SDL */

#include <stdarg.h>
#include <stdint.h>

/*
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef unsigned int size_t;
typedef unsigned long uintptr_t;
*/

#define SDL_AUDIO_DRIVER_DUMMY	1

#define SDL_CDROM_DISABLED	1

#define SDL_JOYSTICK_DISABLED	1

#define SDL_LOADSO_DISABLED	1

#define SDL_THREAD_PTHREAD	1
#define SDL_THREAD_PTHREAD_RECURSIVE_MUTEX	1

#define SDL_TIMER_UNIX	1

#define SDL_VIDEO_DRIVER_ANDROID	1

#define HAVE_STDIO_H	1

#endif /* _SDL_config_minimal_h */

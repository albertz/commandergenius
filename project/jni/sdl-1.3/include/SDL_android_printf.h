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

#ifndef __SDL_android_printf_h
#define __SDL_android_printf_h

#ifdef __cplusplus
extern "C" {
#endif

/* Outputs to logcat */
extern int __SDL_android_printf(const char * fmt, ...) __attribute__((format(printf, 1, 2)));

#define __sF __SDL_fake_stdout

#ifdef __cplusplus
#define cout __SDL_fake_cout
#define cerr __SDL_fake_cerr
#define clog __SDL_fake_clog
}

/* Generates lot of warning messages in standard headers, enable only for ultra debug mode */
/*
#include <stdio.h>
#define printf __SDL_android_printf
*/

#endif

#endif

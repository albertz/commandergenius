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
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <string.h> // for memset()

#include "SDL_config.h"

#include "SDL_version.h"
#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "SDL_events.h"
#if (SDL_VERSION_ATLEAST(1,3,0))
#include "../../events/SDL_events_c.h"
#include "../../events/SDL_keyboard_c.h"
#include "../../events/SDL_mouse_c.h"
#include "SDL_scancode.h"
#include "SDL_compat.h"
#else
#include "SDL_keysym.h"
#include "../../events/SDL_events_c.h"
#endif
#include "SDL_joystick.h"
#include "../../joystick/SDL_sysjoystick.h"
#include "../../joystick/SDL_joystick_c.h"

#include "SDL_androidvideo.h"
#include "SDL_androidinput.h"
#include "jniwrapperstuff.h"

#include "touchscreenfont.h"

enum {
FONT_LEFT = 0, FONT_RIGHT = 1, FONT_UP = 2, FONT_DOWN = 3,
FONT_BTN1 = 4, FONT_BTN2 = 5, FONT_BTN3 = 6, FONT_BTN4 = 7
};

enum { MAX_BUTTONS = 4 } ; // Max amount of custom buttons

static GLshort fontGL[sizeof(font)/sizeof(font[0])][FONT_MAX_LINES_PER_CHAR * 4 + 1];
enum { FONT_CHAR_LINES_COUNT = FONT_MAX_LINES_PER_CHAR * 4 };

void prepareFont(float scale)
{
    int i, idx, count = 0;

	for( idx = 0; idx < sizeof(font)/sizeof(font[0]); idx++ )
	{
		for( i = 0; i < FONT_MAX_LINES_PER_CHAR; i++ )
			if( font[idx][i].x1 == 0 && font[idx][i].y1 == 0 && 
				font[idx][i].x2 == 0 && font[idx][i].y2 == 0 )
				break;
		count = i;
		for (i = 0; i < count; ++i) 
		{
			fontGL[idx][4*i+0] = (GLshort)(x + font[idx][i].x1 * scale);
			fontGL[idx][4*i+1] = (GLshort)(y + font[idx][i].y1 * scale);
			fontGL[idx][4*i+2] = (GLshort)(x + font[idx][i].x2 * scale);
			fontGL[idx][4*i+3] = (GLshort)(y + font[idx][i].y2 * scale);
		}
		fontGL[idx][FONT_CHAR_LINES_COUNT] = count*2;
	}
};

static const float inv255f = 1.0f / 255.0f;

// TODO: use SDL 1.3 renderer routines? It will not be pixel-aligned then, if the screen is resized
void drawChar(int idx, Uint16 x, Uint16 y, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    glColor4f((GLfloat) r * inv255f,
                    (GLfloat) g * inv255f,
                    (GLfloat) b * inv255f,
                    (GLfloat) a * inv255f);

    glVertexPointer(2, GL_SHORT, 0, vertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP, 0, fontGL[idx][FONT_CHAR_LINES_COUNT]);
    glDisableClientState(GL_VERTEX_ARRAY);
}

static SDL_Rect arrows, buttons[MAX_BUTTONS];
static int nbuttons;
static SDLKey buttonKeysyms[MAX_BUTTONS] = { 
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_0)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_1)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_2)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_3))
};

enum { ARROW_LEFT = 1, ARROW_RIGHT = 2, ARROW_UP = 4, ARROW_DOWN = 8 };
static int arrowsPressedMask = 0;

static inline int InsideRect(const SDL_Rect * r, int x, int y)
{
	return ( x >= r->x && x <= r->x + r->w ) && ( y >= r->y && y <= r->y + r->h );
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetupScreenKeyboard) ( JNIEnv*  env, jobject thiz, jint size, jint _nbuttons )
{
	int i;
	nbuttons = _nbuttons;
	// TODO: works for horizontal screen orientation only!
	// TODO: configurable keyboard size

	// Arrows to the lower-left part of screen
	arrows.x = 0;
	arrows.w = SDL_ANDROID_sWindowWidth / 2;
	arrows.h = arrows.w;
	arrows.y = SDL_ANDROID_sWindowHeight - arrows.h;

	// Main button to the lower-right
	buttons[0].x = SDL_ANDROID_sWindowWidth / 2;
	buttons[0].w = SDL_ANDROID_sWindowWidth / 2;
	buttons[0].y = SDL_ANDROID_sWindowHeight / 2;
	buttons[0].h = SDL_ANDROID_sWindowHeight / 2;
	
	// Row of secondary buttons to the upper-right
	for( i = 1; i < nbuttons; i++ )
	{
		buttons[i].y = 0;
		buttons[i].h = SDL_ANDROID_sWindowHeight / 2;
		buttons[i].w = (SDL_ANDROID_sWindowWidth / 2) / (nbuttons - 1);
		buttons[i].x = SDL_ANDROID_sWindowWidth / 2 + buttons[i].w * (i - 1);
	}
};

int SDL_android_drawTouchscreenKeyboard()
{
	// TODO: draw something here
};

static SDL_Rect * OldCoords[MAX_MULTITOUCH_POINTERS] = { NULL };

int SDL_android_processTouchscreenKeyboard(int x, int y, int action, int pointerId)
{
	int i;
	if( action == MOUSE_DOWN )
	{
		if( InsideRect( &arrows, x, y ) )
		{
			OldCoords[pointerId] = &arrows;
			// TODO: processing
			return 1;
		}

		for( int i = 0; i < nbuttons; i++ )
		{
			if( InsideRect( &buttons[i], x, y) )
			{
				OldCoords[pointerId] = &buttons[i];
				SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym(buttonKeysyms[i]) ,&keysym) );
				return 1;
			}
		}
	}
	if( action == MOUSE_UP )
	{
		if( OldCoords[pointerId] == &arrows )
		{
			OldCoords[pointerId] = NULL;
			// TODO: processing
			return 1;
		}
		for( int i = 0; i < nbuttons; i++ )
		{
			if( OldCoords[pointerId] == &buttons[i] )
			{
				SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym(buttonKeysyms[i]) ,&keysym) );
				OldCoords[pointerId] = NULL;
				return 1;
			}
		}
	}
	if( action == MOUSE_MOVE )
	{
		if( OldCoords[pointerId] && !InsideRect(OldCoords[pointerId], x, y) )
		{
			SDL_android_processTouchscreenKeyboard(x, y, MOUSE_UP, pointerId);
			return SDL_android_processTouchscreenKeyboard(x, y, MOUSE_DOWN, pointerId);
		}
	}
	return 0;
};

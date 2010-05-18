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
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <string.h> // for memset()


static SDLKey keymap[KEYCODE_LAST+1];

/* JNI-C++ wrapper stuff */

#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)


extern void
JAVA_EXPORT_NAME(AccelerometerReader_nativeAccelerometer) ( JNIEnv*  env, jobject  thiz, jfloat accX, jfloat accY, jfloat accZ )
{
	// TODO: use accelerometer as joystick
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
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeKey) ( JNIEnv*  env, jobject thiz, jint key, jint action )
{
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "key event %i %s", key, action ? "down" : "up");
	SDL_keysym keysym;
	if( ! processAndroidTrackballKeyDelays(key, action) )
		SDL_PrivateKeyboard( action ? SDL_PRESSED : SDL_RELEASED, TranslateKey(key, &keysym) );
}


void ANDROID_InitOSKeymap(_THIS)
{
  int i;
	
  /* Initialize the DirectFB key translation table */
  for (i=0; i<SDL_arraysize(keymap); ++i)
    keymap[i] = SDLK_UNKNOWN;

  keymap[KEYCODE_UNKNOWN] = SDLK_UNKNOWN;

  keymap[KEYCODE_BACK] = SDLK_ESCAPE;

  keymap[KEYCODE_MENU] = SDLK_LALT;
  keymap[KEYCODE_CALL] = SDLK_LCTRL;
  keymap[KEYCODE_ENDCALL] = SDLK_LSHIFT;
  keymap[KEYCODE_CAMERA] = SDLK_RSHIFT;
  keymap[KEYCODE_POWER] = SDLK_RALT;

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

  keymap[KEYCODE_SOFT_LEFT] = SDLK_KP4;
  keymap[KEYCODE_SOFT_RIGHT] = SDLK_KP6;
  keymap[KEYCODE_ENTER] = SDLK_KP_ENTER;

  keymap[KEYCODE_VOLUME_UP] = SDLK_PAGEUP;
  keymap[KEYCODE_VOLUME_DOWN] = SDLK_PAGEDOWN;
  keymap[KEYCODE_SEARCH] = SDLK_END;
  keymap[KEYCODE_HOME] = SDLK_HOME;

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

  keymap[KEYCODE_PLUS] = SDLK_PLUS;

  /*

  keymap[KEYCODE_SYM] = SDLK_SYM;
  keymap[KEYCODE_NUM] = SDLK_NUM;

  keymap[KEYCODE_SOFT_LEFT] = SDLK_SOFT_LEFT;
  keymap[KEYCODE_SOFT_RIGHT] = SDLK_SOFT_RIGHT;

  keymap[KEYCODE_ALT_LEFT] = SDLK_ALT_LEFT;
  keymap[KEYCODE_ALT_RIGHT] = SDLK_ALT_RIGHT;
  keymap[KEYCODE_SHIFT_LEFT] = SDLK_SHIFT_LEFT;
  keymap[KEYCODE_SHIFT_RIGHT] = SDLK_SHIFT_RIGHT;

  keymap[KEYCODE_EXPLORER] = SDLK_EXPLORER;
  keymap[KEYCODE_ENVELOPE] = SDLK_ENVELOPE;
  keymap[KEYCODE_HEADSETHOOK] = SDLK_HEADSETHOOK;
  keymap[KEYCODE_FOCUS] = SDLK_FOCUS;
  keymap[KEYCODE_NOTIFICATION] = SDLK_NOTIFICATION;
  keymap[KEYCODE_MEDIA_PLAY_PAUSE=] = SDLK_MEDIA_PLAY_PAUSE=;
  keymap[KEYCODE_MEDIA_STOP] = SDLK_MEDIA_STOP;
  keymap[KEYCODE_MEDIA_NEXT] = SDLK_MEDIA_NEXT;
  keymap[KEYCODE_MEDIA_PREVIOUS] = SDLK_MEDIA_PREVIOUS;
  keymap[KEYCODE_MEDIA_REWIND] = SDLK_MEDIA_REWIND;
  keymap[KEYCODE_MEDIA_FAST_FORWARD] = SDLK_MEDIA_FAST_FORWARD;
  keymap[KEYCODE_MUTE] = SDLK_MUTE;
  */

}

static int AndroidTrackballKeyDelays[4] = {0,0,0,0};

// Key = -1 if we want to send KeyUp events from main loop
int processAndroidTrackballKeyDelays( int key, int action )
{
	#if ! defined(SDL_TRACKBALL_KEYUP_DELAY) || (SDL_TRACKBALL_KEYUP_DELAY == 0)
	return 0;
	#else
	// Send Directional Pad Up events with a delay, so app wil lthink we're holding the key a bit
	static const int KeysMapping[4] = {KEYCODE_DPAD_UP, KEYCODE_DPAD_DOWN, KEYCODE_DPAD_LEFT, KEYCODE_DPAD_RIGHT};
	int idx, idx2;
	SDL_keysym keysym;
	
	if( key < 0 )
	{
		for( idx = 0; idx < 4; idx ++ )
		{
			if( AndroidTrackballKeyDelays[idx] > 0 )
			{
				AndroidTrackballKeyDelays[idx] --;
				if( AndroidTrackballKeyDelays[idx] == 0 )
					SDL_PrivateKeyboard( SDL_RELEASED, TranslateKey(KeysMapping[idx], &keysym) );
			}
		}
	}
	else
	{
		idx = -1;
		// Too lazy to do switch or function
		if( key == KEYCODE_DPAD_UP )
			idx = 0;
		else if( key == KEYCODE_DPAD_DOWN )
			idx = 1;
		else if( key == KEYCODE_DPAD_LEFT )
			idx = 2;
		else if( key == KEYCODE_DPAD_RIGHT )
			idx = 3;
		if( idx >= 0 )
		{
			if( action && AndroidTrackballKeyDelays[idx] == 0 )
			{
				// User pressed key for the first time
				idx2 = (idx + 2) % 4; // Opposite key for current key - if it's still pressing, release it
				if( AndroidTrackballKeyDelays[idx2] > 0 )
				{
					AndroidTrackballKeyDelays[idx2] = 0;
					SDL_PrivateKeyboard( SDL_RELEASED, TranslateKey(KeysMapping[idx2], &keysym) );
				}
				SDL_PrivateKeyboard( SDL_PRESSED, TranslateKey(key, &keysym) );
			}
			else if( !action && AndroidTrackballKeyDelays[idx] == 0 )
			{
				// User released key - make a delay, do not send release event
				AndroidTrackballKeyDelays[idx] = SDL_TRACKBALL_KEYUP_DELAY;
			}
			else if( action && AndroidTrackballKeyDelays[idx] > 0 )
			{
				// User pressed key another time - add some more time for key to be pressed
				AndroidTrackballKeyDelays[idx] += SDL_TRACKBALL_KEYUP_DELAY;
				if( AndroidTrackballKeyDelays[idx] < SDL_TRACKBALL_KEYUP_DELAY * 4 )
					AndroidTrackballKeyDelays[idx] = SDL_TRACKBALL_KEYUP_DELAY * 4;
			}
			return 1;
		}
	}
	return 0;
	
	#endif
}

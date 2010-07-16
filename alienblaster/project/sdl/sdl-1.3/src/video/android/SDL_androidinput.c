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
#include "../../events/SDL_events_c.h"
#include "../../events/SDL_keyboard_c.h"
#include "../../events/SDL_mouse_c.h"

#include "SDL_androidvideo.h"
#include "SDL_scancode.h"
#include "SDL_compat.h"


static SDLKey keymap[KEYCODE_LAST+1];

/* JNI-C++ wrapper stuff */

#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)



enum MOUSE_ACTION { MOUSE_DOWN = 0, MOUSE_UP=1, MOUSE_MOVE=2 };

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeMouse) ( JNIEnv*  env, jobject  thiz, jint x, jint y, jint action )
{
	if( action == MOUSE_DOWN || action == MOUSE_UP )
	{
#if SDL_VERSION_ATLEAST(1,3,0)
		SDL_SendMouseMotion(NULL, 0, x, y);
		SDL_SendMouseButton(NULL, (action == MOUSE_DOWN) ? SDL_PRESSED : SDL_RELEASED, 1 );
#else
		SDL_PrivateMouseMotion(0, 0, x, y);
		SDL_PrivateMouseButton( (action == MOUSE_DOWN) ? SDL_PRESSED : SDL_RELEASED, 1, x, y );
#endif
	}
	if( action == MOUSE_MOVE )
#if SDL_VERSION_ATLEAST(1,3,0)
		SDL_SendMouseMotion(NULL, 0, x, y);
#else
		SDL_PrivateMouseMotion(0, 0, x, y);
#endif
}

#if SDL_VERSION_ATLEAST(1,3,0)

#define SDL_KEY(X) SDL_SCANCODE_ ## X

#else

#define SDL_KEY2(X) SDLK_ ## X
#define SDL_KEY(X) SDL_KEY2(X)

#define SDL_SendKeyboardKey SDL_PrivateKeyboard

// Randomly redefining SDL 1.3 scancodes to SDL 1.2 keycodes
#define KP_0 KP0
#define KP_1 KP1
#define KP_2 KP2
#define KP_3 KP3
#define KP_4 KP4
#define KP_5 KP5
#define KP_6 KP6
#define KP_7 KP7
#define KP_8 KP8
#define KP_9 KP9
#define NUMLOCKCLEAR NUMLOCK
#define GRAVE DOLLAR

#endif

static SDL_scancode TranslateKey(int scancode)
{
	if ( scancode >= SDL_arraysize(keymap) )
		scancode = KEYCODE_UNKNOWN;
	return keymap[scancode];
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeKey) ( JNIEnv*  env, jobject thiz, jint key, jint action )
{
	//if( ! processAndroidTrackballKeyDelays(key, action) )
	SDL_SendKeyboardKey( action ? SDL_PRESSED : SDL_RELEASED, TranslateKey(key) );
}


static void updateOrientation ( float accX, float accY, float accZ )
{
	// TODO: use accelerometer as joystick, make this configurable
	// Currenly it's used as cursor + KP7/KP9 keys
	static const float dx = 0.04, dy = 0.1, dz = 0.1;

	static float midX = 0, midY = 0, midZ = 0;
	static int pressLeft = 0, pressRight = 0, pressUp = 0, pressDown = 0, pressR = 0, pressL = 0;
	
	midX = 0.0f; // Do not remember old value for phone tilt, it feels weird
	
	if( accX < midX - dx )
	{
		if( !pressLeft )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: press left, acc %f mid %f d %f", accX, midX, dx);
			pressLeft = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, SDL_SCANCODE_LEFT );
		}
	}
	else
	{
		if( pressLeft )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: release left, acc %f mid %f d %f", accX, midX, dx);
			pressLeft = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, SDL_SCANCODE_LEFT );
		}
	}
	if( accX < midX - dx*2 )
		midX = accX + dx*2;

	if( accX > midX + dx )
	{
		if( !pressRight )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: press right, acc %f mid %f d %f", accX, midX, dx);
			pressRight = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, SDL_SCANCODE_RIGHT );
		}
	}
	else
	{
		if( pressRight )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: release right, acc %f mid %f d %f", accX, midX, dx);
			pressRight = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, SDL_SCANCODE_RIGHT );
		}
	}
	if( accX > midX + dx*2 )
		midX = accX - dx*2;

	if( accY < midY - dy )
	{
		if( !pressUp )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: press up, acc %f mid %f d %f", accY, midY, dy);
			pressUp = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, SDL_SCANCODE_DOWN );
		}
	}
	else
	{
		if( pressUp )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: release up, acc %f mid %f d %f", accY, midY, dy);
			pressUp = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, SDL_SCANCODE_DOWN );
		}
	}
	if( accY < midY - dy*2 )
		midY = accY + dy*2;

	if( accY > midY + dy )
	{
		if( !pressDown )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: press down, acc %f mid %f d %f", accY, midY, dy);
			pressDown = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, SDL_SCANCODE_UP );
		}
	}
	else
	{
		if( pressDown )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: release down, acc %f mid %f d %f", accY, midY, dy);
			pressDown = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, SDL_SCANCODE_UP );
		}
	}
	if( accY > midY + dy*2 )
		midY = accY - dy*2;

	if( accZ < midZ - dz )
	{
		if( !pressL )
		{
			pressL = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, SDL_SCANCODE_KP_7 );
		}
	}
	else
	{
		if( pressL )
		{
			pressL = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, SDL_SCANCODE_KP_7 );
		}
	}
	if( accZ < midZ - dz*2 )
		midZ = accZ + dz*2;

	if( accZ > midZ + dz )
	{
		if( !pressR )
		{
			pressR = 1;
			SDL_SendKeyboardKey( SDL_PRESSED, SDL_SCANCODE_KP_9 );
		}
	}
	else
	{
		if( pressR )
		{
			pressR = 0;
			SDL_SendKeyboardKey( SDL_RELEASED, SDL_SCANCODE_KP_9 );
		}
	}
	if( accZ > midZ + dz*2 )
		midZ = accZ - dz*2;

}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(AccelerometerReader_nativeAccelerometer) ( JNIEnv*  env, jobject  thiz, jfloat accPosX, jfloat accPosY, jfloat accPosZ )
{
	// Calculate two angles from three coordinates - TODO: this is faulty!
	//float accX = atan2f(-accPosX, sqrtf(accPosY*accPosY+accPosZ*accPosZ) * ( accPosY > 0 ? 1.0f : -1.0f ) ) * M_1_PI * 180.0f;
	//float accY = atan2f(accPosZ, accPosY) * M_1_PI;
	float normal = sqrt(accPosX*accPosX+accPosY*accPosY+accPosZ*accPosZ);
	if(normal <= 0.0000001f)
		normal = 1.0f;
	/*
	static int count = 0;
	count++;
	if(count > 50)
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accel: %-1.3f %-1.3f %-1.3f", accPosX/normal, accPosY/normal, accPosZ/normal);
	}
	*/
	
	updateOrientation (accPosX/normal, accPosY/normal, 0.0f);
}


JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(AccelerometerReader_nativeOrientation) ( JNIEnv*  env, jobject  thiz, jfloat accX, jfloat accY, jfloat accZ )
{
	updateOrientation (accX, accY, accZ);
}

void ANDROID_InitOSKeymap()
{
  int i;

#if (SDL_VERSION_ATLEAST(1,3,0))
  SDLKey defaultKeymap[SDL_NUM_SCANCODES];
  SDL_GetDefaultKeymap(defaultKeymap);
  SDL_SetKeymap(0, defaultKeymap, SDL_NUM_SCANCODES);
#endif

  // TODO: keys are mapped rather randomly

  for (i=0; i<SDL_arraysize(keymap); ++i)
    keymap[i] = SDL_KEY(UNKNOWN);

  keymap[KEYCODE_UNKNOWN] = SDL_KEY(UNKNOWN);

  keymap[KEYCODE_BACK] = SDL_KEY(ESCAPE);

  // HTC Evo has only two keys - Menu and Back, and all games require Enter. (Also Volume Up/Down, but they are hard to reach) 
  // TODO: make this configurable
  keymap[KEYCODE_MENU] = SDL_KEY(RETURN);

  keymap[KEYCODE_CALL] = SDL_KEY(LCTRL);
  keymap[KEYCODE_ENDCALL] = SDL_KEY(LSHIFT);
  keymap[KEYCODE_CAMERA] = SDL_KEY(RSHIFT);
  keymap[KEYCODE_POWER] = SDL_KEY(RALT);

  keymap[KEYCODE_0] = SDL_KEY(0);
  keymap[KEYCODE_1] = SDL_KEY(1);
  keymap[KEYCODE_2] = SDL_KEY(2);
  keymap[KEYCODE_3] = SDL_KEY(3);
  keymap[KEYCODE_4] = SDL_KEY(4);
  keymap[KEYCODE_5] = SDL_KEY(5);
  keymap[KEYCODE_6] = SDL_KEY(6);
  keymap[KEYCODE_7] = SDL_KEY(7);
  keymap[KEYCODE_8] = SDL_KEY(8);
  keymap[KEYCODE_9] = SDL_KEY(9);
  keymap[KEYCODE_STAR] = SDL_KEY(KP_DIVIDE);
  keymap[KEYCODE_POUND] = SDL_KEY(KP_MULTIPLY);

  keymap[KEYCODE_DPAD_UP] = SDL_KEY(UP);
  keymap[KEYCODE_DPAD_DOWN] = SDL_KEY(DOWN);
  keymap[KEYCODE_DPAD_LEFT] = SDL_KEY(LEFT);
  keymap[KEYCODE_DPAD_RIGHT] = SDL_KEY(RIGHT);
  keymap[KEYCODE_DPAD_CENTER] = SDL_KEY(RCTRL);

  keymap[KEYCODE_SOFT_LEFT] = SDL_KEY(KP_4);
  keymap[KEYCODE_SOFT_RIGHT] = SDL_KEY(KP_6);
  keymap[KEYCODE_ENTER] = SDL_KEY(KP_ENTER);

  keymap[KEYCODE_VOLUME_UP] = SDL_KEY(PAGEUP);
  keymap[KEYCODE_VOLUME_DOWN] = SDL_KEY(PAGEDOWN);
  keymap[KEYCODE_SEARCH] = SDL_KEY(END);
  keymap[KEYCODE_HOME] = SDL_KEY(HOME);

  keymap[KEYCODE_CLEAR] = SDL_KEY(BACKSPACE);
  keymap[KEYCODE_A] = SDL_KEY(A);
  keymap[KEYCODE_B] = SDL_KEY(B);
  keymap[KEYCODE_C] = SDL_KEY(C);
  keymap[KEYCODE_D] = SDL_KEY(D);
  keymap[KEYCODE_E] = SDL_KEY(E);
  keymap[KEYCODE_F] = SDL_KEY(F);
  keymap[KEYCODE_G] = SDL_KEY(G);
  keymap[KEYCODE_H] = SDL_KEY(H);
  keymap[KEYCODE_I] = SDL_KEY(I);
  keymap[KEYCODE_J] = SDL_KEY(J);
  keymap[KEYCODE_K] = SDL_KEY(K);
  keymap[KEYCODE_L] = SDL_KEY(L);
  keymap[KEYCODE_M] = SDL_KEY(M);
  keymap[KEYCODE_N] = SDL_KEY(N);
  keymap[KEYCODE_O] = SDL_KEY(O);
  keymap[KEYCODE_P] = SDL_KEY(P);
  keymap[KEYCODE_Q] = SDL_KEY(Q);
  keymap[KEYCODE_R] = SDL_KEY(R);
  keymap[KEYCODE_S] = SDL_KEY(S);
  keymap[KEYCODE_T] = SDL_KEY(T);
  keymap[KEYCODE_U] = SDL_KEY(U);
  keymap[KEYCODE_V] = SDL_KEY(V);
  keymap[KEYCODE_W] = SDL_KEY(W);
  keymap[KEYCODE_X] = SDL_KEY(X);
  keymap[KEYCODE_Y] = SDL_KEY(Y);
  keymap[KEYCODE_Z] = SDL_KEY(Z);
  keymap[KEYCODE_COMMA] = SDL_KEY(COMMA);
  keymap[KEYCODE_PERIOD] = SDL_KEY(PERIOD);
  keymap[KEYCODE_TAB] = SDL_KEY(TAB);
  keymap[KEYCODE_SPACE] = SDL_KEY(SPACE);
  keymap[KEYCODE_DEL] = SDL_KEY(DELETE);
  keymap[KEYCODE_GRAVE] = SDL_KEY(GRAVE);
  keymap[KEYCODE_MINUS] = SDL_KEY(KP_MINUS);
  keymap[KEYCODE_PLUS] = SDL_KEY(KP_PLUS);
  keymap[KEYCODE_EQUALS] = SDL_KEY(EQUALS);
  keymap[KEYCODE_LEFT_BRACKET] = SDL_KEY(LEFTBRACKET);
  keymap[KEYCODE_RIGHT_BRACKET] = SDL_KEY(RIGHTBRACKET);
  keymap[KEYCODE_BACKSLASH] = SDL_KEY(BACKSLASH);
  keymap[KEYCODE_SEMICOLON] = SDL_KEY(SEMICOLON);
  keymap[KEYCODE_APOSTROPHE] = SDL_KEY(APOSTROPHE);
  keymap[KEYCODE_SLASH] = SDL_KEY(SLASH);
  keymap[KEYCODE_AT] = SDL_KEY(KP_PERIOD);

  keymap[KEYCODE_MEDIA_PLAY_PAUSE] = SDL_KEY(KP_2);
  keymap[KEYCODE_MEDIA_STOP] = SDL_KEY(HELP);
  keymap[KEYCODE_MEDIA_NEXT] = SDL_KEY(KP_8);
  keymap[KEYCODE_MEDIA_PREVIOUS] = SDL_KEY(KP_5);
  keymap[KEYCODE_MEDIA_REWIND] = SDL_KEY(KP_1);
  keymap[KEYCODE_MEDIA_FAST_FORWARD] = SDL_KEY(KP_3);
  keymap[KEYCODE_MUTE] = SDL_KEY(KP_0);

  keymap[KEYCODE_SYM] = SDL_KEY(LGUI);
  keymap[KEYCODE_NUM] = SDL_KEY(NUMLOCKCLEAR);

  // TODO: Too lazy to define that
/*
  keymap[KEYCODE_ALT_LEFT] = SDL_KEY(AC_BACK);
  keymap[KEYCODE_ALT_RIGHT] = SDL_KEY(AC_FORWARD);
  keymap[KEYCODE_SHIFT_LEFT] = SDL_KEY(VOLUMEUP);
  keymap[KEYCODE_SHIFT_RIGHT] = SDL_KEY(VOLUMEDOWN);

  keymap[KEYCODE_EXPLORER] = SDL_KEY(WWW);
  keymap[KEYCODE_ENVELOPE] = SDL_KEY(MAIL);

  keymap[KEYCODE_HEADSETHOOK] = SDL_KEY(AC_SEARCH);
  keymap[KEYCODE_FOCUS] = SDL_KEY(AC_REFRESH);
  keymap[KEYCODE_NOTIFICATION] = SDL_KEY(AC_BOOKMARKS);
*/

}


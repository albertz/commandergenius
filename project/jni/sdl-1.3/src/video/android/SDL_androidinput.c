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
#include "SDL_mutex.h"
#include "SDL_events.h"
#if SDL_VERSION_ATLEAST(1,3,0)
#include "SDL_touch.h"
#include "../../events/SDL_touch_c.h"
#endif

#include "../SDL_sysvideo.h"
#include "SDL_androidvideo.h"
#include "SDL_androidinput.h"
#include "jniwrapperstuff.h"


static SDLKey SDL_android_keymap[KEYCODE_LAST+1];

static inline SDL_scancode TranslateKey(int scancode)
{
	if ( scancode >= SDL_arraysize(SDL_android_keymap) )
		scancode = KEYCODE_UNKNOWN;
	return SDL_android_keymap[scancode];
}


static int isTrackballUsed = 0;
static int isMouseUsed = 0;

enum { RIGHT_CLICK_NONE = 0, RIGHT_CLICK_WITH_MULTITOUCH = 1, RIGHT_CLICK_WITH_PRESSURE = 2, RIGHT_CLICK_WITH_MENU_BUTTON = 3 };
enum { LEFT_CLICK_NORMAL = 0, LEFT_CLICK_NEAR_CURSOR = 1, LEFT_CLICK_WITH_MULTITOUCH = 2, LEFT_CLICK_WITH_PRESSURE = 3, LEFT_CLICK_WITH_DPAD = 4 };
static int leftClickMethod = LEFT_CLICK_NORMAL;
static int rightClickMethod = RIGHT_CLICK_NONE;
int SDL_ANDROID_ShowScreenUnderFinger = 0;
SDL_Rect SDL_ANDROID_ShowScreenUnderFingerRect = {0, 0, 0, 0}, SDL_ANDROID_ShowScreenUnderFingerRectSrc = {0, 0, 0, 0};
static int moveMouseWithArrowKeys = 0;
static int clickMouseWithDpadCenter = 0;
static int moveMouseWithKbSpeed = 0;
static int moveMouseWithKbAccel = 0;
static int moveMouseWithKbX = -1, moveMouseWithKbY = -1;
static int moveMouseWithKbSpeedX = 0, moveMouseWithKbSpeedY = 0;
static int moveMouseWithKbAccelX = 0, moveMouseWithKbAccelY = 0;
static int moveMouseWithKbAccelUpdateNeeded = 0;
static int maxForce = 0;
static int maxRadius = 0;
int SDL_ANDROID_isJoystickUsed = 0;
static int isMultitouchUsed = 0;
SDL_Joystick *SDL_ANDROID_CurrentJoysticks[MAX_MULTITOUCH_POINTERS+1] = {NULL};
static int TrackballDampening = 0; // in milliseconds
static Uint32 lastTrackballAction = 0;
enum { TOUCH_PTR_UP = 0, TOUCH_PTR_MOUSE = 1, TOUCH_PTR_SCREENKB = 2 };
int touchPointers[MAX_MULTITOUCH_POINTERS] = {0};
int firstMousePointerId = -1;


static inline int InsideRect(const SDL_Rect * r, int x, int y)
{
	return ( x >= r->x && x <= r->x + r->w ) && ( y >= r->y && y <= r->y + r->h );
}

void UpdateScreenUnderFingerRect(int x, int y)
{
#if SDL_VERSION_ATLEAST(1,3,0)
	return;
	/*
	int screenX = 320, screenY = 240;
	if( !SDL_ANDROID_ShowScreenUnderFinger )
		return;

	SDL_Window * window = SDL_GetFocusWindow();
	if( window && window->renderer->window ) {
		screenX = window->w;
		screenY = window->h;
	}
	*/
#else
	int screenX = SDL_ANDROID_sFakeWindowWidth, screenY = SDL_ANDROID_sFakeWindowHeight;
	if( !SDL_ANDROID_ShowScreenUnderFinger )
		return;

	SDL_ANDROID_ShowScreenUnderFingerRectSrc.w = screenX / 4;
	SDL_ANDROID_ShowScreenUnderFingerRectSrc.h = screenY / 4;
	SDL_ANDROID_ShowScreenUnderFingerRectSrc.x = x - SDL_ANDROID_ShowScreenUnderFingerRectSrc.w/2;
	SDL_ANDROID_ShowScreenUnderFingerRectSrc.y = y - SDL_ANDROID_ShowScreenUnderFingerRectSrc.h/2;
	if( SDL_ANDROID_ShowScreenUnderFingerRectSrc.x < 0 )
		SDL_ANDROID_ShowScreenUnderFingerRectSrc.x = 0;
	if( SDL_ANDROID_ShowScreenUnderFingerRectSrc.y < 0 )
		SDL_ANDROID_ShowScreenUnderFingerRectSrc.y = 0;
	if( SDL_ANDROID_ShowScreenUnderFingerRectSrc.x > screenX - SDL_ANDROID_ShowScreenUnderFingerRectSrc.w )
		SDL_ANDROID_ShowScreenUnderFingerRectSrc.x = screenX - SDL_ANDROID_ShowScreenUnderFingerRectSrc.w;
	if( SDL_ANDROID_ShowScreenUnderFingerRectSrc.y > screenY - SDL_ANDROID_ShowScreenUnderFingerRectSrc.h )
		SDL_ANDROID_ShowScreenUnderFingerRectSrc.y = screenY - SDL_ANDROID_ShowScreenUnderFingerRectSrc.h;
	
	SDL_ANDROID_ShowScreenUnderFingerRect.w = SDL_ANDROID_ShowScreenUnderFingerRectSrc.w * 3 / 2;
	SDL_ANDROID_ShowScreenUnderFingerRect.h = SDL_ANDROID_ShowScreenUnderFingerRectSrc.h * 3 / 2;
	SDL_ANDROID_ShowScreenUnderFingerRect.x = x + SDL_ANDROID_ShowScreenUnderFingerRect.w/10;
	SDL_ANDROID_ShowScreenUnderFingerRect.y = y - SDL_ANDROID_ShowScreenUnderFingerRect.h*11/10;
	if( SDL_ANDROID_ShowScreenUnderFingerRect.x < 0 )
		SDL_ANDROID_ShowScreenUnderFingerRect.x = 0;
	if( SDL_ANDROID_ShowScreenUnderFingerRect.y < 0 )
		SDL_ANDROID_ShowScreenUnderFingerRect.y = 0;
	if( SDL_ANDROID_ShowScreenUnderFingerRect.x + SDL_ANDROID_ShowScreenUnderFingerRect.w >= screenX )
		SDL_ANDROID_ShowScreenUnderFingerRect.x = screenX - SDL_ANDROID_ShowScreenUnderFingerRect.w - 1;
	if( SDL_ANDROID_ShowScreenUnderFingerRect.y + SDL_ANDROID_ShowScreenUnderFingerRect.h >= screenY )
		SDL_ANDROID_ShowScreenUnderFingerRect.y = screenY - SDL_ANDROID_ShowScreenUnderFingerRect.h - 1;
	if( InsideRect(&SDL_ANDROID_ShowScreenUnderFingerRect, x, y) )
		SDL_ANDROID_ShowScreenUnderFingerRect.x = x - SDL_ANDROID_ShowScreenUnderFingerRect.w*11/10 - 1;
#endif
}


JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeMouse) ( JNIEnv*  env, jobject  thiz, jint x, jint y, jint action, jint pointerId, jint force, jint radius )
{
	int i;
#if SDL_VERSION_ATLEAST(1,3,0)

	SDL_Window * window = SDL_GetFocusWindow();
	if( !window )
		return;

#define SDL_ANDROID_sFakeWindowWidth window->w
#define SDL_ANDROID_sFakeWindowHeight window->h

#else
	if( !SDL_CurrentVideoSurface )
		return;
#endif
	if(pointerId < 0)
		pointerId = 0;
	if(pointerId > MAX_MULTITOUCH_POINTERS)
		pointerId = MAX_MULTITOUCH_POINTERS;
	
	// The ouch is passed either to on-screen keyboard or as mouse event for all duration of touch between down and up,
	// even if the finger is not anymore above screen kb button it will not acr as mouse event, and if it's initially
	// touches the screen outside of screen kb it won't trigger button keypress -
	// I think it's more logical this way
	if( SDL_ANDROID_isTouchscreenKeyboardUsed && ( action == MOUSE_DOWN || touchPointers[pointerId] == TOUCH_PTR_SCREENKB ) )
	{
		if( SDL_ANDROID_processTouchscreenKeyboard(x, y, action, pointerId) && action == MOUSE_DOWN )
			touchPointers[pointerId] = TOUCH_PTR_SCREENKB;
		if( touchPointers[pointerId] == TOUCH_PTR_SCREENKB )
		{
			if( action == MOUSE_UP )
				touchPointers[pointerId] = TOUCH_PTR_UP;
			return;
		}
	}
	
	if( action == MOUSE_DOWN )
	{
		touchPointers[pointerId] = TOUCH_PTR_MOUSE;
		firstMousePointerId = -1;
		for( i = 0; i < MAX_MULTITOUCH_POINTERS; i++ )
		{
			if( touchPointers[i] == TOUCH_PTR_MOUSE )
			{
				firstMousePointerId = i;
				break;
			}
		}
	}

#if SDL_VIDEO_RENDER_RESIZE
	// Translate mouse coordinates

	x = x * SDL_ANDROID_sFakeWindowWidth / SDL_ANDROID_sWindowWidth;
	y = y * SDL_ANDROID_sFakeWindowHeight / SDL_ANDROID_sWindowHeight;

#endif

	if( isMultitouchUsed )
	{

#if SDL_VERSION_ATLEAST(1,3,0)
		// Use nifty SDL 1.3 multitouch API
		if( action == MOUSE_MOVE )
			SDL_ANDROID_MainThreadPushMultitouchMotion(pointerId, x, y, force*radius / 16);
		else
			SDL_ANDROID_MainThreadPushMultitouchButton(pointerId, action == MOUSE_DOWN ? 1 : 0, x, y, force*radius / 16);
#endif

		SDL_ANDROID_MainThreadPushJoystickAxis(pointerId+1, 0, x);
		SDL_ANDROID_MainThreadPushJoystickAxis(pointerId+1, 1, y);
		SDL_ANDROID_MainThreadPushJoystickAxis(pointerId+1, 2, force);
		SDL_ANDROID_MainThreadPushJoystickAxis(pointerId+1, 3, radius);
		if( action == MOUSE_DOWN )
			SDL_ANDROID_MainThreadPushJoystickButton(pointerId+1, 0, SDL_PRESSED);
		if( action == MOUSE_UP )
			SDL_ANDROID_MainThreadPushJoystickButton(pointerId+1, 0, SDL_RELEASED);
	}
	if( !isMouseUsed && !SDL_ANDROID_isTouchscreenKeyboardUsed )
	{
		SDL_keysym keysym;
		if( action != MOUSE_MOVE )
			SDL_ANDROID_MainThreadPushKeyboardKey( action == MOUSE_DOWN ? SDL_PRESSED : SDL_RELEASED, SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_0)) );
		return;
	}

	if( !isMouseUsed )
		return;

	if( pointerId == firstMousePointerId )
	{
		int oldX, oldY;
		SDL_GetMouseState( &oldX, &oldY );
		if( action == MOUSE_UP )
		{
			if( SDL_GetMouseState( NULL, NULL ) & SDL_BUTTON(SDL_BUTTON_LEFT) )
				SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_LEFT );
			if( SDL_GetMouseState( NULL, NULL ) & SDL_BUTTON(SDL_BUTTON_RIGHT) )
				SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_RIGHT );
			SDL_ANDROID_ShowScreenUnderFingerRect.w = SDL_ANDROID_ShowScreenUnderFingerRect.h = 0;
			SDL_ANDROID_ShowScreenUnderFingerRectSrc.w = SDL_ANDROID_ShowScreenUnderFingerRectSrc.h = 0;
			if( SDL_ANDROID_ShowScreenUnderFinger )
			{
				// Move mouse by 1 pixel so it will force screen update and mouse-under-finger window will be removed
				if( moveMouseWithKbX >= 0 )
					SDL_ANDROID_MainThreadPushMouseMotion(moveMouseWithKbX > 0 ? moveMouseWithKbX-1 : 0, moveMouseWithKbY);
				else
					SDL_ANDROID_MainThreadPushMouseMotion(x > 0 ? x-1 : 0, y);
			}
			moveMouseWithKbX = -1;
			moveMouseWithKbY = -1;
			moveMouseWithKbSpeedX = 0;
			moveMouseWithKbSpeedY = 0;
		}
		if( action == MOUSE_DOWN )
		{
			if( (moveMouseWithKbX >= 0 || leftClickMethod == LEFT_CLICK_NEAR_CURSOR) &&
				abs(oldX - x) < SDL_ANDROID_sFakeWindowWidth / 4 && abs(oldY - y) < SDL_ANDROID_sFakeWindowHeight / 4 )
			{
				SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_LEFT );
				moveMouseWithKbX = oldX;
				moveMouseWithKbY = oldY;
				action == MOUSE_MOVE;
			}
			else
			if( leftClickMethod == LEFT_CLICK_NORMAL )
			{
				SDL_ANDROID_MainThreadPushMouseMotion(x, y);
				SDL_ANDROID_MainThreadPushMouseButton( SDL_PRESSED, SDL_BUTTON_LEFT );
			}
			else
			{
				SDL_ANDROID_MainThreadPushMouseMotion(x, y);
				action == MOUSE_MOVE;
			}
			UpdateScreenUnderFingerRect(x, y);
		}
		if( action == MOUSE_MOVE )
		{
			if( moveMouseWithKbX >= 0 )
			{
				if( abs(moveMouseWithKbX - x) > SDL_ANDROID_sFakeWindowWidth / 10 )
					moveMouseWithKbSpeedX += moveMouseWithKbX > x ? -1 : 1;
				else
					moveMouseWithKbSpeedX = moveMouseWithKbSpeedX * 2 / 3;
				if( abs(moveMouseWithKbY - y) > SDL_ANDROID_sFakeWindowHeight / 10 )
					moveMouseWithKbSpeedY += moveMouseWithKbY > y ? -1 : 1;
				else
					moveMouseWithKbSpeedY = moveMouseWithKbSpeedY * 2 / 3;

				moveMouseWithKbX += moveMouseWithKbSpeedX;
				moveMouseWithKbY += moveMouseWithKbSpeedY;

				if( abs(moveMouseWithKbX - x) > SDL_ANDROID_sFakeWindowWidth / 5 ||
					abs(moveMouseWithKbY - y) > SDL_ANDROID_sFakeWindowHeight / 5 )
				{
					moveMouseWithKbX = -1;
					moveMouseWithKbY = -1;
					moveMouseWithKbSpeedX = 0;
					moveMouseWithKbSpeedY = 0;
					SDL_ANDROID_MainThreadPushMouseMotion(x, y);
				}
				else
					SDL_ANDROID_MainThreadPushMouseMotion(moveMouseWithKbX, moveMouseWithKbY);
			}
			else
				SDL_ANDROID_MainThreadPushMouseMotion(x, y);

			if( rightClickMethod == RIGHT_CLICK_WITH_PRESSURE || leftClickMethod == LEFT_CLICK_WITH_PRESSURE )
			{
				int button = (leftClickMethod == LEFT_CLICK_WITH_PRESSURE) ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT;
				int buttonState = ( force > maxForce || radius > maxRadius );
				if( button == SDL_BUTTON_RIGHT && (SDL_GetMouseState( NULL, NULL ) & SDL_BUTTON(SDL_BUTTON_LEFT)) )
					SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_LEFT );
				if( ( (SDL_GetMouseState( NULL, NULL ) & SDL_BUTTON(button)) != 0 ) != buttonState )
					SDL_ANDROID_MainThreadPushMouseButton( buttonState ? SDL_PRESSED : SDL_RELEASED, button );
			}
			UpdateScreenUnderFingerRect(x, y);
		}
	}
	if( pointerId != firstMousePointerId && (action == MOUSE_DOWN || action == MOUSE_UP) )
	{
		if( leftClickMethod == LEFT_CLICK_WITH_MULTITOUCH )
		{
			SDL_ANDROID_MainThreadPushMouseButton( (action == MOUSE_DOWN) ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_LEFT );
		}
		else if( rightClickMethod == RIGHT_CLICK_WITH_MULTITOUCH )
		{
			if( SDL_GetMouseState( NULL, NULL ) & SDL_BUTTON(SDL_BUTTON_LEFT) )
				SDL_ANDROID_MainThreadPushMouseButton( SDL_RELEASED, SDL_BUTTON_LEFT );
			SDL_ANDROID_MainThreadPushMouseButton( (action == MOUSE_DOWN) ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_RIGHT );
		}
	}

	if( action == MOUSE_UP )
	{
		touchPointers[pointerId] = TOUCH_PTR_UP;
		firstMousePointerId = -1;
		for( i = 0; i < MAX_MULTITOUCH_POINTERS; i++ )
		{
			if( touchPointers[i] == TOUCH_PTR_MOUSE )
			{
				firstMousePointerId = i;
				break;
			}
		}
	}
}

static int processAndroidTrackball(int key, int action);

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeKey) ( JNIEnv*  env, jobject thiz, jint key, jint action )
{
#if SDL_VERSION_ATLEAST(1,3,0)
#else
	if( !SDL_CurrentVideoSurface )
		return;
#endif
	if( isTrackballUsed )
		if( processAndroidTrackball(key, action) )
			return;
	if( key == KEYCODE_MENU && rightClickMethod == RIGHT_CLICK_WITH_MENU_BUTTON )
	{
		SDL_ANDROID_MainThreadPushMouseButton( action ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_RIGHT );
		return;
	}
	if( key == KEYCODE_DPAD_CENTER && ( clickMouseWithDpadCenter || leftClickMethod == LEFT_CLICK_WITH_DPAD ) )
	{
		SDL_ANDROID_MainThreadPushMouseButton( action ? SDL_PRESSED : SDL_RELEASED, SDL_BUTTON_LEFT );
		return;
	}

	SDL_ANDROID_MainThreadPushKeyboardKey( action ? SDL_PRESSED : SDL_RELEASED, TranslateKey(key) );
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(DemoGLSurfaceView_nativeTextInput) ( JNIEnv*  env, jobject thiz, jint ascii, jint unicode )
{
	SDL_ANDROID_MainThreadPushText(ascii, unicode);
}


static void updateOrientation ( float accX, float accY, float accZ );

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(AccelerometerReader_nativeAccelerometer) ( JNIEnv*  env, jobject  thiz, jfloat accPosX, jfloat accPosY, jfloat accPosZ )
{
#if SDL_VERSION_ATLEAST(1,3,0)
#else
	if( !SDL_CurrentVideoSurface )
		return;
#endif
	// Calculate two angles from three coordinates - TODO: this is faulty!
	//float accX = atan2f(-accPosX, sqrtf(accPosY*accPosY+accPosZ*accPosZ) * ( accPosY > 0 ? 1.0f : -1.0f ) ) * M_1_PI * 180.0f;
	//float accY = atan2f(accPosZ, accPosY) * M_1_PI;
	
	float normal = sqrt(accPosX*accPosX+accPosY*accPosY+accPosZ*accPosZ);
	if(normal <= 0.0000001f)
		normal = 0.00001f;
	
	
	updateOrientation (accPosX/normal, accPosY/normal, 0.0f);
}


JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(AccelerometerReader_nativeOrientation) ( JNIEnv*  env, jobject  thiz, jfloat accX, jfloat accY, jfloat accZ )
{
#if SDL_VERSION_ATLEAST(1,3,0)
#else
	if( !SDL_CurrentVideoSurface )
		return;
#endif
	updateOrientation (accX, accY, accZ); // TODO: make values in range 0.0:1.0
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetTrackballUsed) ( JNIEnv*  env, jobject thiz)
{
	isTrackballUsed = 1;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetMouseUsed) ( JNIEnv*  env, jobject thiz,
		jint RightClickMethod, jint ShowScreenUnderFinger, jint LeftClickMethod,
		jint MoveMouseWithJoystick, jint ClickMouseWithDpad,
		jint MaxForce, jint MaxRadius,
		jint MoveMouseWithJoystickSpeed, jint MoveMouseWithJoystickAccel)
{
	isMouseUsed = 1;
	rightClickMethod = RightClickMethod;
	SDL_ANDROID_ShowScreenUnderFinger = ShowScreenUnderFinger;
	moveMouseWithArrowKeys = MoveMouseWithJoystick;
	clickMouseWithDpadCenter = ClickMouseWithDpad;
	leftClickMethod = LeftClickMethod;
	maxForce = MaxForce;
	maxRadius = MaxRadius;
	moveMouseWithKbSpeed = MoveMouseWithJoystickSpeed + 1;
	moveMouseWithKbAccel = MoveMouseWithJoystickAccel;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetJoystickUsed) ( JNIEnv*  env, jobject thiz)
{
	SDL_ANDROID_isJoystickUsed = 1;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetMultitouchUsed) ( JNIEnv*  env, jobject thiz)
{
	isMultitouchUsed = 1;
}


static float dx = 0.04, dy = 0.1, dz = 0.1, joystickSensitivity = 400.0f; // For accelerometer
enum { ACCELEROMETER_CENTER_FLOATING, ACCELEROMETER_CENTER_FIXED_START, ACCELEROMETER_CENTER_FIXED_HORIZ };
static int accelerometerCenterPos = ACCELEROMETER_CENTER_FLOATING;

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetAccelerometerSettings) ( JNIEnv*  env, jobject thiz, jint sensitivity, jint centerPos)
{
	dx = 0.04; dy = 0.08; dz = 0.08; joystickSensitivity = 32767.0f * 3.0f; // Fast sensitivity
	if( sensitivity == 1 )
	{
		dx = 0.1; dy = 0.15; dz = 0.15; joystickSensitivity = 32767.0f * 2.0f; // Medium sensitivity
	}
	if( sensitivity == 2 )
	{
		dx = 0.2; dy = 0.25; dz = 0.25; joystickSensitivity = 32767.0f; // Slow sensitivity
	}
	accelerometerCenterPos = centerPos;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetTrackballDampening) ( JNIEnv*  env, jobject thiz, jint value)
{
	TrackballDampening = (value * 200);
}

void updateOrientation ( float accX, float accY, float accZ )
{
	SDL_keysym keysym;
	// TODO: ask user for accelerometer precision from Java

	static float midX = 0, midY = 0, midZ = 0;
	static int pressLeft = 0, pressRight = 0, pressUp = 0, pressDown = 0, pressR = 0, pressL = 0;
	
	if( accelerometerCenterPos == ACCELEROMETER_CENTER_FIXED_START )
	{
		accelerometerCenterPos = ACCELEROMETER_CENTER_FIXED_HORIZ;
		midX = accX;
		midY = accY;
		midZ = accZ;
	}
	
	// midX = 0.0f; // Do not remember old value for phone tilt, it feels weird

	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "updateOrientation(): %f %f %f", accX, accY, accZ);
	
	if( SDL_ANDROID_isJoystickUsed ) // TODO: mutex for that stuff?
	{
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "updateOrientation(): sending joystick event");
		SDL_ANDROID_MainThreadPushJoystickAxis(0, 0, (Sint16)(fminf(32767.0f, fmax(-32767.0f, -(accX - midX) * joystickSensitivity))));
		SDL_ANDROID_MainThreadPushJoystickAxis(0, 1, (Sint16)(fminf(32767.0f, fmax(-32767.0f, (accY - midY) * joystickSensitivity))));
		SDL_ANDROID_MainThreadPushJoystickAxis(0, 2, (Sint16)(fminf(32767.0f, fmax(-32767.0f, (accZ - midZ) * joystickSensitivity))));

		if( accelerometerCenterPos == ACCELEROMETER_CENTER_FLOATING )
		{
			if( accY < midY - dy*2 )
				midY = accY + dy*2;
			if( accY > midY + dy*2 )
				midY = accY - dy*2;
			if( accZ < midZ - dz*2 )
				midZ = accZ + dz*2;
			if( accZ > midZ + dz*2 )
				midZ = accZ - dz*2;
		}
	}

	if(SDL_ANDROID_isJoystickUsed)
		return;

	if( accX < midX - dx )
	{
		if( !pressLeft )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: press left, acc %f mid %f d %f", accX, midX, dx);
			pressLeft = 1;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_DPAD_LEFT) );
		}
	}
	else
	{
		if( pressLeft )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: release left, acc %f mid %f d %f", accX, midX, dx);
			pressLeft = 0;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_LEFT) );
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
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_DPAD_RIGHT) );
		}
	}
	else
	{
		if( pressRight )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: release right, acc %f mid %f d %f", accX, midX, dx);
			pressRight = 0;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_RIGHT) );
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
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_DPAD_DOWN) );
		}
	}
	else
	{
		if( pressUp )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: release up, acc %f mid %f d %f", accY, midY, dy);
			pressUp = 0;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_DOWN) );
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
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_DPAD_UP) );
		}
	}
	else
	{
		if( pressDown )
		{
			//__android_log_print(ANDROID_LOG_INFO, "libSDL", "Accelerometer: release down, acc %f mid %f d %f", accY, midY, dy);
			pressDown = 0;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_UP) );
		}
	}
	if( accY > midY + dy*2 )
		midY = accY - dy*2;

	if( accZ < midZ - dz )
	{
		if( !pressL )
		{
			pressL = 1;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_ALT_LEFT) );
		}
	}
	else
	{
		if( pressL )
		{
			pressL = 0;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_ALT_LEFT) );
		}
	}
	if( accZ < midZ - dz*2 )
		midZ = accZ + dz*2;

	if( accZ > midZ + dz )
	{
		if( !pressR )
		{
			pressR = 1;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(KEYCODE_ALT_RIGHT) );
		}
	}
	else
	{
		if( pressR )
		{
			pressR = 0;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_ALT_RIGHT) );
		}
	}
	if( accZ > midZ + dz*2 )
		midZ = accZ - dz*2;

}

static int leftPressed = 0, rightPressed = 0, upPressed = 0, downPressed = 0;

int processAndroidTrackball(int key, int action)
{
	SDL_keysym keysym;
	
	if( ! action && (
		key == KEYCODE_DPAD_UP ||
		key == KEYCODE_DPAD_DOWN ||
		key == KEYCODE_DPAD_LEFT ||
		key == KEYCODE_DPAD_RIGHT ) )
		return 1;
	lastTrackballAction = SDL_GetTicks();

	if( key == KEYCODE_DPAD_UP )
	{
		if( downPressed )
		{
			downPressed = 0;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_DOWN) );
			return 1;
		}
		if( !upPressed )
		{
			upPressed = 1;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key) );
		}
		else
		{
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(key) );
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key) );
		}
		return 1;
	}

	if( key == KEYCODE_DPAD_DOWN )
	{
		if( upPressed )
		{
			upPressed = 0;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_UP) );
			return 1;
		}
		if( !upPressed )
		{
			downPressed = 1;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key) );
		}
		else
		{
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(key) );
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key) );
		}
		return 1;
	}

	if( key == KEYCODE_DPAD_LEFT )
	{
		if( rightPressed )
		{
			rightPressed = 0;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_RIGHT) );
			return 1;
		}
		if( !leftPressed )
		{
			leftPressed = 1;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key) );
		}
		else
		{
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(key) );
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key) );
		}
		return 1;
	}

	if( key == KEYCODE_DPAD_RIGHT )
	{
		if( leftPressed )
		{
			leftPressed = 0;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_LEFT) );
			return 1;
		}
		if( !rightPressed )
		{
			rightPressed = 1;
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key) );
		}
		else
		{
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(key) );
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, TranslateKey(key) );
		}
		return 1;
	}

	return 0;
}

void SDL_ANDROID_processAndroidTrackballDampening()
{
	if( !TrackballDampening )
		return;
	if( SDL_GetTicks() > TrackballDampening + lastTrackballAction  )
	{
		if( upPressed )
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_UP) );
		if( downPressed )
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_DOWN) );
		if( leftPressed )
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_LEFT) );
		if( rightPressed )
			SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, TranslateKey(KEYCODE_DPAD_RIGHT) );
		upPressed = 0;
		downPressed = 0;
		leftPressed = 0;
		rightPressed = 0;
	}
}

int SDL_SYS_JoystickInit(void)
{
	SDL_numjoysticks = 0;
	if( SDL_ANDROID_isJoystickUsed )
		SDL_numjoysticks = 1;
	if( isMultitouchUsed )
		SDL_numjoysticks = MAX_MULTITOUCH_POINTERS+1;

	return(SDL_numjoysticks);
}

/* Function to get the device-dependent name of a joystick */
const char *SDL_SYS_JoystickName(int index)
{
	if(index)
		return("Android multitouch");
	return("Android accelerometer/orientation sensor");
}

/* Function to open a joystick for use.
   The joystick to open is specified by the index field of the joystick.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
int SDL_SYS_JoystickOpen(SDL_Joystick *joystick)
{
	joystick->nbuttons = 0; // Ignored
	joystick->nhats = 0;
	joystick->nballs = 0;
	if( joystick->index == 0 )
		joystick->naxes = 3;
	else
	{
		joystick->naxes = 4;
		joystick->nbuttons = 1;
	}
	SDL_ANDROID_CurrentJoysticks[joystick->index] = joystick;
	return(0);
}

/* Function to update the state of a joystick - called as a device poll.
 * This function shouldn't update the joystick structure directly,
 * but instead should call SDL_PrivateJoystick*() to deliver events
 * and update joystick device state.
 */
void SDL_SYS_JoystickUpdate(SDL_Joystick *joystick)
{
	return;
}

/* Function to close a joystick after use */
void SDL_SYS_JoystickClose(SDL_Joystick *joystick)
{
	SDL_ANDROID_CurrentJoysticks[joystick->index] = NULL;
	return;
}

/* Function to perform any system-specific joystick related cleanup */
void SDL_SYS_JoystickQuit(void)
{
	int i;
	for(i=0; i<MAX_MULTITOUCH_POINTERS+1; i++)
		SDL_ANDROID_CurrentJoysticks[i] = NULL;
	return;
}


enum { MAX_BUFFERED_EVENTS = 64 };
static SDL_Event BufferedEvents[MAX_BUFFERED_EVENTS];
static int BufferedEventsStart = 0, BufferedEventsEnd = 0;
static SDL_mutex * BufferedEventsMutex = NULL;

#if SDL_VERSION_ATLEAST(1,3,0)

#define SDL_SendKeyboardKey(state, keysym) SDL_SendKeyboardKey(state, (keysym)->sym)

#else

#define SDL_SendMouseMotion(A,B,X,Y) SDL_PrivateMouseMotion(0, 0, X, Y)
#define SDL_SendMouseButton(N, A, B) SDL_PrivateMouseButton( A, B, 0, 0 )
#define SDL_SendKeyboardKey(state, keysym) SDL_PrivateKeyboard(state, keysym)

#endif

extern void SDL_ANDROID_PumpEvents()
{
	SDL_ANDROID_processAndroidTrackballDampening();
	SDL_ANDROID_processMoveMouseWithKeyboard();

	if( !BufferedEventsMutex )
		BufferedEventsMutex = SDL_CreateMutex();
	SDL_mutexP(BufferedEventsMutex);
	while( BufferedEventsStart != BufferedEventsEnd )
	{
		SDL_Event * ev = &BufferedEvents[BufferedEventsStart];
		
		switch( ev->type )
		{
			case SDL_MOUSEMOTION:
				SDL_SendMouseMotion(NULL, 0, ev->motion.x, ev->motion.y);
				break;
			case SDL_MOUSEBUTTONDOWN:
				SDL_SendMouseButton( NULL, ev->button.state, ev->button.button );
				break;
			case SDL_KEYDOWN:
				SDL_SendKeyboardKey( ev->key.state, &ev->key.keysym );
				break;
			case SDL_JOYAXISMOTION:
				if( ev->jaxis.which < MAX_MULTITOUCH_POINTERS+1 && SDL_ANDROID_CurrentJoysticks[ev->jaxis.which] )
					SDL_PrivateJoystickAxis( SDL_ANDROID_CurrentJoysticks[ev->jaxis.which], ev->jaxis.axis, ev->jaxis.value );
				break;
			case SDL_JOYBUTTONDOWN:
				if( ev->jbutton.which < MAX_MULTITOUCH_POINTERS+1 && SDL_ANDROID_CurrentJoysticks[ev->jbutton.which] )
					SDL_PrivateJoystickButton( SDL_ANDROID_CurrentJoysticks[ev->jbutton.which], ev->jbutton.button, ev->jbutton.state );
				break;
#if SDL_VERSION_ATLEAST(1,3,0)
			case SDL_FINGERMOTION:
				SDL_SendTouchMotion(0, ev->tfinger.fingerId, 0, ev->tfinger.x, ev->tfinger.y, ev->tfinger.pressure);
				break;
			case SDL_FINGERDOWN:
				SDL_SendFingerDown(0, ev->tfinger.fingerId, ev->tfinger.state ? 1 : 0, ev->tfinger.x, ev->tfinger.y, ev->tfinger.pressure);
				break;
			case SDL_TEXTINPUT:
				SDL_SendKeyboardText(ev->text.text);
				break;
#endif
		}
		
		ev->type = 0;
		BufferedEventsStart++;
		if( BufferedEventsStart >= MAX_BUFFERED_EVENTS )
			BufferedEventsStart = 0;
	}
	SDL_mutexV(BufferedEventsMutex);
};
// Queue events to main thread
static int getNextEvent()
{
	int nextEvent;
	if( !BufferedEventsMutex )
		return -1;
	SDL_mutexP(BufferedEventsMutex);
	nextEvent = BufferedEventsEnd;
	nextEvent++;
	if( nextEvent >= MAX_BUFFERED_EVENTS )
		nextEvent = 0;
	while( nextEvent == BufferedEventsStart )
	{
		SDL_mutexV(BufferedEventsMutex);
		if( SDL_ANDROID_InsideVideoThread() )
			SDL_ANDROID_PumpEvents();
		else
			SDL_Delay(100);
		SDL_mutexP(BufferedEventsMutex);
		nextEvent = BufferedEventsEnd;
		nextEvent++;
		if( nextEvent >= MAX_BUFFERED_EVENTS )
			nextEvent = 0;
	}
	return nextEvent;
}

extern void SDL_ANDROID_MainThreadPushMouseMotion(int x, int y)
{
	int nextEvent = getNextEvent();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	
	ev->type = SDL_MOUSEMOTION;
	ev->motion.x = x;
	ev->motion.y = y;
	
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
};
extern void SDL_ANDROID_MainThreadPushMouseButton(int pressed, int button)
{
	int nextEvent = getNextEvent();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	
	ev->type = SDL_MOUSEBUTTONDOWN;
	ev->button.state = pressed;
	ev->button.button = button;
	
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
};

extern void SDL_ANDROID_MainThreadPushKeyboardKey(int pressed, SDL_scancode key)
{
	int nextEvent = getNextEvent();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	

	if( moveMouseWithArrowKeys && (
		key == SDL_KEY(UP) || key == SDL_KEY(DOWN) ||
		key == SDL_KEY(LEFT) || key == SDL_KEY(RIGHT) ) )
	{
		if( moveMouseWithKbX < 0 )
			SDL_GetMouseState( &moveMouseWithKbX, &moveMouseWithKbY );

		if( pressed )
		{
			if( key == SDL_KEY(LEFT) )
			{
				if( moveMouseWithKbSpeedX > 0 )
					moveMouseWithKbSpeedX = 0;
				moveMouseWithKbSpeedX -= moveMouseWithKbSpeed;
				moveMouseWithKbAccelX = -moveMouseWithKbAccel;
				moveMouseWithKbAccelUpdateNeeded |= 1;
			}
			else if( key == SDL_KEY(RIGHT) )
			{
				if( moveMouseWithKbSpeedX < 0 )
					moveMouseWithKbSpeedX = 0;
				moveMouseWithKbSpeedX += moveMouseWithKbSpeed;
				moveMouseWithKbAccelX = moveMouseWithKbAccel;
				moveMouseWithKbAccelUpdateNeeded |= 1;
			}

			if( key == SDL_KEY(UP) )
			{
				if( moveMouseWithKbSpeedY > 0 )
					moveMouseWithKbSpeedY = 0;
				moveMouseWithKbSpeedY -= moveMouseWithKbSpeed;
				moveMouseWithKbAccelY = -moveMouseWithKbAccel;
				moveMouseWithKbAccelUpdateNeeded |= 2;
			}
			else if( key == SDL_KEY(DOWN) )
			{
				if( moveMouseWithKbSpeedY < 0 )
					moveMouseWithKbSpeedY = 0;
				moveMouseWithKbSpeedY += moveMouseWithKbSpeed;
				moveMouseWithKbAccelY = moveMouseWithKbAccel;
				moveMouseWithKbAccelUpdateNeeded |= 2;
			}
		}
		else
		{
			if( key == SDL_KEY(LEFT) || key == SDL_KEY(RIGHT) )
			{
				moveMouseWithKbSpeedX = 0;
				moveMouseWithKbAccelX = 0;
				moveMouseWithKbAccelUpdateNeeded &= ~1;
			}
			if( key == SDL_KEY(UP) || key == SDL_KEY(DOWN) )
			{
				moveMouseWithKbSpeedY = 0;
				moveMouseWithKbAccelY = 0;
				moveMouseWithKbAccelUpdateNeeded &= ~2;
			}
		}

		moveMouseWithKbX += moveMouseWithKbSpeedX;
		moveMouseWithKbY += moveMouseWithKbSpeedY;

		SDL_mutexV(BufferedEventsMutex);

		SDL_ANDROID_MainThreadPushMouseMotion(moveMouseWithKbX, moveMouseWithKbY);
		return;
	}

	ev->type = SDL_KEYDOWN;
	ev->key.state = pressed;
	ev->key.keysym.scancode = key;
	ev->key.keysym.sym = key;
	ev->key.keysym.mod = KMOD_NONE;
	ev->key.keysym.unicode = 0;
#if SDL_VERSION_ATLEAST(1,3,0)
#else
	if ( SDL_TranslateUNICODE )
		ev->key.keysym.unicode = key;
#endif
	
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
};

extern void SDL_ANDROID_MainThreadPushJoystickAxis(int joy, int axis, int value)
{
	if( ! ( joy < MAX_MULTITOUCH_POINTERS+1 && SDL_ANDROID_CurrentJoysticks[joy] ) )
		return;

	int nextEvent = getNextEvent();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	
	ev->type = SDL_JOYAXISMOTION;
	ev->jaxis.which = joy;
	ev->jaxis.axis = axis;
	ev->jaxis.value = value;
	
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
};
extern void SDL_ANDROID_MainThreadPushJoystickButton(int joy, int button, int pressed)
{
	if( ! ( joy < MAX_MULTITOUCH_POINTERS+1 && SDL_ANDROID_CurrentJoysticks[joy] ) )
		return;

	int nextEvent = getNextEvent();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	
	ev->type = SDL_JOYBUTTONDOWN;
	ev->jbutton.which = joy;
	ev->jbutton.button = button;
	ev->jbutton.state = pressed;
	
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
};
extern void SDL_ANDROID_MainThreadPushMultitouchButton(int id, int pressed, int x, int y, int force)
{
#if SDL_VERSION_ATLEAST(1,3,0)
	int nextEvent = getNextEvent();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	
	ev->type = SDL_FINGERDOWN;
	ev->tfinger.fingerId = id;
	ev->tfinger.state = pressed;
	ev->tfinger.x = x;
	ev->tfinger.y = y;
	ev->tfinger.pressure = force;
	
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
#endif
};
extern void SDL_ANDROID_MainThreadPushMultitouchMotion(int id, int x, int y, int force)
{
#if SDL_VERSION_ATLEAST(1,3,0)
	int nextEvent = getNextEvent();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	
	ev->type = SDL_FINGERMOTION;
	ev->tfinger.fingerId = id;
	ev->tfinger.x = x;
	ev->tfinger.y = y;
	ev->tfinger.pressure = force;
	
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
#endif
};
extern void SDL_ANDROID_MainThreadPushText( int scancode, int unicode )
{
	int nextEvent = getNextEvent();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	
#if SDL_VERSION_ATLEAST(1,3,0)

	// TODO: convert to UTF-8
	ev->type = SDL_TEXTINPUT;
	ev->text.text[0] = scancode;
	ev->text.text[1] = 0;

#else

	ev->type = SDL_KEYDOWN;
	ev->key.state = SDL_PRESSED;
	ev->key.keysym.scancode = scancode;
	ev->key.keysym.sym = scancode;
	ev->key.keysym.mod = KMOD_NONE;
	ev->key.keysym.unicode = 0;
	if ( SDL_TranslateUNICODE )
		ev->key.keysym.unicode = unicode;
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);

	nextEvent = getNextEvent();
	{
		SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
		ev->type = SDL_KEYUP;
		ev->key.state = SDL_PRESSED;
		ev->key.keysym.scancode = scancode;
		ev->key.keysym.sym = scancode;
		ev->key.keysym.mod = KMOD_NONE;
		ev->key.keysym.unicode = 0;
		if ( SDL_TranslateUNICODE )
			ev->key.keysym.unicode = unicode;
	}

#endif
	
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
};


Uint32 lastMoveMouseWithKeyboardUpdate = 0;

void SDL_ANDROID_processMoveMouseWithKeyboard()
{
	if( ! moveMouseWithKbAccelUpdateNeeded )
		return;

	Uint32 ticks = SDL_GetTicks();

	if( ticks - lastMoveMouseWithKeyboardUpdate < 20 ) // Update at 50 FPS max, or it will not work properlty on very fast devices
		return;

	lastMoveMouseWithKeyboardUpdate = ticks;

	moveMouseWithKbSpeedX += moveMouseWithKbAccelX;
	moveMouseWithKbSpeedY += moveMouseWithKbAccelY;

	moveMouseWithKbX += moveMouseWithKbSpeedX;
	moveMouseWithKbY += moveMouseWithKbSpeedY;
	SDL_ANDROID_MainThreadPushMouseMotion(moveMouseWithKbX, moveMouseWithKbY);
};


void ANDROID_InitOSKeymap()
{
  int i;
  SDLKey * keymap = SDL_android_keymap;

#if (SDL_VERSION_ATLEAST(1,3,0))
  SDLKey defaultKeymap[SDL_NUM_SCANCODES];
  SDL_GetDefaultKeymap(defaultKeymap);
  SDL_SetKeymap(0, defaultKeymap, SDL_NUM_SCANCODES);

  SDL_Touch touch;
  memset( &touch, 0, sizeof(touch) );
  touch.x_min = touch.y_min = touch.pressure_min = 0.0f;
  touch.pressure_max = 1000000;
  touch.x_max = SDL_ANDROID_sWindowWidth;
  touch.y_max = SDL_ANDROID_sWindowHeight;

  // These constants are hardcoded inside SDL_touch.c, which makes no sense for me.
  touch.xres = touch.yres = 32768;
  touch.native_xres = touch.native_yres = 32768.0f;

  touch.pressureres = 1;
  touch.native_pressureres = 1.0f;
  touch.id = 0;

  SDL_AddTouch(&touch, "Android touch screen");

#endif

  // TODO: keys are mapped rather randomly

  for (i=0; i<SDL_arraysize(SDL_android_keymap); ++i)
    SDL_android_keymap[i] = SDL_KEY(UNKNOWN);

  keymap[KEYCODE_UNKNOWN] = SDL_KEY(UNKNOWN);

  keymap[KEYCODE_BACK] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_5));

  // TODO: make this configurable
  keymap[KEYCODE_MENU] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_4));

  keymap[KEYCODE_DPAD_CENTER] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_1));
  keymap[KEYCODE_SEARCH] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_9));
  keymap[KEYCODE_CALL] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_10));

  //keymap[KEYCODE_CALL] = SDL_KEY(RCTRL);
  //keymap[KEYCODE_DPAD_CENTER] = SDL_KEY(LALT);
  
  keymap[KEYCODE_VOLUME_UP] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_2));
  keymap[KEYCODE_VOLUME_DOWN] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_3));
  
  keymap[KEYCODE_HOME] = SDL_KEY(HOME); // Cannot be used in application

  keymap[KEYCODE_CAMERA] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_6));

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

  keymap[KEYCODE_SOFT_LEFT] = SDL_KEY(KP_4);
  keymap[KEYCODE_SOFT_RIGHT] = SDL_KEY(KP_6);
  keymap[KEYCODE_ENTER] = SDL_KEY(RETURN); //SDL_KEY(KP_ENTER);


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
  keymap[KEYCODE_DEL] = SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_8));
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

  keymap[KEYCODE_ALT_LEFT] = SDL_KEY(KP_7);
  keymap[KEYCODE_ALT_RIGHT] = SDL_KEY(KP_9);

  keymap[KEYCODE_SHIFT_LEFT] = SDL_KEY(F1);
  keymap[KEYCODE_SHIFT_RIGHT] = SDL_KEY(F2);

  keymap[KEYCODE_EXPLORER] = SDL_KEY(F3);
  keymap[KEYCODE_ENVELOPE] = SDL_KEY(F4);

  keymap[KEYCODE_HEADSETHOOK] = SDL_KEY(F5);
  keymap[KEYCODE_FOCUS] = SDL_KEY(F6);
  keymap[KEYCODE_NOTIFICATION] = SDL_KEY(F7);

  // Cannot be received by application, OS internal
  keymap[KEYCODE_ENDCALL] = SDL_KEY(LSHIFT);
  keymap[KEYCODE_POWER] = SDL_KEY(RALT);

}

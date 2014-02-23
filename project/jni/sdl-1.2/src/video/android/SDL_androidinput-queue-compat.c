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
#include "unicodestuff.h"
#include "atan2i.h"


#ifdef SDL_COMPATIBILITY_HACKS_SLOW_COMPATIBLE_EVENT_QUEUE

// This code is left here to rot - it's bad, complicated and needed by only one applicaiton

#if SDL_VERSION_ATLEAST(1,3,0)

#define SDL_SendKeyboardKey(state, keysym) SDL_SendKeyboardKey(state, (keysym)->sym)
extern SDL_Window * ANDROID_CurrentWindow;

#else

#define SDL_SendMouseMotion(A,B,X,Y) SDL_PrivateMouseMotion(0, 0, X, Y)
#define SDL_SendMouseButton(N, A, B) SDL_PrivateMouseButton( A, B, 0, 0 )
#define SDL_SendKeyboardKey(state, keysym) SDL_PrivateKeyboard(state, keysym)

#endif

enum { MAX_BUFFERED_EVENTS = 64 };
static SDL_Event BufferedEvents[MAX_BUFFERED_EVENTS];
static int BufferedEventsStart = 0, BufferedEventsEnd = 0;
static SDL_mutex * BufferedEventsMutex = NULL;

/* We need our own event queue, because Free Heroes 2 game uses
 * SDL_SetEventFilter(), and it calls SDL_Flip() from inside
 * it's custom filter function, and SDL_Flip() does not work
 * when it's not called from the main() thread.
 * So we, like, push the events into our own queue,
 * read each event from that queue inside SDL_ANDROID_PumpEvents(),
 * unlock the mutex, and push the event to SDL queue,
 * which is then immediately read by SDL from the same thread,
 * and then SDL invokes event filter function from FHeroes2.
 * FHeroes2 call SDL_Flip() from inside that event filter function,
 * and it works, because it is called from the main() thread.
 */
extern void SDL_ANDROID_PumpEvents()
{
	static int oldMouseButtons = 0;
	SDL_Event ev;
	SDL_ANDROID_processAndroidTrackballDampening();
	SDL_ANDROID_processMoveMouseWithKeyboard();
#if SDL_VERSION_ATLEAST(1,3,0)
	SDL_Window * window = SDL_GetFocusWindow();
	if( !window )
		return;
#endif

	if( !BufferedEventsMutex )
		BufferedEventsMutex = SDL_CreateMutex();

	SDL_mutexP(BufferedEventsMutex);
	while( BufferedEventsStart != BufferedEventsEnd )
	{
		ev = BufferedEvents[BufferedEventsStart];
		BufferedEvents[BufferedEventsStart].type = 0;
		BufferedEventsStart++;
		if( BufferedEventsStart >= MAX_BUFFERED_EVENTS )
			BufferedEventsStart = 0;
		SDL_mutexV(BufferedEventsMutex);
		
		switch( ev.type )
		{
			case SDL_MOUSEMOTION:
				SDL_SendMouseMotion( ANDROID_CurrentWindow, 0, ev.motion.x, ev.motion.y );
				break;
			case SDL_MOUSEBUTTONDOWN:
				if( ((oldMouseButtons & SDL_BUTTON(ev.button.button)) != 0) != ev.button.state )
				{
					oldMouseButtons = (oldMouseButtons & ~SDL_BUTTON(ev.button.button)) | (ev.button.state ? SDL_BUTTON(ev.button.button) : 0);
					SDL_SendMouseButton( ANDROID_CurrentWindow, ev.button.state, ev.button.button );
				}
				break;
			case SDL_KEYDOWN:
				//__android_log_print(ANDROID_LOG_INFO, "libSDL", "SDL_KEYDOWN: %i %i", ev->key.keysym.sym, ev->key.state);
				SDL_SendKeyboardKey( ev.key.state, &ev.key.keysym );
				break;
			case SDL_JOYAXISMOTION:
				if( ev.jaxis.which < MAX_MULTITOUCH_POINTERS+1 && SDL_ANDROID_CurrentJoysticks[ev.jaxis.which] )
					SDL_PrivateJoystickAxis( SDL_ANDROID_CurrentJoysticks[ev.jaxis.which], ev.jaxis.axis, ev.jaxis.value );
				break;
			case SDL_JOYBUTTONDOWN:
				if( ev.jbutton.which < MAX_MULTITOUCH_POINTERS+1 && SDL_ANDROID_CurrentJoysticks[ev.jbutton.which] )
					SDL_PrivateJoystickButton( SDL_ANDROID_CurrentJoysticks[ev.jbutton.which], ev.jbutton.button, ev.jbutton.state );
				break;
			case SDL_JOYBALLMOTION:
				if( ev.jball.which < MAX_MULTITOUCH_POINTERS+1 && SDL_ANDROID_CurrentJoysticks[ev.jbutton.which] )
					SDL_PrivateJoystickBall( SDL_ANDROID_CurrentJoysticks[ev.jball.which], ev.jball.ball, ev.jball.xrel, ev.jball.yrel );
				break;
#if SDL_VERSION_ATLEAST(1,3,0)
				//if( ANDROID_CurrentWindow )
				//	SDL_SendWindowEvent(ANDROID_CurrentWindow, SDL_WINDOWEVENT_MINIMIZED, 0, 0);
#else
			case SDL_ACTIVEEVENT:
				SDL_PrivateAppActive(ev.active.gain, ev.active.state);
				break;
#endif
#if SDL_VERSION_ATLEAST(1,3,0)
			case SDL_FINGERMOTION:
				SDL_SendTouchMotion(0, ev.tfinger.fingerId, 0, (float)ev.tfinger.x / (float)window->w, (float)ev.tfinger.y / (float)window->h, ev.tfinger.pressure);
				break;
			case SDL_FINGERDOWN:
				SDL_SendFingerDown(0, ev.tfinger.fingerId, ev.tfinger.state ? 1 : 0, (float)ev.tfinger.x / (float)window->w, (float)ev.tfinger.y / (float)window->h, ev.tfinger.pressure);
				break;
			case SDL_TEXTINPUT:
				SDL_SendKeyboardText(ev.text.text);
				break;
			case SDL_MOUSEWHEEL:
				SDL_SendMouseWheel( ANDROID_CurrentWindow, ev.wheel.x, ev.wheel.y );
				break;
#endif
		}

		SDL_mutexP(BufferedEventsMutex);
	}
	SDL_mutexV(BufferedEventsMutex);
};
// Queue events to main thread
static int getNextEventAndLock()
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

static int getPrevEventNoLock()
{
	int prevEvent;
	if(BufferedEventsStart == BufferedEventsEnd)
		return -1;
	prevEvent = BufferedEventsEnd;
	prevEvent--;
	if( prevEvent < 0 )
		prevEvent = MAX_BUFFERED_EVENTS - 1;
	return prevEvent;
}

extern void SDL_ANDROID_MainThreadPushMouseMotion(int x, int y)
{
	int nextEvent = getNextEventAndLock();
	if( nextEvent == -1 )
		return;
	
	int prevEvent = getPrevEventNoLock();
	if( prevEvent > 0 && BufferedEvents[prevEvent].type == SDL_MOUSEMOTION )
	{
		// Reuse previous mouse motion event, to prevent mouse movement lag
		BufferedEvents[prevEvent].motion.x = x;
		BufferedEvents[prevEvent].motion.y = y;
	}
	else
	{
		SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
		ev->type = SDL_MOUSEMOTION;
		ev->motion.x = x;
		ev->motion.y = y;
	}
	SDL_ANDROID_currentMouseX = x;
	SDL_ANDROID_currentMouseY = y;
	
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
};
extern void SDL_ANDROID_MainThreadPushMouseButton(int pressed, int button)
{
	int nextEvent = getNextEventAndLock();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	
	ev->type = SDL_MOUSEBUTTONDOWN;
	ev->button.state = pressed;
	ev->button.button = button;

	if(pressed)
		SDL_ANDROID_currentMouseButtons |= SDL_BUTTON(button);
	else
		SDL_ANDROID_currentMouseButtons &= ~(SDL_BUTTON(button));
	
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
};

extern void SDL_ANDROID_MainThreadPushKeyboardKey(int pressed, SDL_scancode key, int unicode)
{
	int nextEvent = getNextEventAndLock();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	
	if( SDL_ANDROID_moveMouseWithArrowKeys && (
		key == SDL_KEY(UP) || key == SDL_KEY(DOWN) ||
		key == SDL_KEY(LEFT) || key == SDL_KEY(RIGHT) ) )
	{
		if( SDL_ANDROID_moveMouseWithKbX < 0 )
		{
			SDL_ANDROID_moveMouseWithKbX = SDL_ANDROID_currentMouseX;
			SDL_ANDROID_moveMouseWithKbY = SDL_ANDROID_currentMouseY;
		}

		if( pressed )
		{
			if( key == SDL_KEY(LEFT) )
			{
				if( SDL_ANDROID_moveMouseWithKbSpeedX > 0 )
					SDL_ANDROID_moveMouseWithKbSpeedX = 0;
				SDL_ANDROID_moveMouseWithKbSpeedX -= SDL_ANDROID_moveMouseWithKbSpeed;
				SDL_ANDROID_moveMouseWithKbAccelX = -SDL_ANDROID_moveMouseWithKbAccel;
				SDL_ANDROID_moveMouseWithKbAccelUpdateNeeded |= 1;
			}
			else if( key == SDL_KEY(RIGHT) )
			{
				if( SDL_ANDROID_moveMouseWithKbSpeedX < 0 )
					SDL_ANDROID_moveMouseWithKbSpeedX = 0;
				SDL_ANDROID_moveMouseWithKbSpeedX += SDL_ANDROID_moveMouseWithKbSpeed;
				SDL_ANDROID_moveMouseWithKbAccelX = SDL_ANDROID_moveMouseWithKbAccel;
				SDL_ANDROID_moveMouseWithKbAccelUpdateNeeded |= 1;
			}

			if( key == SDL_KEY(UP) )
			{
				if( SDL_ANDROID_moveMouseWithKbSpeedY > 0 )
					SDL_ANDROID_moveMouseWithKbSpeedY = 0;
				SDL_ANDROID_moveMouseWithKbSpeedY -= SDL_ANDROID_moveMouseWithKbSpeed;
				SDL_ANDROID_moveMouseWithKbAccelY = -SDL_ANDROID_moveMouseWithKbAccel;
				SDL_ANDROID_moveMouseWithKbAccelUpdateNeeded |= 2;
			}
			else if( key == SDL_KEY(DOWN) )
			{
				if( SDL_ANDROID_moveMouseWithKbSpeedY < 0 )
					SDL_ANDROID_moveMouseWithKbSpeedY = 0;
				SDL_ANDROID_moveMouseWithKbSpeedY += SDL_ANDROID_moveMouseWithKbSpeed;
				SDL_ANDROID_moveMouseWithKbAccelY = SDL_ANDROID_moveMouseWithKbAccel;
				SDL_ANDROID_moveMouseWithKbAccelUpdateNeeded |= 2;
			}
		}
		else
		{
			if( key == SDL_KEY(LEFT) || key == SDL_KEY(RIGHT) )
			{
				SDL_ANDROID_moveMouseWithKbSpeedX = 0;
				SDL_ANDROID_moveMouseWithKbAccelX = 0;
				SDL_ANDROID_moveMouseWithKbAccelUpdateNeeded &= ~1;
			}
			if( key == SDL_KEY(UP) || key == SDL_KEY(DOWN) )
			{
				SDL_ANDROID_moveMouseWithKbSpeedY = 0;
				SDL_ANDROID_moveMouseWithKbAccelY = 0;
				SDL_ANDROID_moveMouseWithKbAccelUpdateNeeded &= ~2;
			}
		}

		SDL_ANDROID_moveMouseWithKbX += SDL_ANDROID_moveMouseWithKbSpeedX;
		SDL_ANDROID_moveMouseWithKbY += SDL_ANDROID_moveMouseWithKbSpeedY;

		SDL_mutexV(BufferedEventsMutex);

		SDL_ANDROID_MainThreadPushMouseMotion(SDL_ANDROID_moveMouseWithKbX, SDL_ANDROID_moveMouseWithKbY);
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
#endif
		ev->key.keysym.unicode = key;

	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
};

extern void SDL_ANDROID_MainThreadPushJoystickAxis(int joy, int axis, int value)
{
	if( ! ( joy < MAX_MULTITOUCH_POINTERS+1 && SDL_ANDROID_CurrentJoysticks[joy] ) )
		return;

	int nextEvent = getNextEventAndLock();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	
	ev->type = SDL_JOYAXISMOTION;
	ev->jaxis.which = joy;
	ev->jaxis.axis = axis;
	ev->jaxis.value = MAX( -32768, MIN( 32767, value ) );
	
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
};
extern void SDL_ANDROID_MainThreadPushJoystickButton(int joy, int button, int pressed)
{
	if( ! ( joy < MAX_MULTITOUCH_POINTERS+1 && SDL_ANDROID_CurrentJoysticks[joy] ) )
		return;

	int nextEvent = getNextEventAndLock();
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
extern void SDL_ANDROID_MainThreadPushJoystickBall(int joy, int ball, int x, int y)
{
	if( ! ( joy < MAX_MULTITOUCH_POINTERS+1 && SDL_ANDROID_CurrentJoysticks[joy] ) )
		return;

	int nextEvent = getNextEventAndLock();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	
	ev->type = SDL_JOYBALLMOTION;
	ev->jball.which = joy;
	ev->jball.ball = ball;
	ev->jball.xrel = x;
	ev->jball.yrel = y;
	
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
}
extern void SDL_ANDROID_MainThreadPushMultitouchButton(int id, int pressed, int x, int y, int force)
{
#if SDL_VERSION_ATLEAST(1,3,0)
	int nextEvent = getNextEventAndLock();
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
	int nextEvent = getNextEventAndLock();
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

extern void SDL_ANDROID_MainThreadPushMouseWheel(int x, int y)
{
#if SDL_VERSION_ATLEAST(1,3,0)
	int nextEvent = getNextEventAndLock();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	
	ev->type = SDL_MOUSEWHEEL;
	ev->wheel.x = x;
	ev->wheel.y = y;
	
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
#endif
}

extern void SDL_ANDROID_MainThreadPushAppActive(int active)
{
#if SDL_VERSION_ATLEAST(1,3,0)
				//if( ANDROID_CurrentWindow )
				//	SDL_SendWindowEvent(ANDROID_CurrentWindow, SDL_WINDOWEVENT_MINIMIZED, 0, 0);
#else
	int nextEvent = getNextEventAndLock();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	
	ev->type = SDL_ACTIVEEVENT;
	ev->active.gain = active;
	ev->active.state = SDL_APPACTIVE|SDL_APPINPUTFOCUS|SDL_APPMOUSEFOCUS;
	
	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
#endif
}

enum { DEFERRED_TEXT_COUNT = 256 };
static struct { int scancode; int unicode; int down; } deferredText[DEFERRED_TEXT_COUNT];
static int deferredTextIdx1 = 0;
static int deferredTextIdx2 = 0;
static SDL_mutex * deferredTextMutex = NULL;

void SDL_ANDROID_DeferredTextInput()
{
	if( !deferredTextMutex )
		deferredTextMutex = SDL_CreateMutex();

	SDL_mutexP(deferredTextMutex);
	
	if( deferredTextIdx1 != deferredTextIdx2 )
	{
		int nextEvent = getNextEventAndLock();
		if( nextEvent == -1 )
		{
			SDL_mutexV(deferredTextMutex);
			return;
		}
		SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
		
		deferredTextIdx1++;
		if( deferredTextIdx1 >= DEFERRED_TEXT_COUNT )
			deferredTextIdx1 = 0;
		
		ev->type = SDL_KEYDOWN;
		ev->key.state = deferredText[deferredTextIdx1].down;
		ev->key.keysym = asciiToKeysym( deferredText[deferredTextIdx1].scancode, deferredText[deferredTextIdx1].unicode );
		
		BufferedEventsEnd = nextEvent;
		SDL_mutexV(BufferedEventsMutex);
		if( SDL_ANDROID_isMouseUsed )
			SDL_ANDROID_MainThreadPushMouseMotion(SDL_ANDROID_currentMouseX + (SDL_ANDROID_currentMouseX % 2 ? -1 : 1), SDL_ANDROID_currentMouseY); // Force screen redraw
	}
	else
	{
		if( SDL_ANDROID_TextInputFinished )
		{
			SDL_ANDROID_TextInputFinished = 0;
			SDL_ANDROID_IsScreenKeyboardShownFlag = 0;
		}
	}
	
	SDL_mutexV(deferredTextMutex);
};

extern void SDL_ANDROID_MainThreadPushText( int ascii, int unicode )
{
	int shiftRequired;

	int nextEvent = getNextEventAndLock();
	if( nextEvent == -1 )
		return;
	
	SDL_Event * ev = &BufferedEvents[BufferedEventsEnd];
	
#if SDL_VERSION_ATLEAST(1,3,0)

	ev->type = SDL_TEXTINPUT;
	UnicodeToUtf8(unicode, ev->text.text);

#endif

	if( !deferredTextMutex )
		deferredTextMutex = SDL_CreateMutex();

	SDL_mutexP(deferredTextMutex);

	ev->type = 0;
	
	shiftRequired = checkShiftRequired(&ascii);
	
	if( shiftRequired )
	{
		deferredTextIdx2++;
		if( deferredTextIdx2 >= DEFERRED_TEXT_COUNT )
			deferredTextIdx2 = 0;
		deferredText[deferredTextIdx2].down = SDL_PRESSED;
		deferredText[deferredTextIdx2].scancode = SDLK_LSHIFT;
		deferredText[deferredTextIdx2].unicode = 0;
	}
	deferredTextIdx2++;
	if( deferredTextIdx2 >= DEFERRED_TEXT_COUNT )
		deferredTextIdx2 = 0;
	deferredText[deferredTextIdx2].down = SDL_PRESSED;
	deferredText[deferredTextIdx2].scancode = ascii;
	deferredText[deferredTextIdx2].unicode = unicode;

	deferredTextIdx2++;
	if( deferredTextIdx2 >= DEFERRED_TEXT_COUNT )
		deferredTextIdx2 = 0;
	deferredText[deferredTextIdx2].down = SDL_RELEASED;
	deferredText[deferredTextIdx2].scancode = ascii;
	deferredText[deferredTextIdx2].unicode = 0;
	if( shiftRequired )
	{
		deferredTextIdx2++;
		if( deferredTextIdx2 >= DEFERRED_TEXT_COUNT )
			deferredTextIdx2 = 0;
		deferredText[deferredTextIdx2].down = SDL_RELEASED;
		deferredText[deferredTextIdx2].scancode = SDLK_LSHIFT;
		deferredText[deferredTextIdx2].unicode = 0;
	}

	SDL_mutexV(deferredTextMutex);

	BufferedEventsEnd = nextEvent;
	SDL_mutexV(BufferedEventsMutex);
};

#endif

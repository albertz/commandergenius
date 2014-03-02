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
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <netinet/in.h>

#include "SDL_config.h"

#include "SDL_version.h"

//#include "SDL_opengles.h"
#include "SDL_screenkeyboard.h"
#include "../SDL_sysvideo.h"
#include "SDL_androidvideo.h"
#include "SDL_androidinput.h"
#include "jniwrapperstuff.h"
#include "atan2i.h"

// TODO: this code is a HUGE MESS

enum { MAX_BUTTONS = SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM-1, MAX_JOYSTICKS = 3, MAX_BUTTONS_AUTOFIRE = 2, BUTTON_TEXT_INPUT = SDL_ANDROID_SCREENKEYBOARD_BUTTON_TEXT, BUTTON_ARROWS = SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD } ; // Max amount of custom buttons

int SDL_ANDROID_isTouchscreenKeyboardUsed = 0;
static short touchscreenKeyboardTheme = 0;
static short touchscreenKeyboardShown = 1;
static SDL_Rect hiddenButtons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM];
static short buttonsize = 1;
static short buttonDrawSize = 1;
static float transparency = 128.0f/255.0f;

static SDL_Rect arrows[MAX_JOYSTICKS], arrowsExtended[MAX_JOYSTICKS], buttons[MAX_BUTTONS];
static SDL_Rect arrowsDraw[MAX_JOYSTICKS], buttonsDraw[MAX_BUTTONS];
static SDLKey buttonKeysyms[MAX_BUTTONS] = {
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_0)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_1)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_2)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_3)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_4)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_SCREENKB_KEYCODE_5)),
0
};

enum { ARROW_LEFT = 1, ARROW_RIGHT = 2, ARROW_UP = 4, ARROW_DOWN = 8 };
static short oldArrows = 0;

static Sint8 pointerInButtonRect[MAX_BUTTONS + MAX_JOYSTICKS];
static Sint8 buttonsGenerateSdlEvents[MAX_BUTTONS + MAX_JOYSTICKS];
static Sint8 buttonsStayPressedAfterTouch[MAX_BUTTONS + MAX_JOYSTICKS];

typedef struct
{
    GLuint id;
    GLfloat w;
    GLfloat h;
} GLTexture_t;

static GLTexture_t arrowImages[5];
static GLTexture_t buttonAutoFireImages[MAX_BUTTONS_AUTOFIRE*2]; // These are not used anymore
static GLTexture_t buttonImages[MAX_BUTTONS*2];
static GLTexture_t mousePointer;
enum { MOUSE_POINTER_W = 32, MOUSE_POINTER_H = 32, MOUSE_POINTER_X = 5, MOUSE_POINTER_Y = 7 }; // X and Y are offsets of the pointer tip

static int sunTheme = 0;
static int joystickTouchPoints[MAX_JOYSTICKS*2];

static void R_DumpOpenGlState(void);

static inline int InsideRect(const SDL_Rect * r, int x, int y)
{
	return ( x >= r->x && x <= r->x + r->w ) && ( y >= r->y && y <= r->y + r->h );
}

static struct ScreenKbGlState_t
{
	GLboolean texture2d;
	GLuint texunitId;
	GLuint clientTexunitId;
	GLuint textureId;
	GLfloat color[4];
	GLint texEnvMode;
	GLboolean blend;
	GLenum blend1, blend2;
	GLint texFilter1, texFilter2;
	GLboolean colorArray;
}
oldGlState;

static inline void beginDrawingTex()
{
#ifndef SDL_TOUCHSCREEN_KEYBOARD_SAVE_RESTORE_OPENGL_STATE
	// Make the video somehow work on emulator
	oldGlState.texture2d = GL_TRUE;
	oldGlState.texunitId = GL_TEXTURE0;
	oldGlState.clientTexunitId = GL_TEXTURE0;
	oldGlState.textureId = 0;
	oldGlState.texEnvMode = GL_MODULATE;
	oldGlState.blend = GL_TRUE;
	oldGlState.blend1 = GL_SRC_ALPHA;
	oldGlState.blend2 = GL_ONE_MINUS_SRC_ALPHA;
	oldGlState.colorArray = GL_FALSE;
#else
	// Save OpenGL state
	// This code does not work on 1.6 emulator, and on some older devices
	// However GLES 1.1 spec defines all theese values, so it's a device fault for not implementing them
	oldGlState.texture2d = glIsEnabled(GL_TEXTURE_2D);
	glGetIntegerv(GL_ACTIVE_TEXTURE, &oldGlState.texunitId);
	glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE, &oldGlState.clientTexunitId);
#endif

	//R_DumpOpenGlState();

	/*
	glActiveTexture(GL_TEXTURE1);
	glClientActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	*/

	glActiveTexture(GL_TEXTURE0);
	glClientActiveTexture(GL_TEXTURE0);

#ifdef SDL_TOUCHSCREEN_KEYBOARD_SAVE_RESTORE_OPENGL_STATE
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldGlState.textureId);
	glGetFloatv(GL_CURRENT_COLOR, &(oldGlState.color[0]));
	glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &oldGlState.texEnvMode);
	oldGlState.blend = glIsEnabled(GL_BLEND);
	glGetIntegerv(GL_BLEND_SRC, &oldGlState.blend1);
	glGetIntegerv(GL_BLEND_DST, &oldGlState.blend2);
	glGetBooleanv(GL_COLOR_ARRAY, &oldGlState.colorArray);
	// It's very unlikely that some app will use GL_TEXTURE_CROP_RECT_OES, so just skip it
#endif

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisableClientState(GL_COLOR_ARRAY);
	//static const GLfloat color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	//glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_ALPHA_TEST);
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	//glDisableClientState(GL_NORMAL_ARRAY);
	//glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

static inline void endDrawingTex()
{
	// Restore OpenGL state
	if( oldGlState.texture2d == GL_FALSE )
		glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, oldGlState.textureId);
	glColor4f(oldGlState.color[0], oldGlState.color[1], oldGlState.color[2], oldGlState.color[3]);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, oldGlState.texEnvMode);
	if( oldGlState.blend == GL_FALSE )
		glDisable(GL_BLEND);
	glBlendFunc(oldGlState.blend1, oldGlState.blend2);
	glActiveTexture(oldGlState.texunitId);
	glClientActiveTexture(oldGlState.clientTexunitId);
	if( oldGlState.colorArray )
		glEnableClientState(GL_COLOR_ARRAY);
}

static inline void drawCharTexFlip(GLTexture_t * tex, SDL_Rect * src, SDL_Rect * dest, int flipX, int flipY, float r, float g, float b, float a)
{
	GLint cropRect[4];

	if( !dest->h || !dest->w )
		return;

	glBindTexture(GL_TEXTURE_2D, tex->id);

	glColor4f(r, g, b, a);

	if(src)
	{
		cropRect[0] = src->x;
		cropRect[1] = tex->h - src->y;
		cropRect[2] = src->w;
		cropRect[3] = -src->h;
	}
	else
	{
		cropRect[0] = 0;
		cropRect[1] = tex->h;
		cropRect[2] = tex->w;
		cropRect[3] = -tex->h;
	}
	if(flipX)
	{
		cropRect[0] += cropRect[2];
		cropRect[2] = -cropRect[2];
	}
	if(flipY)
	{
		cropRect[1] += cropRect[3];
		cropRect[3] = -cropRect[3];
	}
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropRect);
	glDrawTexiOES(dest->x, SDL_ANDROID_sRealWindowHeight - dest->y - dest->h, 0, dest->w, dest->h);
}

static inline void drawCharTex(GLTexture_t * tex, SDL_Rect * src, SDL_Rect * dest, float r, float g, float b, float a)
{
	drawCharTexFlip(tex, src, dest, 0, 0, r, g, b, a);
}

static void drawTouchscreenKeyboardLegacy()
{
	int i;
	float blendFactor;

	blendFactor =		( SDL_GetKeyboardState(NULL)[SDL_KEY(LEFT)] ? 1 : 0 ) +
						( SDL_GetKeyboardState(NULL)[SDL_KEY(RIGHT)] ? 1 : 0 ) +
						( SDL_GetKeyboardState(NULL)[SDL_KEY(UP)] ? 1 : 0 ) +
						( SDL_GetKeyboardState(NULL)[SDL_KEY(DOWN)] ? 1 : 0 );
	if( blendFactor == 0 || SDL_ANDROID_joysticksAmount >= 1 )
		drawCharTex( &arrowImages[0], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency );
	else
	{
		if( SDL_GetKeyboardState(NULL)[SDL_KEY(LEFT)] )
			drawCharTex( &arrowImages[1], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency / blendFactor );
		if( SDL_GetKeyboardState(NULL)[SDL_KEY(RIGHT)] )
			drawCharTex( &arrowImages[2], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency / blendFactor );
		if( SDL_GetKeyboardState(NULL)[SDL_KEY(UP)] )
			drawCharTex( &arrowImages[3], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency / blendFactor );
		if( SDL_GetKeyboardState(NULL)[SDL_KEY(DOWN)] )
			drawCharTex( &arrowImages[4], NULL, &arrowsDraw[0], 1.0f, 1.0f, 1.0f, transparency / blendFactor );
	}
	if( SDL_ANDROID_joysticksAmount >= 2 )
		drawCharTex( &arrowImages[0], NULL, &arrowsDraw[1], 1.0f, 1.0f, 1.0f, transparency );
	if( SDL_ANDROID_joysticksAmount >= 3 )
		drawCharTex( &arrowImages[0], NULL, &arrowsDraw[2], 1.0f, 1.0f, 1.0f, transparency );

	for( i = 0; i < MAX_BUTTONS; i++ )
	{
		if( ! buttons[i].h || ! buttons[i].w )
			continue;

		drawCharTex( &buttonImages[ SDL_GetKeyboardState(NULL)[buttonKeysyms[i]] ? (i * 2 + 1) : (i * 2) ],
					NULL, &buttonsDraw[i], 1.0f, 1.0f, 1.0f, transparency );
	}
}

static void drawTouchscreenKeyboardSun()
{
	int i;

	for( i = 0; i < SDL_ANDROID_joysticksAmount || (i == 0 && arrowsDraw[0].w > 0); i++ )
	{
		drawCharTex( &arrowImages[0], NULL, &arrowsDraw[i], 1.0f, 1.0f, 1.0f, transparency );
		if(pointerInButtonRect[BUTTON_ARROWS+i] != -1)
		{
			SDL_Rect touch = arrowsDraw[i];
			touch.w /= 2;
			touch.h /= 2;
			touch.x = joystickTouchPoints[0+i*2] - touch.w / 2;
			touch.y = joystickTouchPoints[1+i*2] - touch.h / 2;
			drawCharTex( &arrowImages[0], NULL, &touch, 1.0f, 1.0f, 1.0f, transparency );
		}
	}

	for( i = 0; i < MAX_BUTTONS; i++ )
	{
		int pressed = SDL_GetKeyboardState(NULL)[buttonKeysyms[i]];
		if( ! buttons[i].h || ! buttons[i].w )
			continue;

		drawCharTexFlip( &buttonImages[ pressed ? (i * 2 + 1) : (i * 2) ],
						NULL, &buttonsDraw[i], (i >= 2 && pressed), 0, 1.0f, 1.0f, 1.0f, transparency );
	}
}

int SDL_ANDROID_drawTouchscreenKeyboard()
{
	if( !SDL_ANDROID_isTouchscreenKeyboardUsed || !touchscreenKeyboardShown )
		return 0;

	beginDrawingTex();

	if(sunTheme)
		drawTouchscreenKeyboardSun();
	else
		drawTouchscreenKeyboardLegacy();

	endDrawingTex();

	return 1;
};

static inline int ArrowKeysPressed(int x, int y)
{
	int ret = 0, dx, dy;
	dx = x - arrows[0].x - arrows[0].w / 2;
	dy = y - arrows[0].y - arrows[0].h / 2;
	// Single arrow key pressed
	if( abs(dy / 2) >= abs(dx) )
	{
		if( dy < 0 )
			ret |= ARROW_UP;
		else
			ret |= ARROW_DOWN;
	}
	else
	if( abs(dx / 2) >= abs(dy) )
	{
		if( dx > 0 )
			ret |= ARROW_RIGHT;
		else
			ret |= ARROW_LEFT;
	}
	else // Two arrow keys pressed
	{
		if( dx > 0 )
			ret |= ARROW_RIGHT;
		else
			ret |= ARROW_LEFT;

		if( dy < 0 )
			ret |= ARROW_UP;
		else
			ret |= ARROW_DOWN;
	}
	return ret;
}

unsigned SDL_ANDROID_processTouchscreenKeyboard(int x, int y, int action, int pointerId)
{
	int i, j;
	unsigned processed = 0;
	int joyAmount = SDL_ANDROID_joysticksAmount;
	if( joyAmount == 0 && arrows[0].w > 0 )
		joyAmount = 1;
	
	if( !touchscreenKeyboardShown )
		return 0;
	
	if( action == MOUSE_DOWN )
	{
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "touch %03dx%03d ptr %d action %d", x, y, pointerId, action);
		for( j = 0; j < joyAmount; j++ )
		{
			if( InsideRect( &arrows[j], x, y ) )
			{
				processed |= 1<<(BUTTON_ARROWS+j);
				if( pointerInButtonRect[BUTTON_ARROWS+j] == -1 )
				{
					pointerInButtonRect[BUTTON_ARROWS+j] = pointerId;
					joystickTouchPoints[0+j*2] = x;
					joystickTouchPoints[1+j*2] = y;
					if( SDL_ANDROID_joysticksAmount > 0 )
					{
						int xx = (x - arrows[j].x - arrows[j].w / 2) * 65534 / arrows[j].w;
						if( xx == 0 ) // Do not allow (0,0) coordinate, when the user touches the joystick
							xx = 1;
						int axis = j < 2 ? j*2 : MAX_MULTITOUCH_POINTERS + 4;
						SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis, xx );
						SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis + 1, (y - arrows[j].y - arrows[j].h / 2) * 65534 / arrows[j].h );
					}
					else
					{
						i = ArrowKeysPressed(x, y);
						if( i & ARROW_UP )
							SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(UP), 0 );
						if( i & ARROW_DOWN )
							SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(DOWN), 0 );
						if( i & ARROW_LEFT )
							SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(LEFT), 0 );
						if( i & ARROW_RIGHT )
							SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(RIGHT), 0 );
						oldArrows = i;
					}
				}
			}
		}

		for( i = 0; i < MAX_BUTTONS; i++ )
		{
			if( ! buttons[i].h || ! buttons[i].w )
				continue;
			if( InsideRect( &buttons[i], x, y) )
			{
				processed |= 1<<i;
				if( pointerInButtonRect[i] == -1 )
				{
					pointerInButtonRect[i] = pointerId;
					if( i == BUTTON_TEXT_INPUT )
						SDL_ANDROID_ToggleScreenKeyboardTextInput(NULL);
					else if( buttonsStayPressedAfterTouch[i] )
						SDL_ANDROID_MainThreadPushKeyboardKey( SDL_GetKeyboardState(NULL)[buttonKeysyms[i]] == 0 ? SDL_PRESSED : SDL_RELEASED, buttonKeysyms[i], 0 );
					else
						SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, buttonKeysyms[i], 0 );
				}
			}
		}
	}
	else
	if( action == MOUSE_UP )
	{
		//__android_log_print(ANDROID_LOG_INFO, "libSDL", "touch %03dx%03d ptr %d action %d", x, y, pointerId, action);
		for( j = 0; j < joyAmount; j++ )
		{
			if( pointerInButtonRect[BUTTON_ARROWS+j] == pointerId )
			{
				processed |= 1<<(BUTTON_ARROWS+j);
				pointerInButtonRect[BUTTON_ARROWS+j] = -1;
				if( SDL_ANDROID_joysticksAmount > 0 )
				{
					int axis = j < 2 ? j*2 : MAX_MULTITOUCH_POINTERS + 4;
					SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis, 0 );
					SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis + 1, 0 );
				}
				else
				{
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(UP), 0 );
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(DOWN), 0 );
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(LEFT), 0 );
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(RIGHT), 0 );
					oldArrows = 0;
				}
			}
		}
		for( i = 0; i < MAX_BUTTONS; i++ )
		{
			if( ! buttons[i].h || ! buttons[i].w )
				continue;
			if( pointerInButtonRect[i] == pointerId )
			{
				processed |= 1<<i;
				pointerInButtonRect[i] = -1;
				if( i != BUTTON_TEXT_INPUT && !buttonsStayPressedAfterTouch[i] )
					SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, buttonKeysyms[i], 0 );
			}
		}
	}
	else
	if( action == MOUSE_MOVE )
	{
		// Process cases when pointer enters button area (it won't send keypress twice if button already pressed)
		int processOtherButtons = 1;
		for( i = 0; i < MAX_BUTTONS; i++ )
		{
			if( buttonsGenerateSdlEvents[i] && pointerInButtonRect[i] == pointerId )
			{
				processOtherButtons = 0;
				break;
			}
		}
		if( processOtherButtons )
		{
			processed |= SDL_ANDROID_processTouchscreenKeyboard(x, y, MOUSE_DOWN, pointerId);
		}
		
		// Process cases when pointer leaves button area
		// TODO: huge code size, split it or somehow make it more readable
		for( j = 0; j < joyAmount; j++ )
		{
			if( pointerInButtonRect[BUTTON_ARROWS+j] == pointerId )
			{
				processed |= 1<<(BUTTON_ARROWS+j);
				if( ! InsideRect( &arrowsExtended[j], x, y ) )
				{
					pointerInButtonRect[BUTTON_ARROWS+j] = -1;
					if( SDL_ANDROID_joysticksAmount > 0 )
					{
						int axis = j < 2 ? j*2 : MAX_MULTITOUCH_POINTERS + 4;
						SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis, 0 );
						SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis + 1, 0 );
					}
					else
					{
						SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(UP), 0 );
						SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(DOWN), 0 );
						SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(LEFT), 0 );
						SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(RIGHT), 0 );
						oldArrows = 0;
					}
				}
				else
				{
					joystickTouchPoints[0+j*2] = x;
					joystickTouchPoints[1+j*2] = y;
					if( SDL_ANDROID_joysticksAmount > 0 )
					{
						int axis = j < 2 ? j*2 : MAX_MULTITOUCH_POINTERS + 4;
						SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis, (x - arrows[j].x - arrows[j].w / 2) * 65534 / arrows[j].w );
						SDL_ANDROID_MainThreadPushJoystickAxis( 0, axis + 1, (y - arrows[j].y - arrows[j].h / 2) * 65534 / arrows[j].h );
					}
					else
					{
						i = ArrowKeysPressed(x, y);
						if( i != oldArrows )
						{
							if( oldArrows & ARROW_UP && ! (i & ARROW_UP) )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(UP), 0 );
							if( oldArrows & ARROW_DOWN && ! (i & ARROW_DOWN) )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(DOWN), 0 );
							if( oldArrows & ARROW_LEFT && ! (i & ARROW_LEFT) )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(LEFT), 0 );
							if( oldArrows & ARROW_RIGHT && ! (i & ARROW_RIGHT) )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, SDL_KEY(RIGHT), 0 );
							if( i & ARROW_UP )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(UP), 0 );
							if( i & ARROW_DOWN )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(DOWN), 0 );
							if( i & ARROW_LEFT )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(LEFT), 0 );
							if( i & ARROW_RIGHT )
								SDL_ANDROID_MainThreadPushKeyboardKey( SDL_PRESSED, SDL_KEY(RIGHT), 0 );
						}
						oldArrows = i;
					}
				}
			}
		}
		for( i = 0; i < MAX_BUTTONS; i++ )
		{
			if( ! buttons[i].h || ! buttons[i].w )
				continue;
			if( pointerInButtonRect[i] == pointerId )
			{
				processed |= 1<<i;
				if( ! InsideRect( &buttons[i], x, y ) && ! buttonsGenerateSdlEvents[i] )
				{
					pointerInButtonRect[i] = -1;
					if( i != BUTTON_TEXT_INPUT )
						SDL_ANDROID_MainThreadPushKeyboardKey( SDL_RELEASED, buttonKeysyms[i], 0 );
				}
			}
		}
	}

	for( i = 0; i <= MAX_BUTTONS ; i++ )
		if( ( processed & (1<<i) ) && buttonsGenerateSdlEvents[i] )
			processed |= TOUCHSCREEN_KEYBOARD_PASS_EVENT_DOWN_TO_SDL;
	return processed;
};

void shrinkButtonRect(SDL_Rect s, SDL_Rect * d)
{
	int i;

	if( !buttonDrawSize )
	{
		memcpy(d, &s, sizeof(s));
		return;
	}

	d->w = s.w * 2 / (buttonDrawSize+2);
	d->h = s.h * 2 / (buttonDrawSize+2);
	d->x = s.x + s.w / 2 - d->w / 2;
	d->y = s.y + s.h / 2 - d->h / 2;
}

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetupScreenKeyboard) ( JNIEnv*  env, jobject thiz, jint size, jint drawsize, jint theme, jint _transparency )
{
	int i, ii;
	int nbuttons1row, nbuttons2row;
	int _nbuttons = MAX_BUTTONS;
	SDL_Rect * r;

	// TODO: screenRatio is not used yet
	enum { STANDARD_PHONE_SCREEN_HEIGHT = 70 }; // And by "standard phone", I mean my own.
	float screenRatio = getenv("DISPLAY_HEIGHT_MM") ? atoi(getenv("DISPLAY_HEIGHT_MM")) / STANDARD_PHONE_SCREEN_HEIGHT : 1.0f;
	if( screenRatio < STANDARD_PHONE_SCREEN_HEIGHT )
		screenRatio = STANDARD_PHONE_SCREEN_HEIGHT;

	touchscreenKeyboardTheme = theme;
	// TODO: works for horizontal screen orientation only!
	buttonsize = size;
	buttonDrawSize = drawsize;
	switch(_transparency)
	{
		case 0: transparency = 32.0f/255.0f; break;
		case 1: transparency = 64.0f/255.0f; break;
		case 2: transparency = 128.0f/255.0f; break;
		case 3: transparency = 192.0f/255.0f; break;
		case 4: transparency = 255.0f/255.0f; break;
		default: transparency = 192.0f/255.0f; break;
	}
	
	// Arrows to the lower-left part of screen
	arrows[0].w = SDL_ANDROID_sRealWindowWidth / (size + 2) * 2 / 3;
	arrows[0].h = arrows[0].w;
	// Move to the screen edge
	arrows[0].x = 0;
	arrows[0].y = SDL_ANDROID_sRealWindowHeight - arrows[0].h;

	arrowsExtended[0].w = arrows[0].w * 2;
	arrowsExtended[0].h = arrows[0].h * 2;
	arrowsExtended[0].x = arrows[0].x + arrows[0].w / 2 - arrowsExtended[0].w / 2;
	arrowsExtended[0].y = arrows[0].y + arrows[0].h / 2 - arrowsExtended[0].h / 2;

	arrows[1].w = arrows[0].w;
	arrows[1].h = arrows[0].h;
	arrows[1].x = SDL_ANDROID_sRealWindowWidth - arrows[1].w;
	arrows[1].y = arrows[0].y;

	arrowsExtended[1].w = arrows[1].w * 2;
	arrowsExtended[1].h = arrows[1].h * 2;
	arrowsExtended[1].x = arrows[1].x + arrows[1].w / 2 - arrowsExtended[1].w / 2;
	arrowsExtended[1].y = arrows[1].y + arrows[1].h / 2 - arrowsExtended[1].h / 2;

	arrows[2].w = arrows[1].w;
	arrows[2].h = arrows[1].h;
	arrows[2].x = arrows[1].x;
	arrows[2].y = arrows[1].y - arrows[1].h;

	arrowsExtended[2].w = arrows[2].w * 2;
	arrowsExtended[2].h = arrows[2].h * 2;
	arrowsExtended[2].x = arrows[2].x + arrows[2].w / 2 - arrowsExtended[2].w / 2;
	arrowsExtended[2].y = arrows[2].y + arrows[2].h / 2 - arrowsExtended[2].h / 2;

	// Buttons to the lower-right in 2 rows
	for(i = 0; i < 3; i++)
	for(ii = 0; ii < 2; ii++)
	{
		// Custom button ordering
		int iii = ii + i*2;
		buttons[iii].w = SDL_ANDROID_sRealWindowWidth / (size + 2) / 3;
		buttons[iii].h = buttons[iii].w;
		// Move to the screen edge
		buttons[iii].x = SDL_ANDROID_sRealWindowWidth - buttons[iii].w * (ii + 1);
		buttons[iii].y = SDL_ANDROID_sRealWindowHeight - buttons[iii].h * (i + 1);
	}
	if( SDL_ANDROID_joysticksAmount >= 2 )
	{
		// Move all buttons to center, 5-th and 6-th button will be misplaced, but we don't care much about that.
		ii = SDL_ANDROID_sRealWindowWidth / 2 - buttons[0].w;
		for(i = 0; i < 6; i++)
			buttons[i].x -= ii;
	}
	buttons[6].x = 0;
	buttons[6].y = 0;
	buttons[6].w = SDL_ANDROID_sRealWindowHeight/10;
	buttons[6].h = SDL_ANDROID_sRealWindowHeight/10;

	for( i = 0; i < sizeof(pointerInButtonRect)/sizeof(pointerInButtonRect[0]); i++ )
		pointerInButtonRect[i] = -1;
	for( i = 0; i < MAX_JOYSTICKS; i++ )
		shrinkButtonRect(arrows[i], &arrowsDraw[i]);
	for( i = 0; i < MAX_BUTTONS; i++ )
		shrinkButtonRect(buttons[i], &buttonsDraw[i]);
	for( i = 0; i < SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM; i++ )
		SDL_ANDROID_GetScreenKeyboardButtonPos(i, &hiddenButtons[i]);
};

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetTouchscreenKeyboardUsed) ( JNIEnv*  env, jobject thiz)
{
	SDL_ANDROID_isTouchscreenKeyboardUsed = 1;
}

void SDL_ANDROID_DrawMouseCursor(int x, int y, int size, float alpha)
{
	SDL_Rect r;
	// I've failed with size calculations, so leaving it as-is
	r.x = x - MOUSE_POINTER_X;
	r.y = y - MOUSE_POINTER_Y;
	r.w = MOUSE_POINTER_W;
	r.h = MOUSE_POINTER_H;
	beginDrawingTex();
	drawCharTex( &mousePointer, NULL, &r, 1.0f, 1.0f, 1.0f, alpha );
	endDrawingTex();
}

static int
power_of_2(int input)
{
    int value = 1;

    while (value < input) {
        value <<= 1;
    }
    return value;
}

static int setupScreenKeyboardButtonTexture( GLTexture_t * data, Uint8 * charBuf )
{
	int w, h, format, bpp;
	int texture_w, texture_h;

	memcpy(&w, charBuf, sizeof(int));
	memcpy(&h, charBuf + sizeof(int), sizeof(int));
	memcpy(&format, charBuf + 2*sizeof(int), sizeof(int));
	w = ntohl(w);
	h = ntohl(h);
	format = ntohl(format);
	bpp = 2;
	if(format == 2)
		bpp = 4;

	texture_w = power_of_2(w);
	texture_h = power_of_2(h);
	data->w = w;
	data->h = h;

	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, &data->id);
	glBindTexture(GL_TEXTURE_2D, data->id);
	//__android_log_print(ANDROID_LOG_INFO, "libSDL", "On-screen keyboard generated OpenGL texture ID %d", data->id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_w, texture_h, 0, GL_RGBA,
					bpp == 4 ? GL_UNSIGNED_BYTE : (format ? GL_UNSIGNED_SHORT_4_4_4_4 : GL_UNSIGNED_SHORT_5_5_5_1), NULL);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA,
						bpp == 4 ? GL_UNSIGNED_BYTE : (format ? GL_UNSIGNED_SHORT_4_4_4_4 : GL_UNSIGNED_SHORT_5_5_5_1),
						charBuf + 3*sizeof(int) );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if( SDL_ANDROID_VideoLinearFilter )
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glDisable(GL_TEXTURE_2D);

	return 3*sizeof(int) + w * h * bpp;
}

static int setupScreenKeyboardButtonLegacy( int buttonID, Uint8 * charBuf )
{
	GLTexture_t * data = NULL;

	if( buttonID < 5 )
		data = &(arrowImages[buttonID]);
	else
	if( buttonID < 9 )
		data = &(buttonAutoFireImages[buttonID-5]);
	else
		data = &(buttonImages[buttonID-9]);

	if( buttonID == 23 )
		data = &mousePointer;
	else if( buttonID > 22 ) // Error, array too big
		return 12; // Return value bigger than zero to iterate it

	return setupScreenKeyboardButtonTexture(data, charBuf);
}

static int setupScreenKeyboardButtonSun( int buttonID, Uint8 * charBuf )
{
	GLTexture_t * data = NULL;
	int i, ret;

	if( buttonID == 0 )
		data = &(arrowImages[0]);
	if( buttonID >= 1 && buttonID <= 4 )
		data = &(buttonImages[buttonID-1]);
	if( buttonID >= 5 && buttonID <= 8 )
		data = &(buttonImages[4+(buttonID-5)*2]);
	if( buttonID == 9 )
		data = &mousePointer;
	else if( buttonID > 9 ) // Error, array too big
		return 12; // Return value bigger than zero to iterate it

	ret = setupScreenKeyboardButtonTexture(data, charBuf);

	for( i = 1; i <=4; i++ )
		arrowImages[i] = arrowImages[0];
	
	for( i = 2; i < MAX_BUTTONS; i++ )
		buttonImages[i * 2 + 1] = buttonImages[i * 2];

	for( i = 0; i < MAX_BUTTONS_AUTOFIRE*2; i++ )
		buttonAutoFireImages[i] = arrowImages[0];

	buttonImages[BUTTON_TEXT_INPUT*2] = buttonImages[10];
	buttonImages[BUTTON_TEXT_INPUT*2+1] = buttonImages[10];

	return ret;
}

static int setupScreenKeyboardButton( int buttonID, Uint8 * charBuf, int count )
{
	if(count == 24)
	{
		sunTheme = 0;
		return setupScreenKeyboardButtonLegacy(buttonID, charBuf);
	}
	else if(count == 10)
	{
		sunTheme = 1;
		return setupScreenKeyboardButtonSun(buttonID, charBuf);
	}
	else
	{
		__android_log_print(ANDROID_LOG_FATAL, "libSDL", "On-screen keyboard buton img count = %d, should be 10 or 24", count);
		return 12; // Return value bigger than zero to iterate it
	}
}


JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetupScreenKeyboardButtons) ( JNIEnv*  env, jobject thiz, jbyteArray charBufJava )
{
	jboolean isCopy = JNI_TRUE;
	int len = (*env)->GetArrayLength(env, charBufJava);
	Uint8 * charBuf = (Uint8 *) (*env)->GetByteArrayElements(env, charBufJava, &isCopy);
	int but, pos, count;
	memcpy(&count, charBuf, sizeof(int));
	count = ntohl(count);
	
	for( but = 0, pos = sizeof(int); pos < len; but ++ )
		pos += setupScreenKeyboardButton( but, charBuf + pos, count );
	
	(*env)->ReleaseByteArrayElements(env, charBufJava, (jbyte *)charBuf, 0);
}

JNIEXPORT jint JNICALL
JAVA_EXPORT_NAME(Settings_nativeGetKeymapKeyScreenKb) (JNIEnv* env, jobject thiz, jint keynum)
{
	if( keynum < 0 || keynum > SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 - SDL_ANDROID_SCREENKEYBOARD_BUTTON_0 + 4 )
		return SDL_KEY(UNKNOWN);
		
	if( keynum <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 - SDL_ANDROID_SCREENKEYBOARD_BUTTON_0 )
		return SDL_ANDROID_GetScreenKeyboardButtonKey(keynum + SDL_ANDROID_SCREENKEYBOARD_BUTTON_0);

	return SDL_KEY(UNKNOWN);
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetKeymapKeyScreenKb) (JNIEnv* env, jobject thiz, jint keynum, jint key)
{
	if( keynum < 0 || keynum > SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 - SDL_ANDROID_SCREENKEYBOARD_BUTTON_0 + 4 )
		return;
		
	if( keynum <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 - SDL_ANDROID_SCREENKEYBOARD_BUTTON_0 )
		SDL_ANDROID_SetScreenKeyboardButtonKey(keynum + SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, key);
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetScreenKbKeyUsed) (JNIEnv*  env, jobject thiz, jint keynum, jint used)
{
	SDL_Rect rect = {0, 0, 0, 0};
	int key = -1;
	if( keynum == 0 )
		key = SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD;
	if( keynum == 1 )
		key = SDL_ANDROID_SCREENKEYBOARD_BUTTON_TEXT;
	if( keynum - 2 >= 0 && keynum - 2 <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 - SDL_ANDROID_SCREENKEYBOARD_BUTTON_0 )
		key = keynum - 2 + SDL_ANDROID_SCREENKEYBOARD_BUTTON_0;

	if( keynum == SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2 )
		key = SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2;
	if( keynum == SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3 )
		key = SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3;

	if( key >= 0 && !used )
		SDL_ANDROID_SetScreenKeyboardButtonPos(key, &rect);
}


int SDL_ANDROID_SetScreenKeyboardButtonPos(int buttonId, SDL_Rect * pos)
{
	if( buttonId < 0 || buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM || ! pos )
		return 0;
	
	if( buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD && buttonId <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3 )
	{
		int i = buttonId - SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD;
		arrows[i] = *pos;
		arrowsExtended[i].w = arrows[i].w * 2;
		arrowsExtended[i].h = arrows[i].h * 2;
		arrowsExtended[i].x = arrows[i].x + arrows[i].w / 2 - arrowsExtended[i].w / 2;
		arrowsExtended[i].y = arrows[i].y + arrows[i].h / 2 - arrowsExtended[i].h / 2;
		shrinkButtonRect(arrows[i], &arrowsDraw[i]);
	}
	else
	{
		int i = buttonId;
		buttons[i] = *pos;
		shrinkButtonRect(buttons[i], &buttonsDraw[i]);
	}
	return 1;
};

int SDLCALL SDL_ANDROID_SetScreenKeyboardButtonImagePos(int buttonId, SDL_Rect * pos)
{
	if( buttonId < 0 || buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM || ! pos )
		return 0;

	if( buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD && buttonId <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3 )
		arrowsDraw[buttonId - SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD] = *pos;
	else
		buttonsDraw[buttonId] = *pos;

	return 1;
}

int SDL_ANDROID_GetScreenKeyboardButtonPos(int buttonId, SDL_Rect * pos)
{
	if( buttonId < 0 || buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM || ! pos )
		return 0;
	
	if( buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD && buttonId <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3 )
	{
		*pos = arrows[buttonId - SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD];
	}
	else
	{
		*pos = buttons[buttonId];
	}
	return 1;
};

int SDL_ANDROID_SetScreenKeyboardButtonKey(int buttonId, SDLKey key)
{
	if( buttonId < 0 || buttonId > SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 || ! key )
		return 0;
	buttonKeysyms[buttonId] = key;
	return 1;
};

SDLKey SDL_ANDROID_GetScreenKeyboardButtonKey(int buttonId)
{
	if( buttonId < 0 || buttonId > SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 )
		return SDLK_UNKNOWN;
	return buttonKeysyms[buttonId];
};

int SDL_ANDROID_SetScreenKeyboardShown(int shown)
{
	touchscreenKeyboardShown = shown;
};

int SDL_ANDROID_GetScreenKeyboardShown(void)
{
	return touchscreenKeyboardShown;
};

int SDL_ANDROID_SetScreenKeyboardButtonShown(int buttonId, int shown)
{
	if( buttonId < 0 || buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM )
		return 0;

	//__android_log_print(ANDROID_LOG_INFO, "libsdl", "SDL_ANDROID_SetScreenKeyboardButtonShown: button %d shown %d", buttonId, shown);
	if( !shown && SDL_ANDROID_GetScreenKeyboardButtonShown(buttonId) )
	{
		SDL_Rect pos = { 0, 0, 0, 0 };
		SDL_ANDROID_GetScreenKeyboardButtonPos(buttonId, &hiddenButtons[buttonId]);
		SDL_ANDROID_SetScreenKeyboardButtonPos(buttonId, &pos);
	}
	if( shown && !SDL_ANDROID_GetScreenKeyboardButtonShown(buttonId) )
		SDL_ANDROID_SetScreenKeyboardButtonPos(buttonId, &hiddenButtons[buttonId]);
	return 1;
};

int SDL_ANDROID_GetScreenKeyboardButtonShown(int buttonId)
{
	SDL_Rect pos;
	if( !SDL_ANDROID_GetScreenKeyboardButtonPos(buttonId, &pos) )
		return 0;
	return pos.h > 0 && pos.w > 0;
};

int SDL_ANDROID_GetScreenKeyboardSize(void)
{
	return buttonsize;
};

int SDL_ANDROID_ToggleScreenKeyboardTextInput(const char * previousText)
{
	static char textIn[255];
	if( previousText == NULL )
		previousText = "";
	strncpy(textIn, previousText, sizeof(textIn));
	textIn[sizeof(textIn)-1] = 0;
	SDL_ANDROID_CallJavaShowScreenKeyboard(textIn, NULL, 0);
	return 1;
};

int SDLCALL SDL_ANDROID_GetScreenKeyboardTextInput(char * textBuf, int textBufSize)
{
	SDL_ANDROID_CallJavaShowScreenKeyboard(textBuf, textBuf, textBufSize);
	return 1;
};

int SDLCALL SDL_HasScreenKeyboardSupport(void *unused)
{
	return 1;
}

// SDL2 compatibility
int SDLCALL SDL_ShowScreenKeyboard(void *unused)
{
	return SDL_ANDROID_ToggleScreenKeyboardTextInput(NULL);
}

int SDLCALL SDL_HideScreenKeyboard(void *unused)
{
	SDL_ANDROID_CallJavaHideScreenKeyboard();
	return 1;
}

int SDLCALL SDL_IsScreenKeyboardShown(void *unused)
{
	return SDL_ANDROID_IsScreenKeyboardShown();
}

int SDLCALL SDL_ToggleScreenKeyboard(void *unused)
{
	if( SDL_IsScreenKeyboardShown(NULL) )
		return SDL_HideScreenKeyboard(NULL);
	else
		return SDL_ShowScreenKeyboard(NULL);
}

int SDLCALL SDL_ANDROID_SetScreenKeyboardButtonGenerateTouchEvents(int buttonId, int generateEvents)
{
	if( buttonId < 0 || buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM )
		return 0;
	buttonsGenerateSdlEvents[buttonId] = generateEvents;
	return 1;
}

int SDLCALL SDL_ANDROID_SetScreenKeyboardButtonStayPressedAfterTouch(int buttonId, int stayPressed)
{
	if( buttonId < 0 || buttonId >= SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM )
		return 0;
	buttonsStayPressedAfterTouch[buttonId] = stayPressed;
	return 1;
}

int SDLCALL SDL_ANDROID_SetScreenKeyboardTransparency(int alpha)
{
	transparency = (float)alpha / 255.0f;
}

static int ScreenKbRedefinedByUser = 0;

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetScreenKbKeyLayout) (JNIEnv* env, jobject thiz, jint keynum, jint x1, jint y1, jint x2, jint y2)
{
	SDL_Rect rect = {x1, y1, x2-x1, y2-y1};
	int key = -1;
	// Why didn't I use consistent IDs between Java and C code?
	if( keynum == 0 )
		key = SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD;
	if( keynum == 1 )
		key = SDL_ANDROID_SCREENKEYBOARD_BUTTON_TEXT;
	if( keynum - 2 >= 0 && keynum - 2 <= SDL_ANDROID_SCREENKEYBOARD_BUTTON_5 - SDL_ANDROID_SCREENKEYBOARD_BUTTON_0 )
		key = keynum - 2 + SDL_ANDROID_SCREENKEYBOARD_BUTTON_0;
	if( keynum == SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2 ) // This one is consistent by chance
		key = SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2;
	if( keynum == SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3 ) // This one is consistent by chance
		key = SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3;

	if( key >= 0 )
	{
		ScreenKbRedefinedByUser = 1;
		SDL_ANDROID_SetScreenKeyboardButtonPos(key, &rect);
	}
}

int SDL_ANDROID_GetScreenKeyboardRedefinedByUser()
{
	return ScreenKbRedefinedByUser;
}

int SDL_ANDROID_SetScreenKeyboardHintMesage(const char * hint)
{
	SDL_ANDROID_CallJavaSetScreenKeyboardHintMessage(hint);
	return 1;
}

/**
 * @brief Dumps OpenGL state for debugging - typically every capability set with glEnable().
 */
void R_DumpOpenGlState(void)
{
#define CAPABILITY( X ) {GL_ ## X, # X}
	/* List taken from here: http://www.khronos.org/opengles/sdk/1.1/docs/man/glIsEnabled.xml */
	const struct { GLenum idx; const char * text; } openGLCaps[] = {
		CAPABILITY(ALPHA_TEST),
		CAPABILITY(BLEND),
		CAPABILITY(COLOR_ARRAY),
		CAPABILITY(COLOR_LOGIC_OP),
		CAPABILITY(COLOR_MATERIAL),
		CAPABILITY(CULL_FACE),
		CAPABILITY(DEPTH_TEST),
		CAPABILITY(DITHER),
		CAPABILITY(FOG),
		CAPABILITY(LIGHTING),
		CAPABILITY(LINE_SMOOTH),
		CAPABILITY(MULTISAMPLE),
		CAPABILITY(NORMAL_ARRAY),
		CAPABILITY(NORMALIZE),
		CAPABILITY(POINT_SMOOTH),
		CAPABILITY(POLYGON_OFFSET_FILL),
		CAPABILITY(RESCALE_NORMAL),
		CAPABILITY(SAMPLE_ALPHA_TO_COVERAGE),
		CAPABILITY(SAMPLE_ALPHA_TO_ONE),
		CAPABILITY(SAMPLE_COVERAGE),
		CAPABILITY(SCISSOR_TEST),
		CAPABILITY(STENCIL_TEST),
		CAPABILITY(VERTEX_ARRAY)
	};
#undef CAPABILITY

	char s[1024] = "";
	GLint i;
	GLint maxTexUnits = 0;
	GLint activeTexUnit = 0;
	GLint activeClientTexUnit = 0;
	GLint activeTexId = 0;
	GLfloat texEnvMode = 0;
	const char * texEnvModeStr = "UNKNOWN";
	GLfloat color[4];

	for (i = 0; i < sizeof(openGLCaps) / sizeof(openGLCaps[0]); i++) {
		if (glIsEnabled(openGLCaps[i].idx)) {
			strcat(s, openGLCaps[i].text);
			strcat(s, " ");
		}
	}
	glGetFloatv(GL_CURRENT_COLOR, color);

	__android_log_print(ANDROID_LOG_INFO, "libSDL", "OpenGL enabled caps: %s color %f %f %f %f \n", s, color[0], color[1], color[2], color[3]);

	glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexUnit);
	glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE, &activeClientTexUnit);

	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTexUnits);
	for (i = GL_TEXTURE0; i < GL_TEXTURE0 + maxTexUnits; i++) {
		glActiveTexture(i);
		glClientActiveTexture(i);

		strcpy(s, "");
		if (glIsEnabled (GL_TEXTURE_2D))
			strcat(s, "enabled, ");
		if (glIsEnabled (GL_TEXTURE_COORD_ARRAY))
			strcat(s, "with texcoord array, ");
		if (i == activeTexUnit)
			strcat(s, "active, ");
		if (i == activeClientTexUnit)
			strcat(s, "client active, ");

		glGetIntegerv(GL_TEXTURE_BINDING_2D, &activeTexId);
		glGetTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &texEnvMode);
		if (fabs(texEnvMode - GL_ADD) < 0.1f)
			texEnvModeStr = "ADD";
		if (fabs(texEnvMode - GL_MODULATE) < 0.1f)
			texEnvModeStr = "MODULATE";
		if (fabs(texEnvMode - GL_DECAL) < 0.1f)
			texEnvModeStr = "DECAL";
		if (fabs(texEnvMode - GL_BLEND) < 0.1f)
			texEnvModeStr = "BLEND";
		if (fabs(texEnvMode - GL_REPLACE) < 0.1f)
			texEnvModeStr = "REPLACE";
		if (fabs(texEnvMode - GL_COMBINE) < 0.1f)
			texEnvModeStr = "COMBINE";

		__android_log_print(ANDROID_LOG_INFO, "libSDL", "Texunit: %d texID %d %s texEnv mode %s\n", i - GL_TEXTURE0, activeTexId, s, texEnvModeStr);
	}

	glActiveTexture(activeTexUnit);
	glClientActiveTexture(activeClientTexUnit);
}

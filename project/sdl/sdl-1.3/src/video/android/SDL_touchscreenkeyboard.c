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

#include "SDL_config.h"

#include "SDL_version.h"

#include "../SDL_sysvideo.h"
#include "SDL_androidvideo.h"
#include "SDL_androidinput.h"
#include "jniwrapperstuff.h"

#include "touchscreenfont.h"

#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))

static int isTouchscreenKeyboardUsed = 0;

enum {
FONT_LEFT = 0, FONT_RIGHT = 1, FONT_UP = 2, FONT_DOWN = 3,
FONT_BTN1 = 4, FONT_BTN2 = 5, FONT_BTN3 = 6, FONT_BTN4 = 7
};

enum { MAX_BUTTONS = 4 } ; // Max amount of custom buttons

static GLshort fontGL[sizeof(font)/sizeof(font[0])][FONT_MAX_LINES_PER_CHAR * 4 + 1];
enum { FONT_CHAR_LINES_COUNT = FONT_MAX_LINES_PER_CHAR * 4 };

static SDL_Rect arrows, buttons[MAX_BUTTONS];
static int nbuttons;
static SDLKey buttonKeysyms[MAX_BUTTONS] = { 
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_0)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_1)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_2)),
SDL_KEY(SDL_KEY_VAL(SDL_ANDROID_KEYCODE_3))
};

enum { ARROW_LEFT = 1, ARROW_RIGHT = 2, ARROW_UP = 4, ARROW_DOWN = 8 };
static int oldArrows = 0;
static int Button1AutoFire = 0, Button1AutoFireX = 0, Button1AutoFireRot = 0;

static SDL_Rect * OldCoords[MAX_MULTITOUCH_POINTERS] = { NULL };

static const float inv255f = 1.0f / 255.0f;

// Should be called on each char of font before drawing
static void prepareFontChar(int idx, int w, int h)
{
    int i, count = 0;
    float fw = (float) w / 255.0f;
    float fh = (float) h / 255.0f;

	//for( idx = 0; idx < sizeof(font)/sizeof(font[0]); idx++ )
	{
		for( i = 0; i < FONT_MAX_LINES_PER_CHAR; i++ )
			if( font[idx][i].x1 == 0 && font[idx][i].y1 == 0 && 
				font[idx][i].x2 == 0 && font[idx][i].y2 == 0 )
				break;
		count = i;
		for (i = 0; i < count; ++i) 
		{
			fontGL[idx][4*i+0] = (GLshort)(((int)font[idx][i].x1 - 128) * fw);
			fontGL[idx][4*i+1] = (GLshort)(((int)font[idx][i].y1 - 128) * fh);
			fontGL[idx][4*i+2] = (GLshort)(((int)font[idx][i].x2 - 128) * fw);
			fontGL[idx][4*i+3] = (GLshort)(((int)font[idx][i].y2 - 128) * fh);
		}
		fontGL[idx][FONT_CHAR_LINES_COUNT] = count*2;
	}
};


static inline void beginDrawing()
{
    glPushMatrix();
    glLoadIdentity();
    glOrthox( 0, SDL_ANDROID_sWindowWidth * 0x10000, SDL_ANDROID_sWindowHeight * 0x10000, 0, 0, 1 * 0x10000 );
    glPushMatrix();
    glEnableClientState(GL_VERTEX_ARRAY);
}
static inline void endDrawing()
{
    glDisableClientState(GL_VERTEX_ARRAY);
    glPopMatrix();
    glPopMatrix();
}

// Draws a char on screen using embedded line font, (x, y) are center of char, not upper-left corner
// TODO: use SDL 1.3 renderer routines? It will not be pixel-aligned then, if the screen is resized
static inline void drawChar(int idx, Uint16 x, Uint16 y, int rotation, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    //glColor4f((GLfloat) r * inv255f, (GLfloat) g * inv255f, (GLfloat) b * inv255f, (GLfloat) a * inv255f);
    glColor4x(r * 0x10000, g * 0x10000, b * 0x10000, a * 0x10000);

    glVertexPointer(2, GL_SHORT, 0, fontGL[idx]);
    glPopMatrix();
    glPushMatrix();
    glTranslatex( x * 0x10000, y * 0x10000, 0 );
    if(rotation != 0)
        glRotatex( rotation, 0, 0, 0x10000 );
    glDrawArrays(GL_LINES, 0, fontGL[idx][FONT_CHAR_LINES_COUNT]);
}

static inline int InsideRect(const SDL_Rect * r, int x, int y)
{
	return ( x >= r->x && x <= r->x + r->w ) && ( y >= r->y && y <= r->y + r->h );
}

int SDL_ANDROID_drawTouchscreenKeyboard()
{
	int i;
	if( !isTouchscreenKeyboardUsed )
		return 0;
	beginDrawing();
	// Draw arrow keys
	drawChar( FONT_LEFT, arrows.x + arrows.w / 4, arrows.y + arrows.h / 2, 0, 
				255, 255, SDL_GetKeyboardState(NULL)[SDL_KEY(LEFT)] ? 255 : 0, 128 );
	drawChar( FONT_RIGHT, arrows.x + arrows.w / 4 * 3, arrows.y + arrows.h / 2, 0,
				255, 255, SDL_GetKeyboardState(NULL)[SDL_KEY(RIGHT)] ? 255 : 0, 128 );
	drawChar( FONT_UP, arrows.x + arrows.w / 2, arrows.y + arrows.h / 4, 0, 
				255, 255, SDL_GetKeyboardState(NULL)[SDL_KEY(UP)] ? 255 : 0, 128 );
	drawChar( FONT_DOWN, arrows.x + arrows.w / 2, arrows.y + arrows.h / 4 * 3, 0, 
				255, 255, SDL_GetKeyboardState(NULL)[SDL_KEY(DOWN)] ? 255 : 0, 128 );

	// Draw buttons
	for( i = 0; i < nbuttons; i++ )
	{
		drawChar( FONT_BTN1 + i, buttons[i].x + buttons[i].w / 2, buttons[i].y + buttons[i].h / 2, ( i == 0 ? Button1AutoFireRot * 0x10000 : 0 ),
					( i == 0 && Button1AutoFire ) ? 0 : 255, 255, SDL_GetKeyboardState(NULL)[buttonKeysyms[i]] ? 255 : 0, 128 );
	}
	endDrawing();
	return 1;
};

static inline int ArrowKeysPressed(int x, int y)
{
	int ret = 0, dx, dy;
	dx = x - arrows.x - arrows.w / 2;
	dy = y - arrows.y - arrows.h / 2;
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

int SDL_android_processTouchscreenKeyboard(int x, int y, int action, int pointerId)
{
	int i;
	SDL_keysym keysym;

	if( !isTouchscreenKeyboardUsed )
		return 0;

	if( action == MOUSE_DOWN )
	{
		if( InsideRect( &arrows, x, y ) )
		{
			OldCoords[pointerId] = &arrows;
			i = ArrowKeysPressed(x, y);
			if( i & ARROW_UP )
				SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(UP), &keysym) );
			if( i & ARROW_DOWN )
				SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(DOWN), &keysym) );
			if( i & ARROW_LEFT )
				SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(LEFT), &keysym) );
			if( i & ARROW_RIGHT )
				SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(RIGHT), &keysym) );
			oldArrows = i;
			return 1;
		}

		for( i = 0; i < nbuttons; i++ )
		{
			if( InsideRect( &buttons[i], x, y) )
			{
				OldCoords[pointerId] = &buttons[i];
				SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym(buttonKeysyms[i], &keysym) );
				if( i == 0 )
				{
					Button1AutoFireX = x;
					Button1AutoFire = 0;
					Button1AutoFireRot = 0;
				}
				return 1;
			}
		}
	}
	else
	if( action == MOUSE_UP )
	{
		if( OldCoords[pointerId] == &arrows )
		{
			OldCoords[pointerId] = NULL;
			SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(UP), &keysym) );
			SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(DOWN), &keysym) );
			SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(LEFT), &keysym) );
			SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(RIGHT), &keysym) );
			oldArrows = 0;
			return 1;
		}
		for( i = 0; i < nbuttons; i++ )
		{
			if( OldCoords[pointerId] == &buttons[i] )
			{
				if( ! ( i == 0 && Button1AutoFire ) )
					SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym(buttonKeysyms[i] ,&keysym) );
				OldCoords[pointerId] = NULL;
				return 1;
			}
		}
	}
	else
	if( action == MOUSE_MOVE )
	{
		if( OldCoords[pointerId] && !InsideRect(OldCoords[pointerId], x, y) )
		{
			SDL_android_processTouchscreenKeyboard(x, y, MOUSE_UP, pointerId);
			return SDL_android_processTouchscreenKeyboard(x, y, MOUSE_DOWN, pointerId);
		}
		else
		if( OldCoords[pointerId] == &arrows )
		{
			i = ArrowKeysPressed(x, y);
			if( i == oldArrows )
				return 1;
			if( oldArrows & ARROW_UP && ! (i & ARROW_UP) )
				SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(UP), &keysym) );
			if( oldArrows & ARROW_DOWN && ! (i & ARROW_DOWN) )
				SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(DOWN), &keysym) );
			if( oldArrows & ARROW_LEFT && ! (i & ARROW_LEFT) )
				SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(LEFT), &keysym) );
			if( oldArrows & ARROW_RIGHT && ! (i & ARROW_RIGHT) )
				SDL_SendKeyboardKey( SDL_RELEASED, GetKeysym( SDL_KEY(RIGHT), &keysym) );
			if( i & ARROW_UP )
				SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(UP), &keysym) );
			if( i & ARROW_DOWN )
				SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(DOWN), &keysym) );
			if( i & ARROW_LEFT )
				SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(LEFT), &keysym) );
			if( i & ARROW_RIGHT )
				SDL_SendKeyboardKey( SDL_PRESSED, GetKeysym( SDL_KEY(RIGHT), &keysym) );
			oldArrows = i;
		}
		else
		if( OldCoords[pointerId] == &buttons[0] )
		{
			Button1AutoFire = abs(Button1AutoFireX - x) > buttons[0].w / 2;
			if( !Button1AutoFire )
				Button1AutoFireRot = Button1AutoFireX - x;
		}

		if( OldCoords[pointerId] )
			return 1;

		return SDL_android_processTouchscreenKeyboard(x, y, MOUSE_DOWN, pointerId);
	}
	return 0;
};

JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetupScreenKeyboard) ( JNIEnv*  env, jobject thiz, jint size, jint _nbuttons )
{
	int i;
	nbuttons = _nbuttons;
	if( nbuttons > MAX_BUTTONS )
		nbuttons = MAX_BUTTONS;
	// TODO: works for horizontal screen orientation only!
	// TODO: configurable keyboard size

	// Arrows to the lower-left part of screen
	arrows.w = SDL_ANDROID_sWindowWidth / (size + 2);
	arrows.h = arrows.w;
	arrows.x = 0;
	arrows.y = SDL_ANDROID_sWindowHeight - arrows.h;
	
	// Main button to the lower-right
	buttons[0].w = SDL_ANDROID_sWindowWidth / (size + 2);
	buttons[0].h = SDL_ANDROID_sWindowHeight / (size + 2);
	buttons[0].x = SDL_ANDROID_sWindowWidth - buttons[0].w;
	buttons[0].y = SDL_ANDROID_sWindowHeight - buttons[0].h;

	// Row of secondary buttons to the upper-right
	for( i = 1; i < nbuttons; i++ )
	{
		buttons[i].w = SDL_ANDROID_sWindowWidth / (nbuttons - 1) / (size + 2);
		buttons[i].h = SDL_ANDROID_sWindowHeight / (size + 2);
		buttons[i].x = SDL_ANDROID_sWindowWidth - buttons[i].w * (nbuttons - i);
		buttons[i].y = 0;
	}
	
	// Resize char images
	prepareFontChar(FONT_LEFT, arrows.w / 2, arrows.h / 2);
	prepareFontChar(FONT_RIGHT, arrows.w / 2, arrows.h / 2);
	prepareFontChar(FONT_UP, arrows.w / 2, arrows.h / 2);
	prepareFontChar(FONT_DOWN, arrows.w / 2, arrows.h / 2);

	for( i = 0; i < nbuttons; i++ )
	{
		prepareFontChar(FONT_BTN1 + i, MIN(buttons[i].h, buttons[i].w), MIN(buttons[i].h, buttons[i].w));
	}
};


JNIEXPORT void JNICALL 
JAVA_EXPORT_NAME(Settings_nativeSetTouchscreenKeyboardUsed) ( JNIEnv*  env, jobject thiz)
{
	isTouchscreenKeyboardUsed = 1;
}


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

#include "touchscreenfont.h"

enum {
FONT_LEFT = 0, FONT_RIGHT = 1, FONT_UP = 2, FONT_DOWN = 3,
FONT_BTN1 = 4, FONT_BTN2 = 5, FONT_BTN3 = 6, FONT_BTN4 = 7
};

int SDL_android_processTouchscreenKeyboard(int x, int y, int action)
{
};

static const float inv255f = 1.0f / 255.0f;

// TODO: use SDL 1.3 renderer routines? It will not be pixel-aligned then, if the screen is resized
void drawChar(int idx, Uint16 x, Uint16 y, float scale, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    int i;
    GLshort *vertices;
    int count = 0;

	for( i = 0; i < FONT_MAX_LINES_PER_CHAR; i++ )
		if( font[idx][i].x1 == 0 && font[idx][i].y1 == 0 && 
			font[idx][i].x2 == 0 && font[idx][i].y2 == 0 )
			break;
	count = i;
	
	if( count <= 0 )
		return;

	glColor4f((GLfloat) r * inv255f,
                    (GLfloat) g * inv255f,
                    (GLfloat) b * inv255f,
                    (GLfloat) a * inv255f);

    vertices = SDL_stack_alloc(GLshort, count*4);
    for (i = 0; i < count; ++i) {
        vertices[4*i+0] = (GLshort)(x + font[idx][i].x1 * scale);
        vertices[4*i+1] = (GLshort)(y + font[idx][i].y1 * scale);
        vertices[4*i+2] = (GLshort)(x + font[idx][i].x2 * scale);
        vertices[4*i+3] = (GLshort)(y + font[idx][i].y2 * scale);
    }
    glVertexPointer(2, GL_SHORT, 0, vertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_LINE_STRIP, 0, count*2);
    glDisableClientState(GL_VERTEX_ARRAY);
    SDL_stack_free(vertices);
}

int SDL_android_drawTouchscreenKeyboard()
{
};

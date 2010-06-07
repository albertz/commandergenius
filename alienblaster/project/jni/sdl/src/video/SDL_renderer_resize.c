/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2010 Sam Lantinga

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

#if SDL_VIDEO_RENDER_RESIZE

#include "SDL_video.h"
#include "SDL_sysvideo.h"
#include "SDL_pixels_c.h"
#include "SDL_rect_c.h"

#include "SDL_renderer_gl.h"
#include "SDL_renderer_gles.h"
#include "SDL_renderer_sw.h"

#ifdef ANDROID
#include <android/log.h>
#endif

static int RESIZE_RenderDrawPoints(SDL_Renderer * renderer,
                                 const SDL_Point * points, int count);
static int RESIZE_RenderDrawLines(SDL_Renderer * renderer,
                                const SDL_Point * points, int count);
static int RESIZE_RenderDrawRects(SDL_Renderer * renderer,
                                const SDL_Rect ** rects, int count);
static int RESIZE_RenderFillRects(SDL_Renderer * renderer,
                                const SDL_Rect ** rects, int count);
static int RESIZE_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                           const SDL_Rect * srcrect,
                           const SDL_Rect * dstrect);
static void RESIZE_DestroyRenderer(SDL_Renderer * renderer);


typedef struct
{
    /* It should work fast on FPU-less processors, so I did not use floats but ints.
    Hopefully compiler will somehow optimize (realW / fakeW) because it's adjacent bytes */
    uint8_t realW, fakeW, realH, fakeH; 
    SDL_Renderer renderer;

} RESIZE_RenderData;


static int
RESIZE_LoadFunctions(RESIZE_RenderData * data)
{
    return 0;
}

int
RESIZE_CreateRenderer(SDL_Window * window)
{

	SDL_Renderer * renderer;
	RESIZE_RenderData * data;
	int realW, realH;
	int fakeW, fakeH;
	size_t driverDataSize = 0;

	realW = window->display->current_mode.w;
	realH = window->display->current_mode.h;
	fakeW = window->w;
	fakeH = window->h;

	/* Here we're assuming that both real and fake dimensins can be shrinked to byte-size ints
		by dividing both in half, for example real 480x320 : fake 640x480 becomes 3x2 : 4x3 */
	while( (realW / 2) * 2 == realW && (fakeW / 2) * 2 == fakeW ) {
		realW /= 2;
		fakeW /= 2;
	}

	while( (realH / 2) * 2 == realH && (fakeH / 2) * 2 == fakeH ) {
		realH /= 2;
		fakeH /= 2;
	}

	while( (realW / 5) * 5 == realW && (fakeW / 5) * 5 == fakeW ) {
		realW /= 5;
		fakeW /= 5;
	}

	while( (realH / 5) * 5 == realH && (fakeH / 5) * 5 == fakeH ) {
		realH /= 5;
		fakeH /= 5;
	}
	
	if( realW > 255 || realH > 255 || fakeW > 255 || fakeH > 255 ||
		realW == 0 || realH == 0 || fakeW == 0 || fakeH == 0 )
		return -1;

	#ifdef ANDROID
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "Using resize renderer: from %dx%d to %dx%d", 
					window->w, window->h, window->display->current_mode.w, window->display->current_mode.h);
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "Resize renderer ratio: from %dx%d to %dx%d", 
					fakeW, fakeH, realW, realH );
	#endif
	
#if SDL_VIDEO_RENDER_OGL_ES
    if( !strcmp(window->renderer->info.name, "opengl_es") )
        driverDataSize = GLES_RenderDataSize;
#endif
#if SDL_VIDEO_RENDER_OGL
    if( !strcmp(window->renderer->info.name, "opengl") )
        driverDataSize = GL_RenderDataSize;
#endif
    if( !strcmp(window->renderer->info.name, "software") )
        driverDataSize = SW_RenderDataSize;

/* TODO: add other renderers */

    if( !driverDataSize )
        return -1;


    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        return -1;
    }

    data = (RESIZE_RenderData *) SDL_calloc(1, sizeof(RESIZE_RenderData) + driverDataSize);
    if (!data) {
        SDL_free(renderer);
        SDL_OutOfMemory();
        return -1;
    }

    data->realW = realW;
    data->fakeW = fakeW;
    data->realH = realH;
    data->fakeH = fakeH;

    memcpy(&data->renderer, window->renderer, sizeof(SDL_Renderer));
    memcpy(renderer, window->renderer, sizeof(SDL_Renderer));
    /* Copy data from old renderer to renderer->driverdata */
    memcpy(((void *)data) + sizeof(RESIZE_RenderData), window->renderer->driverdata, driverDataSize);

    renderer->RenderDrawPoints = RESIZE_RenderDrawPoints;
    renderer->RenderDrawLines = RESIZE_RenderDrawLines;
    renderer->RenderDrawRects = RESIZE_RenderDrawRects;
    renderer->RenderFillRects = RESIZE_RenderFillRects;
    renderer->RenderCopy = RESIZE_RenderCopy;
    renderer->DestroyRenderer = RESIZE_DestroyRenderer;
    /* Our own data will be located above the renderer->driverdata pointer */
    renderer->driverdata = ((void *)data) + sizeof(RESIZE_RenderData);

    window->renderer = renderer;

    return 0;
}

static inline void
RESIZE_resizePoints(uint8_t realW, uint8_t fakeW, uint8_t realH, uint8_t fakeH,
					const SDL_Point * src, SDL_Point * dest, int count )
{
	int i;
	for( i = 0; i < count; i++ ) {
		dest[i].x = src[i].x * realW / fakeW;
		dest[i].y = src[i].y * realH / fakeH;
	}
}

static int
RESIZE_RenderDrawPoints(SDL_Renderer * renderer, const SDL_Point * points,
                      int count)
{
	RESIZE_RenderData *data = (RESIZE_RenderData *) (renderer->driverdata - sizeof(RESIZE_RenderData));
	int ret;
	
	SDL_Point * resized = SDL_stack_alloc( SDL_Point, count );
	if( ! resized ) {
		SDL_OutOfMemory();
		return -1;
	}
	
	data->renderer.r = renderer->r;
	data->renderer.g = renderer->g;
	data->renderer.b = renderer->b;
	data->renderer.a = renderer->a;
	data->renderer.blendMode = renderer->blendMode;

	RESIZE_resizePoints( data->realW, data->fakeW, data->realH, data->fakeH, points, resized, count );
	
	ret = data->renderer.RenderDrawPoints(&data->renderer, resized, count);
	
	SDL_stack_free(resized);

	return ret;
}

static int
RESIZE_RenderDrawLines(SDL_Renderer * renderer, const SDL_Point * points,
                     int count)
{
	RESIZE_RenderData *data = (RESIZE_RenderData *) (renderer->driverdata - sizeof(RESIZE_RenderData));
	int ret;
	
	SDL_Point * resized = SDL_stack_alloc( SDL_Point, count * 2 );
	if( ! resized ) {
		SDL_OutOfMemory();
		return -1;
	}

	data->renderer.r = renderer->r;
	data->renderer.g = renderer->g;
	data->renderer.b = renderer->b;
	data->renderer.a = renderer->a;
	data->renderer.blendMode = renderer->blendMode;
	
	RESIZE_resizePoints( data->realW, data->fakeW, data->realH, data->fakeH, points, resized, count * 2 );
	
	ret = data->renderer.RenderDrawLines(&data->renderer, resized, count);
	
	SDL_stack_free(resized);

	return ret;
}

static inline void
RESIZE_resizeRects(uint8_t realW, uint8_t fakeW, uint8_t realH, uint8_t fakeH,
					const SDL_Rect ** src, SDL_Rect * dest, int count )
{
	int i;
	for( i = 0; i < count; i++ ) {
		dest[i].x = src[i]->x * realW / fakeW;
		dest[i].w = src[i]->w * realW / fakeW;
		dest[i].y = src[i]->y * realH / fakeH;
		dest[i].h = src[i]->h * realH / fakeH;
	}
}

static int
RESIZE_RenderDrawRects(SDL_Renderer * renderer, const SDL_Rect ** rects,
                     int count)
{
	RESIZE_RenderData *data = (RESIZE_RenderData *) (renderer->driverdata - sizeof(RESIZE_RenderData));
	int i, ret;
	
	SDL_Rect * resized = SDL_stack_alloc( SDL_Rect, count );
	if( ! resized ) {
		SDL_OutOfMemory();
		return -1;
	}
	
	const SDL_Rect ** resizedPtrs = SDL_stack_alloc( const SDL_Rect *, count );
	if( ! resizedPtrs ) {
		SDL_OutOfMemory();
		return -1;
	}
	
	for( i = 0; i < count; i++ ) {
		resizedPtrs[i] = &(resized[i]);
	}

	RESIZE_resizeRects( data->realW, data->fakeW, data->realH, data->fakeH, rects, resized, count );
	
	data->renderer.r = renderer->r;
	data->renderer.g = renderer->g;
	data->renderer.b = renderer->b;
	data->renderer.a = renderer->a;
	data->renderer.blendMode = renderer->blendMode;

	ret = data->renderer.RenderDrawRects(&data->renderer, resizedPtrs, count);
	
	SDL_stack_free(resizedPtrs);
	SDL_stack_free(resized);

	return ret;
}

static int
RESIZE_RenderFillRects(SDL_Renderer * renderer, const SDL_Rect ** rects,
                     int count)
{
	RESIZE_RenderData *data = (RESIZE_RenderData *) (renderer->driverdata - sizeof(RESIZE_RenderData));
	int i, ret;
	
	SDL_Rect * resized = SDL_stack_alloc( SDL_Rect, count );
	if( ! resized ) {
		SDL_OutOfMemory();
		return -1;
	}

	const SDL_Rect ** resizedPtrs = SDL_stack_alloc( const SDL_Rect *, count );
	if( ! resizedPtrs ) {
		SDL_OutOfMemory();
		return -1;
	}
	
	for( i = 0; i < count; i++ ) {
		resizedPtrs[i] = &(resized[i]);
	}
	
	RESIZE_resizeRects( data->realW, data->fakeW, data->realH, data->fakeH, rects, resized, count * 4 );
	
	data->renderer.r = renderer->r;
	data->renderer.g = renderer->g;
	data->renderer.b = renderer->b;
	data->renderer.a = renderer->a;
	data->renderer.blendMode = renderer->blendMode;

	ret = data->renderer.RenderFillRects(&data->renderer, resizedPtrs, count);
	
	SDL_stack_free(resizedPtrs);
	SDL_stack_free(resized);

	return ret;
}

static int
RESIZE_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                const SDL_Rect * srcrect, const SDL_Rect * dstrect)
{
	RESIZE_RenderData *data = (RESIZE_RenderData *) (renderer->driverdata - sizeof(RESIZE_RenderData));
	uint8_t realW = data->realW, fakeW = data->fakeW, realH = data->realH, fakeH = data->fakeH;
	SDL_Rect dest;

	dest.x = (dstrect->x * realW) / fakeW;
	dest.w = (dstrect->w * realW) / fakeW;
	dest.y = (dstrect->y * realH) / fakeH;
	dest.h = (dstrect->h * realH) / fakeH;

	/*
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "RESIZE_RenderCopy: from x%dy%dw%dh%d to x%dy%dw%dh%d rW%d/fW%d rH%d/fH%d", 
					dstrect->x, dstrect->y, dstrect->w, dstrect->h, 
					dest.x, dest.y, dest.w, dest.h,
					realW, fakeW, realH, fakeH);
    */
	/* HACK */
	/* dest.y += 160; */

	return data->renderer.RenderCopy(&data->renderer, texture, srcrect, &dest);
}


static void
RESIZE_DestroyRenderer(SDL_Renderer * renderer)
{

    RESIZE_RenderData *data = (RESIZE_RenderData *) (renderer->driverdata - sizeof(RESIZE_RenderData));

    if (data->renderer.DestroyRenderer) {
        data->renderer.DestroyRenderer(&data->renderer);
    }

    SDL_free((void *)data);

    SDL_free(renderer);
}

#endif /* SDL_VIDEO_RENDER_RESIZE */

/* vi: set ts=4 sw=4 expandtab: */

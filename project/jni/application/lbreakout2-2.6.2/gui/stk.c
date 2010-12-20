/***************************************************************************
                          stk.c  -  description
                             -------------------
    begin                : Thu Oct 12 2002
    copyright            : (C) 2002 by Michael Speck
    email                : kulkanie@gmx.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <png.h>
#include "stk.h"

//#define STK_DEBUG

/*
====================================================================
LOCAL
====================================================================
*/

int stk_quit_request = 0;
SDL_Surface *stk_display = 0;
enum { STK_UPDATE_RECT_LIMIT = 200 };
SDL_Rect stk_update_rects[STK_UPDATE_RECT_LIMIT];
int stk_update_rect_count = 0;
int stk_display_use_fade = 1; /* fading allowed? */
SDL_Cursor *stk_empty_cursor = 0;
SDL_Cursor *stk_standard_cursor = 0;
char *stk_surface_path = 0;
char *stk_audio_path = 0;
SDL_Rect stk_srect, stk_drect;
int stk_old_alpha = 0;
int stk_audio_ok = 0;
#ifdef AUDIO_ENABLED
int stk_audio_mixchannel_count = MIX_CHANNELS;
#else
int stk_audio_mixchannel_count = 0;
#endif
int stk_audio_sound_enabled = 1;
int stk_audio_sound_volume = 127;
int stk_audio_buffer_size = 512;
int stk_screenshot_id = 1;

/* Load a PNG type image from an SDL datasource */
#define IMG_SetError	SDL_SetError
static void png_read_data(png_structp ctx, png_bytep area, png_size_t size)
{
	SDL_RWops *src;

	src = (SDL_RWops *)png_get_io_ptr(ctx);
	SDL_RWread(src, area, size, 1);
}
static SDL_Surface *IMG_LoadPNG_RW(SDL_RWops *src)
{
	SDL_Surface *volatile surface;
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	Uint32 Rmask;
	Uint32 Gmask;
	Uint32 Bmask;
	Uint32 Amask;
	SDL_Palette *palette;
	png_bytep *volatile row_pointers;
	int row, i;
	volatile int ckey = -1;
	png_color_16 *transv;

	/* Initialize the data we will clean up when we're done */
	png_ptr = NULL; info_ptr = NULL; row_pointers = NULL; surface = NULL;

	/* Check to make sure we have something to do */
	if ( ! src ) {
		goto done;
	}

	/* Create the PNG loading context structure */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
					  NULL,NULL,NULL);
	if (png_ptr == NULL){
		IMG_SetError("Couldn't allocate memory for PNG file");
		goto done;
	}

	 /* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		IMG_SetError("Couldn't create image information for PNG file");
		goto done;
	}

	/* Set error handling if you are using setjmp/longjmp method (this is
	 * the normal method of doing things with libpng).  REQUIRED unless you
	 * set up your own error handlers in png_create_read_struct() earlier.
	 */
	if ( setjmp(png_ptr->jmpbuf) ) {
		IMG_SetError("Error reading the PNG file.");
		goto done;
	}

	/* Set up the input control */
	png_set_read_fn(png_ptr, src, png_read_data);

	/* Read PNG header info */
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_type, NULL, NULL);

	/* tell libpng to strip 16 bit/color files down to 8 bits/color */
	png_set_strip_16(png_ptr) ;

	/* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
	 * byte into separate bytes (useful for paletted and grayscale images).
	 */
	png_set_packing(png_ptr);

	/* scale greyscale values to the range 0..255 */
	if(color_type == PNG_COLOR_TYPE_GRAY)
		png_set_expand(png_ptr);

	/* For images with a single "transparent colour", set colour key;
	   if more than one index has transparency, or if partially transparent
	   entries exist, use full alpha channel */
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
	        int num_trans;
		Uint8 *trans;
		png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans,
			     &transv);
		if(color_type == PNG_COLOR_TYPE_PALETTE) {
		    /* Check if all tRNS entries are opaque except one */
		    int i, t = -1;
		    for(i = 0; i < num_trans; i++)
			if(trans[i] == 0) {
			    if(t >= 0)
				break;
			    t = i;
			} else if(trans[i] != 255)
			    break;
		    if(i == num_trans) {
			/* exactly one transparent index */
			ckey = t;
		    } else {
			/* more than one transparent index, or translucency */
			png_set_expand(png_ptr);
		    }
		} else
		    ckey = 0; /* actual value will be set later */
	}

	if ( color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
		png_set_gray_to_rgb(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_type, NULL, NULL);

	/* Allocate the SDL surface to hold the image */
	Rmask = Gmask = Bmask = Amask = 0 ; 
	if ( color_type != PNG_COLOR_TYPE_PALETTE ) {
		if ( SDL_BYTEORDER == SDL_LIL_ENDIAN ) {
			Rmask = 0x000000FF;
			Gmask = 0x0000FF00;
			Bmask = 0x00FF0000;
			Amask = (info_ptr->channels == 4) ? 0xFF000000 : 0;
		} else {
		        int s = (info_ptr->channels == 4) ? 0 : 8;
			Rmask = 0xFF000000 >> s;
			Gmask = 0x00FF0000 >> s;
			Bmask = 0x0000FF00 >> s;
			Amask = 0x000000FF >> s;
		}
	}
	surface = SDL_AllocSurface(SDL_SWSURFACE, width, height,
			bit_depth*info_ptr->channels, Rmask,Gmask,Bmask,Amask);
	if ( surface == NULL ) {
		IMG_SetError("Out of memory");
		goto done;
	}

	if(ckey != -1) {
	        if(color_type != PNG_COLOR_TYPE_PALETTE)
			/* FIXME: Should these be truncated or shifted down? */
		        ckey = SDL_MapRGB(surface->format,
			                 (Uint8)transv->red,
			                 (Uint8)transv->green,
			                 (Uint8)transv->blue);
	        SDL_SetColorKey(surface, SDL_SRCCOLORKEY, ckey);
	}

	/* Create the array of pointers to image data */
	row_pointers = (png_bytep*) malloc(sizeof(png_bytep)*height);
	if ( (row_pointers == NULL) ) {
		IMG_SetError("Out of memory");
		SDL_FreeSurface(surface);
		surface = NULL;
		goto done;
	}
	for (row = 0; row < (int)height; row++) {
		row_pointers[row] = (png_bytep)
				(Uint8 *)surface->pixels + row*surface->pitch;
	}

	/* Read the entire image in one go */
	png_read_image(png_ptr, row_pointers);

	/* read rest of file, get additional chunks in info_ptr - REQUIRED */
	png_read_end(png_ptr, info_ptr);

	/* Load the palette, if any */
	palette = surface->format->palette;
	if ( palette ) {
	    if(color_type == PNG_COLOR_TYPE_GRAY) {
		palette->ncolors = 256;
		for(i = 0; i < 256; i++) {
		    palette->colors[i].r = i;
		    palette->colors[i].g = i;
		    palette->colors[i].b = i;
		}
	    } else if (info_ptr->num_palette > 0 ) {
		palette->ncolors = info_ptr->num_palette; 
		for( i=0; i<info_ptr->num_palette; ++i ) {
		    palette->colors[i].b = info_ptr->palette[i].blue;
		    palette->colors[i].g = info_ptr->palette[i].green;
		    palette->colors[i].r = info_ptr->palette[i].red;
		}
	    }
	}

done:	/* Clean up and return */
	png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : (png_infopp)0,
								(png_infopp)0);
	if ( row_pointers ) {
		free(row_pointers);
	}
	return(surface); 
}

static SDL_Surface *png_load( char *path )
{
    SDL_RWops *src;
    SDL_Surface *surf;
    if ( ( src = SDL_RWFromFile(path, "rb") ) == 0 ) {
        fprintf( stderr, "%s\n", SDL_GetError() );
        return 0;
    }
    else {
        surf = IMG_LoadPNG_RW( src );
        SDL_RWclose( src );
        return surf;
    }
}

/*
====================================================================
Close display and call SDL_Quit.
====================================================================
*/
static void stk_quit( void )
{
    stk_surface_free( &stk_display );
    stk_cursor_free( &stk_empty_cursor );
    if ( stk_surface_path ) {
        free( stk_surface_path );
        stk_surface_path = 0;
    }
    printf( "STK finalized\n" );
    SDL_Quit();
    printf( "SDL finalized\n" );
}

/*
====================================================================
PUBLIC
====================================================================
*/

/* GENERAL */

/*
====================================================================
Initiate SDL, build a default cursor and install the clean
up function stk_quit().
====================================================================
*/
void stk_init( int flags )
{
    /* remove sound flag if disabled */
#ifndef WITH_SOUND
    if ( flags & SDL_INIT_AUDIO )
        flags = flags & ~SDL_INIT_AUDIO;
#endif
    /* init SDL */
    if ( SDL_Init( flags ) < 0 )
        STK_ABORT( SDL_GetError() );
    /* enable unicode */
    SDL_EnableUNICODE( 1 );
    /* set cleanup function */
    atexit( stk_quit );
    /* set current directory as path for surfaces */
    stk_surface_set_path( "." );
    /* set current directory as path for sounds */
    stk_audio_set_path( "." );
    /* create empty cursor */
    stk_empty_cursor = stk_cursor_create( 16, 16, 8, 8,
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                "
                                  "                " );
    stk_standard_cursor = SDL_GetCursor();
}
 
/*
====================================================================
Block until either a key or button was pressed.
====================================================================
*/
void stk_wait_for_input( void )
{
    SDL_Event event;
    while ( 1 ) {
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) {
            stk_quit_request = 1;
            return;
        }
        if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN)
            return;
    }
}
 
/* SCREEN */

/*
====================================================================
Open a display with the passed settings. Depth may be modified due
to Xserver settings and if the resolution is completely 
unavailable 640 x 480 x 16 x SDL_SWSURFACE is opened.
If the display is already open it is closed and re-opened.
There is no function to close the display as this is handled by the
stk_quit() function installed by stk_init().
The display can be accessed by  
  extern SDL_Surface *stk_display;
This funtion returns True if the wanted width and height are 
available, False if not and it aborts when no display is found.
====================================================================
*/
int stk_display_open( int flags, int width, int height, int depth )
{
#ifdef STK_DEBUG
    SDL_PixelFormat	*format;
#endif
    if ( stk_display ) {
        /* if display is already open check if the requested
           resolution is already the current one */
        if ( stk_display->w == width )
            if ( stk_display->h == height )
                if ( stk_display->flags == flags )
                    return 1;
        /* close old display */
        stk_surface_free( &stk_display );
    }
    else { /* load window icon on first time setup */
	SDL_Surface *icon;
	icon = png_load(SRC_DIR "/gfx/win_icon.png");
	SDL_SetColorKey(icon, SDL_SRCCOLORKEY, 0);
	SDL_WM_SetIcon(icon, NULL);
    }
	    
    /* open new display */
    if ( ( depth = SDL_VideoModeOK( width, height, depth, flags ) ) == 0 ) {
        fprintf( stderr, "Requested mode %ix%i %s unavailable\n",
                 width, height, (flags&SDL_FULLSCREEN)?"Fullscreen":"Window" );
        stk_display = SDL_SetVideoMode( 640, 480, 16, SDL_SWSURFACE );
        return 0;
    }
    else
        if ( ( stk_display = SDL_SetVideoMode( width, height, depth, flags ) ) == 0 )
            STK_ABORT( SDL_GetError() );

#ifdef STK_DEBUG				
    printf( "set display %ix%i %s\n",
            width, height, (flags&SDL_FULLSCREEN)?"Fullscreen":"Window" );
        
    format = stk_display->format;
    printf("video mode format:\n");
    printf("Masks: R=%i, G=%i, B=%i\n", 
        format->Rmask, format->Gmask, format->Bmask);
    printf("LShft: R=%i, G=%i, B=%i\n", 
        format->Rshift, format->Gshift, format->Bshift);
    printf("RShft: R=%i, G=%i, B=%i\n", 
        format->Rloss, format->Gloss, format->Bloss);
    printf("BBP: %i\n", format->BitsPerPixel);
    printf("-----\n");
#endif    		
		
    return 1;
}

/*
====================================================================
Add an update rectangle that will be updated by 
stk_display_update(). Regardless of clipping the rectangles
must fit the screen else it'll raise an X error.
If NULL is passed as 'rect' the whole screen is stored for update.
====================================================================
*/
void stk_display_store_rect( SDL_Rect *rect )
{
    if ( stk_update_rect_count < STK_UPDATE_RECT_LIMIT ) {
        if ( rect ) {
            /* for safety this check is kept although it should be 
               unnescessary */
            if ( rect->x < 0 ) {
                rect->w += rect->x;
                rect->x = 0;
            }
            if ( rect->y < 0 ) {
                rect->h += rect->y;
                rect->y = 0;
            }
            if ( rect->x + rect->w > stk_display->w )
                rect->w = stk_display->w - rect->x;
            if ( rect->y + rect->h > stk_display->h )
                rect->h = stk_display->h - rect->y;
            if ( rect->w > 0 && rect->h > 0 )
                stk_update_rects[stk_update_rect_count++] = *rect;
        }
        else
            stk_update_rect_count = STK_UPDATE_RECT_LIMIT;
    }
}

/*
====================================================================
Store the destination rectangle of the last blit operation. SDL
modified this rectangle to the actually updated clipping region.
====================================================================
*/
void stk_display_store_drect( void )
{
    if ( stk_update_rect_count < STK_UPDATE_RECT_LIMIT )
        stk_update_rects[stk_update_rect_count++] = stk_drect;
}

/*
====================================================================
Either update all gathered rects or simply the full screen.
In any case the stored regions are cleared.
====================================================================
*/
void stk_display_update( int type )
{
    if ( type == STK_UPDATE_ALL || 
         stk_update_rect_count == STK_UPDATE_RECT_LIMIT )
        SDL_UpdateRect( stk_display, 0, 0, 0, 0 );
    else
        SDL_UpdateRects( stk_display, stk_update_rect_count,
                         stk_update_rects );
    stk_update_rect_count = 0;
}

/*
====================================================================
Fade the current contents of the display either in or out. 'time'
is the time in milliseconds the fading is supposed to take.
====================================================================
*/
void stk_display_fade( int type, int time )
{
    SDL_Surface *buffer = 0;
    float alpha;
    float alpha_change; /* per ms */
    int leave = 0;
    int ms;

    if ( stk_quit_request ) return;
    
    if ( !stk_display_use_fade ) {
        if ( type == STK_FADE_IN )
            stk_display_update( STK_UPDATE_ALL );
        else {
            stk_surface_fill( stk_display, 0, 0, -1, -1, 0x0 );
            stk_display_update( STK_UPDATE_ALL );
        }
    }

    /* get screen contents */
    buffer = stk_surface_create( SDL_SWSURFACE, stk_display->w, stk_display->h );
    SDL_SetColorKey( buffer, 0, 0 );
    stk_surface_blit( stk_display, 0, 0, -1, -1, buffer, 0, 0 );

    /* compute alpha and alpha change */
    if ( type == STK_FADE_OUT ) {
        alpha = 255;
        alpha_change = -255.0 / time;
    }
    else {
        alpha = 0;
        alpha_change = 255.0 / time;
    }

    /* fade */
    stk_timer_reset();
    while ( !leave ) {
        ms = stk_timer_get_time();
        alpha += alpha_change * ms;
        if ( type == STK_FADE_IN && alpha >= 255 ) break;
        if ( type == STK_FADE_OUT && alpha <= 0 ) break;
        /* update */
        stk_surface_fill( stk_display, 0, 0, -1, -1, 0x0 );
        SDL_SetAlpha( buffer, SDL_SRCALPHA, (int)alpha );
        stk_surface_blit( buffer, 0, 0, -1, -1, stk_display, 0, 0);
        stk_display_update( STK_UPDATE_ALL );
    }

    /* update screen */
    SDL_SetAlpha( buffer, 0, 0 );
    if ( type == STK_FADE_IN )
        stk_surface_blit( buffer, 0, 0, -1, -1, stk_display, 0, 0 );
    else
        stk_surface_fill( stk_display, 0, 0, -1, -1, 0x0 );
    stk_display_update( STK_UPDATE_ALL );
    stk_surface_free( &buffer );
}


/*
====================================================================
Take a screenshot and save it to screenshot[index].bmp in the
current directory.
====================================================================
*/
void stk_display_take_screenshot()
{
    char str[32];
    snprintf( str, 32, "screenshot%i.bmp", stk_screenshot_id++ );
    SDL_SaveBMP( stk_display, str );
}

/*
====================================================================
Switch fullscreen/windowed for current resolution.
====================================================================
*/
void stk_display_apply_fullscreen( int fullscreen )
{
    int flags = stk_display->flags;
    if ( fullscreen && stk_display->flags & SDL_FULLSCREEN )
        return;
    if ( !fullscreen && !(stk_display->flags & SDL_FULLSCREEN ) )
        return;
    if ( fullscreen )
        flags |= SDL_FULLSCREEN;
    else
        flags &= ~SDL_FULLSCREEN;
    stk_display_open( flags, stk_display->w, stk_display->h, 
        stk_display->format->BitsPerPixel );
}

/* SURFACE */

/*
====================================================================
If stk_surface_load() is called with a relative path this prefix
is added. Default is '.'; 'path' is copied.
====================================================================
*/
void stk_surface_set_path( char *path )
{
#ifdef STK_DEBUG
    printf( "surface prefix set to: %s\n", path );
#endif
    if ( stk_surface_path )
        free( stk_surface_path );
    if ( ( stk_surface_path = strdup( path ) ) == 0 )
        STK_ABORT( "Out Of Memory" )
}

static int is_path_relative(char *path)
{
    int is_relative;
#ifdef _WIN32    	
    is_relative = ((*path != '\\') && ((strchr(path,':') == NULL)));
#else			    
    is_relative = (*path != '/');
#endif
    return is_relative;
}

	    
/*
====================================================================
Load a surface from a path. If it's not an absolute directory 
(starting with '/') the prefix passed in stk_surface_set_path() 
is prepended.
If a video mode was set the surface will be converted to its
format to speed up blitting. As resource either BMP or PNG may
be provided.
If SDL_NONFATAL is passed a warning is displayed and the function
returns NULL else it will exit the program with an error.
====================================================================
*/
SDL_Surface* stk_surface_load( int flags, char *format, ... )
{
    SDL_Surface *surface = 0;
    SDL_Surface *converted_surface = 0;
    char path[512], *ptr;
    va_list args;

    /* build full path */
    memset( path, 0, 512 ); ptr = path;
    if ( is_path_relative(format) && stk_surface_path ) {
        sprintf( path, "%s/", stk_surface_path );
        ptr = path + strlen( path );
    }
    va_start( args, format );
    vsnprintf( ptr, 511 - (path-ptr), format, args );
    va_end( args );
#ifdef STK_DEBUG
    printf( "loading %s\n", path );
#endif
    
    /* load surface */
    ptr = path + strlen( path ) - 3;
    if ( !strncmp( ptr, "png", 3 ) )
        /* as png */
        surface = png_load( path );
    else
        /* as bmp */
        surface = SDL_LoadBMP( path );
    if ( surface == 0 ) {
        if ( flags & SDL_NONFATAL ) {
            fprintf( stderr, "%s\n", SDL_GetError() );
            return 0;
        }
        else
            STK_ABORT( SDL_GetError() )
    }
    
    /* convert if display open */
    if ( stk_display ) {
        converted_surface = SDL_DisplayFormat( surface );
        if ( converted_surface == 0 ) {
            fprintf( stderr, "Conversion of %s failed: %s\n", 
                     path, SDL_GetError() );
            return surface;
        }
        else {
            SDL_FreeSurface( surface );
            SDL_SetColorKey( 
                converted_surface, SDL_SRCCOLORKEY, 0x0 );
            SDL_SetAlpha( converted_surface, 0, 0 );
            return converted_surface;
        }
    }
    else {
#ifdef STK_DEBUG
            printf( "  not converted\n" );
#endif
        return surface;
    }
}

/*
====================================================================
Create a surface with the given size and the format of the 
video mode which must be set.
The default color key of the surface is black and no alpha.
====================================================================
*/
SDL_Surface* stk_surface_create( int flags, int width, int height )
{
    SDL_Surface *surface = 0;
    if ( !stk_display ) {
        if ( flags & SDL_NONFATAL ) {
            fprintf( stderr, 
                "Can't create surface as no display is open\n" );
            return 0;
        }
        else
            STK_ABORT( 
                "Can't create surface as no display is open\n" );
    }
    surface = SDL_CreateRGBSurface( flags, width, height,
                  stk_display->format->BitsPerPixel,
                  stk_display->format->Rmask, 
                  stk_display->format->Gmask,
                  stk_display->format->Bmask,
                  stk_display->format->Amask );
    if ( surface == 0 ) {
        if ( flags & SDL_NONFATAL ) {
            fprintf( stderr, "%s\n", SDL_GetError() );
            return 0;
        }
        else
            STK_ABORT( SDL_GetError() );
    }
    SDL_SetColorKey( surface, SDL_SRCCOLORKEY, 0x0 );
    SDL_SetAlpha( surface, 0, 0 );
#ifdef STK_DEBUG
    printf( "surface %ix%i created\n", width, height );
#endif
    return surface;
}

/*
====================================================================
Free the memory of a surface if not NULL and reset the pointer
to NULL.
====================================================================
*/
void stk_surface_free( SDL_Surface **surface )
{
    if ( *surface ) {
        SDL_FreeSurface( *surface );
        *surface = 0;
    }
}

/*
====================================================================
Blit retangle from 'src' to 'dest' with current alpha of 'src'.
If 'sw' is -1 the full source width is used, 'sh' analogue.
stk_display_store_drect() can be used to store the update rect
of the blitted surface.
====================================================================
*/
void stk_surface_blit( 
    SDL_Surface *src, int sx, int sy, int sw, int sh,
    SDL_Surface *dest, int dx, int dy )
{    
    /* build rectangles */
    stk_drect.x = dx; stk_drect.y = dy; 
    stk_drect.w = (sw==-1)?src->w:sw; 
    stk_drect.h = (sh==-1)?src->h:sh;
    stk_srect.x = sx; stk_srect.y = sy; 
    stk_srect.w = stk_drect.w; 
    stk_srect.h = stk_drect.h;
    /* blit */
    SDL_BlitSurface( src, &stk_srect, dest, &stk_drect );
}

/*
====================================================================
Different from stk_surface_blit() this function temporarily
overwrites 'src's alpha value. It is not recommended to use
this function if performance is important as it is slower
than an SDL_SetAlpha() combined with stk_surface_blit().
====================================================================
*/
void stk_surface_alpha_blit( 
    SDL_Surface *src, int sx, int sy, int sw, int sh,
    SDL_Surface *dest, int dx, int dy, int alpha )
{
    if ( src->flags & SDL_SRCALPHA )
        stk_old_alpha = src->format->alpha;
    else
        stk_old_alpha = -1;
    SDL_SetAlpha( src, SDL_SRCALPHA, alpha );
    stk_surface_blit( src, sx,sy,sw,sh, dest, dx,dy );
    if ( stk_old_alpha == -1 )
        SDL_SetAlpha( src, 0, 0 );
    else
        SDL_SetAlpha( src, SDL_SRCALPHA, 
                      stk_old_alpha );
}

/*
====================================================================
Fill a rectangle of the surface with a given color of the format
0xRRGGBB. 'dw' == -1 and 'dh' == -1 have the same effect as in
stk_surface_blit().
====================================================================
*/
void stk_surface_fill( 
    SDL_Surface *dest, int dx, int dy, int dw, int dh,
    int color )
{
    /* build rectangle */
    stk_drect.x = dx; stk_drect.y = dy; 
    stk_drect.w = (dw==-1)?dest->w:dw; 
    stk_drect.h = (dh==-1)?dest->h:dh;
    /* fill surface */
    SDL_FillRect( dest, &stk_drect, 
        SDL_MapRGB( stk_display->format, 
                    color >> 16, 
                    (color >> 8) & 0xFF, 
                    color & 0xFF) );
}

/*
====================================================================
Set the clipping region of a surface. All blit operations into
this surface will only have effect within the clipping region.
'dw' == -1 and 'dh' == -1 have the same effect as in
stk_surface_blit().
====================================================================
*/
void stk_surface_clip( 
    SDL_Surface *dest, int dx, int dy, int dw, int dh )
{
    /* build rectangle */
    stk_drect.x = dx; stk_drect.y = dy; 
    stk_drect.w = (dw<=0)?dest->w:dw; 
    stk_drect.h = (dh<=0)?dest->h:dh;
    /* clip */
    SDL_SetClipRect( dest, &stk_drect );
}

/*
====================================================================
Lock/unlock surface for direct access.
====================================================================
*/
#define stk_surface_lock( surface ) \
 if (SDL_MUSTLOCK((surface))) SDL_LockSurface((surface))
#define stk_surface_unlock( surface ) \
 if (SDL_MUSTLOCK((surface))) SDL_UnlockSurface((surface))

/*
====================================================================
Get or set a pixel from/to a surface. This time the pixel must
already be in SDL format.
====================================================================
*/
void stk_surface_set_pixel( 
    SDL_Surface *dest, int dx, int dy, Uint32 pixel )
{
    memcpy( dest->pixels + dy * dest->pitch + 
            dx * dest->format->BytesPerPixel,
            &pixel, dest->format->BytesPerPixel );
}
Uint32 stk_surface_get_pixel( SDL_Surface *src, int sx, int sy )
{
    Uint32 pixel = 0;
    memcpy( &pixel, 
            src->pixels + sy * src->pitch + 
            sx * src->format->BytesPerPixel, 
            src->format->BytesPerPixel );
    return pixel;
}
 
/*
====================================================================
Convert part of the surface to gray and if 'dark' is set cut the 
brightness in half. 'dw' == -1 and 'dh' == -1 have the same 
effect as in tk_surface_blit().
====================================================================
*/
void stk_surface_gray( 
    SDL_Surface *dest, int dx, int dy, int dw, int dh, int dark )
{
    int i, j, y_offset, x_offset;
    Uint32 pixel, temp;
    Uint8 red, green, blue, gray;
    /* adjust size */
    if ( dw == -1 ) dw = dest->w;
    if ( dh == -1 ) dh = dest->h;
    /* gray */
    y_offset = dest->pitch * dy;
    for ( j = dy; j < dy + dh; j++ ) {
        for ( i = dx, x_offset = dx * dest->format->BytesPerPixel; 
              i < dx + dw; 
              i++, x_offset += dest->format->BytesPerPixel ) {
            pixel = 0;
            /* get pixel */
            memcpy( &pixel,
                    dest->pixels +
                    y_offset + x_offset,
                    dest->format->BytesPerPixel );
            /* extract colors */
            temp=pixel & dest->format->Rmask;
            temp=temp>>dest->format->Rshift;
            temp=temp<<dest->format->Rloss;
            red=(Uint8)temp;
            temp=pixel & dest->format->Gmask;
            temp=temp>>dest->format->Gshift;
            temp=temp<<dest->format->Gloss;
            green=(Uint8)temp;
            temp=pixel & dest->format->Bmask;
            temp=temp>>dest->format->Bshift;
            temp=temp<<dest->format->Bloss;
            blue=(Uint8)temp;
            /* build new color */
            gray = (red + green + green + blue) >> (2 + dark);
            pixel = SDL_MapRGB( dest->format, gray, gray, gray );
            /* replace */
            memcpy( dest->pixels +
                    y_offset + x_offset,
                    &pixel, 
                    dest->format->BytesPerPixel );
        }
        y_offset += dest->pitch;
    }
}
 
/*
====================================================================
Add a 3D frame to surface and dark to contents. The frame border
is 'border' thick.
====================================================================
*/
void stk_surface_add_3dframe(
    SDL_Surface *dest, int dx, int dy, int dw, int dh, int border )
{
    printf( "not implemented yet!\n" );
}

/*
====================================================================
Fill the surface with the wallpaper (clipped)
====================================================================
*/
void stk_surface_apply_wallpaper( 
    SDL_Surface *dest, int dx, int dy, int dw, int dh, 
    SDL_Surface *wallpaper, int alpha )
{
    int i, j;
    /* adjust dw and dh */
    if ( dw == -1 ) dw = dest->w;
    if ( dh == -1 ) dh = dest->h;
    stk_surface_clip( dest, dx, dy, dw, dh );
    /* apply */
    for ( i = 0; i < dw; i += wallpaper->w )
        for ( j = 0; j < dh; j += wallpaper->h )
            stk_surface_alpha_blit( wallpaper, 0, 0, -1, -1,
                              dest, dx + i, dy + j, alpha );
    stk_surface_clip( dest, 0,0,-1,-1 );
}

/*
====================================================================
Apply a frame to the surface. The frame resource provides the eight
square-like parts for the frame composed as a little frame: corners
and sides.
'dw' == -1 and 'dh' == -1 have the same effect as in
stk_surface_blit().
Returns the size of the border.
====================================================================
*/
int stk_surface_apply_frame(
    SDL_Surface *dest, int dx, int dy, int dw, int dh, 
    SDL_Surface *frame )
{
    int tile_size = frame->w / 3;
    int i;
    /* adjust dw and dh */
    if ( dw == -1 ) dw = dest->w;
    if ( dh == -1 ) dh = dest->h;
    /* horizontal middle parts */
    for ( i = dx; i < dx + dw - tile_size; i += tile_size ) {
        stk_surface_blit( frame, tile_size, 0, 
                          tile_size, tile_size,
                          dest, i, dy );
        stk_surface_blit( frame, tile_size, tile_size << 1, 
                          tile_size, tile_size,
                          dest, i, dy + dh - tile_size );
    }
    /* vertical middle parts */
    for ( i = dy; i < dy + dh - tile_size; i += tile_size ) {
        stk_surface_blit( frame, 0, tile_size,
                          tile_size, tile_size,
                          dest, dx, i );
        stk_surface_blit( frame, tile_size << 1, tile_size, 
                          tile_size, tile_size,
                          dest, dx + dw - tile_size, i );
    }
    /* corners */
    stk_surface_blit( frame, 0, 0, tile_size, tile_size,
                      dest, dx, dy );
    stk_surface_blit( frame, tile_size << 1, 0, 
                      tile_size, tile_size,
                      dest, dx + dw - tile_size, dy );
    stk_surface_blit( frame, 0, tile_size << 1, 
                      tile_size, tile_size,
                      dest, dx, dy + dh - tile_size );
    stk_surface_blit( frame, tile_size << 1, tile_size << 1, 
                      tile_size, tile_size,
                      dest, dx + dw - tile_size, 
                      dy + dh - tile_size );
    return tile_size;
}

/* FIXED FONT */

/*
====================================================================
Load a fixed font which is simply a surface containing the ASCII
characters from 32 (blank) to 96 (whatever) where all characters
have the same width.
====================================================================
*/
StkFont* stk_font_load( int flags, char *format, ... )
{
    char path[512];
    va_list args;
    StkFont *font = calloc( 1, sizeof ( StkFont ) );
    if ( font == 0 )
        STK_ABORT( "Out Of Memory" );
    /* build path */
    memset( path, 0, 512 );
    va_start( args, format );
    vsnprintf( path, 511, format, args );
    va_end( args );
    /* load surface */
    font->surface = stk_surface_load( flags, path );
    if ( font == 0 ) {
        if ( flags & SDL_NONFATAL ) {
            fprintf( stderr, "%s\n", SDL_GetError() );
            return 0;
        }
        else
            STK_ABORT( SDL_GetError() );
    }
    /* do the rest */
    font->height = font->surface->h;
    font->width = font->surface->w / 96;
    return font;
}

/*
====================================================================
Free a font if not NULL and reset pointer to NULL.
====================================================================
*/
void stk_font_free( StkFont **font )
{
    if ( *font ) {
        stk_surface_free( &(*font)->surface );
        free(*font); *font = 0;
    }
}

/*
====================================================================
Write string to surface. If 'alpha' is -1 the font is displayed 
with the current alpha value else the new alpha is used and kept.
stk_display_store_drect() can be used to store the update rect
of the written string.
====================================================================
*/
void stk_font_write( 
    StkFont *font, SDL_Surface *dest, int dx, int dy, 
    int alpha, char *string )
{
    int	x, i, width = 0;
	
    /* get pixel width of string */
    width = stk_font_string_width( font, string );

    /* ajust dx,dy to alignment */
    if ( font->align & STK_FONT_ALIGN_CENTER_X )
        dx -= width >> 1;
    else
        if ( font->align & STK_FONT_ALIGN_RIGHT )
            dx -= width;
    if ( font->align & STK_FONT_ALIGN_CENTER_Y )
        dy -= (font->height >> 1 ) + 1;
    else
        if ( font->align & STK_FONT_ALIGN_BOTTOM )
            dy -= font->height;

    /* draw characters */
    if ( alpha != -1 )
        SDL_SetAlpha( font->surface, SDL_SRCALPHA, alpha );
    for ( i = 0, x = dx; i < strlen( string ); 
          i++, x += font->width ) {
        stk_surface_blit( 
            font->surface, font->width * (string[i] - 32), 0,
            font->width, font->height,
            dest, x, dy );
    }
    
    /* fake destination rectangle to full string */
    stk_drect.x = dx; stk_drect.y = dy; 
    stk_drect.w = width; stk_drect.h = font->height; 
    if (stk_drect.x < 0) {
        stk_drect.w += stk_drect.x;
        stk_drect.x = 0;
    }
    if (stk_drect.y < 0) {
        stk_drect.h += stk_drect.y;
        stk_drect.y = 0;
    }
    if (stk_drect.x + stk_drect.w >= stk_display->w) 
        stk_drect.w = stk_display->w - stk_drect.x;
    if (stk_drect.y + stk_drect.h >= stk_display->h) 
        stk_drect.h = stk_display->h - stk_drect.y;
}

/* CURSOR */

/*
====================================================================
Build an SDL cursor with the given hotspot. The resource mask
contains 'b' for black 'w' for white and ' ' for transparent.
====================================================================
*/
SDL_Cursor* stk_cursor_create(
    int width, int height, int hot_x, int hot_y, char *source )
{
    char *mask = 0, *data = 0;
    SDL_Cursor *cursor = 0;
    int i, j, k;
    char data_byte, mask_byte;
    int pot;
    /* create mask&data */
    mask = calloc( width * height / 8, sizeof ( char ) );
    data = calloc( width * height / 8, sizeof ( char ) );
    /* convert */
    k = 0;
    for (j = 0; j < width * height; j += 8, k++) {
        pot = 1;
        data_byte = mask_byte = 0;
        /* create byte */
        for (i = 7; i >= 0; i--) {
            switch ( source[j + i] ) {
                case 'b':
                    data_byte += pot;
                case 'w':
                    mask_byte += pot;
                    break;
            }
            pot *= 2;
        }
        /* add to mask */
        data[k] = data_byte;
        mask[k] = mask_byte;
    }
    /* create and return cursor */
    cursor = SDL_CreateCursor( data, mask, width, height, hot_x, hot_y );
    free( mask );
    free( data );
    return cursor;
}

/*
====================================================================
Free a cursor if not NULL and reset pointer to NULL.
====================================================================
*/
void stk_cursor_free( SDL_Cursor **cursor )
{
    if ( *cursor ) {
        SDL_FreeCursor( *cursor );
        *cursor = 0;
    }
}

/* TIMER */

int stk_timer_last_call = 0;

/*
====================================================================
Reset the timer.
====================================================================
*/
void stk_timer_reset( void )
{
    stk_timer_last_call = SDL_GetTicks();
}

/*
====================================================================
Get the time since last call or reset.
====================================================================
*/
int stk_timer_get_time( void )
{
    int ms, current = SDL_GetTicks();
    if ( current == stk_timer_last_call )
        SDL_Delay(1);
    current = SDL_GetTicks();
    ms = current - stk_timer_last_call;
    stk_timer_last_call = current;
    return ms;
}

/* AUDIO */

/*
====================================================================
Open mixer. If this fails an error message is
displayed (program doesn't abort) and all stk_audio and stk_sound
functions have no effect.
  'frequency': 11025, 22050, 44100 ...
  'format': MIX_DEFAULT_FORMAT is recommended
  'channels': mono(1) or stereo(2)
  'chunksize': mix buffer size (1024 Bytes recommended)
Passing 0 for an argument means to use the SDL default:
  22050Hz in format AUDIO_S16LSB stereo with 1024 bytes chunksize
stk_audio_open() installs stk_audio_close() as exit function.
There are 8 channels available for mixing.
====================================================================
*/
int stk_audio_open( 
    int frequency, Uint16 format, int channels, int chunksize )
{
#ifdef AUDIO_ENABLED
    if ( frequency == 0 ) frequency = MIX_DEFAULT_FREQUENCY;
    if ( format == 0 ) format = MIX_DEFAULT_FORMAT;
    if ( channels == 0 ) channels = 2;
    if ( chunksize == 0 ) chunksize = stk_audio_buffer_size;
    if ( Mix_OpenAudio( 
             frequency, format, channels, chunksize ) < 0 ) {
        fprintf( stderr, "%s\n", SDL_GetError() );
        stk_audio_ok = 0;
        return 0;
    }
    stk_audio_ok = 1;
    atexit( stk_audio_close );
    return 1;
#else
    stk_audio_ok = 0;
    return 0;
#endif
}

/*
====================================================================
Close mixer if previously opened.
====================================================================
*/
void stk_audio_close( void )
{
#ifdef AUDIO_ENABLED
    if ( stk_audio_ok ) {
        Mix_CloseAudio();
        stk_audio_ok = 0;
    }
#endif
    printf( "Audio finalized\n" );
}

/*
====================================================================
If stk_sound_load() is called with a relative path this prefix
is added. Default is '.'; 'path' is copied.
====================================================================
*/
void stk_audio_set_path( char *path )
{
#ifdef STK_DEBUG
    printf( "sound prefix set to: %s\n", path );
#endif
    if ( stk_audio_path )
        free( stk_audio_path );
    if ( ( stk_audio_path = strdup( path ) ) == 0 )
        STK_ABORT( "Out Of Memory" )
}

/*
====================================================================
Enable/disable sound. If disabled stk_sound_play () has no effect.
====================================================================
*/
void stk_audio_enable_sound( int enable )
{
    stk_audio_sound_enabled = enable;
}

/*
====================================================================
Set default volume of all sounds: 0 - 128
====================================================================
*/
void stk_audio_set_sound_volume( int volume )
{
    if ( !stk_audio_ok ) return;
    if ( volume < 0 ) volume = 0;
    if ( volume > 127 ) volume = 127;
    stk_audio_sound_volume = volume;
#ifdef AUDIO_ENABLED
    Mix_Volume( -1, volume ); /* all sound channels */
#endif
}

/*
====================================================================
Fade out a specific sound channel. If 'channel is -1 all
channels fade out. 'ms' is the time the fading shall take.
====================================================================
*/
void stk_audio_fade_out( int channel, int ms )
{
#ifdef AUDIO_ENABLED
    if ( stk_audio_ok )
        Mix_FadeOutChannel( channel, ms );
#endif
}

/* SOUND */

/*
====================================================================
Load a sound from a path. If it's not an absolute directory 
(starting with '/') the prefix passed in stk_sound_set_path() 
is prepended. Loading sounds is non-fatal thus if a sound
cannot be found it is created but empty. In this case
a warning is displayed.
The volume is set to the default set by
stk_audio_set_sound_volume(). Sounds with an equal 'channel' 
will share it so if a new sound is played the old one breaks up
thus channel should be used to group sounds.
If channel is -1 the first available channel is used
to play sound.
====================================================================
*/
StkSound *stk_sound_load( int channel, char *format, ... )
{
    StkSound *sound;
#ifdef AUDIO_ENABLED
    char path[512], *ptr;
    va_list args;

    /* build full path */
    memset( path, 0, 512 ); ptr = path;
    if ( is_path_relative(format)  && stk_audio_path ) {
        sprintf( path, "%s/", stk_audio_path );
        ptr = path + strlen( path );
    }
    va_start( args, format );
    vsnprintf( ptr, 511 - (path-ptr), format, args );
    va_end( args );
#ifdef STK_DEBUG
    printf( "loading %s\n", path );
#endif
    
    /* load sound */
    if ( ( sound = calloc( 1, sizeof( StkSound ) ) ) == 0 )
        STK_ABORT( "Out Of Memory" );
    if ( ( sound->chunk = Mix_LoadWAV( path ) ) == 0 )
        fprintf( stderr, "Couldn't open %s (%s)\n", 
            path, SDL_GetError() );
#else
    if ( ( sound = calloc( 1, sizeof( StkSound ) ) ) == 0 )
        STK_ABORT( "Out Of Memory" );
#endif
    sound->volume = stk_audio_sound_volume;
    sound->channel = channel;
    
    /* done */
    return sound;
}

/*
====================================================================
Free the memory of a sound if not NULL and reset the pointer
to NULL.
====================================================================
*/
void stk_sound_free( StkSound **sound )
{
    if ( *sound ) {
#ifdef AUDIO_ENABLED
        if ( (*sound)->chunk )
            Mix_FreeChunk( (*sound)->chunk );
#endif
        free( *sound );
    }
}

/*
====================================================================
Set channel/volume of sound.
====================================================================
*/
void stk_sound_set_volume( StkSound *sound, int volume )
{
    if ( volume < 0 ) volume = 0;
    if ( volume > 128 ) volume = 128;
    sound->volume = volume;
}
void stk_sound_set_channel( StkSound *sound, int channel )
{
    if ( channel < 0 ) channel = 0;
    if ( channel > stk_audio_mixchannel_count )
        channel = stk_audio_mixchannel_count;
    sound->channel = channel;
}

/*
====================================================================
Play a sound.
====================================================================
*/
void stk_sound_play( StkSound *sound )
{
    int channel;
#ifdef AUDIO_ENABLED
    if ( stk_audio_ok && stk_audio_sound_enabled ) {
        /* if channel is -1 use first free channel */
        channel = sound->channel;
        if ( channel == -1 )
            channel = Mix_GroupAvailable(-1);
        Mix_Volume( channel, stk_audio_sound_volume );
        Mix_PlayChannel( channel, sound->chunk, 0 );
    }
#endif
}
/*
====================================================================
Play a sound at horizontal position x.
====================================================================
*/
void stk_sound_play_x( int x, StkSound *sound )
{
    int channel;
#ifdef AUDIO_ENABLED
    if ( stk_audio_ok && stk_audio_sound_enabled ) {
        x = (x - 40) * 255 / (640 - 40 - 40);
        if (x < 0) x = 0;
        if (x > 255) x = 255;
        /* if channel is -1 use first free channel */
        channel = sound->channel;
        if ( channel == -1 )
            channel = Mix_GroupAvailable(-1);
        Mix_SetPanning( channel, 255 - x, x );
        Mix_Volume( channel, stk_audio_sound_volume );
        Mix_PlayChannel( channel, sound->chunk, 0 );
    }
#endif
}

/*
====================================================================
Fade out the channel of this sound.
====================================================================
*/
void stk_sound_fade_out( StkSound *sound, int ms )
{
#ifdef AUDIO_ENABLED
    if ( stk_audio_ok )
        Mix_FadeOutChannel( sound->channel, ms );
#endif
}

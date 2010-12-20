/***************************************************************************
                          sdl.c  -  description
                             -------------------
    begin                : Thu Apr 20 2000
    copyright            : (C) 2000 by Michael Speck
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

#include <SDL.h>
#include <stdlib.h>
#include <string.h>
#include "sdl.h"

#ifdef USE_PNG
#include <png.h>
#endif

extern int  term_game;

Sdl sdl;
SDL_Cursor *empty_cursor = 0;
SDL_Cursor *std_cursor = 0;

/*
====================================================================
Default video modes. The first value is the id and indicates
if a mode is a standard video mode. If the mode was created by
directly by video_mode() this id is set to -1. The very last
value indicates if this is a valid mode and is checked by
init_sdl(). Init_sdl sets the available desktop bit depth.
====================================================================
*/
int const mode_count = 2;
Video_Mode modes[] = {
    { 0, "640x480x16 Window",     640, 480, BITDEPTH, SDL_SWSURFACE, 0 },
    { 1, "640x480x16 Fullscreen", 640, 480, BITDEPTH, SDL_SWSURFACE | SDL_FULLSCREEN, 0 },
};
Video_Mode *def_mode = &modes[0]; /* default resolution */
Video_Mode cur_mode; /* current video mode set in set_video_mode */

/* timer */
int cur_time, last_time;

/* sdl surface */

#ifdef USE_PNG
/* loads an image from png file and returns surface
 * or NULL in case of error;
 * you can get additional information with SDL_GetError
 *
 * stolen from SDL_image:
 *
 * Copyright (C) 1999  Sam Lantinga
 *
 * Sam Lantinga
 * 5635-34 Springhouse Dr.
 * Pleasanton, CA 94588 (USA)
 * slouken@devolution.com
 */
SDL_Surface *load_png( const char *file )
{
	FILE		*volatile fp = NULL;
	SDL_Surface	*volatile surface = NULL;
	png_structp	png_ptr = NULL;
	png_infop	info_ptr = NULL;
	png_bytep	*volatile row_pointers = NULL;
	png_uint_32	width, height;
	int		bit_depth, color_type, interlace_type;
	Uint32		Rmask;
	Uint32		Gmask;
	Uint32		Bmask;
	Uint32		Amask;
	SDL_Palette	*palette;
	int		row, i;
	volatile int	ckey = -1;
	png_color_16	*transv;

	/* create the PNG loading context structure */
	png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING,
					  NULL, NULL, NULL );
	if( png_ptr == NULL ) {
		SDL_SetError( "Couldn't allocate memory for PNG file" );
		goto done;
	}

	/* allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct( png_ptr );
	if( info_ptr == NULL ) {
		SDL_SetError( "Couldn't create image information for PNG file" );
		goto done;
	}

	/* set error handling if you are using setjmp/longjmp method (this is
	 * the normal method of doing things with libpng).  REQUIRED unless you
	 * set up your own error handlers in png_create_read_struct() earlier.
	 */
	if( setjmp( png_ptr->jmpbuf ) ) {
		SDL_SetError( "Error reading the PNG file." );
		goto done;
	}

	/* open file */
	fp = fopen( file, "r" );
	if( fp == NULL ) {
		SDL_SetError( "Could not open png file." );
		goto done;
	}
	png_init_io( png_ptr, fp );

	/* read PNG header info */
	png_read_info( png_ptr, info_ptr );
	png_get_IHDR( png_ptr, info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_type, NULL, NULL );

	/* tell libpng to strip 16 bit/color files down to 8 bits/color */
	png_set_strip_16( png_ptr );

	/* extract multiple pixels with bit depths of 1, 2, and 4 from a single
	 * byte into separate bytes (useful for paletted and grayscale images).
	 */
	png_set_packing( png_ptr );

	/* scale greyscale values to the range 0..255 */
	if( color_type == PNG_COLOR_TYPE_GRAY )
		png_set_expand( png_ptr );

	/* for images with a single "transparent colour", set colour key;
	   if more than one index has transparency, use full alpha channel */
	if( png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ) ) {
	        int num_trans;
		Uint8 *trans;
		png_get_tRNS( png_ptr, info_ptr, &trans, &num_trans,
			      &transv );
		if( color_type == PNG_COLOR_TYPE_PALETTE ) {
			if( num_trans == 1 ) {
				/* exactly one transparent value: set colour key */
				ckey = trans[0];
			} else
				png_set_expand( png_ptr );
		} else
		    ckey = 0; /* actual value will be set later */
	}

	if( color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
		png_set_gray_to_rgb( png_ptr );

	png_read_update_info( png_ptr, info_ptr );

	png_get_IHDR( png_ptr, info_ptr, &width, &height, &bit_depth,
			&color_type, &interlace_type, NULL, NULL );

	/* allocate the SDL surface to hold the image */
	Rmask = Gmask = Bmask = Amask = 0 ;
	if( color_type != PNG_COLOR_TYPE_PALETTE ) {
		if( SDL_BYTEORDER == SDL_LIL_ENDIAN ) {
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
	surface = SDL_AllocSurface( SDL_SWSURFACE, width, height,
			bit_depth * info_ptr->channels, Rmask, Gmask, Bmask, Amask );
	if( surface == NULL ) {
		SDL_SetError( "Out of memory" );
		goto done;
	}

	if( ckey != -1 ) {
		if( color_type != PNG_COLOR_TYPE_PALETTE )
			/* FIXME: should these be truncated or shifted down? */
			ckey = SDL_MapRGB( surface->format,
					   (Uint8)transv->red,
					   (Uint8)transv->green,
					   (Uint8)transv->blue );
		SDL_SetColorKey( surface, SDL_SRCCOLORKEY, ckey );
	}

	/* create the array of pointers to image data */
	row_pointers = (png_bytep*)malloc( sizeof( png_bytep ) * height );
	if( ( row_pointers == NULL ) ) {
		SDL_SetError( "Out of memory" );
		SDL_FreeSurface( surface );
		surface = NULL;
		goto done;
	}
	for( row = 0; row < (int)height; row++ ) {
		row_pointers[row] = (png_bytep)
				(Uint8*)surface->pixels + row * surface->pitch;
	}

	/* read the entire image in one go */
	png_read_image( png_ptr, row_pointers );

	/* read rest of file, get additional chunks in info_ptr - REQUIRED */
	png_read_end( png_ptr, info_ptr );

	/* load the palette, if any */
	palette = surface->format->palette;
	if( palette ) {
	    if( color_type == PNG_COLOR_TYPE_GRAY ) {
		palette->ncolors = 256;
		for( i = 0; i < 256; i++ ) {
		    palette->colors[i].r = i;
		    palette->colors[i].g = i;
		    palette->colors[i].b = i;
		}
	    } else if( info_ptr->num_palette > 0 ) {
		palette->ncolors = info_ptr->num_palette;
		for( i = 0; i < info_ptr->num_palette; ++i ) {
		    palette->colors[i].b = info_ptr->palette[i].blue;
		    palette->colors[i].g = info_ptr->palette[i].green;
		    palette->colors[i].r = info_ptr->palette[i].red;
		}
	    }
	}

done:
	/* clean up and return */
	png_destroy_read_struct( &png_ptr, info_ptr ? &info_ptr : (png_infopp)0,
								  (png_infopp)0 );
	if( row_pointers )
		free( row_pointers );
	if( fp )
		fclose( fp );

	return surface;
}
#endif

/* return full path of bitmap */
inline void get_full_bmp_path( char *full_path, char *file_name )
{
    sprintf(full_path,  "%s/gfx/%s", SRC_DIR, file_name );
}

/*
    load a surface from file putting it in soft or hardware mem
*/
SDL_Surface* load_surf(char *fname, int f)
{
    SDL_Surface *buf;
    SDL_Surface *new_sur;
    char path[ 512 ];
    SDL_PixelFormat *spf;
#ifdef USE_PNG
    char png_name[32];
#endif

#ifdef USE_PNG
    /* override file name as all graphics were changed from
    bitmap to png so the extension must be corrected */
    memset( png_name, 0, sizeof( png_name ) );
    strncpy( png_name, fname, strlen( fname ) - 4 );
    strcat( png_name, ".png" );
    get_full_bmp_path( path, png_name );
    buf = load_png( path );
#else
    get_full_bmp_path( path, fname );
    buf = SDL_LoadBMP( path );
#endif
    if ( buf == 0 ) {

        fprintf( stderr, "load_surf: file '%s' not found or not enough memory\n", path );
        if ( f & SDL_NONFATAL )
            return 0;
        else
            exit( 1 );

    }
/*    if ( !(f & SDL_HWSURFACE) ) {

        SDL_SetColorKey( buf, SDL_SRCCOLORKEY, 0x0 );
        return buf;

    }
    new_sur = create_surf(buf->w, buf->h, f);
    SDL_BlitSurface(buf, 0, new_sur, 0);
    SDL_FreeSurface(buf);*/
    spf = SDL_GetVideoSurface()->format;
    new_sur = SDL_ConvertSurface( buf, spf, f );
    SDL_FreeSurface( buf );
    SDL_SetColorKey( new_sur, SDL_SRCCOLORKEY, 0x0 );
    SDL_SetAlpha( new_sur, 0, 0 ); /* no alpha */
    return new_sur;
}

/*
    create an surface
    MUST NOT BE USED IF NO SDLSCREEN IS SET
*/
SDL_Surface* create_surf(int w, int h, int f)
{
    SDL_Surface *sur;
    SDL_PixelFormat *spf = SDL_GetVideoSurface()->format;
    if ((sur = SDL_CreateRGBSurface(f, w, h, spf->BitsPerPixel, spf->Rmask, spf->Gmask, spf->Bmask, spf->Amask)) == 0) {
        fprintf(stderr, "create_surf: not enough memory to create surface...\n");
        exit(1);
    }
/*    if (f & SDL_HWSURFACE && !(sur->flags & SDL_HWSURFACE))
        fprintf(stderr, "unable to create surface (%ix%ix%i) in hardware memory...\n", w, h, spf->BitsPerPixel);*/
    SDL_SetColorKey(sur, SDL_SRCCOLORKEY, 0x0);
    SDL_SetAlpha(sur, 0, 0); /* no alpha */
    return sur;
}

/*
 * Free a surface if != NULL and set pointer to NULL
 */
void free_surf( SDL_Surface **surf )
{
    if ( *surf ) SDL_FreeSurface( *surf );
    *surf = 0;
}
/*
    lock surface
*/
inline void lock_surf(SDL_Surface *sur)
{
    if (SDL_MUSTLOCK(sur))
        SDL_LockSurface(sur);
}

/*
    unlock surface
*/
inline void unlock_surf(SDL_Surface *sur)
{
    if (SDL_MUSTLOCK(sur))
        SDL_UnlockSurface(sur);
}

/*
    blit surface with destination DEST and source SOURCE using it's actual alpha and color key settings
*/
void blit_surf(void)
{
#ifdef SDL_1_1_5
    if (sdl.s.s->flags & SDL_SRCALPHA)
        SDL_SetAlpha(sdl.s.s, SDL_SRCALPHA, 255 - sdl.s.s->format->alpha);
#endif
    SDL_BlitSurface(sdl.s.s, &sdl.s.r, sdl.d.s, &sdl.d.r);
#ifdef SDL_1_1_5
    if (sdl.s.s->flags & SDL_SRCALPHA)
        SDL_SetAlpha(sdl.s.s, SDL_SRCALPHA, 255 - sdl.s.s->format->alpha);
#endif
}

/*
    do an alpha blit
*/
void alpha_blit_surf(int alpha)
{
 #ifdef SDL_1_1_5
    SDL_SetAlpha(sdl.s.s, SDL_SRCALPHA, 255 - alpha);
#else
    SDL_SetAlpha(sdl.s.s, SDL_SRCALPHA, alpha);
#endif
    SDL_BlitSurface(sdl.s.s, &sdl.s.r, sdl.d.s, &sdl.d.r);
    SDL_SetAlpha(sdl.s.s, 0, 0);
}

/*
    fill surface with color c
*/
void fill_surf(int c)
{
    SDL_FillRect(sdl.d.s, &sdl.d.r, SDL_MapRGB(sdl.d.s->format, c >> 16, (c >> 8) & 0xFF, c & 0xFF));
}

/* set clipping rect */
void set_surf_clip( SDL_Surface *surf, int x, int y, int w, int h )
{
#ifdef SDL_1_1_5
    SDL_Rect rect = { x, y, w, h };
    if ( w == h || h == 0 )
        SDL_SetClipRect( surf, 0 );
    else
        SDL_SetClipRect( surf, &rect );
#else
    SDL_SetClipping( surf, x, y, w, h );
#endif
}

/* set pixel */
Uint32 set_pixel( SDL_Surface *surf, int x, int y, int pixel )
{
    int pos = 0;

    pos = y * surf->pitch + x * surf->format->BytesPerPixel;
    memcpy( surf->pixels + pos, &pixel, surf->format->BytesPerPixel );
    return pixel;
}

/* get pixel */
Uint32 get_pixel( SDL_Surface *surf, int x, int y )
{
    int pos = 0;
    Uint32 pixel = 0;

    pos = y * surf->pitch + x * surf->format->BytesPerPixel;
    memcpy( &pixel, surf->pixels + pos, surf->format->BytesPerPixel );
    return pixel;
}

/* draw a shadowed frame and darken contents which starts at cx,cy */
void draw_3dframe( SDL_Surface *surf, int cx, int cy, int w, int h, int border )
{
    int i, j;
    SDL_Surface *frame = 0;
    SDL_Surface *contents = 0;
    frame = create_surf( w + border * 2, h + border * 2, SDL_SWSURFACE );
    SDL_SetColorKey( frame, SDL_SRCCOLORKEY, SDL_MapRGB( surf->format, 0xff, 0, 0 ) );
    FULL_DEST( frame ); fill_surf ( 0xff0000 );
    /* move contents by border size -1 */
    DEST( surf, cx, cy, w, h );
    SOURCE( surf, cx - border + 1, cy - border + 1 );
    blit_surf();
    /* shadow part */
    FULL_DEST( frame ); fill_surf ( 0xff0000 );
    DEST( frame, 0, 0, w + border, border ); fill_surf( 0x0 );
    DEST( frame, 0, 0, border, h + border ); fill_surf( 0x0 );
    for ( i = 0; i < border; i++ ) {
        for ( j = 0; j < border; j++ ) {
            if ( i < j )
                set_pixel( frame, border + w + i, border - j - 1, SDL_MapRGB( frame->format, 0,0,0 ) );
        }
    }
    for ( i = 0; i < border; i++ ) {
        for ( j = 0; j < border; j++ ) {
            if ( i > j )
                set_pixel( frame, border - i - 1, border + h + j, SDL_MapRGB( frame->format, 0,0,0 ) );
        }
    }
    DEST( surf, cx - border, cy - border, w + border * 2, h + border * 2 );
    SOURCE( frame, 0, 0 );
    alpha_blit_surf( 48 );
    /* bright part */
    FULL_DEST( frame ); fill_surf ( 0xff0000 );
    DEST( frame, w + border, border, border, h + border ); fill_surf( 0xffffff );
    DEST( frame, border, h + border, w + border, border ); fill_surf( 0xffffff );
    for ( i = 0; i < border; i++ ) {
        for ( j = 0; j < border; j++ ) {
            if ( i >= j )
                set_pixel( frame, border + w + i, border - j - 1, SDL_MapRGB( frame->format, 0xff,0xff,0xff ) );
        }
    }
    for ( i = 0; i < border; i++ ) {
        for ( j = 0; j < border; j++ ) {
            if ( i <= j )
                set_pixel( frame, border - i - 1, border + h + j, SDL_MapRGB( frame->format, 0xff,0xff,0xff ) );
        }
    }
    DEST( surf, cx - border, cy - border, w + border * 2, h + border * 2 );
    SOURCE( frame, 0, 0 );
    alpha_blit_surf( 128 );
    /* darken contents */
    contents = create_surf( w, h, SDL_SWSURFACE );
    SDL_SetColorKey( contents, 0, 0 );
    FULL_DEST( contents ); fill_surf( 0x0 );
    DEST( surf, cx, cy, w, h );
    SOURCE( contents, 0, 0 );
    alpha_blit_surf( 96 );

    SDL_FreeSurface( contents );
    SDL_FreeSurface( frame ); 
}

/* sdl font */

/* return full font path */
void get_full_font_path( char *path, char *file_name )
{
    strcpy( path, file_name );
/*    sprintf(path, "./gfx/fonts/%s", file_name ); */
}

/*
    load a font using the width values in the file
*/
Font* load_font(char *fname)
{
    Font    *fnt = 0;
    FILE    *file = 0;
    char    path[512];
    int     i;

    get_full_font_path( path, fname );

    fnt = malloc(sizeof(Font));
    if (fnt == 0) {
        fprintf(stderr, "load_font: not enough memory\n");
        exit(1);
    }

    if ((fnt->pic = load_surf(path, SDL_HWSURFACE)) == 0)
        exit(1);
    /* use very first pixel as transparency key */
    SDL_SetColorKey( fnt->pic, SDL_SRCCOLORKEY, get_pixel( fnt->pic, 0, 0 ) );		
		
    fnt->align = ALIGN_X_LEFT | ALIGN_Y_TOP;
    fnt->color = 0x00FFFFFF;
    fnt->height = fnt->pic->h;
	
    /* table */
    file = fopen(path, "r");
    fseek(file, -1, SEEK_END);
    fread(&fnt->offset, 1, 1, file);
#ifdef SDL_DEBUG
    printf("offset: %i\n", fnt->offset);
#endif
    fseek(file, -2, SEEK_END);
    fread(&fnt->length, 1, 1, file);
#ifdef SDL_DEBUG
    printf("number: %i\n", fnt->length);
#endif
    fseek(file, -2 - fnt->length, SEEK_END);
    fread(fnt->char_width, 1, fnt->length, file);
#ifdef SDL_DEBUG
    printf("letter width: %i\n", fnt->length);
    for (i = 0; i < fnt->length; i++)
        printf("%i ", fnt->char_width[i]);
    printf("\n");
#endif
    fclose(file);

    /* letter offsets */
    fnt->char_offset[0] = 0;
    for (i = 1; i < fnt->length; i++)	
        fnt->char_offset[i] = fnt->char_offset[i - 1] + fnt->char_width[i - 1];

    /* allowed keys */
    memset(fnt->keys, 0, sizeof(fnt->keys));
    for (i = 0; i < fnt->length; i++) {
        fnt->keys[i + fnt->offset] = 1;
    }

    fnt->last_x = fnt->last_y = fnt->last_width = fnt->last_height = 0;
    return fnt;
}

/*
    load a font with fixed size
*/
Font *load_fixed_font(char *f, int off, int len, int w)
{
    int     i;
    Font    *fnt;
    char    path[512];

    get_full_font_path( path, f );

    fnt = malloc(sizeof(Font));
    if (fnt == 0) {
        fprintf(stderr, "load_fixed_font: not enough memory\n");
        exit(1);
    }

    if ((fnt->pic = load_surf(path, SDL_HWSURFACE)) == 0)
        exit(1);
    /* use very first pixel as transparency key */
    SDL_SetColorKey( fnt->pic, SDL_SRCCOLORKEY, get_pixel( fnt->pic, 0, 0 ) );		

    fnt->align = ALIGN_X_LEFT | ALIGN_Y_TOP;
    fnt->color = 0x00FFFFFF;
    fnt->height = fnt->pic->h;
	
	fnt->offset = off;
	fnt->length = len;
	
	for (i = 0; i < len; i++)
	    fnt->char_width[i] = w;
	
    /* letter offsets */
    fnt->char_offset[0] = 0;
    for (i = 1; i < fnt->length; i++)	
        fnt->char_offset[i] = fnt->char_offset[i - 1] + w;
	
    /* allowed keys*/
    memset(fnt->keys, 0, sizeof(fnt->keys));
    for (i = 0; i < fnt->length; i++) {
        fnt->keys[i + fnt->offset] = 1;
    }
	
    fnt->last_x = fnt->last_y = fnt->last_width = fnt->last_height = 0;
    return fnt;
}

/*
    free memory
*/
void free_font(Font **fnt)
{
    if ( (*fnt)->pic) SDL_FreeSurface( (*fnt)->pic);
    free( *fnt );
    *fnt = 0;
}

/*
    write something with transparency
*/
int write_text(Font *fnt, SDL_Surface *dest, int x, int y, char *str, int alpha)
{
    int	c_abs;
    int len = strlen(str);
    int pix_len = 0;
    int px = x, py = y;
    int i;
    SDL_Surface *spf = SDL_GetVideoSurface();
	
    pix_len = text_width(fnt, str);
	for (i = 0; i < len; i++)
	    if (!fnt->keys[(int)str[i]])
	        str[i] = ' ';

    /* alignment */
    if (fnt->align & ALIGN_X_CENTER)
        px -= pix_len >> 1;
    else
        if (fnt->align & ALIGN_X_RIGHT)
            px -= pix_len;
    if (fnt->align & ALIGN_Y_CENTER)
        py -= (fnt->height >> 1 ) + 1;
    else
        if (fnt->align & ALIGN_Y_BOTTOM)
            py -= fnt->height;

    /* do only set last rect if font->save_last is true */
    if ( fnt->save_last ) {
        fnt->last_x = px; if (fnt->last_x < 0) fnt->last_x = 0;
        fnt->last_y = py; if (fnt->last_y < 0) fnt->last_y = 0;
        fnt->last_width = pix_len; if (fnt->last_x + fnt->last_width >= spf->w) fnt->last_width = spf->w - fnt->last_x;
        fnt->last_height = fnt->height; if (fnt->last_y + fnt->last_height >= spf->h) fnt->last_height = spf->h - fnt->last_y;
    }

    if (alpha != 0)
        SDL_SetAlpha(fnt->pic, SDL_SRCALPHA, alpha);
    else
        SDL_SetAlpha(fnt->pic, 0, 0);
    for (i = 0; i < len; i++) {
       	c_abs = str[i] - fnt->offset;
       	DEST(dest, px, py, fnt->char_width[c_abs], fnt->height);
       	SOURCE(fnt->pic, fnt->char_offset[c_abs], 0);
       	blit_surf();
        px += fnt->char_width[c_abs];
    }
	
    return 0;
}

/*
    lock font surface
*/
inline void lock_font(Font *fnt)
{
    if (SDL_MUSTLOCK(fnt->pic))
        SDL_LockSurface(fnt->pic);
}

/*
    unlock font surface
*/
inline void unlock_font(Font *fnt)
{
    if (SDL_MUSTLOCK(fnt->pic))
        SDL_UnlockSurface(fnt->pic);
}
	
/*
    return last update region
*/
SDL_Rect last_write_rect(Font *fnt)
{
    SDL_Rect    rect={fnt->last_x, fnt->last_y, fnt->last_width, fnt->last_height};
    return rect;
}

/*
    return the text width in pixels
*/
int text_width(Font *fnt, char *str)
{
    unsigned int i;
    int pix_len = 0;
    for (i = 0; i < strlen(str); i++)
        pix_len += fnt->char_width[str[i] - fnt->offset];
    return pix_len;
}

/* sdl */

/*
    initialize sdl
*/
void init_sdl( int f )
{
    int i;
    int valid_depth = 0;

    /* check flags: if SOUND is not enabled flag SDL_INIT_AUDIO musn't be set */
#ifndef WITH_SOUND
    if ( f & SDL_INIT_AUDIO )
        f = f & ~SDL_INIT_AUDIO;
#endif

    sdl.screen = 0;
    if (SDL_Init(f) < 0) {
        fprintf(stderr, "ERR: sdl_init: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_EnableUNICODE(1);
    atexit(SDL_Quit);
    /* check resolutions */
    for ( i = 0; i < mode_count; i++ ) {
        if ( ( valid_depth = SDL_VideoModeOK( modes[i].width, modes[i].height, modes[i].depth, modes[i].flags ) ) != 0 ) {
            modes[i].depth = valid_depth;
            modes[i].ok = 1;
            printf( "Mode %s valid\n", modes[i].name );
        }
    }
    /* reset default video mode if none found exit */
    if ( !def_mode->ok ) {
        for ( i = 0; i < mode_count; i++ )
            if ( modes[i].ok ) {
                def_mode = &modes[i];
                break;
            }
        /* no valid default mode found? exit */
        if ( i == mode_count ) {
            fprintf( stderr, "No valid video mode found!\n" );
            exit( 1 );
        }
    }
    /* create empty cursor */
    empty_cursor = create_cursor( 16, 16, 8, 8,
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
    std_cursor = SDL_GetCursor();
}

/*
    free screen
*/
void quit_sdl()
{
    if (sdl.screen) SDL_FreeSurface(sdl.screen);
    if ( empty_cursor ) SDL_FreeCursor( empty_cursor );
}

/*
====================================================================
Get a verified video mode.
====================================================================
*/
Video_Mode def_video_mode()
{
    return *def_mode;
}
Video_Mode std_video_mode( int id )
{
    return modes[id];
}
Video_Mode video_mode( int width, int height, int depth, int flags )
{
    Video_Mode mode;
    /* set name */
    sprintf( mode.name, "%ix%ix%i", width, height, depth );
    if ( flags & SDL_FULLSCREEN )
        strcat( mode.name, " Fullscreen" );
    else
        strcat( mode.name, " Window" );
    /* check mode */
    if ( SDL_VideoModeOK( width, height, depth, flags ) != depth ) {
        fprintf( stderr, "video_mode: %s invalid: using default mode\n", mode.name );
        return def_video_mode();
    }
    /* set and return this mode */
    mode.id = -1;
    mode.width = width;
    mode.height = height;
    mode.depth = depth;
    mode.flags = flags;
    return mode;
}
/*
====================================================================
Current video mode.
====================================================================
*/
Video_Mode* cur_video_mode()
{
    return &cur_mode;
}
/*
====================================================================
Get a list with all valid standard mode names.
====================================================================
*/
char** get_mode_names( int *count )
{
    char **lines;
    int i, j;

    *count = 0;
    for ( i = 0; i < mode_count; i++ )
        if ( modes[i].ok )
            (*count)++;
    lines = calloc( *count, sizeof( char* ) );
    for ( i = 0, j = 0; i < mode_count; i++ )
        if ( modes[i].ok )
            lines[j++] = strdup( modes[i].name );
    return lines;
}
/*
====================================================================
Switch to passed video mode.
====================================================================
*/
int	set_video_mode( Video_Mode mode )
{
#ifdef SDL_DEBUG
    SDL_PixelFormat	*fmt;
#endif
	
    /* free old screen */
    if (sdl.screen) SDL_FreeSurface( sdl.screen );

    /* check again */
    mode = video_mode( mode.width, mode.height, mode.depth, mode.flags );
    /* set as current mode */
    cur_mode = mode;

    /* set video mode */
    if ( ( sdl.screen = SDL_SetVideoMode( mode.width, mode.height, mode.depth, mode.flags ) ) == 0 ) {
        fprintf(stderr, "set_video_mode: cannot allocate screen: %s\n", SDL_GetError());
        return 1;
    }

#ifdef SDL_DEBUG				
    if (f & SDL_HWSURFACE && !(sdl.screen->flags & SDL_HWSURFACE))
       	fprintf(stderr, "unable to create screen in hardware memory...\n");
    if (f & SDL_DOUBLEBUF && !(sdl.screen->flags & SDL_DOUBLEBUF))
        fprintf(stderr, "unable to create double buffered screen...\n");
    if (f & SDL_FULLSCREEN && !(sdl.screen->flags & SDL_FULLSCREEN))
        fprintf(stderr, "unable to switch to fullscreen...\n");

    fmt = sdl.screen->format;
    printf("video mode format:\n");
    printf("Masks: R=%i, G=%i, B=%i\n", fmt->Rmask, fmt->Gmask, fmt->Bmask);
    printf("LShft: R=%i, G=%i, B=%i\n", fmt->Rshift, fmt->Gshift, fmt->Bshift);
    printf("RShft: R=%i, G=%i, B=%i\n", fmt->Rloss, fmt->Gloss, fmt->Bloss);
    printf("BBP: %i\n", fmt->BitsPerPixel);
    printf("-----\n");
#endif    		
		
    return 0;
}

/*
    show hardware capabilities
*/
void hardware_cap()
{
    const SDL_VideoInfo	*vi = SDL_GetVideoInfo();
    char *ny[2] = {"No", "Yes"};

    printf("video hardware capabilities:\n");
    printf("Hardware Surfaces: %s\n", ny[vi->hw_available]);
    printf("HW_Blit (CC, A): %s (%s, %s)\n", ny[vi->blit_hw], ny[vi->blit_hw_CC], ny[vi->blit_hw_A]);
    printf("SW_Blit (CC, A): %s (%s, %s)\n", ny[vi->blit_sw], ny[vi->blit_sw_CC], ny[vi->blit_sw_A]);
    printf("HW_Fill: %s\n", ny[vi->blit_fill]);
    printf("Video Memory: %i\n", vi->video_mem);
    printf("------\n");
}

/*
    update rectangle (0,0,0,0)->fullscreen
*/
inline void refresh_screen(int x, int y, int w, int h)
{
    SDL_UpdateRect(sdl.screen, x, y, w, h);
}

/*
    draw all update regions
*/
void refresh_rects()
{
    if (sdl.rect_count == RECT_LIMIT)
        SDL_UpdateRect(sdl.screen, 0, 0, sdl.screen->w, sdl.screen->h);
    else
        SDL_UpdateRects(sdl.screen, sdl.rect_count, sdl.rect);
    sdl.rect_count = 0;
}

/*
    add update region
*/
void add_refresh_rect(int x, int y, int w, int h)
{
    if (sdl.rect_count == RECT_LIMIT) return;
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x + w > sdl.screen->w)
        w = sdl.screen->w - x;
    if (y + h > sdl.screen->h)
        h = sdl.screen->h - y;
    if (w <= 0 || h <= 0)
        return;
    sdl.rect[sdl.rect_count].x = x;
    sdl.rect[sdl.rect_count].y = y;
    sdl.rect[sdl.rect_count].w = w;
    sdl.rect[sdl.rect_count].h = h;
    sdl.rect_count++;
}

/*
    fade screen to black
*/
void dim_screen(int steps, int delay, int trp)
{
#ifndef NODIM
    SDL_Surface    *buffer;
    int per_step = trp / steps;
    int i;
    if (term_game) return;
    buffer = create_surf(sdl.screen->w, sdl.screen->h, SDL_SWSURFACE);
    SDL_SetColorKey(buffer, 0, 0);
    FULL_DEST(buffer);
    FULL_SOURCE(sdl.screen);
    blit_surf();
    for (i = 0; i <= trp; i += per_step) {
        FULL_DEST(sdl.screen);
        fill_surf(0x0);
        FULL_SOURCE(buffer);
        alpha_blit_surf(i);
        refresh_screen( 0, 0, 0, 0);
        SDL_Delay(delay);
    }
    if (trp == 255) {
        FULL_DEST(sdl.screen);
        fill_surf(0x0);
        refresh_screen( 0, 0, 0, 0);
    }
    SDL_FreeSurface(buffer);
#else
    refresh_screen( 0, 0, 0, 0);
#endif
}

/*
    undim screen
*/
void undim_screen(int steps, int delay, int trp)
{
#ifndef NODIM
    SDL_Surface    *buffer;
    int per_step = trp / steps;
    int i;
    if (term_game) return;
    buffer = create_surf(sdl.screen->w, sdl.screen->h, SDL_SWSURFACE);
    SDL_SetColorKey(buffer, 0, 0);
    FULL_DEST(buffer);
    FULL_SOURCE(sdl.screen);
    blit_surf();
    for (i = trp; i >= 0; i -= per_step) {
        FULL_DEST(sdl.screen);
        fill_surf(0x0);
        FULL_SOURCE(buffer);
        alpha_blit_surf(i);
        refresh_screen( 0, 0, 0, 0);
        SDL_Delay(delay);
    }
    FULL_DEST(sdl.screen);
    FULL_SOURCE(buffer);
    blit_surf();
    refresh_screen( 0, 0, 0, 0);
    SDL_FreeSurface(buffer);
#else
    refresh_screen( 0, 0, 0, 0);
#endif
}

/*
    wait for a key
*/
int wait_for_key()
{
    /* wait for key */
    SDL_Event event;
    while (1) {
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) {
            term_game = 1;
            return 0;
        }
        if (event.type == SDL_KEYDOWN)
            return event.key.keysym.sym;
    }
}

/*
    wait for a key or mouse click
*/
void wait_for_click()
{
    /* wait for key or button */
    SDL_Event event;
    while (1) {
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) {
            term_game = 1;
            return;
        }
        if (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONUP)
            return;
    }
}

/*
    lock surface
*/
inline void lock_screen()
{
    if (SDL_MUSTLOCK(sdl.screen))
        SDL_LockSurface(sdl.screen);
}

/*
    unlock surface
*/
inline void unlock_screen()
{
    if (SDL_MUSTLOCK(sdl.screen))
        SDL_UnlockSurface(sdl.screen);
}

/*
    flip hardware screens (double buffer)
*/
inline void flip_screen()
{
    SDL_Flip(sdl.screen);
}

/* cursor */

/* creates cursor */
SDL_Cursor* create_cursor( int width, int height, int hot_x, int hot_y, char *source )
{
    unsigned char *mask = 0, *data = 0;
    SDL_Cursor *cursor = 0;
    int i, j, k;
    unsigned char data_byte, mask_byte;
    int pot;

    /* meaning of char from source:
        b : black, w: white, ' ':transparent */

    /* create mask&data */
    mask = malloc( width * height * sizeof ( char ) / 8 );
    data = malloc( width * height * sizeof ( char ) / 8 );

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
    get milliseconds since last call
*/
inline int get_time()
{
    int ms;
    cur_time = SDL_GetTicks();
    ms = cur_time - last_time;
    last_time = cur_time;
    if (ms == 0) {
        ms = 1;
        SDL_Delay(1);
    }
    return ms;
}

/*
    reset timer
*/
inline void reset_timer()
{
    last_time = SDL_GetTicks();
}

void fade_screen( int type, int length )
{
    SDL_Surface *buffer = 0;
    float alpha;
    float alpha_change; /* per ms */
    int leave = 0;
    int ms;

    if ( !sdl.fade ) {
        if ( type == FADE_IN )
            refresh_screen( 0, 0, 0, 0 );
        else {
            FULL_DEST( sdl.screen );
            fill_surf( 0x0 );
            refresh_screen( 0, 0, 0, 0 );
        }
    }

    /* get screen contents */
    buffer = create_surf( sdl.screen->w, sdl.screen->h, SDL_SWSURFACE );
    SDL_SetColorKey( buffer, 0, 0 );
    FULL_DEST( buffer ); FULL_SOURCE( sdl.screen ); blit_surf();

    /* compute alpha and alpha change */
    if ( type == FADE_OUT ) {
        alpha = 0;
        alpha_change = 255.0 / length;
    }
    else {
        alpha = 255;
        alpha_change = -255.0 / length;
    }

    /* fade */
    reset_timer();
    while ( !leave ) {
        ms = get_time();
        alpha += alpha_change * ms;
        if ( type == FADE_OUT && alpha >= 255 ) break;
        if ( type == FADE_IN && alpha <= 0 ) break;
        /* update */
        FULL_DEST( sdl.screen ); fill_surf(0x0);
        FULL_SOURCE( buffer ); alpha_blit_surf( (int)alpha );
        refresh_screen( 0, 0, 0, 0);
    }

    /* update screen */
    FULL_DEST( sdl.screen ); FULL_SOURCE( buffer );
    if ( type == FADE_IN )
        blit_surf();
    else
        fill_surf( 0x0 );
    refresh_screen( 0, 0, 0, 0 );
    SDL_FreeSurface(buffer);
}

void take_screenshot( int i )
{
    char str[32];
    sprintf( str, "screenshot%i.bmp", i );
    SDL_SaveBMP( sdl.screen, str );
}

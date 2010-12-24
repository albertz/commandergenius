/***************************************************************************
                          stk.h  -  description
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
 
#ifndef __STK_H
#define __STK_H

#include <SDL.h>
#ifdef AUDIO_ENABLED
  #include <SDL_mixer.h>
#endif

/* GENERAL */

#define SDL_NONFATAL 0x10000000

#define STK_ABORT( msg ) \
 { fprintf( stderr, "Fatal STK Error!\n%s\n", msg ); exit( 1 ); }

#define STK_IN_RECT( rect, mx, my ) \
 ( mx >= (rect).x && my >= (rect).y && \
   mx < (rect).x + (rect).w && my < (rect).y + (rect).h )
 
#define STK_OPAQUE SDL_ALPHA_OPAQUE
#define STK_TRANSPARENT SDL_ALPHA_TRANSPARENT
 
enum {
    STK_BUTTON_LEFT = 1,
    STK_BUTTON_MIDDLE,
    STK_BUTTON_RIGHT,
    STK_WHEEL_UP,
    STK_WHEEL_DOWN,
    STK_BUTTON_COUNT
};
 
/*
====================================================================
Initiate SDL, build a default cursor and install the clean
up function stk_quit().
====================================================================
*/
void stk_init( int flags );
 
/*
====================================================================
Block until either a key or button was pressed. If SDL_QUIT
was received this will set stk_quit_request True.
====================================================================
*/
void stk_wait_for_input( void );
 
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
int stk_display_open( int flags, int width, int height, int depth );

/*
====================================================================
Add an update rectangle that will be updated by 
stk_display_update(). Regardless of clipping the rectangles
must fit the screen else it'll raise an X error.
If NULL is passed as 'rect' the whole screen is stored for update.
====================================================================
*/
void stk_display_store_rect( SDL_Rect *rect );

/*
====================================================================
Store the destination rectangle of the last blit operation. SDL
modified this rectangle to the actually updated clipping region.
====================================================================
*/
void stk_display_store_drect( void );

/*
====================================================================
Either update all gathered rects or simply the full screen.
In any case the stored regions are cleared.
====================================================================
*/
enum {
    STK_UPDATE_ALL = 0,
    STK_UPDATE_RECTS
};
void stk_display_update( int type );

/*
====================================================================
Fade the current contents of the display either in or out. 'time'
is the time in milliseconds the fading is supposed to take.
====================================================================
*/
enum {
    STK_FADE_IN = 0,
    STK_FADE_OUT,
    STK_FADE_DEFAULT_TIME = 350
};
void stk_display_fade( int type, int time );

/*
====================================================================
Take a screenshot and save it to screenshot[index].bmp in the
current directory.
====================================================================
*/
void stk_display_take_screenshot();

/*
====================================================================
Switch fullscreen/windowed for current resolution.
====================================================================
*/
void stk_display_apply_fullscreen( int fullscreen );

/* SURFACE */

/*
====================================================================
If stk_surface_load() is called with a relative path this prefix
is added. Default is '.'; 'path' is copied.
====================================================================
*/
void stk_surface_set_path( char *path );

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
The default color key of the surface is black and no alpha.
====================================================================
*/
SDL_Surface* stk_surface_load( int flags, char *format, ... );

/*
====================================================================
Create a surface with the given size and the format of the 
video mode which must be set.
The default color key of the surface is black and no alpha.
====================================================================
*/
SDL_Surface* stk_surface_create( int flags, int width, int height );

/*
====================================================================
Free the memory of a surface if not NULL and reset the pointer
to NULL.
====================================================================
*/
void stk_surface_free( SDL_Surface **surface );

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
    SDL_Surface *dest, int dx, int dy );

/*
====================================================================
Different from stk_surface_blit() this function temporarily
overwrites 'src's alpha value. It is not recommended to use
this function if performance is important as it is 10 times slower
than an SDL_SetAlpha() combined with stk_surface_blit().
====================================================================
*/
void stk_surface_alpha_blit( 
    SDL_Surface *src, int sx, int sy, int sw, int sh,
    SDL_Surface *dest, int dx, int dy, int alpha );
    
/*
====================================================================
Fill a rectangle of the surface with a given color of the format
0xRRGGBB. 'dw' == -1 and 'dh' == -1 have the same effect as in
stk_surface_blit().
====================================================================
*/
void stk_surface_fill( 
    SDL_Surface *dest, int dx, int dy, int dw, int dh,
    int color );

/*
====================================================================
Set the clipping region of a surface. All blit operations into
this surface will only have effect within the clipping region.
'dw' == -1 and 'dh' == -1 have the same effect as in
stk_surface_blit().
====================================================================
*/
void stk_surface_clip( 
    SDL_Surface *dest, int dx, int dy, int dw, int dh );

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
    SDL_Surface *dest, int dx, int dy, Uint32 pixel );
Uint32 stk_surface_get_pixel( SDL_Surface *src, int sx, int sy );
 
/*
====================================================================
Convert part of the surface to gray and if 'dark' is set cut the 
brightness in half. 'dw' == -1 and 'dh' == -1 have the same 
effect as in tk_surface_blit().
====================================================================
*/
void stk_surface_gray( 
    SDL_Surface *dest, int dx, int dy, int dw, int dh, int dark );
 
/*
====================================================================
Add a 3D frame to surface and dark to contents. The frame border
is 'border' thick.
====================================================================
*/
void stk_surface_add_3dframe(
    SDL_Surface *dest, int dx, int dy, int dw, int dh, int border );

/*
====================================================================
Fill the surface with the wallpaper (clipped)
====================================================================
*/
void stk_surface_apply_wallpaper( 
    SDL_Surface *dest, int dx, int dy, int dw, int dh, 
    SDL_Surface *wallpaper, int alpha );

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
    SDL_Surface *frame );

/* FIXED FONT */

enum {
    STK_FONT_ALIGN_LEFT     = (1L << 1),
    STK_FONT_ALIGN_RIGHT    = (1L << 2),
    STK_FONT_ALIGN_TOP      = (1L << 3),
    STK_FONT_ALIGN_BOTTOM   = (1L << 4),
    STK_FONT_ALIGN_CENTER_X = (1L << 5),
    STK_FONT_ALIGN_CENTER_Y = (1L << 6)
};
typedef struct {
    SDL_Surface *surface;
    int         align; /* text aligment */
    int         width; /* of a single character */
    int         height; /* of a single character */
} StkFont;

/*
====================================================================
Load a fixed font which is simply a surface containing the ASCII
characters from 32 (blank) to 96 (whatever) where all characters
have the same width.
====================================================================
*/
StkFont* stk_font_load( int flags, char *format, ... );

/*
====================================================================
Free a font if not NULL and reset pointer to NULL.
====================================================================
*/
void stk_font_free( StkFont **font );

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
    int alpha, char *string );

/*
====================================================================
Query the length of the string in pixels.
====================================================================
*/
#define stk_font_string_width( font, string ) \
 (strlen(string) * font->width)

/* CURSOR */

/*
====================================================================
Build an SDL cursor with the given hotspot. The resource mask
contains 'b' for black 'w' for white and ' ' for transparent.
There are two cursors provided:
  extern SDL_Cursor *empty_cursor  (all transparent)
  extern SDL_Cursor *std_cursor    (SDL standard cursor)
====================================================================
*/
SDL_Cursor* stk_cursor_create(
    int width, int height, int hot_x, int hot_y, char *src );

/*
====================================================================
Free a cursor if not NULL and reset pointer to NULL.
====================================================================
*/
void stk_cursor_free( SDL_Cursor **cursor );

/* TIMER */

/*
====================================================================
Reset the timer.
====================================================================
*/
void stk_timer_reset( void );

/*
====================================================================
Get the time since last call or reset.
====================================================================
*/
int stk_timer_get_time( void );

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
    int frequency, Uint16 format, int channels, int chunksize );

/*
====================================================================
Close mixer if previously opened.
====================================================================
*/
void stk_audio_close( void );

/*
====================================================================
If stk_sound_load() is called with a relative path this prefix
is added. Default is '.'; 'path' is copied.
====================================================================
*/
void stk_audio_set_path( char *path );

/*
====================================================================
Enable/disable sound. If disabled stk_sound_play () has no effect.
====================================================================
*/
void stk_audio_enable_sound( int enable );

/*
====================================================================
Set default volume of all sounds: 0 - 128
====================================================================
*/
void stk_audio_set_sound_volume( int volume );

/*
====================================================================
Fade out a specific sound channel. If 'channel is -1 all
channels fade out. 'ms' is the time the fading shall take.
====================================================================
*/
void stk_audio_fade_out( int channel, int ms );

/* SOUND */

typedef struct {
#ifdef AUDIO_ENABLED
    Mix_Chunk *chunk; /* SDL_Mixer's sound buffer */
#endif
    int       channel;
    int       volume;
} StkSound;

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
StkSound *stk_sound_load( int channel, char *format, ... );

/*
====================================================================
Free the memory of a sound if not NULL and reset the pointer
to NULL.
====================================================================
*/
void stk_sound_free( StkSound **sound );

/*
====================================================================
Set channel/volume of sound.
====================================================================
*/
void stk_sound_set_volume( StkSound *sound, int volume );
void stk_sound_set_channel( StkSound *sound, int channel );

/*
====================================================================
Play a sound.
====================================================================
*/
void stk_sound_play( StkSound *sound );
/*
====================================================================
Play a sound at horizontal position x..
====================================================================
*/
void stk_sound_play_x( int x, StkSound *sound );

/*
====================================================================
Fade out the channel of this sound.
====================================================================
*/
void stk_sound_fade_out( StkSound *sound, int ms );


#endif

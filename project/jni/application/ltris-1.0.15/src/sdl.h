/***************************************************************************
                          sdl.h  -  description
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

#ifndef SDL_H
#define SDL_H

#include <SDL.h>

#define BITDEPTH 16

#ifdef __cplusplus
extern "C" {
#endif

// draw region //
#define DEST(p, i, j, k, l) {sdl.d.s = p; sdl.d.r.x = i; sdl.d.r.y = j; sdl.d.r.w = k; sdl.d.r.h = l;}
#define SOURCE(p, i, j) {sdl.s.s = p; sdl.s.r.x = i; sdl.s.r.y = j; sdl.s.r.w = sdl.d.r.w; sdl.s.r.h = sdl.d.r.h;}
#define FULL_DEST(p) {sdl.d.s = p; sdl.d.r.x = 0; sdl.d.r.y = 0; sdl.d.r.w = (p)->w; sdl.d.r.h = (p)->h;}
#define FULL_SOURCE(p) {sdl.s.s = p; sdl.s.r.x = 0; sdl.s.r.y = 0; sdl.s.r.w = sdl.d.r.w; sdl.s.r.h = sdl.d.r.h;}
typedef struct {
    SDL_Surface *s;
    SDL_Rect    r;
} DrawRgn;

// Sdl Surface //
#define SDL_NONFATAL 0x10000000
SDL_Surface* load_surf(char *fname, int f);
SDL_Surface* create_surf(int w, int h, int f);
void free_surf( SDL_Surface **surf );
inline void lock_surf(SDL_Surface *sur);
inline void unlock_surf(SDL_Surface *sur);
void blit_surf(void);
void alpha_blit_surf(int alpha);
void fill_surf(int c);
void set_surf_clip( SDL_Surface *surf, int x, int y, int w, int h );
Uint32 set_pixel( SDL_Surface *surf, int x, int y, int pixel );
Uint32 get_pixel( SDL_Surface *surf, int x, int y );
/* draw a shadowed frame and darken contents which starts at cx,cy */
void draw_3dframe( SDL_Surface *surf, int cx, int cy, int w, int h, int border );

// Sdl Font //
enum {
    OPAQUE = 0
};
enum {
    ALIGN_X_LEFT	= (1L<<1),
    ALIGN_X_CENTER	= (1L<<2),
    ALIGN_X_RIGHT	= (1L<<3),
    ALIGN_Y_TOP	    = (1L<<4),
    ALIGN_Y_CENTER	= (1L<<5),
    ALIGN_Y_BOTTOM	= (1L<<6)
};

typedef struct {
    SDL_Surface *pic;
    int         align;
    int         color;
    int         height;
    char        char_width[256];
    int         char_offset[256];
    char        keys[256];
    char        offset;
    char        length;
    //last written rect
    int         save_last;
    int     	last_x;
    int         last_y;
    int	        last_width;
    int	        last_height;
} Font;
Font* load_font(char *fname);
Font* load_fixed_font(char *fname, int off, int len, int w);
void free_font(Font **sfnt);
int  write_text(Font *sfnt, SDL_Surface *dest, int x, int y, char *str, int alpha);
inline void lock_font(Font *sfnt);
inline void unlock_font(Font *sfnt);
SDL_Rect last_write_rect(Font *fnt);
int  text_width(Font *fnt, char *str);

/* mouse buttons */
enum {
    LEFT_BUTTON = 1,
    MIDDLE_BUTTON = 2,
    RIGHT_BUTTON = 3,
    WHEEL_UP = 4,
    WHEEL_DOWN = 5,
    BUTTON_COUNT
};

/* video modes */
typedef struct {
    int id;
    char name[64];
    int width, height, depth;
    int flags;
    int ok;
} Video_Mode;
/* Sdl */
enum {
    RECT_LIMIT = 200,
    FADE_DEF_TIME = 500,
    FADE_IN = 0,
    FADE_OUT = 1
};
typedef struct {
    SDL_Surface *screen;
    DrawRgn     d, s;
    int         rect_count;
    SDL_Rect    rect[RECT_LIMIT];
    int         fade;
} Sdl;
void init_sdl( int f );
void quit_sdl();
Video_Mode def_video_mode();
Video_Mode std_video_mode( int id );
Video_Mode video_mode( int width, int height, int depth, int flags );
Video_Mode* cur_video_mode();
char** get_mode_names( int *count );
int  set_video_mode( Video_Mode mode );
void hardware_cap();
inline void refresh_screen( int x, int y, int w, int h );
void refresh_rects();
void add_refresh_rect(int x, int y, int w, int h);
int  wait_for_key();
void wait_for_click();
inline void lock_screen();
inline void unlock_screen();
inline void flip_screen();
void fade_screen( int type, int ms );
void take_screenshot( int i );

/* cursor */
/* creates cursor */
SDL_Cursor* create_cursor( int width, int height, int hot_x, int hot_y, char *source );

/* timer */
inline int get_time();
inline void reset_timer();

#ifdef __cplusplus
};
#endif

#endif

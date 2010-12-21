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

#include <SDL/SDL.h>
#include <SDL_mixer.h>

#ifdef __cplusplus
extern "C" {
#endif

// draw region //
#define D_SRC(p, i, j) {sdl.s.s = p; sdl.s.r.x = i; sdl.s.r.y = j; sdl.s.r.w = sdl.d.r.w; sdl.s.r.h = sdl.d.r.h;}
#define D_DST(p, i, j, k, l) {sdl.d.s = p; sdl.d.r.x = i; sdl.d.r.y = j; sdl.d.r.w = k; sdl.d.r.h = l;}
#define D_FSRC(p) {sdl.s.s = p; sdl.s.r.x = 0; sdl.s.r.y = 0; sdl.s.r.w = sdl.d.r.w; sdl.s.r.h = sdl.d.r.h;}
#define D_FDST(p) {sdl.d.s = p; sdl.d.r.x = 0; sdl.d.r.y = 0; sdl.d.r.w = (p)->w; sdl.d.r.h = (p)->h;}
typedef struct {
    SDL_Surface *s;
    SDL_Rect    r;
} DrawRgn;

// Sdl Surface //
#define SDL_NONFATAL 0x10000000
SDL_Surface* SS_Ld(char *fname, int f);
SDL_Surface* SS_Crt(int w, int h, int f);
int  SS_DpyFmt(SDL_Surface *sur);
void SS_Bgn(SDL_Surface *sur);
void SS_End(SDL_Surface *sur);
void SS_Blt(void);
void SS_ABlt(int alpha);
void SS_Fill(int c);
Uint32 get_pixel( SDL_Surface *surf, int x, int y );

// Sdl Font //
#define TA_X_L	(1L<<1)
#define TA_X_C	(1L<<2)
#define TA_X_R	(1L<<3)
#define TA_Y_T	(1L<<4)
#define TA_Y_C	(1L<<5)
#define TA_Y_B	(1L<<6)
typedef struct {
    SDL_Surface *ss;
    int         algn;
    int         clr;
    int         h;
    char        lw[256];
    int         loff[256];
    char        keys[256];
    char        off;
    char        len;
    //last written rect
    int     	lX;
    int         lY;
    int	        lW;
    int	        lH;
} SFnt;
SFnt* SF_Ld(char *fname);
SFnt* SF_LdFxd(char *fname, int off, int len, int w);
void SF_Fr(SFnt *sfnt);
int  SF_Wrt(SFnt *sfnt, SDL_Surface *dest, int x, int y, char *str, int alpha);
void SF_Bgn(SFnt *sfnt);
void SF_End(SFnt *sfnt);
SDL_Rect SF_LstR(SFnt *fnt);
int  SF_TxtW(SFnt *fnt, char *str);

// Sdl //
#define SDL_MAX_RGNS    200
#define SDL_DIM_STEPS   8
#define SDL_DIM_DELAY   20
#define SDL_DIM()   Sdl_Dim(SDL_DIM_STEPS, SDL_DIM_DELAY, 255)
#define SDL_UNDIM() Sdl_UnDim(SDL_DIM_STEPS, SDL_DIM_DELAY, 255)
typedef struct {
    SDL_Surface *scr;
    DrawRgn     d, s;
    int         rnum;
    SDL_Rect    rgns[SDL_MAX_RGNS];
} Sdl;
void Sdl_Ini(int f);
void Sdl_Qut();
int  Sdl_StVdMd(int w, int h, int d, int f);
void Sdl_HwCps();
void Sdl_Upd(int x, int y, int w, int h);
void Sdl_FUpd();
void Sdl_UpdR();
void Sdl_AddR(int x, int y, int w, int h);
void Sdl_Dim(int steps, int delay, int trp);
void Sdl_UnDim(int steps, int delay, int trp);
int  Sdl_WtFrKy();
void Sdl_WtFrClk();
// hardware funcs //
void Sdl_Bgn();
void Sdl_End();
void Sdl_Flp();

#ifdef __cplusplus
};
#endif

#endif

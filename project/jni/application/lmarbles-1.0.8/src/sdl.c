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

#include "sdl.h"
#include <stdlib.h>
#include <string.h>

extern int  trm_gm;

Sdl sdl;

// sdl surface //

/*
    load a surface from file putting it in soft or hardware mem
*/
SDL_Surface* SS_Ld(char *fname, int f)
{
    SDL_Surface *buf;
    SDL_Surface *new_sur;
    char path[strlen(SRC_DIR)+ strlen(fname) + 2];
    sprintf(path, "%s/%s", SRC_DIR, fname);
    buf = SDL_LoadBMP(path);
    if (buf == 0) {
        fprintf(stderr, "ERR: ssur_load: file '%s' not found or not enough memory\n", path);
        if (f & SDL_NONFATAL)
            return 0;
        else
            exit(1);
    }
    new_sur = SS_Crt(buf->w, buf->h, f);
    SDL_BlitSurface(buf, 0, new_sur, 0);
    SDL_FreeSurface(buf);
    return new_sur;
}

/*
    create an surface
    MUST NOT BE USED IF NO SDLSCREEN IS SET
*/
SDL_Surface* SS_Crt(int w, int h, int f)
{
    SDL_Surface *sur;
    SDL_PixelFormat *spf = SDL_GetVideoSurface()->format;
    if ((sur = SDL_CreateRGBSurface(f, w, h, spf->BitsPerPixel, spf->Rmask, spf->Gmask, spf->Bmask, spf->Amask)) == 0) {
        fprintf(stderr, "ERR: ssur_create: not enough memory to create surface...\n");
        exit(1);
    }
    SDL_SetColorKey(sur, SDL_SRCCOLORKEY, SDL_MapRGB(sur->format,0,0,0));
    SDL_SetAlpha(sur, 0, 0); // no alpha //
    return sur;
}

/*
    return display format
*/
int SS_DpyFmt(SDL_Surface *sur)
{
    if ((sur = SDL_DisplayFormat(sur)) == 0) {
        fprintf(stderr, "ERR: ssur_displayformat: convertion failed\n");
        return 1;
    }
    return 0;
}

/*
    lock surface
*/
void SS_Bgn(SDL_Surface *sur)
{
    if (SDL_MUSTLOCK(sur))
        SDL_LockSurface(sur);
}

/*
    unlock surface
*/
void SS_End(SDL_Surface *sur)
{
    if (SDL_MUSTLOCK(sur))
        SDL_UnlockSurface(sur);
}

/*
    blit surface with destination D_DST and source D_SRC using it's actual alpha and color key settings
*/
void SS_Blt(void)
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
void SS_ABlt(int alpha)
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
void SS_Fill(int c)
{
    SDL_FillRect(sdl.d.s, &sdl.d.r, SDL_MapRGB(sdl.d.s->format, c >> 16, (c >> 8) & 0xFF, c & 0xFF));
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

// sdl font //

/*
    load a font using the width values in the file
*/
SFnt* SF_Ld(char *fname)
{
    SFnt    *fnt = 0;
    FILE    *file = 0;
    char    path[strlen(SRC_DIR)+ strlen(fname) + 2];
    int     i;

    sprintf(path, "%s/%s", SRC_DIR, fname);

    fnt = malloc(sizeof(SFnt));
    if (fnt == 0) {
        fprintf(stderr, "ERR: sfnt_load: not enough memory\n");
        exit(1);
    }

    if ((fnt->ss = SS_Ld(fname, SDL_SWSURFACE)) == 0)
        exit(1);
		
    fnt->algn = TA_X_L | TA_Y_T;
    fnt->clr = 0x00FFFFFF;
    fnt->h = fnt->ss->h;
	
    //table
    file = fopen(path, "r");
    fseek(file, -1, SEEK_END);
    fread(&fnt->off, 1, 1, file);
#ifdef DEBUG
    printf("offset: %i\n", fnt->off);
#endif
    fseek(file, -2, SEEK_END);
    fread(&fnt->len, 1, 1, file);
#ifdef DEBUG
    printf("number: %i\n", fnt->len);
#endif
    fseek(file, -2 - fnt->len, SEEK_END);
    fread(fnt->lw, 1, fnt->len, file);
#ifdef DEBUG
    printf("letter width: %i\n", fnt->len);
    for (i = 0; i < fnt->len; i++)
        printf("%i ", fnt->lw[i]);
    printf("\n");
#endif
    fclose(file);

    //letter offsets
    fnt->loff[0] = 0;
    for (i = 1; i < fnt->len; i++)	
        fnt->loff[i] = fnt->loff[i - 1] + fnt->lw[i - 1];
	
    //allowed keys
    memset(fnt->keys, 0, 256);
    for (i = 0; i < fnt->len; i++) {
        fnt->keys[i + fnt->off] = 1;
    }
	
    fnt->lX = fnt->lY = fnt->lW = fnt->lH = 0;
    return fnt;
}

/*
    load a font with fixed size
*/
SFnt* SF_LdFxd(char *f, int off, int len, int w)
{
    int     i;
    SFnt    *fnt;
    char    path[strlen(SRC_DIR)+ strlen(f) + 2];

    sprintf(path, "%s/%s", SRC_DIR, f);

    fnt = malloc(sizeof(SFnt));
    if (fnt == 0) {
        fprintf(stderr, "ERR: sfnt_load: not enough memory\n");
        exit(1);
    }

    if ((fnt->ss = SS_Ld(f, SDL_SWSURFACE)) == 0)
        exit(1);
		
    fnt->algn = TA_X_L | TA_Y_T;
    fnt->clr = 0x00FFFFFF;
    fnt->h = fnt->ss->h;
	
	fnt->off = off;
	fnt->len = len;
	
	for (i = 0; i < len; i++)
	    fnt->lw[i] = w;
	
    //letter offsets
    fnt->loff[0] = 0;
    for (i = 1; i < fnt->len; i++)	
        fnt->loff[i] = fnt->loff[i - 1] + w;
	
    //allowed keys
    memset(fnt->keys, 0, 256);
    for (i = 0; i < fnt->len; i++) {
        fnt->keys[i + fnt->off] = 1;
    }
	
    fnt->lX = fnt->lY = fnt->lW = fnt->lH = 0;
    return fnt;
}

/*
    free memory
*/
void SF_Fr(SFnt *fnt)
{
    if (fnt->ss) SDL_FreeSurface(fnt->ss);
    free(fnt);
}

/*
    write something with transparency
*/
int SF_Wrt(SFnt *fnt, SDL_Surface *dest, int x, int y, char *str, int alpha)
{
    int	c_abs;
    int len = strlen(str);
    int pix_len = 0;
    int px = x, py = y;
    int i;
    SDL_Surface *spf = SDL_GetVideoSurface();
	
    pix_len = SF_TxtW(fnt, str);
	for (i = 0; i < len; i++)
	    if (!fnt->keys[(int)str[i]])
	        str[i] = ' ';

    //alignment
    if (fnt->algn & TA_X_C)
        px -= pix_len >> 1;
    else
        if (fnt->algn & TA_X_R)
            px -= pix_len;
    if (fnt->algn & TA_Y_C)
        py -= fnt->h >> 1;
    else
        if (fnt->algn & TA_Y_B)
            py -= fnt->h;

    fnt->lX = px; if (fnt->lX < 0) fnt->lX = 0;
    fnt->lY = py; if (fnt->lY < 0) fnt->lY = 0;
    fnt->lW = pix_len; if (fnt->lX + fnt->lW >= spf->w) fnt->lW = spf->w - fnt->lX;
    fnt->lH = fnt->h; if (fnt->lY + fnt->lH >= spf->h) fnt->lH = spf->h - fnt->lY;

    if (alpha != 0)
        SDL_SetAlpha(fnt->ss, SDL_SRCALPHA, alpha);
    else
        SDL_SetAlpha(fnt->ss, 0, 0);
    for (i = 0; i < len; i++) {
       	c_abs = str[i] - fnt->off;
       	D_DST(dest, px, py, fnt->lw[c_abs], fnt->h);
       	D_SRC(fnt->ss, fnt->loff[c_abs], 0);
       	SS_Blt();
        px += fnt->lw[c_abs];
    }
	
    return 0;
}

/*
    lock font surface
*/
void SF_Bgn(SFnt *fnt)
{
    SS_Bgn(fnt->ss);
}

/*
    unlock font surface
*/
void SF_End(SFnt *fnt)
{
    SS_End(fnt->ss);
}
	
/*
    return last update region
*/
SDL_Rect SF_LstR(SFnt *fnt)
{
    SDL_Rect    rect={fnt->lX, fnt->lY, fnt->lW, fnt->lH};
    return rect;
}

/*
    return the text width in pixels
*/
int SF_TxtW(SFnt *fnt, char *str)
{
    unsigned int i;
    int pix_len = 0;
    for (i = 0; i < strlen(str); i++)
        pix_len += fnt->lw[str[i] - fnt->off];
    return pix_len;
}

// sdl //

/*
    initialize sdl
*/
void Sdl_Ini(int f)
{
    sdl.scr = 0;
    if (SDL_Init(f) < 0) {
        fprintf(stderr, "ERR: sdl_init: %s", SDL_GetError());
        exit(1);
    }
    SDL_EnableUNICODE(1);
    atexit(SDL_Quit);
}

/*
    free screen
*/
void Sdl_Qut()
{
    if (sdl.scr) SDL_FreeSurface(sdl.scr);
}

/*
    set video mode and give information about hardware capabilities
*/
int	Sdl_StVdMd(int w, int h, int d, int f)
{
    int depth;

#ifdef DEBUG
    SDL_PixelFormat	*fmt;
#endif
    if (sdl.scr) { 
        SDL_FreeSurface(sdl.scr);
    }
    else { /* load window icon on first time setup */
        SDL_Surface *icon;
        if ((icon = SDL_LoadBMP(SRC_DIR "/gfx/win_icon.bmp")) != NULL) {
	   SDL_SetColorKey(icon, SDL_SRCCOLORKEY, SDL_MapRGB(icon->format,255,255,255));
           SDL_WM_SetIcon(icon, NULL);
	}
    }
    
    // is this mode is supported
    depth = SDL_VideoModeOK(w, h, d, f);
    if ( depth == 0 ) {
        fprintf(stderr, "ERR: SDL_VideoModeOK says mode %ix%ix%i is invalid...\ntrying to emulate with 16 bits depth\n", w, h, d);
        depth = 16;
    }

    // set video mode
    if ((sdl.scr = SDL_SetVideoMode(w, h, depth, f)) == 0) {
        fprintf(stderr, "ERR: sdl_setvideomode: %s", SDL_GetError());
        return 1;
    }

#ifdef DEBUG				
    if (f & SDL_HWSURFACE && !(sdl.scr->flags & SDL_HWSURFACE))
       	fprintf(stderr, "unable to create screen in hardware memory...\n");
    if (f & SDL_DOUBLEBUF && !(sdl.scr->flags & SDL_DOUBLEBUF))
        fprintf(stderr, "unable to create double buffered screen...\n");
    if (f & SDL_FULLSCREEN && !(sdl.scr->flags & SDL_FULLSCREEN))
        fprintf(stderr, "unable to switch to fullscreen...\n");

    fmt = sdl.scr->format;
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
void Sdl_HwCps()
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
    update just one rect
*/
void Sdl_Upd(int x, int y, int w, int h)
{
    SDL_UpdateRect(sdl.scr, x, y, w, h);
}

/*
    do a full update
*/
void Sdl_FUpd()
{
    SDL_UpdateRect(sdl.scr, 0, 0, 0 ,0);
}

/*
    draw all update regions
*/
void Sdl_UpdR()
{
    if (sdl.rnum == SDL_MAX_RGNS)
        SDL_UpdateRect(sdl.scr, 0, 0, sdl.scr->w, sdl.scr->h);
    else
        SDL_UpdateRects(sdl.scr, sdl.rnum, sdl.rgns);
    sdl.rnum = 0;
}

/*
    add update region
*/
void Sdl_AddR(int x, int y, int w, int h)
{
    if (sdl.rnum == SDL_MAX_RGNS) return;
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x + w > sdl.scr->w)
        w = sdl.scr->w - x;
    if (y + h > sdl.scr->h)
        h = sdl.scr->h - y;
    if (w <= 0 || h <= 0)
        return;
    sdl.rgns[sdl.rnum].x = x;
    sdl.rgns[sdl.rnum].y = y;
    sdl.rgns[sdl.rnum].w = w;
    sdl.rgns[sdl.rnum].h = h;
    sdl.rnum++;
}

/*
    fade screen to black
*/
void Sdl_Dim(int steps, int delay, int trp)
{
#ifndef NODIM
    SDL_Surface    *buffer;
    int per_step = trp / steps;
    int i;
    if (trm_gm) return;
    buffer = SS_Crt(sdl.scr->w, sdl.scr->h, SDL_SWSURFACE);
    SDL_SetColorKey(buffer, 0, 0);
    D_FDST(buffer);
    D_FSRC(sdl.scr);
    SS_Blt();
    for (i = 0; i <= trp; i += per_step) {
        D_FDST(sdl.scr);
        SS_Fill(0x0);
        D_FSRC(buffer);
        SS_ABlt(i);
        Sdl_FUpd();
        SDL_Delay(delay);
    }
    if (trp == 255) {
        D_FDST(sdl.scr);
        SS_Fill(0x0);
        Sdl_FUpd();
    }
    SDL_FreeSurface(buffer);
#else
    Sdl_FUpd();
#endif
}

/*
    undim screen
*/
void Sdl_UnDim(int steps, int delay, int trp)
{
#ifndef NODIM
    SDL_Surface    *buffer;
    int per_step = trp / steps;
    int i;
    if (trm_gm) return;
    buffer = SS_Crt(sdl.scr->w, sdl.scr->h, SDL_SWSURFACE);
    SDL_SetColorKey(buffer, 0, 0);
    D_FDST(buffer);
    D_FSRC(sdl.scr);
    SS_Blt();
    for (i = trp; i >= 0; i -= per_step) {
        D_FDST(sdl.scr);
        SS_Fill(0x0);
        D_FSRC(buffer);
        SS_ABlt(i);
        Sdl_FUpd();
        SDL_Delay(delay);
    }
    D_FDST(sdl.scr);
    D_FSRC(buffer);
    SS_Blt();
    Sdl_FUpd();
    SDL_FreeSurface(buffer);
#else
    Sdl_FUpd();
#endif
}

/*
    wait for a key
*/
int Sdl_WtFrKy()
{
    //wait for key
    SDL_Event event;
    while (1) {
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) {
            trm_gm = 1;
            return 0;
        }
        if (event.type == SDL_KEYUP)
            return event.key.keysym.sym;
    }
}

/*
    wait for a key or mouse click
*/
void Sdl_WtFrClk()
{
    //wait for key or button
    SDL_Event event;
    while (1) {
        SDL_WaitEvent(&event);
        if (event.type == SDL_QUIT) {
            trm_gm = 1;
            return;
        }
        if (event.type == SDL_KEYUP || event.type == SDL_MOUSEBUTTONUP)
            return;
    }
}

/*
    lock surface
*/
void Sdl_Bgn()
{
    if (SDL_MUSTLOCK(sdl.scr))
        SDL_LockSurface(sdl.scr);
}

/*
    unlock surface
*/
void Sdl_End()
{
    if (SDL_MUSTLOCK(sdl.scr))
        SDL_UnlockSurface(sdl.scr);
}

/*
    flip hardware screens (double buffer)
*/
void Sdl_Flp()
{
    SDL_Flip(sdl.scr);
}

/***************************************************************************
                          gfx.h  -  description
                             -------------------
    begin                : Wed Mar 1 2000
    copyright            : (C) 2000 by Michael Speck
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef GFX_H
#define GFX_H

/*  SDL's graphical hardware support does not work on
    my computer and so I'm using self-written assembler
    functions because SDL's software emulation is _much_
    too slow...
*/

extern unsigned char rrShft;
extern unsigned char grShft;
extern unsigned char brShft;
extern unsigned char rlShft;
extern unsigned char glShft;
extern unsigned char blShft;
extern unsigned int  rMask;
extern unsigned int  gMask;
extern unsigned int  bMask;
extern unsigned char bpp;
extern unsigned char pixelsize;
extern unsigned int  clp_lx;
extern unsigned int  clp_ty;
extern unsigned int  clp_rx;
extern unsigned int  clp_by;

extern "C" {

void SD_Pnt(void* d, int dw, int x, int y, int rgb);
void SD_Box(void* d, int dw, int x, int y, int w, int h, int rgb);
void SD_Rct(void* d, int dw, int x, int y, int w, int h, int rgb);
void SD_CBx(void* d, int dw, int x, int y, int w, int h, int rgb, int pct);
void SC_Opq(void* d, int dw, int dx, int dy, void* s, int sw, int sx, int sy, int w, int h);
void SC_Clp(void* d, int dw, int dx, int dy, void* s, int sw, int sx, int sy, int w, int h);
void SC_Trp_11(void* d, int dw, int dx, int dy, void* s, int sw, int sx, int sy, int w, int h);
void SC_Trp_X(void* d, int dw, int dx, int dy, void* s, int sw, int sx, int sy, int w, int h, int pct);
void SC_Clr_11(void* d, int dw, int dx, int dy, void* src, int sw, int sx, int sy, int w,  int h, int clr);
void SC_Clr_X(void* d, int dw, int dx, int dy, void* s, int sw, int sx, int sy, int w,  int h, int clr, int pct);
void Gfx_SetClipRgn(int x, int y, int w, int h);

};

#define RGB(r, g, b) ( (r<<16) | (g<<8) | (b) )

#endif

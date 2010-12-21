/***************************************************************************
                          menu.h  -  description
                             -------------------
    begin                : Sat Aug 5 2000
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

#ifndef MENU_H
#define MENU_H

#include "sdl.h"
#include "dynlist.h"
#include "audio.h"

// menu entry //
// entry types //
#define ME_SEP  0
#define ME_SUB  1
#define ME_SWT  2
#define ME_RNG  3
#define ME_ACT  4
#define ME_KEY  5
#define ME_STR  6
typedef struct {
    char    nm[32];
    char    str[64];
    DLst    sw;
    int	    t;
    int	    *p;
    int	    min;
    int	    max;
    int     stp;
    int	    act;
    void    *smn;
    int     x, y;
    int	    dx;
    int     dy;
    int	    dw;
    int	    dh;
    float   a;
    void    (*cb)(void);
} MEnt;
void ME_Fr(void *e);
void ME_Ini(MEnt *e, int t, char *n);
MEnt* ME_CrtSep(char *s);
MEnt* ME_CrtSw2(char *n, int *p, char *cap1, char *cap2);
MEnt* ME_CrtSwX(char *n, int *p, char **caps, int num);
MEnt* ME_CrtRng(char *n, int *p, int min, int max, int stp);
MEnt* ME_CrtAct(char *n, int a_id);
MEnt* ME_CrtStr(char *n, char *p, int m);
MEnt* ME_CrtSub(char *n, void *m);
MEnt* ME_CrtKey(char *n, int *p);
void ME_SUpd(MEnt *e);
void ME_Edt(MEnt *e, int c, int u);
void ME_CngSwX(MEnt *e, int *p, char **caps, int num);

// menu //
typedef struct {
    DLst        ent;
    MEnt        *c_e;
} Menu;
Menu* M_Crt();
void M_Fr(void *m);
void M_Add(Menu *m, MEnt *e);
void M_Hd(Menu *m);
void M_Shw(Menu *m);
void M_SUpd(Menu *m);
void M_CmA(Menu *m, int ms);

// menu manager //
// resize or keep resolution ? //
#define MM_KP   0
#define MM_RSZ  1
// return values for MM_Evt(..) //
#define MM_TRMGM    -1
#define MM_NONE     0
// increase or decrease a value when use entry ? //
#define MM_INC      0
#define MM_DEC      1
typedef struct {
    SDL_Surface *ss_bkgd;
    SDL_Surface *ss_logo;
    SFnt        *ft_nml;
    SFnt        *ft_sel;
    int         x, y; // centre of menu in ss_bkgnd //
    int         lx, ly; // position of logo //
    float       a_c; // alpha change per ms used for enlightning //
    int         d; // delay per loop //
    DLst        mn;
    Menu        *c_mn;
    char        vkys[SDLK_LAST];
#ifdef SOUND
    Sound_Chunk *s_clk;
#endif
} MMng;
void MM_Ini(int x, int y, int ly, SDL_Surface *ss_bk, SDL_Surface *ss_lg, SFnt *fn, SFnt *fs);
void MM_Shw(int rsz);
void MM_Trm();
void MM_Add(Menu *m);
void MM_AdjP();
void MM_DfVKys();
void MM_Ck();
int MM_Evt(SDL_Event *e);
int MM_UseE(int c);
int MM_PrvE();
int MM_NxtE();
int MM_SelE(int x, int y);
void MM_CB();

#endif

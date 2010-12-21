/***************************************************************************
                          game.c  -  description
                             -------------------
    begin                : Mon Aug 14 2000
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

#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "cfg.h"
#include "menu.h"
#include "timer.h"

Game gm;
/* levelset names & levelsets -- levels.h */
extern char **ls_lst;
extern int  ls_n;
extern DLst l_sts;
/* line counter -- file.c */
extern int f_ln;
/* config -- cfg.c */
extern Cfg cfg;
/* Sdl -- sdl.c */
extern Sdl sdl;
/* profiles -- profile.c */
extern DLst prfs;
/* terminate game -- sdl.c */
extern int trm_gm;

void modify_score( int *b_lvl, int *b_tm );

/*
    initialize game
*/
void G_Ini()
{
    FILE *f;
    int i;
    char str[256];
    int ok = 0;
    // cursors //
    char data[32], mask[32];
    char csr[6][256] = {
        {
            0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 2, 1, 2, 2, 1, 2, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 2, 1, 2, 2, 1, 2, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 2, 1, 2, 1, 1, 2, 1, 2, 0, 0, 0, 0,
            0, 0, 0, 0, 2, 1, 2, 1, 1, 2, 1, 2, 0, 0, 0, 0,
            0, 0, 0, 2, 1, 2, 1, 1, 1, 1, 2, 1, 2, 0, 0, 0,
            0, 0, 0, 2, 1, 2, 1, 1, 1, 1, 2, 1, 2, 0, 0, 0,
            0, 0, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 2, 0, 0,
            0, 0, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 2, 0, 0,
            0, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 0,
            0, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 0,
            2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2,
            2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        },
        {
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
            2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2,
            0, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 0,
            0, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 0,
            0, 0, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 2, 0, 0,
            0, 0, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 2, 0, 0,
            0, 0, 0, 2, 1, 2, 1, 1, 1, 1, 2, 1, 2, 0, 0, 0,
            0, 0, 0, 2, 1, 2, 1, 1, 1, 1, 2, 1, 2, 0, 0, 0,
            0, 0, 0, 0, 2, 1, 2, 1, 1, 2, 1, 2, 0, 0, 0, 0,
            0, 0, 0, 0, 2, 1, 2, 1, 1, 2, 1, 2, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 2, 1, 2, 2, 1, 2, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 2, 1, 2, 2, 1, 2, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0,
        },
        {
            2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            2, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            2, 1, 2, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            2, 1, 2, 2, 2, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0,
            2, 1, 2, 1, 1, 2, 2, 1, 1, 2, 2, 0, 0, 0, 0, 0,
            2, 1, 2, 1, 1, 1, 1, 2, 2, 1, 1, 2, 2, 0, 0, 0,
            2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 2, 2, 0,
            2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 2,
            2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 2,
            2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 2, 2, 0,
            2, 1, 2, 1, 1, 1, 1, 2, 2, 1, 1, 2, 2, 0, 0, 0,
            2, 1, 2, 1, 1, 2, 2, 1, 1, 2, 2, 0, 0, 0, 0, 0,
            2, 1, 2, 2, 2, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0,
            2, 1, 2, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            2, 1, 1, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        },
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 1, 2,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 1, 2, 1, 2,
            0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 1, 2, 2, 2, 1, 2,
            0, 0, 0, 0, 0, 2, 2, 1, 1, 2, 2, 1, 1, 2, 1, 2,
            0, 0, 0, 2, 2, 1, 1, 2, 2, 1, 1, 1, 1, 2, 1, 2,
            0, 2, 2, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 1, 2,
            2, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2,
            2, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2,
            0, 2, 2, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 1, 2,
            0, 0, 0, 2, 2, 1, 1, 2, 2, 1, 1, 1, 1, 2, 1, 2,
            0, 0, 0, 0, 0, 2, 2, 1, 1, 2, 2, 1, 1, 2, 1, 2,
            0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 1, 2, 2, 2, 1, 2,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 1, 2, 1, 2,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 1, 2,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2,
        },
        {
            2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2,
            2, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 2,
            2, 1, 1, 1, 1, 2, 0, 0, 0, 0, 2, 1, 1, 1, 1, 2,
            2, 1, 1, 1, 1, 1, 2, 0, 0, 2, 1, 1, 1, 1, 1, 2,
            2, 1, 1, 1, 1, 1, 2, 0, 0, 2, 1, 1, 1, 1, 2, 0,
            0, 2, 1, 1, 1, 1, 2, 0, 0, 2, 1, 1, 1, 2, 0, 0,
            0, 0, 2, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 0, 0, 0,
            0, 0, 2, 1, 1, 1, 2, 0, 0, 2, 1, 1, 1, 2, 0, 0,
            0, 2, 1, 1, 1, 1, 2, 0, 0, 2, 1, 1, 1, 1, 2, 0,
            2, 1, 1, 1, 1, 1, 2, 0, 0, 2, 1, 1, 1, 1, 1, 2,
            2, 1, 1, 1, 1, 2, 0, 0, 0, 0, 2, 1, 1, 1, 1, 2,
            2, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 2, 1, 1, 1, 2,
            2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2,
        },
        {
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
            2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2,
            0, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 2, 0,
            0, 2, 1, 1, 2, 2, 2, 0, 0, 2, 2, 1, 1, 1, 2, 0,
            0, 0, 2, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 2, 0, 0,
            0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0,
            0, 0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0,
            0, 0, 0, 0, 2, 2, 1, 1, 1, 1, 2, 2, 0, 0, 0, 0,
            0, 0, 0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0,
            0, 0, 2, 1, 2, 2, 1, 1, 1, 1, 2, 2, 1, 2, 0, 0,
            0, 2, 1, 2, 0, 2, 1, 1, 1, 1, 2, 0, 2, 1, 2, 0,
            0, 2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 2, 2, 1, 2, 0,
            2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 2,
            2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        }
    };

    // initialize dyn list l_sts
    DL_Ini(&l_sts);
    l_sts.flgs = DL_AUTODEL;
    l_sts.cb = L_DelSt;
    // load level sets
    printf("loading levelsets...\n");
    for (i = 0; i < ls_n; i++) {
        printf("%s... ", ls_lst[i]);
        sprintf(str, "%s/levels/%s", SRC_DIR, ls_lst[i]);
        f = fopen(str, "r");
        if (f != 0) {
            f_ln = 1;
            if (L_LdSt(f)) {
                ok = 1;
                strcpy(((LSet*)l_sts.tl.p->d)->nm, ls_lst[i]);
            }
            else
                L_AddInvSt(ls_lst[i]);
            fclose(f);
        }
        else {
            L_AddInvSt(ls_lst[i]);
            printf("WARNING: levelset not found...\n");
        }
    }

    // found something ? //
    if (!ok) {
        printf("ERROR: no valid level set found; need at least one level set...\n");
        exit(1);
    }

    /* clear gm struct */
    //memset( &gm, 0, sizeof( gm ) );
    
    // initialize gfx set
    DL_Ini(&gm.g_sts);
    gm.g_sts.flgs = DL_AUTODEL;
    gm.g_sts.cb = G_DelGSt;
    // load gfx sets
    G_LdGSts();

    // load board
    gm.s_brd = SS_Ld("gfx/board.bmp", SDL_SWSURFACE);
    SDL_SetColorKey(gm.s_brd, 0, 0);
    // load lights
    gm.s_lghts = SS_Ld("gfx/lights.bmp", SDL_SWSURFACE);
    // load marbles
    gm.s_mrb = SS_Ld("gfx/marbles.bmp", SDL_SWSURFACE);
    // load figure
    gm.s_fig = SS_Ld("gfx/figure.bmp", SDL_SWSURFACE);
    // load marble frame
    gm.s_mf = SS_Ld("gfx/m_frame.bmp", SDL_SWSURFACE);
    // load marble select frame
    gm.s_msf = SS_Ld("gfx/m_sframe.bmp", SDL_SWSURFACE);
    // background //
    gm.s_bkgd = 0;

    // load fonts
    gm.f_sml = SF_LdFxd("gfx/f_small.bmp", 32, 96, 8);
    gm.f_wht = SF_LdFxd("gfx/f_white.bmp", 32, 96, 10);
    gm.f_rd = SF_LdFxd("gfx/f_red.bmp", 32, 96, 10);

    // level
    gm.c_lvl = 0;

    // marble
    gm.m_v = 0.15;
    gm.m_a.p = 0;
    gm.m_a.f = 4;
    gm.m_a.c = 0.016 + 0.008;
    gm.m_a.w = gm.m_a.h = 32;
    gm.m_act = M_EMPTY;
    gm.m_o_x = gm.m_o_y = -1;
    gm.m_warp = 0;

    // marble select frame
    gm.msf_a.f = 4;
    gm.msf_a.w = gm.msf_a.h = 40;
    gm.msf_a.c = 0.02;
    gm.msf_a.p = 0;

    // marble frame
    gm.mf_a.f = 4;
    gm.mf_a.w = gm.mf_a.h = 40;
    gm.mf_a.c = 0.02;
    gm.mf_a.p = 0;

    // layout
    gm.b_x = 640 - 200;
    gm.f_x = 35;
    gm.f_y = 145;
    gm.i_x = 20;
    gm.i_y = 25;
    gm.t_x = 40;
    gm.t_y = 105;
    gm.s_x = 15;
    gm.s_y = 290;

    // geometry
    gm.t_w = gm.t_h = 32;
    gm.f_w = gm.f_h = 12;
    gm.f_fw = gm.f_fh = 130;
    gm.scr_w = 640;
    gm.scr_h = 480;
    gm.brd_w = 200;
    gm.s_w = 170;
    gm.s_h = 175;
    gm.c_off = 12;

    // cursors
    memset(data, 0, sizeof(data));
    gm.c_u = gm.c_d = gm.c_l = gm.c_r = gm.c_s = gm.c_w = 0;
    gm.c_n = SDL_GetCursor();
    Cr_Ld(csr[0], data, mask);
    gm.c_u = SDL_CreateCursor(data, mask, 16, 16, 8, 8);
    Cr_Ld(csr[1], data, mask);
    gm.c_d = SDL_CreateCursor(data, mask, 16, 16, 8, 8);
    Cr_Ld(csr[2], data, mask);
    gm.c_r = SDL_CreateCursor(data, mask, 16, 16, 8, 8);
    Cr_Ld(csr[3], data, mask);
    gm.c_l = SDL_CreateCursor(data, mask, 16, 16, 8, 8);
    Cr_Ld(csr[4], data, mask);
    gm.c_s = SDL_CreateCursor(data, mask, 16, 16, 8, 8);
    Cr_Ld(csr[5], data, mask);
    gm.c_w = SDL_CreateCursor(data, mask, 16, 16, 8, 8);

    // map animations
    gm.m_ani = 0;

    // oneway animation info
    gm.ma_ow_a.f = 4;
    gm.ma_ow_a.c = 0.008;
    gm.ma_ow_a.p = 0;
    gm.ma_ow_a.w = gm.t_w;
    gm.ma_ow_a.h = gm.t_h;

    // teleport information
    gm.ma_tlp_a.f = 4;
    gm.ma_tlp_a.c = 0.008;
    gm.ma_tlp_a.p = 0;
    gm.ma_tlp_a.w = gm.t_w;
    gm.ma_tlp_a.h = gm.t_h;
    gm.tlp_a = 32;

    // sounds
#ifdef SOUND
    gm.wv_tlp = sound_chunk_load("teleport.wav");
    gm.wv_sel = sound_chunk_load("select.wav");
    gm.wv_stp = sound_chunk_load("stop.wav");
    gm.wv_clk = sound_chunk_load("click.wav");
    gm.wv_exp = sound_chunk_load("explode.wav");
    gm.wv_alm = sound_chunk_load("alarm.wav");
    gm.wv_arw = sound_chunk_load("arrow.wav");
    gm.wv_scr = sound_chunk_load("score.wav");
#endif

    // shrapnells
    DL_Ini(&gm.shr);
    gm.shr.flgs = DL_AUTODEL;
    gm.shr.cb = Shr_Del;
    gm.shr_a_c = 0.1;

    // credits
    gm.cr_a_c = 0.1;
    gm.cr_tm = 3000;
    gm.cr_y = 10;

    // shnapshot
    gm.snap = 0;
}

/*
    terminate game
*/
void G_Trm()
{
    // release gfx
    if (gm.s_brd) SDL_FreeSurface(gm.s_brd);
    if (gm.s_lghts) SDL_FreeSurface(gm.s_lghts);
    if (gm.s_mrb) SDL_FreeSurface(gm.s_mrb);
    if (gm.s_fig) SDL_FreeSurface(gm.s_fig);
    if (gm.s_mf) SDL_FreeSurface(gm.s_mf);
    if (gm.s_msf) SDL_FreeSurface(gm.s_msf);
    // release fonts
    if (gm.f_sml) SF_Fr(gm.f_sml);
    if (gm.f_wht) SF_Fr(gm.f_wht);
    if (gm.f_rd) SF_Fr(gm.f_rd);
    // release dynlists
    DL_Clr(&gm.g_sts);
    DL_Clr(&l_sts);
    DL_Clr(&gm.shr);
    // cursors //
    if (gm.c_u) SDL_FreeCursor(gm.c_u);
    if (gm.c_d) SDL_FreeCursor(gm.c_d);
    if (gm.c_l) SDL_FreeCursor(gm.c_l);
    if (gm.c_r) SDL_FreeCursor(gm.c_r);
    if (gm.c_s) SDL_FreeCursor(gm.c_s);
    if (gm.c_w) SDL_FreeCursor(gm.c_w);
    // free map animations //
    if (gm.m_ani)
        free(gm.m_ani);
    // sounds
#ifdef SOUND
    if (gm.wv_tlp) sound_chunk_free(&gm.wv_tlp);
    if (gm.wv_sel) sound_chunk_free(&gm.wv_sel);
    if (gm.wv_stp) sound_chunk_free(&gm.wv_stp);
    if (gm.wv_clk) sound_chunk_free(&gm.wv_clk);
    if (gm.wv_exp) sound_chunk_free(&gm.wv_exp);
    if (gm.wv_alm) sound_chunk_free(&gm.wv_alm);
    if (gm.wv_arw) sound_chunk_free(&gm.wv_arw);
    if (gm.wv_scr) sound_chunk_free(&gm.wv_scr);
#endif
}

/*
    open a new game
*/
int G_Opn()
{
    Prf     *p;
    DL_E    *e;
    int     flgs = SDL_SWSURFACE;

    // get current level set
    gm.c_l_st = (LSet*)DL_Get(&l_sts, cfg.ls);

    // check if current level set is valid
    if (!G_CkLSt()) {
        // restore menu
        MM_Shw(MM_RSZ);

        return 0;
    }

    // current profile
    gm.c_prf = (Prf*)DL_Get(&prfs, cfg.prf);

    // current set info
    gm.c_s_inf = Prf_RegLS(gm.c_prf, gm.c_l_st);

    // dim & resize
    if (cfg.dim)
        SDL_DIM();
    if (cfg.fscr)
        flgs = flgs | SDL_FULLSCREEN;
    Sdl_StVdMd(gm.scr_w, gm.scr_h, 16, flgs);

    // create background
    gm.s_bkgd = SS_Crt(gm.scr_w, gm.scr_h, SDL_SWSURFACE);
    SDL_SetColorKey(gm.s_bkgd, 0, 0);

    // get highest score
    gm.hi_scr = 0;
    e = prfs.hd.n;
    while (e != &prfs.tl) {
        p = (Prf*)e->d;
        if (p->scr > gm.hi_scr)
            gm.hi_scr = p->scr;
        e = e->n;
    }

    // clear old chapter
    gm.o_ch = -1;

    // init first level
    L_Ini(0, 0);

    return 1;
}

/*
    close game
*/
void G_Cls()
{
    // dim
    if (!trm_gm && cfg.dim)
        SDL_DIM();

    // free background
    if (gm.s_bkgd) SDL_FreeSurface(gm.s_bkgd);

    // free level
    free(gm.c_lvl);
    gm.c_lvl = 0;

    // restore cursor
    SDL_SetCursor(gm.c_n);

    // show menu
    MM_Shw(MM_RSZ);
}

/*
    game's main loop
*/
void G_Run()
{
    int leave = 0;
    int restart = 0;
    int ms;
    SDL_Event ev;
    SDL_Surface *buf;
    int flgs;
    int restore_pos;
    int tm_rel = 0;
    int ign_c_stat = 0;
    int bonus_level, bonus_moves; /* bonus for level completion and remaining moves */
    
    while (!trm_gm) {
        // clear input
        while (SDL_PollEvent(&ev));
        // main loop
        while (!leave && !trm_gm && !gm.l_done && !restart) {
            // don't consume all cpu time
            SDL_Delay( 5 );
            
            restore_pos = 0; // do not restore old position
            ign_c_stat = 0; // do not ignore cursor state
            // get input
            if (SDL_PollEvent(&ev)) {
                switch (ev.type) {
                    case SDL_QUIT:
                        trm_gm = 1;
                        break;
                    case SDL_KEYUP:
                        switch (ev.key.keysym.sym) {
                            case SDLK_TAB:
                                SnapShot();
                                break;
                            case SDLK_ESCAPE:
                                if (G_CfmQut())
                                    leave = 1;
                                break;
                            case SDLK_p:
                                G_Ps();
                                break;
                            case SDLK_r:
                                if (G_CfmRst())
                                    restart = 1;
                                break;
                            case SDLK_f:
                                buf = SS_Crt(gm.scr_w, gm.scr_h, SDL_SWSURFACE);
                                D_FDST(buf);
                                D_FSRC(sdl.scr);
                                SS_Blt();
                                cfg.fscr = !cfg.fscr;
                                flgs = SDL_SWSURFACE;
                                if (cfg.fscr)
                                    flgs = flgs | SDL_FULLSCREEN;
                                Sdl_StVdMd(gm.scr_w, gm.scr_h, 16, flgs);
                                D_FDST(sdl.scr);
                                D_FSRC(buf);
                                SS_Blt();
                                Sdl_FUpd();
                                break;
                            default:
                                /* if no marble is selected we don't have to check anything */
                                if ( !gm.m_sel ) 
                                    break;
                                /* warp? */
                                if (gm.m_mv && (ev.key.keysym.sym == cfg.k_right || ev.key.keysym.sym == cfg.k_left || ev.key.keysym.sym == cfg.k_up || ev.key.keysym.sym == cfg.k_down)) {
                                    gm.m_warp = 1;
                                    break;
                                }
                                // undo key
                                if (ev.key.keysym.sym == cfg.k_undo) {
                                    restore_pos = 1;
#ifdef SOUND
                                    sound_play(gm.wv_clk);
#endif
                                }
                                // up key
                                if (ev.key.keysym.sym == cfg.k_up && (gm.m_vd & MD_U) && !gm.m_mv) {
                                    ign_c_stat = 1;
                                    gm.c_stat = C_U;
                                    Mr_IniMv();
#ifdef SOUND
                                    sound_play(gm.wv_clk);
#endif
                                }
                                // down key
                                if (ev.key.keysym.sym == cfg.k_down && (gm.m_vd & MD_D) && !gm.m_mv) {
                                    ign_c_stat = 1;
                                    gm.c_stat = C_D;
                                    Mr_IniMv();
#ifdef SOUND
                                    sound_play(gm.wv_clk);
#endif
                                }
                                // left key
                                if (ev.key.keysym.sym == cfg.k_left && (gm.m_vd & MD_L) && !gm.m_mv) {
                                    ign_c_stat = 1;
                                    gm.c_stat = C_L;
                                    Mr_IniMv();
#ifdef SOUND
                                    sound_play(gm.wv_clk);
#endif
                                }
                                // right key
                                if (ev.key.keysym.sym == cfg.k_right && (gm.m_vd & MD_R) && !gm.m_mv) {
                                    ign_c_stat = 1;
                                    gm.c_stat = C_R;
                                    Mr_IniMv();
#ifdef SOUND
                                    sound_play(gm.wv_clk);
#endif
                                }
                                break;
                        }
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                        break;
                    case SDL_MOUSEBUTTONUP:
                        if ( ev.button.button == 1 && gm.m_mv &&
                             ( gm.c_l_st->limit_type == MOVES ) ) {
                            gm.m_warp = 1;
                            break;
                        }
                        gm.bttn[ev.button.button] = 1;
                        if (ev.button.button == 1) {
                            if (gm.c_stat == C_SEL)
                                Mr_Sel(ev.button.x, ev.button.y);
                            else
                                if (gm.m_sel && !gm.m_mv) {
                                    // start movement of selected marble
                                    Mr_IniMv();
                                }
                        }
                        else
                            if (gm.m_sel && !gm.m_mv)
                                Mr_Rel(ev.button.x, ev.button.y);
                        break;
                    case SDL_MOUSEMOTION:
                        if ( !ign_c_stat )
                            Cr_Cng(ev.motion.x, ev.motion.y);
                        gm.o_mx = ev.motion.x;
                        gm.o_my = ev.motion.y;
                        break;
                    default:
                        break;
                }
            }

            // hide
            if (!leave && !restart) {
                Mr_Hd();
                MF_Hd();
                Tm_Hd();
                Inf_Hd();
                Shr_Hd();
                Cr_Hd();
            }

            // update
            ms = T_Gt();
            MA_Upd(ms);
            if (restore_pos)
                Mr_ResPos();
            if ( !Mr_Upd(ms) )
                restart = 1;
            MF_Upd(ms);
            Shr_Upd(ms);
            Cr_Upd(ms);
            if (!Tm_Upd(ms))
                restart = 1;
            if (Inf_Upd() && G_CfmWrp()) {
                gm.c_ch = gm.w_c;
                gm.c_l_id = gm.w_l;
                restart = 1;
            }

            // show
            if (!leave && !restart)
                MA_Shw();
            if (!leave && !restart) {
                Mr_Shw();
            }
            if (!leave && !restart && !gm.l_done) {
                MF_Shw();
                Tm_Shw();
                Inf_Shw();
                Shr_Shw();
                Cr_Shw();

                // refresh
                Sdl_UpdR();
            }
            else
                sdl.rnum = 0; // reset redraw regions

            // reset buttonstate
            memset(gm.bttn, 0, sizeof(gm.bttn));
        }

        // leave ?
        if (leave)
            break;

        // init next level
        if (gm.l_done || restart) {

            if (gm.l_done) {

                // figure animation
                FA_Run();
                // bonus summary
                if ( !gm.c_s_inf->cmp[gm.c_ch * gm.c_s_inf->l_num + gm.c_l_id] ) {

                    /* level wasn't completed until now so gain score for it */
                    bonus_level = LB_COMPLETED;
                    bonus_moves = gm.c_lvl->tm * LB_PER_MOVE;
                    modify_score( &bonus_level, &bonus_moves );
                    BS_Run( bonus_level, bonus_moves );
                    tm_rel = ( 1000 * gm.c_lvl->tm ) / gm.c_l_st->ch[gm.c_ch].lvls[gm.c_l_id].tm;
                    Prf_Upd(gm.c_prf, gm.c_s_inf, gm.c_ch * gm.c_l_st->l_num + gm.c_l_id, tm_rel, bonus_level + bonus_moves );

                }

            }

            if (cfg.dim)
                SDL_DIM();
            if (!restart)
                if (!L_FndNxt()) // game finished ?
                    break;
            L_Ini(gm.c_ch, gm.c_l_id);
            restart = 0;
        }
    }
    // save profiles
    Prf_Sv();
}

/*
    load all gfx sets
*/
void G_LdGSts()
{
    char    d_nm[256];
    char    path[256+64];
    DIR     *dir = 0;
    struct dirent  *e;
    struct stat     s;
    GSet    *g_st;

    printf("loading graphics sets...\n");

    // create directory string //
    sprintf(d_nm, "%s/gfx", SRC_DIR);

    // find and open directory //
    if ((dir = opendir(d_nm)) == 0) {
        fprintf(stderr, "ERROR: can't find directory '%s'\n", d_nm);
        exit(1);
    }

    // well, let's check for directories //
    while ((e = readdir(dir)) != 0) {
        sprintf(path, "%s/%s", d_nm, e->d_name);
        stat(path, &s);
        if (S_ISDIR(s.st_mode) && e->d_name[0] != '.') {
            printf("'%s'... \n", e->d_name);
            // load gfx //
            g_st = (GSet*)malloc(sizeof(GSet));
            memset(g_st, 0, sizeof(GSet));
            strcpy(g_st->nm, e->d_name);
            g_st->ok = 1;

            sprintf(path, "gfx/%s/%s", e->d_name, "background.bmp");
            g_st->s_bkgd = SS_Ld(path, SDL_SWSURFACE | SDL_NONFATAL);
            SDL_SetColorKey(g_st->s_bkgd, 0, 0);

            sprintf(path, "gfx/%s/%s", e->d_name, "wall.bmp");
            if ((g_st->s_wl = SS_Ld(path, SDL_SWSURFACE | SDL_NONFATAL)) == 0)
                g_st->ok = 0;

            sprintf(path, "gfx/%s/%s", e->d_name, "floor.bmp");
            if ((g_st->s_flr = SS_Ld(path, SDL_SWSURFACE | SDL_NONFATAL)) == 0)
                g_st->ok = 0;

            sprintf(path, "gfx/%s/%s", e->d_name, "arrow_r.bmp");
            if ((g_st->s_r_arw = SS_Ld(path, SDL_SWSURFACE | SDL_NONFATAL)) == 0)
                g_st->ok = 0;

            sprintf(path, "gfx/%s/%s", e->d_name, "arrow_l.bmp");
            if ((g_st->s_l_arw = SS_Ld(path, SDL_SWSURFACE | SDL_NONFATAL)) == 0)
                g_st->ok = 0;

            sprintf(path, "gfx/%s/%s", e->d_name, "arrow_u.bmp");
            if ((g_st->s_u_arw = SS_Ld(path, SDL_SWSURFACE | SDL_NONFATAL)) == 0)
                g_st->ok = 0;

            sprintf(path, "gfx/%s/%s", e->d_name, "arrow_d.bmp");
            if ((g_st->s_d_arw = SS_Ld(path, SDL_SWSURFACE | SDL_NONFATAL)) == 0)
                g_st->ok = 0;

            sprintf(path, "gfx/%s/%s", e->d_name, "barrier_lr.bmp");
            if ((g_st->s_lr_bar = SS_Ld(path, SDL_SWSURFACE | SDL_NONFATAL)) == 0)
                g_st->ok = 0;

            sprintf(path, "gfx/%s/%s", e->d_name, "barrier_ud.bmp");
            if ((g_st->s_ud_bar = SS_Ld(path, SDL_SWSURFACE | SDL_NONFATAL)) == 0)
                g_st->ok = 0;

            sprintf(path, "gfx/%s/%s", e->d_name, "teleport0.bmp");
            if ((g_st->s_tlp_0 = SS_Ld(path, SDL_SWSURFACE | SDL_NONFATAL)) == 0)
                g_st->ok = 0;

            sprintf(path, "gfx/%s/%s", e->d_name, "teleport1.bmp");
            if ((g_st->s_tlp_1 = SS_Ld(path, SDL_SWSURFACE | SDL_NONFATAL)) == 0)
                g_st->ok = 0;

            sprintf(path, "gfx/%s/%s", e->d_name, "teleport2.bmp");
            if ((g_st->s_tlp_2 = SS_Ld(path, SDL_SWSURFACE | SDL_NONFATAL)) == 0)
                g_st->ok = 0;

            sprintf(path, "gfx/%s/%s", e->d_name, "teleport3.bmp");
            if ((g_st->s_tlp_3 = SS_Ld(path, SDL_SWSURFACE | SDL_NONFATAL)) == 0)
                g_st->ok = 0;

            sprintf(path, "gfx/%s/%s", e->d_name, "wall_crumble.bmp");
            if ((g_st->s_crmbl = SS_Ld(path, SDL_SWSURFACE | SDL_NONFATAL)) == 0)
                g_st->ok = 0;

            if (g_st->ok) {
                printf("ok\n");
                // opaque
                SDL_SetColorKey(g_st->s_flr, 0, 0);
                SDL_SetColorKey(g_st->s_wl, 0, 0);
            }
            DL_Add(&gm.g_sts, g_st);
        }
    }
    closedir(dir);
}


/*
    delete a gset
*/
void G_DelGSt(void *p)
{
    GSet *st = (GSet*)p;
    if (st->s_bkgd) SDL_FreeSurface(st->s_bkgd);
    if (st->s_wl) SDL_FreeSurface(st->s_wl);
    if (st->s_flr) SDL_FreeSurface(st->s_flr);
    if (st->s_u_arw) SDL_FreeSurface(st->s_u_arw);
    if (st->s_d_arw) SDL_FreeSurface(st->s_d_arw);
    if (st->s_r_arw) SDL_FreeSurface(st->s_r_arw);
    if (st->s_l_arw) SDL_FreeSurface(st->s_l_arw);
    if (st->s_lr_bar) SDL_FreeSurface(st->s_lr_bar);
    if (st->s_ud_bar) SDL_FreeSurface(st->s_ud_bar);
    if (st->s_tlp_0) SDL_FreeSurface(st->s_tlp_0);
    if (st->s_tlp_1) SDL_FreeSurface(st->s_tlp_1);
    if (st->s_tlp_2) SDL_FreeSurface(st->s_tlp_2);
    if (st->s_tlp_3) SDL_FreeSurface(st->s_tlp_3);
    if (st->s_crmbl) SDL_FreeSurface(st->s_crmbl);
    free(st);
}

/*
    pause game
*/
void G_Ps()
{
    SFnt *ft = gm.f_sml;
    SDL_Surface *buf;
    char str[256];
    SDL_Event e;
    int leave = 0;
    int flgs;
    int mx = gm.o_mx, my = gm.o_my;

    // save screen //
    buf = SS_Crt(gm.scr_w, gm.scr_h, SDL_SWSURFACE);
    SDL_SetColorKey(buf, 0, 0);
    D_FDST(buf);
    D_FSRC(sdl.scr);
    SS_Blt();

    // cursor
    SDL_SetCursor(gm.c_n);

    // fill with black
    D_FDST(sdl.scr);
    SS_Fill(0x0);

    // write info
    ft->algn = TA_X_C | TA_Y_C;
    sprintf(str, "Game paused");
    SF_Wrt(ft, sdl.scr, gm.scr_w / 2, gm.scr_h / 2, str, 0);

    Sdl_FUpd();

    // wait for 'p'
    while (!leave) {
        SDL_WaitEvent(&e);
        switch (e.type) {
            case SDL_QUIT:
                trm_gm = 1;
                leave = 1;
                break;
            case SDL_KEYUP:
                if (e.key.keysym.sym == SDLK_p)
                    leave = 1;
                if (e.key.keysym.sym == SDLK_f) {
                    cfg.fscr = !cfg.fscr;
                    flgs = SDL_SWSURFACE;
                    if (cfg.fscr)
                        flgs = flgs | SDL_FULLSCREEN;
                    Sdl_StVdMd(gm.scr_w, gm.scr_h, 16, flgs);
                    D_FDST(sdl.scr);
                    SS_Fill(0x0);
                    SF_Wrt(ft, sdl.scr, gm.scr_w / 2, gm.scr_h / 2, str, 0);
                    Sdl_FUpd();
                }
                break;
            case SDL_MOUSEMOTION:
                mx = e.motion.x;
                my = e.motion.y;
                break;
        }
    }

    // restore screen
    D_FDST(sdl.scr);
    D_FSRC(buf);
    SS_Blt();
    Sdl_FUpd();
    SDL_FreeSurface(buf);

    // cursor
    Cr_Cng(mx, my);

    //reset time //
    T_Rst();
}

/*
    check if the figure has been completed
*/
void G_CkFgr()
{
    int i, j, k, l;
    Lvl *lvl = gm.c_lvl;

    for (i = 0; i < lvl->m_w - lvl->f_w; i++)
        for (j = 0; j < lvl->m_h - lvl->f_h; j++) {
            gm.l_done = 1;
            for (k = 0; k < lvl->f_w; k++) {
                for (l = 0; l < lvl->f_h; l++) {
                    if (lvl->fgr[k][l] != -1)
                        if (lvl->map[i + k][j + l].m != lvl->fgr[k][l]) {
                            gm.l_done = 0;
                            break;
                        }
                }
                if (!gm.l_done)
                    break;
            }
            if (gm.l_done)
                return;
        }
}

/*
    confirm warp
*/
int G_CfmWrp()
{
    SFnt *ft = gm.f_sml;
    SDL_Surface *buf;
    SDL_Event e;
    char str[256];
    int leave = 0, ret = 0;
    int mx = gm.o_mx, my = gm.o_my;

    // save screen //
    buf = SS_Crt(gm.scr_w, gm.scr_h, SDL_SWSURFACE);
    SDL_SetColorKey(buf, 0, 0);
    D_FDST(buf);
    D_FSRC(sdl.scr);
    SS_Blt();

    // cursor
    SDL_SetCursor(gm.c_n);

    // fill with black
    D_FDST(sdl.scr);
    SS_Fill(0x0);

    // write info
    ft->algn = TA_X_C | TA_Y_T;
    sprintf(str, "Your current position is level %i of chapter %i.", gm.c_l_id + 1, gm.c_ch + 1);
    SF_Wrt(ft, sdl.scr, gm.scr_w / 2, 200, str, 0);
    sprintf(str, "Do you really want to enter level %i of chapter %i?", gm.w_l + 1, gm.w_c + 1);
    SF_Wrt(ft, sdl.scr, gm.scr_w / 2, 200 + 15, str, 0);
    sprintf(str, "(All changes in the current level will be lost!)");
    SF_Wrt(ft, sdl.scr, gm.scr_w / 2, 200 + 30, str, 0);
    sprintf(str, "(press y/n)");
    SF_Wrt(ft, sdl.scr, gm.scr_w / 2, 200 + 50, str, 0);

    Sdl_FUpd();

#ifdef SOUND
    sound_play(gm.wv_clk);
#endif
    while (!leave) {
        SDL_WaitEvent(&e);
        switch (e.type) {
            case SDL_QUIT:
                trm_gm = leave = 1;
                break;
            case SDL_KEYUP:
                switch (e.key.keysym.sym) {
                    case SDLK_y:
                        ret = 1;
                        leave = 1;
                        break;
                    case SDLK_n:
                        ret = 0;
                        leave = 1;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                switch (e.button.button) {
                    case 1:
                        ret = 1;
                        leave = 1;
                        break;
                    default:
                        ret = 0;
                        leave = 1;
                        break;
                }
                break;
            case SDL_MOUSEMOTION:
                mx = e.motion.x;
                my = e.motion.y;
                break;
        }
    }
#ifdef SOUND
    sound_play(gm.wv_clk);
#endif

    // restore screen
    if (cfg.dim)
        SDL_DIM();
    if (!ret) {
        D_FDST(sdl.scr);
        D_FSRC(buf);
        SS_Blt();
        if (cfg.dim)
            SDL_UNDIM();
        else
            Sdl_FUpd();

        // cursor
        Cr_Cng(mx, my);

    }
    SDL_FreeSurface(buf);

    // reset time //
    T_Rst();

    return ret;
}

/*
    confirm quit
*/
int G_CfmQut()
{
    SFnt *ft = gm.f_sml;
    SDL_Surface *buf;
    SDL_Event e;
    char str[256];
    int leave = 0, ret = 0;
    int mx = gm.o_mx, my = gm.o_my;

    // save screen //
    buf = SS_Crt(gm.scr_w, gm.scr_h, SDL_SWSURFACE);
    SDL_SetColorKey(buf, 0, 0);
    D_FDST(buf);
    D_FSRC(sdl.scr);
    SS_Blt();

    // cursor
    SDL_SetCursor(gm.c_n);

    // fill with black
    D_FDST(sdl.scr);
    SS_Fill(0x0);

    // write info
    ft->algn = TA_X_C | TA_Y_C;
    sprintf(str, "Do you really want to quit? (y/n)");
    SF_Wrt(ft, sdl.scr, gm.scr_w / 2, gm.scr_h / 2, str, 0);

    Sdl_FUpd();

#ifdef SOUND
    sound_play(gm.wv_clk);
#endif
    while (!leave) {
        SDL_WaitEvent(&e);
        switch (e.type) {
            case SDL_QUIT:
                trm_gm = leave = 1;
                break;
            case SDL_KEYUP:
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                    case SDLK_y:
                        ret = 1;
                        leave = 1;
                        break;
                    case SDLK_n:
                        ret = 0;
                        leave = 1;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                switch (e.button.button) {
                    case 1:
                        ret = 1;
                        leave = 1;
                        break;
                    default:
                        ret = 0;
                        leave = 1;
                        break;
                }
                break;
        }
    }
#ifdef SOUND
    sound_play(gm.wv_clk);
#endif

    // restore screen
    if (!ret) {
        if (cfg.dim)
            SDL_DIM();
        D_FDST(sdl.scr);
        D_FSRC(buf);
        SS_Blt();
        if (cfg.dim)
            SDL_UNDIM();
        else
            Sdl_FUpd();

        // cursor
        Cr_Cng(mx, my);

    }
    SDL_FreeSurface(buf);

    // reset time //
    T_Rst();

    return ret;
}

/*
    confirm quit
*/
int G_CfmRst()
{
    SFnt *ft = gm.f_sml;
    SDL_Surface *buf;
    SDL_Event e;
    char str[256];
    int leave = 0, ret = 0;
    int mx = gm.o_mx, my = gm.o_my;

    // save screen //
    buf = SS_Crt(gm.scr_w, gm.scr_h, SDL_SWSURFACE);
    SDL_SetColorKey(buf, 0, 0);
    D_FDST(buf);
    D_FSRC(sdl.scr);
    SS_Blt();

    // cursor
    SDL_SetCursor(gm.c_n);

    // fill with black
    D_FDST(sdl.scr);
    SS_Fill(0x0);

    // write info
    ft->algn = TA_X_C | TA_Y_C;
    sprintf(str, "Do you really want to restart? (y/n)");
    SF_Wrt(ft, sdl.scr, gm.scr_w / 2, gm.scr_h / 2, str, 0);

    Sdl_FUpd();

#ifdef SOUND
    sound_play(gm.wv_clk);
#endif
    while (!leave) {
        SDL_WaitEvent(&e);
        switch (e.type) {
            case SDL_QUIT:
                trm_gm = leave = 1;
                break;
            case SDL_KEYUP:
                switch (e.key.keysym.sym) {
                    case SDLK_r:
                    case SDLK_y:
                        ret = 1;
                        leave = 1;
                        break;
                    case SDLK_n:
                        ret = 0;
                        leave = 1;
                        break;
                    default:
                        break;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                switch (e.button.button) {
                    case 1:
                        ret = 1;
                        leave = 1;
                        break;
                    default:
                        ret = 0;
                        leave = 1;
                        break;
                }
                break;
        }
    }
#ifdef SOUND
    sound_play(gm.wv_clk);
#endif

    // restore screen
    if (cfg.dim)
        SDL_DIM();
    if (!ret) {
        D_FDST(sdl.scr);
        D_FSRC(buf);
        SS_Blt();
        if (cfg.dim)
            SDL_UNDIM();
        else
            Sdl_FUpd();

        // cursor
        Cr_Cng(mx, my);

    }
    SDL_FreeSurface(buf);

    // reset time //
    T_Rst();

    return ret;
}

/*
    check if level set can be played and ask for confirmation if it got errors
*/
int G_CkLSt()
{
    char str[256];
    SFnt *ft = gm.f_sml;
    SDL_Event e;

    if (gm.c_l_st->ch == 0) {
        D_FDST(sdl.scr);
        SS_Fill(0x0);
        sprintf(str, "This level set cannot be played.\n");
        ft->algn = TA_X_C | TA_Y_C;
        SF_Wrt(ft, sdl.scr, sdl.scr->w / 2, sdl.scr->h / 2, str, 0);
        Sdl_FUpd();
        Sdl_WtFrClk();
        return 0;
    }
    if (!gm.c_l_st->ok) {
        D_FDST(sdl.scr);
        SS_Fill(0x0);
        sprintf(str, "This level set has errors. Play anyway? (y/n)\n");
        ft->algn = TA_X_C | TA_Y_C;
        SF_Wrt(ft, sdl.scr, sdl.scr->w / 2, sdl.scr->h / 2, str, 0);
        Sdl_FUpd();
        while (1) {
            if (SDL_WaitEvent(&e))
                switch (e.type) {
                    case SDL_KEYUP:
                        switch (e.key.keysym.sym) {
                            case SDLK_ESCAPE:
                            case SDLK_n:
                                return 0;
                            case SDLK_y:
                                return 1;
                            default:
                                break;
                        }
                    case SDL_MOUSEBUTTONUP:
                        switch (e.button.button) {
                            case 1:
                                return 1;
                            default:
                                return 0;
                                break;
                        }
                        break;
                }
        }
    }
    return 1;
}

// marble //
/*
    hide current marble
*/
void Mr_Hd()
{
    if (!gm.m_sel) return;
    D_DST(sdl.scr, (int)gm.m_x, (int)gm.m_y, gm.t_w, gm.t_h);
    D_SRC(gm.s_bkgd, (int)gm.m_x, (int)gm.m_y);
    SS_Blt();
    Sdl_AddR((int)gm.m_x, (int)gm.m_y, gm.t_w, gm.t_h);
}

/*
    show current marble
*/
void Mr_Shw()
{
    if (!gm.m_sel) return;
    D_DST(sdl.scr, (int)gm.m_x, (int)gm.m_y, gm.t_w, gm.t_h);
    D_SRC(gm.s_mrb, (int)gm.m_a.p * gm.m_a.w, gm.m_id * gm.t_h);
    SS_Blt();
    Sdl_AddR((int)gm.m_x, (int)gm.m_y, gm.t_w, gm.t_h);
}

/*
    update current marble
    return 0 if times out( moves )
*/
int Mr_Upd(int ms)
{
    float c; // change
    int stp = 0; // marble stopped ?

    if (!gm.m_sel || !gm.m_mv) return 1;

    // marble animation (frame)
    switch (gm.m_d) {
        case 1:
        case 2:
            gm.m_a.p += gm.m_a.c * ms;
            if (gm.m_a.p >= gm.m_a.f)
                gm.m_a.p = 0;
            break;
        case 0:
        case 3:
            gm.m_a.p -= gm.m_a.c * ms;
            if (gm.m_a.p <= 0)
                gm.m_a.p = gm.m_a.f;
            break;
    }

    c = ms * gm.m_v;
    switch (gm.m_d) {
        case 0:
            gm.m_y -= c;
            if (gm.m_y <= gm.m_ty) {
                Mr_Stp();
                stp = 1;
            }
            break;
        case 1:
            gm.m_x += c;
            if (gm.m_x >= gm.m_tx) {
                Mr_Stp();
                stp = 1;
            }
            break;
        case 2:
            gm.m_y += c;
            if (gm.m_y >= gm.m_ty) {
                Mr_Stp();
                stp = 1;
            }
            break;
        case 3:
            gm.m_x -= c;
            if (gm.m_x <= gm.m_tx) {
                Mr_Stp();
                stp = 1;
            }
            break;
    }

    // warp???
    if ( gm.m_warp ) {
        gm.m_x = gm.m_tx; gm.m_y = gm.m_ty;
        Mr_Stp();
        stp = 1;
        gm.m_warp = 0;
    }

    // check time if move limit //
    if ( stp && gm.c_l_st->limit_type == MOVES &&
         gm.m_act != M_TLP_0 && gm.m_act != M_TLP_1 &&
         gm.m_act != M_TLP_2 && gm.m_act != M_TLP_3) {

        gm.c_lvl->tm--;
        if ( gm.c_lvl->tm <= 0 && !gm.l_done /* completion with last move is okay */ )
            return 0;

    }

    // stopped and awaiting action ?
    if (stp && gm.m_act != M_EMPTY)
        Mr_Act();
    else
        if (stp) {
#ifdef SOUND
            sound_play(gm.wv_stp);
#endif
        }

    return 1;
}

/*
    select a marble
*/
void Mr_Sel(int x, int y)
{
    int mx, my;

    mx = (x - gm.l_x) / gm.t_w;
    my = (y - gm.l_y) / gm.t_h;

    if (gm.m_sel) {
        gm.mf_a.p = gm.msf_a.p = 0;
        Mr_Ins();
    }

    // set selected
    gm.m_sel = 1;
    // get map position
    gm.m_mx = mx;
    gm.m_my = my;
    // check valid moving directions
    Mr_CkVDir(mx, my);
    // delete marble from background
    L_DrwMpTl(mx, my);
    // get position in screen
    gm.m_x = mx * gm.t_w + gm.l_x;
    gm.m_y = my * gm.t_h + gm.l_y;
    // get id //
    gm.m_id = gm.c_lvl->map[mx][my].m;
    // delete marble from map
    gm.c_lvl->map[mx][my].m = -1;
    // save old position
    gm.m_o_x = (int)gm.m_x;
    gm.m_o_y = (int)gm.m_y;
    gm.m_o_move_count = gm.c_lvl->tm;

#ifdef SOUND
    sound_play(gm.wv_sel);
#endif
}

/*
    release a marble
*/
void Mr_Rel(int x, int y)
{
    Mr_Ins();
    gm.m_sel = 0;
    SDL_SetCursor(gm.c_n);
    gm.m_o_x = gm.m_o_y = -1;
}

/*
    initialize movement
*/
void Mr_IniMv()
{
    int x_a = 0, y_a = 0, tx, ty;
    int t_fnd = 0;

    // direction
    gm.m_d = gm.c_stat -2;
    if (gm.m_d < 0 || gm.m_d > 4)
        return;

    // direction verified; activate movement
    gm.m_mv = 1;

    // store position if no action
    if ( gm.m_act == M_EMPTY ) {
        gm.m_o_x = (int)gm.m_x;
        gm.m_o_y = (int)gm.m_y;
        gm.m_o_move_count = gm.c_lvl->tm;
    }

    // clear previous action
    gm.m_act = M_EMPTY;

    // compute target position
    switch (gm.m_d) {
        case 0: y_a = -1; break;
        case 1: x_a = 1; break;
        case 2: y_a = 1; break;
        case 3: x_a = -1; break;
    }
    tx = gm.m_mx + x_a; ty = gm.m_my + y_a;
    while ( // target already found
            !t_fnd &&
            // wall
            gm.c_lvl->map[tx][ty].t != M_WALL &&
            // crumbling wall
            gm.c_lvl->map[tx][ty].t != M_CRUMBLE &&
            // marble
            gm.c_lvl->map[tx][ty].m == -1 &&
            // up
            !((gm.c_lvl->map[tx][ty].t == M_OW_D || gm.c_lvl->map[tx][ty].t == M_OW_D_C || gm.c_lvl->map[tx][ty].t == M_OW_L_C || gm.c_lvl->map[tx][ty].t == M_OW_R_C) && y_a == -1) &&
            // down
            !((gm.c_lvl->map[tx][ty].t == M_OW_U || gm.c_lvl->map[tx][ty].t == M_OW_U_C || gm.c_lvl->map[tx][ty].t == M_OW_L_C || gm.c_lvl->map[tx][ty].t == M_OW_R_C) && y_a == 1) &&
            // right
            !((gm.c_lvl->map[tx][ty].t == M_OW_L || gm.c_lvl->map[tx][ty].t == M_OW_L_C || gm.c_lvl->map[tx][ty].t == M_OW_U_C || gm.c_lvl->map[tx][ty].t == M_OW_D_C) && x_a == 1) &&
            // left
            !((gm.c_lvl->map[tx][ty].t == M_OW_R || gm.c_lvl->map[tx][ty].t == M_OW_R_C || gm.c_lvl->map[tx][ty].t == M_OW_U_C || gm.c_lvl->map[tx][ty].t == M_OW_D_C) && x_a == -1)
           ) {

        // check action
        switch (gm.c_lvl->map[tx][ty].t) {
            case M_TLP_0:
            case M_TLP_1:
            case M_TLP_2:
            case M_TLP_3:
                t_fnd = 1;
                gm.m_act = gm.c_lvl->map[tx][ty].t;
                break;
            case M_OW_U:
                if (y_a != -1) {
                    gm.m_act = M_OW_U;
                    t_fnd = 1;
                }
                break;
            case M_OW_D:
                if (y_a != 1) {
                    gm.m_act = M_OW_D;
                    t_fnd = 1;
                }
                break;
            case M_OW_L:
                if (x_a != -1) {
                    gm.m_act = M_OW_L;
                    t_fnd = 1;
                }
                break;
            case M_OW_R:
                if (x_a != 1) {
                    gm.m_act = M_OW_R;
                    t_fnd = 1;
                }
                break;
        }
        tx += x_a;
        ty += y_a;
    }

    // crumbling wall
    if (gm.c_lvl->map[tx][ty].t == M_CRUMBLE)
        gm.m_act = M_CRUMBLE;

    tx -= x_a;
    ty -= y_a;

    gm.m_tx = tx * gm.t_w + gm.l_x;
    gm.m_ty = ty * gm.t_h + gm.l_y;

    // wait cursor
    SDL_SetCursor(gm.c_w);
}

/*
    stop a marble
*/
void Mr_Stp()
{
    int mx, my;

    // position in screen
    gm.m_x = gm.m_tx;
    gm.m_y = gm.m_ty;

    mx = (gm.m_x - gm.l_x) / gm.t_w;
    my = (gm.m_y - gm.l_y) / gm.t_h;

    // position in map
    gm.m_mx = mx;
    gm.m_my = my;

    // check valid moving directions
    Mr_CkVDir(mx, my);

    gm.m_mv = 0;
    gm.m_a.p = 0;

    // check cursor //
    Cr_Cng(gm.o_mx, gm.o_my);

    // if no action check if the figure is completed
    if (gm.m_act == M_EMPTY || gm.m_act == M_CRUMBLE) {

        gm.c_lvl->map[mx][my].m = gm.m_id;
        G_CkFgr();
        gm.c_lvl->map[mx][my].m = -1;
    }
}

/*
    insert a marble into map
*/
void Mr_Ins()
{
    int mx, my, x, y;

    mx = (gm.m_x - gm.l_x) / gm.t_w;
    my = (gm.m_y - gm.l_y) / gm.t_h;
    x = mx * gm.t_w + gm.l_x;
    y = my * gm.t_h + gm.l_y;

    //hide frame
    MF_Hd();
    // draw to background
    D_DST(gm.s_bkgd, x, y, gm.t_w, gm.t_h);
    D_SRC(gm.s_mrb, 0, gm.m_id * gm.t_h);
    SS_Blt();
    // to screen
    D_DST(sdl.scr, x, y, gm.t_w, gm.t_h);
    D_SRC(gm.s_mrb, 0, gm.m_id * gm.t_h);
    SS_Blt();
    // and to map
    gm.c_lvl->map[mx][my].m = gm.m_id;
}

/*
    check valid directions
*/
void Mr_CkVDir(int mx, int my)
{
    gm.m_vd = 0;
    if ( gm.c_lvl->map[mx][my].t != M_OW_D_C && gm.c_lvl->map[mx][my].t != M_OW_U_C && gm.c_lvl->map[mx - 1][my].t != M_WALL && gm.c_lvl->map[mx - 1][my].t != M_CRUMBLE && gm.c_lvl->map[mx - 1][my].m == -1 && gm.c_lvl->map[mx - 1][my].t != M_OW_R && gm.c_lvl->map[mx - 1][my].t != M_OW_R_C && gm.c_lvl->map[mx - 1][my].t != M_OW_U_C && gm.c_lvl->map[mx - 1][my].t != M_OW_D_C)
        gm.m_vd = gm.m_vd | MD_L;
    if ( gm.c_lvl->map[mx][my].t != M_OW_D_C && gm.c_lvl->map[mx][my].t != M_OW_U_C && gm.c_lvl->map[mx + 1][my].t != M_WALL && gm.c_lvl->map[mx + 1][my].t != M_CRUMBLE && gm.c_lvl->map[mx + 1][my].m == -1 && gm.c_lvl->map[mx + 1][my].t != M_OW_L && gm.c_lvl->map[mx + 1][my].t != M_OW_L_C && gm.c_lvl->map[mx + 1][my].t != M_OW_U_C && gm.c_lvl->map[mx + 1][my].t != M_OW_D_C)
        gm.m_vd = gm.m_vd | MD_R;
    if ( gm.c_lvl->map[mx][my].t != M_OW_L_C && gm.c_lvl->map[mx][my].t != M_OW_R_C && gm.c_lvl->map[mx][my - 1].t != M_WALL && gm.c_lvl->map[mx][my - 1].t != M_CRUMBLE && gm.c_lvl->map[mx][my - 1].m == -1 && gm.c_lvl->map[mx][my - 1].t != M_OW_D && gm.c_lvl->map[mx][my - 1].t != M_OW_D_C && gm.c_lvl->map[mx][my - 1].t != M_OW_L_C && gm.c_lvl->map[mx][my - 1].t != M_OW_R_C)
        gm.m_vd = gm.m_vd | MD_U;
    if ( gm.c_lvl->map[mx][my].t != M_OW_L_C && gm.c_lvl->map[mx][my].t != M_OW_R_C && gm.c_lvl->map[mx][my + 1].t != M_WALL && gm.c_lvl->map[mx][my + 1].t != M_CRUMBLE && gm.c_lvl->map[mx][my + 1].m == -1 && gm.c_lvl->map[mx][my + 1].t != M_OW_U && gm.c_lvl->map[mx][my + 1].t != M_OW_U_C && gm.c_lvl->map[mx][my + 1].t != M_OW_L_C && gm.c_lvl->map[mx][my + 1].t != M_OW_R_C)
        gm.m_vd = gm.m_vd | MD_D;
}

/*
    handle actions!
*/
void Mr_Act()
{
    int x_a = 0, y_a = 0, ow = 0, mx = gm.m_mx, my = gm.m_my, tx, ty;
    int i, j;

    // crumbling wall ?
    if (gm.m_act == M_CRUMBLE) {
        tx = mx + (gm.m_d == 1 ? 1 : gm.m_d == 3 ? -1 : 0);
        ty = my + (gm.m_d == 0 ? -1 : gm.m_d == 2 ? 1 : 0);
        if (gm.c_lvl->map[tx][ty].t == M_CRUMBLE) {
#ifdef SOUND
                sound_play(gm.wv_stp);
#endif
            if (gm.c_lvl->map[tx][ty].id > 0)
                gm.c_lvl->map[tx][ty].id--;
            else {
                Wl_Exp(gm.l_x + tx * gm.t_w, gm.l_y + ty * gm.t_h, gm.m_d);
                gm.c_lvl->map[tx][ty].t = M_FLOOR;
                // check moving direction
                Mr_CkVDir(gm.m_mx, gm.m_my);
                Cr_Cng(gm.o_mx, gm.o_my);
                // reset restore position
                gm.m_o_x = gm.m_tx;
                gm.m_o_y = gm.m_ty;
                gm.m_o_move_count = gm.c_lvl->tm;
#ifdef SOUND
                sound_play(gm.wv_exp);
#endif
            }
            // draw to background
            L_DrwMpTl(tx, ty);
            // draw to screen
            D_DST(sdl.scr, gm.l_x + tx * gm.t_w, gm.l_y + ty * gm.t_h, gm.t_w, gm.t_h);
            D_SRC(gm.s_bkgd, gm.l_x + tx * gm.t_w, gm.l_y + ty * gm.t_h);
            SS_Blt();
            // add refresh rect
            Sdl_AddR(gm.l_x + tx * gm.t_w, gm.l_y + ty * gm.t_h, gm.t_w, gm.t_h);
        }
        // no action
//        gm.m_act = M_EMPTY;
        return;
    }

    // oneway ?
    switch (gm.m_act) {
        case M_OW_U:
            y_a = -1;
            ow = 1;
            gm.c_stat = C_U;
            break;
        case M_OW_D:
            y_a = 1;
            ow = 1;
            gm.c_stat = C_D;
            break;
        case M_OW_R:
            x_a = 1;
            ow = 1;
            gm.c_stat = C_R;
            break;
        case M_OW_L:
            x_a = -1;
            ow = 1;
            gm.c_stat = C_L;
            break;
    }
    if (ow) {
/*        mx += x_a; my += y_a;
        while (gm.c_lvl->map[mx][my].m != -1) {
            mx += x_a;
            my += y_a;
        }
        mx -= x_a; my -= y_a;
        if (mx != gm.m_mx || my != gm.m_my) {
            Mr_Ins();
            Mr_Sel(gm.l_x + mx * gm.t_w, gm.l_y + my * gm.t_h);
        }*/
#ifdef SOUND
        sound_play(gm.wv_arw);
#endif
        Mr_IniMv();
        return;
    }

    // teleport ?
    if (gm.m_act >= M_TLP_0 && gm.m_act <= M_TLP_3) {
        for (i = 0; i < gm.c_lvl->m_w; i++) {
            for (j = 0; j < gm.c_lvl->m_h; j++)
                if (gm.c_lvl->map[i][j].t == gm.m_act && (i != gm.m_mx || j != gm.m_my)) {
                    // only warp if destination is empty //
                    if (gm.c_lvl->map[i][j].m == -1) {
                        gm.m_mx = i;
                        gm.m_my = j;
                        gm.m_x = gm.m_mx * gm.t_w + gm.l_x;
                        gm.m_y = gm.m_my * gm.t_h + gm.l_y;
#ifdef SOUND
                        sound_play(gm.wv_tlp);
#endif
                    }
                    gm.c_stat = gm.m_d + 2; // restore c_stat for movement initialization
                    // initate movement
                    Mr_IniMv();

/*                    // check if the figure is completed
                    gm.c_lvl->map[gm.m_mx][gm.m_my].m = gm.m_id;
                    G_CkFgr();
                    gm.c_lvl->map[gm.m_mx][gm.m_my].m = -1;*/

                    return;
                }
        }
    }

//    gm.m_act = M_EMPTY;
}

/*
    restore old position
*/
void Mr_ResPos()
{
    if ( !gm.m_sel || gm.m_o_x == -1 ) return;

    gm.m_act = M_EMPTY;
    gm.m_tx = gm.m_o_x;
    gm.m_ty = gm.m_o_y;
    if ( gm.c_l_st->limit_type == MOVES )
        gm.c_lvl->tm = gm.m_o_move_count;
    Mr_Stp();
}

// timer //
/*
    hide time
*/
void Tm_Hd()
{
    int w = gm.brd_w - gm.t_x * 2;
    int h = gm.f_wht->h;

    D_DST(sdl.scr, gm.t_x + gm.b_x, gm.t_y, w, h);
    D_SRC(gm.s_bkgd, gm.t_x + gm.b_x, gm.t_y);
    SS_Blt();
    Sdl_AddR(gm.t_x + gm.b_x, gm.t_y, w, h);
}

/*
    show time
*/
void Tm_Shw()
{
    SFnt *ft;
    char str_tm[16];
    char str_sec[4];
    int tm;

    // adjust time //
    if ( gm.c_l_st->limit_type == TIME )
        tm = gm.c_lvl->tm / 1000;
    else
        tm = gm.c_lvl->tm;

    // select font
    ft = gm.f_wht;
    if ( gm.c_l_st->limit_type == TIME && tm <= 30 )
        ft = gm.f_rd;
    else
        if ( gm.c_l_st->limit_type == MOVES && tm <= 10 )
            ft = gm.f_rd;

    // draw "time"
    ft->algn = TA_X_L | TA_Y_T;
    if ( gm.c_l_st->limit_type == TIME )
        SF_Wrt(ft, sdl.scr, gm.t_x + gm.b_x, gm.t_y, "Time:", 0);
    else
        SF_Wrt(ft, sdl.scr, gm.t_x + gm.b_x, gm.t_y, "Moves:", 0);

    // compute and draw time str
    if ( gm.c_l_st->limit_type == TIME ) {

        sprintf(str_tm, "%i:", tm / 60);
        sprintf(str_sec, "%i", tm % 60);
        if (strlen(str_sec) < 2)
            strcat(str_tm, "0");
        strcat(str_tm, str_sec);

    }
    else
        sprintf( str_tm, "%i", tm );

    ft->algn = TA_X_R | TA_Y_T;
    SF_Wrt(ft, sdl.scr, gm.scr_w - gm.t_x, gm.t_y, str_tm, 0);

    Sdl_AddR(gm.t_x + gm.b_x, gm.t_y, gm.b_x - gm.t_x*2, ft->h);
}

/*
    update time
    return 0 if time out
*/
int Tm_Upd(int ms)
{
#ifdef SOUND
    int old_sec = gm.c_lvl->tm / 1000;
#endif

    gm.blink_time += ms;

    // if limit_type is MOVES, time is ignored //
    if ( gm.c_l_st->limit_type == MOVES ) return 1;

    gm.c_lvl->tm -= ms;

    // new second ?
#ifdef SOUND
    if ( old_sec != gm.c_lvl->tm / 1000 && old_sec <= 30 )
        sound_play(gm.wv_alm);
#endif

    if (gm.c_lvl->tm < 0) {
        gm.c_lvl->tm = 0;
        return 0;
    }
    return 1;
}

// level info //
/*
    hide level info
*/
void Inf_Hd()
{
    D_DST(sdl.scr, gm.b_x + gm.s_x, gm.s_y + gm.s_h - 20, gm.s_w, 20);
    D_SRC(gm.s_bkgd, gm.b_x + gm.s_x, gm.s_y + gm.s_h - 20);
    SS_Blt();
    Sdl_AddR(gm.b_x + gm.s_x, gm.s_y + gm.s_h - 20, gm.s_w, 20);
}

/*
    update level info
*/
int Inf_Upd()
{
    int x, y;

    x = (gm.o_mx - gm.c_x - gm.b_x) / L_SIZE;
    y = (gm.o_my - gm.c_y) / L_SIZE;

    if (gm.o_mx < gm.c_x + gm.b_x || gm.o_my < gm.c_y  || x >= gm.c_l_st->l_num || y >= gm.c_l_st->c_num) {
        sprintf(gm.inf_str, "Tier %i, Puzzle %i", gm.c_ch + 1, gm.c_l_id + 1);
        return 0;
    }
    if (!gm.c_s_inf->cmp[y * gm.c_s_inf->l_num + x] && !gm.c_s_inf->c_opn[y]) {
        sprintf(gm.inf_str, "Access Denied");
        return 0;
    }
    sprintf(gm.inf_str, "Tier %i, Puzzle %i", y + 1, x + 1);
    if (gm.bttn[1]) {
        gm.w_c = y;
        gm.w_l = x;
        return 1;
    }
    return 0;
}

/*
    show level info
*/
void Inf_Shw()
{
    gm.f_sml->algn = TA_X_C | TA_Y_B;
    SF_Wrt(gm.f_sml, sdl.scr, gm.b_x + gm.brd_w / 2, gm.s_y + gm.s_h - 5, gm.inf_str, 0);
}

// cursor
/*
    load cursor
*/
void Cr_Ld(char *src, char *d, char*m)
{
    int w=16, h = 16;
    int i, j, k;
    char b_d, b_m;
    int p;

    k = 0;
    for (j = 0; j < w * h; j += 8, k++) {
        p = 1;
        b_d = b_m = 0;
        // create byte
        for (i = 7; i >= 0; i--) {
            switch (src[j + i]) {
                case 2:
                    b_d += p;
                case 1:
                    b_m += p;
                    break;
            }
            p *= 2;
        }
        // add to mask
        d[k] = b_d;
        m[k] = b_m;
    }
}

/*
    change cursors appearing
*/
void Cr_Cng(int x, int y)
{
    int mx, my, cx, cy;

    if (gm.m_mv) {
        if (x > gm.scr_w - gm.brd_w)
            SDL_SetCursor(gm.c_n);
        else
            SDL_SetCursor(gm.c_w);
        return;
    }

    mx = (x - gm.l_x) / gm.t_w;
    my = (y - gm.l_y) / gm.t_h;
    
    if ( mx >= 0 && my >= 0 && mx < gm.c_lvl->m_w && my < gm.c_lvl->m_h 
     && (gm.c_lvl->map[mx][my].m != - 1 || (gm.m_mx == mx && gm.m_my == my))) {
        // on marble
        SDL_SetCursor(gm.c_s);
        gm.c_stat = C_SEL;
    }
    else
        if (!gm.m_sel || x > gm.scr_w - gm.brd_w) {
            // nothing selected
            SDL_SetCursor(gm.c_n);
            gm.c_stat = C_NONE;
        }
        else {
            // up, left, right, down
            cx = x - (gm.m_mx * gm.t_w + gm.l_x + gm.t_w / 2);
            cy = y - (gm.m_my * gm.t_h + gm.l_y + gm.t_h / 2);
            if (abs(cx) > abs(cy)) {
                if (cx > 0) {
                    if (gm.m_vd & MD_R) {
                        SDL_SetCursor(gm.c_r);
                        gm.c_stat = C_R;
                    }
                    else {
                       SDL_SetCursor(gm.c_n);
                       gm.c_stat = C_NONE;
                    }
                }
                else {
                    if (gm.m_vd & MD_L) {
                        SDL_SetCursor(gm.c_l);
                        gm.c_stat = C_L;
                    }
                    else {
                        SDL_SetCursor(gm.c_n);
                        gm.c_stat = C_NONE;
                    }
                }
            }
            else {
                if (cy > 0) {
                    if (gm.m_vd & MD_D) {
                        SDL_SetCursor(gm.c_d);
                        gm.c_stat = C_D;
                    }
                    else {
                        SDL_SetCursor(gm.c_n);
                        gm.c_stat = C_NONE;
                    }
                }
                else {
                    if (gm.m_vd & MD_U) {
                        SDL_SetCursor(gm.c_u);
                        gm.c_stat = C_U;
                    }
                    else {
                        SDL_SetCursor(gm.c_n);
                        gm.c_stat = C_NONE;
                    }
                }
            }
        }
}

// frame //
/*
    hide marble frame
*/
void MF_Hd()
{
    int x, y, w, h;

    if (!gm.m_sel) return;

    // get size
    if (gm.msf_a.p == gm.msf_a.f) {
        w = gm.mf_a.w;
        h = gm.mf_a.h;
    }
    else {
        w = gm.msf_a.w;
        h = gm.msf_a.h;
    }

    // get position
    x = gm.m_x + (gm.t_w - w) / 2;
    y = gm.m_y + (gm.t_h - h) / 2;

    // hide
    D_DST(sdl.scr, x, y, w, h);
    D_SRC(gm.s_bkgd, x, y);
    SS_Blt();
    Sdl_AddR(x - 1, y - 1, w + 1, h + 1);
}

/*
    update marble frame
*/
void MF_Upd(int ms)
{
    if (!gm.m_sel) {
        gm.mf_a.p = gm.msf_a.p = 0;
        return;
    }
    if (gm.msf_a.p != gm.msf_a.f) {
        // still select animation
        gm.msf_a.p += gm.msf_a.c * ms;
        if (gm.msf_a.p >= gm.msf_a.f)
            gm.msf_a.p = gm.msf_a.f;
    }
    else {
        gm.mf_a.p += gm.mf_a.c * ms;
        if (gm.mf_a.p >= gm.mf_a.f)
            gm.mf_a.p = 0;
    }
}

/*
    show marble frame
*/
void MF_Shw()
{
    int x, y;
    AInf *a;
    SDL_Surface *s;

    if (!gm.m_sel) return;

    // get animation info
    if (gm.msf_a.p == gm.msf_a.f) {
        a = &gm.mf_a;
        s = gm.s_mf;
    }
    else {
        a = &gm.msf_a;
        s = gm.s_msf;
    }

    // get position
    x = gm.m_x + (gm.t_w - a->w) / 2;
    y = gm.m_y + (gm.t_h - a->h) / 2;

    // show
    D_DST(sdl.scr, x, y, a->w, a->h);
    D_SRC(s, (int)a->p * a->w, 0);
    SS_Blt();
    Sdl_AddR(x, y, a->w, a->h);
}

// map animations //
/*
    get position and type of all animations
*/
void MA_Ini()
{
    int i, j;

    // free and reallocate m_ani
    if (gm.m_ani)
        free(gm.m_ani);
    gm.m_ani = (MAni*)malloc(sizeof(MAni) * gm.c_lvl->m_w * gm.c_lvl->m_h);
    gm.ma_num = 0;

    // parse level map
    for (i = 0; i < gm.c_lvl->m_w; i++)
        for (j = 0; j < gm.c_lvl->m_h; j++)
            switch (gm.c_lvl->map[i][j].t) {
                case M_OW_U:
                case M_OW_D:
                case M_OW_L:
                case M_OW_R:
                case M_OW_U_C:
                case M_OW_D_C:
                case M_OW_L_C:
                case M_OW_R_C:
                    gm.m_ani[gm.ma_num].x = i;
                    gm.m_ani[gm.ma_num].y = j;
                    gm.m_ani[gm.ma_num].t = gm.c_lvl->map[i][j].t;
                    gm.m_ani[gm.ma_num].a = &gm.ma_ow_a;
                    gm.ma_num++;
                    break;
                case M_TLP_0:
                case M_TLP_1:
                case M_TLP_2:
                case M_TLP_3:
                    gm.m_ani[gm.ma_num].x = i;
                    gm.m_ani[gm.ma_num].y = j;
                    gm.m_ani[gm.ma_num].t = gm.c_lvl->map[i][j].t;
                    gm.m_ani[gm.ma_num].a = &gm.ma_tlp_a;
                    gm.ma_num++;
                    break;
                default:
                    break;
            }
}

void MA_Upd(int ms)
{
    if (!cfg.ani) return;

    gm.ma_ow_a.p += (float)ms * gm.ma_ow_a.c;
    if (gm.ma_ow_a.p >= gm.ma_ow_a.f)
        gm.ma_ow_a.p = 0;

    gm.ma_tlp_a.p += (float)ms * gm.ma_tlp_a.c;
    if (gm.ma_tlp_a.p >= gm.ma_tlp_a.f)
        gm.ma_tlp_a.p = 0;
}

/*
    show map animations
*/
void MA_Shw()
{
    int i;
    int x, y;

    if (!cfg.ani) return;

    for (i = 0; i < gm.ma_num; i++) {
        // get position in screen
        x = gm.l_x + gm.m_ani[i].x * gm.t_w;
        y = gm.l_y + gm.m_ani[i].y * gm.t_h;

        // draw empty floor
        D_DST(sdl.scr, x, y, gm.t_w, gm.t_h);
        D_SRC(gm.c_g_st->s_flr, 0, 0);
        SS_Blt();

        // oneway
        D_DST(sdl.scr, x, y, gm.t_w, gm.t_h);
        switch (gm.m_ani[i].t) {
            case M_OW_R:
            case M_OW_R_C:
                D_SRC(gm.c_g_st->s_r_arw, (int)gm.m_ani[i].a->p * gm.t_w, 0);
                SS_Blt();
                break;
            case M_OW_L:
            case M_OW_L_C:
                D_SRC(gm.c_g_st->s_l_arw, (int)gm.m_ani[i].a->p * gm.t_w, 0);
                SS_Blt();
                break;
            case M_OW_U:
            case M_OW_U_C:
                D_SRC(gm.c_g_st->s_u_arw, 0, (int)gm.m_ani[i].a->p * gm.t_h);
                SS_Blt();
                break;
            case M_OW_D:
            case M_OW_D_C:
                D_SRC(gm.c_g_st->s_d_arw, 0, (int)gm.m_ani[i].a->p * gm.t_h);
                SS_Blt();
                break;
        }
        D_DST(sdl.scr, x, y, gm.t_w, gm.t_h);
        switch (gm.m_ani[i].t) {
            case M_OW_U_C:
            case M_OW_D_C:
                D_SRC(gm.c_g_st->s_lr_bar, 0, 0);
                SS_Blt();
                break;
            case M_OW_L_C:
            case M_OW_R_C:
                D_SRC(gm.c_g_st->s_ud_bar, 0, 0);
                SS_Blt();
                break;
        }

        // teleport
        D_DST(sdl.scr, x, y, gm.t_w, gm.t_h);
        switch (gm.m_ani[i].t) {
            case M_TLP_0:
                D_SRC(gm.c_g_st->s_tlp_0, (int)gm.m_ani[i].a->p * gm.t_w, 0);
                SS_ABlt(gm.tlp_a);
                break;
            case M_TLP_1:
                D_SRC(gm.c_g_st->s_tlp_1, (int)gm.m_ani[i].a->p * gm.t_w, 0);
                SS_ABlt(gm.tlp_a);
                break;
            case M_TLP_2:
                D_SRC(gm.c_g_st->s_tlp_2, (int)gm.m_ani[i].a->p * gm.t_w, 0);
                SS_ABlt(gm.tlp_a);
                break;
            case M_TLP_3:
                D_SRC(gm.c_g_st->s_tlp_3, (int)gm.m_ani[i].a->p * gm.t_w, 0);
                SS_ABlt(gm.tlp_a);
                break;
        }

        // marble on animation
        if (gm.c_lvl->map[gm.m_ani[i].x][gm.m_ani[i].y].m != -1 &&
            ( ((gm.blink_time / 250) & 1) || gm.l_done ) ) {
            D_DST(sdl.scr, x, y, gm.t_w, gm.t_h);
            D_SRC(gm.s_mrb, 0, gm.c_lvl->map[gm.m_ani[i].x][gm.m_ani[i].y].m * gm.t_w);
            SS_Blt();
        }

        // refresh rect
        Sdl_AddR(x, y, gm.t_w, gm.t_h);
    }
}

// shrapnells //
/*
    add new shrapnell
*/
void Shr_Add(int x, int y, int w, int h, Vec d, SDL_Surface *s_shr)
{
    Shr *s;

    s = (Shr*)malloc(sizeof(Shr));
    s->d = d;
    s->x = x;
    s->y = y;
    s->w = w;
    s->h = h;
    s->a = 0;
    if ( s_shr )
        s->s_shr = s_shr;
    else  {

        s->s_shr = SS_Crt(w, h, SDL_SWSURFACE);
        D_DST(s->s_shr, 0, 0, w, h);
        D_SRC(sdl.scr, x, y);
        SS_Blt();

    }

    DL_Add(&gm.shr, s);
}

/*
    delete shrapnell
*/
void Shr_Del(void *p)
{
    Shr *s = (Shr*)p;
    if ( s->s_shr ) SDL_FreeSurface(s->s_shr);
    free(p);
}

/*
    hide shrapnell
*/
void Shr_Hd()
{
    DL_E *e = gm.shr.hd.n;
    Shr *s;

    while ( e != &gm.shr.tl ) {

        s = (Shr*)e->d;

        D_DST(sdl.scr, (int)s->x, (int)s->y, s->w, s->h);
        D_SRC(gm.s_bkgd, (int)s->x, (int)s->y);
        SS_Blt();
        Sdl_AddR((int)s->x, (int)s->y, s->w, s->h);

        e = e->n;

    }

}

/*
    update shrapnell
*/
void Shr_Upd(int ms)
{
    DL_E *n, *e = gm.shr.hd.n;
    Shr *s;

    while ( e != &gm.shr.tl ) {

        n = e->n;

        s = (Shr*)e->d;

        s->x += s->d.x * (float)ms;
        s->y += s->d.y * (float)ms;
        s->a += gm.shr_a_c * (float)ms;

        if (s->a >= 255)
            DL_DelE(&gm.shr, e);

        e = n;

    }
}

/*
    show shrapnell
*/
void Shr_Shw()
{
    DL_E *e = gm.shr.hd.n;
    Shr *s;

    while ( e != &gm.shr.tl ) {

        s = (Shr*)e->d;

        D_DST(sdl.scr, (int)s->x, (int)s->y, s->w, s->h);
        D_SRC(s->s_shr, 0, 0);
        SS_ABlt(s->a);
        Sdl_AddR((int)s->x, (int)s->y, s->w, s->h);

        e = e->n;

    }

}

// wall //
/*
    explode crumble wall into lot of pieces
*/
void Wl_Exp(int x, int y, int d)
{
    int i, j;
    int sz = 4;
    Vec v;
    int x_r, x_off, y_r, y_off; // direction values
    SDL_Surface *s_shr;

    if ( !cfg.ani ) return;

    x_r = y_r = 200;
    x_off = y_off = 100;

    // adjust direction values
    switch (d) {

        case 0:
            y_r = y_off = 100;
            break;

        case 1:
            x_r = 100;
            x_off = 0;
            break;

        case 2:
            y_r = 100;
            y_off = 0;
            break;

        case 3:
            x_r = x_off = 100;
            break;

    }

    for ( i = 0; i < gm.t_w; i += sz )
        for ( j = 0; j < gm.t_h; j += sz ) {

            v.x = (float)((rand() % x_r ) - x_off) / 1000;
            v.y = (float)((rand() % y_r ) - y_off) / 1000;

            s_shr = SS_Crt(sz, sz, SDL_SWSURFACE);
            D_DST(s_shr, 0, 0, sz, sz);
            D_SRC(gm.c_g_st->s_crmbl, i, j);
            SS_Blt();

            Shr_Add(x + i, y + j, sz, sz, v, s_shr);

        }
}

// figure animation //
/*
    main animation function; select and explode marbles one by one
*/
void FA_Run()
{
    int i, j, k;
    int m_cnt = 0; // marble count
    int m_pos[gm.c_lvl->f_w * gm.c_lvl->f_h][2], b;
    int ms;
    SDL_Event e;
    int leave = 0;
    int tm, c_tm; // time in ms

    if (!cfg.ani) return;

    if (gm.m_sel)
        Mr_Ins();

    // count marbles and get position
    for ( i = 0; i < gm.c_lvl->m_w; i++ )
        for (j = 0; j < gm.c_lvl->m_h; j++ )
            if ( gm.c_lvl->map[i][j].m != - 1 ) {

                m_pos[m_cnt][0] = i;
                m_pos[m_cnt][1] = j;
                m_cnt++;

            }

    // unsort positions
    for ( k = 0; k < m_cnt * 5; k++) {

        i = rand() % m_cnt;
        j = rand() % m_cnt;

        b = m_pos[i][0];
        m_pos[i][0] = m_pos[j][0];
        m_pos[j][0] = b;
        b = m_pos[i][1];
        m_pos[i][1] = m_pos[j][1];
        m_pos[j][1] = b;

    }

    // explosions
    MF_Hd();
    T_Rst();
    tm = 250;
    c_tm = 0;
    m_cnt--;
    SDL_SetCursor(gm.c_w);
    while ( (m_cnt >= 0 || gm.shr.cntr > 0) && !trm_gm && !leave ) {

        // termination ?
        if (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    trm_gm = 1;
                    break;
                case SDL_MOUSEBUTTONUP:
                case SDL_KEYUP:
                    if (e.key.keysym.sym == SDLK_TAB)
                        SnapShot();
                    else
                        leave = 1;
                    break;
            }
        }

        // show shrapnells
        Shr_Hd();
        ms = T_Gt();
        c_tm -= ms;
        MA_Upd(ms);
        Shr_Upd(ms);
        MA_Shw();
        Shr_Shw();
        Tm_Shw();
        Inf_Shw();
        Sdl_UpdR();

        //add new shrapnells
        if (m_cnt >= 0 && c_tm <= 0) {

            FA_Add(m_pos[m_cnt][0],
                   m_pos[m_cnt][1],
                   gm.c_lvl->map[m_pos[m_cnt][0]][m_pos[m_cnt][1]].m);
            c_tm = tm;
            m_cnt--;
#ifdef SOUND
            sound_play(gm.wv_exp);
#endif

        }

    }

}

/*
    add shrapnells at x,y with marble-id m
*/
void FA_Add(int mx, int my, int m)
{
    int x, y;
    int i, j;
    int sz = 4; // size
    SDL_Surface *s_shr;
    Vec v;

    x = gm.l_x + mx * gm.t_w;
    y = gm.l_y + my * gm.t_h;

    // delete from screne
    gm.c_lvl->map[mx][my].m = -1;
    L_DrwMpTl(mx, my);
    Sdl_AddR(x, y, gm.t_w, gm.t_h);

    // create shrapnells
    for ( i = 0; i < gm.t_w; i += sz )
        for ( j = 0; j < gm.t_h; j += sz ) {

            v.x = (float)((rand() % 200 ) - 100) / 1000;
            v.y = (float)((rand() % 200 ) - 100) / 1000;

            s_shr = SS_Crt(sz, sz, SDL_SWSURFACE);
            D_DST(s_shr, 0, 0, sz, sz);
            D_SRC(gm.s_mrb, i, gm.t_h * m + j);
            SS_Blt();

            Shr_Add(x + i, y + j, sz, sz, v, s_shr);

        }
}

// credits //
/*
    initiate credits
*/
void Cr_Ini()
{
    gm.cr_st = 0;
    gm.cr_a = 255;
    gm.cr_c_tm = gm.cr_tm;
    sprintf(gm.cr_str, "'%s' (Author: %s)", gm.c_l_st->ch[gm.c_ch].nm, gm.c_l_st->ch[gm.c_ch].authr);
    gm.cr_w = SF_TxtW(gm.f_sml, gm.cr_str);
    gm.cr_h = gm.f_sml->h;
    gm.cr_x = (gm.scr_w - gm.brd_w - gm.cr_w) / 2;
}

/*
    hide credits
*/
void Cr_Hd()
{
    if ( gm.cr_st == 3 ) return;

    D_DST(sdl.scr, gm.cr_x, gm.cr_y, gm.cr_w, gm.cr_h);
    D_SRC(gm.s_bkgd, gm.cr_x, gm.cr_y);
    SS_Blt();
}

/*
    update credits
*/
void Cr_Upd(int ms)
{
    if ( gm.cr_st == 3 ) return;

    switch ( gm.cr_st ) {

        case 0:
            // undim
            gm.cr_a -= gm.cr_a_c * (float)ms;
            if ( gm.cr_a <= 0 ) {

                gm.cr_a = 0;
                gm.cr_st = 1;

            }
            break;

        case 1:
            // just show till timeout
            gm.cr_c_tm -= ms;
            if ( gm.cr_c_tm <= 0 )
                gm.cr_st = 2;
            break;

        case 2:
            // undim
            gm.cr_a += gm.cr_a_c * (float)ms;
            if ( gm.cr_a >= 255 ) {

                gm.cr_a = 255;
                gm.cr_st = 3;

            }
            break;

    }
}

/*
    show credits
*/
void Cr_Shw()
{
    if ( gm.cr_st == 3) return;

    gm.f_sml->algn = TA_X_L | TA_Y_T;
    SF_Wrt(gm.f_sml, sdl.scr, gm.cr_x, gm.cr_y, gm.cr_str, (int)gm.cr_a);
    Sdl_AddR(gm.cr_x, gm.cr_y, gm.cr_w, gm.cr_h);
}

// modify score //
void modify_score( int *b_lvl, int *b_tm )
{
    /* modify score according to difficulty level */
    switch (cfg.diff) {
        case DIFF_EASY:
            *b_lvl /= 2;
            *b_tm  /= 2;
            break;
        case DIFF_NORMAL: break;
        case DIFF_HARD:
            *b_lvl *= 2;
            *b_tm  *= 2;
            break;
        case DIFF_BRAINSTORM:
            *b_tm  *= 5;
            *b_lvl *= 5;
            break;
    }
}

// bonus summary //
/*
    give a bonus summary
*/
void BS_Run(float b_lvl, float b_tm)
{
    SDL_Surface *buf;
    SDL_Event e;
    int leave = 0;
    int coff, cy; // level completed
    int toff, ty; // time bonus
    int soff, sy; // score
    int ms;
    int sw = 80, sh = gm.f_sml->h; // string width, height
    float b_c = 1.0; // bonus change
    float scr = gm.c_prf->scr;
    int end_scr;
    int old_scr;

    end_scr = gm.c_prf->scr + (int)b_lvl + (int)b_tm;

    // normal cursor
    SDL_SetCursor(gm.c_n);

    // darken screen
    buf = SS_Crt(gm.scr_w, gm.scr_h, SDL_SWSURFACE);
    SDL_SetColorKey(buf, 0, 0);
    D_FDST(buf);
    SS_Fill(0x0);
    D_FDST(sdl.scr);
    D_FSRC(buf);
    SS_ABlt(128);
    SDL_FreeSurface(buf);
    D_FDST(gm.s_bkgd);
    D_FSRC(sdl.scr);
    SS_Blt();

    // positions
    cy = 200; coff = 200;
    ty = 220; toff = 200;
    sy = 250; soff = 200;

    // info
    gm.f_sml->algn = TA_X_L | TA_Y_T;
    SF_Wrt(gm.f_sml, sdl.scr, coff, cy, "Level Bonus:", 0);
    SF_Wrt(gm.f_sml, sdl.scr, toff, ty, "Move Bonus:", 0);
    SF_Wrt(gm.f_sml, sdl.scr, soff, sy, "Total Score:", 0);
    Sdl_FUpd();

    // show bonus first time
    gm.f_sml->algn = TA_X_R | TA_Y_T;
    BS_Shw(gm.scr_w - soff, sy, (int)scr);
    Sdl_UpdR();
    SDL_Delay(500);
    BS_Shw(gm.scr_w - coff, cy, (int)b_lvl);
    Sdl_UpdR();
#ifdef SOUND
    sound_play(gm.wv_exp);
#endif
    SDL_Delay(500);
    BS_Shw(gm.scr_w - toff, ty, (int)b_tm);
    Sdl_UpdR();
#ifdef SOUND
    sound_play(gm.wv_exp);
#endif
    SDL_Delay(500);

    T_Rst();
    while ( !leave ) {

        // break?
        if ( SDL_PollEvent(&e) )
            switch ( e.type ) {

                case SDL_QUIT:
                    trm_gm = 1;
                    break;

                case SDL_MOUSEBUTTONUP:
                case SDL_KEYUP:
                    leave = 1;
                    break;

            }

        // time
        ms = T_Gt();

        // hide
        BS_Hd(gm.scr_w - coff - sw, cy, sw, sh);
        BS_Hd(gm.scr_w - toff - sw, ty, sw, sh);
        BS_Hd(gm.scr_w - soff - sw, sy, sw, sh);

        // update
        old_scr = (int)scr;
        if ( b_lvl > 0 ) {

            b_lvl -= b_c * (float)ms;
            scr += b_c * (float)ms;
            if ( b_lvl < 0 )
                b_lvl = 0;

        }
        if ( b_tm > 0 ) {

            b_tm -= b_c * (float)ms;
            scr += b_c * (float)ms;
            if ( b_tm < 0 )
                b_tm = 0;

        }
        if ( (int)scr >= end_scr)
            scr = end_scr;
        if (b_lvl == 0 && b_tm == 0)
            scr = end_scr;
#ifdef SOUND
        if ( (old_scr / 50) != (int)scr / 50 )
            sound_play(gm.wv_scr);
#endif

        // show
        BS_Shw(gm.scr_w - coff, cy, (int)b_lvl);
        BS_Shw(gm.scr_w - toff, ty, (int)b_tm);
        BS_Shw(gm.scr_w - soff, sy, (int)scr);

        Sdl_UpdR();
    }
}

/*
    hide number
*/
void BS_Hd(int x, int y, int w, int h)
{
    D_DST(sdl.scr, x, y, w, h);
    D_SRC(gm.s_bkgd, x, y);
    SS_Blt();
    Sdl_AddR(x, y, w, h);
}

/*
    show number
*/
void BS_Shw(int x, int y, int v)
{
    char str[10];

    sprintf(str, "%i", v);
    SF_Wrt(gm.f_sml, sdl.scr, x, y, str, 0);
    Sdl_AddR(x - SF_TxtW(gm.f_sml, str), y, SF_TxtW(gm.f_sml, str), gm.f_sml->h);
}

// snap shot //
/*
    take a screenshot
*/
void SnapShot()
{
	char filename[32];
#ifdef SOUND
    	sound_play(gm.wv_clk);
#endif
	sprintf(filename, "snapshot_%i.bmp", gm.snap++);
	SDL_SaveBMP(sdl.scr, filename);
}

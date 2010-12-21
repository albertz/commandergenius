/***************************************************************************
                          levels.c  -  description
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "levels.h"
#include "dynlist.h"
#include "game.h"
#include "file.h"
#include "sdl.h"
#include "timer.h"
#include "cfg.h"

/* game struct -- game.c */
extern Game gm;
/* line counter -- file.c */
extern int f_ln;
/* Sdl -- sdl.c */
extern Sdl sdl;
/* profiles -- profile.c */
extern DLst prfs;
/* config -- cfg.c */
extern Cfg cfg;

char **ls_lst = 0;
int  ls_n = 0;
DLst l_sts;

/*
    count and create a list with all loadable filenames found in SRC_DIR/levels
*/
void L_CrtLst()
{
    int     i;
    char    d_nm[256];
    char    path[256+64];
    DIR     *dir = 0;
    struct dirent  *e;
    struct stat     s;

    ls_n = 0;

    // create directory string //
    sprintf(d_nm, "%s/levels", SRC_DIR);

    // find and open directory //
    if ((dir = opendir(d_nm)) == 0) {
        fprintf(stderr, "ERROR: can't find directory '%s'\n", d_nm);
        exit(1);
    }

    printf("searching for level sets...\n");
    // well, let's check the count the entries //
    while ((e = readdir(dir)) != 0) {
        sprintf(path, "%s/%s", d_nm, e->d_name);
        stat(path, &s);
        if (S_ISREG(s.st_mode)) {
            (ls_n)++;
            printf("'%s'\n", e->d_name);
        }
    }

    if (ls_n == 0) {
        fprintf(stderr, "ERROR: '%s' seems to be empty\n", d_nm);
        closedir(dir);
        exit(1);
    }
    else
        printf("...total of %i\n", ls_n);

    // now we'll create the list //
    rewinddir(dir);
    ls_lst = malloc(sizeof(char*) * (ls_n));
    for (i = 0; i < ls_n; i++) {
        do {
            e = readdir(dir);
            if (e == 0) continue;
            sprintf(path, "%s/%s", d_nm, e->d_name);
            stat(path, &s);
        } while (!S_ISREG(s.st_mode));
        ls_lst[i] = malloc(strlen(e->d_name) + 1);
        strcpy(ls_lst[i], e->d_name);
    }

    // close dir //
    closedir(dir);
}

/*
    free list memory
*/
void L_DelLst()
{
    int i;
    if (!ls_lst) return;
    for (i = 0; i < ls_n; i++)
        free(ls_lst[i]);
    free(ls_lst);
}

/*
    callback for dynlist l_sts to delete a level set
*/
void L_DelSt(void *p)
{
    int i;
    LSet *st = (LSet*)p;
    if (st->ch) {
        for (i = 0; i < st->c_num; i++)
            free(st->ch[i].lvls);
        free(st->ch);
    }
    free(p);
}

/*
    add an empty and invalid entry
*/
void L_AddInvSt(char *nm)
{
    LSet *st = malloc(sizeof(LSet));
    st->c_num = 0;
    st->ch = 0;
    st->ok = 0;
    strcpy(st->nm, nm);
    DL_Add(&l_sts, st);
}

/*
    set marble, type and id of a map tile
*/
void L_StMpT(MapT *tl, int m, int t, int id)
{
    tl->m = m;
    tl->t = t;
    tl->id = id;
}

/*
    parse and add a new lset from file f
*/
int L_LdSt(FILE *f)
{
    int i, j, k, l;
    char val[64];
    char str[512];
    LSet *st = malloc(sizeof(LSet));

    // info section //
    F_GetE(f, str, F_SUB | F_VAL);
    if (!F_CkE(str, F_SUB, "<info>", 0)) {
        printf("ERROR: line %i: '<info>' expected\n", f_ln);
        free(st);
        return 0;
    }
    // levels per chapter //
    F_GetE(f, str, F_VAL);
    if (!F_CkE(str, F_VAL, "levels", val)) {
        printf("ERROR: line %i: 'levels' expected\n", f_ln);
        free(st);
        return 0;
    }
    st->l_num = atoi(val);
    // chapters //
    F_GetE(f, str, F_VAL);
    if (!F_CkE(str, F_VAL, "chapters", val)) {
        printf("ERROR: line %i: 'chapters' expected\n", f_ln);
        free(st);
        return 0;
    }
    st->c_num = atoi(val);
    // limit type //
    F_GetE(f, str, F_VAL);
    if (!F_CkE(str, F_VAL, "limit", val)) {
        printf("ERROR: line %i: 'limit' expected\n", f_ln);
        free(st);
        return 0;
    }
    if ( !strncmp( "time", val, 4 ) )
        st->limit_type = TIME;
    else
        st->limit_type = MOVES;
    // info section //
    F_GetE(f, str, F_SUB | F_VAL);
    if (!F_CkE(str, F_SUB, "</info>", 0)) {
        printf("ERROR: line %i: '</info>' expected\n", f_ln);
        free(st);
        return 0;
    }

    // get memory
    st->ch = malloc( sizeof(Chptr) * st->c_num );
    for (i = 0; i < st->c_num; i++)
        st->ch[i].lvls = malloc( sizeof(Lvl) * st->l_num );

    // chapters
    for (i = 0; i < st->c_num; i++) {
        // chapter start //
        F_GetE(f, str, F_SUB | F_VAL);
        if (!F_CkE(str, F_SUB, "<chapter>", 0)) {
            printf("ERROR: line %i: '<chapter>' expected\n", f_ln);
            goto failure;
        }
        // name //
        F_GetE(f, str, F_VAL);
        if (!F_CkE(str, F_VAL, "name", st->ch[i].nm)) {
            printf("ERROR: line %i: 'name' expected\n", f_ln);
            goto failure;
        }
        // author //
        F_GetE(f, str, F_VAL);
        if (!F_CkE(str, F_VAL, "author", st->ch[i].authr)) {
            printf("ERROR: line %i: 'author' expected\n", f_ln);
            goto failure;
        }
        // gset //
        F_GetE(f, str, F_VAL);
        if (!F_CkE(str, F_VAL, "gfx_set", st->ch[i].g_st)) {
            printf("ERROR: line %i: 'gfx_set' expected\n", f_ln);
            goto failure;
        }
        // open for play ? //
        F_GetE(f, str, F_VAL);
        if (!F_CkE(str, F_VAL, "open", val)) {
            printf("ERROR: line %i: 'open' expected\n", f_ln);
            goto failure;
        }
        st->ch[i].opn = atoi(val);

        // levels //
        for (j = 0; j < st->l_num; j++) {
            // level start //
            F_GetE(f, str, F_SUB | F_VAL);
            if (!F_CkE(str, F_SUB, "<level>", 0)) {
                printf("ERROR: line %i: '<level>' expected\n", f_ln);
                goto failure;
            }
            // time //
            F_GetE(f, str, F_VAL);
            if (!F_CkE(str, F_VAL, "limit", val)) {
                printf("ERROR: line %i: 'limit' expected\n", f_ln);
                goto failure;
            }
            st->ch[i].lvls[j].tm = atoi(val);
            // map width //
            F_GetE(f, str, F_VAL);
            if (!F_CkE(str, F_VAL, "map_w", val)) {
                printf("ERROR: line %i: 'map_w' expected\n", f_ln);
                goto failure;
            }
            if ((st->ch[i].lvls[j].m_w = atoi(val)) > L_MAX_W || atoi(val) < L_MIN_W) {
                printf("ERROR: line %i: 'map_w' out of range (%i-%i): %i\n", f_ln, L_MIN_W, L_MAX_W, atoi(val));
                goto failure;
            }
            // map height //
            F_GetE(f, str, F_VAL);
            if (!F_CkE(str, F_VAL, "map_h", val)) {
                printf("ERROR: line %i: 'map_h' expected\n", f_ln);
                goto failure;
            }
            if ((st->ch[i].lvls[j].m_h = atoi(val)) > L_MAX_H || atoi(val) < L_MIN_H) {
                printf("ERROR: line %i: 'map_w' out of range (%i-%i): %i\n", f_ln, L_MIN_H, L_MAX_H, atoi(val));
                goto failure;
            }
            // figure width //
            F_GetE(f, str, F_VAL);
            if (!F_CkE(str, F_VAL, "fig_w", val)) {
                printf("ERROR: line %i: 'fig_w' expected\n", f_ln);
                goto failure;
            }
            st->ch[i].lvls[j].f_w = atoi(val);
            if ((st->ch[i].lvls[j].f_w = atoi(val)) > F_MAX_W || atoi(val) < F_MIN_W) {
                printf("ERROR: line %i: 'fig_w' out of range (%i-%i): %i\n", f_ln, F_MIN_W, F_MAX_W, atoi(val));
                goto failure;
            }
            // figure height //
            F_GetE(f, str, F_VAL);
            if (!F_CkE(str, F_VAL, "fig_h", val)) {
                printf("ERROR: line %i: 'fig_h' expected\n", f_ln);
                goto failure;
            }
            if ((st->ch[i].lvls[j].f_h = atoi(val)) > F_MAX_H || atoi(val) < F_MIN_H) {
                printf("ERROR: line %i: 'fig_h' out of range (%i-%i): %i\n", f_ln, F_MIN_H, F_MAX_H, atoi(val));
                goto failure;
            }

            // figure start //
            F_GetE(f, str, F_SUB | F_VAL);
            if (!F_CkE(str, F_SUB, "<figure>", 0)) {
                printf("ERROR: line %i: '<figure>' expected\n", f_ln);
                goto failure;
            }
            // figure //
            for (k = 0; k < st->ch[i].lvls[j].f_h; k++) {
                F_GetE(f, str, F_VAL);
                for (l = 0; l < st->ch[i].lvls[j].f_w; l++) {
                    if (str[l] >= '0' && str[l] <= '9')
                        st->ch[i].lvls[j].fgr[l][k] = str[l] - 48;
                    else
                        if (str[l] != 32)
                            printf("WARNING: line %i: bad figure marble '%c'\n", f_ln, str[l]);
                        else
                            st->ch[i].lvls[j].fgr[l][k] = -1;
                }
            }
            // figure end //
            F_GetE(f, str, F_SUB | F_VAL);
            if (!F_CkE(str, F_SUB, "</figure>", 0)) {
                printf("ERROR: line %i: '</figure>' expected\n", f_ln);
                goto failure;
            }

            // map start //
            F_GetE(f, str, F_SUB | F_VAL);
            if (!F_CkE(str, F_SUB, "<map>", 0)) {
                printf("ERROR: line %i: '<map>' expected\n", f_ln);
                goto failure;
            }
            // map
            for (k = 0; k < st->ch[i].lvls[j].m_h; k++) {
                memset(str, 0, 256);
                F_GetE(f, str, F_VAL);
                for (l = 0; l < st->ch[i].lvls[j].m_w; l++) {
                    if (str[l] >= '0' && str[l] <= '9')
                        L_StMpT(&st->ch[i].lvls[j].map[l][k], str[l] - 48, M_FLOOR, 0);
                    else
                        switch (str[l]) {
                            case 'a':
                            case 'b':
                            case 'c':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_WALL, str[l] - 97);
                                break;
                            case ' ':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_FLOOR, 0);
                                break;
                            case 'u':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_OW_U, 0);
                                break;
                            case 'd':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_OW_D, 0);
                                break;
                            case 'r':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_OW_R, 0);
                                break;
                            case 'l':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_OW_L, 0);
                                break;
                            case 'U':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_OW_U_C, 0);
                                break;
                            case 'D':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_OW_D_C, 0);
                                break;
                            case 'R':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_OW_R_C, 0);
                                break;
                            case 'L':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_OW_L_C, 0);
                                break;
                            case 'w':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_TLP_0, 0);
                                break;
                            case 'x':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_TLP_1, 0);
                                break;
                            case 'y':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_TLP_2, 0);
                                break;
                            case 'z':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_TLP_3, 0);
                                break;
                            case 'A':
                            case 'B':
                            case 'C':
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_CRUMBLE, str[l] - 65);
                                break;
                            default:
                                L_StMpT(&st->ch[i].lvls[j].map[l][k], -1, M_EMPTY, 0);
                        break;
                        }
                }
            }
            // map end //
            F_GetE(f, str, F_SUB | F_VAL);
            if (!F_CkE(str, F_SUB, "</map>", 0)) {
                printf("ERROR: line %i: '</map>' expected\n", f_ln);
                goto failure;
            }

            // level end //
            F_GetE(f, str, F_SUB | F_VAL);
            if (!F_CkE(str, F_SUB, "</level>", 0)) {
                printf("ERROR: line %i: '</level>' expected\n", f_ln);
                goto failure;
            }
        }

        // chapter end //
        F_GetE(f, str, F_SUB);
        if (!F_CkE(str, F_SUB, "</chapter>", 0)) {
            printf("ERROR: line %i: '</chapter>' expected\n", f_ln);
            goto failure;
        }
    }

    // return 1 in any case cuz 0 assumes that an empty set must be added
    printf("ok\n");
    DL_Add(&l_sts, st);
    st->ok = 1;
    return 1;

failure:
    DL_Add(&l_sts, st);
//    L_DelSt(st);
    st->ok = 0;
    return 1;
}

/*
    initialize level l
*/
void L_Ini(int c, int l)
{
    char *nm;
    DL_E *e;
    int gst_ok = 0;
    int px, py, i, j, k;
    Lvl *lp;
    char str[64];
    int off;
    char *str_num[] = {"1.", "2.", "3.", "4.", "5.", "6.", "7.", "8.", "9.", "10."};
    float mv_mod;
        
    // show credit when new chapter
    if ( gm.o_ch == -1 || gm.o_ch != c )
        Cr_Ini();

    // set level & chapter  index
    gm.o_ch = c;
    gm.c_ch = c;
    gm.c_l_id = l;
    gm.l_done = 0;

    // set open flag
    gm.c_s_inf->c_opn[gm.c_ch] = 1;

    // copy current level
    if (gm.c_lvl) free(gm.c_lvl);
    gm.c_lvl = malloc(sizeof(Lvl));
    memcpy(gm.c_lvl, &gm.c_l_st->ch[gm.c_ch].lvls[gm.c_l_id], sizeof(Lvl));

    // load gfx set
    nm = gm.c_l_st->ch[gm.c_ch].g_st;
    e = gm.g_sts.hd.n;
    while (e != &gm.g_sts.tl) {
        if (!strncmp(nm, ((GSet*)e->d)->nm, strlen(nm)) && ((GSet*)e->d)->ok) {
            gm.c_g_st = (GSet*)e->d;
            gst_ok = 1;
            break;
        }
        e = e->n;
    }
    if (!gst_ok) {
        printf("WARNING: unknown or unuseable gfx set '%s';\nsearching for a good one...\n", nm);
        // find first useable set
        e = gm.g_sts.hd.n;
        while (e != &gm.g_sts.tl) {
            if (((GSet*)e->d)->ok) {
                gm.c_g_st = (GSet*)e->d;
                printf("'%s' used instead\n", gm.c_g_st->nm);
                gst_ok = 1;
                break;
            }
            e = e->n;
        }
        if (!gst_ok) {
            printf("ERROR: no good gfx set found...\n");
            exit(1);
        }
    }

    // set bkgd picture
    if (gm.c_g_st->s_bkgd) {
        for (j = 0; j < gm.scr_w - gm.brd_w; j += gm.c_g_st->s_bkgd->w)
            for (k = 0; k < gm.scr_h; k += gm.c_g_st->s_bkgd->h) {
                D_DST(gm.s_bkgd, j, k, gm.c_g_st->s_bkgd->w, gm.c_g_st->s_bkgd->h);
                D_SRC(gm.c_g_st->s_bkgd, 0, 0);
                SS_Blt();
            }
        }
    else {
        D_DST(gm.s_bkgd, 0, 0, gm.scr_w - gm.brd_w, gm.scr_h);
        SS_Fill(0x0);
    }

    // add board
    D_DST(gm.s_bkgd, gm.s_bkgd->w - gm.s_brd->w, 0, gm.s_brd->w, gm.s_brd->h);
    D_SRC(gm.s_brd, 0, 0);
    SS_Blt();

    // add static level gfx
    lp = &gm.c_l_st->ch[gm.c_ch].lvls[gm.c_l_id];
    gm.l_x = (gm.scr_w - gm.brd_w - lp->m_w * gm.t_w) / 2;
    gm.l_y = (gm.scr_h - lp->m_h * gm.t_h) / 2;
    for (i = 0; i < lp->m_w; i++)
        for (j = 0; j < lp->m_h; j++)
            L_DrwMpTl(i, j);

    // add marbles
    for (i = 0; i < lp->m_w; i++)
        for (j = 0; j < lp->m_h; j++)
            if (lp->map[i][j].m != - 1) {
                D_DST(gm.s_bkgd, gm.l_x + i * gm.t_w, gm.l_y + j * gm.t_h, gm.t_w, gm.t_h);
                D_SRC(gm.s_mrb, 0, lp->map[i][j].m * gm.t_h);
                SS_Blt();
            }

    // add figure
    px = gm.b_x + gm.f_x + (gm.f_fw - lp->f_w * gm.f_h) / 2;
    py = gm.f_y + (gm.f_fh - lp->f_h * gm.f_h)/ 2;
    for (i = 0; i < lp->f_w; i++)
        for (j = 0; j < lp->f_h; j++)
            if (lp->fgr[i][j] != -1) {
                D_DST(gm.s_bkgd, px + i * gm.f_w, py + j * gm.f_h, gm.f_w, gm.f_h);
                D_SRC(gm.s_fig, 0, lp->fgr[i][j] * gm.f_h);
                SS_Blt();
            }

    // add player info
    off = gm.f_sml->h + 2;
    gm.f_sml->algn = TA_X_L | TA_Y_T;
    SF_Wrt(gm.f_sml, gm.s_bkgd, gm.i_x + gm.b_x, gm.i_y, "Player:", 0);
    gm.f_sml->algn = TA_X_R | TA_Y_T;
    SF_Wrt(gm.f_sml, gm.s_bkgd, gm.scr_w - gm.i_x, gm.i_y, gm.c_prf->nm, 0);
    gm.f_sml->algn = TA_X_L | TA_Y_T;
    SF_Wrt(gm.f_sml, gm.s_bkgd, gm.i_x + gm.b_x, gm.i_y + off, "Score:", 0);
    gm.f_sml->algn = TA_X_R | TA_Y_T;
    sprintf(str, "%i", gm.c_prf->scr);
    SF_Wrt(gm.f_sml, gm.s_bkgd, gm.scr_w - gm.i_x, gm.i_y + off, str, 0);
    gm.f_sml->algn = TA_X_L | TA_Y_T;
    SF_Wrt(gm.f_sml, gm.s_bkgd, gm.i_x + gm.b_x, gm.i_y + off*2 + 5, "HighScore:", 0);
    gm.f_sml->algn = TA_X_R | TA_Y_T;
    sprintf(str, "%i", gm.hi_scr);
    SF_Wrt(gm.f_sml, gm.s_bkgd, gm.scr_w - gm.i_x, gm.i_y + off*2 + 5, str, 0);

    // add name of levelset above set info
    gm.f_sml->algn = TA_X_C | TA_Y_T;
    SF_Wrt(gm.f_sml, gm.s_bkgd, gm.b_x + gm.brd_w / 2, gm.s_y + 5, gm.c_l_st->nm, 0);

    // compute position of first chapter
    gm.c_x = 50; // changed
    gm.c_y = gm.s_y + (gm.s_h - gm.c_l_st->c_num * L_SIZE) / 2;

    // add chapter numbers
    gm.f_sml->algn = TA_X_L | TA_Y_T;
    for (i = 0; i < gm.c_l_st->c_num; i++) {
         SF_Wrt(gm.f_sml, gm.s_bkgd, gm.b_x + gm.c_x - 30, gm.c_y + i * L_SIZE, str_num[i], 0);
    }

    // draw chapter lights
    for (j = 0; j < gm.c_l_st->c_num; j++)
        for (i = 0; i < gm.c_l_st->l_num; i++) {
            D_DST(gm.s_bkgd, gm.b_x + gm.c_x + i * L_SIZE, gm.c_y + j * L_SIZE, L_SIZE, L_SIZE);
            if (!gm.c_s_inf->c_opn[j]) {
                D_SRC(gm.s_lghts, L_RED * L_SIZE, 0);
            }
            else
                if ( gm.c_s_inf->cmp[j * gm.c_l_st->l_num + i] ) {
                    D_SRC(gm.s_lghts, L_GREEN * L_SIZE, 0);
                }
                else {
                    D_SRC(gm.s_lghts, L_ORANGE * L_SIZE, 0);
                }
            SS_Blt();
        }
    // current level is white
    D_DST(gm.s_bkgd, gm.b_x + gm.c_x + l * L_SIZE, gm.c_y + c * L_SIZE, L_SIZE, L_SIZE);
    D_SRC(gm.s_lghts, L_WHITE * L_SIZE, 0);
    SS_Blt();

    // initiate animations

    MA_Ini();

    // undim
    D_FDST(sdl.scr);
    D_FSRC(gm.s_bkgd);
    SS_Blt();
    if (cfg.dim)
        SDL_UNDIM();
    else
        Sdl_FUpd();

    // reset timer
    T_Rst();

    // reset marble and frame
    gm.m_mv = 0;
    gm.m_sel = 0;
    gm.mf_a.p = 0;
    gm.msf_a.p = 0;
    gm.m_act = M_EMPTY;
    gm.m_mx = gm.m_my = -1;

    if ( gm.c_l_st->limit_type == TIME ) {
        /* set time from seconds to milliseconds if time used */
        gm.c_lvl->tm *= 1000;
        gm.c_lvl->tm+=1000;
    }
    else {
        /* gm.c_lvl->tm containts the move limit. this is modified according
           to the difficulty levels */
        switch ( cfg.diff ) {
            case DIFF_EASY:
                mv_mod = (int)ceil((float)gm.c_lvl->tm * 0.2 * ( 5 - gm.c_ch ) );
                gm.c_lvl->tm += mv_mod;
                if ( gm.c_lvl->tm % 2 ) gm.c_lvl->tm++;
                break;
            case DIFF_NORMAL:
                mv_mod = (int)ceil((float)gm.c_lvl->tm * 0.1 * ( 5 - gm.c_ch ) );
                gm.c_lvl->tm += mv_mod;
                if ( gm.c_lvl->tm % 2 ) gm.c_lvl->tm++;
                break;
            case DIFF_HARD:
                mv_mod = (int)ceil((float)gm.c_lvl->tm * 0.05 * ( 5 - gm.c_ch ) );
                gm.c_lvl->tm += mv_mod;
                if ( gm.c_lvl->tm % 2 ) gm.c_lvl->tm++;
                break;
            case DIFF_BRAINSTORM:
                break;
        }
    }

    // init blink time //
    gm.blink_time = 0;

    // cursor state
    gm.c_stat = C_NONE;
    SDL_SetCursor(gm.c_n);

    // reset shrapnells
    DL_Clr(&gm.shr);
}

/*
    draw a map tile to gm.s_bkgd with x,y position in the map using current map
*/
void L_DrwMpTl(int i, int j)
{
    switch (gm.c_lvl->map[i][j].t) {
        case M_WALL:
            // add wall shadow on right side ? //
            if ( i + 1 >= gm.c_lvl->m_w || gm.c_lvl->map[i + 1][j].t == M_EMPTY ) {

                D_DST(gm.s_bkgd, gm.l_x + (i + 1) * gm.t_w, gm.l_y + j * gm.t_h, 1, gm.t_h + 1);
                SS_Fill(0x0);

            }
            // add wall shadow on bottom ? //
            if ( j + 1 >= gm.c_lvl->m_h || gm.c_lvl->map[i][j + 1].t == M_EMPTY ) {

                D_DST(gm.s_bkgd, gm.l_x + i * gm.t_w, gm.l_y + (j + 1) * gm.t_h, gm.t_w + 1, 1);
                SS_Fill(0x0);

            }
            // add wall shadow on left side ? //
            if ( (i == 0) 
	       || ((i - 1 >= gm.c_lvl->m_w) || gm.c_lvl->map[i - 1][j].t == M_EMPTY )) {
                D_DST(gm.s_bkgd, gm.l_x + i * gm.t_w - 1, gm.l_y + j * gm.t_h, 1, gm.t_h + 1);
                SS_Fill(0x0);

            }
            // add wall shadow on top ? //
	    if ( (j == 0) 
	       || ((j - 1 >= gm.c_lvl->m_h) || gm.c_lvl->map[i][j - 1].t == M_EMPTY )) {
                D_DST(gm.s_bkgd, gm.l_x + i * gm.t_w, gm.l_y + j * gm.t_h - 1, gm.t_w + 1, 1);
                SS_Fill(0x0);

            }
            D_DST(gm.s_bkgd, gm.l_x + i * gm.t_w, gm.l_y + j * gm.t_h, gm.t_w, gm.t_h);
            D_SRC(gm.c_g_st->s_wl, gm.c_lvl->map[i][j].id * gm.t_w, 0);
            SS_Blt();
            break;
        case M_CRUMBLE:

            D_DST(gm.s_bkgd, gm.l_x + i * gm.t_w, gm.l_y + j * gm.t_h, gm.t_w, gm.t_h);
            D_SRC(gm.c_g_st->s_flr, 0, 0);
            SS_Blt();

            D_DST(gm.s_bkgd, gm.l_x + i * gm.t_w, gm.l_y + j * gm.t_h, gm.t_w, gm.t_h);
            D_SRC(gm.c_g_st->s_crmbl, gm.c_lvl->map[i][j].id * gm.t_w, 0);
            SS_Blt();

            gm.f_sml->algn = TA_X_R | TA_Y_B;
            switch (gm.c_lvl->map[i][j].id) {
                case 0:
                    SF_Wrt(gm.f_sml, gm.s_bkgd, gm.l_x + (i + 1) * gm.t_w - 1, gm.l_y + (j + 1) * gm.t_h - 1, "1", 0);
                break;
                case 1:
                    SF_Wrt(gm.f_sml, gm.s_bkgd, gm.l_x + (i + 1) * gm.t_w - 1, gm.l_y + (j + 1) * gm.t_h - 1, "2", 0);
                break;
                case 2:
                    SF_Wrt(gm.f_sml, gm.s_bkgd, gm.l_x + (i + 1) * gm.t_w - 1, gm.l_y + (j + 1) * gm.t_h - 1, "3", 0);
                break;
            }

            break;
        case M_EMPTY:
            break;
        default:
            D_DST(gm.s_bkgd, gm.l_x + i * gm.t_w, gm.l_y + j * gm.t_h, gm.t_w, gm.t_h);
            D_SRC(gm.c_g_st->s_flr, 0, 0);
            SS_Blt();
            break;
    }

    // static animations ? //
    if (!cfg.ani) {
        D_DST(gm.s_bkgd, gm.l_x + i * gm.t_w, gm.l_y + j * gm.t_h, gm.t_w, gm.t_h);
        switch (gm.c_lvl->map[i][j].t) {
            case M_OW_U:
            case M_OW_U_C:
                D_SRC(gm.c_g_st->s_u_arw, 0, 0);
                SS_Blt();
                break;
            case M_OW_D:
            case M_OW_D_C:
                D_SRC(gm.c_g_st->s_d_arw, 0, 0);
                SS_Blt();
                break;
            case M_OW_L:
            case M_OW_L_C:
                D_SRC(gm.c_g_st->s_l_arw, 0, 0);
                SS_Blt();
                break;
            case M_OW_R:
            case M_OW_R_C:
                D_SRC(gm.c_g_st->s_r_arw, 0, 0);
                SS_Blt();
                break;
        }
        D_DST(gm.s_bkgd, gm.l_x + i * gm.t_w, gm.l_y + j * gm.t_h, gm.t_w, gm.t_h);
        switch (gm.c_lvl->map[i][j].t) {
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

        D_DST(gm.s_bkgd, gm.l_x + i * gm.t_w, gm.l_y + j * gm.t_h, gm.t_w, gm.t_h);
        switch (gm.c_lvl->map[i][j].t) {
            case M_TLP_0:
                D_SRC(gm.c_g_st->s_tlp_0, 0, 0);
                SS_ABlt(gm.tlp_a);
                break;
            case M_TLP_1:
                D_SRC(gm.c_g_st->s_tlp_1, 0, 0);
                SS_ABlt(gm.tlp_a);
                break;
            case M_TLP_2:
                D_SRC(gm.c_g_st->s_tlp_2, 0, 0);
                SS_ABlt(gm.tlp_a);
                break;
            case M_TLP_3:
                D_SRC(gm.c_g_st->s_tlp_3, 0, 0);
                SS_ABlt(gm.tlp_a);
                break;
        }
    }
}

/*
    proceed to next valid level
*/
int L_FndNxt()
{
    int i, j;

    // proceed to next level
    gm.c_l_id++;
    if (gm.c_l_id >= gm.c_l_st->l_num) {
        gm.c_l_id = 0;
        gm.c_ch++;
        // check if this chapter can be entered or if this was the last chapter if not take first unsolved level found
        if (gm.c_ch >= gm.c_l_st->c_num || !gm.c_l_st->ch[gm.c_ch].opn) {
            for (i = 0; i < gm.c_ch; i++)
                for (j = 0; j < gm.c_l_st->l_num; j++)
                    if (!gm.c_s_inf->cmp[i * gm.c_l_st->l_num + j]) {
                        // not all levels completed jump back to first unsolved level
                        gm.c_ch = i;
                        gm.c_l_id = j;
                        return 1;
                    }
            if (gm.c_ch >= gm.c_l_st->c_num)
                return 0;
            else
                return 1;
        }
    }
    else {
        // check if all levels of this chapter has been solved
        for (i = 0; i <= gm.c_ch; i++)
            for (j = 0; j < gm.c_l_st->l_num; j++)
                if (!gm.c_s_inf->cmp[i * gm.c_l_st->l_num + j]) {
                    return 1;
                }
        // yes! open next chapter
        if ( gm.c_ch < gm.c_l_st->c_num - 1 )
            gm.c_s_inf->c_opn[gm.c_ch + 1] = 1;
    }
    return 1;
}

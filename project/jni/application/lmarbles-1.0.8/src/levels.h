/***************************************************************************
                          levels.h  -  description
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

#ifndef LEVELS_H
#define LEVELS_H

#include <stdio.h>

#define MAX_LVLS    10
#define MAX_CHPTRS  10
#define L_MAX_W     13
#define L_MIN_W     4
#define L_MAX_H     14
#define L_MIN_H     4
#define F_MAX_W     10
#define F_MIN_W     2
#define F_MAX_H     10
#define F_MIN_H     1

// level bonuses //
#define LB_COMPLETED    1000
#define LB_PER_MOVE     50

// limit type //
#define TIME 0
#define MOVES 1

// tile types //
enum {
    M_EMPTY = 0, // background //
    M_FLOOR, // simple floor //
    M_OW_U_C, // oneway directed to north (up) closed to the left and right side //
    M_OW_D_C, // analog //
    M_OW_L_C, // analog //
    M_OW_R_C, // analog //
    M_OW_U, // oneway to north not closed at the sides //
    M_OW_D, // analog //
    M_OW_L, // analog //
    M_OW_R, // analog //
    M_WALL, // impassible wall //
    M_TLP_0, // teleporter //
    M_TLP_1, // analog //
    M_TLP_2, // analog //
    M_TLP_3, // analog //
    M_CRUMBLE, // crumbling wall //
    M_NUM
};

typedef struct {
    int m; // marble //
    int t; // type //
    int id;
} MapT;

typedef struct {
    int     tm; // time //
    int     m_w, m_h; // map size //
    int     f_w, f_h; // figure size //
    int     fgr[F_MAX_W][F_MAX_H]; // figure //
    MapT    map[L_MAX_W][L_MAX_H]; // map //
} Lvl;

typedef struct {
    char    nm[64];
    char    authr[32];
    char    g_st[32];
    int     bkgd;
    int     opn; // levels from this chapter can be played
    Lvl     *lvls;
} Chptr;

typedef struct {
    char    nm[64];
    int     c_num;
    int     l_num;
    Chptr   *ch;
    int     ok;
    int     limit_type; /* which type of limit? */
} LSet;

void L_CrtLst();
void L_DelLst();
void L_DelSt(void *p);
void L_AddInvSt(char *nm);
int L_LdSt(FILE *f);
void L_Ini(int c, int l);
void L_DrwMpTl(int x, int y);
int  L_FndNxt();

#endif

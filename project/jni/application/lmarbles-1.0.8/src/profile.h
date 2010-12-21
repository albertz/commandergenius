/***************************************************************************
                          profile.h  -  description
                             -------------------
    begin                : Sun Sep 17 2000
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

#ifndef PROFILE_H
#define PROFILE_H

#include "levels.h"
#include "dynlist.h"

typedef struct {
    char    nm[32];
    int     num;
    int     l_num; // levels per chapter //
    int     c_num;
    char    c_opn[MAX_CHPTRS];
    char    cmp[MAX_LVLS * MAX_CHPTRS];
} SInf;

typedef struct {
    char    nm[12];
    int     lvls;
    int     scr; // total score gained
    float   pct; // percentage of time needed to complete a level
    DLst    sts;
} Prf;

void Prf_Ini();
void Prf_Trm();
int Prf_Ld();
void Prf_Sv();
void Prf_Crt(char *nm);
void Prf_Del(void *p);
SInf* Prf_RegLS(Prf *p, LSet *l_st);
void Prf_CrtLst();
void Prf_DelLst();
void Prf_Srt();
void Prf_Upd(Prf *p, SInf *inf, int l_id, float pct, int scr);

#endif

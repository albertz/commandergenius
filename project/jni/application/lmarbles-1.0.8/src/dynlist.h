/***************************************************************************
                          dynlist.h  -  description
                             -------------------
    begin                : Sat Apr 8 2000
    copyright            : (C) 2000 by
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

#ifndef DYNLIST_H
#define DYNLIST_H

#ifdef __cplusplus
extern "C" {
#endif

//dl flags
#define DL_NONE     (0)
#define DL_AUTODEL  (1L<<0)
#define DL_NOCB     (1L<<1)

typedef struct _DL_E {
    struct _DL_E    *n;
    struct _DL_E    *p;
    void            *d;
} DL_E;

typedef struct {
    unsigned int    flgs;
    unsigned int    cntr; //don't edit
    DL_E        hd; //don't edit
    DL_E        tl; //don't edit
    void            (*cb)(void*);
} DLst;

void DL_Ini(DLst *dlst);
int  DL_Ins(DLst *dlst, unsigned int i, void *item); //insert item
int  DL_Add(DLst *dlst, void *item); //insert at the end
int  DL_DelE(DLst *dlst, DL_E *e);
int  DL_DelP(DLst *dlst, void *item);
int  DL_Del(DLst *dlst, unsigned int i);
void* DL_Get(DLst *dlst, int i);
DL_E *DL_GetE(DLst *dlst, void *item);
void DL_Clr(DLst *dlst); //clear full list

#ifdef __cplusplus
};
#endif

#endif

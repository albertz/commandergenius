/***************************************************************************
                          profile.c  -  description
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

#include "profile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cfg.h"
#include "file.h"

// profiles //
DLst prfs;
// string list for the menu //
char **prf_lst = 0;
int  prf_n = 0;
// profile source path //
char prf_pth[256];
// configuration //
extern Cfg cfg;

/* initialize dyn list and source path */
void Prf_Ini()
{
    DL_Ini(&prfs);
    prfs.flgs = DL_AUTODEL;
    prfs.cb = Prf_Del;

    sprintf(prf_pth, "%s/lmarbles.prfs", PRF_DIR);
}

/* terminate profiles; must be saved first */
void Prf_Trm()
{
    Prf_DelLst();
    DL_Clr(&prfs);
}

/* load profiles */
int Prf_Ld()
{
    Prf *p;
    SInf *st;
    int s_num = 0, p_num = 0;
    FILE    *f = 0;
    int  not_f = 0;
    char    str[256];
#ifdef ASCII
    int     i;
#endif

    printf("loading profiles... ");

    // read access ? //
    if ((f = fopen(prf_pth, "r")) == 0) {
        printf("\nWARNING: file %s does not exist; cannot read profiles\n", prf_pth);
        Prf_Crt("Michael");
        not_f = 1;
    }
    else {
        /* load ascii identification */
#ifdef ASCII
        F_GetE(f, str, F_VAL); str[strlen(str) - 1] = 0;
        if (strncmp(str,"ascii",5)) {
            printf("\nWARNING: trying to load raw binary data in ascii; cannot read profiles\n", prf_pth);
            Prf_Crt("Michael");
            not_f = 1;
        }
        else {
#else
        fread(str, 5, 1, f);
        if (!strncmp(str,"ascii",5)) {
            printf("\nWARNING: trying to load in ascii instead of raw binary format; cannot read profiles\n");
            Prf_Crt("Michael");
            not_f = 1;
        }
        else {
            fseek(f, SEEK_SET, 0);
#endif
        /* load numbers of profiles */
#ifdef ASCII
        F_GetE(f, str, F_VAL); F_ValToInt(str, &p_num);
#else
        fread((void*)&p_num, sizeof(int), 1, f);
#endif
        if (p_num <= 0) {
            printf("WARNING: bad profile counter: %i\n", p_num);
            Prf_Crt("Michael");
            not_f = 1;
        }
        else {
            while (p_num--) {
                 p = malloc(sizeof(Prf));
#ifdef ASCII
                 /* name */
                 F_GetE(f, p->nm, F_VAL); p->nm[strlen(p->nm) - 1] = 0;
                 /* levels played */
                 F_GetE(f, str, F_VAL); F_ValToInt(str, &p->lvls);
                 /* score */
                 F_GetE(f, str, F_VAL); F_ValToInt(str, &p->scr);
                 /* percentage */
                 F_GetE(f, str, F_VAL); F_ValToFloat(str, &p->pct);
                 /* number of levelsets */
                 F_GetE(f, str, F_VAL); F_ValToInt(str, &s_num);
#else
                 /* name */
                 fread(p->nm, sizeof(p->nm), 1, f);
                 /* levels played */
                 fread(&p->lvls, sizeof(p->lvls), 1, f);
                 /* score */
                 fread(&p->scr, sizeof(p->scr), 1, f);
                 /* percentage */
                 fread(&p->pct, sizeof(p->pct), 1, f);
                 /* number of levelsets */
                 fread(&s_num, sizeof(int), 1, f);
#endif
                 DL_Ini(&p->sts);
                 p->sts.flgs = DL_AUTODEL | DL_NOCB;
                 if (s_num >= 0)
                     while (s_num--) {
                         /* level sets */
                         st = malloc(sizeof(SInf));
#ifdef ASCII
                         F_GetE(f, st->nm, F_VAL); st->nm[strlen(st->nm) - 1] = 0;
                         F_GetE(f, str, F_VAL); F_ValToInt(str, &st->num);
                         F_GetE(f, str, F_VAL); F_ValToInt(str, &st->l_num);
                         F_GetE(f, str, F_VAL); F_ValToInt(str, &st->c_num);
                         for ( i = 0; i < st->c_num; i++) {
                             F_GetE(f, str, F_VAL); F_ValToChar(str, &st->c_opn[i]);
                         }
                         for ( i = 0; i < st->num; i++) {
                             F_GetE(f, str, F_VAL); F_ValToChar(str, &st->cmp[i]);
                         }
#else
                         fread(st->nm, sizeof(st->nm), 1, f);
                         fread(&st->num, sizeof(int), 1, f);
                         fread(&st->l_num, sizeof(int), 1, f);
                         fread(&st->c_num, sizeof(int), 1, f);
                         fread(st->c_opn, sizeof(char), st->c_num, f);
                         fread(st->cmp, sizeof(char), st->num, f);
#endif
                         DL_Add(&p->sts, st);
                     }
                 DL_Add(&prfs, p);
            }
        }
        printf("ok\n");
#ifdef ASCII
        }
#else
        }
#endif
        fclose(f);
    }

    Prf_CrtLst();
    return !not_f;
}

/* save profiles */
void Prf_Sv()
{
    DL_E *e = prfs.hd.n, *le = 0;
    Prf *p;
    SInf *st;
    FILE *f;
#ifdef ASCII
    int i;
    char str[256];
#endif

    printf("saving profiles... ");
    if ((f = fopen(prf_pth, "w")) == 0) {
        printf("WARNING: no write access to %s\n", prf_pth);
    }
    else {
        /* save ascii identification */
#ifdef ASCII
        F_WrtE(f, "ascii");
#endif
        /* save numbers of profiles */
#ifdef ASCII
        F_IntToStr(str, prfs.cntr); F_WrtE(f, str);
#else
        fwrite((void*)&prfs.cntr, sizeof(int), 1, f);
#endif
        while (e != &prfs.tl) {
            p = (Prf*)e->d;
#ifdef ASCII
            /* save name -- 12 chars */
            F_WrtE(f, p->nm);
            /* levels played */
            F_IntToStr(str, p->lvls); F_WrtE(f, str);
            /* score */
            F_IntToStr(str, p->scr); F_WrtE(f, str);
            /* percentage */
            F_FloatToStr(str, p->pct); F_WrtE(f, str);
            /* save number of levelsets */
            F_IntToStr(str, p->sts.cntr); F_WrtE(f, str);
#else
            /* save name -- 12 chars */
            fwrite(p->nm, sizeof(p->nm), 1, f);
            /* levels played */
            fwrite(&p->lvls, sizeof(p->lvls), 1, f);
            /* score */
            fwrite(&p->scr, sizeof(p->scr), 1, f);
            /* percentage */
            fwrite(&p->pct, sizeof(p->pct), 1, f);
            /* save number of levelsets */
            fwrite(&p->sts.cntr, sizeof(int), 1, f);
#endif
            /* save all levelsets */
            le = p->sts.hd.n;
            while (le != &p->sts.tl) {
                st = (SInf*)le->d;
#ifdef ASCII
                /* save name -- 32 chars */
                F_WrtE(f, st->nm);
                /* save number of flags */
                F_IntToStr(str, st->num); F_WrtE(f, str);
                /* save level peer chapter number */
                F_IntToStr(str, st->l_num); F_WrtE(f, str);
                /* save chapter number */
                F_IntToStr(str, st->c_num); F_WrtE(f, str);
                /* save chapter open flags */
                for ( i = 0; i < st->c_num; i++ ) {
                    F_IntToStr(str, st->c_opn[i]); F_WrtE(f, str);
                }
                /* save flags */
                for ( i = 0; i < st->num; i++ ) {
                    F_IntToStr(str, st->cmp[i]); F_WrtE(f, str);
                }
#else
                /* save name -- 32 chars */
                fwrite(st->nm, sizeof(st->nm), 1, f);
                /* save number of flags */
                fwrite(&st->num, sizeof(int), 1, f);
                /* save level peer chapter number */
                fwrite(&st->l_num, sizeof(int), 1, f);
                /* save chapter number */
                fwrite(&st->c_num, sizeof(int), 1, f);
                /* save chapter open flags */
                fwrite(st->c_opn, sizeof(char), st->c_num, f);
                /* save flags */
                fwrite(st->cmp, sizeof(char), st->num, f);
#endif
                le = le->n;
            }
            e = e->n;
        }
        printf("ok\n");
        fclose(f);
    }
}

/* create a new profile */
void Prf_Crt(char *nm)
{
    Prf *p;
    DL_E *e = prfs.hd.n;
    // if the name already exists the profile is not created //
    while (e != &prfs.tl) {
        if (!strcmp(((Prf*)e->d)->nm, nm)) {
            printf("WARNING: profile '%s' already exists\n", nm);
            return;
        }
        e = e->n;
    }

    p = malloc(sizeof(Prf));
    strcpy(p->nm, nm);
    p->scr = 0;
    p->pct = 0;
    p->lvls = 0;
    DL_Ini(&p->sts);
    p->sts.flgs = DL_AUTODEL | DL_NOCB;
    DL_Add(&prfs, p);
}

/* delete an existing profile by pointer */
void Prf_Del(void *p)
{
    DL_Clr(&((Prf*)p)->sts);
    free(p);
}

/* register or find a levelset with name nm */
SInf* Prf_RegLS(Prf *p, LSet *l_st)
{
    int i;
    DL_E *e = p->sts.hd.n;
    SInf  *s;
    /* maybe it already exists */
    while (e != &p->sts.tl) {
        s = (SInf*)e->d;
        if (!strcmp(s->nm, l_st->nm)) {
            if (l_st->c_num != s->c_num || l_st->l_num != s->l_num) {
                // seems to be changed; clear it //
                s->num = l_st->c_num * l_st->l_num;
                s->l_num = l_st->l_num;
                s->c_num = l_st->c_num;
                for (i = 0; i < s->c_num; i++)
                    s->c_opn[i] = l_st->ch[i].opn;
                memset(s->cmp, 0, sizeof(s->cmp));
                printf("WARNING: profile '%s': set info '%s' seems to be out of date\n", p->nm, l_st->nm);
            }
            return s;
        }
        e = e->n;
    }
    /* must be registered */
    s = malloc(sizeof(SInf));
    strcpy(s->nm, l_st->nm);
    s->num = l_st->c_num * l_st->l_num;
    s->l_num = l_st->l_num;
    s->c_num = l_st->c_num;
    for (i = 0; i < s->c_num; i++)
        s->c_opn[i] = l_st->ch[i].opn;
    memset(s->cmp, 0, sizeof(s->cmp));
    DL_Add(&p->sts, s);
    return s;
}

/* create profile name list */
void Prf_CrtLst()
{
    int i = 0;
    DL_E *e = prfs.hd.n;
    Prf *p;
    Prf_DelLst();
    prf_lst = malloc(sizeof(char*) * prfs.cntr);
    while (e != &prfs.tl) {
        p = (Prf*)e->d;
        prf_lst[i] = malloc(sizeof(p->nm) + 1);
        strcpy(prf_lst[i], p->nm);
        i++;
        e = e->n;
    }
    prf_n = i;
}

/* delete profile name list */
void Prf_DelLst()
{
    int i;
    if (prf_lst == 0) return;
    for (i = 0; i < prf_n; i++)
        free(prf_lst[i]);
    free(prf_lst);
}

/*
    sort all profiles best profile comes first
*/
void Prf_Srt()
{
    void *p;
    DL_E *e = prfs.hd.n, *e2;

    if (prfs.cntr == 0) return;

    while (e != prfs.tl.p) {
        e2 = e->n;
        while (e2 != &prfs.tl) {
            if (((Prf*)e2->d)->scr > ((Prf*)e->d)->scr) {
                p = e2->d;
                e2->d = e->d;
                e->d = p;
            }
            e2 = e2->n;
        }
        e = e->n;
    }
}

/*
    update Profile p's score and info
    s is rem_time / max_time of that level
*/
void Prf_Upd(Prf *p, SInf *inf, int l_id, float pct, int scr)
{
    float new_p;
#ifdef DEBUG
    float old_p = p->pct;
    printf("level %i...", l_id);
#endif
    if (!inf->cmp[l_id]) {
        /* mark as completed */
        inf->cmp[l_id] = 1;
        /* update percentage */
        if (p->lvls == 0)
            p->pct = pct;
         else {
            new_p = (p->pct * p->lvls + pct) / (p->lvls + 1);
            p->pct = new_p;
         }
         p->lvls++;
         p->scr += scr;
#ifdef DEBUG
    printf("marked as completed\n");
    printf("added %4.2f: percentage changed from %4.2f to %4.2f\n", pct, old_p, p->pct);
    printf("score added: %i\n", scr);
#endif
    }
#ifdef DEBUG
    else
        printf("already finished\n");
#endif
}

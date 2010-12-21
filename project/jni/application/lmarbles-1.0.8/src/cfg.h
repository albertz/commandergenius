/***************************************************************************
                          cfg.h  -  description
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

#ifndef CFG_H
#define CFG_H

enum {
    DIFF_EASY = 0,
    DIFF_NORMAL,
    DIFF_HARD,
    DIFF_BRAINSTORM
};

// config file //
typedef struct {
    // profile
    char prf_nm[12];
    int prf;
    // level set //
    int ls;
    // sound
    int s_on;
    int s_vol;
    // gfx //
    int ani;
    int trp;
    int fscr; // fullscreen
    int dim; // dim effect on?
    int diff; // difficulty
    int k_up; // controls
    int k_down;
    int k_left;
    int k_right;
    int k_undo;
} Cfg;

void C_StPth();
void C_Ld();
void C_Sv();
void C_Def();

#endif

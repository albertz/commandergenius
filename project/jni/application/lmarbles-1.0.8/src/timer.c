/***************************************************************************
                          timer.c  -  description
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

#include <sys/time.h>
#include <time.h>
#include "timer.h"
#include "sdl.h"

int t_c, t_lc;

/*
    get milliseconds since last call
*/
int T_Gt()
{
    int ms;
    t_c = SDL_GetTicks();
    ms = t_c - t_lc;
    t_lc = t_c;
    if (ms == 0) {
        ms = 1;
        SDL_Delay(1);
    }
    return ms;
}

/*
    reset timer
*/
void T_Rst()
{
    t_lc = SDL_GetTicks();
}

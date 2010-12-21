/***************************************************************************
                          file.h  -  description
                             -------------------
    begin                : Tue Sep 26 2000
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

#ifndef FILE_H
#define FILE_H

#include <stdio.h>

#define F_SUB   0x0001
#define F_COM   0x0002
#define F_VAL   0x0004

void F_GetE(FILE *f, char *str, int flgs);
int  F_CkE(char *str, int t, char *nm, char *v);

void F_WrtE(FILE *f, char *str);
void F_IntToStr(char *str, int i);
void F_FloatToStr(char *str, float f);
void F_ValToInt(char *str, int *i);
void F_ValToFloat(char *str, float *f);
void F_ValToChar(char *str, char *c);

#endif

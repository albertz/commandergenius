/***************************************************************************
                          file.c  -  description
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

#include "file.h"
#include <stdlib.h>
#include <string.h>

int f_ln = 0;

/*
    read an entry and return if the correct flag is set else read next entry
*/
void F_GetE(FILE *f, char *str, int flgs)
{
    int i = 0;
    char c;
    str[0] = 0;
    while (!feof(f)) {
        fread(&c, 1/*sizeof(char)*/, 1, f); /* MUST BE exactly on byte */
        if (c == 10) {
            f_ln++; // increase line counter //
            continue; // ignore returns //
        }
        str[i++] = c; str[i] = 0;
        if ( (c == ';' && flgs & F_VAL) || (c == '>' && flgs & F_SUB) || (c == ')' && flgs & F_COM) )
            break;
        if (c == ';' || c == '>' || c == ')')
            i = 0;
    }
    if (i == 1)
        str[0] = 0;
}

/*
    returns the first character bigger than 32
*/
char F_FstC(char *str, char **n)
{
    int i = 0;
    while (str[i] <= 32) {
        i++;
        if (i >= strlen(str)) return 0;
    }
    *n = str + i;
    return str[i];
}

/*
    returns the last character bigger than 32
*/
char F_LstC(char *str)
{
    int i = strlen(str) - 1;
    while (str[i] <= 32) {
        i--;
        if (i <= 0) return 0;
    }
    return str[i];
}

/*
    read value to v
*/
int F_GetV(char *str, char *v)
{
    int i;
    char *n;
    for (i = 0; i < strlen(str); i++)
        if (str[i] == '=') {
            F_FstC(str + i + 1, &n);
            strcpy(v, n);
            v[strlen(v) - 1] = 0; // mask semicolon
            return 1;
        }
    return 0;
}

/*
    check entry for type and target name and assign p the value
*/
int  F_CkE(char *str, int t, char *nm, char *v)
{
    char *n;

    if (strlen(str) == 0) return 0;

    if (t & F_VAL) {
        F_FstC(str, &n);
        if (strncmp(nm, n, strlen(nm)))
            return 0;
         if (v != 0 )
             return F_GetV(str, v);
         else
            return 0;
    }
    else
        if (t & F_SUB) {
            F_FstC(str, &n);
            if (!strncmp(nm, n, strlen(nm)))
                return 1;
        }
        else
            if (t & F_COM && F_FstC(str, 0) == '(' && F_LstC(str) == ')')
                return 1;
    return 0;
}

/*
    write an entry with a semicolon at its end
*/
void F_WrtE(FILE *f, char *str)
{
    char f_str[strlen(str) + 2];

    sprintf(f_str, "%s;", str);
    fwrite(f_str, strlen(f_str), 1, f);
}

/*
    convert an integer to string
*/
void F_IntToStr(char *str, int i)
{
    sprintf(str,"%i", i);
}

/*
    convert an float to string
*/
void F_FloatToStr(char *str, float f)
{
    sprintf(str,"%2.2f",f);
}

/* convert a value only containing one integer */
void F_ValToInt(char *str, int *i)
{
    str[strlen(str) - 1] = 0;
    *i = atoi(str);
}

/* convert a value only containing one float */
void F_ValToFloat(char *str, float *f)
{
    str[strlen(str) - 1] = 0;
    *f = (float)strtod(str, 0);
}

/* convert a value only containing one character */
void F_ValToChar(char *str, char *c)
{
    str[strlen(str) - 1] = 0;
    *c = (char)atoi(str);
}

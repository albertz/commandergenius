/***************************************************************************
                          theme.h  -  description
                             -------------------
    begin                : Fri Feb 15 2002
    copyright            : (C) 2001 by Michael Speck
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

#ifndef __THEME_H
#define __THEME_H

/*
====================================================================
Get/delete names of all themes found in themes directory.
====================================================================
*/
void theme_get_list();
void theme_delete_list();

/*
====================================================================
Load theme by name or delete current theme (is done by load too
so it might be used to change a theme as well.)
====================================================================
*/
void theme_load( char *name );
void theme_delete();

/*
====================================================================
Check if there is a file ABOUT in the theme directory and return
it's contents up to limit characters.
If this file isn't found 'NO INFO' is set.
====================================================================
*/
void theme_get_info( char *name, char *text, int limit );

#endif

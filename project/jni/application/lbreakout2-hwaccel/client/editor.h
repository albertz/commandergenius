/***************************************************************************
                          editor.h  -  description
                             -------------------
    begin                : Fri Oct 12 2001
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

/*
====================================================================
Create/delete editor resources
====================================================================
*/
void editor_create();
void editor_delete();
/*
====================================================================
Initiate and clear stuff for each editor call.
file_name is the name of the edited file in home directory.
====================================================================
*/
int  editor_init( char *file_name );
void editor_clear();
/*
====================================================================
Run the editor
====================================================================
*/
void editor_run();

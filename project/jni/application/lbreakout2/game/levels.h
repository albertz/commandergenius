/***************************************************************************
                          levels.h  -  description
                             -------------------
    begin                : Thu Sep 6 2001
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

#ifndef __LEVELS_H
#define __LEVELS_H

#include "../client/lbreakout.h"

#define NEW_SET _("<CREATE SET>")
#define TOURNAMENT _("!FREAKOUT!")

/*
====================================================================
Open a levelset file by name.
====================================================================
*/
FILE *levelset_open( char *fname, char *mode );
/*
====================================================================
Load all levels from file and add them to the list.
====================================================================
*/
int levels_load( char *fname, List *levels, int *version, int *update );
/*
====================================================================
Load all levels from either 
home directory (fname begins with ~) or installation directory.
====================================================================
*/
LevelSet *levelset_load( char *fname, int addBonusLevels );
/*
====================================================================
Load all levelSETS listed in 'levelsets' (names) into one big
levelset and shake the levels a bit. Use a fixed seed for this
and reinit random generator with current time when done.
====================================================================
*/
LevelSet *levelset_load_all( List *levelsets, int seed, int addBonusLevels );
/*
====================================================================
Build a levelset from a level list and delete the list.
The levels are taken from the list so it must not have AUTO_DELETE
enabled!
====================================================================
*/
LevelSet *levelset_build_from_list( List *levels, char *name, int version, int update );
/*
====================================================================
Save levelset to home directory (regardsless of ~ in front of it).
Return Value: True if successful.
====================================================================
*/
int levelset_save( LevelSet *set, char *fname );
/*
====================================================================
Create an all empty levelset.
====================================================================
*/
LevelSet *levelset_create_empty( int count, char *author, char *name );
/*
====================================================================
Delete levels and set pointer NULL. Second version is for use with
lists.
====================================================================
*/
void levelset_delete( LevelSet **set );
void levelset_list_delete( void *ptr );
/*
====================================================================
Get first/next level from a set starting at the first level. If 
no more levels remain, NULL is returned.
====================================================================
*/
Level* levelset_get_first( LevelSet *set );
Level* levelset_get_next( LevelSet *set );
/*
====================================================================
Return list id of this level or -1 if not within this set.
====================================================================
*/
int levelset_get_id( LevelSet *set, Level *level );
/*
====================================================================
Load level from current file position.
====================================================================
*/
Level* level_load( FILE *file );
/*
====================================================================
Create an empty level
====================================================================
*/
Level* level_create_empty( char *author, char *name );
/*
====================================================================
Delete level pointer.
====================================================================
*/
void level_delete( void *level_ptr );

/*
====================================================================
Get version and current update of levelset: x.x
Will reset the file pointer to beginning.
====================================================================
*/
void levelset_get_version( FILE *file, int *version, int *update );
/*
====================================================================
Get the name of the author of the first level.
====================================================================
*/
void levelset_get_first_author( FILE *file, char *author );

#endif


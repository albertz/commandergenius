/***************************************************************************
                          slot.c  -  description
                             -------------------
    begin                : Sat Dec 15 2001
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

#include "lbreakout.h"
#include "slot.h"

static FILE *open_slot( int id, char *mode )
{
    char fname[512];
    snprintf( fname, 511, "%s/%s/lbr2_save_%d", 
              (getenv( "HOME" )?getenv( "HOME" ):"."), 
              CONFIG_DIR_NAME, id );
    //printf( "accessing '%s' in mode '%s'\n", fname, mode );
    return fopen( fname, mode );
}

/*
====================================================================
Load/save a game to a slot. 'slot_load' returns 1 on success and 
updates gs. 'slot_save' takes content of 'gs' and saves it.
====================================================================
*/
int slot_load( int slot_id, GameSlot *gs )
{
    FILE *file = open_slot( slot_id, "r" );
    if ( file == 0 ) 
    {
        fprintf( stderr, "ERROR: cannot open slot %d!\n", slot_id );
        return 0;
    }
    fread( gs, sizeof(GameSlot), 1, file );
    fclose( file );
    return 1;
}

int slot_save( int slot_id, GameSlot *gs )
{
    FILE *file = open_slot( slot_id, "w" );
    if ( file == 0 ) 
    {
        fprintf( stderr, "ERROR: cannot open slot %d!\n", slot_id );
        return 0;
    }
    fwrite( gs, sizeof(GameSlot), 1, file );
    fclose( file );
    return 1;
}
int slot_delete( int slot_id )
{
    char fname[512];
    snprintf( fname, 511, "%s/%s/lbr2_save_%d", 
              (getenv( "HOME" )?getenv( "HOME" ):"."), 
              CONFIG_DIR_NAME, slot_id );
    if ( remove(fname) != 0 )
    {
        fprintf( stderr, "ERROR: couldn't delete file %s\n", fname );
        return 0;
    }
    return 1;
}

/*
====================================================================
Build info hint from game saved to slot 'slot_id'.
====================================================================
*/
void slot_update_hint( int slot_id, Hint *hint )
{
    GameSlot gs;
    int i;
    char text[256], text2[256];
    char *str_diff[] = { "Kids", "Easy", "Medium", "Hard" }; /* HACK: copied from manager.c */
    
    if ( slot_load( slot_id, &gs ) )
    {
        snprintf( text, 255, "%s - %s, %d Player(s)#",
                  gs.setname, str_diff[gs.diff], gs.player_count );
        for ( i = 0; i < gs.player_count; i++ )
        {
            snprintf( text2, 255, "#%s: %d (Lvl %d)",
                  gs.player_names[i], gs.player_scores[i], 
                  gs.player_cur_level_id[i]+1 );
            strcat( text, text2 );
        }
    }
    else
        strcpy( text, "Right now, there is no game you could resume." );
    hint_set_contents( hint, text );
}



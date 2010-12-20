/***************************************************************************
                          cpu.c  -  description
                             -------------------
    begin                : Sun Jan 6 2002
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

#include "ltris.h"
#include "list.h"
#include "cpu.h"

typedef struct {
    int x, y, rot; /* destination data used by bowl */
    int score; /* score of this destination. highest score wins */
    /* debug score mods */
    int holes_mod, complete_mod, alt_mod, steep_mod, abyss_mod, block_mod;
} CPU_Dest;

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Reset bowl with original data:
empty = 0
blocked = 1
====================================================================
*/
void cpu_reset_bowl( CPU_Data *cpu_data )
{
    int i, j;
    for ( i = 0; i < cpu_data->bowl_w; i++ )
        for ( j = 0; j < cpu_data->bowl_h; j++ )
            cpu_data->bowl[i][j] = cpu_data->original_bowl[i][j];
}

/*
====================================================================
Check if cpu_block fits in at this position.
====================================================================
*/
int cpu_validate_block_pos( CPU_Data *cpu_data, int x, int y, int rot )
{
    int i, j;
    for ( j = 3; j >= 0; j-- )
        for ( i = 3; i >= 0; i-- )
            if ( cpu_data->block->mask[rot][i][j] ) {
                if ( x + i < 0 || x + i >= cpu_data->bowl_w )
                    return 0;
                if ( y + j >= cpu_data->bowl_h )
                    return 0;
                if ( y + j < 0 ) 
                    continue;
                if ( cpu_data->bowl[x + i][y + j] )
                    return 0;
            }
    return 1;
}

/*
====================================================================
Check if the passed position is valid for cpu_data->block, drop the 
block and insert to cpu_bowl with index 2.
====================================================================
*/
int cpu_insert_block( CPU_Data *cpu_data, int x, int rot, int *y )
{
    int i, j;
    /* check if out of bowl at the sides*/
    for ( i = 0; i < 4; i++ ) {
        for ( j = 0; j < 4; j++ ) 
            if ( cpu_data->block->mask[rot][i][j] ) 
                if ( x + i < 0 || x + i >= cpu_data->bowl_w )
                    return 0;
    }
    /* drop tile down */
    *y = -3;
    while ( cpu_validate_block_pos( cpu_data, x, *y + 1, rot ) ) 
        (*y)++;
    /* insert block */
    for ( i = 0; i < 4; i++ ) {
        for ( j = 0; j < 4; j++ ) 
            if ( cpu_data->block->mask[rot][i][j] )
                cpu_data->bowl[x + i][(*y) + j] = 2;
    }
    return 1;
}

/*
====================================================================
Count number of completed lines.
====================================================================
*/
int cpu_count_compl_lines( CPU_Data *cpu_data )
{
    int i, j, line_count;
    line_count = 0;
    for ( j = 0; j < cpu_data->bowl_h; j++ ) {
        for ( i = 0; i < cpu_data->bowl_w; i++ )
            if ( !cpu_data->bowl[i][j] ) break;
        if ( i == cpu_data->bowl_w ) {
            for ( i = 0; i < cpu_data->bowl_w; i++ )
                cpu_data->bowl[i][j] = 3;
            line_count++;
        }
    }
    return line_count;
}

/*
====================================================================
Remove completed lines.
====================================================================
*/
void cpu_remove_compl_lines( CPU_Data *cpu_data )
{
    int i, j, l;
    int line_count = 0;
    int line_y[4];
    /* count lines and get pos */
    for ( j = 0; j < cpu_data->bowl_h; j++ ) {
        if ( cpu_data->bowl[0][j] == 3 ) {
            line_y[line_count++] = j;
        }
    }
    /* remove */
    for ( j = 0; j < line_count; j++ )
        for ( i = 0; i < cpu_data->bowl_w; i++ ) {
            for ( l = line_y[j]; l > 0; l-- )
                cpu_data->bowl[i][l] =  cpu_data->bowl[i][l - 1];
            cpu_data->bowl[i][0] = 0;
        }
}

/*
====================================================================
Get alteration of column.
====================================================================
*/
int cpu_get_alt( CPU_Data *cpu_data, int col )
{
    int j;
    if ( col == -1 || col == cpu_data->bowl_w ) 
        return cpu_data->bowl_h;
    for ( j = 0; j < cpu_data->bowl_h; j++ )
        if ( cpu_data->bowl[col][j] != 0 )
            break;
    return cpu_data->bowl_h - j;
}

/*
====================================================================
This is the main analyze function.
CPU_Data::bowl has the block already added and 'dest' already 
contains the position of the block and does only need the score.
====================================================================
*/
int CPU_SCORE_BASIC  =   100;  /* starting score */
int CPU_SCORE_HOLE   =  -26;   /* each single hole */
int CPU_SCORE_ALT    =    5;   /* each altitude level (tile ) */
int CPU_SCORE_LINE   =   17;   /* each line completed */
int CPU_SCORE_STEEP  =   -3;   /* steepness score */
int CPU_SCORE_ABYSS  =   -7;   /* an abyss hole counts this score */
int CPU_SCORE_BLOCK  =   -5;   /* every tile above the last hole belonging to the inserted block cashes this penalty */ 
void cpu_analyze_bowl( CPU_Data *cpu_data, CPU_Dest *dest )
{
    int i, j;
    int line_count;
    int max_alt = 0;
    int line_score;
    int y, abyss_depth;
    int diff;
    int aux_alt;
    
    /* count completed lines -- lines that will be removed are marked as 3 */
    line_count = cpu_count_compl_lines( cpu_data );
    cpu_remove_compl_lines( cpu_data );
    /* get maximum alt */
    for ( i = 0; i < cpu_data->bowl_w; i++ ) {
        if ( max_alt < cpu_get_alt( cpu_data, i ) )
            max_alt = cpu_get_alt( cpu_data, i );
    }
        
    /* ANALYZE */
    
    /* LINES */
    /* clearing a single line is quite bad as there'll propably be holes and
     * some tiles remaining so we don't give any bonus for this except if
     * the bowl contents grows to high. legalize it!
     */
    line_score = CPU_SCORE_LINE + (max_alt/2);
    if ( line_count == 1 ) {
        switch ( cpu_data->aggr ) {
            case 0:
                /* defensive */
                if ( max_alt > 9 )
                    dest->complete_mod = line_score;
                break;
            case 1:
                /* normal */
                if ( max_alt <= 5 )
                    dest->complete_mod = -line_score;
                else
                    if ( max_alt <= 12 )
                        dest->complete_mod = -(line_score/2);
                break;
            case 2:
                /* aggressive */
                if ( max_alt <= 12 )
                     dest->complete_mod = -line_score;
                break;
            case 3:
                /* kamikaze */
                if ( max_alt <= 8 )
                    dest->complete_mod = -(line_score * 2);
                else
                    if ( max_alt <= 14 )
                        dest->complete_mod = -line_score;
                break;
        }
    }
    else
        dest->complete_mod = line_score * line_count * line_count;
    
    /* HOLES */
    /* each hole simply count a score */
    for ( i = 0; i < cpu_data->bowl_w; i++ ) {
        for ( j = cpu_data->bowl_h - 1; j > cpu_data->bowl_h - 1 - cpu_get_alt( cpu_data, i ); j-- ) {
            if ( !cpu_data->bowl[i][j] )
                dest->holes_mod += CPU_SCORE_HOLE;
        }
    }
    
    /* ALTITUDE */
    /* the deeper you place the tile the better */
    dest->alt_mod = CPU_SCORE_ALT * dest->y;
    
    /* STEEPNESS */
    /* height difference to the neighbored tiles */
    for ( i = 1; i < BOWL_WIDTH - 1; i++ ) {
        diff = abs( cpu_get_alt( cpu_data, i ) - cpu_get_alt( cpu_data, i - 1 ) ) + abs( cpu_get_alt( cpu_data, i ) - cpu_get_alt( cpu_data, i + 1 ) );
        dest->steep_mod += CPU_SCORE_STEEP;
    }

    /* ABYSS */
    /* abyss is the deepness of a struct with a single hole at the highest point */
    for ( i = 0; i < cpu_data->bowl_w; i++ ) {
        /* get deepest point */
        aux_alt = cpu_get_alt( cpu_data, i - 1 );
        if ( cpu_get_alt( cpu_data, i + 1 ) < aux_alt )
            aux_alt = cpu_get_alt( cpu_data, i + 1 );
        /* if this value is below the col's alt we don't have an abyss */
        if ( cpu_get_alt( cpu_data, i ) >= aux_alt )
            continue;
        y = cpu_data->bowl_h - aux_alt - 1;
        /* check abyss_depth */
        abyss_depth = 0;
        while ( y + 1 < cpu_data->bowl_h && !cpu_data->bowl[i][y + 1] ) {
            abyss_depth++;
            y++;
        }
        /* we're at position above the abyss and have it's height */
        if ( abyss_depth >= 2 )
            dest->abyss_mod += CPU_SCORE_ABYSS * abyss_depth;
    }

    /* BLOCK */
    /* to keep the bowl down we need to complete lines and therefore keep the upper holes in reach.
     * therefore we punish  each tile above the last hole. as already inserted blocks doesn't matter
     * we just score the current block. */
    for ( i = 0; i < cpu_data->bowl_w; i++ ) {
        /* first col tile */
        y = cpu_data->bowl_h - cpu_get_alt( cpu_data, i );
        while ( y < cpu_data->bowl_h && cpu_data->bowl[i][y] ) {
            if ( cpu_data->bowl[i][y] == 2 )
                dest->block_mod += CPU_SCORE_BLOCK;
            y++;
        }
    }
    
    /* score together */
    dest->score = CPU_SCORE_BASIC + dest->holes_mod + dest->complete_mod + 
                  dest->alt_mod + dest->abyss_mod + dest->steep_mod + dest->block_mod;
}

/*
====================================================================
Backup/restore bowl information. Used for preview analyzes.
====================================================================
*/
void cpu_backup_bowl( CPU_Data *cpu_data )
{
    int i, j;
    for ( i = 0; i < cpu_data->bowl_w; i++ )
        for ( j = 0; j < cpu_data->bowl_h; j++ )
            cpu_data->backup_bowl[i][j] = cpu_data->bowl[i][j];
}
void cpu_restore_bowl( CPU_Data *cpu_data )
{
    int i, j;
    for ( i = 0; i < cpu_data->bowl_w; i++ )
        for ( j = 0; j < cpu_data->bowl_h; j++ )
            cpu_data->bowl[i][j] = cpu_data->backup_bowl[i][j];
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Analyze situation and set CPU_Data::dest_x and CPU_Data::dest_rot
which is used by bowl to move the block.
The important part is the cpu_analyze_bowl() function above.
You want your own AI... edit this function!.
====================================================================
*/
void cpu_analyze_data( CPU_Data *cpu_data )
{
    int x, rot, y;
    CPU_Dest *dest, *best_dest;
    List *list = list_create( LIST_AUTO_DELETE, LIST_NO_CALLBACK );
    
    /* get and analyze valid positions of block */
    cpu_data->block = cpu_data->original_block;
    for ( rot = 0; rot < 4; rot++ ) {
        for ( x = -4; x < 14; x++ ) {
            cpu_reset_bowl( cpu_data );
            if ( cpu_insert_block( cpu_data, x, rot, &y ) ) {
                dest = calloc( 1, sizeof( CPU_Dest ) );
                dest->x = x; 
                dest->y = y;
                dest->rot = rot;
                cpu_analyze_bowl( cpu_data, dest );
                list_add( list, dest );
            }
        }
    }
    /* get highest score as final dest -- same score the deeper block is used */
    best_dest = list_first( list );
    while ( ( dest = list_next( list ) ) != 0 ) {
        if ( dest->score > best_dest->score )
            best_dest = dest;
        else
            if ( dest->score == best_dest->score && dest->y > best_dest->y )
                best_dest = dest;
    }
    cpu_data->dest_x = best_dest->x;
    cpu_data->dest_y = best_dest->y;
    cpu_data->dest_rot = best_dest->rot;
    cpu_data->dest_score = best_dest->score;
    
    /* DEBUG */
/*    printf( "DESTINATION: %i/%i\n", best_dest->x, best_dest->rot);
    printf( "Lines:      %6i\n", best_dest->complete_mod );
    printf( "Alt:        %6i\n", best_dest->alt_mod );
    printf( "Holes:      %6i\n", best_dest->holes_mod );
    printf( "Steepness:  %6i\n", best_dest->steep_mod );
    printf( "Abyss:      %6i\n", best_dest->abyss_mod );
    printf( "Blocking:   %6i\n", best_dest->block_mod );
    printf( "----------  %6i\n", best_dest->score );*/
    
    list_delete( list );
}

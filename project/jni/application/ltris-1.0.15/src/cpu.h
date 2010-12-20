/***************************************************************************
                          cpu.h  -  description
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

#ifndef __CPU_H
#define __CPU_H

/*
====================================================================
CPU_Data containing the situation CPU has to analyze:
original_bowl: 0 - empty
               1 - blocked
bowl:          2 - inserted block
               3 - removed line 
====================================================================
*/
typedef struct {
    int aggr; /* if playing aggressive for multiplayer action we punish single lines */
    Block_Mask *original_block, *original_preview; /* the two blocks tested */
    int bowl_w, bowl_h;
    int original_bowl[BOWL_WIDTH][BOWL_HEIGHT]; /* set by bowl before calling cpu_analyze_data() */
    int backup_bowl[BOWL_WIDTH][BOWL_HEIGHT]; /* this bowl stores the bowl information by cpu_backup_bowl() 
                                                 and restores by cpu_restore_bowl() */
    int bowl[BOWL_WIDTH][BOWL_HEIGHT]; /* this bowl is used to actually compute stuff */
    Block_Mask *block; /* actual block tested */
    int dest_x, dest_y, dest_rot, dest_score; /* this is the CPU result for this data */
} CPU_Data;

/*
====================================================================
Analyze situation and set CPU_Data::dest_x and CPU_Data::dest_rot
which is used by bowl to move the block.
====================================================================
*/
void cpu_analyze_data( CPU_Data *cpu_data );

#endif

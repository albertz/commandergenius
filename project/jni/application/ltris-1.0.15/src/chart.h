/***************************************************************************
                          chart.h  -  description
                             -------------------
    begin                : Mon Sep 24 2001
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
Highscore chart entry
====================================================================
*/
typedef struct {
    char name[32];
    int level;
    double score;
    int new_entry; /* newly entered the highscore */
} Chart_Entry;
/*
====================================================================
Levelset highscore chart.
====================================================================
*/
enum { CHART_ENTRY_COUNT = 10 };
typedef struct {
	Chart_Entry entries[CHART_ENTRY_COUNT + 1]; /* last entry is space holder for new entry */
	char *name;
} Set_Chart;

/*
====================================================================
Load highscore. If it doesn't exist try to create it in HI_DIR if
this fails fall back to ~/.lbreakout and create highscore there.
====================================================================
*/
void chart_load();
/*
====================================================================
Delete resources
====================================================================
*/
void chart_delete();
/*
====================================================================
Save chart in directory it was loaded from.
====================================================================
*/
void chart_save();
/*
====================================================================
Check if this player entered the highscore chart and update it.
====================================================================
*/
void chart_add( Set_Chart *chart, char *name, int level, double score );
/*
====================================================================
Sort chart with stable algorithm (bubble sort's ok) by wanted
value.
====================================================================
*/
enum { SORT_BY_LEVEL = 0, SORT_BY_SCORE };
void chart_sort( Set_Chart *chart, int type );
/*
====================================================================
Draw highscores centered in regio x,y,w,h
====================================================================
*/
void chart_show( Set_Chart *chart, int x, int y, int w, int h );
/*
====================================================================
Clear all new_entry flags (done before new players are added
to chart when game over).
====================================================================
*/
void chart_clear_new_entries();
/*
====================================================================
Query set chart by this name or if not found create a new one
by this name.
====================================================================
*/
Set_Chart* chart_set_query( char *name );
/*
====================================================================
Query chart by id. If id is invalid return 0.
====================================================================
*/
Set_Chart* chart_set_query_id( int id );

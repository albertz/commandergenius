/***************************************************************************
                          chart.c  -  description
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

#include "lbreakout.h"
#include "config.h"
#include "chart.h"

#define CHART_FILE_NAME "lbreakout2.hscr"

List *charts = 0; /* list of all loaded level set charts */

char chart_path[512]; /* path charts where actually loaded/created from */
int chart_loaded = 0; /* true if highscore succesfully loaded */
SDL_Rect chart_pos; /* used to store size. x,y is set when drawing */
int chart_gap = 10;
int chart_level_offset = 0; /* level offset (name's left aligned, score's right aligned) */
extern SDL_Surface *stk_display;
extern Config config;
extern StkFont *cfont, *chfont, *ccfont; /* normal, highlight and caption font */
extern FILE *hi_dir_chart_file;

/*
====================================================================
Local
====================================================================
*/

/*
====================================================================
Reset charts
====================================================================
*/
void chart_reset( Set_Chart *chart )
{
    int i;
    memset( chart->entries, 0, sizeof( Chart_Entry ) * CHART_ENTRY_COUNT );
    for ( i = 0; i < CHART_ENTRY_COUNT; i++ )
        chart_add( chart, "............",  i + 1, 10000 * ( i + 1 ) );
	chart->entries[0].score = 300000;
	chart->entries[1].score = 200000;
	chart->entries[2].score = 150000;
	chart->entries[3].score = 100000;
	chart->entries[4].score =  70000;
    for ( i = 0; i < CHART_ENTRY_COUNT; i++ )
        chart->entries[i].new_entry = 0;
}

/*
====================================================================
Create/delete set chart.
====================================================================
*/
Set_Chart* chart_set_create( char *name )
{
	Set_Chart *chart = calloc( 1, sizeof( Set_Chart ) );
	chart->name = strdup( name );
	chart_reset( chart );
	return chart;
}
void chart_set_delete( void *ptr )
{
	Set_Chart *chart = (Set_Chart*)ptr;
	if ( !chart ) return;
	if ( chart->name ) free( chart->name );
	free( chart );
}

/*
====================================================================
Read entries of a chart from file position.
====================================================================
*/
void chart_read_entries( FILE *file, char *file_name, Set_Chart *chart )
{
    char buffer[128];
    int i;

    for ( i = 0; i < CHART_ENTRY_COUNT; i++ ) {
        if ( feof( file ) ) {
            fprintf( stderr, "Unexpected EOF in '%s' in entry %i... reading of highscore aborted.\n",
                     file_name, i );
            break;
        }
        fgets( buffer, 128, file ); buffer[strlen(buffer) - 1] = 0; /* remove newline */
        strcpy( chart->entries[i].name, buffer );
        fgets( buffer, 128, file ); buffer[strlen(buffer) - 1] = 0; /* remove newline */
        chart->entries[i].level = atoi( buffer );
        fgets( buffer, 128, file ); buffer[strlen(buffer) - 1] = 0; /* remove newline */
        chart->entries[i].score = atoi( buffer );
    }
}

/*
====================================================================
Load all highscores from path. If not found create chart with
one set 'Original'.
ReturnValue: True if successful
====================================================================
*/
int chart_load_from_path( char *path )
{
    FILE *file = 0;
    char file_name[512];
	char setname[1024];
	char aux[4];
	Set_Chart *chart = 0;
    /* full file name */
    sprintf( file_name, "%s/%s", path, CHART_FILE_NAME );
	/* clear chart list */
	list_clear( charts );

    if (!strcmp(path, HI_DIR) && hi_dir_chart_file) {
        file = hi_dir_chart_file;
        rewind(hi_dir_chart_file);
    }
    else {
        /* open file */
        file = fopen( file_name, "rb" );
    }

    if ( file ) {
		/* test if it's new format or old one. */
		fread( aux, sizeof( char ), 3, file ); aux[3] = 0;
		fseek( file, 0, SEEK_SET );
		if ( strequal( ">>>", aux ) ) {
			/* new format: load all set charts */
			while( !feof( file ) ) {
				/* check if next sign is an '>' else skip reading */
				aux[0] = 0;
				fread( aux, sizeof( char ), 1, file );
				fseek( file, -1, SEEK_CUR );
				if ( aux[0] != '>' ) break;
				chart = calloc( 1, sizeof( Set_Chart ) );
				/* get name: >>>name */
				fscanf( file, ">>>%1023s\n", setname );
				chart->name = strdup( setname );
				/* entries */
				chart_read_entries( file, file_name, chart );
				/* add */
				list_add( charts, chart );
			}
		}
		else {
			/* old format: load single chart as 'Original' */
			fprintf( stderr, _("Converting highscore chart as format changed.\n") );
			chart = chart_set_create( "LBreakout2" );
			chart_read_entries( file, file_name, chart );
			list_add( charts, chart );
		}
	if (file != hi_dir_chart_file)
            fclose( file );
    }
    else {
        fprintf( stderr, _("Highscore chart doesn't exist in '%s'... creating new one.\n"), path );
		chart = chart_set_create( "LBreakout2" );
		list_add( charts, chart );
    }
    if (file != hi_dir_chart_file) {
        /* test if writing is allowed without changing actual contents */
        if ( ( file = fopen( file_name, "a" ) ) == 0 ) {
            fprintf( stderr, _("Write permission for '%s' denied.\n"), file_name );
            return 0;
        }
        else
            fclose( file );
    }
    strcpy( chart_path, path );
    chart_loaded = 1;
    return 1;
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Load highscore. If it doesn't exist try to create it in HI_DIR if
this fails fall back to ~/.lbreakout and create highscore there.
====================================================================
*/
void chart_load()
{
	chart_loaded = 0;
	/* create list */
	if ( charts ) list_delete( charts ); charts = 0;
	charts = list_create( LIST_AUTO_DELETE, chart_set_delete );
    /* load highscore */
    if ( !chart_load_from_path( HI_DIR ) ) {
        fprintf( stderr, _("Unable to access highscore chart in '%s'.\n"), HI_DIR );
        fprintf( stderr, _("Trying to use config directory '%s'.\n"), config.dir_name );
        if ( !chart_load_from_path( config.dir_name ) ) {
            fprintf( stderr, _("Unable to access highscore chart in config directory... won't be "
                             "able to save any results. Sorry.\n") );
            return;
        }
    }
    printf( _("Saving highscore chart in: %s\n"), chart_path );
    /* compute size and position stuff of highscore */
    char *cheader = _("Name      Level Score");
    chart_pos.w = stk_font_string_width( ccfont, cheader );
    chart_pos.h = ccfont->height +  chart_gap + /* title + gap */
			  cfont->height * CHART_ENTRY_COUNT + /* entries */
              chart_gap + /*gap between caption and entries */
              ccfont->height; /* caption size */
    chart_level_offset = stk_font_string_width( ccfont, _("name.-----") ) + stk_font_string_width( ccfont, _("Level") ) / 2; /* level offset centered */
}
/*
====================================================================
Delete resources
====================================================================
*/
void chart_delete()
{
	if ( charts ) list_delete( charts ); charts = 0;
}
/*
====================================================================
Save chart in directory it was loaded from.
====================================================================
*/
void chart_save()
{
    char file_name[512];
    int i;
	Set_Chart *chart = 0;
    FILE *file = 0;
    if ( !chart_loaded ) return;
    /* full file name */
    sprintf( file_name, "%s/%s", chart_path, CHART_FILE_NAME );

    if (!strcmp(chart_path, HI_DIR) && hi_dir_chart_file) {
        file = hi_dir_chart_file;
        rewind(hi_dir_chart_file);
    }
    else {
        /* open file */
        file = fopen( file_name, "w" );
    }

    if ( !file ) {
        fprintf( stderr, _("??? Highscore chart loaded properly but cannot save? (%s)\n"),file_name );
        return;
    }
	/* save all charts */
	list_reset( charts );
	while ( ( chart = list_next( charts ) ) !=  0 ) {
		fprintf( file, ">>>%s\n", chart->name );
    	for ( i = 0; i < CHART_ENTRY_COUNT; i++ )
        	fprintf( file, "%s\n%i\n%i\n", chart->entries[i].name, chart->entries[i].level, chart->entries[i].score );
	}		

    if (file != hi_dir_chart_file)
        fclose( file );
}
/*
====================================================================
Check if this player entered the highscore chart and update it.
====================================================================
*/
void chart_add( Set_Chart *chart, char *name, int level, int score )
{
    /* add new entry at blind end of chart */
    strcpy( chart->entries[CHART_ENTRY_COUNT].name, name );
    chart->entries[CHART_ENTRY_COUNT].level = level;
    chart->entries[CHART_ENTRY_COUNT].score = score;
    chart->entries[CHART_ENTRY_COUNT].new_entry = 1;
    /* sort algorithm is stable so first sort levels then score */
    chart_sort( chart, SORT_BY_LEVEL );
    chart_sort( chart, SORT_BY_SCORE );
}
/*
====================================================================
Sort chart with stable algorithm (bubble sort's ok) by wanted
value.
====================================================================
*/
void chart_swap( Chart_Entry *entry1, Chart_Entry *entry2 )
{
    Chart_Entry dummy;
    dummy = *entry1;
    *entry1 = *entry2;
    *entry2 = dummy;
}
void chart_sort( Set_Chart *chart, int type )
{
    int j;
    int changed = 0;
    /* use entry dummy as well so count is CHART_ENTRY_COUNT + 1 */
    do {
        changed = 0;
        for ( j = 0; j < CHART_ENTRY_COUNT; j++ )
            switch ( type ) {
                case SORT_BY_LEVEL:
                    if ( chart->entries[j].level < chart->entries[j + 1].level ) {
                        chart_swap( &chart->entries[j], &chart->entries[j + 1] );
                        changed = 1;
                    }
                    break;
                case SORT_BY_SCORE:
                    if ( chart->entries[j].score < chart->entries[j + 1].score ) {
                        chart_swap( &chart->entries[j], &chart->entries[j + 1] );
                        changed = 1;
                    }
                    break;
            }
    } while ( changed );
}
/*
====================================================================
Draw highscores centered in regio x,y,w,h
====================================================================
*/
void chart_show( Set_Chart *chart, int x, int y, int w, int h )
{
    char number_buffer[24];
    int entry_offset; /* y offset of entries */
    StkFont *font;
    int i;
    /* adjust x,y so it's centered */
    chart_pos.x = x + ( w - chart_pos.w ) / 2;
    chart_pos.y = y + ( h - chart_pos.h ) / 2;
	/* title = name of levelset */
    ccfont->align = STK_FONT_ALIGN_CENTER_X | STK_FONT_ALIGN_TOP;
    stk_font_write( ccfont, stk_display, 
        chart_pos.x + ( chart_pos.w >> 1 ), chart_pos.y, -1, 
        chart->name );
	/* caption */
    ccfont->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
    char *cheader = _("Name      Level Score");
    stk_font_write( ccfont, stk_display, 
        chart_pos.x, chart_pos.y + ccfont->height + chart_gap, -1, 
        cheader );
    /* get entry offset */
    entry_offset = ( ccfont->height + chart_gap ) * 2 + chart_pos.y;
    /* entries */
    for ( i = 0; i < CHART_ENTRY_COUNT; i++ ) {
        font = cfont;
        if ( chart->entries[i].new_entry ) font = chfont;
        /* name */
        font->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
        stk_font_write( font, stk_display, 
            chart_pos.x, entry_offset, -1, chart->entries[i].name );
        /* level */
        font->align = STK_FONT_ALIGN_CENTER_X | STK_FONT_ALIGN_TOP;
        sprintf( number_buffer, "%i", chart->entries[i].level );
        stk_font_write( font, stk_display, 
            chart_pos.x + chart_level_offset, entry_offset, -1, number_buffer );
        /* score */
        font->align = STK_FONT_ALIGN_RIGHT | STK_FONT_ALIGN_TOP;
        sprintf( number_buffer, "%i", chart->entries[i].score );
        stk_font_write( font, stk_display, 
            chart_pos.x + chart_pos.w, entry_offset, -1, number_buffer );
        /* change offset */
        entry_offset += font->height;
    }
    stk_display_store_rect( &chart_pos );
}
/*
====================================================================
Clear all new_entry flags (done before new players are added
to chart when game over).
====================================================================
*/
void chart_clear_new_entries()
{
    int i;
    Set_Chart *chart;
    list_reset( charts );
    while ( ( chart = list_next( charts ) ) != 0 ) {
        chart->entries[CHART_ENTRY_COUNT].score = chart->entries[CHART_ENTRY_COUNT].level = 0;
        for ( i = 0; i < CHART_ENTRY_COUNT + 1; i++ ) 
            chart->entries[i].new_entry = 0;
    }
}
/*
====================================================================
Query set chart by this name or if not found create a new one
by this name.
====================================================================
*/
Set_Chart* chart_set_query( char *name )
{
	Set_Chart *chart = 0;
	list_reset( charts );
	while ( ( chart = list_next( charts ) ) != 0 )
		if ( strequal( name, chart->name ) )
			return chart;
	/* not found so create it */
	fprintf( stderr, _("First chart query for '%s'. Creating this chart.\n"), name );
	chart = chart_set_create( name );
	list_add( charts, chart );
	return chart;
}
/*
====================================================================
Query chart by id. If id is invalid return 0.
====================================================================
*/
Set_Chart* chart_set_query_id( int id )
{
	if ( id >= charts->count ) {
		fprintf( stderr, _("Chart index '%i' is out of range!\n"), id );
		return 0;
	}
	return (Set_Chart*)list_get( charts, id );
}

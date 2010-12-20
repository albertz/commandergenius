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

#include "ltris.h"
#include "chart.h"
#include "list.h"

#define CHART_FILE_NAME "ltris.hscr"

List *charts = 0; /* list of all loaded level set charts */

char chart_path[512]; /* path charts where actually loaded/created from */
int chart_loaded = 0; /* true if highscore succesfully loaded */
int chart_w, chart_h; /* size of higscore chart */
int chart_gap = 10;
int chart_level_offset = 0; /* level offset (name's left aligned, score's right aligned) */
char cheader[128];
Font *cfont = 0, *chfont = 0, *ccfont = 0; /* normal, highlight and caption font */
extern Config config;
extern Sdl sdl;
extern char gametype_ids[8][64];
extern char gametype_names[8][64];

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
    memset( chart->entries, 0, sizeof( chart->entries ) );
    for ( i = 0; i < CHART_ENTRY_COUNT; i++ )
        chart_add( chart, "............",  i + 1, 10000 * ( i + 1 ) );
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
        chart->entries[i].score = strtod( buffer, 0 );
    }
}

/*
====================================================================
Load all highscores from path. If not found create chart with
one set 'Classic'.
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
    /* open file */
    file = fopen( file_name, "r" );
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
			/* old format: load single chart as 'Classic' */
			fprintf( stderr, "Converting highscore chart as format changed.\n" );
			chart = chart_set_create( gametype_ids[1] );
			chart_read_entries( file, file_name, chart );
			list_add( charts, chart );
		}
        fclose( file );
    }
    else {
        fprintf( stderr, "Highscore chart doesn't exist in '%s'... creating new one.\n", path );
		chart = chart_set_create( gametype_ids[1] );
		list_add( charts, chart );
    }
    /* test if writing is allowed without changing actual contents */
    if ( ( file = fopen( file_name, "a" ) ) == 0 ) {
        fprintf( stderr, "Write permission for '%s' denied.\n", file_name );
        return 0;
    }
    else
        fclose( file );
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
    /* HACK: not sure whether this comes really first to any chart usage??? */
    strcpy(cheader, _("Name      Level    Score")); 
	/* create list */
	if ( charts ) list_delete( charts ); charts = 0;
	charts = list_create( LIST_AUTO_DELETE, chart_set_delete );
    /* load highscore */
    if ( !chart_load_from_path( HI_DIR ) ) {
        fprintf( stderr, "Unable to access highscore chart in '%s'.\n", HI_DIR );
        fprintf( stderr, "Trying to use config directory '%s'.\n", config.dir_name );
        if ( !chart_load_from_path( config.dir_name ) ) {
            fprintf( stderr, "Unable to access highscore chart in config directory... won't be "
                             "able to save any results. Sorry.\n" );
            return;
        }
    }
    printf( "Saving highscore chart in: %s\n", chart_path );
    /* load resources */
    cfont = load_fixed_font( "f_small_yellow.bmp", 32, 96, 8 );
    chfont = load_fixed_font( "f_small_white.bmp", 32, 96, 8 );
    ccfont = load_fixed_font( "f_yellow.bmp", 32, 96, 10 );
    /* compute size and position stuff of highscore */
    chart_w = text_width( ccfont, cheader );
    chart_h = ccfont->height +  chart_gap + /* title + gap */
			  cfont->height * CHART_ENTRY_COUNT + /* entries */
              chart_gap + /*gap between caption and entries */
              ccfont->height; /* caption size */
    chart_level_offset = text_width( ccfont, "name.-----" ) + text_width( ccfont, _("Level") ) / 2; /* level offset centered */
}
/*
====================================================================
Delete resources
====================================================================
*/
void chart_delete()
{
    free_font( &cfont );
    free_font( &chfont );
    free_font( &ccfont );
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
    /* open file */
    file = fopen( file_name, "w" );
    if ( !file ) {
        fprintf( stderr, "??? Highscore chart loaded properly but cannot save?\n" );
        return;
    }
	/* save all charts */
	list_reset( charts );
	while ( ( chart = list_next( charts ) ) !=  0 ) {
		fprintf( file, ">>>%s\n", chart->name );
    	for ( i = 0; i < CHART_ENTRY_COUNT; i++ )
        	fprintf( file, "%s\n%i\n%.0f\n", chart->entries[i].name, chart->entries[i].level, chart->entries[i].score );
	}		
    fclose( file );
}
/*
====================================================================
Check if this player entered the highscore chart and update it.
====================================================================
*/
void chart_add( Set_Chart *chart, char *name, int level, double score )
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
    Font *font;
    int i;
    char *cname;
    
    /* adjust x,y so it's centered */
    x += ( w - chart_w ) / 2;
    y += ( h - chart_h ) / 2;
    
	/* FIXME: map chart name (=id) to readable i18n name. would be
	 * better to use gametype id directly in chart instead of name
	 * because then we could skip this linear search. but since there
	 * are not so many charts it does not hurt either... if name cannot
	 * be mapped, use plain id. */
	cname = NULL;
 	for (i=0;i<GAME_LAST;i++)
		if (strcmp(chart->name,gametype_ids[i])==0) {
			cname=gametype_names[i];
			break;
		}
	if (cname==NULL)
		cname=chart->name;
    
	/* title = name of levelset */
    ccfont->align = ALIGN_X_CENTER | ALIGN_Y_TOP;
    write_text( ccfont, sdl.screen, x + ( chart_w >> 1 ), y,  cname, OPAQUE );
    
	/* caption */
    ccfont->align = ALIGN_X_LEFT | ALIGN_Y_TOP;
    write_text( ccfont, sdl.screen, x, y + ccfont->height + chart_gap, cheader, OPAQUE );
    /* get entry offset */
    entry_offset = ( ccfont->height + chart_gap ) * 2 + y;
    /* entries */
    for ( i = 0; i < CHART_ENTRY_COUNT; i++ ) {
        font = cfont;
        if ( chart->entries[i].new_entry ) font = chfont;
        /* name */
        font->align = ALIGN_X_LEFT | ALIGN_Y_TOP;
        write_text( font, sdl.screen, x, entry_offset, chart->entries[i].name, OPAQUE );
        /* level */
        font->align = ALIGN_X_CENTER | ALIGN_Y_TOP;
        sprintf( number_buffer, "%i", chart->entries[i].level );
        write_text( font, sdl.screen, x + chart_level_offset, entry_offset, number_buffer, OPAQUE );
        /* score */
        font->align = ALIGN_X_RIGHT | ALIGN_Y_TOP;
        sprintf( number_buffer, "%.0f", chart->entries[i].score );
        write_text( font, sdl.screen, x + chart_w, entry_offset, number_buffer, OPAQUE );
        /* change offset */
        entry_offset += font->height;
    }
    add_refresh_rect( x, y, chart_w, chart_h );
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
	fprintf( stderr, "First chart query for '%s'. Creating this chart.\n", name );
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
		fprintf( stderr, "Chart index '%i' is out of range!\n", id );
		return 0;
	}
	return (Set_Chart*)list_get( charts, id );
}

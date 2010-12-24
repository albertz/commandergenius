/***************************************************************************
                          tools.h  -  description
                             -------------------
    begin                : Fri Jan 19 2001
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

#ifndef __TOOLS_H
#define __TOOLS_H

/* this file contains some useful tools */

/* free with a check */
#define FREE( ptr ) { if ( ptr ) free( ptr ); ptr = 0; }

/* check if a serious of flags is set in source */
#define CHECK_FLAGS( source, flags ) ( source & (flags) )

/* return random value between ( and including ) upper,lower limit */
#define RANDOM( lower, upper ) ( ( rand() % ( ( upper ) - ( lower ) + 1 ) ) + ( lower ) )

/* compute distance of two vectors */
#define VEC_DIST( vec1, vec2 ) ( sqrt( ( vec1.x - vec2.x ) * ( vec1.x - vec2.x ) + ( vec1.y - vec2.y ) * ( vec1.y - vec2.y ) ) )

/* return true if strings are fully equal */
#define STRCMP( str1, str2 ) ( ( strlen( str1 ) == strlen( str2 ) ) && !strncmp( str1, str2, strlen( str1 ) ) )

/* return minimum */
#define MINIMUM( a, b ) ((a<b)?a:b)

/* return maximum */
#define MAXIMUM( a, b ) ((a>b)?a:b)

/* square value */
#define SQUARE( x ) ((x)*(x))

/* compares to strings and returns true if their first strlen(str1) chars are equal */
int strequal( char *str1, char *str2 );

/* delete lines */
void delete_lines( char **lines, int line_number );

/* delay struct */
typedef struct {
    int limit;
    int cur;
} Delay;

/* set delay to ms milliseconds */
void delay_set( Delay *delay, int ms );

/* reset delay ( cur = 0 )*/
void delay_reset( Delay *delay );

/* check if time's out ( add ms milliseconds )and reset */
int delay_timed_out( Delay *delay, int ms );

/* return distance betwteen to map positions */
int get_dist( int x1, int y1, int x2, int y2 );

/* init random seed by using ftime */
void set_random_seed();

/* get coordintaes from string */
void get_coord( char *str, int *x, int *y );

// text structure //
typedef struct {
    char **lines;
    int count;
} Text;
// convert a str into text ( for listbox ) //
Text* create_text( char *str, int char_width );
// delete text //
void delete_text( Text *text );

/*
====================================================================
Get type and prefix from string:
    type::prefix
Set both pointers 0 if failure.
====================================================================
*/
void get_type_and_prefix( char *arg, char **ext, char **prefix );

/*
====================================================================
Replace any existence of character old into new.
====================================================================
*/
void strrepl( char **str, char c_old, char c_new );

/*
====================================================================
Copy source to dest and at maximum limit chars. Terminate with 0.
====================================================================
*/
void strcpy_lt( char *dest, char *src, int limit );

/*
====================================================================
Parse a version string and return the major version and the current
update.
====================================================================
*/
void parse_version( char *string, int *version, int *update );


/* allocate memory or exit with error if out of it */
void *salloc( int num, int size );

/* print contents of pointer raw */
void print_raw( int len, char *buf );

/* check whether a string does only contain letters, digits or
 * underscores */
int is_alphanum( char *str );

#endif

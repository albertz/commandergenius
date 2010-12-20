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

/* compares to strings and returns true if their first strlen(str1) chars are equal */
inline int strequal( char *str1, char *str2 );

/* delete lines */
void delete_lines( char **lines, int line_number );

/* delay struct */
typedef struct {
    int limit;
    int cur;
} Delay;

/* set delay to ms milliseconds */
inline void delay_set( Delay *delay, int ms );

/* reset delay ( cur = 0 )*/
inline void delay_reset( Delay *delay );

/* check if time's out ( add ms milliseconds )and reset */
inline int delay_timed_out( Delay *delay, int ms );

/* set timer so that we have a time out next call of delay_timed_out() */
inline void delay_force_time_out( Delay *delay );

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
Counter with a current float value and a target value. Approaches
the target value until reached when counter_update() is called.
====================================================================
*/
typedef struct {
    double approach; /* approaching value usually used for a smooth counter display */
    double value; /* actual value */
} Counter;
void counter_set( Counter *counter, double value );
void counter_add( Counter *counter, double add );
double counter_get_approach( Counter counter );
double counter_get( Counter counter );
void counter_update( Counter *counter, int ms );

void fill_int_array_rand( int *array, int start, int count, 
                                      int low, int high );

#endif

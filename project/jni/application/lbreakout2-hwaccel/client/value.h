/***************************************************************************
                          value.h  -  description
                             -------------------
    begin                : Thu Sep 20 2001
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

#ifndef __VALUE_H
#define __VALUE_H

/*
====================================================================
Value contained and handled by item.
====================================================================
*/
enum {
    VALUE_NONE = 0,     /* no value: separator */
    VALUE_RANGE_INT,    /* range of integers */
    VALUE_RANGE_STR,    /* range of strings: min == 0, max == count, step == 1 */
    VALUE_KEY,          /* single key value */
    VALUE_EDIT          /* editable string */
};
typedef struct _Value {
    int type;           /* type of value */
    int min, max, step; /* integer range of value */
    int *val_int;       /* value as integer (position in range or key value) */
    char *val_str;      /* value as string (either name or converted val_int)
                           --duplicated except for VALUE_EDIT */
    char **names;       /* if set val_int is associated to the indexed name and val_str
                           is set to this name instead of the converted integer value
                           --duplicated */
    int name_count;     /* number of names */
    int *filter;        /* key filter (size: SDLK_LAST) --duplicated */
    List *other_keys;   /* list of other key values for VALUE_KEY: these are
                           restricted, too */
    int grab;           /* if type is VALUE_KEY and this is set all events except QUIT and
                           KEYUP are ignored */
} Value;

/*
====================================================================
Auxiliary functions to setup a key filter.
====================================================================
*/
void filter_clear( int *filter );
void filter_set( int *filter, int first, int last, int allowed );

/*
====================================================================
Create a value. names and filter are duplicated and freed by
value_delete().
Return Value: value
====================================================================
*/
Value *value_create_empty();
Value *value_create_range_int( int *val_int, int min, int max, int step );
Value *value_create_range_str( int *val_int, char **names, int count );
Value *value_create_key( int *val_int, int *filter );
Value *value_create_edit( char *val_str, int limit );
/*
====================================================================
Delete a value
====================================================================
*/
void value_delete( Value *value );
/*
====================================================================
Increase, decrease if range.
====================================================================
*/
void value_dec( Value *value );
void value_inc( Value *value );
/*
====================================================================
Grab input of VALUE_KEY
====================================================================
*/
void value_grab( Value *value );
/*
====================================================================
Set key value if VALUE_KEY and clear grab flag
====================================================================
*/
void value_set_key( Value *value, int val_int );
/*
====================================================================
Edit string if VALUE_EDIT
====================================================================
*/
void value_edit( Value *value, int code, int unicode );
/*
====================================================================
Add another dynamically restricted key (as VALUE_KEY) to other_key
list.
====================================================================
*/
void value_add_other_key( Value *value, Value *other_key );
/*
====================================================================
Set a new name list (and update position) for VALUE_RANGE_STR.
====================================================================
*/
void value_set_new_names( Value *value, char **names, int count );

#endif

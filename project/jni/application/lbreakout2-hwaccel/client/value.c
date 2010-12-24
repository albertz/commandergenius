/***************************************************************************
                          value.c  -  description
                             -------------------
    begin                : Sun Sep 23 2001
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

#include <ctype.h>
#include "manager.h"

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Update val_str if not ITEM_VALUE
====================================================================
*/
void value_update_str( Value *value )
{
    switch ( value->type ) {
        case VALUE_RANGE_INT: sprintf( value->val_str, "%i", *value->val_int ); break;
        case VALUE_RANGE_STR: sprintf( value->val_str, "%s", value->names[*value->val_int] ); break;
        case VALUE_KEY:
            if ( value->grab ) { strcpy( value->val_str, "???" ); break; }
            switch ( *value->val_int ) {
                case SDLK_SPACE: strcpy( value->val_str, _("Space") ); break;
                case SDLK_LEFT: strcpy( value->val_str, _("Left") ); break;
                case SDLK_RIGHT: strcpy( value->val_str, _("Right") ); break;
                case SDLK_UP: strcpy( value->val_str, _("Up") ); break;
                case SDLK_DOWN: strcpy( value->val_str, _("Down") ); break;
                case SDLK_RSHIFT: strcpy( value->val_str, _("Right Shift") ); break;
                case SDLK_LSHIFT: strcpy( value->val_str, _("Left Shift") ); break;
                case SDLK_RCTRL: strcpy( value->val_str, _("Right Ctrl") ); break;
                case SDLK_LCTRL: strcpy( value->val_str, _("Left Ctrl") ); break;
                case SDLK_RALT: strcpy( value->val_str, _("Right Alt") ); break;
                case SDLK_LALT: strcpy( value->val_str, _("Left Alt") ); break;
                case SDLK_BACKSPACE: strcpy( value->val_str, _("BackSpace") ); break;
                default:
                    if ( *value->val_int >= SDLK_a && *value->val_int <= SDLK_z )
                        sprintf( value->val_str, "'%c'", (char)toupper( *value->val_int ) );
                    else
                        sprintf( value->val_str, "%i", *value->val_int );
                    break;
            }
            break;
    }
}

/*
====================================================================
Create basic value
====================================================================
*/
Value *value_create_basic( int type, int *val_int )
{
    Value *value = calloc( 1, sizeof( Value ) );
    value->type = type;
    value->val_int = val_int;
    /* if not VALUE_EDIT create val_str which carries translated val_int */
    if ( type != VALUE_EDIT ) value->val_str = calloc( 256, sizeof( char ) );
    return value;
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Auxiliary functions to setup a key filter.
====================================================================
*/
void filter_clear( int *filter )
{
    memset( filter, 0, sizeof( int ) * SDLK_LAST );
}
void filter_set( int *filter, int first, int last, int allowed )
{
    int i;
    for ( i = first; i <= last; i++ )
        filter[i] = allowed;
}

/*
====================================================================
Create a value. names and filter are duplicated and freed by
value_delete().
Return Value: value
====================================================================
*/
Value *value_create_empty()
{
    return value_create_basic( VALUE_NONE, 0 );
}
Value *value_create_range_int( int *val_int, int min, int max, int step )
{
    Value *value = value_create_basic( VALUE_RANGE_INT, val_int );
    value->min = min; value->max = max; value->step = step;
    value_update_str( value );
    return value;
}
Value *value_create_range_str( int *val_int, char **names, int count )
{
    int i;
    Value *value = value_create_basic( VALUE_RANGE_STR, val_int );
    value->min = 0; value->max = count - 1; value->step = 1;
    /* duplicate names */
    value->name_count = count;
    value->names = calloc( count, sizeof( char* ) );
    for ( i = 0; i < count; i++ )
        value->names[i] = strdup( names[i] );
    value_update_str( value );
    return value;
}
Value *value_create_key( int *val_int, int *filter )
{
    int i;
    Value *value = value_create_basic( VALUE_KEY, val_int );
    /* duplicate filter */
    value->filter = calloc( SDLK_LAST, sizeof( int ) );
    for ( i = 0; i < SDLK_LAST; i++ ) value->filter[i] = filter[i];
    value_update_str( value );
    /* list of other key values */
    value->other_keys = list_create( LIST_NO_AUTO_DELETE, LIST_NO_CALLBACK );
    return value;
}
Value *value_create_edit( char *val_str, int limit )
{
    Value *value = value_create_basic( VALUE_EDIT, 0 );
    value->val_str = val_str;
    value->max = limit;
    return value;
}
/*
====================================================================
Delete a value
====================================================================
*/
void value_delete( Value *value )
{
    int i;
    if ( !value ) return;
    if ( value->filter ) free( value->filter );
    if ( value->names ) {
        for ( i = 0; i < value->name_count; i++ )
            if ( value->names[i] ) free( value->names[i] );
        free( value->names );
    }
    if ( value->type != VALUE_EDIT && value->val_str ) free( value->val_str );
    if ( value->other_keys ) list_delete( value->other_keys );
    free( value );
}
/*
====================================================================
Increase, decrease if range.
====================================================================
*/
void value_dec( Value *value )
{
    *value->val_int -= value->step;
    if ( *value->val_int < value->min ) *value->val_int = value->max;
    value_update_str( value );
}
void value_inc( Value *value )
{
    *value->val_int += value->step;
    if ( *value->val_int > value->max ) *value->val_int = value->min;
    value_update_str( value );
}
/*
====================================================================
Grab input of VALUE_KEY
====================================================================
*/
void value_grab( Value *value )
{
    if ( value->type == VALUE_KEY ) {
        value->grab = 1;
        value_update_str( value );
    }
}
/*
====================================================================
Set key value if VALUE_KEY and clear grab flag
====================================================================
*/
void value_set_key( Value *value, int val_int )
{
    Value *other_key;
    if ( !value->filter[val_int] ) return;
    list_reset( value->other_keys );
    while ( ( other_key = list_next( value->other_keys ) ) )
        if ( *other_key->val_int == val_int ) return;
    /* ok, set */
    *value->val_int = val_int;
    value->grab = 0;
    value_update_str( value );
}
/*
====================================================================
Edit string if VALUE_EDIT
====================================================================
*/
void value_edit( Value *value, int code, int unicode )
{
    int length = strlen( value->val_str );
	if ( code == SDLK_BACKSPACE && length > 0 )
		value->val_str[length - 1] = 0;
	else
		if ( unicode >= 32 && unicode < 128 && length < value->max )
			value->val_str[length] = unicode;
}
/*
====================================================================
Add another dynamically restricted key (as VALUE_KEY) to other_key
list.
====================================================================
*/
void value_add_other_key( Value *value, Value *other_key )
{
    if ( value->type != VALUE_KEY ) return;
    list_add( value->other_keys, other_key );
}
/*
====================================================================
Set a new name list (and update position) for VALUE_RANGE_STR.
====================================================================
*/
void value_set_new_names( Value *value, char **names, int count )
{
    int i;
    if ( value->type != VALUE_RANGE_STR ) return;
    if ( value->names ) {
        for ( i = 0; i < value->name_count; i++ )
            if ( value->names[i] ) free( value->names[i] );
        free( value->names );
    }
    value->name_count = count;
    value->max = count - 1;
    if ( *value->val_int > value->max ) *value->val_int = 0;
    value->names = calloc( count, sizeof( char* ) );
    for ( i = 0; i < count; i++ )
        value->names[i] = strdup( names[i] );
    value_update_str( value );
}

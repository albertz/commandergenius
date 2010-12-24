/***************************************************************************
                          parser.h  -  description
                             -------------------
    begin                : Sat Mar 9 2002
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


#ifndef __PARSER_H
#define __PARSER_H

#include "list.h"
#include <stdio.h>

/*
====================================================================
This module provides functions to parse ASCII data from strings
and files.
Synopsis:
  groupname <begin group> entry1 .. entryX <end group>
  variable  <set> value
A group entry may either be a variable or a group (interlacing).
A variable value may either be a single token or a list of tokens
enclosed by <begin list> <end list>.
Text enclosed by ".." is counted as a single token.
====================================================================
*/

/*
====================================================================
Symbols. 
Note: These symbols are ignored when found in a token "<expression>" 
as they belong to this token then.
PARSER_GROUP_BEGIN:   <begin group>
PARSER_GROUP_END:     <end group>
PARSER_SET:           <set>
PARSER_LIST_BEGIN:    <begin list>
PARSER_LIST_END:      <end list>
PARSER_COMMENT_BEGIN: <begin comment>
PARSER_COMMENT_END:   <end comment>
PARSER_SYMBOLS:       List of all symbols + whitespace used to 
                      split strings and tokens.
PARSER_SKIP_SYMBOLS:  text bewteen these two symbols is handled as 
                      comment and therefore completely ignored
====================================================================
*/
#define PARSER_GROUP_BEGIN   '{'
#define PARSER_GROUP_END     '}'
#define PARSER_SET           '='
#define PARSER_LIST_BEGIN    '('
#define PARSER_LIST_END      ')'
#define PARSER_COMMENT_BEGIN '['
#define PARSER_COMMENT_END   ']'
#define PARSER_SYMBOLS       " =(){}[]"
#define PARSER_SKIP_SYMBOLS  "[]"

/*
====================================================================
An input string is converted into a PData tree struct.
The name identifies this entry and it's the token that is searched
for when reading this entry.
Either 'values' or 'entries' is set.
If 'entries' is not NULL the PData is a group and 'entries' 
contains pointers to other groups or lists.
If 'values' is not NULL the PData is a list and 'values' contains
a list of value strings associated with 'name'.
====================================================================
*/
typedef struct {
    char *name;
    List *values;
    List *entries;
} PData;

/*
====================================================================
This function splits a string into tokens using the characters
found in symbols as breakpoints. If the first symbol is ' ' all
whitespaces are used as breakpoints though NOT added as a token 
(thus removed from string).
====================================================================
*/
List* parser_split_string( char *string, char *symbols );
/*
====================================================================
This is the light version of parser_split_string which checks for
just one character and does not add this glue characters to the 
list. It's about 2% faster. Wow.
====================================================================
*/
List *parser_explode_string( char *string, char c );

/*
====================================================================
This function reads in a whole file and converts it into a
PData tree struct. If an error occurs NULL is returned and 
parser_error is set. 'tree_name' is the name of the PData tree.
====================================================================
*/
PData* parser_read_file( char *tree_name, char *fname );

/*
====================================================================
This function frees a PData tree struct.
====================================================================
*/
void parser_free( PData **pdata );

/*
====================================================================
Functions to access a PData tree. 
'name' is the pass within tree 'pd' where subtrees are separated 
by '/' (e.g.: name = 'config/graphics/animations')
parser_get_pdata   : get pdata entry associated with 'name'
parser_get_entries : get list of subtrees (PData structs) in 'name'
parser_get_values  : get value list of 'name'
parser_get_value   : get a single value from value list of 'name'
parser_get_int     : get first value of 'name' converted to integer
parser_get_double  : get first value of 'name' converted to double
parser_get_string  : get first value of 'name' _duplicated_
If an error occurs result is set NULL, False is returned and
parse_error is set.
====================================================================
*/
int parser_get_pdata  ( PData *pd, char *name, PData  **result );
int parser_get_entries( PData *pd, char *name, List   **result );
int parser_get_values ( PData *pd, char *name, List   **result );
int parser_get_value  ( PData *pd, char *name, char   **result, int index );
int parser_get_int    ( PData *pd, char *name, int     *result );
int parser_get_double ( PData *pd, char *name, double  *result );
int parser_get_string ( PData *pd, char *name, char   **result );

/*
====================================================================
If an error occurred you can query the message with this function.
====================================================================
*/
char* parser_get_error( void );

#endif

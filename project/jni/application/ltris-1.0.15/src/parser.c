/***************************************************************************
                          parser.c  -  description
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

#include <stdlib.h>
#include <string.h>
#include "parser.h"

/*
====================================================================
Error string.
====================================================================
*/
static char parser_sub_error[1024];
static char parser_error[1024];

/*
====================================================================
This buffer is used to fully load resource files when the 
compact format is used.
====================================================================
*/
enum { CBUFFER_SIZE = 131072 }; /* 128 KB */
static char cbuffer[CBUFFER_SIZE];
static char* cbuffer_pos = 0; /* position in cbuffer */

/*
====================================================================
As we need constant strings sometimes we have to define a maximum
length for tokens.
====================================================================
*/
enum { PARSER_MAX_TOKEN_LENGTH = 1024 };

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Macro to shorten the fread call for a single character.
====================================================================
*/
#define FILE_READCHAR( file, c ) fread( &c, sizeof( char ), 1, file )

/*
====================================================================
Copy source to dest and at maximum limit chars. Terminate with 0.
====================================================================
*/
void strcpy_lt( char *dest, char *src, int limit )
{
    int len = strlen( src );
    if ( len > limit ) {
        strncpy( dest, src, limit );
        dest[limit] = 0;
    }
    else
        strcpy( dest, src );
}

/*
====================================================================
Find next newline in cbuffer and replace it with \0 and return the 
pointer to the current line.
====================================================================
*/
static char* parser_get_next_line()
{
    char *line = cbuffer_pos;
    char *newpos;
    if ( cbuffer_pos[0] == 0 )
        return 0; /* completely read. no more lines. */
    if ( ( newpos = strchr( cbuffer_pos, 10 ) ) == 0 )
        cbuffer_pos += strlen( cbuffer_pos ); /* last line */
    else {
        cbuffer_pos = newpos + 1; /* set pointer to next line */
        newpos[0] = 0; /* terminate current line */
    }
    return line;
}

/*
====================================================================
Set parse error string: "file:line: error"
====================================================================
*/
static void parser_set_parse_error( char *fname, FILE *file, char *error )
{
    int end, pos;
    int line_count = 1;
    char c;
    end = ftell( file ); pos = 0;
    fseek( file, 0, SEEK_SET );
    while ( pos < end ) {
        FILE_READCHAR( file, c ); pos++;
        if ( c == 10 ) line_count++;
    }
    sprintf( parser_error, "%s: %i: %s", 
             fname, line_count, error ); 
}

/*
====================================================================
Check if the given character occurs in the symbol list.
If the first symbol is ' ' it is used as wildcard for all
white-spaces.
====================================================================
*/
static int is_symbol( int c, char *symbols )
{
    int i = 0;
    if ( symbols[0] == ' ' && c <= 32 ) return 1;
    while ( symbols[i] != 0 )
        if ( c == symbols[i++] ) 
            return 1;
    return 0;
}

/*
====================================================================
Move file position forward until reading in the given character.
If stop is ' ' whitespaces will be ignored.
====================================================================
*/
static void file_skip( FILE *file, char stop )
{
    char c = 0;
    FILE_READCHAR( file, c );
    while ( ( ( stop == ' ' && c <= 32 ) || ( stop != ' ' && c != stop ) ) && !feof( file ) )
        FILE_READCHAR( file, c );
    if ( !feof( file ) )
        fseek( file, -1, SEEK_CUR );
}

/*
====================================================================
Read next token from current file position where symbols is a
list of characters used to break up the tokens. The symbols 
themself are returned as tokens. If ' ' occurs in the symbol list
it will be ignored and whitespaces are removed automatically.
The token does not exceed PARSER_MAX_TOKEN_LENGTH.
Enclosing ".." are kept at the token. Use file_compare_token()
to test it's contents.
Returns False on EoF.
====================================================================
*/
static int file_read_token_intern( FILE *file, char *symbols, char *token )
{
    int pos = 0;
    char c;
    token[0] = 0;
    file_skip( file, ' ' );
    FILE_READCHAR( file, c );
    if ( feof( file ) ) { 
        sprintf( parser_sub_error, "unexpected end of file" ); 
        return 0;
    }
    /* string? */
    if ( c == '"' ) {
        token[pos++] = '"';
        FILE_READCHAR( file, c );
        while ( ( !feof( file ) && c != '"' ) ) {
            token[pos++] = c;
            if ( pos == PARSER_MAX_TOKEN_LENGTH - 2 ) {
                token[pos++] = '"';
                token[pos] = 0;
                sprintf( parser_sub_error, "token exceeds limit" );
                return 0;
            }
            FILE_READCHAR( file, c );
        }
        token[pos++] = '"';
        token[pos] = 0;
        if ( feof( file ) ) { 
            sprintf( parser_sub_error, "unexpected end of file" ); 
            token[0] = 0; 
            return 0;
        }
        return 1;
    }
    /* symbol? */
    if ( is_symbol( c, symbols ) ) {
        token[0] = c; token[1] = 0;
        return 1;
    }
    /* other token */
    while ( !is_symbol( c, symbols ) && !feof( file ) ) {
        token[pos++] = c;
        if ( pos == PARSER_MAX_TOKEN_LENGTH - 1 ) {
            token[pos] = 0;
            sprintf( parser_sub_error, "token exceeds limit" );
            return 0;
        }
        FILE_READCHAR( file, c );
    }
    token[pos] = 0;
    if ( feof( file ) ) 
        return 1;
    fseek( file, -1, SEEK_CUR );
    return 1;
}

/*
====================================================================
Skip all tokens until one begins with character 'stop'. This
token is also ignored.
====================================================================
*/
static void file_skip_section( FILE *file, char stop )
{
    char token[PARSER_MAX_TOKEN_LENGTH];
    do {
        file_read_token_intern( file, PARSER_SYMBOLS, token );
    } while ( !feof( file ) && token[0] != stop );
}

/*
====================================================================
Read next token and skip comments enclosed in tokens
skip[0], skip[1] (if skip is not NULL).
Return 0 if EoF.
====================================================================
*/
static int file_read_token( FILE *file, char *symbols, char *skip, char *token )
{
    while ( 1 ) {
        if ( !file_read_token_intern( file, symbols, token ) )
            return 0;
        if ( skip ) {
            if ( token[0] == skip[0] )
                file_skip_section( file, skip[1] );
            else
                break;
        }
        else
            break;
    }
    return 1;
}

/*
====================================================================
Remove quotes if any and return result as newly allocated string.
====================================================================
*/
static char* parser_remove_quotes( char *string )
{
    char *new;
    if ( string[0] != '"' ) 
        return strdup( string );
    new = calloc( strlen( string ) - 1, sizeof( char ) );
    strncpy( new, string + 1, strlen( string ) - 2 );
    new[strlen( string ) - 2] = 0;
    return new;
}

/*
====================================================================
Proceed in the given string until it ends or non-whitespace occurs
and return the new position.
====================================================================
*/
static char* string_ignore_whitespace( char *string )
{
    int i = 0;
    while ( string[i] != 0 && string[i] <= 32 ) i++;
    return string + i;
}

/*
====================================================================
This function searches file from the current position for the next
pdata entry.
====================================================================
*/
static PData* parser_parse_file( FILE *file )
{
    char token[PARSER_MAX_TOKEN_LENGTH];
    PData *pd = 0, *sub = 0;
    
    /* get name */
    if ( !file_read_token( file, PARSER_SYMBOLS, PARSER_SKIP_SYMBOLS, token ) )
        return 0;
    if ( is_symbol( token[0], PARSER_SYMBOLS ) ) {
        sprintf( parser_sub_error, "parse error before '%s'", token );
        return 0;
    }
    pd = calloc( 1, sizeof( PData ) );
    pd->name = parser_remove_quotes( token );
    /* check type */
    if ( !file_read_token( file, PARSER_SYMBOLS, PARSER_SKIP_SYMBOLS, token ) )
        goto failure;
    switch ( token[0] ) {
        case PARSER_SET:
            /* assign single value or list */
            pd->values = list_create( LIST_AUTO_DELETE, LIST_NO_CALLBACK );
            if ( !file_read_token( file, PARSER_SYMBOLS, PARSER_SKIP_SYMBOLS, token ) ) 
                goto failure;
            if ( token[0] != PARSER_LIST_BEGIN ) {
                if ( is_symbol( token[0], PARSER_SYMBOLS ) ) {
                    sprintf( parser_sub_error, "parse error before '%s'", token );
                    goto failure;
                }
                else
                    list_add( pd->values, parser_remove_quotes( token ) );
            }
            else {
                if ( !file_read_token( file, PARSER_SYMBOLS, PARSER_SKIP_SYMBOLS, token ) )
                    goto failure;
                while ( token[0] != PARSER_LIST_END ) {
                    if ( is_symbol( token[0], PARSER_SYMBOLS ) ) {
                        sprintf( parser_sub_error, "parse error before '%s'", token );
                        goto failure;
                    }
                    else
                        list_add( pd->values, parser_remove_quotes( token ) );
                    if ( !file_read_token( file, PARSER_SYMBOLS, PARSER_SKIP_SYMBOLS, token ) )
                        goto failure;
                }
            }
            break;
        case PARSER_GROUP_BEGIN:
            /* check all entries until PARSER_GROUP_END */
            pd->entries = list_create( LIST_NO_AUTO_DELETE, LIST_NO_CALLBACK );
            while ( 1 ) {
                if ( !file_read_token( file, PARSER_SYMBOLS, PARSER_SKIP_SYMBOLS, token ) ) 
                    goto failure;
                if ( token[0] == PARSER_GROUP_END )
                    break;
                fseek( file, -strlen( token ), SEEK_CUR );
                sub = parser_parse_file( file );
                if ( sub ) 
                    list_add( pd->entries, sub );
                else
                    goto failure;
            }
            break;
        default:
            sprintf( parser_sub_error, "parse error before '%s'", token );
            goto failure;
    }
    return pd;
failure:
    parser_free( &pd );
    return 0;
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
This function splits a string into tokens using the characters
found in symbols as breakpoints. If the first symbol is ' ' all
whitespaces are used as breakpoints though NOT added as a token 
(thus removed from string).
====================================================================
*/
List* parser_split_string( char *string, char *symbols )
{
    int pos;
    char *token = 0;
    List *list = list_create( LIST_AUTO_DELETE, LIST_NO_CALLBACK );
    while ( string[0] != 0 ) {
        if ( symbols[0] == ' ' )
            string = string_ignore_whitespace( string ); 
        if ( string[0] == 0 ) break;
        pos = 1; /* 'read in' first character */
        while ( string[pos - 1] != 0 && !is_symbol( string[pos - 1], symbols ) && string[pos - 1] != '"' ) pos++;
        if ( pos > 1 ) 
            pos--;
        else
            if ( string[pos - 1] == '"' ) {
                /* read a string */
                string = string + 1; pos = 0;
                while ( string[pos] != 0 && string[pos] != '"' ) pos++;
                token = calloc( pos + 1, sizeof( char ) );
                strncpy( token, string, pos ); token[pos] = 0;
                list_add( list, token );
                string = string + pos + (string[pos] != 0);
                continue;
            }
        token = calloc( pos + 1, sizeof( char ) );
        strncpy( token, string, pos); token[pos] = 0;
        list_add( list, token );
        string = string + pos;
    }
    return list;
}
/*
====================================================================
This is the light version of parser_split_string which checks for
just one character and does not add this glue characters to the 
list. It's about 2% faster. Wow.
====================================================================
*/
List *parser_explode_string( char *string, char c )
{
    List *list = list_create( LIST_AUTO_DELETE, LIST_NO_CALLBACK );
    char *next_slash = 0;
    char buffer[64];
    while ( string[0] != 0 && ( next_slash = strchr( string, c ) ) != 0 ) {
        if ( next_slash != string ) {
            strcpy_lt( buffer, string, (next_slash-string>63)?63:(next_slash-string) );
            list_add( list, strdup( buffer ) );
        }
        string += next_slash - string + 1;
    }
    if ( string[0] != 0 )
        list_add( list, strdup( string ) );
    return list;
}

/*
====================================================================
This function reads in a whole file and converts it into a
PData tree struct. If an error occurs NULL is returned and 
parser_error is set.
====================================================================
*/
static int parser_read_file_full( FILE *file, PData *top )
{
    PData *sub = 0;
    char token[1024];
    /* parse file */
    while ( !feof( file ) ) {
        if ( ( sub = parser_parse_file( file ) ) != 0 )
            list_add( top->entries, sub );
        else
            return 0; 
        /* skip comments and whitespaces */
        if ( !file_read_token( file, PARSER_SYMBOLS, PARSER_SKIP_SYMBOLS, token ) ) {
            if ( token[0] != 0 )
                return 0;
            break;
        }
        else
            fseek( file, -strlen( token ), SEEK_CUR );
    }
    return 1;
}
static int parser_read_file_compact( PData *section )
{
    /* section is the parent pdata that needs some 
       entries */
    PData *pd = 0;
    char *line, *cur;
    while ( ( line = parser_get_next_line() ) ) {
        switch ( line[0] ) {
            case '>':
                /* this section is finished */
                return 1;
            case '<':
                /* add a whole subsection */
                pd = calloc( 1, sizeof( PData ) );
                pd->name = strdup( line + 1 );
                pd->entries = list_create( LIST_NO_AUTO_DELETE, LIST_NO_CALLBACK );
                parser_read_file_compact( pd );
                /* add to section */
                list_add( section->entries, pd );
                break;
            case '\0': break;
            default:
                /* read values as subsection */
                pd = calloc( 1, sizeof( PData ) );
                /* check name */
                if ( ( cur = strchr( line, '»' ) ) == 0 ) {
                    sprintf( parser_sub_error, "parse error: use '»' for assignment or '<' for section" );
                    return 0;
                }
                cur[0] = 0; cur++;
                pd->name = strdup( line );
                /* get values */
                pd->values = parser_explode_string( cur, '°' );
                /* add to section */
                list_add( section->entries, pd );
                break;
        }
    }
    return 1;
}
PData* parser_read_file( char *tree_name, char *fname )
{
    int size;
    char magic = 0;
    FILE *file = 0;
    PData *top = 0;
    /* open file */
    if ( ( file = fopen( fname, "r" ) ) == 0 ) {
        sprintf( parser_error, "%s: file not found", fname );
        return 0;
    }
    /* create top level pdata */
    top = calloc( 1, sizeof( PData ) );
    top->name = strdup( tree_name );
    top->entries = list_create( LIST_NO_AUTO_DELETE, LIST_NO_CALLBACK );
    /* parse */
    FILE_READCHAR( file, magic );
    if ( magic == '@' ) {
        /* get the whole contents -- 1 and CBUFFER_SIZE are switched */
        fseek( file, 0, SEEK_END ); size = ftell( file ) - 2;
        if ( size >= CBUFFER_SIZE ) {
            fprintf( stderr, "%s: file's too big to fit the compact buffer (128KB)\n", fname );
            size = CBUFFER_SIZE - 1;
        }
        fseek( file, 2, SEEK_SET );
        fread( cbuffer, 1, size, file );
        cbuffer[size] = 0;
        /* set indicator to beginning of text */
        cbuffer_pos = cbuffer;
        /* parse cbuffer */
        if ( !parser_read_file_compact( top ) ) {
            parser_set_parse_error( fname, file, parser_sub_error );
            goto failure;
        }
    }
    else {
        fseek( file, 0, SEEK_SET );
        if ( !parser_read_file_full( file, top ) ) {
            parser_set_parse_error( fname, file, parser_sub_error );
            goto failure;
        }
    }
    /* finalize */
    fclose( file );
    return top;
failure:
    fclose( file );
    parser_free( &top );
    return 0;
}

/*
====================================================================
This function frees a PData tree struct.
====================================================================
*/
void parser_free( PData **pdata )
{
    PData *entry = 0;
    if ( (*pdata) == 0 ) return;
    if ( (*pdata)->name ) free( (*pdata)->name );
    if ( (*pdata)->values ) list_delete( (*pdata)->values );
    if ( (*pdata)->entries ) {
        list_reset( (*pdata)->entries );
        while ( ( entry = list_next( (*pdata)->entries ) ) )
            parser_free( &entry );
        list_delete( (*pdata)->entries );
    }
    free( *pdata ); *pdata = 0;
}

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
int parser_get_pdata  ( PData *pd, char *name, PData  **result )
{
    int i, found;
    PData *pd_next = pd;
    PData *entry = 0;
    char *sub = 0;
    List *path = parser_explode_string( name, '/' );
    for ( i = 0, list_reset( path ); i < path->count; i++ ) {
        sub = list_next( path );
        if ( !pd_next->entries ) {
            sprintf( parser_sub_error, "%s: no subtrees", pd_next->name );
            goto failure;
        }
        list_reset( pd_next->entries ); found = 0;
        while ( ( entry = list_next( pd_next->entries ) ) )
            if ( strlen( entry->name ) == strlen( sub ) && !strncmp( entry->name, sub, strlen( sub ) ) ) {
                pd_next = entry;
                found = 1;
                break;
            }
        if ( !found ) {
            sprintf( parser_sub_error, "%s: subtree '%s' not found", pd_next->name, sub );
            goto failure;
        }
    }
    list_delete( path );
    *result = pd_next;
    return 1;
failure:
    sprintf( parser_error, "parser_get_pdata: %s/%s: %s", pd->name, name, parser_sub_error );
    list_delete( path );
    *result = 0;
    return 0;
}
int parser_get_entries( PData *pd, char *name, List   **result )
{
    PData *entry;
    *result = 0;
    if ( !parser_get_pdata( pd, name, &entry ) ) {
        sprintf( parser_sub_error, "parser_get_entries:\n %s", parser_error );
        strcpy( parser_error, parser_sub_error );
        return 0;
    }
    if ( !entry->entries || entry->entries->count == 0 ) {
        sprintf( parser_error, "parser_get_entries: %s/%s: no subtrees", pd->name, name );
        return 0;
    }
    *result = entry->entries;
    return 1;
}
int parser_get_values ( PData *pd, char *name, List   **result )
{
    PData *entry;
    *result = 0;
    if ( !parser_get_pdata( pd, name, &entry ) ) {
        sprintf( parser_sub_error, "parser_get_values:\n %s", parser_error );
        strcpy( parser_error, parser_sub_error );
        return 0;
    }
    if ( !entry->values || entry->values->count == 0 ) {
        sprintf( parser_error, "parser_get_values: %s/%s: no values", pd->name, name );
        return 0;
    }
    *result = entry->values;
    return 1;
}
int parser_get_value  ( PData *pd, char *name, char   **result, int index )
{
    List *values;
    if ( !parser_get_values( pd, name, &values ) ) {
        sprintf( parser_sub_error, "parser_get_value:\n %s", parser_error );
        strcpy( parser_error, parser_sub_error );
        return 0;
    }
    if ( index >= values->count ) {
        sprintf( parser_error, "parser_get_value: %s/%s: index %i out of range (%i elements)", 
                 pd->name, name, index, values->count );
        return 0;
    }
    *result = list_get( values, index );
    return 1;
}
int parser_get_int    ( PData *pd, char *name, int     *result )
{
    char *value;
    if ( !parser_get_value( pd, name, &value, 0 ) ) {
        sprintf( parser_sub_error, "parser_get_int:\n %s", parser_error );
        strcpy( parser_error, parser_sub_error );
        return 0;
    }
    *result = atoi( value );
    return 1;
}
int parser_get_double ( PData *pd, char *name, double  *result )
{
    char *value;
    if ( !parser_get_value( pd, name, &value, 0 ) ) {
        sprintf( parser_sub_error, "parser_get_double:\n %s", parser_error );
        strcpy( parser_error, parser_sub_error );
        return 0;
    }
    *result = strtod( value, 0 );
    return 1;
}
int parser_get_string ( PData *pd, char *name, char   **result )
{
    char *value;
    if ( !parser_get_value( pd, name, &value, 0 ) ) {
        sprintf( parser_sub_error, "parser_get_string:\n %s", parser_error );
        strcpy( parser_error, parser_sub_error );
        return 0;
    }
    *result = strdup( value );
    return 1;
}

/*
====================================================================
If an error occurred you can query the reason with this function.
====================================================================
*/
char* parser_get_error( void )
{
    return parser_error;
}


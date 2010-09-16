
/***************************************************************************
                          file.c  -  description
                             -------------------
    begin                : Thu Jan 18 2001
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "../common/list.h"
#include "../common/tools.h"
#include "file.h"

//#define FILE_DEBUG

/*
====================================================================
Test file in path as mode.
Return Value: True if sucessful
====================================================================
*/
int file_check( char *path, char *fname, char *mode )
{
    char *full_path;
    FILE *file = 0;
    int ok = 0;

    full_path = calloc( strlen( path ) + strlen( fname ) + 2, sizeof( char ) );
    sprintf( full_path, "%s/%s", path, fname );

    if ( ( file = fopen( full_path, mode ) ) != 0 ) {
        fclose( file );
        ok = 1;
    }
    free( full_path );
    return ok;
}
/*
====================================================================
Open file in path according to type (write, read, append)
Return Value: File handle if successful else Null
====================================================================
*/
FILE *file_open( char *path, char *fname, int type )
{
    FILE *file = 0;
    char *full_path;
    char mode[3] = "a";

    full_path = calloc( strlen( path ) + strlen( fname ) + 2, sizeof( char ) );
    sprintf( full_path, "%s/%s", path, fname );

    switch ( type ) {
        case FILE_READ:
            if ( ( file = fopen( full_path, "rb" ) ) == 0 )
                fprintf( stderr, "file_open: cannot open file '%s' for reading: permission denied or non-existent\n", full_path );
            break;
        case FILE_WRITE:
            sprintf( mode, "w" );
        case FILE_APPEND:
            if ( ( file = fopen( full_path, mode ) ) == 0 )
                fprintf( stderr, "file_open: cannot open file '%s': permission denied\n", full_path );
            break;
    }
    return file;
}

/*
====================================================================
Read all lines from file pointer and return as static array.
Resets the file pointer. Should only be used when reading a whole
file.
====================================================================
*/
char** file_read_lines( FILE *file, int *count )
{
    int nl_count = 0;
    char c;
    char **lines;
    char buffer[1024];

    if ( !file ) return 0;

    /* count new_lines */
    fseek( file, 0, SEEK_SET );
    while ( !feof( file ) ) {
        fread( &c, sizeof( char ), 1, file );
        if ( c == 10 ) nl_count++;
    }
    fseek( file, 0, SEEK_SET );
    nl_count++; /* maybe last lines wasn't terminated */

    /* get mem */
    lines = calloc( nl_count, sizeof( char* ) );

    /* read lines */
    *count = 0;
    while( !feof( file ) ) {
        if ( !fgets( buffer, 1023, file ) ) break;
        if ( buffer[0] == 10 ) continue; /* empty line */
        buffer[strlen( buffer ) - 1] = 0; /* cancel newline */
        lines[*count] = strdup( buffer );
        (*count)++;
    }

    return lines;
}

/* check consistence of file (all brackets/comments closed).
will reset the file pos to the very beginning */
int check_file_cons( FILE *file )
{
    int brac = 0, comm = 0;
    char c;
    int ok = 1;

    fseek( file, 0, SEEK_SET );

    while ( !feof( file ) ) {

        fread( &c, 1, 1, file );
        switch ( c ) {

            case '(': brac++; break;
            case ')': brac--; break;
            case '#': comm++; break;

        }

    }

    fseek( file, 0, SEEK_SET );

    if ( brac != 0 || ( comm % 2) != 0 ) {

#ifdef FILE_DEBUG
        if ( brac != 0 )
            printf("the number of opening and closing brackets does not fit!...\n");
        else
            printf("the number of opening and closing comment hashes does not fit!...\n");
#endif
        ok = 0;

    }

    return ok;
}

/* return line number; keeps current file pos */
int get_line_number( FILE *file ) {
    int count = 0;
    char c;
    int pos = ftell( file );

    fseek( file, 0, SEEK_SET );
    while ( ftell( file ) < pos - 1 ) {
        fread( &c, sizeof( char ), 1, file );
        if ( c == 10 ) count++;
    }
    fseek( file, pos, SEEK_SET );
    return count + 1;
}

/* ignore all blanks and jump to next none-blank */
void ignore_blanks( FILE *file )
{
    char c;
    do {
        fread( &c, sizeof( char ), 1, file );
    } while ( c <= 32 && !feof( file ) );
    if ( !feof( file ) )
        fseek( file, -1, SEEK_CUR ); /* restore last none-blank */
}

/* add character to token and check max length; return true if below max length */
int add_char( char *token, int c )
{
    int length = strlen( token );
    /* check token length */
    if ( length == MAX_TOKEN_LENGTH - 1 ) {
        fprintf( stderr,
                 "read_token: token '%s' reached maximum length of %i, reading skipped\n",
                 token, length );
        return 0;
    }
    token[length++] = c;
    token[length] = 0;
    return 1;
}
/* read token from current file position; ignore spaces;
tokes are:
    (
    )
    =
    # comment #
    " string "
    normal_token
save token in token and check that MAX_TOKEN_LENGTH is not exceeded
return true if not end of file */
int read_token( FILE *file, char *token )
{
    int length = 0; /* token length */
    char c;
    int read_str = 0; /* if this is set token is a string "..." */
    int i;

    /* clear token */
    token[0] = 0;

    /* ignore all blanks before token */
    ignore_blanks( file );

    while( !feof( file ) ) {
        fread( &c, sizeof( char ), 1, file );
        /* treat new_lines as blanks */
        if ( c == 10 ) c = 32;
        /* check if this is a comment; if so ignore all characters in between */
        if ( c == '#' && !read_str ) {
            /* read all characters until '#' occurs */
            do {
                fread( &c, sizeof( char ), 1, file );
            } while ( c != '#' );
            /* ignore all blanks after comment */
            ignore_blanks( file );
            continue; /* start reading again */
        }
        /* add char */
        if ( !add_char( token, c ) ) {
            /* in this case restore last char as it belongs to next token */
            fseek( file, -1, SEEK_CUR );
            break;
        }
        else
            length++;
        /* check if token ends with a special single-character assignment token  */
        if ( !read_str )
            if ( c == '(' || c == ')' || c == '=' ) {
                /* if this wasn't the first character it already belongs to a new token, so skip it */
                if ( length > 1 ) {
                    fseek( file, -1, SEEK_CUR );
                    token[--length] = 0;
                }
                break;
            }
        /* check if this char is a blank */
        if ( c <= 32 && !read_str ) {
            /* remvoe this blank from token */
            token[--length] = 0;
            break;
        }
        /* check if this is a long string embraced by "..." */
        if ( c == '"' ) {
            if ( length > 1 ) {
                if ( read_str )
                    /* termination of string; stop reading */
                    break;
                else {
                    /* token read and this " belongs to next token */
                    /* in this case restore last char */
                    fseek( file, -1, SEEK_CUR );
                    token[--length] = 0;
                    break;
                }
            }
            else
                read_str = 1;
        }
    }
    if ( read_str ) {
        /* delete brackets from token */
        for ( i = 1; i < strlen( token ); i++ )
            token[i - 1] = token[i];
        token[strlen( token ) - 2] = 0;
    }
    if ( feof( file ) ) return 0;
    return 1;
}

/* find a string in the file and set file stream to this position */
int find_token( FILE *file, char *name, int type, int warning )
{
    char token[MAX_TOKEN_LENGTH];

    if ( type == RESET_FILE_POS )
        fseek( file, 0, SEEK_SET );
    while( read_token( file, token ) )
        if ( strequal( name, token ) ) {
            /* adjust position this token must be read */
            fseek( file, -strlen( token ) -1, SEEK_CUR );
            return 1;
        }
    if ( warning == WARNING )
        fprintf( stderr, "find_token: warning: token '%s' not found\n", name );
    return 0;
}

/* read argument string of a single assignment */
char* get_arg( FILE *file, char *name, int type )
{
    char token[MAX_TOKEN_LENGTH];
    char *arg = 0;

    /* search entry_name */
    if ( !find_token( file, name, type, WARNING ) ) return 0;

    /* token was found so read it */
    read_token( file, token );
    /* next token must be an equation */
    read_token( file, token );
    if ( token[0] != '=' ) {
        fprintf( stderr,
                 "get_arg: line %i: '=' expected after token '%s' but found '%s' instead\n", get_line_number( file ), name, token );
        return 0;
    }
    /* get argument */
    read_token( file, token );
    if ( token[0] == 0 )
        fprintf( stderr, "get_arg: line %i: warning: argument for '%s' is empty\n", get_line_number( file ), name );
    arg = strdup( token );
#ifdef FILE_DEBUG
    printf( "get_arg: %s = %s\n", name, arg );
#endif
    return arg;
}

/* read a cluster of arguments and return as static list */
char** get_arg_cluster( FILE *file, char *name, int *count, int type, int warning )
{
    List *args;
    char token[MAX_TOKEN_LENGTH];
    char **arg_list = 0;
    int i;

    *count = 0;

    /* search entry_name */
    if ( !find_token( file, name, type, warning ) ) return 0;

    /* init list */
    args = list_create( LIST_AUTO_DELETE, LIST_NO_CALLBACK );

    /* read entry_name */
    read_token( file, token );
    /* next token must be an '(' */
    read_token( file, token );
    if ( token[0] != '(' ) {
        fprintf( stderr, "get_arg_cluster: line %i: '(' expected after token '%s' but found '%s' instead\n", get_line_number( file ), name, token );
        return 0;
    }

    /* read tokens and add to dynamic list until ')' occurs; if an '=' or '(' is read instead something
    gone wrong */
    while ( 1 ) {
        read_token( file, token );
        if ( token[0] == ')' ) break;
        if ( token[0] == '(' || token[0] == '=' ) {
            fprintf( stderr, "get_arg_cluster: line %i: ')' expected to terminate argument list of entry '%s' but found '%c' instead\n", get_line_number( file ), name, token[0] );
            list_clear( args );
            return 0;
        }
        /* everything's ok; add to list */
        list_add( args, strdup( token ) );
    }

    /* static argument list */
    arg_list = calloc( args->count, sizeof( char* ) );
    for ( i = 0; i < args->count; i++ )
        arg_list[i] = strdup( (char*)list_get( args, i ) );
    *count = args->count;

    list_delete( args );

    return arg_list;
}

/* free arg cluster */
void delete_arg_cluster( char **cluster, int count )
{
    int i;
    if ( cluster ) {
        for ( i = 0; i < count; i++ )
            if ( cluster[i] )
                FREE( cluster[i] );
            FREE( cluster );
    }
}

/* count number of entries */
int count_arg( FILE *file, char *name )
{
    char token[MAX_TOKEN_LENGTH];
    int count = 0;

    fseek( file, 0, SEEK_SET );
    while ( read_token( file, token ) ) {
        if ( strequal( name, token ) )
            count++;
    }
    return count;
}

/*
====================================================================
Swap these two pointers.
====================================================================
*/
void swap( char **str1, char **str2 )
{
    char *dummy;
    dummy = *str1;
    *str1 = *str2;
    *str2 = dummy;
}

/*
====================================================================
Return a list with all accessible files and directories in path
with the extension ext (if != 0). Don't show hidden files.
Root is the name of the parent directory that can't be left. If this
is next directory up '..' is not added.
====================================================================
*/
Text* get_file_list( char *path, char *ext, char *root )
{
    Text *text = 0;
    int i, j;
    DIR *dir;
    DIR *test_dir;
    struct dirent *dirent = 0;
    List *list = 0;
    struct stat fstat;
    char file_name[512];
    FILE *file;
    int len;

    /* open this directory */
    if ( ( dir = opendir( path ) ) == 0 ) {
        fprintf( stderr, "get_file_list: can't open parent directory '%s'\n", path );
        return 0;
    }

    text = calloc( 1, sizeof( Text ) );

    /* use dynamic list to gather all valid entries */
    list = list_create( LIST_AUTO_DELETE, LIST_NO_CALLBACK );
    /* read each entry and check if its a valid entry, then add it to the dynamic list */
    while ( ( dirent = readdir( dir ) ) != 0 ) {
        /* hiden stuff is not displayed */
        if ( dirent->d_name[0] == '.' && dirent->d_name[1] != '.' ) continue;
        /* check if it's the root directory */
        if ( root )
            if ( dirent->d_name[0] == '.' )
                if ( strlen( path ) > strlen( root ) )
                    if ( !strncmp( path + strlen( path ) - strlen( root ), root, strlen( root ) ) )
                        continue;
        /* get stats */
        sprintf( file_name, "%s/%s", path, dirent->d_name );
        if ( stat( file_name, &fstat ) == -1 ) continue;
        /* check directory */
        if ( S_ISDIR( fstat.st_mode ) ) {
            if ( ( test_dir = opendir( file_name ) ) == 0  ) continue;
            closedir( test_dir );
            sprintf( file_name, "*%s", dirent->d_name );
            list_add( list, strdup( file_name ) );
        }
        else
        /* check regular file */
        if ( S_ISREG( fstat.st_mode ) ) {
            /* test it */
            if ( ( file = fopen( file_name, "r" ) ) == 0 ) continue;
            fclose( file );
            /* check if this file has the proper extension */
            if ( ext )
                if ( !strequal( dirent->d_name + ( strlen( dirent->d_name ) - strlen( ext ) ), ext ) )
                    continue;
            list_add( list, strdup( dirent->d_name ) );
        }
    }
    /* close dir */
    closedir( dir );

    /* convert to static list */
    text->count = list->count;
    text->lines = calloc( list->count, sizeof( char* ));
    for ( i = 0; i < text->count; i++ )
        text->lines[i] = strdup( (char*)list_get( list, i ) );
    list_delete( list );

    /* sort this list: directories at top and everything in alphabetical order */
    if ( text->count > 0 )
        for ( i = 0; i < text->count - 1; i++ )
            for ( j = i + 1; j < text->count; j++ ) {
                /* directory comes first */
                if ( text->lines[j][0] == '*' ) {
                    if ( text->lines[i][0] != '*' )
                        swap( &text->lines[i], &text->lines[j] );
                    else {
                        /* do not exceed buffer size of smaller buffer */
                        len = strlen( text->lines[i] );
                        if ( strlen( text->lines[j] ) < len ) len = strlen( text->lines[j] );
                        if ( strncmp( text->lines[j], text->lines[i], len ) < 0 )
                            swap( &text->lines[i], &text->lines[j] );
                    }
                }
                else {
                    /* do not exceed buffer size of smaller buffer */
                    len = strlen( text->lines[i] );
                    if ( strlen( text->lines[j] ) < len ) len = strlen( text->lines[j] );
                    if ( strncmp( text->lines[j], text->lines[i], len ) < 0 )
                        swap( &text->lines[i], &text->lines[j] );
                }
            }

    return text;
}

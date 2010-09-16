/***************************************************************************
                          file.h  -  description
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


#ifndef __FILE_H
#define __FILE_H

/*
this file contains function to work with files like
open,close,read/write binary/ascii data...
*/

/* maximum length of a token */
enum {
    MAX_TOKEN_LENGTH = 1024
};

/* shall find_arg reset to beginning of file or use current position? */
enum {
    RESET_FILE_POS = 0,
    FROM_CURRENT_FILE_POS
};

/* shall read_file_token go to next token or stay at end of current one */
enum {
    GO_TO_NEXT_TOKEN = 0,
    STAY_AT_TOKEN
};

/* display warning if find_arg failed */
enum {
    NO_WARNING = 0,
    WARNING
};

/*
====================================================================
Test file in path as mode.
Return Value: True if sucessful
====================================================================
*/
int file_check( char *path, char *file, char *mode );
/*
====================================================================
Open file in path according to type (write, read, append)
Return Value: File handle if successful else Null
====================================================================
*/
enum {
    FILE_WRITE,
    FILE_READ,
    FILE_APPEND
};
FILE *file_open( char *path, char *fname, int type );
/*
====================================================================
Read all lines from file pointer and return as static array.
Resets the file pointer. Should only be used when reading a whole
file.
====================================================================
*/
char** file_read_lines( FILE *file, int *count );

/* check consistence of file (all brackets/comments closed).
will reset the file pos to the very beginning */
int check_file_cons( FILE *file );

/* find a string in the file and set file stream to this position */
int find_token( FILE *file, char *name, int type, int warning );

/* read argument string of a single assignment */
char* get_arg( FILE *file, char *name, int type );

/* read a cluster of arguments and return as static list */
char** get_arg_cluster( FILE *file, char *name, int *count, int type, int warning );

/* free arg cluster */
void delete_arg_cluster( char **cluster, int count );

/* count number of entries */
int count_arg( FILE *file, char*name );

/*
====================================================================
Return a list with all accessible files and directories in path
with the extension ext (if != 0). Don't show hidden files.
Root is the name of the parent directory that can't be left. If this
is next directory up '..' is not added.
====================================================================
*/
Text* get_file_list( char *path, char *ext, char *root );

#endif

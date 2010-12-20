/***************************************************************************
                          levels.c  -  description
                             -------------------
    begin                : Thu Sep 6 2001
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

#include "levels.h"

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Read in next line.
====================================================================
*/
static int next_line( FILE *file, char *buffer )
{
    /* lines with an # are comments: ignore them */
    if ( !fgets( buffer, 1023, file ) ) return 0;
    if ( buffer[strlen(buffer) - 1] == 10 )
        buffer[strlen(buffer) - 1] = 0;
    return 1;
}

/* compare with brick conversion table and return true or false
 * depending on whether brick is destructible by normal hit */
static int is_destructible( char brick )
{
    if ( (brick >= 'a' && brick <= 'k') ||
         (brick >= 'x' && brick <= 'z') ||
          brick == 'v' || 
          brick == '*' ||
          brick == '!' )
        return 1;
    return 0;
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Open a levelset file by name.
====================================================================
*/
FILE *levelset_open( char *fname, char *mode )
{
	FILE *file;
	char path[512];
	if ( fname[0] == '~' ) {
		snprintf( path, sizeof(path)-1, "%s/%s/lbreakout2-levels/%s", 
			(getenv( "HOME" )?getenv( "HOME" ):"."), 
			CONFIG_DIR_NAME, fname + 1 );
	}
	else
	if ( fname[0] != '/' ) /* keep global pathes */
		snprintf( path, sizeof(path)-1, "%s/levels/%s", SRC_DIR, fname );
	else
		snprintf( path, sizeof(path)-1, "%s", fname );
				
	if ( ( file = fopen( path, mode ) ) == 0 ) {
		fprintf( stderr, "couldn't open %s\n", path );
		return 0;
	}
	return file;
}
/*
====================================================================
Load all levels from file and add them to the list.
====================================================================
*/
int levels_load( char *fname, List *levels, int *version, int *update )
{
	/* load all levels from levelset 'fname' and put
	   them to list 'levels' */
	FILE *file;
	Level *level;
	/* get file handle */
	if ( ( file = levelset_open( fname, "rb" ) ) == 0 )
		return 0;
	/* check version */
	levelset_get_version( file, version, update );
	/* read levels */
	while( ( level = level_load( file ) ) != 0 ) 
		list_add( levels, level );
	fclose( file );
	return 1;
}
/*
====================================================================
Load all levels from either 
home directory (fname begins with ~) or installation directory.
addBonusLevels is also the seed if not 0.
====================================================================
*/
LevelSet *levelset_load( char *fname, int addBonusLevels )
{
	int version, update, i, j, num;
	LevelSet *set;
    Level *level;
	List *levels = list_create( LIST_NO_AUTO_DELETE, 0 );

    /* check virtual sets */
    if (fname[0]=='!')
    {
        set = levelset_create_empty(1,"Michael Speck","Bonus Level");
        sprintf(set->name,"%s",fname);
        if (!strcmp(fname,_("!JUMPING_JACK!")))
            set->levels[0]->type = LT_JUMPING_JACK;
        else
        if (!strcmp(fname,_("!OUTBREAK!")))
            set->levels[0]->type = LT_OUTBREAK;
        else
        if (!strcmp(fname,_("!BARRIER!")))
            set->levels[0]->type = LT_BARRIER;
        else
        if (!strcmp(fname,_("!SITTING_DUCKS!")))
            set->levels[0]->type = LT_SITTING_DUCKS;
        else
        if (!strcmp(fname,_("!HUNTER!")))
            set->levels[0]->type = LT_HUNTER;
        else
        if (!strcmp(fname,_("!INVADERS!")))
            set->levels[0]->type = LT_DEFENDER;
        return set;
    }
    
    /* load normal file */
	levels_load( fname, levels, &version, &update );
    
    /* add bonus levels every four normal levels */
    if (addBonusLevels)
    {
        srand(addBonusLevels);
        num = levels->count / 4;
        for (i=0,j=4;i<num;i++)
        {
            level = level_create_empty("Michael Speck", "Bonus Level");
            level->type = RANDOM(LT_JUMPING_JACK,LT_LAST-1);
            list_insert(levels,level,j);
            j += 5;
        }
    }
    
	set = levelset_build_from_list( levels, fname, version, update );
	if ( set == 0 )
		fprintf( stderr, "empty levelset: %s\n", fname );
	else
		printf( "%s v%i.%02i: %i levels\n", fname, set->version, set->update, set->count );
	return set;
}
/*
====================================================================
Load all levelSETS listed in 'levelsets' (names) into one big
levelset and shake the levels a bit. Use a fixed seed for this
and reinit random generator with current time when done.
Use sets from install directory only (no preceding ~)
====================================================================
*/
LevelSet *levelset_load_all( List *levelsets, int seed, int addBonusLevels )
{
	LevelSet *set;
	char *setname;
	int version, update;
	List *levels = list_create( LIST_NO_AUTO_DELETE, 0 );
	int i, j, num;
	ListEntry *entry;
	Level **pointers, *level;

        /* use sets from install directory only (no preceding ~) */
	list_reset( levelsets );
	while ( (setname = list_next( levelsets )) && setname[0] != '~' )
        levels_load( setname, levels, &version, &update );
	
    /* shake the levels a bit */
    srand(seed);
    list_reset( levels ); i = 0;
    pointers = calloc( levels->count, sizeof( Level* ) );
    while ( ( level = list_next( levels ) ) ) {
        i = rand() % levels->count;
        while ( pointers[i] ) {
            i++;
            if ( i == levels->count )
                i = 0;
        }
        pointers[i] = level;
    }
    entry = levels->head->next;
    for ( i = 0; i < levels->count; i++ ) {
        entry->item = pointers[i];
        entry = entry->next;
    }
    free( pointers );
    /* add bonus levels every four normal levels */
    if (addBonusLevels)
    {
        srand(addBonusLevels);
        num = levels->count / 4;
        for (i=0,j=4;i<num;i++)
        {
            level = level_create_empty("Michael Speck", "Bonus Level");
            level->type = RANDOM(LT_JUMPING_JACK,LT_LAST-1);
            list_insert(levels,level,j);
            j += 5;
        }
    }
    srand(time(0));
        
	version = 1; update = 0;
	set = levelset_build_from_list( levels, TOURNAMENT, version, update );
	if ( set == 0 )
		fprintf( stderr, "empty levelset: %s\n", TOURNAMENT );
	else
		printf( "%s v%i.%02i: %i levels\n", TOURNAMENT, 
			set->version, set->update, set->count );
	return set;
}
/*
====================================================================
Build a levelset from a level list and delete the list.
The levels are taken from the list so it must not have AUTO_DELETE
enabled!
====================================================================
*/
LevelSet *levelset_build_from_list( List *levels, char *name, int version, int update )
{
	LevelSet *set = 0;
	Level *level;
	int i = 0;
	
	if ( levels->count == 0 ) return 0;
	set = salloc( 1, sizeof( LevelSet ) );
	snprintf( set->name, 20, "%s", name );
	set->levels = salloc( levels->count, sizeof( Level* ) );
	set->count = levels->count;
	set->version = version;
	set->update = update;
	list_reset( levels );
	while ( (level = list_next( levels )) ) {
		set->levels[i] = level;
		i++;
	}
	list_delete( levels );
	return set;
}
/*
====================================================================
Save levelset to home directory (regardsless of ~ in front of it).
Return Value: True if successful.
====================================================================
*/
int levelset_save( LevelSet *set, char *fname )
{
	FILE *file;
	Level *level;
	char path[512];
	int i, j, k;

	if ( set == 0 || set->count == 0 ) return 0;

	snprintf( path, sizeof(path)-1, "%s/%s/lbreakout2-levels/%s", 
			(getenv( "HOME" )?getenv( "HOME" ):"."), 
			CONFIG_DIR_NAME, (fname[0]=='~')?fname+1:fname );
	if ( ( file = fopen( path, "w" ) ) == 0 ) {
		fprintf( stderr, "couldn't open %s\n", path );
		return 0;
	}

	fprintf( file, "Version: %i.%02i\n", 
			set->version, set->update );
	for ( k = 0; k < set->count; k++ ) {
		level = set->levels[k];
    
		fprintf( file, "Level:\n%s\n%s\nBricks:\n",
				level->author, level->name );
		for ( j = 0; j < EDIT_HEIGHT; j++ ) {
			for ( i = 0; i < EDIT_WIDTH; i++ )
				fprintf( file, "%c", level->bricks[i][j] );
			fprintf( file, "\n" );
		}
		fprintf( file, "Bonus:\n" );
		for ( j = 0; j < EDIT_HEIGHT; j++ ) {
			for ( i = 0; i < EDIT_WIDTH; i++ )
				fprintf( file, "%c", level->extras[i][j] );
			fprintf( file, "\n" );
		}
	}
	
	fclose( file );
	return 1;
}
/*
====================================================================
Create an all empty levelset.
====================================================================
*/
LevelSet *levelset_create_empty( int count, char *author, char *name )
{
	int i;
	LevelSet *set = salloc( 1, sizeof( LevelSet ) );
	
	strcpy( set->name, name );
	set->count = count;
	set->levels = salloc( count, sizeof( Level* ) );
	for ( i = 0; i < count; i++ )
		set->levels[i] = level_create_empty( author, name );
	set->version = 1; set->update = 0;
	return set;
}
/*
====================================================================
Delete levels and set pointer NULL. Second version is for use with
lists.
====================================================================
*/
void levelset_delete( LevelSet **set )
{
	int i;
	if ( *set == 0 ) return;
	if ( (*set)->levels ) {
		for ( i = 0; i < (*set)->count; i++ )
			if ( (*set)->levels[i] )
				level_delete( (*set)->levels[i] );
		free( (*set)->levels );
	}
	free( *set ); *set = 0;
}
void levelset_list_delete( void *ptr )
{
	LevelSet *set = (LevelSet*)ptr;
	levelset_delete( &set );
}
/*
====================================================================
Get next level from a set starting at the first level. If no more
levels remain, NULL is returned.
====================================================================
*/
Level* levelset_get_first( LevelSet *set )
{
	return set->levels[0];
}
Level* levelset_get_next( LevelSet *set )
{
	if ( set->cur_level == set->count )
		return 0;
	return set->levels[set->cur_level++];
}
/*
====================================================================
Return list id of this level or -1 if not within this set.
====================================================================
*/
int levelset_get_id( LevelSet *set, Level *level )
{
	int i;
	
	for ( i = 0; i < set->count; i++ )
		if ( level == set->levels[i] )
			return i;
	return -1;
}
/*
====================================================================
Load level from current file position.
====================================================================
*/
Level* level_load( FILE *file )
{
	Level *level = 0;
	char buffer[1024];
	int i, j;

	/* file handle ok? */
	if ( !file ) 
		return 0;
	/* get level mem */
	level = calloc( 1, sizeof( Level ) );
	/* read entries */
	/* level: */
	if ( !next_line( file, buffer ) ) goto failure;
	if ( !strequal( "Level:", buffer ) ) goto failure;
	/* author */
	if ( !next_line( file, buffer ) ) goto failure;
	snprintf( level->author, 31, "%s", buffer );
	/* level name */
	if ( !next_line( file, buffer ) ) goto failure;
	snprintf( level->name, 31, "%s", buffer );
	/* bricks: */
	if ( !next_line( file, buffer ) ) goto failure;
	if ( !strequal( "Bricks:", buffer ) ) goto failure;
	/* load bricks */
	for ( i = 0; i < EDIT_HEIGHT; i++ ) {
		if ( !next_line( file, buffer ) ) goto failure;
		if ( strlen( buffer ) < EDIT_WIDTH ) goto failure;
		for ( j = 0; j < EDIT_WIDTH; j++ )
			level->bricks[j][i] = buffer[j];
	}
	/* extras: */
	if ( !next_line( file, buffer ) ) goto failure;
	if ( !strequal( "Bonus:", buffer ) ) goto failure;
	/* load extras */
	for ( i = 0; i < EDIT_HEIGHT; i++ ) {
		if ( !next_line( file, buffer ) ) goto failure;
		if ( strlen( buffer ) < EDIT_WIDTH ) goto failure;
		for ( j = 0; j < EDIT_WIDTH; j++ )
			level->extras[j][i] = buffer[j];
	}
        /* count destructible bricks */
        level->normal_brick_count = 0;
        for ( i = 0; i < EDIT_HEIGHT; i++ )
            for ( j = 0; j < EDIT_WIDTH; j++ )
	        if ( is_destructible(level->bricks[j][i]) )
                    level->normal_brick_count++;
	/* a normally loaded level is always of type LT_NORMAL */
	level->type = LT_NORMAL;
	/* return level */
	return level;
failure:
	level_delete( level );
	return 0;
}
/*
====================================================================
Create an empty level
====================================================================
*/
Level* level_create_empty( char *author, char *name )
{
    int i, j;
    Level *level = calloc( 1, sizeof( Level ) );
    snprintf( level->author, 31, "%s", author );
    snprintf( level->name, 31, "%s", name );
    /* empty arena */
    for ( i = 0; i < EDIT_WIDTH; i++ )
        for ( j = 0; j < EDIT_HEIGHT; j++ ) {
            level->extras[i][j] = '.';
            level->bricks[i][j] = '.';
        }
    /* empty level is always of type LT_NORMAL */
    level->type = LT_NORMAL;
    return level;
}
/*
====================================================================
Delete level pointer.
====================================================================
*/
void level_delete( void *level_ptr )
{
    Level *level = (Level*)level_ptr;
    if ( level == 0 ) return;
    free( level );
}

/*
====================================================================
Get version and current update of levelset: x.x
Will reset the file pointer to beginning.
====================================================================
*/
void levelset_get_version( FILE *file, int *version, int *update )
{
    char buffer[1024];
    *version = 1; *update = 0;
    fseek( file, 0, SEEK_SET );
    next_line( file, buffer );
    if ( strlen( buffer ) > 8 && !strncmp( buffer, "Version:", 8 ) )
        parse_version( buffer + 8, version, update );
    else
        fseek( file, 0, SEEK_SET );
}

/*
====================================================================
Get the name of the author of the first level.
====================================================================
*/
void levelset_get_first_author( FILE *file, char *author )
{
    char buffer[1024];
    int dummy;
    levelset_get_version( file, &dummy, &dummy );
    next_line( file, buffer );
    next_line( file, buffer );
    strcpy_lt( author, buffer, 31 );
}

/***************************************************************************
                          editor.c  -  description
                             -------------------
    begin                : Fri Oct 12 2001
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

#include "lbreakout.h"
#include "../game/game.h"
#include "game.h"
#include "config.h"
#include "bricks.h"
#include "frame.h"
#include "editor.h"

SDL_Surface *sel_frame = 0, *buttons = 0;
SDL_Surface *editor_bkgnd = 0; /* background (black with frame) of editor */
char  edit_file_name[512]; /* full path of edited file */
Level *edit_levels[MAX_LEVELS]; /* editor levels */
char edit_version[16]; /* version of edited set */
int   edit_level_count; /* how many levels currently used? */
int   edit_cur_level_id;
Level *edit_cur_level; /* current level modified */
enum  { EDITOR_BRICK, EDITOR_EXTRA };
int   edit_sel_type; /* type of selected tile */
int   edit_sel_id; /* brick or extra id */
int   edit_sel_x, edit_sel_y; /* position in map of selected tile */
int   edit_tile_x = 1, edit_tile_y = 20, edit_tile_w = MAP_WIDTH - 2, edit_tile_h = 3; /* part where either bricks ro bonuses 
                                                                                          are displayed */
int   extra_vis; /* extras currently shown? blinks. */
int   first_swap_level = -1; /* if not -1 this and the current level will be swapped
                                next time the button is pressed */
enum {
    /* tiles */
    BUTTON_NONE = 0,
    BUTTON_FIRST_BRICK,
    BUTTON_LAST_BRICK = BUTTON_FIRST_BRICK + ( BRICK_COUNT_REGULAR ),
    BUTTON_FIRST_EXTRA,
    BUTTON_LAST_EXTRA = BUTTON_FIRST_EXTRA + ( EX_NUMBER -1 ),
    BUTTON_EDIT,
    BUTTON_EDIT_AUTHOR,
    BUTTON_EDIT_NAME,
    /* buttons */
    BUTTON_FIRST,
    BUTTON_NEXT,
    BUTTON_PREV,
    BUTTON_CLEAR,
    BUTTON_SWAP,
    BUTTON_ADD,
    BUTTON_INSERT,
    BUTTON_DELETE,
    BUTTON_VERSION,
    BUTTON_LOAD,
    BUTTON_SAVE,
    BUTTON_PLAY
};
int   edit_buttons[MAP_WIDTH][MAP_HEIGHT]; /* an action is assigned to each map tile */
/* externals */
extern SDL_Surface *stk_display;
extern SDL_Surface *frame;
extern SDL_Surface *extra_pic;
extern SDL_Surface *brick_pic;
extern StkFont *mfont; /* use menu's font to draw status */
extern StkFont *font; /* use game's font to confirm */
extern Config config;
extern int stk_quit_request;

/* extra conversion table may be found in bricks.c */
extern Extra_Conv extra_conv_table[EX_NUMBER];
extern Brick_Conv brick_conv_table[BRICK_COUNT];

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
We had a right click into the tile region so check and change the
bricks displayed there. We select the first new tile and set
edit_sel_id, edit_sel_x, edit_sel_y. The full update is initated
by editor_handle_click().
====================================================================
*/
void editor_switch_tiles()
{
    int x_off = 2, y_off = 20, len = MAP_WIDTH - x_off * 2; /* offset in map of tiles, len is the number of tiles in one line */
    int i, j;
    /* clear edit buttons */
    for ( i = edit_tile_x; i < edit_tile_x + edit_tile_w; i++ )
        for ( j = edit_tile_y; j < edit_tile_y + edit_tile_h; j++ )
            edit_buttons[i][j] = BUTTON_NONE;
    /* clear this part of the editor bkjgnd */
    stk_surface_fill( editor_bkgnd, 
        edit_tile_x * BRICK_WIDTH,  edit_tile_y * BRICK_HEIGHT,
        edit_tile_w * BRICK_WIDTH, edit_tile_h * BRICK_HEIGHT, 0x0 );
    /* switch */
    if ( edit_sel_type == EDITOR_BRICK ) {
        /* flag */
        edit_sel_type = EDITOR_EXTRA;
        /* button map & background */
        i = 0; j = 0;
        while ( i + j * len < EX_NUMBER ) {
            edit_buttons[x_off + i][y_off + j] = BUTTON_FIRST_EXTRA + i + j * len;
            stk_surface_blit( extra_pic, ( i + j * len ) * BRICK_WIDTH, 0,
                BRICK_WIDTH, BRICK_HEIGHT,
                editor_bkgnd, ( i + x_off ) * BRICK_WIDTH, ( j + y_off ) * BRICK_HEIGHT );
            i++;
            if ( i == len ) {
                i = 0;
                j++;
            }
        }
        /* select first tile */
        edit_sel_id = 0;
        edit_sel_x = x_off;
        edit_sel_y = y_off;
    }
    else {
        /* flag */
        edit_sel_type = EDITOR_BRICK;
        /* button map & background */
        i = 0; j = 0;
        while ( i + j * len < BRICK_COUNT_REGULAR ) {
            edit_buttons[x_off + i][y_off + j] = BUTTON_FIRST_BRICK + i + j * len;
            stk_surface_blit( brick_pic, ( i + j * len ) * BRICK_WIDTH, 0,
                BRICK_WIDTH, BRICK_HEIGHT,
                editor_bkgnd, ( i + x_off ) * BRICK_WIDTH, ( j + y_off ) * BRICK_HEIGHT );
            i++;
            if ( i == len ) {
                i = 0;
                j++;
            }
        }
        /* select first tile */
        edit_sel_id = 0;
        edit_sel_x = x_off;
        edit_sel_y = y_off;
    }
}

/*
====================================================================
Draw a helping grid.
====================================================================
*/
void editor_draw_grid()
{
    SDL_Surface *buffer;
    int i, alpha;
    buffer = stk_surface_create( SDL_SWSURFACE, EDIT_WIDTH * BRICK_WIDTH, 1 );
    stk_surface_fill( buffer, 0,0,-1,-1, 0xffffff );
    stk_surface_blit( buffer, 0,0,-1,-1, editor_bkgnd, BRICK_WIDTH, BRICK_HEIGHT );
    stk_surface_blit( buffer, 0,0,-1,-1, editor_bkgnd, BRICK_WIDTH, 
        ( EDIT_HEIGHT + 1 ) * BRICK_HEIGHT - 1 );
    for ( i = 0; i < EDIT_HEIGHT - 1; i++ ) {
        if ( i == EDIT_HEIGHT / 2 - 1 )
            alpha = 192;
        else
        if ( i % (EDIT_HEIGHT/4) == 0 )
            alpha = 128;
        else
            alpha = 64;
        stk_surface_alpha_blit( buffer, 0,0,-1,-1, editor_bkgnd,
            BRICK_WIDTH, ( i + 1 ) * BRICK_HEIGHT + BRICK_HEIGHT - 1,
            alpha );
    }
    SDL_FreeSurface( buffer );
    buffer = stk_surface_create( SDL_SWSURFACE, 1, EDIT_HEIGHT * BRICK_HEIGHT );
    stk_surface_fill( buffer, 0,0,-1,-1, 0xffffff );
    stk_surface_blit( buffer, 0,0,-1,-1, editor_bkgnd, BRICK_WIDTH, BRICK_HEIGHT );
    stk_surface_blit( buffer, 0,0,-1,-1, editor_bkgnd, 
        ( EDIT_WIDTH + 1 ) * BRICK_WIDTH - 1, BRICK_HEIGHT );
    for ( i = 0; i < EDIT_WIDTH - 1; i++ ) {
        if ( i == EDIT_WIDTH / 2 - 1 )
            alpha = 192;
        else
        if ( i % (EDIT_WIDTH/4) == 0 )
            alpha = 128;
        else
            alpha = 64;
        stk_surface_alpha_blit( buffer, 0,0,-1,-1, editor_bkgnd,
             ( i + 1 ) * BRICK_WIDTH + BRICK_WIDTH- 1, BRICK_HEIGHT,
             alpha );
    }
    SDL_FreeSurface( buffer );
}

/*
====================================================================
Translate the saved character strings into extra and brick indices
for the editor.
====================================================================
*/
enum { INDICES_2_CHAR, CHAR_2_INDICES };
void editor_translate_level( Level *level, int type )
{
    int i, j, k;
    if ( type == CHAR_2_INDICES ) {
        for ( i = 0; i < EDIT_WIDTH; i++ )
            for ( j = 0; j < EDIT_HEIGHT; j++ ) {
	            /* bricks */
				for ( k = 0; k < BRICK_COUNT; k++ )
					if ( brick_conv_table[k].c == level->bricks[i][j] ) {
						level->bricks[i][j] = brick_conv_table[k].id;
						break;
					}
				if ( k == BRICK_COUNT ) level->bricks[i][j] = -1;	
                /* extras */
                for ( k = 0; k < EX_NUMBER; k++ )
                    if ( extra_conv_table[k].c == level->extras[i][j] ) {
                        level->extras[i][j] = extra_conv_table[k].type;
                        break;
                    }
                if ( k == EX_NUMBER ) level->extras[i][j] = EX_NONE;
            }
    }
    else {
        /* indices to characters */
        for ( i = 0; i < EDIT_WIDTH; i++ )
            for ( j = 0; j < EDIT_HEIGHT; j++ ) {
	            /* bricks */
				if ( level->bricks[i][j] == -1 )
					level->bricks[i][j] = '.';
				else
					for ( k = 0; k < BRICK_COUNT; k++ )
						if ( level->bricks[i][j] == brick_conv_table[k].id ) {
							level->bricks[i][j] = brick_conv_table[k].c;
							break;
						}
                /* extras */
                if ( level->extras[i][j] == EX_NONE )
                    level->extras[i][j] = '.';
                else
                    for ( k = 0; k < EX_NUMBER; k++ )
                        if ( level->extras[i][j] == extra_conv_table[k].type ) {
                            level->extras[i][j] = extra_conv_table[k].c;
                            break;
                        }
            }
    }
}
/*
====================================================================
Draw name of set file, current level, current count, remaining
levels, and other info stuff.
====================================================================
*/
void editor_draw_status()
{
    char str[512];
    int x = BRICK_WIDTH, y = ( MAP_HEIGHT - 1 ) * BRICK_HEIGHT - 2;
    int height = 10;
    /* locartion */
    sprintf( str, _("Location: %s"), edit_file_name );
    mfont->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
    stk_font_write( mfont, stk_display, x, y, STK_OPAQUE, str );
    /* current level */
    sprintf( str, _("Current Level: %i/%i (Free: %i)"), edit_cur_level_id + 1, edit_level_count, MAX_LEVELS - edit_level_count );
    mfont->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
    stk_font_write( mfont, stk_display, x, y + height, STK_OPAQUE, str );
    /* swap */
    mfont->align = STK_FONT_ALIGN_RIGHT | STK_FONT_ALIGN_TOP;
    if ( first_swap_level != -1 ) {
        sprintf( str, _("*** Level %i Marked For Swap ***"), first_swap_level + 1 );
        stk_font_write( mfont, stk_display, stk_display->w - BRICK_WIDTH, y + height, STK_OPAQUE, str );
    }
    else {
        /* version */
        sprintf( str, _("Version: %s "), edit_version );
        stk_font_write( mfont, stk_display, stk_display->w - BRICK_WIDTH, y + height, STK_OPAQUE, str );
    }
    /* name and author */
    mfont->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
    sprintf( str, _("Title: %s"), edit_cur_level->name );
    stk_font_write( mfont, stk_display, BRICK_WIDTH + 2, ( MAP_HEIGHT - 5 ) * BRICK_HEIGHT + 5, STK_OPAQUE, str );
    mfont->align = STK_FONT_ALIGN_RIGHT | STK_FONT_ALIGN_TOP;
    sprintf( str, _("Author: %s"), edit_cur_level->author );
    stk_font_write( mfont, stk_display, stk_display->w - BRICK_WIDTH - 2, ( MAP_HEIGHT - 5 ) * BRICK_HEIGHT + 5, STK_OPAQUE, str );
}
/*
====================================================================
Draw brick and extra (if any) from screen map pos
so it fits the editable field (no frame, no bottom).
====================================================================
*/
void editor_draw_brick( int edit_map_x, int edit_map_y )
{
    /* brick */
    if ( edit_cur_level->bricks[edit_map_x][edit_map_y] != -1 ) {
        if ( edit_cur_level->bricks[edit_map_x][edit_map_y] != INVIS_BRICK_ID )
            stk_surface_blit( brick_pic, 
                edit_cur_level->bricks[edit_map_x][edit_map_y] * BRICK_WIDTH, 0,
                BRICK_WIDTH, BRICK_HEIGHT,
                stk_display, (edit_map_x + 1) * BRICK_WIDTH, (edit_map_y + 1) * BRICK_HEIGHT );
        else
            stk_surface_fill( stk_display, 
                (edit_map_x + 1) * BRICK_WIDTH, (edit_map_y + 1) * BRICK_HEIGHT,
                BRICK_WIDTH, BRICK_HEIGHT, 0x777777 );
    }
    else {
        stk_surface_blit( editor_bkgnd, 
            (edit_map_x + 1) * BRICK_WIDTH, (edit_map_y + 1) * BRICK_HEIGHT,
            BRICK_WIDTH, BRICK_HEIGHT, stk_display,
            (edit_map_x + 1) * BRICK_WIDTH, (edit_map_y + 1) * BRICK_HEIGHT );
    }
    /* extra */
    if ( ( extra_vis || edit_sel_type == EDITOR_EXTRA ) && edit_cur_level->extras[edit_map_x][edit_map_y] != EX_NONE) {
        stk_surface_blit( extra_pic, 
            ( edit_cur_level->extras[edit_map_x][edit_map_y] ) * BRICK_WIDTH, 0,
            BRICK_WIDTH, BRICK_HEIGHT,
            stk_display, (edit_map_x + 1) * BRICK_WIDTH, (edit_map_y + 1) * BRICK_HEIGHT );
    }
    stk_display_store_drect();
}
/*
====================================================================
Redraw and refresh full screen
====================================================================
*/
void editor_full_update()
{
    int i, j;
    /* background */
    stk_surface_blit( editor_bkgnd, 0,0,-1,-1, stk_display, 0,0 );
    /* bricks&extras */
    for ( i = 0; i < EDIT_WIDTH; i++ )
        for ( j = 0; j < EDIT_HEIGHT; j++ )
            editor_draw_brick( i, j );
    /* selection frame */
    stk_surface_blit( sel_frame, 0,0,-1,-1, 
        stk_display, edit_sel_x * BRICK_WIDTH, edit_sel_y * BRICK_HEIGHT );
    /* status */
    editor_draw_status();
    /* refresh */
    stk_display_update( STK_UPDATE_ALL );
}
/*
====================================================================
Do only redraw and refresh those bricks with an extra
====================================================================
*/
void editor_update_extra_bricks()
{
    int i, j;
    for ( i = 0; i < EDIT_WIDTH; i++ )
        for ( j = 0; j < EDIT_HEIGHT; j++ )
            if ( edit_cur_level->extras[i][j] != EX_NONE ) 
                editor_draw_brick( i, j );
    stk_display_update( STK_UPDATE_RECTS );
}
/*
====================================================================
Free all editor levels
====================================================================
*/
void editor_clear_levels()
{
    int i;
    for ( i = 0; i < MAX_LEVELS; i++ )
        if ( edit_levels[i] ) level_delete( edit_levels[i] );
    memset( edit_levels, 0, sizeof( Level* ) * MAX_LEVELS );
    edit_level_count = 0;
}
/*
====================================================================
Save/load levels to/from editor file.
====================================================================
*/
void editor_load_levels()
{
    int i, version, update;
    Level *level;
    FILE *file = 0;
    /* clear levels first */
    editor_clear_levels();
    /* read levels while there are some in it */
    edit_level_count = 0;
    if ( ( file = fopen( edit_file_name, "rb" ) ) != 0 ) {
        levelset_get_version( file, &version, &update );
        sprintf( edit_version, "%i.%02i", version, update );
        while ( ( level = level_load( file ) ) != 0 )
            edit_levels[edit_level_count++] = level;
        fclose( file );
    }
    /* if we got no level at all create an empty one */
    if ( edit_level_count == 0 ) {
        edit_level_count = 1;
        edit_levels[0] = level_create_empty( _("noname"), _("untitled") );
    }
    /* translate the character strings to editor info */
    for ( i = 0; i < edit_level_count; i++ )
        editor_translate_level( edit_levels[i], CHAR_2_INDICES );
}
void editor_save_levels()
{
    FILE *file = 0;
    int i, j, k;
    Level *level = 0;
    /* convert */
    for ( i = 0; i < edit_level_count; i++ )
        editor_translate_level( edit_levels[i], INDICES_2_CHAR );
    /* save */
    if ( ( file = fopen( edit_file_name, "w" ) ) != 0 ) {
        fprintf( file, "Version: %s\n", edit_version );
        for ( i = 0; i < edit_level_count; i++ ) {
            level = edit_levels[i];
            /* save level */
            fprintf( file, "Level:\n%s\n%s\nBricks:\n", level->author, level->name );
            for ( j = 0; j < EDIT_HEIGHT; j++ ) {
                for ( k = 0; k < EDIT_WIDTH; k++ )
                    fprintf( file, "%c", level->bricks[k][j] );
                fprintf( file, "\n" );
            }
            fprintf( file, "Bonus:\n" );
            for ( j = 0; j < EDIT_HEIGHT; j++ ) {
                for ( k = 0; k < EDIT_WIDTH; k++ )
                    fprintf( file, "%c", level->extras[k][j] );
                fprintf( file, "\n" );
            }
        }
        fclose( file );
    }
    /* convert back */
    for ( i = 0; i < edit_level_count; i++ )
        editor_translate_level( edit_levels[i], CHAR_2_INDICES );
}
/*
====================================================================
Handle button action
====================================================================
*/
void editor_handle_button( int type, int *full_update ) {
    Level *dummy_ptr;
    int old_pos;
    int version, update;
    char *name, *author;
    int i;
    switch ( type ) {
        case BUTTON_FIRST:
            edit_cur_level_id = 0;
            edit_cur_level = edit_levels[0];
            *full_update = 1;
            break;
        case BUTTON_NEXT:
            edit_cur_level_id++;
            if ( edit_cur_level_id == edit_level_count ) edit_cur_level_id = 0;
            edit_cur_level = edit_levels[edit_cur_level_id];
            *full_update = 1;
            break;
        case BUTTON_PREV:
            edit_cur_level_id--;
            if ( edit_cur_level_id == -1 ) edit_cur_level_id = edit_level_count - 1;
            edit_cur_level = edit_levels[edit_cur_level_id];
            *full_update = 1;
            break;
        case BUTTON_SWAP:
            if ( first_swap_level == -1 ) {
                first_swap_level = edit_cur_level_id;
                *full_update = 1;
            }
            else {
                /* swap current and marked level */
                dummy_ptr = edit_levels[first_swap_level];
                edit_levels[first_swap_level] = edit_levels[edit_cur_level_id];
                edit_levels[edit_cur_level_id] = dummy_ptr;
                edit_cur_level = edit_levels[edit_cur_level_id];
                first_swap_level = -1;
                *full_update = 1;
            }
            break;
        case BUTTON_LOAD:
            if ( !confirm( font, _("Discard All Changes? y/n"), CONFIRM_YES_NO ) ) break;
            /* load levels and reset position if level doesn't exist */
            old_pos = edit_cur_level_id;
            editor_load_levels();
            if ( old_pos >= edit_level_count ) edit_cur_level_id = 0;
            edit_cur_level = edit_levels[edit_cur_level_id];
            first_swap_level = -1;
            *full_update = 1;
            break;
        case BUTTON_SAVE:
            if ( !confirm( font, _("Save Changes? y/n"), CONFIRM_YES_NO ) ) break;
            editor_save_levels();
            break;
        case BUTTON_CLEAR:
            if ( !confirm( font, _("Clear Level? y/n"), CONFIRM_YES_NO ) ) break;
            author = strdup( edit_cur_level->author );
            name = strdup( edit_cur_level->name );
            level_delete( edit_levels[edit_cur_level_id] );
            edit_levels[edit_cur_level_id] = level_create_empty( author, name );
            edit_cur_level = edit_levels[edit_cur_level_id];
            free( name ); free( author );
            *full_update = 1;
            break;
        case BUTTON_ADD:
            if ( edit_level_count == MAX_LEVELS ) break;
            if ( !confirm( font, _("Add Level? y/n"), CONFIRM_YES_NO ) ) break;
            edit_levels[edit_level_count] = level_create_empty( edit_levels[edit_level_count - 1]->author, edit_levels[edit_level_count - 1]->name );
            edit_level_count++;
            *full_update = 1;
            break;
        case BUTTON_INSERT:
            if ( edit_level_count == MAX_LEVELS ) break;
            if ( !confirm( font, _("Insert Level? y/n"), CONFIRM_YES_NO ) ) break;
            for ( i = edit_level_count; i > edit_cur_level_id; i-- )
                edit_levels[i] = edit_levels[i - 1];
            edit_level_count++;
            edit_levels[edit_cur_level_id] = level_create_empty( edit_cur_level->author, edit_cur_level->name );
            edit_cur_level = edit_levels[edit_cur_level_id];
            *full_update = 1;
            break;
        case BUTTON_DELETE:
            if ( edit_level_count == 1 ) break; /* last level may not be removed */
            if ( !confirm( font, _("Delete Level? y/n"), CONFIRM_YES_NO ) ) break;
            level_delete( edit_levels[edit_cur_level_id] );
            for ( i = edit_cur_level_id; i < edit_level_count - 1; i++ )
                edit_levels[i] = edit_levels[i + 1];
            edit_levels[i] = 0;
            edit_level_count--;
            if ( edit_cur_level_id >= edit_level_count )
                edit_cur_level_id = edit_level_count - 1;
            edit_cur_level = edit_levels[edit_cur_level_id];
            *full_update = 1;
            break;
        case BUTTON_PLAY:
            stk_display_fade( STK_FADE_OUT, STK_FADE_DEFAULT_TIME );
            /* translate */
            editor_translate_level( edit_cur_level, INDICES_2_CHAR );
            /* run */
            client_game_test_level( edit_cur_level );
            /* translate back */
            editor_translate_level( edit_cur_level, CHAR_2_INDICES );
            *full_update = 1;
            break;
        case BUTTON_VERSION:
            if ( enter_string( font, _("Levelset Version:"), edit_version, 8 ) ) {
                parse_version( edit_version, &version, &update );
                sprintf( edit_version, "%i.%02i", version, update );
                *full_update = 1;
            }
            break;
    }
}
/*
====================================================================
Handle a click on a map tile.
If set is False a remove action was requested (only for
editing)
====================================================================
*/
int near_grow_brick( int x, int y ) {
    int i, j;
    for ( i = x - 1; i <= x + 1; i++ )
        for ( j = y - 1; j <= y + 1; j++ )
            if ( i != x || j != y )
                if ( i >= 0 && j >= 0 && i < EDIT_WIDTH && j < EDIT_HEIGHT )
                    if ( edit_cur_level->bricks[i][j] == GROW_BRICK_ID ) 
                        return 1;
    return 0;
}
void editor_handle_click( int x, int y, int set, int *full_update )
{
    int sel = 0;
    int edit_x, edit_y;
    char str[32];
    /* if !set and within the tile field we perform a switch */
    if ( !set ) {
        if ( x >= edit_tile_x && y >= edit_tile_y && x < edit_tile_x + edit_tile_w && y < edit_tile_y + edit_tile_h ) {
            editor_switch_tiles();
            *full_update = 1;
            return;
        }
    }
    /* the remaining stuff requires a tile at the position */
    if ( edit_buttons[x][y] == BUTTON_NONE ) return; /* no action */
    /* bricks */
    if ( edit_buttons[x][y] >= BUTTON_FIRST_BRICK && edit_buttons[x][y] <= BUTTON_LAST_BRICK ) {
        sel = 1;
        edit_sel_type = EDITOR_BRICK;
        edit_sel_id = edit_buttons[x][y] - BUTTON_FIRST_BRICK;
    }
    /* extras */
    if ( edit_buttons[x][y] >= BUTTON_FIRST_EXTRA && edit_buttons[x][y] <= BUTTON_LAST_EXTRA ) {
        sel = 1;
        edit_sel_type = EDITOR_EXTRA;
        edit_sel_id = edit_buttons[x][y] - BUTTON_FIRST_EXTRA;
    }
    /* edit field */
    if ( edit_buttons[x][y] == BUTTON_EDIT ) {
        edit_x = x - 1;
        edit_y = y - 1;
        if ( edit_sel_type == EDITOR_BRICK ) {
            if ( set ) {
                /* set brick */
                edit_cur_level->bricks[edit_x][edit_y] = edit_sel_id;
                *full_update = 1;
            }
            else {
                /* remove brick and extra if any */
                if ( edit_cur_level->bricks[edit_x][edit_y] != -1 ) {
                    edit_cur_level->bricks[edit_x][edit_y] = -1;
                    edit_cur_level->extras[edit_x][edit_y] = EX_NONE;
                    *full_update = 1;
                }
            }
        }
        else {
            if ( set ) {
                /* set extra - must be on a brick or beside a grow brick */
                if ( edit_cur_level->bricks[edit_x][edit_y] != -1 || near_grow_brick( edit_x, edit_y ) ) {
                    edit_cur_level->extras[edit_x][edit_y] = edit_sel_id;
                    *full_update = 1;
                }
            }
            else {
                /* remove extra */
                if ( edit_cur_level->extras[edit_x][edit_y] != EX_NONE ) {
                    edit_cur_level->extras[edit_x][edit_y] = EX_NONE;
                    *full_update = 1;
                }
            }
        }
    }
    /* buttons */
    editor_handle_button( edit_buttons[x][y], full_update );
    /* name&author */
    strcpy( str, "" );
    if ( edit_buttons[x][y] == BUTTON_EDIT_AUTHOR )
        if ( enter_string( font, _("Author's Name:"), str, 24 ) ) {
            snprintf( edit_cur_level->author, 31, "%s", str );
            *full_update = 1;
        }
    if ( edit_buttons[x][y] == BUTTON_EDIT_NAME )
        if ( enter_string( font, _("Title:"), str, 24 ) ) {
            snprintf( edit_cur_level->name, 31, "%s", str );
            *full_update = 1;
        }
    /* sel frame tile position */
    if ( sel ) {
        edit_sel_x = x;
        edit_sel_y = y;
        *full_update = 1;
    }
}

/*
====================================================================
Publics
====================================================================
*/
/*
====================================================================
Create/delete editor resources
====================================================================
*/
void editor_create()
{
    int i, j;
    /* clear all level pointers */
    memset( edit_levels, 0, sizeof( Level* ) * MAX_LEVELS );
    /* load sel frame */
    sel_frame = stk_surface_load( SDL_SWSURFACE, "sel_frame.png" );
    /* load buttons */
    buttons = stk_surface_load( SDL_SWSURFACE, "buttons.png" );
    /* background is black + frame */
    editor_bkgnd = stk_surface_create( SDL_SWSURFACE, stk_display->w, stk_display->h );
    SDL_SetColorKey( editor_bkgnd, 0, 0 );
    stk_surface_fill( editor_bkgnd, 0,0,-1,-1, 0x0 );
    /* add helping grid */
    editor_draw_grid();
    /* set actions */
    /* editable part */
    for ( i = 0; i < EDIT_WIDTH; i++ )
        for ( j = 0; j < EDIT_HEIGHT; j++ )
            edit_buttons[i + 1][j + 1] = BUTTON_EDIT;
    /* buttons */
    for ( i = 0; i < 11; i++ )
        edit_buttons[0][MAP_HEIGHT - 11 + i] = BUTTON_FIRST + i;
    edit_buttons[MAP_WIDTH - 1][MAP_HEIGHT - 1] = BUTTON_PLAY;
    /* name&author */
    for ( i = 1; i < MAP_WIDTH / 2; i++ )
        edit_buttons[i][MAP_HEIGHT - 5] = BUTTON_EDIT_NAME;
    for ( i = MAP_WIDTH / 2; i < MAP_WIDTH - 1; i++ )
        edit_buttons[i][MAP_HEIGHT - 5] = BUTTON_EDIT_AUTHOR;
    /* draw buttons */
    for ( i = 0; i < 11; i++ ) {
        stk_surface_blit( buttons, i * BRICK_WIDTH, 0,
            BRICK_WIDTH, BRICK_HEIGHT,
            editor_bkgnd, 0, ( MAP_HEIGHT - 11 + i ) * BRICK_HEIGHT );
    }
    stk_surface_blit( buttons, 11 * BRICK_WIDTH, 0,
        BRICK_WIDTH, BRICK_HEIGHT,
        editor_bkgnd, 
        ( MAP_WIDTH - 1 ) * BRICK_WIDTH, 
        ( MAP_HEIGHT - 1 ) * BRICK_HEIGHT );
}
void editor_delete()
{
    stk_surface_free( &editor_bkgnd );
    stk_surface_free( &sel_frame );
    stk_surface_free( &buttons );
}
/*
====================================================================
Initiate and clear stuff for each editor call.
file_name is the name of the edited file in home directory.
====================================================================
*/
int editor_init( char *file_name )
{
    FILE *file = 0;
    /* set full file name */
    snprintf( edit_file_name, sizeof(edit_file_name)-1, "%s/%s/lbreakout2-levels/%s", (getenv( "HOME" )?getenv( "HOME" ):"."), CONFIG_DIR_NAME, file_name );
    /* test this file for write access. use append to keep contents */
    if ( ( file = fopen( edit_file_name, "a" ) ) == 0 ) {
        fprintf( stderr, "Permission to write to file '%s' denied.\n", edit_file_name );
        return 0;
    }
    else
        fclose( file );
    /* load levels */
    editor_load_levels();
    /* select first level */
    edit_cur_level_id = 0;
    edit_cur_level = edit_levels[0];
    /* select first brick */
    edit_sel_type = EDITOR_EXTRA;
    editor_switch_tiles();
    /* clear other flags */
    extra_vis = 0;
    first_swap_level = -1;
    return 1;
}
void editor_clear()
{
    /* free all levels */
    editor_clear_levels();
}
/*
====================================================================
Run the editor
====================================================================
*/
void editor_run()
{
    SDL_Event event;
    int leave = 0;
    int ms;
    int last_switch_time = 0;
    int full_update = 0, set;
	int x, y, xoff,yoff;
	Uint8 buttonstate;
   
	/* reset any alpha keys */
	SDL_SetAlpha( extra_pic, 0,0 );
    /* draw first time */
    editor_full_update();
    /* main loop */
    stk_timer_reset();
    while ( !leave && !stk_quit_request ) {
        if ( SDL_PollEvent( &event ) ) {
            switch ( event.type ) {
                case SDL_QUIT: leave = 1; stk_quit_request = 1; break;
                case SDL_MOUSEBUTTONDOWN:
                    editor_handle_click( event.button.x / BRICK_WIDTH,
                                         event.button.y / BRICK_HEIGHT,
                                         (event.button.button == STK_BUTTON_LEFT),
                                         &full_update );
                    break;
                case SDL_KEYDOWN:
                    switch ( event.key.keysym.sym ) {
                        case SDLK_ESCAPE:
                            if ( confirm( font, _("Quit Editor? y/n"), CONFIRM_YES_NO ) ) leave = 1;
                            break;
                        case SDLK_LEFT: editor_handle_button( BUTTON_PREV, &full_update ); break;
                        case SDLK_RIGHT: editor_handle_button( BUTTON_NEXT, &full_update ); break;
                        case SDLK_UP: editor_handle_button( BUTTON_FIRST, &full_update ); break;
                        case SDLK_f:
                            config.fullscreen = !config.fullscreen;
                            stk_display_apply_fullscreen( config.fullscreen );
                            full_update = 1;
                            break;
                        default: break;
                    }
                    break;
                default: break;
            }
        }
        /* mouse motion is handled directly */
	buttonstate = SDL_GetRelativeMouseState( &xoff, &yoff );
	if ( (xoff || yoff) && buttonstate ) {
		buttonstate = SDL_GetMouseState( &x, &y );
		set = 0; if ( buttonstate & SDL_BUTTON(1) ) set = 1;
		editor_handle_click( x / BRICK_WIDTH, y / BRICK_HEIGHT, set, &full_update );
        }
        ms = stk_timer_get_time();
        if ( ( last_switch_time -= ms ) <= 0 ) {
            extra_vis = !extra_vis;
            last_switch_time = 500;
            editor_update_extra_bricks();
        }
        /* full update? */
        if ( full_update ) {
            editor_full_update();
            full_update = 0;
        }
        /* don't consume all CPU time */
        SDL_Delay( 5 );
    }
}

/***************************************************************************
                          config.c  -  description
                             -------------------
    begin                : Tue Feb 13 2001
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

#include <SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "sdl.h"
#include "config.h"
#include "parser.h"

Config config;

#define CONFIG_FILE_NAME "ltris.conf"

/* game type names. hacky place to define them but who cares? */
char gametype_names[8][64];
char gametype_ids[8][64];

/* check if config directory exists; if not create it and set config_dir */
void config_check_dir()
{
#ifdef DISABLE_INSTALL
    sprintf( config.dir_name, "." );
#else    
    snprintf( config.dir_name, sizeof(config.dir_name)-1, "%s/%s", getenv( "HOME" ), CONFIG_DIR_NAME );
#endif
    if ( opendir( config.dir_name ) == 0 ) {
        fprintf( stderr, "couldn't find/open config directory '%s'\n", config.dir_name );
        fprintf( stderr, "attempting to create it... " );
#ifdef WIN32
        mkdir( config.dir_name );
#else
        mkdir( config.dir_name, S_IRWXU );
#endif
        if ( opendir( config.dir_name ) == 0 )
            fprintf( stderr, "failed\n" );
        else
            fprintf( stderr, "ok\n" );
    }
}

/* set config to default */
void config_reset()
{
    /* game options */
    config.gametype = GAME_DEMO;
    config.starting_level = 0;
    config.preview = 1;
    config.help = 1;
    config.slow = 0;
    config.same_blocks_for_all = 1;
    strcpy( config.player1.name, "Michael" );
    strcpy( config.player2.name, "Sabine" );
    strcpy( config.player3.name, "Thomas" );
    config.expert = 0;
    /* multiplayer */
    config.holes = 2;
    config.rand_holes = 1;
    config.send_all = 0;
    config.send_tetris = 1;
    /* cpu */
    config.cpu_aggr = 1;
    config.cpu_delay = 700;
    config.cpu_rot_delay = 100;
    /* controls */
    config.hori_delay = 3;
    config.player1.controls.left = SDLK_LEFT;
    config.player1.controls.right = SDLK_RIGHT;
    config.player1.controls.rot_left = SDLK_UP;
    config.player1.controls.rot_right = SDLK_PAGEDOWN;
    config.player1.controls.down = SDLK_DOWN;
    config.player1.controls.drop = SDLK_END;
    config.player2.controls.left = 'a';
    config.player2.controls.right = 'd';
    config.player2.controls.rot_left = 'w';
    config.player2.controls.rot_right = 'e';
    config.player2.controls.down = 's';
    config.player2.controls.drop = 'y';
    config.player3.controls.left = SDLK_KP1;
    config.player3.controls.right = SDLK_KP3;
    config.player3.controls.rot_left = SDLK_KP5;
    config.player3.controls.rot_right = SDLK_KP6;
    config.player3.controls.down = SDLK_KP2;
    config.player3.controls.drop = SDLK_KP8;
    /* sound */
    config.sound = 1;
    config.volume = 6; /* 1 - 8 */
    /* graphics */
    config.trp = 1;
    config.anim = 1;
    config.fullscreen = 0;
    config.fade = 1;
    config.fps = 0; /* frames per second: 0 - no limit, 1 - 50, 2 - 100, 3 - 200 */
    config.bkgnd = 0;
    config.smooth_hori = 0;
    config.block_by_block = 0;
    /* lbreakout2 event data */
    config.rel_motion = 0;
    config.motion_mod = 100;
    config.invert = 0;
    config.grab = 0;
    /* various */
    config.quick_help = 1;
    config.visualize = 0;
    config.keep_bkgnd = 0;
    config.clear_keystate = 1;
    config.center_preview = 0;
    config.async_col_check = 0;
}

/* load config */
static void parse_player( PData *pd, Player *player )
{
    char *str;
    PData *sub;
    if ( parser_get_value( pd, "name", &str, 0 ) )
        strcpy( player->name, str );
    if ( parser_get_pdata( pd, "controls", &sub ) ) {
        parser_get_int( sub, "left", &player->controls.left );
        parser_get_int( sub, "right", &player->controls.right );
        parser_get_int( sub, "rot_left", &player->controls.rot_left );
        parser_get_int( sub, "rot_right", &player->controls.rot_right );
        parser_get_int( sub, "down", &player->controls.down );
        parser_get_int( sub, "drop", &player->controls.drop );
    }
}
void config_load( )
{
    char file_name[512];
    PData *pd, *sub; 
    /* set to defaults */
    config_check_dir();
    config_reset();
    /* load config */
    sprintf( file_name, "%s/%s", config.dir_name, CONFIG_FILE_NAME );
    if ( ( pd = parser_read_file( "config", file_name ) ) == 0 ) {
        fprintf( stderr, "%s\n", parser_get_error() );
        return;
    }
    /* parse config */
    parser_get_int( pd, "gametype", &config.gametype );
    parser_get_int( pd, "starting_level", &config.starting_level );
    if (config.starting_level > 9 || config.starting_level < 0) {
	    config.starting_level = 0;
    }
    parser_get_int( pd, "preview", &config.preview );
    parser_get_int( pd, "help", &config.help );
    parser_get_int( pd, "expert", &config.expert );
    parser_get_int( pd, "same_blocks", &config.same_blocks_for_all );
    parser_get_int( pd, "center_preview", &config.center_preview );
    parser_get_int( pd, "holes", &config.holes );
    parser_get_int( pd, "rand_holes", &config.rand_holes );
    parser_get_int( pd, "send_all", &config.send_all );
    parser_get_int( pd, "send_tetris", &config.send_tetris );
    if ( parser_get_pdata( pd, "player1", &sub ) )
        parse_player( sub, &config.player1 );
    if ( parser_get_pdata( pd, "player2", &sub ) )
        parse_player( sub, &config.player2 );
    if ( parser_get_pdata( pd, "player3", &sub ) )
        parse_player( sub, &config.player3 );
    parser_get_int( pd, "clear_keystate", &config.clear_keystate );
    parser_get_int( pd, "cpu_aggr", &config.cpu_aggr );
    parser_get_int( pd, "cpu_delay", &config.cpu_delay );
    parser_get_int( pd, "cpu_rot_delay", &config.cpu_rot_delay );
    parser_get_int( pd, "sound", &config.sound );
    parser_get_int( pd, "volume", &config.volume );
    parser_get_int( pd, "transparency", &config.trp );
    parser_get_int( pd, "animations", &config.anim );
    parser_get_int( pd, "fullscreen", &config.fullscreen );
    parser_get_int( pd, "fading", &config.fade );
    parser_get_int( pd, "fps", &config.fps );
    parser_get_int( pd, "background", &config.bkgnd );
    parser_get_int( pd, "static_background", &config.keep_bkgnd );
    parser_get_int( pd, "smooth_hori", &config.smooth_hori );
    parser_get_int( pd, "hori_delay", &config.hori_delay );
    parser_get_int( pd, "block_by_block", &config.block_by_block );
    parser_get_int( pd, "motion_mod", &config.motion_mod );
    parser_get_int( pd, "relative_motion", &config.rel_motion );
    parser_get_int( pd, "grap_input", &config.grab );
    parser_get_int( pd, "invert_mouse", &config.invert );
    parser_get_int( pd, "quick_help", &config.quick_help );
    parser_get_int( pd, "async_collision_check", &config.async_col_check );
    parser_free( &pd );
}

/* save config */
static void print_player( FILE *file, int i, Player *player )
{
    fprintf( file, "<player%i\n", i );
    fprintf( file, "name»%s\n", player->name );
    fprintf( file, "<controls\n" );
    fprintf( file, "left»%i\n", player->controls.left );
    fprintf( file, "right»%i\n", player->controls.right );
    fprintf( file, "rot_left»%i\n", player->controls.rot_left );
    fprintf( file, "rot_right»%i\n", player->controls.rot_right );
    fprintf( file, "down»%i\n", player->controls.down );
    fprintf( file, "drop»%i\n", player->controls.drop );
    fprintf( file, ">\n" );
    fprintf( file, ">\n" );
}
void config_save( )
{
    FILE *file = 0;
    char file_name[512];

    sprintf( file_name, "%s/%s", config.dir_name, CONFIG_FILE_NAME );
    if ( ( file = fopen( file_name, "w" ) ) == 0 )
        fprintf( stderr, "Cannot access config file '%s' to save settings\n", file_name );
    else {
        fprintf( file, "@\n" );
        fprintf( file, "gametype»%i\n", config.gametype );
        fprintf( file, "starting_level»%i\n", config.starting_level );
        fprintf( file, "preview»%i\n", config.preview );
        fprintf( file, "help»%i\n", config.help );
        fprintf( file, "expert»%i\n", config.expert );
        fprintf( file, "same_blocks»%i\n",config.same_blocks_for_all);
        fprintf( file, "center_preview»%i\n", config.center_preview );
        fprintf( file, "holes»%i\n", config.holes );
	fprintf( file, "rand_holes»%i\n", config.rand_holes );
        fprintf( file, "send_all»%i\n", config.send_all );
        fprintf( file, "send_tetris»%i\n", config.send_tetris );
        print_player( file, 1, &config.player1 );
        print_player( file, 2, &config.player2 );
        print_player( file, 3, &config.player3 );
        fprintf( file, "clear_keystate»%i\n", config.clear_keystate );
        fprintf( file, "cpu_aggr»%i\n", config.cpu_aggr );
        fprintf( file, "cpu_delay»%i\n", config.cpu_delay );
        fprintf( file, "cpu_rot_delay»%i\n", config.cpu_rot_delay );
        fprintf( file, "sound»%i\n", config.sound );
        fprintf( file, "volume»%i\n", config.volume );
        fprintf( file, "transparency»%i\n", config.trp );
        fprintf( file, "animations»%i\n", config.anim );
        fprintf( file, "fullscreen»%i\n", config.fullscreen );
        fprintf( file, "fading»%i\n", config.fade );
        fprintf( file, "fps»%i\n", config.fps );
        fprintf( file, "background»%i\n", config.bkgnd );
        fprintf( file, "static_background»%i\n", config.keep_bkgnd );
        fprintf( file, "smooth_hori»%i\n", config.smooth_hori );
        fprintf( file, "hori_delay»%i\n", config.hori_delay );
        fprintf( file, "block_by_block»%i\n", config.block_by_block );
        fprintf( file, "motion_mod»%i\n", config.motion_mod );
        fprintf( file, "relative_motion»%i\n", config.rel_motion );
        fprintf( file, "grab_input»%i\n", config.grab );
        fprintf( file, "invert_mouse»%i\n", config.invert );
        fprintf( file, "quick_help»%i\n", config.quick_help );
        fprintf( file, "async_collision_check»%i\n", config.async_col_check );
    }
}

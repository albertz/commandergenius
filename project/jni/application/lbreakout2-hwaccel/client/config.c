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

#include "lbreakout.h"

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "config.h"
#include "../common/parser.h"

#ifdef _WIN32
 #define MKDIR(d, p) mkdir(d)
#else
 #define MKDIR mkdir
#endif

Config config;

#define CONFIG_FILE_NAME "lbreakout2.conf"

/* check if config directory exists; if not create it and set config_dir */
void config_check_dir()
{
    char level_dir[512];
    snprintf( config.dir_name, sizeof(config.dir_name)-1, "%s/%s", (getenv( "HOME" )?getenv( "HOME" ):"."), CONFIG_DIR_NAME );
    /* test and create .lgames */
    if ( opendir( config.dir_name ) == 0 ) {
        fprintf( stderr, "couldn't find/open config directory '%s'\n", config.dir_name );
        fprintf( stderr, "attempting to create it... " );
        MKDIR( config.dir_name, S_IRWXU );
        if ( opendir( config.dir_name ) == 0 )
            fprintf( stderr, "failed\n" );
        else
            fprintf( stderr, "ok\n" );
    }
    /* create levels directory */
    sprintf( level_dir, "%s/lbreakout2-levels", config.dir_name );
    MKDIR( level_dir, S_IRWXU );
    /* create themes directory */
    sprintf( level_dir, "%s/lbreakout2-themes", config.dir_name );
    MKDIR( level_dir, S_IRWXU );
}

/* set config to default */
void config_reset()
{
    /* levels */
    config.levelset_id_local = 0;
    config.levelset_count_local = 0;
    config.levelset_id_home = 0;
    config.levelset_count_home = 0;
    /* player */
    config.player_count = 1;
    strcpy( config.player_names[0], "Michael" );
    strcpy( config.player_names[1], _("Mr.X") );
    strcpy( config.player_names[2], _("Mr.Y") );
    strcpy( config.player_names[3], _("Mr.Z") );
    /* game */
    config.diff = 2;
    config.startlevel = 0;
    config.rel_warp_limit = 80;
    config.addBonusLevels = 1;
    /* controls */
    config.k_left = SDLK_LEFT;
    config.k_right = SDLK_RIGHT;
    config.k_rfire = SDLK_SPACE;
    config.k_lfire = SDLK_y;
    config.k_return = SDLK_BACKSPACE;
    config.k_turbo = SDLK_x;
    config.k_warp = SDLK_w;
    config.k_maxballspeed = SDLK_c;
    config.grab = 1;
    config.rel_motion = 1;
    config.i_motion_mod = 120;
    config.motion_mod = 1.2;
    config.convex = 1;
    config.linear_corner = 0;
    config.invert = 0;
    config.i_key_speed = 500;
    config.key_speed = 0.5;
    config.random_angle = 1;
    config.maxballspeed_int1000 = 900;
    config.maxballspeed_float = 0.7;
    /* sounds */
    config.sound = 1;
    config.volume = 8;
    config.speech = 1;
    config.badspeech = 0;
    config.audio_buffer_size = 512;
    /* graphics */
    config.anim = 2;
    config.fullscreen = 0;
    config.fade = 1;
    config.bonus_info = 1;
    config.fps = 0;
	config.ball_level = BALL_BELOW_BONUS;
	config.debris_level = DEBRIS_ABOVE_BALL;
    /* various */
    config.use_hints = 1;
    config.return_on_click = 0;
    config.theme_id = 0;
    config.theme_count = 4;
    /* multiplayer */
    strcpy( config.server, "217.160.141.22:8000" );
    strcpy( config.local_port, "8001" );
    strcpy( config.username, _("player") );
    config.mp_diff = 1;
    config.mp_rounds = 1;
    config.mp_balls = 3;
    config.mp_frags = 10;
    config.recv_limit = 500;
}

/* load config */
void config_load( )
{
    char *str;
    char file_name[512];
    PData *pd; 
    /* set to defaults */
    config_check_dir();
    config_reset();
    /* load config */
    sprintf( file_name, "%s/%s", config.dir_name, CONFIG_FILE_NAME );
    if ( ( pd = parser_read_file( "config", file_name ) ) == 0 ) {
        fprintf( stderr, "%s\n", parser_get_error() );
        return;
    }
    /* assign */
    parser_get_int( pd, "set_id_local", &config.levelset_id_local );
    parser_get_int( pd, "set_count_local", &config.levelset_count_local );
    parser_get_int( pd, "set_id_home", &config.levelset_id_home );
    parser_get_int( pd, "set_count_home", &config.levelset_count_home );
    parser_get_int( pd, "player_count", &config.player_count );
    if ( parser_get_value( pd, "player0", &str, 0 ) )
        strcpy_lt( config.player_names[0], str, 31 );
    if ( parser_get_value( pd, "player1", &str, 0 ) )
        strcpy_lt( config.player_names[1], str, 31 );
    if ( parser_get_value( pd, "player2", &str, 0 ) )
        strcpy_lt( config.player_names[2], str, 31 );
    if ( parser_get_value( pd, "player3", &str, 0 ) )
        strcpy_lt( config.player_names[3], str, 31 );
    parser_get_int( pd, "diff", &config.diff );
    parser_get_int( pd, "starting_level", &config.startlevel );
    parser_get_int( pd, "rel_warp_limit", &config.rel_warp_limit );
    parser_get_int( pd, "add_bonus_levels", &config.addBonusLevels );
    parser_get_int( pd, "left", &config.k_left );
    parser_get_int( pd, "right", &config.k_right );
    parser_get_int( pd, "fire_left", &config.k_lfire );
    parser_get_int( pd, "fire_right", &config.k_rfire );
    parser_get_int( pd, "return", &config.k_return );
    parser_get_int( pd, "turbo", &config.k_turbo );
    parser_get_int( pd, "rel_motion", &config.rel_motion );
    parser_get_int( pd, "grab", &config.grab );
    parser_get_int( pd, "motion_mod", &config.i_motion_mod );
    config.motion_mod = 0.01 * config.i_motion_mod;
    parser_get_int( pd, "convex", &config.convex );
    parser_get_int( pd, "linear_corner", &config.linear_corner );
    parser_get_int( pd, "random_angle", &config.random_angle );
    parser_get_int( pd, "maxballspeed", &config.maxballspeed_int1000 );
    config.maxballspeed_float = (float)config.maxballspeed_int1000 / 1000;
    parser_get_int( pd, "invert", &config.invert );
    parser_get_int( pd, "sound", &config.sound );
    parser_get_int( pd, "volume", &config.volume );
    parser_get_int( pd, "speech", &config.speech );
    parser_get_int( pd, "badspeech", &config.badspeech );
    parser_get_int( pd, "audio_buffer_size", &config.audio_buffer_size );
    parser_get_int( pd, "anim", &config.anim );
    parser_get_int( pd, "fullscreen", &config.fullscreen );
    parser_get_int( pd, "fade", &config.fade );
    parser_get_int( pd, "bonus_info", &config.bonus_info );
    parser_get_int( pd, "fps", &config.fps );
    parser_get_int( pd, "ball_level", &config.ball_level );
    parser_get_int( pd, "debris_level", &config.debris_level );
    parser_get_int( pd, "i_key_speed", &config.i_key_speed );
    config.key_speed = 0.001 * config.i_key_speed;
    parser_get_int( pd, "use_hints", &config.use_hints );
    parser_get_int( pd, "return_on_click", &config.return_on_click );
    parser_get_int( pd, "theme_id", &config.theme_id );
    parser_get_int( pd, "theme_count", &config.theme_count );
    if ( parser_get_value( pd, "server", &str, 0 ) )
        strcpy_lt( config.server, str, 64 );
    if ( parser_get_value( pd, "local_port", &str, 0 ) )
        strcpy_lt( config.local_port, str, 6 );
    if ( parser_get_value( pd, "username", &str, 0 ) )
        strcpy_lt( config.username, str, 15 );
    parser_get_int( pd, "mp_diff", &config.mp_diff );
    parser_get_int( pd, "mp_rounds", &config.mp_rounds );
    parser_get_int( pd, "mp_frags", &config.mp_frags );
    parser_get_int( pd, "mp_balls", &config.mp_balls );
    parser_free( &pd );
}

/* save config */
void config_save( )
{
    FILE *file = 0;
    char file_name[512];

    sprintf( file_name, "%s/%s", config.dir_name, CONFIG_FILE_NAME );
    if ( ( file = fopen( file_name, "w" ) ) == 0 )
        fprintf( stderr, "Cannot access config file '%s' to save settings\n", file_name );
    else {
        fprintf( file, "@\n" );
        fprintf( file, "set_id_local»%i\n", config.levelset_id_local );
        fprintf( file, "set_count_local»%i\n", config.levelset_count_local );
        fprintf( file, "set_id_home»%i\n", config.levelset_id_home );
        fprintf( file, "set_count_home»%i\n", config.levelset_count_home );
        fprintf( file, "player_count»%i\n", config.player_count );
        fprintf( file, "player0»%s\n", config.player_names[0] );
        fprintf( file, "player1»%s\n", config.player_names[1] );
        fprintf( file, "player2»%s\n", config.player_names[2] );
        fprintf( file, "player3»%s\n", config.player_names[3] );
        fprintf( file, "diff»%i\n", config.diff );
        fprintf( file, "starting_level»%i\n", config.startlevel );
        fprintf( file, "rel_warp_limit»%i\n", config.rel_warp_limit );
        fprintf( file, "add_bonus_levels»%i\n", config.addBonusLevels );
        fprintf( file, "left»%i\n", config.k_left );
        fprintf( file, "right»%i\n", config.k_right );
        fprintf( file, "fire_left»%i\n", config.k_lfire );
        fprintf( file, "fire_right»%i\n", config.k_rfire );
        fprintf( file, "return»%i\n", config.k_return );
        fprintf( file, "turbo»%i\n", config.k_turbo );
        fprintf( file, "rel_motion»%i\n", config.rel_motion );
        fprintf( file, "grab»%i\n", config.grab );
        fprintf( file, "motion_mod»%i\n", config.i_motion_mod);
        fprintf( file, "convex»%i\n", config.convex );
        fprintf( file, "linear_corner»%i\n", config.linear_corner );
        fprintf( file, "random_angle»%i\n", config.random_angle );
        fprintf( file, "maxballspeed»%i\n", config.maxballspeed_int1000 );
        fprintf( file, "invert»%i\n", config.invert );
        fprintf( file, "sound»%i\n", config.sound );
        fprintf( file, "volume»%i\n", config.volume );
        fprintf( file, "speech»%i\n", config.speech );
        fprintf( file, "badspeech»%i\n", config.badspeech );
        fprintf( file, "audio_buffer_size»%i\n", config.audio_buffer_size );
        fprintf( file, "anim»%i\n", config.anim );
        fprintf( file, "fullscreen»%i\n", config.fullscreen );
        fprintf( file, "fade»%i\n", config.fade );
        fprintf( file, "bonus_info»%i\n", config.bonus_info );
        fprintf( file, "fps»%i\n", config.fps );
        fprintf( file, "ball_level»%i\n", config.ball_level );
        fprintf( file, "debris_level»%i\n", config.debris_level );
        fprintf( file, "i_key_speed»%i\n", config.i_key_speed );
        fprintf( file, "use_hints»%i\n", config.use_hints );
        fprintf( file, "return_on_click»%i\n", config.return_on_click );
        fprintf( file, "theme_id»%i\n", config.theme_id );
        fprintf( file, "theme_count»%i\n", config.theme_count );
        fprintf( file, "server»%s\n", config.server );
        fprintf( file, "local_port»%s\n", config.local_port );
        fprintf( file, "username»%s\n", config.username );
        fprintf( file, "mp_diff»%i\n", config.mp_diff );
        fprintf( file, "mp_rounds»%i\n", config.mp_rounds );
        fprintf( file, "mp_frags»%i\n", config.mp_frags );
        fprintf( file, "mp_balls»%i\n", config.mp_balls );
    }
}

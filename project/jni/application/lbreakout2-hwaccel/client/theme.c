/***************************************************************************
                          theme.c  -  description
                             -------------------
    begin                : Fri Feb 15 2002
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

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "lbreakout.h"
#include "config.h"
#include "theme.h"

extern SDL_Surface *stk_display;
extern Config config;
extern int ball_w, ball_h;
extern int shot_w, shot_h;
extern int paddle_cw;
/* theme name list */
char **theme_names = 0;
int theme_count = 0;
/* current theme name */
char theme_name[32] = "Default";

/*
====================================================================
Themeable gfx and sounds.
====================================================================
*/
SDL_Surface *exp_pic = 0;
SDL_Surface *exp_dark_pic = 0;
StkFont        *font = 0;
StkFont        *chat_font_normal = 0;
StkFont        *chat_font_error = 0;
StkFont        *chat_font_name = 0;
#ifdef AUDIO_ENABLED
StkSound *wav_click = 0;
StkSound *wav_damn = 0, *wav_dammit = 0, *wav_wontgiveup = 0, *wav_excellent = 0, *wav_verygood = 0;
#endif
SDL_Surface *brick_pic = 0;
SDL_Surface *brick_shadow = 0;
#ifdef AUDIO_ENABLED
StkSound *wav_boom = 0;
#endif
SDL_Surface *paddle_pic = 0;
SDL_Surface *paddle_shadow = 0;
SDL_Surface *weapon_pic = 0;
SDL_Surface *weapon_shadow = 0;
SDL_Surface *ammo_pic = 0;
#ifdef AUDIO_ENABLED
StkSound *wav_expand = 0, *wav_shrink = 0, *wav_frozen = 0;
#endif
SDL_Surface *ball_pic = 0; /* ball pictures */
SDL_Surface *ball_shadow = 0;
#ifdef AUDIO_ENABLED
StkSound *wav_reflect_brick = 0;
StkSound *wav_reflect_paddle = 0;
StkSound *wav_attach = 0;
#endif
SDL_Surface *shot_pic = 0;
SDL_Surface *shot_shadow = 0;
#ifdef AUDIO_ENABLED
StkSound *wav_shot = 0;
#endif
SDL_Surface *extra_pic = 0; /* graphics */
SDL_Surface *extra_shadow = 0;
SDL_Surface *wall_pic = 0;
#ifdef AUDIO_ENABLED
StkSound *wav_score = 0, *wav_metal = 0, *wav_std = 0, *wav_wall = 0, *wav_joker = 0;
StkSound *wav_goldshower = 0;
StkSound *wav_speedup = 0, *wav_speeddown = 0;
StkSound *wav_chaos = 0, *wav_darkness = 0, *wav_ghost = 0;
StkSound *wav_timeadd = 0, *wav_expl_ball = 0, *wav_weak_ball = 0;
StkSound *wav_bonus_magnet = 0, *wav_malus_magnet = 0, *wav_disable = 0;
#endif
SDL_Surface *shine_pic = 0;
SDL_Surface *frame_left = 0, *frame_top = 0, *frame_right = 0;
SDL_Surface *frame_left_shadow = 0, *frame_top_shadow = 0, *frame_right_shadow = 0;
SDL_Surface *frame_mp_left = 0, *frame_mp_right = 0; /* multiplayer variants */
SDL_Surface *frame_mp_left_shadow = 0, *frame_mp_right_shadow = 0;
SDL_Surface *lamps = 0; /* life lamps */
StkFont *display_font;
StkFont *display_highlight_font;
#ifdef AUDIO_ENABLED
StkSound *wav_life_up = 0, *wav_life_down = 0;
#endif
SDL_Surface **bkgnds = 0;
int bkgnd_count = 0;
SDL_Surface *nuke_bkgnd = 0; /* special background for nuke mode */
/* chatroom */
SDL_Surface *cr_up = 0, *cr_down = 0, *cr_track = 0, *cr_wallpaper = 0;
/* frame box */
SDL_Surface *fr_hori = 0, *fr_vert = 0, *fr_luc = 0, *fr_llc = 0, *fr_ruc = 0, *fr_rlc = 0;
/* menu manager */
StkFont *mfont = 0, *mhfont = 0, *mcfont = 0; /* font, highlight font, caption font */
SDL_Surface *mbkgnd = 0; /* menu background */
#ifdef AUDIO_ENABLED
StkSound *wav_menu_click = 0;
StkSound *wav_menu_motion = 0;
#endif
/* charts */
StkFont *cfont = 0, *chfont = 0, *ccfont = 0; /* normal, highlight and caption font */
/* warp icon (indicates you can now go to next level) */
SDL_Surface *warp_pic = 0;

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Build the theme directory from passed name. The resources are then
loaded from this directory and the full file name is saved to 
theme_path.
====================================================================
*/
static char theme_dir[512];
static char theme_path[512];
void theme_set_dir( char *name )
{
    if ( name[0] == '~' )
        snprintf( theme_dir, sizeof(theme_dir)-1, "%s/%s/lbreakout2-themes/%s", 
                 (getenv( "HOME" )?getenv( "HOME" ):"."), CONFIG_DIR_NAME, name + 1 );
    else {
        if ( STRCMP( SRC_DIR, "." ) ) {
            getcwd( theme_dir, 511 );
            strcat( theme_dir, "/gfx/" );
            strcat( theme_dir, name );
        }
        else
            snprintf( theme_dir, sizeof(theme_dir)-1, "%s/gfx/%s", SRC_DIR, name );
    }
}

/*
====================================================================
Load a picture or sound from theme directory or (if not found)
from original source directory.
====================================================================
*/
SDL_Surface* theme_load_surf( char *name )
{
    struct stat filestat;
    SDL_Surface *surf = 0;
    sprintf( theme_path, "%s/%s", theme_dir, name );
    if ( stat( theme_path, &filestat ) == -1 || 
         ( surf = stk_surface_load( SDL_SWSURFACE | SDL_NONFATAL, theme_path ) ) == 0 )
        surf = stk_surface_load( SDL_SWSURFACE, name );
    return surf;
}
#ifdef AUDIO_ENABLED
StkSound* theme_load_sound( char *name )
{
    struct stat filestat;
    StkSound *sound = 0;
    sprintf( theme_path, "%s/%s", theme_dir, name );
    if ( stat( theme_path, &filestat ) == -1 ||  
         ( sound = stk_sound_load( -1, theme_path ) ) == 0 )
        sound = stk_sound_load( -1, name );
    return sound;
}
#endif
StkFont* theme_load_font_fixed( char *name, int start, int len, int width ) 
{
    struct stat filestat;
    StkFont *font = 0;
    sprintf( theme_path, "%s/%s", theme_dir, name );
    if ( stat( theme_path, &filestat ) == -1 || 
         ( font = stk_font_load( SDL_SWSURFACE | SDL_NONFATAL, theme_path ) ) == 0 )
        font = stk_font_load( SDL_SWSURFACE, name );
    if ( font )
        SDL_SetColorKey( font->surface, SDL_SRCCOLORKEY, 
            stk_surface_get_pixel( font->surface, 0,0 ) );
    return font;
}

/*
====================================================================
The frame is a bit complex to be done.
====================================================================
*/
void theme_load_frame()
{
    Uint32 ckey;

    /* load resources */
    frame_left = theme_load_surf( "fr_left.png" );
    frame_top = theme_load_surf( "fr_top.png" );
    frame_right = theme_load_surf( "fr_right.png" );
    /* get colorkey */
    ckey = stk_surface_get_pixel( frame_right, frame_right->w - 1, 0 );
    SDL_SetColorKey( frame_left, SDL_SRCCOLORKEY, ckey );
    SDL_SetColorKey( frame_top, SDL_SRCCOLORKEY, ckey );
    SDL_SetColorKey( frame_right, SDL_SRCCOLORKEY, ckey );
    /* build shadows */
    frame_left_shadow = create_shadow( frame_left, 0, 0, frame_left->w, frame_left->h );
    frame_top_shadow = create_shadow( frame_top, 0, 0, frame_top->w, frame_top->h );
    frame_right_shadow = create_shadow( frame_right, 0, 0, frame_right->w, frame_right->h );
        
    /* build the columns for multiplayer */
    frame_mp_left = theme_load_surf( "fr_mp_left.png" );
    frame_mp_right = theme_load_surf( "fr_mp_right.png" );
    SDL_SetColorKey( frame_mp_left, SDL_SRCCOLORKEY, ckey );
    SDL_SetColorKey( frame_mp_right, SDL_SRCCOLORKEY, ckey );
    frame_mp_left_shadow = create_shadow( frame_mp_left, 0, 0, frame_mp_left->w, frame_mp_left->h );
    frame_mp_right_shadow = create_shadow( frame_mp_right, 0, 0, frame_mp_right->w, frame_mp_right->h );
    
    /* load other resources */
    lamps = theme_load_surf( "life.png" );
    display_font = theme_load_font_fixed( "f_frame.png", 32, 96, 8 );
    display_font->align = STK_FONT_ALIGN_RIGHT | STK_FONT_ALIGN_CENTER_Y;
    display_highlight_font = theme_load_font_fixed( "f_small_white.png", 32, 96, 8 );
    display_highlight_font->align = STK_FONT_ALIGN_RIGHT | STK_FONT_ALIGN_CENTER_Y;
	
	/* waves */
#ifdef AUDIO_ENABLED
    wav_life_up = theme_load_sound( "gainlife.wav" );
    wav_life_down = theme_load_sound( "looselife.wav" );
#endif
}

/*
====================================================================
Load backgrounds. If no background was found use the original
backgrounds. Else count the number of new backgrounds and use these.
====================================================================
*/
void theme_load_bkgnds()
{
    int i;
    struct stat filestat;
    sprintf( theme_path, "%s/back0.png", theme_dir );
    if ( stat( theme_path, &filestat ) == -1 ) {
        /* use original backs */
        bkgnds = calloc( ORIG_BACK_COUNT, sizeof( SDL_Surface* ) );
        for ( i = 0; i < ORIG_BACK_COUNT; i++ ) {
            sprintf( theme_path, "back%i.png", i );
            bkgnds[i] = stk_surface_load( SDL_SWSURFACE, theme_path );
        }
        bkgnd_count = ORIG_BACK_COUNT;
    }
    else {
        /* use new backs */
        bkgnd_count = -1;
        do {
            bkgnd_count++;
            sprintf( theme_path, "%s/back%i.png", theme_dir, bkgnd_count );
        } while ( stat( theme_path, &filestat ) == 0 );
        bkgnds = calloc( bkgnd_count, sizeof( SDL_Surface* ) );
        /* load backs */
        for ( i = 0; i < bkgnd_count; i++ ) {
            sprintf( theme_path, "%s/back%i.png", theme_dir, i );
            bkgnds[i] = stk_surface_load( SDL_SWSURFACE, theme_path );
        }
    }

    for ( i = 0; i < bkgnd_count; i++ )
	    SDL_SetColorKey( bkgnds[i], 0, 0x0 );
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Get/delete names of all themes found in themes directory.
====================================================================
*/
void theme_get_list()
{
    int i;
    int default_theme_count = 4;
    char *default_theme_names[] = {
        "AbsoluteB",
        "Oz",
        "Moiree",
        "Classic"
    };
    char name[64];
    char dir[256];
    DIR *hdir = 0;
    struct dirent *entry = 0;
    struct stat estat;
    List *list = 0;
    /* auxilary list */
    list = list_create( LIST_NO_AUTO_DELETE, LIST_NO_CALLBACK );
    /* install theme directory */
    snprintf( dir, sizeof(dir) - 1, "%s/gfx", SRC_DIR );
    if ( ( hdir = opendir( dir ) ) != 0 ) {
        while ( ( entry = readdir( hdir ) ) ) {
            if ( entry->d_name[0] == '.' ) 
                continue;
            /* full path */
            sprintf( theme_path, "%s/%s", dir, entry->d_name );
            stat( theme_path, &estat );
            if ( S_ISDIR( estat.st_mode ) ) {
                /* ignore entry if it is a default theme */
                for ( i = 0; i < default_theme_count; i++ )
                    if ( STRCMP( default_theme_names[i], entry->d_name ) ) {
                        i = -1;
                        break;
                    }
                if ( i == -1 ) continue;
                /* store it otherwise */
                sprintf( name, "%s", entry->d_name );
                list_add( list, strdup( name ) );
            }
        }
        closedir( hdir );
    }
    /* home theme directory */
    snprintf( dir, sizeof(dir)-1, "%s/%s/lbreakout2-themes", (getenv( "HOME" )?getenv( "HOME" ):"."), CONFIG_DIR_NAME );
    if ( ( hdir = opendir( dir ) ) != 0 ) {
        while ( ( entry = readdir( hdir ) ) ) {
            if ( entry->d_name[0] == '.' ) 
                continue;
            /* full path */
            sprintf( theme_path, "%s/%s", dir, entry->d_name );
            stat( theme_path, &estat );
            if ( S_ISDIR( estat.st_mode ) ) {
                sprintf( name, "~%s", entry->d_name );
                list_add( list, strdup( name ) );
            }
        }
        closedir( hdir );
    }
    /* create static list. the default themes are the
       first four entries in the order at the
       beginning of this function. */
    theme_count = list->count + default_theme_count;
    theme_names = calloc( theme_count, sizeof( char* ) );
    for ( i = 0; i < default_theme_count; i++ )
        theme_names[i] = strdup( default_theme_names[i] );
    for ( i = 0; i < list->count; i++ ) {
        theme_names[i + default_theme_count] = list_get( list, i );
    }
    list_delete( list );
    /* if config's theme count is not equal reset the theme_id to 0 */
    if ( theme_count != config.theme_count ) {
        config.theme_count = theme_count;
        config.theme_id = 0;
    }
}
void theme_delete_list()
{
    int i;
    if ( theme_names ) {
        for ( i = 0; i < theme_count; i++ )
            if ( theme_names[i] ) free( theme_names[i] );
        free( theme_names );
        theme_names = 0;
    }
}

/*
====================================================================
Load theme by name or delete current theme (is done by load too
so it might be used to change a theme as well.)
====================================================================
*/
void theme_load( char *name )
{
    SDL_Surface *surf;
    struct stat filestat;
    char aux[128];
    int i;
    printf( "Loading theme '%s'\n", name );
    /* clear resources */
    theme_delete();
    /* set theme directory */
    theme_set_dir( name );
    /* load resources */
    /* explosions */
    exp_pic = theme_load_surf( "explosions.png" );
    exp_dark_pic = theme_load_surf( "explosions_dark.png" );
    /* main game font */
    font = theme_load_font_fixed( "f_game.png", 32, 96, 10 );
    font->align = STK_FONT_ALIGN_CENTER_X | STK_FONT_ALIGN_CENTER_Y;
    /* normal chat font (messages by server) */
    chat_font_normal = theme_load_font_fixed( "f_small_yellow.png", 32, 96, 8 );
    /* error font (messages by server) */
    chat_font_error = theme_load_font_fixed( "f_small_red.png", 32, 96, 8 );
    /* username font (also for server) */
    chat_font_name = theme_load_font_fixed( "f_small_white.png", 32, 96, 8 );
    /* main game sounds */
#ifdef AUDIO_ENABLED
    wav_click = theme_load_sound( "click.wav" );
    wav_damn = theme_load_sound( "damn.wav" );
    wav_dammit = theme_load_sound( "dammit.wav" );
    wav_wontgiveup = theme_load_sound( "wontgiveup.wav" );
    wav_excellent = theme_load_sound( "excellent.wav" );
    wav_verygood = theme_load_sound( "verygood.wav" );
#endif
    /* brick pics and shadow template */
    brick_pic = theme_load_surf( "bricks.png" );
    /* default color key is: BLACK but you may specify a special
       key by adding a column of one pixel width at the end of the file */
    if ( brick_pic->w & 1 )
        SDL_SetColorKey( brick_pic, SDL_SRCCOLORKEY, stk_surface_get_pixel( brick_pic, brick_pic->w - 1, 0 ) );
    brick_shadow = create_shadow( brick_pic, 0, 0, brick_pic->w, brick_pic->h );
    /* brick destruction sound */
#ifdef AUDIO_ENABLED
    wav_boom = theme_load_sound( "exp.wav" );
#endif
    /* paddle and weapon */
    paddle_pic = theme_load_surf( "paddle.png" );
    SDL_SetColorKey( paddle_pic, SDL_SRCCOLORKEY, 
        stk_surface_get_pixel( paddle_pic, 0, 0 ) );
    if ( paddle_pic->w < 6 * paddle_cw ) {
        /* no top paddle specified so duplicate first one */
        surf = stk_surface_create( SDL_SWSURFACE, paddle_pic->w*2, paddle_pic->h );
        SDL_SetColorKey( surf, SDL_SRCCOLORKEY, paddle_pic->format->colorkey );
        SDL_FillRect( surf, 0, paddle_pic->format->colorkey );
        stk_surface_blit( paddle_pic, 0,0,-1,-1, surf, 0,0 );
        stk_surface_blit( paddle_pic, 0,0,-1,-1, surf, paddle_pic->w,0 );
        stk_surface_free( &paddle_pic ); paddle_pic = surf;
    }
    weapon_pic = theme_load_surf( "weapon.png" );
    paddle_shadow = create_shadow( paddle_pic, 0, 0, paddle_pic->w, paddle_pic->h );
    weapon_shadow = create_shadow( weapon_pic, 0, 0, weapon_pic->w, weapon_pic->h );
    ammo_pic = theme_load_surf( "ammo.png" );
    SDL_SetColorKey( ammo_pic, SDL_SRCCOLORKEY, 0x0 );
    /* paddle sounds */
#ifdef AUDIO_ENABLED
    wav_expand = theme_load_sound( "expand.wav" );
    wav_shrink = theme_load_sound( "shrink.wav" );
    wav_frozen = theme_load_sound( "freeze.wav" );
#endif
    /* ball gfx */
    ball_pic = theme_load_surf( "ball.png" );
    SDL_SetColorKey( ball_pic, SDL_SRCCOLORKEY, stk_surface_get_pixel( ball_pic, 0, 0 ) );
    ball_shadow = create_shadow( ball_pic, 0, 0, ball_w, ball_h );
    /* ball sounds */
#ifdef AUDIO_ENABLED
    wav_reflect_brick = theme_load_sound( "reflect_brick.wav" );
    wav_reflect_paddle = theme_load_sound( "reflect_paddle.wav" );
    wav_attach = theme_load_sound( "attach.wav" );
#endif
    /* shot gfx */
    shot_pic = theme_load_surf( "shot.png" );
    shot_shadow = create_shadow( shot_pic, 0, 0, shot_w, shot_h );
#ifdef AUDIO_ENABLED
    wav_shot = theme_load_sound( "shot.wav" );
#endif
    /* extra pic */
    extra_pic = theme_load_surf( "extras.png" );
	SDL_SetColorKey( extra_pic, 0, 0 );
    /* default color key is: disabled but you may specify a special
       key by adding a column of one pixel width at the end of the file */
    if ( extra_pic->w & 1 )
        SDL_SetColorKey( extra_pic, SDL_SRCCOLORKEY, stk_surface_get_pixel( extra_pic, extra_pic->w - 1, 0 ) );
    extra_shadow = create_shadow( extra_pic, 0, 0, extra_pic->w, extra_pic->h );
    /* extra sounds */
#ifdef AUDIO_ENABLED
    wav_score = theme_load_sound( "score.wav" );
    wav_metal = theme_load_sound( "metal.wav" );
    wav_speedup = theme_load_sound( "speedup.wav" );
    wav_speeddown = theme_load_sound( "speeddown.wav" );
    wav_std = theme_load_sound( "standard.wav" );
    wav_wall = theme_load_sound( "wall.wav" );
    wav_joker = theme_load_sound( "joker.wav" );
    wav_goldshower = theme_load_sound( "goldshower.wav" );
    wav_chaos = theme_load_sound( "chaos.wav" );
    wav_darkness = theme_load_sound( "darkness.wav" );
    wav_ghost = theme_load_sound( "ghost.wav" );
    wav_timeadd = theme_load_sound( "timeadd.wav" );
    wav_expl_ball = theme_load_sound( "expl_ball.wav" );
    wav_weak_ball = theme_load_sound( "weak_ball.wav" );
    wav_bonus_magnet = theme_load_sound( "bonus_magnet.wav" );
    wav_malus_magnet = theme_load_sound( "malus_magnet.wav" );
    wav_disable = theme_load_sound( "disable.wav" );
#endif
    /* shine pic */
    shine_pic = theme_load_surf( "shine.png" );
    /* chatroom */
    cr_up = theme_load_surf( "scroll_up.png" );
    cr_down = theme_load_surf( "scroll_down.png" );
    cr_track = theme_load_surf( "scroll_track.png" );
    cr_wallpaper = theme_load_surf( "cr_back.png" );
    SDL_SetColorKey( cr_wallpaper, 0,0 );
    /* frame box */
    fr_hori = theme_load_surf( "fr_hori.png" );
    fr_vert = theme_load_surf( "fr_vert.png" );
    fr_luc = theme_load_surf( "fr_luc.png" );
    fr_llc = theme_load_surf( "fr_llc.png" );
    fr_ruc = theme_load_surf( "fr_ruc.png" );
    fr_rlc = theme_load_surf( "fr_rlc.png" );
    /* frame */
    theme_load_frame();
    /* wall */
    sprintf( theme_path, "%s/floor.png", theme_dir );
    if ( stat( theme_path, &filestat ) == -1 ) {
        /* build default floor */
        wall_pic = stk_surface_create( 
            SDL_SWSURFACE, stk_display->w - 2 * BRICK_WIDTH, 
            BRICK_HEIGHT );
        /* bricks must have been loaded at this point! */
        for ( i = 0; i < MAP_WIDTH - 2; i++ )
            stk_surface_blit( brick_pic, 0, 0,
                BRICK_WIDTH, BRICK_HEIGHT, 
                wall_pic, i * BRICK_WIDTH, 0 );
    }
    else {
        /* load floor */
        wall_pic = theme_load_surf( "floor.png" );
    }
    /* backgrounds */
    theme_load_bkgnds();
    /* build nuke bkgnd */
    nuke_bkgnd = stk_surface_create( SDL_SWSURFACE,
        stk_display->w, stk_display->h );
    surf = stk_surface_load( SDL_SWSURFACE, "nukeback.png" );
    stk_surface_apply_wallpaper( nuke_bkgnd, 0,0,-1,-1,
        surf, 128 );
    stk_surface_free( &surf );
    stk_surface_gray( nuke_bkgnd, 0,0,-1,-1, 0 );
    /* manager */
    mbkgnd = theme_load_surf( "menuback.png" );
    SDL_SetColorKey( mbkgnd, 0, 0 );
    mfont = theme_load_font_fixed( "f_small_yellow.png", 32, 96, 8 );
    mhfont = theme_load_font_fixed( "f_white.png", 32, 96, 10 );
    mcfont = theme_load_font_fixed( "f_yellow.png", 32, 96, 10 );
#ifdef AUDIO_ENABLED
    wav_menu_click = theme_load_sound( "menu_click.wav" );
    wav_menu_motion = theme_load_sound( "menu_motion.wav" );
#endif
    /* add version to background */
    mfont->align = STK_FONT_ALIGN_RIGHT | STK_FONT_ALIGN_BOTTOM;
    sprintf( aux, "v%s", VERSION );
    stk_font_write( mfont, mbkgnd, mbkgnd->w - 4, stk_display->h - 4 - mfont->height, STK_OPAQUE, aux );
    stk_font_write( mfont, mbkgnd, mbkgnd->w - 4, stk_display->h - 4, STK_OPAQUE, "http://lgames.sf.net" );
    /* charts */
    /* load resources */
    cfont = theme_load_font_fixed( "f_small_yellow.png", 32, 96, 8 );
    chfont = theme_load_font_fixed( "f_small_white.png", 32, 96, 8 );
    ccfont = theme_load_font_fixed( "f_yellow.png", 32, 96, 10 );
    /* warp picture */
    warp_pic = theme_load_surf( "warp.png" );
}
void theme_delete()
{
    int i;
    stk_surface_free( &exp_pic );
    stk_surface_free( &exp_dark_pic );
    stk_font_free( &font );
    stk_font_free( &chat_font_normal );
    stk_font_free( &chat_font_error );
    stk_font_free( &chat_font_name );
#ifdef AUDIO_ENABLED
    stk_sound_free( &wav_click );
    stk_sound_free( &wav_damn );
    stk_sound_free( &wav_dammit );
    stk_sound_free( &wav_wontgiveup );
    stk_sound_free( &wav_excellent );
    stk_sound_free( &wav_verygood );
#endif
    stk_surface_free( &brick_pic );
    stk_surface_free( &brick_shadow );
#ifdef AUDIO_ENABLED
    stk_sound_free( &wav_boom );
#endif
    stk_surface_free( &paddle_pic );
    stk_surface_free( &weapon_pic );
    stk_surface_free( &ammo_pic );
    stk_surface_free( &paddle_shadow );
    stk_surface_free( &weapon_shadow );
#ifdef AUDIO_ENABLED
    stk_sound_free( &wav_expand );
    stk_sound_free( &wav_shrink );
    stk_sound_free( &wav_frozen );
#endif
    stk_surface_free( &ball_pic );
    stk_surface_free( &ball_shadow );
#ifdef AUDIO_ENABLED
    stk_sound_free( &wav_reflect_paddle );
    stk_sound_free( &wav_reflect_brick );
    stk_sound_free( &wav_attach );
#endif
    stk_surface_free( &shot_pic );
    stk_surface_free( &shot_shadow );
#ifdef AUDIO_ENABLED
    stk_sound_free( &wav_shot );
#endif
    stk_surface_free( &extra_pic );
    stk_surface_free( &extra_shadow );
    stk_surface_free( &wall_pic );
#ifdef AUDIO_ENABLED
    stk_sound_free( &wav_score );
    stk_sound_free( &wav_metal );
    stk_sound_free( &wav_speedup );
    stk_sound_free( &wav_speeddown );
    stk_sound_free( &wav_std );
    stk_sound_free( &wav_wall );
    stk_sound_free( &wav_joker );
    stk_sound_free( &wav_goldshower );
    stk_sound_free( &wav_chaos );
    stk_sound_free( &wav_darkness );
    stk_sound_free( &wav_ghost );
    stk_sound_free( &wav_timeadd );
    stk_sound_free( &wav_expl_ball );
    stk_sound_free( &wav_weak_ball );
    stk_sound_free( &wav_bonus_magnet );
    stk_sound_free( &wav_malus_magnet );
    stk_sound_free( &wav_disable );
#endif
    stk_surface_free( &shine_pic );
    stk_surface_free( &frame_left );
    stk_surface_free( &frame_right );
    stk_surface_free( &frame_top );
    stk_surface_free( &frame_left_shadow );
    stk_surface_free( &frame_right_shadow );
    stk_surface_free( &frame_top_shadow );
    stk_surface_free( &lamps );
    stk_font_free( &display_font );
    stk_font_free( &display_highlight_font );
#ifdef AUDIO_ENABLED
    stk_sound_free( &wav_life_up );
    stk_sound_free( &wav_life_down );
#endif
    if ( bkgnds ) {
        for ( i = 0; i < bkgnd_count; i++ )
            stk_surface_free( &bkgnds[i] );
        free( bkgnds );
        bkgnds = 0;
        bkgnd_count = 0;
    }
    stk_surface_free( &nuke_bkgnd );
    /* chatroom */
    stk_surface_free( &cr_up );
    stk_surface_free( &cr_down );
    stk_surface_free( &cr_track );
    stk_surface_free( &cr_wallpaper );
    /* frame box */
    stk_surface_free( &fr_vert );
    stk_surface_free( &fr_hori );
    stk_surface_free( &fr_luc );
    stk_surface_free( &fr_llc );
    stk_surface_free( &fr_ruc );
    stk_surface_free( &fr_rlc );
    /* manager */
    stk_surface_free( &mbkgnd );
    stk_font_free( &mfont );
    stk_font_free( &mhfont );
    stk_font_free( &mcfont );
#ifdef AUDIO_ENABLED
    stk_sound_free( &wav_menu_click );
    stk_sound_free( &wav_menu_motion );
#endif
    /* charts */
    stk_font_free( &cfont );
    stk_font_free( &chfont );
    stk_font_free( &ccfont );
    /* warp picture */
    stk_surface_free( &warp_pic );
}

/*
====================================================================
Check if there is a file ABOUT in the theme directory and return
it's contents up to limit characters.
If this file isn't found 'NO INFO' is set.
====================================================================
*/
void theme_get_info( char *theme_name, char *text, int limit )
{
    FILE *file = 0;
    int i, count;
    theme_set_dir( theme_name );
    sprintf( theme_path, "%s/ABOUT", theme_dir );
    if ( ( file = fopen( theme_path, "rb" ) ) ) {
        count = fread( text, 1, limit, file );
        fclose( file );
        text[(count>=0)?count:0] = 0;
        for ( i = 0; i < strlen( text ); i++ )
            if ( text[i] == 10 ) 
                text[i] = '#';
        /* remove empty lines */
        for ( i = strlen( text ) - 1; i >= 0; i++ )
            if ( text[i] == '#' && i > 0 && text[i-1] == '#' )
                text[i] = 0;
            else
                break;
    }
    else
        strcpy_lt( text, _("NO INFO AVAILABLE"), limit );
}

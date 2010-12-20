/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Die Feb 29 13:27:43 MET 2000
    copyright            : (C) 2000 by Michael Speck
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "ltris.h"
#include "event.h"
#include "chart.h"
#include "manager.h"
#include "tetris.h"
#include "hint.h"

int term_game = 0;
extern Sdl sdl;
extern Config config;
extern char gametype_names[8][64];
extern char gametype_ids[8][64];

int main(int argc, char *argv[])
{
    int result = ACTION_NONE;
    int leave = 0;

    /* i18n */
#ifdef ENABLE_NLS
    setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
    
    /* ltris info */
    printf( "LTris %s\nCopyright 2002-2010 Michael Speck\nPublished under GNU GPL\n---\n", VERSION );
    printf( "Looking up data in: %s\n", SRC_DIR );
#ifndef SOUND
    printf( "Compiled without sound and music\n" );
#endif

    set_random_seed(); /* set random seed */

    /* game ids - not translated to be fixed independant of language */
    strcpy(gametype_ids[0],"demo");
    strcpy(gametype_ids[1],"classic");
    strcpy(gametype_ids[2],"figures");
    strcpy(gametype_ids[3],"vshuman");
    strcpy(gametype_ids[4],"vscpu");
    strcpy(gametype_ids[5],"vshumanhuman");
    strcpy(gametype_ids[6],"vshumancpu");
    strcpy(gametype_ids[7],"vscpucpu");
    /* game names - translated for display */
    strcpy(gametype_names[0],_("Demo"));
    strcpy(gametype_names[1],_("Classic"));
    strcpy(gametype_names[2],_("Figures"));
    strcpy(gametype_names[3],_("Vs Human"));
    strcpy(gametype_names[4],_("Vs CPU"));
    strcpy(gametype_names[5],_("Vs Human&Human"));
    strcpy(gametype_names[6],_("Vs Human&CPU"));
    strcpy(gametype_names[7],_("Vs CPU&CPU"));
    config_load();

    init_sdl( SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER );
    set_video_mode( std_video_mode( config.fullscreen ) );
    SDL_WM_SetCaption( "LTris", 0 );
    sdl.fade = config.fade;
    SDL_SetEventFilter( event_filter );
#ifdef SOUND
    audio_open();
    sound_enable( config.sound );
    sound_volume( config.volume * 16 );
#endif

    /* create */
    hint_load_res();
    manager_create();    
    tetris_create();
    chart_load();
    /* run game */
    manager_fade( FADE_IN );
    while( !leave && !term_game ) {
        result = manager_run();
        switch( result ) {
            case ACTION_QUIT: leave = 1; break;
            case ACTION_MAKE_STAT:
                manager_fade( FADE_OUT );
                tetris_make_stat();
                manager_fade( FADE_IN );
                break;
            case ACTION_PLAY:
                manager_fade( FADE_OUT );
                if ( tetris_init() ) {
                    tetris_run();
                    tetris_clear();
                }
                manager_fade( FADE_IN );
                break;
            default: break;
        }
    }
    manager_fade( FADE_OUT );
    /* delete stuff */
    tetris_delete();
	manager_delete();
	chart_save();
    chart_delete();
    hint_delete_res();
    
#ifdef SOUND
    audio_close();
#endif
    config_save();

    return EXIT_SUCCESS;
}

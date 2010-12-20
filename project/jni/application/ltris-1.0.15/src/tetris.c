/***************************************************************************
                          tetris.c  -  description
                             -------------------
    begin                : Tue Dec 25 2001
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

#include "ltris.h"
#include "event.h"
#include "chart.h"
#include "bowl.h"
#include "shrapnells.h"
#include "tetris.h"

SDL_Surface *blocks = 0;
SDL_Surface *logo = 0;
Font *font = 0, *large_font = 0;
SDL_Surface *qmark = 0; /* question mark */
SDL_Surface *bkgnd = 0; /* background + 3dframes */
SDL_Surface *offscreen = 0; /* offscreen: background + blocks */
int last_bkgnd_id = -99; /* last background chosen */
#ifdef SOUND
Sound_Chunk *wav_click = 0;
#endif
Bowl *bowls[BOWL_COUNT]; /* all bowls */
int  *next_blocks, next_blocks_size = 0; /* buffered game blocks for
                                            games where all players
                                            will receive the same
                                            blocks */

extern Sdl sdl;
extern Config config;
extern int term_game;
extern char gametype_ids[8][64];

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Draw the tetris logo to the offscreen.
====================================================================
*/
void tetris_draw_logo()
{
    int x = 460, y = 100;
    draw_3dframe( offscreen, x - logo->w / 2 - 4, y - logo->h / 2 - 4, logo->w + 8, logo->h + 8, 6 );
    DEST( offscreen, x - logo->w / 2, y - logo->h / 2, logo->w, logo->h );
    FULL_SOURCE( logo );
    blit_surf();
}

/*
====================================================================
Request confirmation.
====================================================================
*/
enum{ CONFIRM_YES_NO, CONFIRM_ANY_KEY, CONFIRM_PAUSE };
void draw_confirm_screen( Font *font, SDL_Surface *buffer, char *str )
{
    FULL_DEST(sdl.screen);
    fill_surf(0x0);
    font->align = ALIGN_X_CENTER | ALIGN_Y_CENTER;
    write_text(font, sdl.screen, sdl.screen->w / 2, sdl.screen->h / 2, str, 0);
}
int confirm( Font *font, char *str, int type )
{
    SDL_Event event;
    int go_on = 1;
    int ret = 0;
    SDL_Surface *buffer = create_surf(sdl.screen->w, sdl.screen->h, SDL_SWSURFACE);
    SDL_SetColorKey(buffer, 0, 0);

#ifdef SOUND
    sound_play( wav_click );
#endif

    FULL_DEST(buffer);
    FULL_SOURCE(sdl.screen);
    blit_surf();
    draw_confirm_screen( font, buffer, str );
    refresh_screen( 0, 0, 0, 0 );

    while (go_on && !term_game) {
        SDL_WaitEvent(&event);
        /* TEST */
        switch ( event.type ) {
            case SDL_QUIT: term_game = 1; break;
            case SDL_KEYUP:
                if ( type == CONFIRM_ANY_KEY ) {
                    ret = 1; go_on = 0;
                    break;
                }
                else
                if ( type == CONFIRM_PAUSE ) {
                    if ( event.key.keysym.sym == SDLK_p ) {
                        ret = 1; go_on = 0;
                        break;
                    }
					else
					if ( event.key.keysym.sym == SDLK_f ) {
						config.fullscreen = !config.fullscreen;
						set_video_mode( std_video_mode( config.fullscreen ) );
						draw_confirm_screen( font, buffer, str );
						refresh_screen( 0, 0, 0, 0 );
					}
                }
                else
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        // ESC always returns false
                        go_on = 0;
                        ret = 0;					
                    } else {
                        // check pressed key against yes/no letters
                        char *keyName = SDL_GetKeyName(event.key.keysym.sym);
                        char *yesLetter = _("y");
                        char *noLetter = _("n");
                        if (strcmp(keyName, yesLetter) == 0) {
                            go_on = 0;
                            ret = 1;
                        }
                        if (strcmp(keyName, noLetter) == 0) {
                            go_on = 0;
                            ret = 0;
                        }
                    }
                break;
        }
    }
#ifdef SOUND
    sound_play( wav_click );
#endif
    FULL_DEST(sdl.screen);
    FULL_SOURCE(buffer);
    blit_surf();
    refresh_screen( 0, 0, 0, 0 );
    SDL_FreeSurface(buffer);
    reset_timer();
    
    return ret;
}
/*
====================================================================
Do a new background to bkgnd and add all nescessary frames for the
specified gametype in config::gametype.
Id == -1 means to use a random background.
Copy result to offscreen and screen. Draws bowl contents.
====================================================================
*/
enum { BACK_COUNT = 6 };
void tetris_recreate_bkgnd( int id )
{
    SDL_Surface *pic = 0;
    char name[32];
    int i, j;

    /* load and wallpaper background */
    if ( id == -1 ) {
        do {
            id = rand() % BACK_COUNT;
        } while ( id == last_bkgnd_id );
        last_bkgnd_id = id;
    }
    FULL_DEST( bkgnd ); fill_surf( 0x0 );
    /* load background */
    sprintf( name, "back%i.bmp", id );
    if ( ( pic = load_surf( name, SDL_SWSURFACE | SDL_NONFATAL ) ) != 0 ) {
        for ( i = 0; i < bkgnd->w; i += pic->w )
            for ( j = 0; j < bkgnd->h; j += pic->h ) {
                DEST( bkgnd, i, j, pic->w, pic->h );
                SOURCE( pic, 0, 0 );
                blit_surf();
            }
        SDL_FreeSurface( pic );
    }
    /* let the bowls contribute to the background :) */
    for ( i = 0; i < BOWL_COUNT; i++ )
        if ( bowls[i] )
            bowl_draw_frames( bowls[i] );
    /* draw to offscreen */
    FULL_DEST( offscreen ); FULL_SOURCE( bkgnd ); blit_surf();
    /* add logo if place */
    if ( config.gametype <= 2 )
        tetris_draw_logo();
    /* draw bowl contents */
    for ( i = 0; i < BOWL_COUNT; i++ )
        if ( bowls[i] )
            bowl_draw_contents( bowls[i] );
    /* put offscreen to screen */
    FULL_DEST( sdl.screen ); FULL_SOURCE( offscreen ); blit_surf();
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Load/delete all tetris resources.
====================================================================
*/
void tetris_create()
{
    logo = load_surf( "logo.bmp", SDL_SWSURFACE );
    SDL_SetColorKey( logo, SDL_SRCCOLORKEY, get_pixel( logo, 0, 0 ) );
    blocks = load_surf( "blocks.bmp", SDL_SWSURFACE );
    qmark = load_surf( "quest.bmp", SDL_SWSURFACE );
    SDL_SetColorKey( qmark, SDL_SRCCOLORKEY, get_pixel( qmark, 0, 0 ) );
    font = load_fixed_font( "f_small_yellow.bmp", 32, 96, 8 );
    large_font = load_fixed_font( "f_white.bmp", 32, 96, 10 );
    bkgnd = create_surf( sdl.screen->w, sdl.screen->h, SDL_SWSURFACE );
    SDL_SetColorKey( bkgnd, 0, 0 );
    offscreen = create_surf( sdl.screen->w, sdl.screen->h, SDL_SWSURFACE );
    SDL_SetColorKey( offscreen, 0, 0 );
    bowl_load_figures();
    bowl_init_block_masks();
#ifdef SOUND
    wav_click = sound_chunk_load( "click.wav" );
#endif
}
void tetris_delete()
{
    if ( logo ) SDL_FreeSurface( logo ); logo = 0;
    if ( blocks ) SDL_FreeSurface( blocks ); blocks = 0;
    if ( qmark ) SDL_FreeSurface( qmark ); qmark = 0;
    free_font( &font );
    free_font( &large_font );
    if ( offscreen ) SDL_FreeSurface( offscreen ); offscreen = 0;
    if ( bkgnd ) SDL_FreeSurface( bkgnd ); bkgnd = 0;
#ifdef SOUND        
    if ( wav_click ) sound_chunk_free( wav_click ); wav_click = 0;
#endif        
}

/*
====================================================================
Initiate/clear a new game from config data.
After tetris_init() the screen is drawn completely though not
updated to use the fade effect.
====================================================================
*/
int  tetris_init()
{
    /* create next block list if desired and multiplayer */
    if ( config.same_blocks_for_all && !config.expert &&
         config.gametype >= 3 )
    {
        next_blocks_size = NEXT_BLOCKS_CHUNK_SIZE;
        next_blocks = calloc( next_blocks_size, sizeof(int) );
        fill_int_array_rand(next_blocks,
                            0,next_blocks_size,
                            0,BLOCK_COUNT-1);
    }
    /* create bowls according to the gametype */
    switch ( config.gametype ) {
        case GAME_DEMO:
            bowls[0] = bowl_create( 70, 0, 460, 300, blocks, qmark, "Demo", 0 );
            break;
        case GAME_CLASSIC:
        case GAME_FIGURES:
            bowls[0] = bowl_create( 70, 0, 460, 300, blocks, qmark, config.player1.name, &config.player1.controls );
            break;
        case GAME_VS_HUMAN:
        case GAME_VS_CPU:
            if ( config.center_preview ) {
                bowls[0] = bowl_create( 20, 0, 273, 200, blocks, qmark, config.player1.name, &config.player1.controls );
                if ( config.gametype == GAME_VS_HUMAN )
                    bowls[1] = bowl_create( 420, 0, 367, 200, blocks, qmark, config.player2.name, &config.player2.controls );
                else
                    bowls[1] = bowl_create( 420, 0, 367, 200, blocks, qmark, "CPU-1", 0 );
            }
            else {
                bowls[0] = bowl_create( 20, 0, 290, 340, blocks, qmark, config.player1.name, &config.player1.controls );
                if ( config.gametype == GAME_VS_HUMAN )
                    bowls[1] = bowl_create( 420, 0, 350, 60, blocks, qmark, config.player2.name, &config.player2.controls );
                else
                    bowls[1] = bowl_create( 420, 0, 350, 60, blocks, qmark, "CPU-1", 0 );
            }
            break;
        case GAME_VS_HUMAN_HUMAN:
        case GAME_VS_HUMAN_CPU:
        case GAME_VS_CPU_CPU:
            bowls[0] = bowl_create( 10, 0, -1, -1, blocks, qmark, config.player1.name, &config.player1.controls );
            if ( config.gametype != GAME_VS_CPU_CPU )
                bowls[1] = bowl_create( 220, 0, -1, -1, blocks, qmark, config.player2.name, &config.player2.controls );
            else
                bowls[1] = bowl_create( 220, 0, -1, -1, blocks, qmark, "CPU-1", 0 );
            if ( config.gametype == GAME_VS_HUMAN_HUMAN )
                bowls[2] = bowl_create( 430, 0, -1, -1, blocks, qmark, config.player3.name, &config.player3.controls );
            else
                bowls[2] = bowl_create( 430, 0, -1, -1, blocks, qmark, "CPU-2", 0 );
            break;
    }
    /* background */
    tetris_recreate_bkgnd( 1 );
    /* shrapnells */
    shrapnells_init();
    return 1;
}
void tetris_clear()
{
    int i;
    
    if ( config.same_blocks_for_all && !config.expert &&
         config.gametype >= 3 )
    {
        next_blocks_size = 0;
        free( next_blocks );
    }
    for ( i = 0; i < BOWL_COUNT; i++ ) {
        if ( bowls[i] ) 
            bowl_delete( bowls[i] );
        bowls[i] = 0;
    }
    /* shrapnells */
    shrapnells_delete();
}

/*
====================================================================
Run a successfully initated game.
====================================================================
*/
void tetris_run()
{
    SDL_Event event;
    int leave = 0;
    int i;
    int fps_delay = 0;
    int ms;
    int request_pause = 0;
    int game_over = 0;
    int bkgnd_level = 0;
    int bowl_count; /* number of active bowls */
    char sshot_str[128];
    int screenshot_id = 0;
    int gain_multiplayer_bonus = 0;
    int escape = 0;
    
    /* count number of bowls */
    bowl_count = 0;
    for ( i = 0; i < BOWL_COUNT; i++ )
        if ( bowls[i] ) 
            bowl_count++;
    
    SDL_ShowCursor( 0 );
    /* delay */
    switch ( config.fps ) {
        case 1: fps_delay = 20; break;
        case 2: fps_delay = 10; break;
        case 3: fps_delay = 5; break;
    }
    /* main loop */
    fade_screen( FADE_IN, FADE_DEF_TIME );
    reset_timer();
    event_reset();
    while ( !leave && !term_game ) {
        if ( config.fps ) SDL_Delay( fps_delay );
        if ( event_poll( &event ) ) {
            switch ( event.type ) {
		case SDL_QUIT:
			term_game = 1;
		    break;
                case SDL_KEYDOWN:
                    for ( i = 0; i < BOWL_COUNT; i++ )
                        if ( bowls[i] ) 
                            bowl_store_key( bowls[i], event.key.keysym.sym );
                    break;
                case SDL_KEYUP:    
                    switch ( event.key.keysym.sym ) {
                        case SDLK_ESCAPE: 
                            if ( game_over ) {
                                leave = 1;
                                break;
                            }
                            escape = 1;
                            if ( confirm( large_font, _("End Game? y/n"), CONFIRM_YES_NO ) ) 
                                for ( i = 0; i < BOWL_COUNT; i++ )
                                    if ( bowls[i] && !bowls[i]->game_over )
                                        bowl_finish_game( bowls[i] );
                            break;
                         case SDLK_p:
                             request_pause = 1;
                             break;
                         case SDLK_f:
                             /* switch fullscreen */
                            config.fullscreen = !config.fullscreen;
                            set_video_mode( std_video_mode( config.fullscreen ) );
                            FULL_DEST( sdl.screen ); FULL_SOURCE( offscreen ); blit_surf();
                            refresh_screen( 0, 0, 0, 0 );
                        case SDLK_TAB:
                            sprintf( sshot_str, "ss%i.bmp", screenshot_id++ );
                            SDL_SaveBMP( sdl.screen, sshot_str );
                            break;
                        default: break;
                    }
                    break;
                default: break;
            }
        }
        ms = get_time();
        for ( i = 0; i < BOWL_COUNT; i++ )
            if ( bowls[i] )
                bowl_hide( bowls[i] );
        shrapnells_hide();

        if ( request_pause && !game_over ) {
            for ( i = 0; i < BOWL_COUNT; i++ )
                if ( bowls[i] )
                    bowl_toggle_pause( bowls[i] );
            request_pause = 0;
        }
            
        for ( i = 0; i < BOWL_COUNT; i++ )
            if ( bowls[i] )
                bowl_update( bowls[i], ms, game_over );
        
        /* check if any of the bowls entered a new level and change background if so */
        if ( !config.keep_bkgnd ) {
            for ( i = 0; i < BOWL_COUNT; i++ )
                if ( bowls[i] )
                    if ( bowls[i]->level > bkgnd_level ) {
                        bkgnd_level = bowls[i]->level;
                        /* recreate background */
                        tetris_recreate_bkgnd( -1 );
                        refresh_screen( 0, 0, 0, 0 );
                        reset_timer();
                    }
        }
        
        shrapnells_update( ms );
        for ( i = 0; i < BOWL_COUNT; i++ )
            if ( bowls[i] )
                bowl_show( bowls[i] );
        shrapnells_show();
        refresh_rects();
        /* check if game's over */
        if ( !game_over ) {
            /* count number of finished bowls */
            game_over = 0;
            for ( i = 0; i < BOWL_COUNT; i++ )
                if ( bowls[i] && bowls[i]->game_over )
                    game_over++;
            if ( ( game_over == 1 && bowl_count == 1 ) || ( bowl_count > 1 && bowl_count - game_over <= 1 ) )
                game_over = 1;
            else
                game_over = 0;
            if ( game_over && bowl_count > 1 && !escape )
                gain_multiplayer_bonus = 1;
        }
        /* the last bowl in multiplayer gains additional 100,000 score */
        if ( gain_multiplayer_bonus ) {
            gain_multiplayer_bonus = 0;
            for ( i = 0; i < BOWL_COUNT; i++ )
                if ( bowls[i] && !bowls[i]->game_over )
                    counter_add( &bowls[i]->score, 50000 );
        }
    }
    fade_screen( FADE_OUT, FADE_DEF_TIME );
    /* highscore entries */
    chart_clear_new_entries();
    for ( i = 0; i < BOWL_COUNT; i++ )
        if ( bowls[i] ) 
            chart_add( chart_set_query( gametype_ids[config.gametype] ), bowls[i]->name, bowls[i]->level, counter_get( bowls[i]->score ) );
    SDL_ShowCursor( 1 );
}

/*
====================================================================
Run a number of CPU games to get an average score gained so you'll
see if your analyze algorithm in cpu.c cpu_analyze_bowl() sucks
or rocks!
====================================================================
*/
extern int CPU_SCORE_HOLE;
extern int CPU_SCORE_ALT;
extern int CPU_SCORE_LINE;
extern int CPU_SCORE_STEEP;
extern int CPU_SCORE_ABYSS;
extern int CPU_SCORE_BLOCK;
extern int count_occ( int *array, int size, int min, int max )
{
    int i, count;
    count = 0;
    for ( i = 0; i < size; i++ )
        if ( array[i] >= min && array[i] <= max )
            count++;
    return count;
}
void tetris_make_stat()
{
    int i;
    int game_count = 50;
    double total = 0;
    int total_lines = 0;
    double scores[1024];
    int lines[1024];
    Bowl *bowl = 0;
    SDL_Event event;
    int leave = 0;
    FILE *file = 0;
    
    printf( "*****\n" );
    
    bowl = bowl_create( 0, 0, -1, -1, blocks, qmark, "Demo", 0 );
    
    /* reset counters */
    total = 0; total_lines = 0;
    memset( lines, 0, sizeof( lines ) );
    memset( scores, 0, sizeof( scores ) );
        
    printf( "Computing: %3i %3i %3i %3i %3i %3i\n", CPU_SCORE_HOLE, CPU_SCORE_ALT, CPU_SCORE_LINE, CPU_SCORE_STEEP, CPU_SCORE_ABYSS, CPU_SCORE_BLOCK );
        
    for ( i = 0; i < game_count; i++ ) {
        if ( SDL_PollEvent( &event ) && event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE )
            leave = 1;
        bowl_quick_game( bowl, 1 );
        lines[i] = bowl->lines;
        total_lines += lines[i];
        scores[i] = bowl->score.value;
        total += scores[i];
        printf( "%3i: %5i: %14.0f\n", i, lines[i], scores[i] );
        if ( leave ) return;
    }
    
    if ( i != game_count ) game_count = i;
    if ( game_count <= 0 ) return;
    
    /* write to file */
    file = fopen( "stats", "a+" );
    fprintf( file, "SETTING: %3i %3i %3i %3i %3i %3i\n", CPU_SCORE_HOLE, CPU_SCORE_ALT, CPU_SCORE_LINE, CPU_SCORE_STEEP, CPU_SCORE_ABYSS, CPU_SCORE_BLOCK );
    for (i = 0; i < game_count;i++)
        fprintf( file, "%3i: %5i: %14.0f\n", i, lines[i], scores[i] );
    fprintf( file, "-----\n" );
    fprintf( file, "  0-100: %4i  101-200: %4i  201-400: %4i\n", 
             count_occ( lines, game_count, 0, 100 ), count_occ( lines, game_count, 101, 200 ), count_occ( lines, game_count, 201, 400 ) );
    fprintf( file, "401-600: %4i  601-800: %4i  rest:    %4i\n", 
             count_occ( lines, game_count, 401, 600 ), count_occ( lines, game_count, 601, 800 ), count_occ( lines, game_count, 801, 100000 ) );
    fprintf( file, "-----\n" );
    fprintf( file, "Avg.Lines: %i Avg.Score: %i\n", total_lines / game_count, (int)(total / game_count) );
    fprintf( file, "\n" );
    fclose( file );
    
    bowl_delete( bowl );
}
#ifdef _1
void tetris_make_stat()
{
    int i;
    int game_count = 10;
    double total = 0;
    int total_lines = 0;
    int scores[1024];
    int lines[1024];
    Bowl *bowl = 0;
    int ms = 4;
    SDL_Event event;
    int leave = 0;
    char str[256];
    
    font->align = ALIGN_X_LEFT | ALIGN_Y_TOP;
    tetris_recreate_bkgnd( 0 );
    DEST( sdl.screen, 300, 0, 320, 480 ); fill_surf( 0x0 );
    refresh_screen( 0, 0, 0, 0 );
    if ( config.visualize )
        shrapnells_init();
    
    for ( i = 0; i < game_count; i++ ) {
        bowl = bowl_create( 0, 0, -1, -1, blocks, qmark, "Demo", 0 );
        if ( config.visualize )
            bowl->blind = 0;
        else
            bowl->blind = 1;
        bowl->mute = 1;
        while ( !bowl->game_over && !leave ) {
            if ( SDL_PollEvent( &event ) && event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE )
                leave = 1;
            bowl_update( bowl, ms, 0 );
            if ( config.visualize )
                refresh_rects();
        }
        lines[i] = bowl->lines;
        scores[i] = counter_get_target( bowl->score );
        sprintf( str, "%4i\n", lines[i] );
        write_text( font, sdl.screen, 310 + ( i % 7 ) * 42, 10 + ( i / 7 ) * 16, str, OPAQUE );
        refresh_screen( 0, 0, 0, 0 );
        total += scores[i];
        total_lines += bowl->lines;
        bowl_delete( bowl );
        printf( "%3i: %12i\n", i, scores[i] ); /* DEBUG */
        if ( leave ) break;
    }
    
    if ( config.visualize ) 
        shrapnells_delete();
    
    if ( i != game_count )
        game_count = i;
    if ( game_count <= 0 ) return;
    
    sprintf( str, "Avg Score: %i Avg. Lines: %i\n", (int)(total / game_count), total_lines / game_count );
    write_text( font, sdl.screen, 330, 428, str, OPAQUE );
    
    sprintf( str, "0-200: %i 201-400:: %i\n", count_occ( lines, game_count, 0, 200 ), count_occ( lines, game_count, 201, 400 ) );
    write_text( font, sdl.screen, 330, 444, str, OPAQUE );
    sprintf( str, "401-600: %i >601: %i\n", count_occ( lines, game_count, 401, 600 ), count_occ( lines, game_count, 601, 10000 ) );
    write_text( font, sdl.screen, 330, 460, str, OPAQUE );

    refresh_screen( 0, 0, 0, 0 );
        
    wait_for_click();
}
#endif

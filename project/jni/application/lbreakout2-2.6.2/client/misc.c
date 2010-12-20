/***************************************************************************
                          misc.c  -  description
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

#include <time.h>
#include "lbreakout.h"
#include "../game/game.h"
#include "config.h"
#include "event.h"
#include "misc.h"

extern SDL_Surface *stk_display, *nuke_bkgnd, *brick_pic;
extern Game *local_game, *game;
extern Paddle *l_paddle;
extern StkFont *font;
extern SDL_Surface *offscreen;
extern int stk_quit_request;
int shadow_size = 8;
#ifdef AUDIO_ENABLED
extern StkSound *wav_click;
#endif
extern int motion_button;
extern Config config;
extern int bkgnd_count;
extern SDL_Surface **bkgnds;

/*
====================================================================
Load background according to id and draw background to offscreen.
Return Value: loaded background surface
====================================================================
*/
void bkgnd_draw( SDL_Surface *bkgnd, int id )
{
	SDL_Surface *pic = 0;
	int i, j;

	if ( id >= bkgnd_count || id == -1 )
		id = rand() % bkgnd_count;

	/* load background */
	pic = bkgnds[id];
	for ( i = 0; i < bkgnd->w; i += pic->w ) {
		for ( j = 0; j < bkgnd->h; j += pic->h ) {
			stk_surface_blit( pic, 0,0,-1,-1,
					bkgnd, i, j);
		}
	}

	/* draw to offscreen */
	stk_surface_blit( bkgnd, 0,0,-1,-1, offscreen, 0,0 );
}
/*
====================================================================
Confirm request. Darkens screen a bit and display text.
Return Value: True if successful
====================================================================
*/
void draw_confirm_screen( StkFont *font, SDL_Surface *buffer, char *str )
{
    int i, y, x;
    Text *text = create_text( str, 60 );
    stk_surface_fill( stk_display, 0,0,-1,-1, 0x0 );
    stk_surface_alpha_blit( buffer, 0,0,-1,-1, stk_display, 0,0, 128 );
    font->align = STK_FONT_ALIGN_CENTER_X | STK_FONT_ALIGN_TOP;
    y = (stk_display->h - text->count * font->height) / 2;
    x = stk_display->w / 2;
    for ( i = 0; i < text->count; i++ ) {
        stk_font_write(font, stk_display, x, y, STK_OPAQUE, text->lines[i]);
        y += font->height;
    }
    delete_text( text );
}
int confirm( StkFont *font, char *str, int type )
{
    SDL_Event event;
    int go_on = 1;
    int ret = 0;
    SDL_Surface *buffer = 
        stk_surface_create( SDL_SWSURFACE, stk_display->w, stk_display->h );
    SDL_SetColorKey(buffer, 0, 0);

#ifdef AUDIO_ENABLED
    stk_sound_play( wav_click );
#endif

    event_clear_sdl_queue();

    stk_surface_blit( stk_display, 0,0,-1,-1, buffer, 0,0 );
    if ( type == CONFIRM_PAUSE )
        stk_surface_gray( stk_display, 0,0,-1,-1,0 );
    else
        draw_confirm_screen( font, buffer, str );
    stk_display_update( STK_UPDATE_ALL );

    while (go_on && !stk_quit_request) {
        SDL_WaitEvent(&event);
        /* TEST */
        switch ( event.type ) {
            case SDL_QUIT: stk_quit_request = 1; break;
            case SDL_MOUSEBUTTONUP:
                if ( type == CONFIRM_ANY_KEY ) {
                    ret = 1; go_on = 0;
                }
/*                else
                if ( type == CONFIRM_YES_NO ) {
                    if ( event.button.button == LEFT_BUTTON )
                        ret = 1;
                    else
                        ret = 0;
                    go_on = 0;
                }*/
                break;
            case SDL_KEYDOWN:
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
                        stk_display_apply_fullscreen( config.fullscreen );
						draw_confirm_screen( font, buffer, str );
						stk_display_update( STK_UPDATE_ALL );
					}
                }
                else
                {
                    char *keyName = SDL_GetKeyName(event.key.keysym.sym);
                    char *yesLetter = _("y");
                    char *noLetter = _("n");
                    if (strcmp(keyName, yesLetter) == 0) {
                        go_on = 0;
                        ret = 1;
                    }
                    else
                    if (event.key.keysym.sym==SDLK_ESCAPE || strcmp(keyName, noLetter) == 0 ) {
                        go_on = 0;
                        ret = 0;
                    }
                    default:
                        break;
                }
                break;
        }
    }
#ifdef AUDIO_ENABLED
    stk_sound_play( wav_click );
#endif
    stk_surface_blit( buffer, 0,0,-1,-1, stk_display, 0,0 );
    stk_display_update( STK_UPDATE_ALL );
    SDL_FreeSurface(buffer);

    /* reset the relative position so paddle wont jump */
    SDL_GetRelativeMouseState(0,0);

    return ret;
}
#ifdef NETWORK_ENABLED
/*
====================================================================
Display a info message (gray screen a bit and display text), 
send a MSG_READY when player has clicked and wait for a remote answer
(timeout 10 secs). Waiting may be cancelled by pressing ESCAPE which
results in sending a MSG_GAME_EXITED.
Return Value: True if both peers clicked to continue, False if the
connection was cancelled for some reason.
====================================================================
*/
int display_info( StkFont *font, char *str, NetSocket *peer )
{
#if 0
    char error[128];
    Net_Msg msg;
    SDL_Event event;
    int ret = 0, leave = 0;
    SDL_Surface *buffer = 
        stk_surface_create( SDL_SWSURFACE, stk_display->w, stk_display->h );
    SDL_SetColorKey(buffer, 0, 0);

#ifdef AUDIO_ENABLED
    stk_sound_play( wav_click );
#endif

    event_clear_sdl_queue();

    stk_surface_blit( stk_display, 0,0,-1,-1, buffer, 0,0 );
	draw_confirm_screen( font, buffer, str );
    stk_display_update( STK_UPDATE_ALL );
    stk_wait_for_input();
    net_write_empty_msg( peer, MSG_READY ); 
	draw_confirm_screen( font, buffer, 
                         _("Waiting for remote answer...") );
    stk_display_update( STK_UPDATE_ALL );
    event_clear_sdl_queue();
    while ( !leave ) {
        if ( SDL_PollEvent( &event ) )
            if ( (event.type == SDL_KEYDOWN && 
                 event.key.keysym.sym == SDLK_ESCAPE) ||
                 event.type == SDL_QUIT ) {
                    net_write_empty_msg( peer, MSG_GAME_EXITED );
                    leave = 1;
                    break;
                }
        if ( net_read_msg( peer, &msg, 0 ) )
            switch ( msg.type ) {
                case MSG_READY:
                    ret = 1; leave = 1;
                    break;
                case MSG_GAME_EXITED:
                    ret = 0; leave = 1;
                    sprintf( error, /* xgettext:no-c-format */ _("remote player cancelled the game\n") );
                    confirm( font, error, CONFIRM_ANY_KEY );
                    break;
            }
        SDL_Delay( 10 );
    }
    
#ifdef AUDIO_ENABLED
    stk_sound_play( wav_click );
#endif
    
    stk_surface_blit( buffer, 0,0,-1,-1, stk_display, 0,0 );
    stk_display_update( STK_UPDATE_ALL );
    SDL_FreeSurface(buffer);
    
    /* reset the relative position so paddle wont jump */
    SDL_GetRelativeMouseState(0,0);

    return ret;
#endif
    return 1;
}
#endif
/*
====================================================================
Create shadow surface for specified region in surface.
Return Value: Shadow surface
====================================================================
*/
SDL_Surface* create_shadow( SDL_Surface *surf, int x, int y, int w, int h )
{
    SDL_Surface *shadow = 0;
    int i, j;
    Uint32 white = SDL_MapRGB( stk_display->format, 0xff, 0xff, 0xff );
    Uint32 black = SDL_MapRGB( stk_display->format, 0, 0, 0 );
    shadow = stk_surface_create( SDL_SWSURFACE, w, h );
    SDL_SetColorKey( shadow, SDL_SRCCOLORKEY, white );
    for ( i = 0; i < w; i++ )
        for ( j = 0; j < h; j++ ) {
            if ( surf->flags & SDL_SRCCOLORKEY &&
                 stk_surface_get_pixel( surf, i, j ) == surf->format->colorkey )
                stk_surface_set_pixel( shadow, i, j, white );
            else
                stk_surface_set_pixel( shadow, i, j, black );
        }
    return shadow;
}


/*
====================================================================
Enter a string and return True if ENTER received and False
if ESCAPE received.
====================================================================
*/
int enter_string( StkFont *font, char *caption, char *edit, int limit )
{
    SDL_Event event;
    int go_on = 1;
    int ret = 0;
    SDL_Surface *buffer = 
        stk_surface_create( SDL_SWSURFACE, stk_display->w, stk_display->h );
    int length = strlen( edit );

    SDL_SetColorKey(buffer, 0, 0);

    stk_surface_blit( stk_display, 0,0,-1,-1, buffer, 0,0 );
    font->align = STK_FONT_ALIGN_CENTER_X | STK_FONT_ALIGN_CENTER_Y;

    while ( go_on && !stk_quit_request ) {

        stk_surface_fill( stk_display, 0,0,-1,-1, 0x0 );
        stk_surface_alpha_blit( buffer, 0,0,-1,-1, stk_display, 0,0, 128 );
        stk_font_write(font, stk_display, stk_display->w / 2, stk_display->h / 2, STK_OPAQUE, caption);
        write_text_with_cursor(font, stk_display, stk_display->w / 2, stk_display->h / 2 + font->height, edit, STK_OPAQUE);
        stk_display_update( STK_UPDATE_ALL );
        event.type = SDL_NOEVENT;
        SDL_PollEvent(&event);
        /* TEST */
        switch ( event.type ) {
            case SDL_QUIT: stk_quit_request = 1; break;
            case SDL_KEYDOWN:
                switch ( event.key.keysym.sym ) {
                    case SDLK_ESCAPE:
                        ret = 0;
                        go_on = 0;
                        break;
                    case SDLK_RETURN:
                        ret = 1;
                        go_on = 0;
                        break;
                    case SDLK_BACKSPACE:
                        if ( length > 0 ) edit[--length] = 0;
                        break;
                    default:
                        if ( event.key.keysym.sym >= 32 && event.key.keysym.sym < 128 && length < limit ) {
                            edit[length++] = event.key.keysym.unicode;
                            edit[length] = 0;
                        }
                        break;
                }
                break;
        }
    }
    stk_surface_blit( buffer, 0,0,-1,-1, stk_display, 0,0 );
    stk_display_update( STK_UPDATE_ALL );
    SDL_FreeSurface(buffer);

    /* reset the relative position so paddle wont jump */
    SDL_GetRelativeMouseState(0,0);

    return ret;
}

/*
====================================================================
Display text blinking.
====================================================================
*/
void  write_text_with_cursor( StkFont *fnt, SDL_Surface *dest,
    int x, int y, char *str, int alpha)
{
    static int cursor_on = 0;
    static Uint32 last_tick = 0;
    // create temporary space for cursor and text
    char *text_with_cursor = calloc(strlen(str) + 2, sizeof(char));
    if (text_with_cursor) {
       strcpy(text_with_cursor, str);

       // Time to blink cursor on/off?
       if (SDL_GetTicks() - last_tick > 500) {
            last_tick = SDL_GetTicks();
	    cursor_on = ! cursor_on;
       }
       // Tack on cursor to end of text
       if (cursor_on) {
	    strcat(text_with_cursor, "_");
       }
       else {
	    strcat(text_with_cursor, " ");
       }
    	    
       stk_font_write(fnt,dest,x,y,alpha,text_with_cursor);
    
       free(text_with_cursor);
    }
}

/*
====================================================================
Enter nuke mode and allow player to disintegrate single bricks
by spending 5% of his/her score.
====================================================================
*/
void game_nuke( void )
{
	char buf[128];
	SDL_Event event;
	int x,y,i,j,leave = 0;
	SDL_Surface *buffer = 
		stk_surface_create( SDL_SWSURFACE, stk_display->w, stk_display->h );
	SDL_Surface *red_mask = 
		stk_surface_create( SDL_SWSURFACE, BRICK_WIDTH, BRICK_HEIGHT );
	stk_surface_fill( red_mask, 0,0,-1,-1, 0xFF0000 );
	SDL_SetAlpha( red_mask, SDL_SRCALPHA, 128 );
	SDL_SetColorKey(buffer, 0, 0);

#ifdef AUDIO_ENABLED
	stk_sound_play( wav_click );
#endif
	SDL_SetEventFilter(0);
	event_clear_sdl_queue();
	/* backup screen contents */
	stk_surface_blit( stk_display, 0,0,-1,-1, buffer, 0,0 );
	/* display bricks darkened */
	stk_surface_blit( nuke_bkgnd, 0,0,-1,-1, 
			stk_display, 0,0 );
	for ( i = 1; i < MAP_WIDTH - 1; i++ )
		for ( j = 1; j < MAP_HEIGHT - 1; j++ )
			if ( game->bricks[i][j].id >= 0 )
				stk_surface_alpha_blit( brick_pic,
					game->bricks[i][j].id * BRICK_WIDTH, 0,
					BRICK_WIDTH, BRICK_HEIGHT,
					stk_display,
					i*BRICK_WIDTH, j*BRICK_HEIGHT, 128 );
	/* info */
	font->align = STK_FONT_ALIGN_LEFT;
	sprintf( buf, _("Plane Of Inner Stability entered (Score: %i)"),
			l_paddle->player->stats.total_score + l_paddle->score );
	stk_font_write( font, stk_display,
			BRICK_WIDTH, (MAP_HEIGHT-1)*BRICK_HEIGHT, 
			128, buf );
	/* show score of player */
	stk_display_update( STK_UPDATE_ALL );

	x = y = -1;
	while (!leave && !stk_quit_request) {
		SDL_WaitEvent(&event);
		switch ( event.type ) {
			case SDL_QUIT: 
				stk_quit_request = 1; 
				break;
			case SDL_MOUSEBUTTONDOWN:
				if ( x != -1 )
				if ( confirm( font, 
					/* xgettext:no-c-format */ _("Disintegrate Brick? (Costs 5% of your score.) y/n"), 
					CONFIRM_YES_NO ) ) {
					/* implant a bomb to this brick and return */
					game_set_current( local_game );
					brick_start_expl( x,y, BRICK_EXP_TIME,
							  local_game->paddles[0] );
					local_game->bricks[x][y].score = 0;
					game_set_current( game );
					l_paddle->player->stats.total_score -= (int)(0.05 * 
					       (l_paddle->score + 
					        l_paddle->player->stats.total_score));
					leave = 1;
				}
				break;
			case SDL_MOUSEMOTION:
				if ( x != -1 ) {
					/* clear old selection */
					stk_surface_blit( nuke_bkgnd,
							x*BRICK_WIDTH, y*BRICK_HEIGHT,
							BRICK_WIDTH, BRICK_HEIGHT,
							stk_display, 
							x*BRICK_WIDTH, y*BRICK_HEIGHT );
					stk_surface_alpha_blit( brick_pic,
							game->bricks[x][y].id * BRICK_WIDTH, 0,
							BRICK_WIDTH, BRICK_HEIGHT,
							stk_display,
							x*BRICK_WIDTH, y*BRICK_HEIGHT, 128 );
					stk_display_store_drect();
					x = y = -1;
				}
				/* make new selection if brick */
				i = event.motion.x / BRICK_WIDTH;
				j = event.motion.y / BRICK_HEIGHT;
				if ( i >= 1 && i <= MAP_WIDTH -2 )
				if ( j >= 1 && j <= MAP_HEIGHT - 2 )
				if ( game->bricks[i][j].id >= 0 ) {
					x = i; y = j;
					stk_surface_blit( red_mask, 0,0,-1,-1,
					stk_display,x*BRICK_WIDTH, y*BRICK_HEIGHT );
					stk_display_store_drect();
				}
				break;
			case SDL_KEYDOWN:
				if ( event.key.keysym.sym == SDLK_ESCAPE )
					leave = 1;
				break;
		}
		stk_display_update( STK_UPDATE_RECTS );
	}

	stk_surface_blit( buffer, 0,0,-1,-1, stk_display, 0,0 );
	stk_display_update( STK_UPDATE_ALL );
	SDL_FreeSurface(red_mask);
	SDL_FreeSurface(buffer);
	SDL_SetEventFilter(event_filter);
}

/* gray screen and display a formatted text, directly update the
 * screen */
void display_text( StkFont *font, char *format, ... )
{
	int i, y, x;
	Text *text;
	char buf[512];
	va_list args;

	va_start( args, format );
	vsnprintf( buf, 512, format, args );
	va_end( args );

    stk_surface_gray( stk_display, 0,0,-1,-1, 2 );
	text = create_text( buf, 60 );
	font->align = STK_FONT_ALIGN_CENTER_X | STK_FONT_ALIGN_TOP;
	y = (stk_display->h - text->count * font->height) / 2;
	x = stk_display->w / 2;
	for ( i = 0; i < text->count; i++ ) {
		stk_font_write(font, stk_display, x, y, STK_OPAQUE, text->lines[i]);
		y += font->height;
	}
	delete_text( text );

	stk_display_update( STK_UPDATE_ALL );
}

/***************************************************************************
                          hint.c  -  description
                             -------------------
    begin                : Sun Jan 6 2002
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
#include "hint.h"

SDL_Surface *hint_balloon = 0;
SDL_Surface *hint_bkgnd = 0;
SDL_Surface *balloon = 0;
SDL_Surface *balloon_peek = 0;
int balloon_peek_add_x = 9, balloon_peek_add_y = 9; /* added to actual balloon to display peek */
int balloon_tile_size;
Hint *cur_hint = 0;
Font *hint_font = 0;
int hint_delay = 0;
float hint_alpha;
int hint_text_x = 10 + 9, hint_text_y = 10; /* text is drawn at this offset within balloon */

extern Config config;
extern Sdl sdl;

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Create actual hint picture displayed.
====================================================================
*/
void hint_create_pic()
{
    int x, y;
    free_surf( &hint_balloon );
    hint_balloon = create_surf( cur_hint->sw, cur_hint->sh, SDL_SWSURFACE );
    for ( y = 0; y < cur_hint->sh - balloon_peek_add_y; y += balloon_tile_size )
        for ( x = 0; x < cur_hint->sw - balloon_peek_add_x; x += balloon_tile_size ) {
            DEST( hint_balloon, x + balloon_peek_add_x, y, balloon_tile_size, balloon_tile_size );
            /* get proper tile */
            if ( y == 0 ) {
                if ( x == 0 )
                    SOURCE( balloon, 0, 0 )
                else
                    if ( x >= cur_hint->sw - balloon_peek_add_x - balloon_tile_size )
                        SOURCE( balloon, balloon_tile_size * 2, 0 )
                    else
                        SOURCE( balloon, balloon_tile_size, 0 )
            }
            else
                if ( y >= cur_hint->sh - balloon_peek_add_y - balloon_tile_size ) {
                    if ( x == 0 )
                        SOURCE( balloon, 0, balloon_tile_size * 2 )
                    else
                        if ( x >= cur_hint->sw - balloon_peek_add_x - balloon_tile_size )
                            SOURCE( balloon, balloon_tile_size * 2, balloon_tile_size * 2 )
                        else
                            SOURCE( balloon, balloon_tile_size, balloon_tile_size * 2 )
                }
                else {
                    if ( x == 0 )
                        SOURCE( balloon, 0, balloon_tile_size )
                    else
                        if ( x >= cur_hint->sw - balloon_peek_add_x - balloon_tile_size )
                            SOURCE( balloon, balloon_tile_size * 2, balloon_tile_size )
                        else
                            SOURCE( balloon, balloon_tile_size, balloon_tile_size )
                }
            blit_surf();
        }
    /* peek */
    DEST( hint_balloon, 0, cur_hint->sh - balloon_peek->h, balloon_peek->w, balloon_peek->h );
    SOURCE( balloon_peek, 0, 0 );
    blit_surf();
    /* text */
    for ( y = 0, x = 0; x < cur_hint->text->count; y += hint_font->height, x++ )
        write_text( hint_font, hint_balloon, hint_text_x, hint_text_y + y, cur_hint->text->lines[x], OPAQUE );
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Load/delete hint resources.
====================================================================
*/
void hint_load_res()
{
    balloon = load_surf( "balloon.bmp", SDL_SWSURFACE );
    balloon_peek = load_surf( "balloon_peek.bmp", SDL_SWSURFACE );
    balloon_tile_size = balloon->w / 3;
    hint_font = load_fixed_font( "f_tiny_black.bmp", 32, 96, 6 );
}
void hint_delete_res()
{
    free_surf( &balloon );
    free_surf( &balloon_peek );
    free_font( &hint_font );
    free_surf( &hint_balloon );
}

/*
====================================================================
Create a hint. The passed x,y position is where the end of the
Sprechblasenspitze is. Contents is converted to lines (duplicated).
====================================================================
*/
Hint* hint_create( int x, int y, char *contents )
{
    Hint *hint = calloc( 1, sizeof( Hint ) );
    /* get text */
    hint->text = create_text( contents, 36 );
    /* get screen size and convert string */
    hint->sw = 240 + balloon_peek_add_x;
    hint->sh = balloon_tile_size * ( 2 + ( hint->text->count + 0 ) * hint_font->height / balloon_tile_size ) + balloon_peek_add_y;
    hint_set_pos( hint, x, y );
    return hint;
}
void hint_delete( Hint *hint )
{
    if ( !hint ) 
        return;
    free( hint );
}

/*
====================================================================
Set the background on which the hints will be displayed.
====================================================================
*/
void hint_set_bkgnd( SDL_Surface *bkgnd )
{
    hint_bkgnd = bkgnd;
}

/*
====================================================================
Set this hint as actual one. If NULL was passed no hint will be
handled.
====================================================================
*/
void hint_set( Hint *hint )
{
    if ( config.quick_help )
        cur_hint = hint;
    else
        cur_hint = 0;
    if ( cur_hint ) {
        hint_alpha = 255;
        hint_delay = 500;
        hint_create_pic();
    }
}

/*
====================================================================
Set hint to this position.
====================================================================
*/
void hint_set_pos( Hint *hint, int x, int y )
{
    /* adjust screen position so that balloon's peek is at x,y */
    hint->sx = x;
    hint->sy = y - hint->sh - balloon_peek->h / 2;
}

/*
====================================================================
Show/hide/update _current_ hint.
====================================================================
*/
void hint_hide()
{
    if ( !cur_hint || hint_delay ) 
        return;
    DEST( sdl.screen, cur_hint->sx, cur_hint->sy, cur_hint->sw, cur_hint->sh );
    SOURCE( hint_bkgnd, cur_hint->sx, cur_hint->sy );
    blit_surf();
    add_refresh_rect( cur_hint->sx, cur_hint->sy, cur_hint->sw, cur_hint->sh );
    /* peek */
    DEST( sdl.screen, cur_hint->sx + balloon_peek_add_x, cur_hint->sy + cur_hint->sh  + balloon_peek_add_y, balloon_peek->w, balloon_peek->h );
    SOURCE( hint_bkgnd, cur_hint->sx + balloon_peek_add_x, cur_hint->sy + cur_hint->sh  + balloon_peek_add_y );
    blit_surf();
    add_refresh_rect( cur_hint->sx + balloon_peek_add_x, cur_hint->sy + cur_hint->sh  + balloon_peek_add_y, balloon_peek->w, balloon_peek->h );
}
void hint_show()
{
    if ( !cur_hint || hint_delay ) 
        return;
    DEST( sdl.screen, cur_hint->sx, cur_hint->sy, cur_hint->sw, cur_hint->sh );
    SOURCE( hint_balloon, 0, 0 );
    alpha_blit_surf( hint_alpha );
    add_refresh_rect( cur_hint->sx, cur_hint->sy, cur_hint->sw, cur_hint->sh );
}
void hint_update( int ms )
{
    /* delay */
    if ( hint_delay > 0 ) {
        hint_delay -= ms;
        if ( hint_delay <= 0 )
            hint_delay = 0;
    }
    /* if delay fade blend if if still alpha */
    if ( hint_delay == 0 && hint_alpha > 0 ) {
        hint_alpha -= 1.0 * ms;
        if ( hint_alpha < 0 )
            hint_alpha = 0;
    }
}

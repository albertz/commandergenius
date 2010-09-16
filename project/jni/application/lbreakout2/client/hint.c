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
 
#include "lbreakout.h" 
#include "hint.h"
#include "config.h"

SDL_Surface *hint_balloon = 0;
SDL_Surface *hint_bkgnd = 0;
SDL_Surface *balloon = 0;
SDL_Surface *balloon_peek = 0;
int balloon_peek_add_x = 9, balloon_peek_add_y = 9; /* added to actual balloon to display peek */
int balloon_tile_size;
Hint *cur_hint = 0;
StkFont *hint_font = 0;
int hint_delay = 0;
float hint_alpha;
int hint_text_x = 10 + 9, hint_text_y = 10; /* text is drawn at this offset within balloon */

extern Config config;
extern SDL_Surface *stk_display;

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
void hint_create_pic( Hint *hint )
{
    int x, y, px, py;
    stk_surface_free( &hint_balloon );
    /* get screen size and position */
    hint->rect.w = 240 + balloon_peek_add_x;
    hint->rect.h = balloon_tile_size * ( 2 + ( hint->text->count + 0 ) * hint_font->height / balloon_tile_size ) + balloon_peek_add_y;
    hint_set_pos( hint, hint->peek_x, hint->peek_y );
    /* build surface */
    hint_balloon = stk_surface_create( SDL_SWSURFACE, cur_hint->rect.w, cur_hint->rect.h );
    for ( y = 0; y < cur_hint->rect.h - balloon_peek_add_y; y += balloon_tile_size )
        for ( x = 0; x < cur_hint->rect.w - balloon_peek_add_x; x += balloon_tile_size ) {
            px = x + balloon_peek_add_x; py = y;
            /* get proper tile */
            if ( y == 0 ) {
                if ( x == 0 )
                    stk_surface_blit( balloon, 0, 0, 
                        balloon_tile_size, balloon_tile_size,
                        hint_balloon, px, py );
                else
                    if ( x >= cur_hint->rect.w - balloon_peek_add_x - balloon_tile_size )
                        stk_surface_blit( balloon, balloon_tile_size * 2, 0, 
                            balloon_tile_size, balloon_tile_size,
                            hint_balloon, px, py );
                    else
                        stk_surface_blit( balloon, balloon_tile_size, 0, 
                            balloon_tile_size, balloon_tile_size,
                            hint_balloon, px, py );
            }
            else
                if ( y >= cur_hint->rect.h - balloon_peek_add_y - balloon_tile_size ) {
                    if ( x == 0 )
                        stk_surface_blit( balloon, 0, balloon_tile_size * 2, 
                            balloon_tile_size, balloon_tile_size,
                            hint_balloon, px, py );
                    else
                        if ( x >= cur_hint->rect.w - balloon_peek_add_x - balloon_tile_size )
                            stk_surface_blit( balloon, 
                                balloon_tile_size * 2, balloon_tile_size * 2, 
                                balloon_tile_size, balloon_tile_size,
                                hint_balloon, px, py );
                        else
                            stk_surface_blit( balloon, 
                                balloon_tile_size, balloon_tile_size * 2, 
                                balloon_tile_size, balloon_tile_size,
                                hint_balloon, px, py );
                }
                else {
                    if ( x == 0 )
                        stk_surface_blit( balloon, 0, balloon_tile_size, 
                            balloon_tile_size, balloon_tile_size,
                            hint_balloon, px, py );
                    else
                        if ( x >= cur_hint->rect.w - balloon_peek_add_x - balloon_tile_size )
                            stk_surface_blit( balloon, 
                                balloon_tile_size * 2, balloon_tile_size, 
                                balloon_tile_size, balloon_tile_size,
                                hint_balloon, px, py );
                        else
                            stk_surface_blit( balloon, 
                                balloon_tile_size, balloon_tile_size, 
                                balloon_tile_size, balloon_tile_size,
                                hint_balloon, px, py );
                }
        }
    /* peek */
    stk_surface_blit( balloon_peek, 0, 0, balloon_peek->w, balloon_peek->h,
            hint_balloon, 0, cur_hint->rect.h - balloon_peek->h );
    /* text */
    for ( y = 0, x = 0; x < cur_hint->text->count; y += hint_font->height, x++ )
        stk_font_write( hint_font, 
            hint_balloon, hint_text_x, hint_text_y + y, 
            -1, cur_hint->text->lines[x] );
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
    balloon = stk_surface_load( SDL_SWSURFACE, "balloon.png" );
    balloon_peek = stk_surface_load( SDL_SWSURFACE, "balloon_peek.png" );
    balloon_tile_size = balloon->w / 3;
    hint_font = stk_font_load( SDL_SWSURFACE, "f_tiny_black.png" );
}
void hint_delete_res()
{
    stk_surface_free( &balloon );
    stk_surface_free( &balloon_peek );
    stk_font_free( &hint_font );
    stk_surface_free( &hint_balloon );
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
    /* get text and position */
    hint->text = create_text( contents, 36 );
    hint->peek_x = x;
    hint->peek_y = y;
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
    if ( config.use_hints )
        cur_hint = hint;
    else
        cur_hint = 0;
    if ( cur_hint ) {
        hint_alpha = 0;
        hint_delay = 500;
        hint_create_pic( hint );
        /* must be in screen */
        if ( hint->rect.x + hint_balloon->w >= stk_display->w )
            hint->rect.x = stk_display->w - hint_balloon->w;
		if ( hint->rect.y < 0 ) hint->rect.y = 0;
    }
}

/*
====================================================================
Set hint to this position.
====================================================================
*/
void hint_set_pos( Hint *hint, int x, int y )
{
    hint->peek_x = x;
    hint->peek_y = y;
    /* adjust screen position so that balloon's peek is at x,y */
    hint->rect.x = x;
    hint->rect.y = y - hint->rect.h - balloon_peek->h / 2;
}

/*
====================================================================
Update the contents of a hint.
====================================================================
*/
void hint_set_contents( Hint *hint, char *contents )
{
    delete_text( hint->text );
    hint->text = create_text( contents, 36 );
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
    stk_surface_blit( hint_bkgnd, cur_hint->rect.x, cur_hint->rect.y,
        cur_hint->rect.w, cur_hint->rect.h,
        stk_display, cur_hint->rect.x, cur_hint->rect.y);
    stk_display_store_drect();
    /* peek */
    stk_surface_blit( 
        hint_bkgnd, 
        cur_hint->rect.x + balloon_peek_add_x, 
        cur_hint->rect.y + cur_hint->rect.h  + balloon_peek_add_y,
        balloon_peek->w, balloon_peek->h,
        stk_display, 
        cur_hint->rect.x + balloon_peek_add_x, 
        cur_hint->rect.y + cur_hint->rect.h  + balloon_peek_add_y );
    stk_display_store_drect();
}
void hint_show()
{
    if ( !cur_hint || hint_delay > 0 ) 
        return;
    SDL_SetAlpha( hint_balloon, SDL_SRCALPHA, hint_alpha );
    stk_surface_blit( hint_balloon, 0, 0,
        cur_hint->rect.w, cur_hint->rect.h,
        stk_display, cur_hint->rect.x, cur_hint->rect.y );
    stk_display_store_drect();
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
    if ( hint_delay == 0 && hint_alpha < 255 ) {
        hint_alpha += 1.0 * ms;
        if ( hint_alpha > 255 )
            hint_alpha = 255;
    }
}

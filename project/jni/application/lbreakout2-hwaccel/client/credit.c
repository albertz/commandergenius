/***************************************************************************
                          credit.c  -  description
                             -------------------
    begin                : Thu Sep 13 2001
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
#include "credit.h"

extern SDL_Surface *stk_display;
extern StkFont *font;
extern SDL_Surface *offscreen;

SDL_Surface *credit_bkgnd = 0;
float       credit_alpha;
float       credit_pms = 0.5;
int         credit_cur;
int         credit_time = 2000;
int         credit_x, credit_y, credit_w, credit_h;
char        credit_str[64];
int         credit_status;

/*
====================================================================
Initiate credit.
====================================================================
*/
void credit_init( char *name, char *author, int level, int level_count )
{
    credit_cur = 0; credit_alpha = 0; credit_status = 0;

    /* string */
    if ( level_count > 0 )
        sprintf( credit_str, "%s (%i/%i) - %s", name, 
                 level + 1, level_count, author );
    else
        sprintf( credit_str, "%s (%i) - %s", name, level + 1, author );

    /* position */
    credit_w = stk_font_string_width( font, credit_str ) + 4;
    credit_h = font->height + 4;
    credit_x = (stk_display->w - credit_w) / 2;
    credit_y = (stk_display->h - credit_h) / 2;
    
    /* background */
    if ( credit_bkgnd ) SDL_FreeSurface( credit_bkgnd );
    credit_bkgnd = stk_surface_create( SDL_SWSURFACE, credit_w, credit_h );
    SDL_SetColorKey( credit_bkgnd, 0, 0 );
    stk_surface_fill( credit_bkgnd, 0,0,-1,-1, 0x0 );
}
void credit_clear()
{
    stk_surface_free( &credit_bkgnd );
}
/*
====================================================================
Show/Hide credits
====================================================================
*/
void credit_hide()
{
    if (credit_status == 3) return;
    stk_surface_blit( offscreen, 
        credit_x, credit_y, credit_w, credit_h,
        stk_display, credit_x, credit_y );
    stk_display_store_drect();
}
void credit_show( )
{
    if (credit_status == 3) return;
        if ( credit_bkgnd )
        stk_surface_alpha_blit( credit_bkgnd, 0, 0,
            credit_w, credit_h, stk_display, 
            credit_x, credit_y, credit_alpha / 2 );
    font->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
    stk_font_write(font, stk_display, credit_x + 2, credit_y + 2, 
        (int)credit_alpha, credit_str );
    font->align = STK_FONT_ALIGN_CENTER_X | STK_FONT_ALIGN_CENTER_Y;
}
void credit_alphashow( int alpha )
{
    if (credit_status == 3) return;
    font->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
    stk_font_write(font, stk_display, credit_x + 2, credit_y + 2,
         (int)alpha, credit_str );
    font->align = STK_FONT_ALIGN_CENTER_X | STK_FONT_ALIGN_CENTER_Y;
}
void credit_update( int ms )
{
    /* appearing */
    if (credit_status == 0) {
        credit_alpha += credit_pms * ms;
        if (credit_alpha >= 255 ) {
            credit_status = 1;
            credit_alpha = 255;
        }
    }
    else
        /* visible */
        if (credit_status == 1) {
            credit_cur += ms;
            if (credit_cur > credit_time)
                credit_status = 2;
        }
        else
            /* disappearing */
            if (credit_status == 2) {
                credit_alpha -= credit_pms * ms;
                if (credit_alpha <= 0)
                    credit_status = 3;
            }
    /* credit_status == 3 means credit disappeared */
}

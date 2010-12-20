/***************************************************************************
                          display.c  -  description
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

#include "lbreakout.h"
#include "display.h"

extern SDL_Surface *offscreen;
extern SDL_Surface *stk_display;
extern StkFont *display_font;
extern StkFont *display_highlight_font;

List *displays = 0;

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Delete a display
====================================================================
*/
void display_delete( void *ptr )
{
    Display *display = ptr;
    if ( display ) {
        stk_surface_free( &display->surf );
        free( display );
    }
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Add a display. The display is cleared by displays_clear() so
the returned display is just a pointer.
The contents of the display is 'text'+'value'. Both may be changed
with the display_set_text/value() functions.
'digits' is the maxium size of 'value'
====================================================================
*/
Display* displays_add( int x, int y, int w, int h, char *text, int value, int digits )
{
    Display *display = 0;
    if ( displays == 0 )
        displays = list_create( LIST_AUTO_DELETE, display_delete );
    display = calloc( 1, sizeof( Display ) );
    if ( display == 0 ) return 0;
    if ( ( display->surf = stk_surface_create( SDL_SWSURFACE,w,h ) ) == 0 ) {
        free( display );
        return 0;
    }
    SDL_SetColorKey( display->surf, 0, 0 );
    display->use_alpha = 1;
    display->x = x;
    display->y = y;
    display->w = w;
    display->h = h;
    display->digits = digits;
    strcpy( display->text, text );
    display->value = value;
    display->cur_value = value;
    list_add( displays, display );
    return display;
}
/*
====================================================================
Remove all displays
====================================================================
*/
void displays_clear()
{
    if ( displays ) {
        list_delete( displays );
        displays = 0;
    }
}

/*
====================================================================
Draw displays
====================================================================
*/
void displays_hide()
{
    Display *display;
    if ( displays == 0 ) return;
    list_reset( displays );
    while ( ( display = list_next( displays ) ) ) {
        stk_surface_blit(offscreen, display->x, display->y,
            display->w, display->h,
            stk_display, display->x, display->y );
        stk_display_store_drect();
    }
}
void displays_show()
{
    int i;
    char str[64], numstr[10];
    StkFont *font;
    Display *display;
    if ( displays == 0 ) return;
    list_reset( displays );
    display_font->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
    display_highlight_font->align = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_TOP;
    while ( ( display = list_next( displays ) ) ) {
        if (display->is_highlighted)
	  font = display_highlight_font;
        else
          font = display_font;
        strcpy( str, display->text );
        if ( display->digits > 0 ) {
            sprintf( numstr, "%i", (int)display->cur_value );
            for ( i = 0; i < display->digits - strlen( numstr ); i++ )
                strcat( str, " " );
            strcat( str, numstr );
        }
        if ( display->use_alpha ) {
            stk_surface_alpha_blit( display->surf, 0,0,
                display->w, display->h,
                stk_display, display->x, display->y, 128 );
        }
        stk_font_write( font, stk_display, 
            display->x + 2, display->y + 1, -1, str );
    }
}

/*
====================================================================
Update displays
====================================================================
*/
void displays_update( int ms )
{
    float change;
    Display *display;
    if ( displays == 0 ) return;
    list_reset( displays );
    while ( ( display = list_next( displays ) ) ) {
        if ( (int)display->cur_value == display->value ) continue;
        /* change relative as for big display->cur_values we must count faster */
        change = ( display->value - display->cur_value ) / 200;
        if ( change > 0 && change < 0.6 ) change = 0.6;
        if ( change < 0 && change > -0.6 ) change = -0.6;
        display->cur_value += change * ms;
        if ( change > 0 && display->cur_value > display->value ) 
            display->cur_value = display->value;
        if ( change < 0 && display->cur_value < display->value ) 
            display->cur_value = display->value;
    }
}
/*
====================================================================
Modify display
====================================================================
*/
void display_set_text( Display *display, char *text )
{
    strcpy( display->text, text );
}
void display_set_value( Display *display, int value )
{
    display->value = value;
}
void display_set_value_directly( Display *display, int value )
{
    display->cur_value = value; /* no smooth approaching of the dest value */  
    display->value = value;
}
void display_set_highlight( Display *display, int on )
{
  display->is_highlighted = on;
}

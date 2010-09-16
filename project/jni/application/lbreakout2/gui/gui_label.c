/***************************************************************************
                          gui_label.c  -  description
                             -------------------
    begin                : Wed Oct 16 2002
    copyright            : (C) 2002 by Michael Speck
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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "gui_widget.h"
#include "gui_label.h"

extern GuiTheme *gui_theme;
extern SDL_Surface *stk_display;

/*
====================================================================
LOCALS
====================================================================
*/

/*
====================================================================
Convert a text string into single lines of maximum length 
'char_width'.
====================================================================
*/
typedef struct {
    char **lines;
    int count;
} LabelText;
static void repl_new_lines( char *text )
{
    int i;
    for ( i = 0; i < strlen( text ); i++ )
        if ( text[i] < 32 )
            text[i] = 32;
}
static LabelText* text_create( char *orig_str, int char_width )
{
    int i, j;
    char line[256]; /* a line should not exceed this length */
    int pos;
    int last_space;
    int new_line;
    LabelText *text = 0;
    char *str = 0;

    text = calloc ( 1, sizeof( LabelText ) );
    /* maybe orig_str is a constant expression; duplicate for safety */
    str = strdup( orig_str );
    /* replace original new_lines with spaces */
    repl_new_lines( str );
    /* change some spaces to new_lines, so that the new text fits the wanted line_length */
    /* NOTE: '#' means new_line ! */
    // if character with is 0 it's just a single line //
    if ( char_width > 0 ) {
        pos = 0;
        while ( pos < strlen( str ) ) {
            last_space = 0;
            new_line = 0;
            i = 0;
            while ( !new_line && i < char_width && i + pos < strlen( str ) ) {
                switch ( str[pos + i] ) {
                    case '#': new_line = 1;
                    case 32: last_space = i; break;
                }
                i++;
            }
            if ( i + pos >= strlen( str ) ) break;
            if ( last_space == 0 ) {
                /* uhh... much to long, we'll have to cut a word into pieces ... */
                last_space = char_width / 2;
            }
            str[pos + last_space] = 10;
            pos += last_space;
        }
    }
    /* count lines */
    if ( char_width > 0 ) {
        for ( i = 0; i < strlen( str ); i++ )
            if ( str[i] == 10 )
                text->count++;
        /* maybe one unfinished line */
        if ( str[strlen( str ) - 1] != 10 )
            text->count++;
    }
    else
        text->count = 1;
    /* get mem */
    text->lines = calloc( text->count, sizeof( char* ) );
    /* get all lines */
    pos = 0;
    for ( j = 0; j < text->count; j++ ) {
        i = 0;
        while ( pos + i < strlen( str ) && str[pos + i] != 10 ) {
            line[i] = str[i + pos];
            i++;
        }
        pos += i; pos++;
        line[i] = 0;
        text->lines[j] = strdup( line );
    }
    if ( text->count == 0 )
        fprintf( stderr, "conv_to_text: warning: line_count is 0\n" );
    free( str );
    return text;
}
static void text_delete( LabelText *text )
{
    int i;
    if ( text == 0 ) return;
    if ( text->lines ) {
        for ( i = 0; i < text->count; i++ )
            if ( text->lines[i] )
                free( text->lines[i] );
        free( text->lines );
    }
    free( text );
}

/*
====================================================================
Default event handler
====================================================================
*/
static void default_event_handler( 
    GuiWidget *widget, GuiEvent *event )
{
    switch ( event->type ) {
        case GUI_DESTROY:
            if ( widget->spec.label.text )
                free( widget->spec.label.text );
            break;
        case GUI_DRAW:
            /* display surface */
            stk_surface_blit( 
                widget->surface, 0, 0, -1, -1, 
                stk_display, 
                widget->screen_region.x,
                widget->screen_region.y );
            break;
    }
}

/*
====================================================================
PUBLICS
====================================================================
*/
            
/*
====================================================================
Create a label widget with the parent's background and maybe a 
frame. 
  'border': space between text and frame
  'text': text that may contain multiple lines separated by '#'
          (duplicated)
  'align': alignment for each line
  'font': used to display the text. if NULL is passed the default
          label font is used
====================================================================
*/
GuiWidget* gui_label_create(
    GuiWidget *parent, int x, int y, int width, int height, int with_frame,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    int border, StkFont *font, int align, char *text )
{
    GuiWidget *widget = gui_widget_create( 
        parent, GUI_LABEL, x, y, width, height, 
        default_event_handler, user_event_handler );
    /* events */
    gui_widget_enable_event( widget, GUI_CLICKED );
    /* create surface and add frame -
       contents is set by label_set_text() */
    widget->surface = stk_surface_create( 
        SDL_SWSURFACE, width, height );
    SDL_SetColorKey( widget->surface, 0,0 );
    if ( with_frame )
        widget->border = stk_surface_apply_frame(
            widget->surface, 0, 0, -1, -1, 
            gui_theme->widget_frame );
    /* add some extra space between frame and text */
    widget->border += border;
    /* size w/o border */
    widget->width -= 2 * widget->border;
    widget->height -= 2 * widget->border;
    /* set font and alignment */
    widget->spec.label.font = (font)?font:gui_theme->label_font;
    widget->spec.label.align = align;
    /* do the text */
    if ( text )
        widget->spec.label.text = strdup( text );
    gui_label_set_text( widget, text );
    /* done */
    return widget;
}

/*
====================================================================
Set label font/alignment/text. Update if visible.
====================================================================
*/
void gui_label_set_font( GuiWidget *widget, StkFont *font )
{
    if ( widget->type != GUI_LABEL ) return;
    widget->spec.label.font = font;
    gui_label_set_text( widget, widget->spec.label.text );
}
void gui_label_set_align( GuiWidget *widget, int align )
{
    if ( widget->type != GUI_LABEL ) return;
    widget->spec.label.align = align;
    gui_label_set_text( widget, widget->spec.label.text );
}
static char label_buffer[1024];
void gui_label_set_text( GuiWidget *widget, char *format, ... )
{
    LabelText *text;
    int px, py, i;
    va_list args;
    if ( widget->type != GUI_LABEL ) return;
    /* free old text */
    if ( widget->spec.label.text ) {
        free( widget->spec.label.text );
        widget->spec.label.text = 0;
    }
    /* clear widget to wallpaper */
    gui_widget_apply_wallpaper( 
        widget, gui_theme->widget_wallpaper, -1 );
    /* set new text if any */
    if ( format ) {
        /* build full string */
        va_start( args, format );
        vsnprintf( label_buffer, 1024, format, args );
        va_end( args );
        /* build text */
        widget->spec.label.text = strdup( label_buffer );
        text = text_create( widget->spec.label.text,
            (widget->screen_region.w - 2 * widget->border) / 
            widget->spec.label.font->width );
        /* add text */
        widget->spec.label.font->align = widget->spec.label.align;
        if ( widget->spec.label.align & STK_FONT_ALIGN_LEFT )
            px = widget->border;
        else
            if ( widget->spec.label.align & STK_FONT_ALIGN_RIGHT )
                px = widget->screen_region.w - widget->border;
            else
                px = widget->border +
                     ((widget->screen_region.w - 
                      (widget->border<<1))>>1);
        py = (widget->parent_region.h - 
              text->count * widget->spec.label.font->height) / 2;
        for ( i = 0; i < text->count; 
              i++, py += widget->spec.label.font->height )
            stk_font_write( widget->spec.label.font, 
                widget->surface, px, py, -1, 
                text->lines[i] );
        text_delete( text );
    }
    /* display */
    if ( widget->visible )
        gui_widget_draw( widget );
}

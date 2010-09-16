/***************************************************************************
                          gui_edit.c  -  description
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
#include "gui_widget.h"
#include "gui_edit.h"

extern GuiTheme *gui_theme;
extern SDL_Surface *stk_display;
extern GuiWidget *gui_key_widget;

/*
====================================================================
LOCALS
====================================================================
*/

/*
====================================================================
Global edit variables that are used by the focused widget and
are global as there is only one keyboard they can be related
with.
====================================================================
*/
static int gui_edit_blink_time = 0; /* used to toggle gui_edit_blink
                                twice a second */
static int gui_edit_blink = 0; /* if true a cursor is displayed at 
                           the edit position */
static int gui_edit_delay = 0; 
static int gui_edit_interval = 0; /* after a delay of 300 ms the
keyevent is repeated at an interval of 100 */
static int gui_edit_keysym = -1; /* last pressed keysym */
static int gui_edit_unicode = -1; /* last pressed unicode */

/* FORWARDED */
static void gui_edit_handle_key( 
    GuiWidget *widget, int keysym, int unicode );
static int gui_edit_adjust_cursor( GuiWidget *widget, int offset );
    
/*
====================================================================
Default event handler
====================================================================
*/
static void default_event_handler( 
    GuiWidget *widget, GuiEvent *event )
{
    char *ptr;
    int px, py, i;
    switch ( event->type ) {
        case GUI_DESTROY:
            if ( widget->spec.edit.buffer )
                free( widget->spec.edit.buffer );
            if ( widget->spec.edit.display_buffer )
                free( widget->spec.edit.display_buffer );
            break;
        case GUI_DRAW:
            /* display surface */
            stk_surface_blit( 
                widget->surface, 0, 0, -1, -1, 
                stk_display, 
                widget->screen_region.x,
                widget->screen_region.y );
            /* add text */
            gui_theme->edit_font->align = STK_FONT_ALIGN_LEFT;
            ptr = widget->spec.edit.display_buffer;
            px = widget->screen_region.x + widget->border;
            py = widget->screen_region.y + widget->border + 
                 widget->spec.edit.y_offset;
            for ( i = 0; i < widget->spec.edit.height; 
                  i++, py += gui_theme->edit_font->height,
                  ptr += widget->spec.edit.width + 1 )
                stk_font_write( gui_theme->edit_font,
                    stk_display, px, py, -1, ptr );
            /* add cursor */
            if ( widget == gui_key_widget || widget->focused )
            if ( gui_edit_blink )
                stk_surface_fill( stk_display, 
                    px + widget->spec.edit.x * 
                    gui_theme->edit_font->width,
                    widget->screen_region.y + 
                    widget->border + widget->spec.edit.y_offset + 
                    widget->spec.edit.y * 
                    gui_theme->edit_font->height,
                    1, gui_theme->edit_font->height,
                    0xffffff );
            break;
        case GUI_KEY_PRESSED:
            if ( gui_edit_keysym != event->key.keysym ) {
                gui_edit_delay = 250;
                gui_edit_handle_key( widget, 
                    event->key.keysym,
                    event->key.unicode );
                //stk_sound_play( gui_theme->type_sound );
            }
            break;
        case GUI_KEY_RELEASED:
            gui_edit_keysym = -1;
            break;
        case GUI_FOCUS_OUT:
            gui_edit_blink = 0;
            gui_widget_draw( widget );
            break;
        case GUI_CLICKED:
            stk_sound_play( gui_theme->click_sound );
            px = event->button.x - widget->screen_region.x - 
                widget->border;
            py = event->button.y - widget->screen_region.y -
                widget->border - widget->spec.edit.y_offset;
            gui_edit_adjust_cursor( widget, 
                (py / gui_theme->edit_font->height) * 
                widget->spec.edit.width + 
                (px / gui_theme->edit_font->width) + 
                widget->spec.edit.start - widget->spec.edit.pos + 1 );
            gui_edit_blink = 1;
            gui_edit_blink_time = 0;
            gui_widget_draw( widget );
            break;
    }
}

/*
====================================================================
Render the display buffer from the edit string and the first
character displayed of it. No screen update.
====================================================================
*/
static void gui_edit_update_display_buffer( GuiWidget *widget )
{
    int i, length, left;
    char *src, *dest;
    memset( widget->spec.edit.display_buffer, 0, 
        (widget->spec.edit.width+1) * widget->spec.edit.height *
        sizeof( char ) );
    if ( widget->spec.edit.multi_line ) {
        src = widget->spec.edit.buffer + widget->spec.edit.start;
        left = strlen( src );
        dest = widget->spec.edit.display_buffer;
        for ( i = 0; i < widget->spec.edit.height; i++ ) {
            length = widget->spec.edit.width;
            if ( length > left )
                length = left;
            strncpy( dest, src, length );
            dest += widget->spec.edit.width + 1;
            src += length;
            left -= length;
            if ( left == 0 ) 
                break;
        }
    }
    else {
        strncpy( widget->spec.edit.display_buffer, 
            widget->spec.edit.buffer + widget->spec.edit.start,
            widget->spec.edit.width );
    }
}
/*
====================================================================
Adjust edit cursor position by offset and return True 
if the display buffer has changed. Do not update display.
====================================================================
*/
static int gui_edit_adjust_cursor( GuiWidget *widget, int offset )
{
    int new_y, changed = 0;
    /* adjust position */
    widget->spec.edit.pos += offset;
    if ( widget->spec.edit.pos < 0 ) widget->spec.edit.pos = 0;
    if ( widget->spec.edit.pos > widget->spec.edit.length )
        widget->spec.edit.pos = widget->spec.edit.length;
    /* don't blink cursor while moving */
    gui_edit_blink_time = 0; 
    /* update display position */
    if ( widget->spec.edit.multi_line ) {
        new_y = widget->spec.edit.pos / widget->spec.edit.width;
        /* if this position is out of screen modify 'line'
           and 'start' */
        if ( new_y < widget->spec.edit.line || 
             new_y >= widget->spec.edit.line + widget->spec.edit.height ) {
            if ( new_y < widget->spec.edit.line )
                widget->spec.edit.line = new_y;
            else
                widget->spec.edit.line = new_y - widget->spec.edit.height + 1;
            widget->spec.edit.start = 
                widget->spec.edit.line * widget->spec.edit.width;
            /* update the display buffer */
            gui_edit_update_display_buffer( widget );
            changed = 1;
        }
        widget->spec.edit.x = 
            widget->spec.edit.pos % widget->spec.edit.width;
        widget->spec.edit.y = new_y - widget->spec.edit.line;
    }
    else {
        if ( widget->spec.edit.pos < widget->spec.edit.start ) {
            widget->spec.edit.start = widget->spec.edit.pos;
            widget->spec.edit.x = 0;
            /* update the display buffer */
            gui_edit_update_display_buffer( widget );
            changed = 1;
        }
        else
        if ( widget->spec.edit.pos >= 
                widget->spec.edit.start + widget->spec.edit.width ) {
            widget->spec.edit.start = widget->spec.edit.pos - 
                widget->spec.edit.width;
            widget->spec.edit.x = widget->spec.edit.width;
            /* update the display buffer */
            gui_edit_update_display_buffer( widget );
            changed = 1;
        }
        else {
            /* within text */
            widget->spec.edit.x = 
                widget->spec.edit.pos - widget->spec.edit.start;
        }
    }
    return changed;
}
/*
====================================================================
Handle the pressed key, modify the edit buffer and update the
displayed edit if visible.
====================================================================
*/
static void gui_edit_handle_key( 
    GuiWidget *widget, int keysym, int unicode )
{
    int i, changed = 0, old_length = widget->spec.edit.length;
    switch ( keysym ) {
        case SDLK_RIGHT:
            gui_edit_adjust_cursor( widget, 1 );
            changed = 1;
            break;
        case SDLK_LEFT:
            gui_edit_adjust_cursor( widget, -1 );
            changed = 1;
            break;
        case SDLK_DOWN:
            gui_edit_adjust_cursor( 
                widget, widget->spec.edit.width );
            changed = 1;
            break;
        case SDLK_UP:
            gui_edit_adjust_cursor( 
                widget, -widget->spec.edit.width );
            changed = 1;
            break;
        case SDLK_HOME:
            gui_edit_adjust_cursor( 
                widget, -widget->spec.edit.pos );
            changed = 1;
            break;
        case SDLK_END:
            widget->spec.edit.pos = 0;
            gui_edit_adjust_cursor( 
                widget, widget->spec.edit.length );
            changed = 1;
            break;
        case SDLK_BACKSPACE:
            if ( widget->spec.edit.pos > 0 ) {
                --widget->spec.edit.pos;
                for ( i = widget->spec.edit.pos; 
                      i < widget->spec.edit.length - 1; i++ )
                    widget->spec.edit.buffer[i] = 
                        widget->spec.edit.buffer[i + 1];
                widget->spec.edit.buffer[i] = 0;
                widget->spec.edit.length--;
                if ( !gui_edit_adjust_cursor( widget, 0 ) )
                    gui_edit_update_display_buffer( widget );
                changed = 1;
            }
            break;
        case SDLK_DELETE:
            if ( widget->spec.edit.pos < widget->spec.edit.length ) {
                for ( i = widget->spec.edit.pos; 
                      i < widget->spec.edit.length - 1; i++ )
                    widget->spec.edit.buffer[i] = 
                        widget->spec.edit.buffer[i + 1];
                widget->spec.edit.buffer[i] = 0;
                widget->spec.edit.length--;
                gui_edit_update_display_buffer( widget );
                changed = 1;
            }
            break;
        default:
            if ( widget->spec.edit.filter[unicode] && widget->spec.edit.pos < widget->spec.edit.size )
                if ( widget->spec.edit.length < widget->spec.edit.size  ) {
                    for ( i = widget->spec.edit.size - 1; 
                          i > widget->spec.edit.pos; i-- )
                        widget->spec.edit.buffer[i] = 
                            widget->spec.edit.buffer[i - 1];
                    widget->spec.edit.buffer[widget->spec.edit.pos++] = unicode;
                    widget->spec.edit.length++;
                    if ( !gui_edit_adjust_cursor( widget, 0 ) )
                        gui_edit_update_display_buffer( widget );
                    changed = 1;
                }
        break;
    }
    if ( changed ) {
        gui_edit_blink_time = 0;
        gui_edit_blink = 1;
        gui_widget_draw( widget );
        gui_edit_interval = 0;
        gui_edit_keysym = keysym;
        gui_edit_unicode = unicode;
        if ( old_length != widget->spec.edit.length ) {
            gui_widget_call_user_event_handler( 
                widget, gui_event_get_simple( GUI_CHANGED ) );
        }
    }
    else
        gui_edit_keysym = -1;
}

/*
====================================================================
PUBLICS
====================================================================
*/

/*
====================================================================
Create a multi-line editable widget. By checking the measurements 
of the standard font the number of lines and the line width is 
determined.
  'text': is copied and displayed when first shown
  'size': limit of characters (excluding the \0)
====================================================================
*/
GuiWidget* gui_edit_create(
    GuiWidget *parent, int x, int y, int width, int height,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    int border, int multi_line, int size, char *text )
{
    GuiWidget *widget = gui_widget_create( 
        parent, GUI_EDIT, x, y, width, height, 
        default_event_handler, user_event_handler );
    /* events */
    gui_widget_enable_event( widget, GUI_CHANGED );
    /* create surface, wallpaper and frame it */
    widget->surface = stk_surface_create( 
        SDL_SWSURFACE, width, height );
    SDL_SetColorKey( widget->surface, 0,0 );
    stk_surface_apply_wallpaper( widget->surface, 0,0,-1,-1,
        gui_theme->widget_wallpaper, STK_OPAQUE );
    widget->border = stk_surface_apply_frame(
        widget->surface, 0, 0, -1, -1, 
        gui_theme->widget_frame );
    /* size w/o border */
    widget->width -= 2 * widget->border;
    widget->height -= 2 * widget->border;
    /* grab keys */
    widget->grab_keys = 1;
    /* prepare text */
    gui_edit_set_filter( widget, GUI_EDIT_DEFAULT );
    widget->spec.edit.multi_line = multi_line;
    widget->border += border;
    widget->spec.edit.width = 
        (width - widget->border*2) / 
         gui_theme->edit_font->width;
    if ( multi_line )
        widget->spec.edit.height = 
            (height - widget->border*2) / 
             gui_theme->edit_font->height;
    else
        widget->spec.edit.height = 1;
    widget->spec.edit.display_buffer = 
        calloc( (widget->spec.edit.width+1) * widget->spec.edit.height, 
            sizeof( char ) );
    if ( widget->spec.edit.display_buffer == 0 )
        GUI_ABORT( "Out Of Memory" );
    gui_edit_resize_buffer( widget, size );
    gui_edit_set_text( widget, text );
    /* center single-line edit */
    if ( !multi_line )
        widget->spec.edit.y_offset = 
            (height - 2 * widget->border - 
            gui_theme->edit_font->height) / 2;
    /* done */
    return widget;
}

/*
====================================================================
Resize the edit buffer and clear any text. (no update)
====================================================================
*/
void gui_edit_resize_buffer( GuiWidget *widget, int size )
{
    if ( widget->type != GUI_EDIT ) return;
    widget->spec.edit.size = size;
    if ( widget->spec.edit.buffer )
        free( widget->spec.edit.buffer );
    widget->spec.edit.buffer = calloc( size + 1, sizeof( char ) );
    if ( widget->spec.edit.buffer == 0 )
        GUI_ABORT( "Out Of Memory" );
}
/*
====================================================================
Set the edit's text. This resets the edit cursor's position to
end of text and updates displayed edit if visible.
====================================================================
*/
void gui_edit_set_text( GuiWidget *widget, char *text )
{
    if ( widget->type != GUI_EDIT ) return;
    /* copy text */
    snprintf( widget->spec.edit.buffer, widget->spec.edit.size + 1, "%s", text );
    widget->spec.edit.length = strlen( widget->spec.edit.buffer );
    /* reset */
    /* first character in first line */
    widget->spec.edit.pos = 0;
    widget->spec.edit.x = widget->spec.edit.y = 0;
    widget->spec.edit.start = widget->spec.edit.line = 0; 
    /* jump to end */
    if ( !gui_edit_adjust_cursor( widget, widget->spec.edit.length ) )
        gui_edit_update_display_buffer( widget );
    /* update */
    if ( widget->visible )
        gui_widget_draw( widget );
}
/*
====================================================================
Copy characters from 'start' to 'length' of the edit string to 
'buffer' (at maximum limit characters including \0). If 'length' 
is -1 the characters copied are those from 'start' to end of text.
====================================================================
*/
int gui_edit_get_text( 
    GuiWidget *widget, char *buffer, int limit, 
    int start, int length )
{
    char *ptr;
    if ( widget->type != GUI_EDIT ) return 0;
    if ( start < 0 ) start = 0;
    ptr = widget->spec.edit.buffer + start;
    if ( length == -1 ) 
        length = strlen( ptr );
    if ( length > limit )
        length = limit;
    if ( length )
        snprintf( buffer, limit, "%s", widget->spec.edit.buffer );
    else
        buffer[0] = 0;
    return 1;
}
/*
====================================================================
Update the blinking cursor flag (no update) of the edit and in 
case a key is pressed call gui_edit_handle_key().
====================================================================
*/
void gui_edit_update( GuiWidget *widget, int ms )
{
    gui_edit_blink_time += ms;
    if ( gui_edit_blink_time > 500 ) {
        gui_edit_blink_time = 0;
        gui_edit_blink = !gui_edit_blink;
        gui_widget_draw( widget );
    }
    if ( gui_edit_keysym != -1 ) {
        if ( gui_edit_delay > 0 ) {
            gui_edit_delay -= ms;
            if ( gui_edit_delay <= 0 )
                gui_edit_delay = 0;
        }
        if ( gui_edit_delay == 0 ) {
            if ( gui_edit_interval > 0 ) {
                gui_edit_interval -= ms;
                if ( gui_edit_interval < 0 )
                    gui_edit_interval = 0;
            }
            if ( gui_edit_interval == 0 ) {
                gui_edit_handle_key( widget, 
                    gui_edit_keysym, gui_edit_unicode );
                gui_edit_interval = 30;
            }
        }
    }
}
/*
====================================================================
Select a character set.
  default: all non-whitespaces >=32 && <= 128
  alpha: A-Z,a-z
  numerical: 0-9
  alphanumerical: A-Z,a-z,0-9
====================================================================
*/
void gui_edit_set_filter( GuiWidget *widget, int type )
{
    int i, j;
    if ( widget->type != GUI_EDIT ) return;
    memset( widget->spec.edit.filter, 0, 
        sizeof( widget->spec.edit.filter ) );
    switch ( type ) {
        case GUI_EDIT_DEFAULT:
            for ( i = 32; i <= 128; i++ )
                widget->spec.edit.filter[i] = 1;
            break;
        case GUI_EDIT_ALPHANUMERICAL:        
        case GUI_EDIT_ALPHANUMERICAL2:        
        case GUI_EDIT_ALPHA:
            for ( i = 65, j = 97; i <= 90; i++, j++ ) {
                widget->spec.edit.filter[i] = 1; 
                widget->spec.edit.filter[j] = 1;
            }
            if ( type == GUI_EDIT_ALPHANUMERICAL || type == GUI_EDIT_ALPHANUMERICAL2 ) {
                widget->spec.edit.filter[45] = 1;
                for ( i = 48; i <= 57; i++ )
                    widget->spec.edit.filter[i] = 1;
            }
            if ( type == GUI_EDIT_ALPHANUMERICAL2 )
                widget->spec.edit.filter[95] = 1;
            break;
        case GUI_EDIT_NUMERICAL:
            for ( i = 48; i <= 57; i++ )
                widget->spec.edit.filter[i] = 1;
            widget->spec.edit.filter[45] = 1;
            break;
    }
}

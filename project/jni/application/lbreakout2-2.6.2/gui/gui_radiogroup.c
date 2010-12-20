/***************************************************************************
                          gui_radiogroup.c  -  description
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
#include "gui_radiogroup.h"

extern GuiTheme *gui_theme;
extern SDL_Surface *stk_display;

/*
====================================================================
LOCALS
====================================================================
*/

/*
====================================================================
Default event handler
====================================================================
*/
static void default_event_handler( 
    GuiWidget *widget, GuiEvent *event )
{
    int px, py, i;
    switch ( event->type ) {
        case GUI_DESTROY:
            if ( widget->spec.radiogroup.checks )
                free( widget->spec.radiogroup.checks );
            break;
        case GUI_DRAW:
            /* display surface */
            stk_surface_blit( 
                widget->surface, 0, 0, -1, -1, 
                stk_display, 
                widget->screen_region.x,
                widget->screen_region.y );
            /* add checkers */
            px = widget->screen_region.x +
                 widget->spec.radiogroup.x;
            py = widget->screen_region.y +
                 widget->spec.radiogroup.y;
            for ( i = 0; i < widget->spec.radiogroup.size; 
                  i++, py += widget->spec.radiogroup.offset ) {
                if ( widget->spec.radiogroup.checks[i] )
                    stk_surface_blit( 
                        gui_theme->checkbox, 
                        gui_theme->checkbox_size, 0, 
                        gui_theme->checkbox_size,
                        gui_theme->checkbox_size,
                        stk_display, px, py );
            }
            break;
        case GUI_CLICKED:
            gui_radiogroup_toggle( widget, 
                (event->button.y - widget->screen_region.y - 
                widget->border) / widget->spec.radiogroup.offset );
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
Create a radiogroup or a checkbox which is nothing but a
single entried radiogroup.
  'caption': checkbox caption
  'items': items - the widget does not modify the pointers
  'checked': is True if checkbox checked else False.
Radiogroup selections can be set with gui_radiogroup_select().  
====================================================================
*/
GuiWidget* gui_checkbox_create(
    GuiWidget *parent, int x, int y, int width, int height, int with_frame,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    char *caption, int checked )
{
    GuiWidget *widget;
    char *items[1]; 
    items[0] = caption;
    /* this is just a single itemed radiogroup */
    widget = gui_radiogroup_create(
        parent, x, y, width, height, with_frame, user_event_handler,
        1, items, 0, 1 );
    gui_radiogroup_set_checked( widget, 0, checked );
    return widget;
}
GuiWidget* gui_radiogroup_create(
    GuiWidget *parent, int x, int y, int width, int height, int with_frame,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    int item_count, char **items, int min, int max )
{
    int px, py, i, j, gap = 4;
    GuiWidget *widget = gui_widget_create( 
        parent, GUI_RADIOGROUP, x, y, width, height, 
        default_event_handler, user_event_handler );
    /* events */
    gui_widget_enable_event( widget, GUI_CLICKED );
    /* create surface, wallpaper and frame it */
    widget->surface = stk_surface_create( 
        SDL_SWSURFACE, width, height );
    SDL_SetColorKey( widget->surface, 0,0 );
    stk_surface_apply_wallpaper( widget->surface, 0,0,-1,-1,
        gui_theme->widget_wallpaper, -1 );
    if ( with_frame )
        widget->border = stk_surface_apply_frame(
            widget->surface, 0, 0, -1, -1, 
            gui_theme->widget_frame );
    /* size w/o border */
    widget->width -= 2 * widget->border;
    widget->height -= 2 * widget->border;
    /* various */
    widget->spec.radiogroup.size = item_count;
    widget->spec.radiogroup.min = min;
    widget->spec.radiogroup.max = max;
    widget->spec.radiogroup.single_check = -1;
    /* compute offset of single items */
    widget->spec.radiogroup.offset = 
        (height - 2 * widget->border) / 
        item_count;
    /* position of first checkbox */
    widget->spec.radiogroup.x = widget->border + gap;
    widget->spec.radiogroup.y = widget->border + 
        (widget->spec.radiogroup.offset - 
         gui_theme->checkbox_size) / 2;
    /* add empty checkboxes and labels */
    gui_theme->label_font->align = STK_FONT_ALIGN_CENTER_Y;
    px = widget->spec.radiogroup.x;
    py = widget->spec.radiogroup.y;
    for ( i = 0, j = 0; i < widget->spec.radiogroup.size; 
          i++, py += widget->spec.radiogroup.offset, j++ ) {
        stk_surface_blit( 
            gui_theme->checkbox, 0, 0,
            gui_theme->checkbox_size,
            gui_theme->checkbox_size,
            widget->surface, px, py );
        stk_font_write( gui_theme->label_font, 
            widget->surface, px + gui_theme->checkbox_size + gap,
            py + gui_theme->checkbox_size/2, -1, items[j] );
    }
    /* create empty checkers */
    widget->spec.radiogroup.checks = calloc( item_count, sizeof( int ) );
    if ( widget->spec.radiogroup.checks == 0 )
        GUI_ABORT( "Out Of Memory" );
    return widget;
}

/*
====================================================================
Set or check if a checkbox in the radiogroup is set.
Stores refresh rects if visible.
====================================================================
*/
void gui_radiogroup_set_checked( 
    GuiWidget *widget, int id, int check )
{
    if ( widget->type != GUI_RADIOGROUP )
        return;
    if ( id >= widget->spec.radiogroup.size || id < 0 )
        return;
    /* if there is at maximum one item selectable
       we clear the selection to simplify usage */
    if ( widget->spec.radiogroup.max == 1 ) {
        memset( widget->spec.radiogroup.checks, 0, 
                widget->spec.radiogroup.size * sizeof( int ) );
        widget->spec.radiogroup.check_count = 0;
        widget->spec.radiogroup.single_check = -1;
    }
    if ( check ) {
        /* select item if possible */
        if ( widget->spec.radiogroup.check_count ==
             widget->spec.radiogroup.max )
            return;
        /* if only one selection is allowed unselect old checker */
        widget->spec.radiogroup.checks[id] = 1;
        widget->spec.radiogroup.single_check = id;
        widget->spec.radiogroup.check_count++;
    }
    else {
        /* unselect item if possible */
        if ( widget->spec.radiogroup.check_count ==
             widget->spec.radiogroup.min )
            return;
        if ( widget->spec.radiogroup.checks[id] == 0 )
            return;
        widget->spec.radiogroup.checks[id] = 0;
        widget->spec.radiogroup.single_check = -1;
        widget->spec.radiogroup.check_count--;
    }
    if ( widget->visible )
        gui_widget_draw( widget );
}
void gui_radiogroup_toggle( GuiWidget *widget, int id )
{
    if ( widget->type != GUI_RADIOGROUP )
        return;
    if ( id >= widget->spec.radiogroup.size || id < 0 )
        return;
    gui_radiogroup_set_checked( 
        widget, id, !widget->spec.radiogroup.checks[id] );
}
int gui_radiogroup_is_checked( GuiWidget *widget, int id )
{
    if ( widget->type != GUI_RADIOGROUP )
        return 0;
    if ( id >= widget->spec.radiogroup.size || id < 0 )
        return 0;
    return widget->spec.radiogroup.checks[id];
}
/*
====================================================================
For radiogroups with at max one items this returns the currently
checked id or -1.
====================================================================
*/
int gui_radiogroup_get_checked( GuiWidget *widget )
{
    if ( widget->type != GUI_RADIOGROUP )
        return -1;
    if ( widget->spec.radiogroup.max != 1 )
        return -1;
    return widget->spec.radiogroup.single_check;
}


/***************************************************************************
                          gui_list.c  -  description
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
#include "gui_widget.h"
#include "gui_button.h"
#include "gui_scrollbar.h"
#include "gui_list.h"

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
    Uint32 color;
    SDL_Rect rect;
    int pos, px, py, item_id, i, j;
    switch ( event->type ) {
        case GUI_DESTROY:
            if ( widget->spec.list.checks )
                free( widget->spec.list.checks );
            stk_surface_free( &widget->spec.list.render_buffer );
            break;
        case GUI_DRAW:
            /* display surface */
            stk_surface_blit( 
                widget->surface, 0, 0, -1, -1, 
                stk_display, 
                widget->screen_region.x,
                widget->screen_region.y );
            /* render items */
            if ( widget->spec.list.item_count == 0 )
                break;
            pos = 0;
            if ( widget->spec.list.scrollbar )
                gui_scrollbar_get_value( 
                    widget->spec.list.scrollbar, &pos );
            item_id = pos * widget->spec.list.columns;
            py = widget->screen_region.y + widget->border;
            for ( j = 0; j < widget->spec.list.rows_per_page; j++ ) {
                px = widget->screen_region.x + widget->border;
                for ( i = 0; i < widget->spec.list.columns; i++ ) {
                    if ( (widget->spec.list.select_type == 
                         GUI_LIST_SINGLE_SELECT &&
                         widget->spec.list.checks[0] == item_id) ||
                         (widget->spec.list.select_type ==
                         GUI_LIST_MULTI_SELECT &&
                         widget->spec.list.checks[item_id] ) ) {
                        color = stk_surface_get_pixel( 
                             gui_theme->list_item_color, 0, 0 );
                        SDL_FillRect( 
                             widget->spec.list.render_buffer, 0, color );
                        stk_surface_alpha_blit( widget->surface, 
                            px - widget->screen_region.x,
                            py - widget->screen_region.y,
                            widget->spec.list.item_width,
                            widget->spec.list.item_height,
                            widget->spec.list.render_buffer, 0, 0,
                            192 );
                    }
                    else
                        stk_surface_fill( 
                            widget->spec.list.render_buffer, 0,0,-1,-1, 
                            0x0 );
                    if ( widget->spec.list.render_item( 
                        i, j + pos, widget->spec.list.render_buffer ) )
                        stk_surface_blit( 
                            widget->spec.list.render_buffer,
                            0,0,-1,-1, stk_display, px, py );
                    px += widget->spec.list.item_width + 
                        widget->spec.list.gap;
                    item_id++;
                    if ( item_id >= widget->spec.list.item_count )
                        break;
                }
                py += widget->spec.list.item_height + 
                    widget->spec.list.gap;
                if ( item_id >= widget->spec.list.item_count )
                    break;
            }
            break;
        case GUI_BUTTON_PRESSED:
            if ( event->button.button == STK_WHEEL_UP )
                gui_scrollbar_add( widget->spec.list.scrollbar, 
                    -widget->spec.list.scrollbar->spec.scrollbar.step*2 );
            else
            if ( event->button.button == STK_WHEEL_DOWN )
                gui_scrollbar_add( widget->spec.list.scrollbar, 
                    widget->spec.list.scrollbar->spec.scrollbar.step*2 );
            else {
                /* handle selection */
                pos = 0;
                if ( widget->spec.list.scrollbar )
                    gui_scrollbar_get_value( 
                        widget->spec.list.scrollbar, &pos );
                rect.w = widget->spec.list.item_width;
                rect.h = widget->spec.list.item_height;
                rect.y = widget->screen_region.y +
                    widget->border;
                for ( j = 0; j < widget->spec.list.rows_per_page; j++ ) {
                    rect.x = widget->screen_region.x + 
                        widget->border;
                    for ( i = 0; i < widget->spec.list.columns; i++ ) {
                        if ( STK_IN_RECT( rect, 
                                 event->button.x, 
                                 event->button.y ) ) {
                            gui_list_select( 
                                widget, i, j + pos, -1 );
                            return;
                        }
                        rect.x += widget->spec.list.item_width + 
                                  widget->spec.list.gap;
                    }
                    rect.y += widget->spec.list.item_height + 
                              widget->spec.list.gap;
                }
            }
            break;
    }
}

/*
====================================================================
Dummy routine to produce an empty list.
====================================================================
*/
static int gui_list_render_item_dummy( 
    int x, int y, SDL_Surface *buffer )
{
    return 0;
}
/*
====================================================================
Default handler for listbox scrollbar.
====================================================================
*/
static void gui_list_scrollbar_handler(
    GuiWidget *widget, GuiEvent *event )
{
    /* update the listbox */
    if ( event->type == GUI_CHANGED )
        gui_widget_draw( widget->parent );
    /* call event handler */
    gui_widget_call_user_event_handler( widget->parent, event );
}

/*
====================================================================
PUBLICS
====================================================================
*/

/*
====================================================================
Create a listbox that may have multiple columns which cannot be
scrolled however. 
  'gap': gap between items and frame
  'columns': the item width is determined by available_width/columns
  'item_height': the number of rows is unlimited and this is the
                 height of an item
  'scrollbar_width' : if > 0 a vertical scrollbar is provided
  'render_item': The scrollbar value (0 if none) tells which row
                 is the first displayed and 
                 render_item(row,column,buffer) is called to 
                 obtain the graphic for each visible item.
                 If 'render_item' returns False the item is not
                 displayed.
  'select_type': no, single, multi select
====================================================================
*/
GuiWidget* gui_list_create(
    GuiWidget *parent, int x, int y, int width, int height,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    int gap, int columns, int item_height,
    int scrollbar_width, int (*render_item)(int,int,SDL_Surface*),
    int select_type )
{
    int old_border;
    GuiWidget *widget = gui_widget_create( 
        parent, GUI_LIST, x, y, width, height, 
        default_event_handler, user_event_handler );
    /* events */
    gui_widget_enable_event( widget, GUI_ITEM_SELECTED );
    gui_widget_enable_event( widget, GUI_ITEM_UNSELECTED );
    /* create surface and wallpaper it */
    widget->surface = stk_surface_create( 
        SDL_SWSURFACE, width, height );
    SDL_SetColorKey( widget->surface, 0,0 );
    stk_surface_apply_wallpaper(
        widget->surface, 0, 0, -1, -1, 
        gui_theme->widget_wallpaper, -1 );
    /* add frame leaving space for scrollbar */
    widget->border = stk_surface_apply_frame(
        widget->surface, 0, 0, width - scrollbar_width, height,
        gui_theme->widget_frame );
    /* size w/o border */
    widget->width -= 2 * widget->border;
    widget->height -= 2 * widget->border;
    /* distance between frame and items */
    widget->spec.list.gap = gap;
    widget->border += gap;
    /* item settings */
    widget->spec.list.columns = columns;
    widget->spec.list.item_width = 
        (width - scrollbar_width - widget->border*2 + gap) / 
        columns - gap;
    widget->spec.list.item_height = item_height;
    widget->spec.list.rows_per_page = 
        (height - widget->border*2 + gap) / (item_height + gap);
    /* selection */
    widget->spec.list.select_type = select_type;
    widget->spec.list.checks = calloc( 1, sizeof( int ) );
    if ( widget->spec.list.checks == 0 )
        GUI_ABORT( "Out Of Memory" );
    /* render callback */
    if ( render_item == 0 ) {
        fprintf( stderr, 
            "You must specify a render callback for lists!\n" );
        widget->spec.list.render_item = gui_list_render_item_dummy;
    }
    else
        widget->spec.list.render_item = render_item;
    /* build scrollbar if any */
    if ( scrollbar_width > 0 ) {
        /* the border is set to 0 as the frame doesn't 
           apply to the scrollbar */
        old_border = widget->border;
        widget->border = 0;
        widget->spec.list.scrollbar = gui_scrollbar_create( widget, 
                width - scrollbar_width, 0, scrollbar_width, height,
                gui_list_scrollbar_handler, 1,
                0, 0, 1, widget->spec.list.rows_per_page / 2, 0 );
        widget->border = old_border;
        gui_widget_disable_event( widget->spec.list.scrollbar, GUI_FOCUS_IN );
        gui_widget_disable_event( widget->spec.list.scrollbar, GUI_FOCUS_OUT );
    }
    /* create the render buffer */
    widget->spec.list.render_buffer =
        stk_surface_create( SDL_SWSURFACE, 
            widget->spec.list.item_width, widget->spec.list.item_height );
    if ( widget->spec.list.render_buffer == 0 )
        GUI_ABORT( "Out Of Memory" );
    return widget;
}

/*
====================================================================
Set item count of list and update it even if item count hasn't
changed. This function is used to tell the list that it
contents has changed and needs to be redrawn. All selections
are cleared by this function.
====================================================================
*/
void gui_list_update( GuiWidget *widget, int count )
{
    int value = 0, old_count = widget->spec.list.item_count;
    if ( widget->type != GUI_LIST ) return;
    /* adjust count and row number */
    widget->spec.list.item_count = count;
    widget->spec.list.rows = 
        widget->spec.list.item_count / widget->spec.list.columns +
        ((widget->spec.list.item_count % 
          widget->spec.list.columns) != 0);
    /* clear checks */
    if ( widget->spec.list.select_type == GUI_LIST_MULTI_SELECT ) {
        free( widget->spec.list.checks );
        widget->spec.list.checks = 
            calloc( (count<=0)?1:count, sizeof( int ) );
    }
    else
        widget->spec.list.checks[0] = -1;
    /* set range and value of scrollbar */
    if ( widget->spec.list.scrollbar ) {
        if ( count > old_count ) {
            /* additional items. current scrollbar value will
               still be valid. no screen update yet. */
            if ( widget->visible )
                widget->spec.list.scrollbar->
                    spec.scrollbar.track->visible = 0;
            value = widget->spec.list.rows - 
                widget->spec.list.rows_per_page;
            if ( value < 0 ) value = 0;
            gui_scrollbar_set_range( 
                widget->spec.list.scrollbar, 0, value, -1, -1 );
            if ( widget->visible )
                widget->spec.list.scrollbar->
                    spec.scrollbar.track->visible = 1;
        }
        else if ( count < old_count ) {
            /* lesser items. the current scrollbar value 
               may be out of range. no screen update yet. */
            gui_scrollbar_get_value( 
                widget->spec.list.scrollbar, &value );
            if ( widget->visible )
                widget->spec.list.scrollbar->
                    spec.scrollbar.track->visible = 0;
            value = widget->spec.list.rows - 
                widget->spec.list.rows_per_page;
            if ( value < 0 ) value = 0;
            if ( value + widget->spec.list.rows_per_page > 
                 widget->spec.list.rows ) {
                gui_scrollbar_set_value( widget->spec.list.scrollbar, 
                     value );
            }
            gui_scrollbar_set_range( 
                widget->spec.list.scrollbar, 0, value, -1, -1 );
            if ( widget->visible )
                widget->spec.list.scrollbar->
                    spec.scrollbar.track->visible = 1;
        }
    }
    /* redraw */
    if ( widget->visible )
        gui_widget_draw( widget );
}
/*
====================================================================
Handle a listbox selection and update listbox.
If 'select' is 1 the item is selected, if it's 0 the item
is unselected, -1 means to toggle the value.
If SINGLE_SELECT toggeling the current selection keeps selection.
====================================================================
*/
void gui_list_select( 
    GuiWidget *widget, int column, int row, int select )
{
    GuiEvent event;
    int id, pos;
    if ( widget->type != GUI_LIST ) return;
    if ( widget->spec.list.select_type == GUI_LIST_NO_SELECT ) return;
    event.type = GUI_NONE;
    event.item.x = column;
    event.item.y = row;
    /* get id of item */
    id = row * widget->spec.list.columns + column;
    /* update selection */
    if ( widget->spec.list.select_type == GUI_LIST_SINGLE_SELECT ) {
        if ( select == 1 || select == -1 ) {
            widget->spec.list.checks[0] = id;
            event.type = GUI_ITEM_SELECTED;
        }
        else
            if ( select == 0 && id == widget->spec.list.checks[0] ) {
                 widget->spec.list.checks[0] = -1;
                event.type = GUI_ITEM_UNSELECTED;
            }
    }
    else {
        if ( select == -1 )
            widget->spec.list.checks[id] = !widget->spec.list.checks[id];
        else
            widget->spec.list.checks[id] = select;
        event.type = (widget->spec.list.checks[id]) ? 
                     GUI_ITEM_SELECTED :
                     GUI_ITEM_UNSELECTED;
    }
    /* selection event? */
    if ( event.type != GUI_NONE )
        gui_widget_call_user_event_handler( widget, &event );
    /* redraw */
    if ( widget->visible ) {
        pos = 0;
        if ( widget->spec.list.scrollbar )
            gui_scrollbar_get_value( 
                widget->spec.list.scrollbar, &pos );
        if ( row >= pos )
            if ( row < pos + widget->spec.list.rows_per_page )
                gui_widget_draw( widget );
    }
}

/*
====================================================================
Display passed row as first one or jump to end if 'row' is -1 .
====================================================================
*/
void gui_list_goto( GuiWidget *widget, int row )
{
    if ( widget->type != GUI_LIST ) return;
    if ( row == -1 )
        gui_scrollbar_add( widget->spec.list.scrollbar, 
            widget->spec.list.item_count );
    else
        gui_scrollbar_add( widget->spec.list.scrollbar, 
            row - widget->spec.list.scrollbar->spec.scrollbar.value );
}

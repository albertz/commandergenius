/***************************************************************************
                          gui_scrollbar.c  -  description
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
    switch ( event->type ) {
        case GUI_DESTROY:
            break;
        case GUI_DRAW:
            /* display surface */
            stk_surface_blit( 
                widget->surface, 0, 0, -1, -1, 
                stk_display, 
                widget->screen_region.x,
                widget->screen_region.y );
            break;
        case GUI_BUTTON_PRESSED:
            if ( widget->spec.scrollbar.vertical ) {
                if ( event->button.y < 
                    widget->spec.scrollbar.track->
                    screen_region.y )
                    gui_scrollbar_add( widget, 
                        -widget->spec.scrollbar.jump );
                else
                    gui_scrollbar_add( widget, 
                        widget->spec.scrollbar.jump );
            }
            else {
                if ( event->button.x < 
                    widget->spec.scrollbar.track->
                    screen_region.x )
                    gui_scrollbar_add( widget, 
                        -widget->spec.scrollbar.jump );
                else
                    gui_scrollbar_add( widget, 
                        widget->spec.scrollbar.jump );
            }
            break;
    }
}

/*
====================================================================
Warp track button of scrollbar to correct position.
====================================================================
*/
static void gui_scrollbar_update_track( GuiWidget *widget )
{
    int offset;
    if ( widget->type != GUI_SCROLLBAR ) return;
    if ( widget->spec.scrollbar.min == widget->spec.scrollbar.max )
        offset = 0;
    else
        offset = 
            (((widget->spec.scrollbar.vertical) ? 
                widget->parent_region.h :
                widget->parent_region.w) -
             (widget->spec.scrollbar.button_size * 3) ) *
            widget->spec.scrollbar.value /
            (widget->spec.scrollbar.max - widget->spec.scrollbar.min);
    gui_widget_warp( widget->spec.scrollbar.track, 
        (widget->spec.scrollbar.vertical) ? 0 :
            (offset + widget->spec.scrollbar.button_size), 
        (widget->spec.scrollbar.vertical)?
            (offset + widget->spec.scrollbar.button_size) : 0 );
}

/*
====================================================================
Update the scrollbar value by position of track button,
return True if it has changed and call event handler with
GUI_CHANGED.
====================================================================
*/
static int gui_scrollbar_adjust( GuiWidget *widget )
{
    int new_value;
    if ( widget->type != GUI_SCROLLBAR ) return 0;
    new_value = (((widget->spec.scrollbar.vertical) ?
                widget->spec.scrollbar.track->parent_region.y :
                widget->spec.scrollbar.track->parent_region.x) -
                widget->spec.scrollbar.button_size) *
                (widget->spec.scrollbar.max - widget->spec.scrollbar.min) /
                (((widget->spec.scrollbar.vertical) ?
                 widget->parent_region.h :
                 widget->parent_region.w) -
                widget->spec.scrollbar.button_size * 3);
    if ( new_value != widget->spec.scrollbar.value ) {
        widget->spec.scrollbar.value = new_value;
        gui_widget_call_user_event_handler( 
            widget, gui_event_get_simple( GUI_CHANGED ) );
        return 1;
    }
    return 0;
}

/*
====================================================================
Default event handlers for scrollbar buttons: update the
scrollbar if a button was clicked and pass the event to the 
scrollbar's user event handler.
====================================================================
*/
static void gui_scrollbar_dec_handler( 
    GuiWidget *widget, GuiEvent *event )
{
    if ( event->type == GUI_CLICKED )
        gui_scrollbar_add( widget->parent, 
            -widget->parent->spec.scrollbar.step );
    gui_widget_call_user_event_handler(
        widget->parent, event );
}
static void gui_scrollbar_inc_handler( 
    GuiWidget *widget, GuiEvent *event )
{
    if ( event->type == GUI_CLICKED )
        gui_scrollbar_add( widget->parent, 
            widget->parent->spec.scrollbar.step );
    gui_widget_call_user_event_handler(
        widget->parent, event );
}
static void gui_scrollbar_track_handler( 
    GuiWidget *widget, GuiEvent *event )
{
    int pos;
    if ( event->type == GUI_MOTION )
    if ( event->motion.button ) {
        if ( widget->parent->spec.scrollbar.vertical ) {
            /* absolute offset in parent */
            pos = widget->parent_region.y + 
                  event->motion.yrel;
            if ( pos < widget->parent->spec.scrollbar.button_size )
                pos = widget->parent->spec.scrollbar.button_size;
            else
            if ( pos >= widget->parent->parent_region.h - 
                        (widget->parent->spec.scrollbar.button_size<<1) )
                pos = widget->parent->parent_region.h - 
                      (widget->parent->spec.scrollbar.button_size<<1);
            /* move */
            gui_widget_warp( widget, 0, pos );
        }
        else {
            /* absolute offset in parent */
            pos = widget->parent_region.x + 
                  event->motion.xrel;
            if ( pos < widget->parent->spec.scrollbar.button_size )
                pos = widget->parent->spec.scrollbar.button_size;
            else
            if ( pos >= widget->parent->parent_region.w - 
                        (widget->parent->spec.scrollbar.button_size<<1) )
                pos = widget->parent->parent_region.w - 
                      (widget->parent->spec.scrollbar.button_size<<1);
            /* move */
            gui_widget_warp( widget, pos, 0 );
        }
        /* update value */
        gui_scrollbar_adjust( widget->parent );
    }
    /* send event */
    gui_widget_call_user_event_handler(
        widget->parent, event );
}

/*
====================================================================
PUBLICS
====================================================================
*/

/*
====================================================================
Create an either vertical or horizontal scrollbar. The increase/
decrease/track buttons are square.
====================================================================
*/
GuiWidget* gui_scrollbar_create(
    GuiWidget *parent, int x, int y, int width, int height,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    int vertical, int min, int max, int step, int jump, int value )
{
    int size;
    GuiWidget *widget = gui_widget_create( 
        parent, GUI_SCROLLBAR, x, y, width, height,
        default_event_handler, user_event_handler );
    /* events */
    gui_widget_enable_event( widget, GUI_CHANGED );
    /* create surface and wallpaper it */
    widget->surface = stk_surface_create( 
        SDL_SWSURFACE, width, height );
    SDL_SetColorKey( widget->surface, 0,0 );
    stk_surface_apply_wallpaper(
        widget->surface, 0, 0, -1, -1, 
        gui_theme->widget_wallpaper, -1 );
    /* get scrollbar size and add frame */
    if ( vertical ) {
        size = width;
        stk_surface_apply_frame( 
            widget->surface, 0, size, size, height - 2 * size, 
            gui_theme->widget_frame );
    }
    else {
        size = height;
        stk_surface_apply_frame( 
            widget->surface, size, 0, width - 2 * size, size, 
            gui_theme->widget_frame );
    }
    /* various */
    widget->spec.scrollbar.vertical = vertical;
    widget->spec.scrollbar.button_size = size;
    /* add inc/dec/track buttons */
    widget->spec.scrollbar.dec = 
        gui_button_create( widget, 0, 0, size, size, 
                           gui_scrollbar_dec_handler,
                           gui_theme->scrollbar_arrows,
                           (vertical)?0:
                               (gui_theme->scrollbar_arrow_size*2), 
                           0,
                           gui_theme->scrollbar_arrow_size,
                           gui_theme->scrollbar_arrow_size, 0 );
    gui_widget_disable_event( widget->spec.scrollbar.dec, GUI_FOCUS_IN );
    gui_widget_disable_event( widget->spec.scrollbar.dec, GUI_FOCUS_OUT );
    widget->spec.scrollbar.inc = 
        gui_button_create( widget, 
                           (vertical)?0:(width-size), 
                           (vertical)?(height-size):0, 
                           size, size, 
                           gui_scrollbar_inc_handler,
                           gui_theme->scrollbar_arrows,
                           gui_theme->scrollbar_arrow_size + 
                           ((vertical)?0:
                               (gui_theme->scrollbar_arrow_size*2)), 
                           0,
                           gui_theme->scrollbar_arrow_size,
                           gui_theme->scrollbar_arrow_size, 0 );
    gui_widget_disable_event( widget->spec.scrollbar.inc, GUI_FOCUS_IN );
    gui_widget_disable_event( widget->spec.scrollbar.inc, GUI_FOCUS_OUT );
    widget->spec.scrollbar.track =
        gui_button_create( widget, 0, 0, size, size, 
                           gui_scrollbar_track_handler,
                           0,0,0,0,0, 0 );
    /* our trackbutton needs the motion event monitored */
    gui_widget_enable_event( widget->spec.scrollbar.track, GUI_MOTION );
    gui_widget_disable_event( widget->spec.scrollbar.track, GUI_FOCUS_IN );
    gui_widget_disable_event( widget->spec.scrollbar.track, GUI_FOCUS_OUT );
    /* track grabs focus if pressed to simplify usage */
    widget->spec.scrollbar.track->grab_focus = 1;
    /* range */
    gui_scrollbar_set_range( widget, min, max, step, jump );
    gui_scrollbar_set_value( widget, value );
    /* done */
    return widget;
}

/*
====================================================================
Set or get scrollbar value. Get returns False if this was no 
scrollbar.
====================================================================
*/
void gui_scrollbar_set_value( GuiWidget *widget, int value )
{
    if ( widget->type == GUI_SCROLLBAR ) {
        widget->spec.scrollbar.value = value;
        gui_scrollbar_update_track( widget );
    }
}
int gui_scrollbar_get_value( GuiWidget *widget, int *value )
{
    if ( widget->type != GUI_SCROLLBAR )
        return 0;
    *value = widget->spec.scrollbar.value;
    return 1;
}
/*
====================================================================
Set the range of the scrollbar. Stores refresh rects if visible.
-1 for any value means to keep the current setting.
====================================================================
*/
int gui_scrollbar_set_range( 
    GuiWidget *widget, int min, int max, int step, int jump )
{
    if ( widget->type != GUI_SCROLLBAR )
        return 0;
    if ( min != -1 ) widget->spec.scrollbar.min = min;
    if ( max != -1 ) widget->spec.scrollbar.max = max;
    if ( step != -1 ) widget->spec.scrollbar.step = step;
    if ( jump != -1 ) widget->spec.scrollbar.jump = jump;
    gui_scrollbar_update_track( widget );
    return 1;
}
/*
====================================================================
Modify value of scrollbar by the passed amount and update,
return True if it has changed and call event handler with
GUI_CHANGED.
====================================================================
*/
int gui_scrollbar_add( GuiWidget *widget, int change )
{
    int old_value = widget->spec.scrollbar.value;
    if ( widget->type != GUI_SCROLLBAR ) return 0;
    widget->spec.scrollbar.value += change;
    if ( widget->spec.scrollbar.value < widget->spec.scrollbar.min )
        widget->spec.scrollbar.value = widget->spec.scrollbar.min;
    if ( widget->spec.scrollbar.value > widget->spec.scrollbar.max )
        widget->spec.scrollbar.value = widget->spec.scrollbar.max;
    gui_scrollbar_update_track( widget );
    if ( old_value != widget->spec.scrollbar.value ) {
        gui_widget_call_user_event_handler( 
            widget, gui_event_get_simple( GUI_CHANGED ) );
        return 1;
    }
    return 0;
}


/***************************************************************************
                          gui_box.c  -  description
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
#include "gui_box.h"

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
    }
}

/*
====================================================================
PUBLICS
====================================================================
*/

/*
====================================================================
Create a box widget that can be used to contain various other
widgets of any kind. If 'grab_input' is True and this widget is 
opened as a root widget it will not pass unhandled events to the 
lower-level root widgets thus it blocks events.
====================================================================
*/
GuiWidget* gui_box_create( 
    GuiWidget *parent, int x, int y, int width, int height, 
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    int grab_input )
{
    GuiWidget *widget = gui_widget_create(
        parent, GUI_BOX, x, y, width, height, 
        default_event_handler, user_event_handler );
    /* create surface, wallpaper and frame it */
    widget->surface = stk_surface_create( 
        SDL_SWSURFACE, width, height );
    SDL_SetColorKey( widget->surface, 0,0 );
    stk_surface_apply_wallpaper( widget->surface, 0,0,-1,-1,
        gui_theme->box_wallpaper, -1 );
    widget->border = stk_surface_apply_frame(
        widget->surface, 0, 0, -1, -1, 
        (parent==0)?gui_theme->box_frame:gui_theme->widget_frame );
    /* various */
    widget->width -= 2 * widget->border;
    widget->height -= 2 * widget->border;
    widget->grab_input = grab_input;
    /* done */
    return widget;
}

/***************************************************************************
                          gui_icon.c  -  description
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
#include "gui_icon.h"

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
Create an icon widget that displays the icon centered and uses
the parent's background.
====================================================================
*/
GuiWidget *gui_icon_create( 
    GuiWidget *parent, int x, int y, int width, int height, int with_frame,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    SDL_Surface *icon, int ix, int iy, int iw, int ih )
{
    GuiWidget *widget = gui_widget_create( 
        parent, GUI_ICON, x, y, width, height, 
        default_event_handler, user_event_handler );
    /* events */
    gui_widget_enable_event( widget, GUI_CLICKED );
    /* create surface and add frame -
       contents is set by icon_set_icon() */
    widget->surface = stk_surface_create( 
        SDL_SWSURFACE, width, height );
    if ( with_frame )
        widget->border = stk_surface_apply_frame(
            widget->surface, 0, 0, -1, -1, 
            gui_theme->widget_frame );
    /* size w/o border */
    widget->width -= 2 * widget->border;
    widget->height -= 2 * widget->border;
    /* set icon */
    gui_icon_set_icon( widget, icon, ix, iy, iw, ih );
    /* done */
    return widget;
}

/*
====================================================================
Apply a new icon.
====================================================================
*/
void gui_icon_set_icon( 
    GuiWidget *widget, SDL_Surface *icon, 
    int ix, int iy, int iw, int ih )
{
    if ( widget->type != GUI_ICON ) return;
    /* clear widget to wallpaper */
    gui_widget_apply_wallpaper( 
        widget, gui_theme->widget_wallpaper, -1 );
    /* add icon */
    if ( icon ) {
        /* adjust size */
        if ( iw == -1 ) iw = icon->w;
        if ( ih == -1 ) ih = icon->h;
        stk_surface_blit( icon, ix, iy, iw, ih,
            widget->surface, 
            (widget->surface->w - iw)/2, 
            (widget->surface->h - ih)/2 );
    }
    /* done */
    if ( widget->visible )
        gui_widget_draw( widget );
}


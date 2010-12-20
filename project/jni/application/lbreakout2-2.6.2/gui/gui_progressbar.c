/***************************************************************************
                          gui_progressbar.c  -  description
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

#include "gui_widget.h"
#include "gui_progressbar.h"

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
static char progressbar_hint[32];
static void default_event_handler( 
    GuiWidget *widget, GuiEvent *event )
{
    switch ( event->type ) {
        case GUI_DESTROY:
            break;
        case GUI_DRAW:
            /* display surface */
            stk_surface_blit( 
                widget->surface, 0,0,-1,-1, 
                stk_display, 
                widget->screen_region.x,
                widget->screen_region.y );
            /* add progress */
            stk_surface_apply_wallpaper( stk_display,
                widget->screen_region.x + widget->border,
                widget->screen_region.y + widget->border,
                widget->spec.progressbar.length,
                widget->screen_region.h - (widget->border<<1),
                widget->spec.progressbar.wallpaper, -1 );
            /* if focused add info */
            if ( widget->focused ) {
                gui_theme->progressbar_font->align = 
                    STK_FONT_ALIGN_CENTER_X | 
                    STK_FONT_ALIGN_CENTER_Y;
                snprintf( progressbar_hint, 32, "%i/%i",
                          widget->spec.progressbar.value,
                          widget->spec.progressbar.max );
                stk_font_write( gui_theme->progressbar_font, 
                    stk_display, 
                    widget->screen_region.x + 
                    widget->screen_region.w/2,
                    widget->screen_region.y + 
                    widget->screen_region.h/2,
                    -1, progressbar_hint );
            }
            break;
        case GUI_FOCUS_IN:
        case GUI_FOCUS_OUT:
            gui_widget_draw( widget );
            break;
    }
}

/*
====================================================================
Adjust length of the beam and set correct beam wallpaper.
(no update)
====================================================================
*/
static void gui_progressbar_adjust( GuiWidget *widget )
{
    int ratio = 100 * widget->spec.progressbar.value / 
                widget->spec.progressbar.max;
    /* wallpaper */
    if ( ratio > 60 )
        widget->spec.progressbar.wallpaper = 
            gui_theme->progress_high;
    else
        if ( ratio > 25 )
            widget->spec.progressbar.wallpaper = 
                gui_theme->progress_medium;
        else
            widget->spec.progressbar.wallpaper =
                gui_theme->progress_low;
    /* length */
    widget->spec.progressbar.length = 
        (widget->parent_region.w - (widget->border<<1)) *
        widget->spec.progressbar.value / widget->spec.progressbar.max;
}

/*
====================================================================
PUBLICS
====================================================================
*/

/*
====================================================================
Create a framed progress bar.
====================================================================
*/
GuiWidget *gui_progressbar_create( 
    GuiWidget *parent, int x, int y, int width, int height,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    int max, int value )
{
    GuiWidget *widget = gui_widget_create( 
        parent, GUI_PROGRESSBAR, x, y, width, height, 
        default_event_handler, user_event_handler );
    /* create surface, wallpaper and frame it */
    widget->surface = stk_surface_create( 
        SDL_SWSURFACE, width, height );
    SDL_SetColorKey( widget->surface, 0,0 );
    stk_surface_apply_wallpaper( widget->surface, 0,0,-1,-1,
        gui_theme->widget_wallpaper, -1 );
    widget->border = stk_surface_apply_frame(
        widget->surface, 0, 0, -1, -1, 
        gui_theme->widget_frame );
    /* size w/o border */
    widget->width -= 2 * widget->border;
    widget->height -= 2 * widget->border;
    /* set value */
    widget->spec.progressbar.max = max;
    gui_progressbar_set_value( widget, value );
    /* done */
    return widget;
}

/*
====================================================================
Get/Set value and limit of progressbar.
====================================================================
*/
void gui_progressbar_set_value( 
    GuiWidget *widget, int value )
{
    if ( widget->type != GUI_PROGRESSBAR ) return;
    /* set value */
    widget->spec.progressbar.value = value;
    gui_progressbar_adjust( widget );
    /* done */
    if ( widget->visible )
        gui_widget_draw( widget );
}
void gui_progressbar_set_max( 
    GuiWidget *widget, int max )
{
    if ( widget->type != GUI_PROGRESSBAR ) return;
    /* set max */
    widget->spec.progressbar.max = max;
    gui_progressbar_adjust( widget );
    /* done */
    if ( widget->visible )
        gui_widget_draw( widget );
}
int gui_progressbar_get_value( 
    GuiWidget *widget, int *value )
{
    if ( widget->type != GUI_PROGRESSBAR ) return 0;
    *value = widget->spec.progressbar.value;
    return 1;
}
int gui_progressbar_get_max( 
    GuiWidget *widget, int *max )
{
    if ( widget->type != GUI_PROGRESSBAR ) return 0;
    *max = widget->spec.progressbar.max;
    return 1;
}


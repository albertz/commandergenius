/***************************************************************************
                          gui_button.c  -  description
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
#include "gui_button.h"

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
            /* the button surface provides for states in a 
               vertical order: normal, focused, clicked, 
               inactive */
            if ( !widget->active ) {
                stk_surface_blit( widget->surface, 
                    0, (widget->screen_region.h<<1) +
                    widget->screen_region.h, 
                    -1, widget->screen_region.h, 
                    stk_display,
                    widget->screen_region.x,
                    widget->screen_region.y );
                break;
            }
            if ( widget->focused ) {
                if ( widget->pressed ) 
                    stk_surface_blit( widget->surface, 
                        0, widget->screen_region.h<<1, 
                        -1, widget->screen_region.h, 
                        stk_display,
                        widget->screen_region.x,
                        widget->screen_region.y );
                else
                    stk_surface_blit( widget->surface, 
                        0, widget->screen_region.h, 
                        -1, widget->screen_region.h, 
                        stk_display,
                        widget->screen_region.x,
                        widget->screen_region.y );
            }
            else
                stk_surface_blit( widget->surface, 0, 0, 
                    -1, widget->screen_region.h, 
                    stk_display,
                    widget->screen_region.x,
                    widget->screen_region.y );
            break;
        case GUI_BUTTON_PRESSED:
            stk_sound_play( gui_theme->click_sound );
        case GUI_ACTIVATED:
        case GUI_DEACTIVATED:
        case GUI_BUTTON_RELEASED:
        case GUI_CLICKED:
        case GUI_FOCUS_IN:
        case GUI_FOCUS_OUT:
            gui_widget_draw( widget );
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
Create a button that may contain an icon and a text. Both are 
displayed centered.
====================================================================
*/
GuiWidget* gui_button_create( 
    GuiWidget *parent, int x, int y, int width, int height, 
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    SDL_Surface *icon, int ix, int iy, int iw, int ih,
    char *caption )
{
    int offset, i;
    GuiWidget *widget = gui_widget_create(
        parent, GUI_BUTTON, x, y, width, height, 
        default_event_handler, user_event_handler );
    /* events */
    gui_widget_enable_event( widget, GUI_CLICKED );
    /* basic surface */
    /* four different states in a vertical order:
       NORMAL: normal wallpaper + normal button frame
       FOCUSED: focus wallpaper + normal button frame
       CLICKED: focus wallpaper + clicked button frame
       INACTIVE: grayed NORMAL
    */
    widget->surface = stk_surface_create( 
        SDL_SWSURFACE, width, height * 4 );
    /* NORMAL */
    stk_surface_apply_wallpaper(
        widget->surface, 0,0,-1,height, 
        gui_theme->button_wallpaper_normal, -1 );
    SDL_SetColorKey( widget->surface, 0,0 );
    widget->border = stk_surface_apply_frame(
            widget->surface, 0,0,-1,height,
            gui_theme->button_frame_normal );
    /* FOCUSED */
    stk_surface_apply_wallpaper(
        widget->surface, 0,height,-1,height, 
        gui_theme->button_wallpaper_focused, -1 );
    stk_surface_apply_frame(
        widget->surface, 0,height,-1,height,
        gui_theme->button_frame_normal );
    /* CLICKED */
    stk_surface_apply_wallpaper(
        widget->surface, 
        widget->border, widget->border + 2 * height,
        -1,height, 
        gui_theme->button_wallpaper_focused, -1 );
    stk_surface_apply_frame(
        widget->surface, 0,2*height,-1,height,
        gui_theme->button_frame_clicked );
    /* INACTIVE */
    stk_surface_apply_wallpaper(
        widget->surface, 0,height*3,-1,height, 
        gui_theme->button_wallpaper_normal, -1 );
    stk_surface_apply_frame(
            widget->surface, 0,height*3,-1,height,
            gui_theme->button_frame_normal );
    stk_surface_gray( widget->surface,
        0, height*3, width, height, 0 );
    /* size w/o border */
    widget->width -= 2 * widget->border;
    widget->height -= 2 * widget->border;
    /* add label and icon */
    offset = width;
    if ( icon )
        offset -= iw;
    if ( caption )
        offset -= stk_font_string_width( 
                      gui_theme->button_font, caption );
    offset /= 2;
    /* icon */
    if ( icon ) {
        for ( i = 0; i < 4; i++ )
            stk_surface_alpha_blit( icon, ix, iy, iw, ih, 
                              widget->surface, 
                              offset + (i==2)*widget->border/2, 
                              i * height + (height - ih) / 2 +
                              (i==2)*widget->border/2, 
                              (i==3)?128:SDL_ALPHA_OPAQUE );
        offset += iw;
    }
    if ( caption ) {
        gui_theme->button_font->align = STK_FONT_ALIGN_CENTER_Y;
        for ( i = 0; i < 4; i++ )
            stk_font_write( gui_theme->button_font, 
                            widget->surface, 
                            offset + (i==2)*widget->border/2, 
                            i * height + height / 2 + 
                            (i==2)*widget->border/2, 
                            (i==3)?128:SDL_ALPHA_OPAQUE, caption );
    }
    /* done */
    return widget;
}

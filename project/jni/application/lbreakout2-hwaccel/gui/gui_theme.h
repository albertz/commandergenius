/***************************************************************************
                          gui_theme.h  -  description
                             -------------------
    begin                : Fri Oct 11 2002
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

#ifndef __GUI_THEME_H
#define __GUI_THEME_H

#include "stk.h"

/*
====================================================================
All widgets need a basic set of graphics and this is it.
It provides a wallpaper, a frame, a basic button, arrows for 
scrollbars etc. However it cannot be changed on runtime so
once a widget is completed it looks like it looks.
====================================================================
*/
typedef struct {
    SDL_Surface *box_wallpaper; /* wallpaper for box widget */
    SDL_Surface *widget_wallpaper; /* wallpaper for other widgets */
    SDL_Surface *box_frame; /* frame for box window */
    SDL_Surface *widget_frame; /* frame for other widgets */
    SDL_Surface *button_frame_normal,
                *button_frame_clicked; /* frames for the different
                                          button types */
    SDL_Surface *button_wallpaper_normal,
                *button_wallpaper_focused; /* button wallpapers */
    StkFont     *font; /* standard font */
    SDL_Surface *scrollbar_arrows; /* scrollbar arrows */
    int         scrollbar_arrow_size; /* arrows are square */
    SDL_Surface *checkbox; /* checkbox and checker */
    int         checkbox_size; /* checkboxes are square */
    SDL_Surface *progress_high,
                *progress_medium,
                *progress_low; /* three wallpapers for
                                  progressbar (25%,60%,100%) */
    SDL_Surface *list_item_color; /* color of the selection box */
    SDL_Surface *spinbutton_arrows; /* scrollbar arrows */
    int         spinbutton_arrow_size; /* arrows are square */
    StkFont     *button_font; /* font for button captions */
    StkFont     *label_font; /* font for labels */
    StkFont     *progressbar_font; 
    StkFont     *edit_font;
    /* sounds */
    StkSound    *click_sound;
    StkSound    *type_sound;
} GuiTheme;

/*
====================================================================
Load a theme from a theme directory. The surface path set by
stk_surface_set_path() is prepended.
====================================================================
*/
GuiTheme* gui_theme_load( char *name );

/*
====================================================================
Free the memory of a surface if not NULL and reset the pointer
to NULL.
====================================================================
*/
void gui_theme_delete( GuiTheme **theme );

/*
====================================================================
Select the current theme. New widgets will use its graphics.
====================================================================
*/
void gui_theme_select( GuiTheme *theme );

#endif


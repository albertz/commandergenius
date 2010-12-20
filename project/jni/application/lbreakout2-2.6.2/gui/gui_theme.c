/***************************************************************************
                          gui_theme.c  -  description
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

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "gui_theme.h"

#define ABORT( msg ) { fprintf( stderr, "%s\n", msg ); exit(1); }

GuiTheme *gui_theme = 0;

/*
====================================================================
Load a theme from a theme directory. The surface path set by
stk_surface_set_path() is prepended.
====================================================================
*/

GuiTheme* gui_theme_load( char *name )
{
    GuiTheme *theme = calloc( 1, sizeof( GuiTheme ) );
    if ( theme == 0 ) ABORT( "Out Of Memory" )

    theme->box_wallpaper = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/box_wallpaper.png", name );
    theme->widget_wallpaper = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/widget_wallpaper.png", name );
    theme->box_frame = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/box_frame.png", name );
    theme->widget_frame = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/widget_frame.png", name );
    theme->button_frame_normal = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/button_frame_normal.png", name );
    theme->button_frame_clicked = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/button_frame_clicked.png", name );
    theme->button_wallpaper_normal = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/button_wallpaper_normal.png", name );
    theme->button_wallpaper_focused = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/button_wallpaper_focused.png", name );
    theme->scrollbar_arrows = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/scrollbar_arrows.png", name );
    theme->scrollbar_arrow_size =
        theme->scrollbar_arrows->h;
    theme->checkbox = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/checkbox.png", name );
    theme->checkbox_size = theme->checkbox->h;
    theme->progress_high = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/progress_high.png", name );
    theme->progress_medium = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/progress_medium.png", name );
    theme->progress_low = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/progress_low.png", name );
    theme->list_item_color = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/list_item_color.png", name );
    theme->spinbutton_arrows = 
        stk_surface_load( SDL_SWSURFACE, 
            "%s/spinbutton_arrows.png", name );
    theme->spinbutton_arrow_size = 
        theme->spinbutton_arrows->h;

    theme->font =
        stk_font_load( SDL_SWSURFACE, "%s/font.png", name );
    theme->edit_font =
        stk_font_load( SDL_SWSURFACE, "%s/font.png", name );
    theme->label_font =
        stk_font_load( SDL_SWSURFACE, "%s/font.png", name );
    theme->button_font = theme->font;
    theme->progressbar_font = theme->font;
    
    theme->click_sound = 
        stk_sound_load( -1, "%s/click.wav", name );
    theme->type_sound = 
        stk_sound_load( -1, "%s/edit.wav", name );
    
    return theme;
}

/*
====================================================================
Free the memory of a surface if not NULL and reset the pointer
to NULL.
====================================================================
*/
void gui_theme_delete( GuiTheme **theme )
{
    if ( *theme ) {
        stk_surface_free( &(*theme)->box_wallpaper );
        stk_surface_free( &(*theme)->widget_wallpaper );
        stk_surface_free( &(*theme)->box_frame );
        stk_surface_free( &(*theme)->widget_frame );
        stk_surface_free( &(*theme)->button_frame_normal );
        stk_surface_free( &(*theme)->button_frame_clicked );
        stk_surface_free( &(*theme)->button_wallpaper_normal );
        stk_surface_free( &(*theme)->button_wallpaper_focused );
        stk_surface_free( &(*theme)->scrollbar_arrows );
        stk_surface_free( &(*theme)->checkbox );
        stk_surface_free( &(*theme)->progress_high );
        stk_surface_free( &(*theme)->progress_medium );
        stk_surface_free( &(*theme)->progress_low );
        stk_surface_free( &(*theme)->list_item_color );
        stk_surface_free( &(*theme)->spinbutton_arrows );
        stk_font_free( &(*theme)->font );
        stk_font_free( &(*theme)->edit_font );
        stk_font_free( &(*theme)->label_font );
        stk_sound_free( &(*theme)->click_sound );
        stk_sound_free( &(*theme)->type_sound );
        free( *theme ); *theme = 0;
    }
}

/*
====================================================================
Select the current theme. New widgets will use its graphics.
====================================================================
*/
void gui_theme_select( GuiTheme *theme )
{
    gui_theme = theme;
}


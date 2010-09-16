/***************************************************************************
                          gui_icon.h  -  description
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

#ifndef __GUI_ICON_H
#define __GUI_ICON_H

/*
====================================================================
Create an icon widget that displays the icon centered and uses
the parent's background.
====================================================================
*/
GuiWidget *gui_icon_create( 
    GuiWidget *parent, int x, int y, int width, int height, 
    int with_frame,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    SDL_Surface *icon, int ix, int iy, int iw, int ih );

/*
====================================================================
Apply a new icon.
====================================================================
*/
void gui_icon_set_icon( 
    GuiWidget *icon, SDL_Surface *source, 
    int ix, int iy, int iw, int ih );

#endif


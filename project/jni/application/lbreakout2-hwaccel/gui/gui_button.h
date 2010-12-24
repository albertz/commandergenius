/***************************************************************************
                          gui_button.h  -  description
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

#ifndef __GUI_BUTTON_H
#define __GUI_BUTTON_H

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
    char *caption );
    
#endif

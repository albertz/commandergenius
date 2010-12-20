/***************************************************************************
                          gui_box.h  -  description
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

#ifndef __GUI_BOX_H
#define __GUI_BOX_H

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
    int grab_input );
    
#endif


/***************************************************************************
                          gui_scrollbar.h  -  description
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

#ifndef __GUI_SCROLLBAR_H
#define __GUI_SCROLLBAR_H

/*
====================================================================
Create an either vertical or horizontal scrollbar. The increase/
decrease/track buttons are square.
====================================================================
*/
GuiWidget* gui_scrollbar_create(
    GuiWidget *parent, int x, int y, int width, int height,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    int vertical, int min, int max, int step, int jump, int value );

/*
====================================================================
Set or get scrollbar value. Get returns False if this was no 
scrollbar.
====================================================================
*/
void gui_scrollbar_set_value( GuiWidget *scrollbar, int value );
int gui_scrollbar_get_value( GuiWidget *scrollbar, int *value );
/*
====================================================================
Set the range of the scrollbar. Stores refresh rects if visible.
-1 for any value means to keep the current setting.
====================================================================
*/
int gui_scrollbar_set_range( 
    GuiWidget *scrollbar, int min, int max, int step, int jump );
/*
====================================================================
Modify value of scrollbar by the passed amount and update,
return True if it has changed and call event handler with
GUI_CHANGED.
====================================================================
*/
int gui_scrollbar_add( GuiWidget *widget, int change );

#endif


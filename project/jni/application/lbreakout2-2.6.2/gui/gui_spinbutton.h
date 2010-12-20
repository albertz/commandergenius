/***************************************************************************
                          gui_spinbutton.h  -  description
                             -------------------
    begin                : Sat Oct 19 2002
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

#ifndef __GUI_SPINBUTTON_H
#define __GUI_SPINBUTTON_H

/* maximum number of characters (minus/digits) in the edit */
enum { GUI_SPINBUTTON_LENGTH = 15 };

/*
====================================================================
Create a spinbutton which is an alphanumerical edit with an 
increase and a decrease button both the size of height x height/2
at the right end of the edit.
====================================================================
*/
GuiWidget* gui_spinbutton_create( 
    GuiWidget *parent, int x, int y, int width, int height, 
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    int min, int max, int step, int value );
    
/*
====================================================================
Get/set spinbutton's value. Get returns False if this was no 
spinbutton.
====================================================================
*/
void gui_spinbutton_set_value( GuiWidget *spinbutton, int value );
int gui_spinbutton_get_value( GuiWidget *spinbutton, int *value );
/*
====================================================================
Set the range of the spinbutton. Stores refresh rects if visible.
-1 for any value means to keep the current setting.
====================================================================
*/
int gui_spinbutton_set_range( 
    GuiWidget *spinbutton, int min, int max, int step );

/*
====================================================================
(De)Activate edit of spinbutton. If the entered value is greater
than the maximum it is set to maximum. If it's less than minimum
it is _not_ reset (else it would be impossible to enter new 
values) However, if the value is queried by 
gui_spinbutton_get_value() it is checked for its range.
====================================================================
*/
void gui_spinbutton_enable_edit( GuiWidget *spinbutton, int enable );

#endif


/***************************************************************************
                          gui_radiogroup.h  -  description
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

#ifndef __GUI_RADIOGROUP_H
#define __GUI_RADIOGROUP_H

/*
====================================================================
Create a radiogroup or a checkbox which is nothing but a
single entried radiogroup.
  'caption': checkbox caption
  'items': items - the widget does not modify the pointers
  'checked': is True if checkbox checked else False.
Radiogroup selections can be set with gui_radiogroup_select().  
====================================================================
*/
GuiWidget* gui_checkbox_create(
    GuiWidget *parent, int x, int y, int width, int height, 
    int with_frame,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    char *caption, int checked );
GuiWidget* gui_radiogroup_create(
    GuiWidget *parent, int x, int y, int width, int height, 
    int with_frame,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    int item_count, char **items, int min, int max );
    
/*
====================================================================
Set or check if a checkbox in the radiogroup is set.
Stores refresh rects if visible.
====================================================================
*/
void gui_radiogroup_set_checked( 
    GuiWidget *radiogroup, int id, int check );
void gui_radiogroup_toggle( GuiWidget *radiogroup, int id );
int gui_radiogroup_is_checked( GuiWidget *radiogroup, int id );
/*
====================================================================
For radiogroups with at max one items this returns the currently
checked id or -1.
====================================================================
*/
int gui_radiogroup_get_checked( GuiWidget *radiogroup );

#endif

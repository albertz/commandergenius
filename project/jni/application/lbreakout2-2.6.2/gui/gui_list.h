/***************************************************************************
                          gui_list.h  -  description
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

#ifndef __GUI_LIST_H
#define __GUI_LIST_H

/* select types */
enum {
    GUI_LIST_MULTI_SELECT = -1,
    GUI_LIST_NO_SELECT,
    GUI_LIST_SINGLE_SELECT
};

/*
====================================================================
Create a listbox that may have multiple columns which cannot be
scrolled however. 
  'gap': gap between items and frame
  'columns': the item width is determined by available_width/columns
  'item_height': the number of rows is unlimited and this is the
                 height of an item
  'scrollbar_width' : if > 0 a vertical scrollbar is provided
  'render_item': The scrollbar value (0 if none) tells which row
                 is the first displayed and 
                 render_item(row,column,buffer) is called to 
                 obtain the graphic for each visible item.
                 If 'render_item' returns False the item is not
                 displayed.
  'select_type': no, single, multi select
====================================================================
*/
GuiWidget* gui_list_create(
    GuiWidget *parent, int x, int y, int width, int height,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    int gap, int columns, int item_height,
    int scrollbar_width, int (*render_item)(int,int,SDL_Surface*),
    int select_type );

/*
====================================================================
Set item count of list and update it even if item count hasn't
changed. This function is used to tell the list that it
contents has changed and needs to be redrawn. All selections
are cleared by this function.
====================================================================
*/
void gui_list_update( GuiWidget *list, int count );
/*
====================================================================
Handle a listbox selection and update listbox.
If 'select' is 1 the item is selected, if it's 0 the item
is unselected, -1 means to toggle the value.
If SINGLE_SELECT toggeling the current selection keeps selection.
====================================================================
*/
void gui_list_select( 
    GuiWidget *list, int column, int row, int select );

/*
====================================================================
Display passed row as first one or jump to end if 'row' is -1 .
====================================================================
*/
void gui_list_goto( GuiWidget *list, int row );

#endif


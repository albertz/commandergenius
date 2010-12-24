/***************************************************************************
                          gui_edit.h  -  description
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

#ifndef __GUI_EDIT_H
#define __GUI_EDIT_H

/*
====================================================================
Create a single/multi-line editable widget. By checking the 
measurements of the standard font the number of lines and the 
line width is determined. If the edit is single-lined height is
always one but edit becomes scrollable.
  'border': between text and frame
  'text': is copied and displayed when first shown
  'size': limit of characters (excluding the \0)
Per default the edit accepts all non-whitespace characters.
====================================================================
*/
GuiWidget* gui_edit_create(
    GuiWidget *parent, int x, int y, int width, int height,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    int border, int multi_line, int size, char *text );

/*
====================================================================
Resize the edit buffer and clear any text. (no update)
====================================================================
*/
void gui_edit_resize_buffer( GuiWidget *edit, int size );
/*
====================================================================
Set the edit's text. This resets the edit cursor's position to
end of text and updates displayed edit if visible.
====================================================================
*/
void gui_edit_set_text( GuiWidget *edit, char *text );
/*
====================================================================
Copy characters from 'start' to 'length' of the edit string to 
'buffer' (at maximum limit characters including \0). If 'length' is 
-1 the characters copied are those from 'start' to end of text.
====================================================================
*/
int gui_edit_get_text( 
    GuiWidget *edit, char *buffer, int limit, 
    int start, int length );
/*
====================================================================
Update the blinking cursor flag (no update) of the edit and in 
case a key is pressed call gui_edit_handle_key().
====================================================================
*/
void gui_edit_update( GuiWidget *widget, int ms );
/*
====================================================================
Select a character filter.
  default: all non-whitespaces >=32 && <= 128
  alpha: A-Z,a-z
  numerical: -,0-9
  alphanumerical: A-Z,a-z,0-9
  alphanumerical2: + underscores
====================================================================
*/
enum {
    GUI_EDIT_DEFAULT = 0,
    GUI_EDIT_ALPHA,
    GUI_EDIT_NUMERICAL,
    GUI_EDIT_ALPHANUMERICAL,
    GUI_EDIT_ALPHANUMERICAL2
};
void gui_edit_set_filter( GuiWidget *edit, int type );

#endif

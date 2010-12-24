/***************************************************************************
                          gui_label.h  -  description
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

#ifndef __GUI_LABEL_H
#define __GUI_LABEL_H

/*
====================================================================
Create a label widget with the parent's background and maybe a 
frame. 
  'border': space between text and frame
  'text': text that may contain multiple lines separated by '#'
          (duplicated)
  'align': alignment for each line
  'font': used to display the text. if NULL is passed the default
          label font is used
====================================================================
*/
GuiWidget* gui_label_create(
    GuiWidget *parent, int x, int y, int width, int height, 
    int with_frame,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    int border, StkFont *font, int align, char *text );

/*
====================================================================
Set label font/alignment/text. Update if visible.
====================================================================
*/
void gui_label_set_font( GuiWidget *label, StkFont *font );
void gui_label_set_align( GuiWidget *label, int align );
void gui_label_set_text( GuiWidget *label, char *format, ... );

#endif

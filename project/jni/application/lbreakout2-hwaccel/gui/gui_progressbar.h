/***************************************************************************
                          gui_progressbar.h  -  description
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

#ifndef __GUI_PROGRESSBAR_H
#define __GUI_PROGRESSBAR_H

/*
====================================================================
Create a framed progress bar.
====================================================================
*/
GuiWidget *gui_progressbar_create( 
    GuiWidget *parent, int x, int y, int width, int height,
    void (*user_event_handler)(GuiWidget*,GuiEvent*),
    int max, int value );
    
/*
====================================================================
Get/Set value and limit of progressbar.
====================================================================
*/
void gui_progressbar_set_value( 
    GuiWidget *progressbar, int value );
void gui_progressbar_set_max( 
    GuiWidget *progressbar, int max );
int gui_progressbar_get_value( 
    GuiWidget *progressbar, int *value );
int gui_progressbar_get_max( 
    GuiWidget *widget, int *max );

#endif


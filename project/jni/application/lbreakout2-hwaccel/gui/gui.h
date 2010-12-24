/***************************************************************************
                          gui.h  -  description
                             -------------------
    begin                : Fri Oct 11 2002
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

#ifndef __GUI_H
#define __GUI_H

#include "gui_widget.h"
#include "gui_box.h"
#include "gui_label.h"
#include "gui_icon.h"
#include "gui_button.h"
#include "gui_radiogroup.h"
#include "gui_progressbar.h"
#include "gui_scrollbar.h"
#include "gui_edit.h"
#include "gui_list.h"
#include "gui_spinbutton.h"

/*
====================================================================
GUI
====================================================================
*/

/*
====================================================================
Initiate the GUI and load the theme either from stk's surface
path or an absolute position. This must be done before any widget 
is created. gui_init() installs gui_finalize() as exit function.
====================================================================
*/
void gui_init( char *theme_path );

/*
====================================================================
Finalize GUI and free any memory allocated by gui_init(). Also
delete all widgets that were not destroyed at this point.
====================================================================
*/
void gui_finalize( void );

/*
====================================================================
Handle SDL and time event for visible root widgets. 
If 'event' is not NULL and it is handled by one of the root widgets
True is returned or False otherwise. If a widget within this queue
has 'grab_input' set True 'event' is not handled by lower level 
widgets and True is returned.
'ms' is the amount of milliseconds passed since the last call from
gui_dispatch_event(). If 'ms' > 0 a GUI_TIME_PASSED event is sent
to all widgets that enabled the GUI_TIME_PASSED event.
The GUI_TIME_PASSED event is send regardless of 'grab_input'.
====================================================================
*/
int gui_dispatch_event( SDL_Event *event, int ms );

/*
====================================================================
Run all visible root widgets until the 'master' widget is closed.
If there is no root widget open gui_run() will immediately exit.
If 'master' is 0 the gui is run until all root widgets are closed.
====================================================================
*/
void gui_run( GuiWidget *master );

/*
====================================================================
Install a special event filter to improve usage of scrollbars.
gui_disable_event_filter() restores the previous filter.
====================================================================
*/
void gui_enable_event_filter();
void gui_disable_event_filter();

#endif

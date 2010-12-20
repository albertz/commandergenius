/***************************************************************************
                          gui_spinbutton.c  -  description
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

#include <stdlib.h>
#include <string.h>
#include "gui_widget.h"
#include "gui_spinbutton.h"
#include "gui_button.h"
#include "gui_edit.h"

extern GuiTheme *gui_theme;
extern SDL_Surface *stk_display;

/*
====================================================================
LOCALS
====================================================================
*/

/*
====================================================================
Default event handler
====================================================================
*/
static void default_event_handler( 
    GuiWidget *widget, GuiEvent *event )
{
    switch ( event->type ) {
        case GUI_DESTROY:
            break;
        case GUI_DRAW:
            break;
    }
}

/*
====================================================================
Handle edit and restrict input to digits and minus. Check range.
The GUI_CHANGED event from the edit is blocked if an invalid
character was added.
====================================================================
*/
static void gui_spinbutton_edit_handler(
    GuiWidget *widget, GuiEvent *event )
{
    char aux[GUI_SPINBUTTON_LENGTH + 1];
    int old_value;
    if ( event->type == GUI_CHANGED ) {
        old_value = widget->parent->spec.spinbutton.value;
        gui_edit_get_text( widget, aux, 
            GUI_SPINBUTTON_LENGTH + 1, 0, -1 );
        widget->parent->spec.spinbutton.value = atoi( aux );
        /* check if we hit upper limit */
        if ( widget->parent->spec.spinbutton.value > 
             widget->parent->spec.spinbutton.max ) {
            widget->parent->spec.spinbutton.value = 
                widget->parent->spec.spinbutton.max;
            snprintf( aux, GUI_SPINBUTTON_LENGTH + 1,
                "%i", widget->parent->spec.spinbutton.value );
            gui_edit_set_text( widget, aux );
        }
        if ( old_value != widget->parent->spec.spinbutton.value )
            gui_widget_call_user_event_handler( 
                widget->parent, event );
    }
    else
        gui_widget_call_user_event_handler( 
            widget->parent, event );
}
/*
====================================================================
Handle increase/decrease buttons.
====================================================================
*/
static void gui_spinbutton_inc_handler(
    GuiWidget *widget, GuiEvent *event )
{
    if ( event->type == GUI_CLICKED )
    if ( widget->parent->spec.spinbutton.value < 
         widget->parent->spec.spinbutton.max ) {
        widget->parent->spec.spinbutton.value += 
            widget->parent->spec.spinbutton.step;
        if ( widget->parent->spec.spinbutton.value > 
             widget->parent->spec.spinbutton.max )
            widget->parent->spec.spinbutton.value = 
                widget->parent->spec.spinbutton.max;
        gui_spinbutton_set_value( widget->parent, 
            widget->parent->spec.spinbutton.value );
        gui_widget_call_user_event_handler( 
            widget->parent, 
            gui_event_get_simple( GUI_CHANGED ) );
    }
    gui_widget_call_user_event_handler( widget->parent, event );
}
static void gui_spinbutton_dec_handler(
    GuiWidget *widget, GuiEvent *event )
{
    if ( event->type == GUI_CLICKED )
    if ( widget->parent->spec.spinbutton.value > 
         widget->parent->spec.spinbutton.min ) {
        widget->parent->spec.spinbutton.value -= 
            widget->parent->spec.spinbutton.step;
        if ( widget->parent->spec.spinbutton.value <
             widget->parent->spec.spinbutton.min )
            widget->parent->spec.spinbutton.value = 
                widget->parent->spec.spinbutton.min;
        gui_spinbutton_set_value( widget->parent, 
            widget->parent->spec.spinbutton.value );
        gui_widget_call_user_event_handler( 
            widget->parent, 
            gui_event_get_simple( GUI_CHANGED ) );
    }
    gui_widget_call_user_event_handler( widget->parent, event );
}

/*
====================================================================
PUBLICS
====================================================================
*/

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
    int min, int max, int step, int value )
{
    GuiWidget *widget = gui_widget_create(
        parent, GUI_SPINBUTTON, x, y, width, height, 
        default_event_handler, user_event_handler );
    /* events */
    gui_widget_enable_event( widget, GUI_CHANGED );
    /* build empty surface */
    widget->surface = stk_surface_create( 
        SDL_SWSURFACE, width, height );
    /* set range */
    widget->spec.spinbutton.min = min;
    widget->spec.spinbutton.max = max;
    widget->spec.spinbutton.step = step;
    /* build edit */
    widget->spec.spinbutton.edit = gui_edit_create( 
        widget, 0, 0, width - height, height, 
        gui_spinbutton_edit_handler, 2, 0, 
        GUI_SPINBUTTON_LENGTH, "undefined" );
    gui_edit_set_filter( 
        widget->spec.spinbutton.edit, GUI_EDIT_NUMERICAL );
    gui_widget_disable_event( 
        widget->spec.spinbutton.edit, GUI_FOCUS_IN );
    gui_widget_disable_event( 
        widget->spec.spinbutton.edit, GUI_FOCUS_OUT );
    /* by default edit can't be edited */
    widget->spec.spinbutton.edit->active = 0;
    /* build buttons */
    widget->spec.spinbutton.inc = gui_button_create(
        widget, width - height, 0, height, height / 2,
        gui_spinbutton_inc_handler,
        gui_theme->spinbutton_arrows, 0,0,
        gui_theme->spinbutton_arrow_size,
        gui_theme->spinbutton_arrow_size, 0 );
    gui_widget_disable_event( 
        widget->spec.spinbutton.inc, GUI_FOCUS_IN );
    gui_widget_disable_event( 
        widget->spec.spinbutton.inc, GUI_FOCUS_OUT );
    widget->spec.spinbutton.dec = gui_button_create(
        widget, width - height, height/2, height, height / 2,
        gui_spinbutton_dec_handler,
        gui_theme->spinbutton_arrows, 
        gui_theme->spinbutton_arrow_size,0,
        gui_theme->spinbutton_arrow_size,
        gui_theme->spinbutton_arrow_size, 0 );
    gui_widget_disable_event( 
        widget->spec.spinbutton.dec, GUI_FOCUS_IN );
    gui_widget_disable_event( 
        widget->spec.spinbutton.dec, GUI_FOCUS_OUT );
    /* set value */
    gui_spinbutton_set_value( widget, value );
    /* done */
    return widget;
}

/*
====================================================================
Get/set spinbutton's value. Get returns False if this was no 
spinbutton.
====================================================================
*/
void gui_spinbutton_set_value( GuiWidget *widget, int value )
{
    char aux[GUI_SPINBUTTON_LENGTH + 1];
    if ( widget->type == GUI_SPINBUTTON ) {
        widget->spec.spinbutton.value = value;
        snprintf( aux, GUI_SPINBUTTON_LENGTH + 1, "%i", value );
        gui_edit_set_text( widget->spec.spinbutton.edit, aux );
    }
}
int gui_spinbutton_get_value( GuiWidget *widget, int *value )
{
    if ( widget->type != GUI_SPINBUTTON )
        return 0;
    *value = widget->spec.spinbutton.value;
    if ( *value < widget->spec.spinbutton.min )
        *value = widget->spec.spinbutton.min;
    if ( *value > widget->spec.spinbutton.max )
        *value = widget->spec.spinbutton.max;
    return 1;
}
/*
====================================================================
Set the range of the spinbutton. Stores refresh rects if visible.
-1 for any value means to keep the current setting.
====================================================================
*/
int gui_spinbutton_set_range( 
    GuiWidget *widget, int min, int max, int step )
{
    if ( widget->type != GUI_SPINBUTTON )
        return 0;
    if ( min != -1 ) widget->spec.spinbutton.min = min;
    if ( max != -1 ) widget->spec.spinbutton.max = max;
    if ( step != -1 ) widget->spec.spinbutton.step = step;
    return 1;
}

/*
====================================================================
(De)Activate edit of spinbutton. If the entered value is greater
than the maximum it is set to maximum. If it's less than minimum
it is _not_ reset (else it would be impossible to enter new 
values) However, if the value is queried by 
gui_spinbutton_get_value() it is checked for its range.
====================================================================
*/
void gui_spinbutton_enable_edit( GuiWidget *widget, int enable )
{
    widget->spec.spinbutton.edit->active = enable;
}

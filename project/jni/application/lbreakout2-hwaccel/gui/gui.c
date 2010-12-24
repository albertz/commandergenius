/***************************************************************************
                          gui.c  -  description
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

#include <stdlib.h>
#include <string.h>
#include "gui.h"

extern int stk_quit_request;
extern GuiTheme *gui_theme;
extern GuiWidget *gui_clicked_widget;
extern GuiWidget *gui_focused_widget;
extern GuiWidget *gui_key_widget;

/*
====================================================================
This is a list of all root widgets (no parent) created. It is used
to delete all widgets in gui_finalize().
====================================================================
*/
List *gui_root_stack = 0;
/*
====================================================================
This is a list of all visible root widgets updated by 
gui_widget_show(). The upper most root widget will handle 
events first.
====================================================================
*/
List *gui_visible_stack = 0;
/*
====================================================================
This is a list of all widgets that enabled the GUI_TIME_PASSED
event on creation.
====================================================================
*/
List *gui_timed_stack = 0;


/*
====================================================================
LOCALS
====================================================================
*/

/*
====================================================================
Event filter for mouse motion. Update 'gui_motion_event' with
the latest motion and pass just the very first motion event to
the queue (so we know that it occured)
====================================================================
*/
SDL_EventFilter gui_old_event_filter = 0;
int gui_uses_event_filter = 0;
SDL_Event gui_sdl_motion_event;
int gui_event_filter( const SDL_Event *event )
{
    int xrel, yrel;
    if ( event->type == SDL_MOUSEMOTION ) {
        if ( gui_sdl_motion_event.type == SDL_NOEVENT ) {
            /* first occurence */
            gui_sdl_motion_event = *event;
        }
        else {
            /* already queued */
            xrel = gui_sdl_motion_event.motion.xrel; 
            yrel = gui_sdl_motion_event.motion.yrel;
            gui_sdl_motion_event = *event;
            gui_sdl_motion_event.motion.xrel += xrel;
            gui_sdl_motion_event.motion.yrel += yrel;
            return 0;
        }
    }
    return 1;
}

/*
====================================================================
Check all visible root widgets and update the focus by
assuming the mouse pointer at the position of the passed
GUI_MOTION event which was not necessarily initiated from
an SDL event thus gui_update_focus() can be used to 
update the focus if a widget grabbed focus on button pressed.
====================================================================
*/
void gui_update_focus( GuiEvent *gui_event )
{
    ListEntry *entry;
    GuiWidget *widget = 0;
    GuiWidget *next, *new_focus = 0;
    /* check all root widgets for new focus */
    new_focus = 0;
    entry = gui_visible_stack->tail->prev;
    while ( entry != gui_visible_stack->head ) {
        widget = (GuiWidget*)entry->item;
        /* clear widget->focused_widget */
        if ( ( next = widget->focused_widget ) ) {
            while ( next ) {
                next->focused = 0;
                next = next->parent;
            }
            widget->focused_widget = 0;
        }
        /* get new focused_widget */
        gui_widget_update_focus( widget, 
            gui_event->motion.x, gui_event->motion.y,
            &widget->focused_widget );
        /* if we have a focused widget this is either
           our current or a new focus */
        if ( widget->focused_widget ) {
            new_focus = widget->focused_widget;
            break;
        }
        /* if 'grab_input' is set we don't check
           lower root widgets */
        if ( widget->grab_input )
            break;
        /* next root widget */
        entry = entry->prev;
    }
    /* check relation new_focus/gui_focused_widget */
    if ( new_focus == gui_focused_widget ) {
        if ( !new_focus ) /* no focus at all */
            return;
        /* unchanged focus thus motion event */
        gui_widget_handle_event( 
            gui_focused_widget, gui_event );
    }
    else {
        if ( gui_focused_widget ) {
            /* focus out event */
            gui_event->type = GUI_FOCUS_OUT;
            next = gui_focused_widget;
            while ( next ) {
                next->focused = 0;
                next = next->parent;
            }
            gui_widget_handle_event( 
                gui_focused_widget, gui_event );
            gui_focused_widget = 0;
        }
        if ( new_focus ) {
            /* focus in event */
            gui_focused_widget = new_focus;
            gui_event->type = GUI_FOCUS_IN;
            gui_widget_handle_event( 
                gui_focused_widget, gui_event );
        }
    }
}
        
/*
====================================================================
Handle an SDL event for the visible root widgets and store all 
nescessary update rects. If the event was handled by a widget
the user_event_handler is called if one has been installed. 
Returns True if the event was handled by the GUI.
====================================================================
*/
static int gui_handle_event( SDL_Event *sdl_event )
{
    GuiWidget *widget = 0;
    GuiEvent gui_event;
    
    switch ( sdl_event->type ) {
        case SDL_QUIT:
            stk_quit_request = 1;
            return 1;
        case SDL_KEYUP:
        case SDL_KEYDOWN:
            /* a key event is either processed by the 
               gui_key_widget or gui_focused_widget */
            if ( gui_key_widget )
                widget = gui_key_widget;
            else
                widget = gui_focused_widget;
            if ( widget ) {
                gui_event_init( &gui_event, sdl_event );
                gui_widget_handle_event( widget, &gui_event );
                return 1;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            /* if our widget is focused update gui_key_widget */
            if ( !gui_focused_widget ) break;
            gui_clicked_widget = gui_focused_widget;
            widget = gui_focused_widget;
            widget->pressed = 1;
            if ( widget->grab_keys ) {
                if ( gui_key_widget )
                    gui_key_widget->default_event_handler(
                        gui_key_widget, 
                        gui_event_get_simple( GUI_FOCUS_OUT ) );
                gui_key_widget = widget;
            }
            gui_event_init( &gui_event, sdl_event );
            gui_widget_handle_event( widget, &gui_event );
            return 1;
        case SDL_MOUSEBUTTONUP:
            /* if clicked_widget grabbed focus we need to
               update the focus by a fake motion event. */
            if ( gui_clicked_widget )
            if ( gui_clicked_widget->pressed )
            if ( gui_clicked_widget->grab_focus ) {
                memset( &gui_event, 0, sizeof( gui_event ) );
                gui_event.type = GUI_MOTION;
                gui_event.motion.x = sdl_event->button.x;
                gui_event.motion.y = sdl_event->button.y;
                /* we don't set buttons and relative position
                   in this fake event so this might cause errors
                   if the 'focus grabbing' widget requires
                   these values. however at this point the
                   only case we need 'grab_focus' is for
                   scrollbar track buttons. so if this ever
                   changes don't forget to drop by. ;-) */
                gui_update_focus( &gui_event );
            }
            /* if our focused widget has 'pressed' True
               this becomes a CLICKED event. clear 'pressed' 
               flag in any case*/
            if ( !gui_focused_widget ) {
                if ( gui_clicked_widget )
                    gui_clicked_widget->pressed = 0;
                break;
            }
            gui_event_init( &gui_event, sdl_event );
            widget = gui_focused_widget;
            if ( widget->pressed )
                gui_event.type = GUI_CLICKED;
            else
                gui_event.type = GUI_BUTTON_RELEASED;
            if ( gui_clicked_widget )
                gui_clicked_widget->pressed = 0;
            gui_widget_handle_event( widget, &gui_event );
            return 1;
        case SDL_MOUSEMOTION:
            /* if we use the improved event filter check for
               the latest motion event in gui_motion_event */
            if ( gui_uses_event_filter ) {
                gui_event_init( &gui_event, &gui_sdl_motion_event );
                gui_sdl_motion_event.type = SDL_NOEVENT;
            }
            else
                gui_event_init( &gui_event, sdl_event );
            /* if clicked widget grabs input do not update the
               focus but pass the event directly to clicked_widget */
            if ( gui_clicked_widget )
            if ( gui_clicked_widget->pressed )
            if ( gui_clicked_widget->grab_focus ) {
                gui_widget_handle_event( gui_clicked_widget, &gui_event );
                break;
            }
            /* update the focus and sent FOCUS_IN, FOCUS_OUT, 
               MOTION events */
            gui_update_focus( &gui_event );
            break;
    }
    return 0; /* grab input later */
}

/*
====================================================================
PUBLICS
====================================================================
*/

/*
====================================================================
Initiate the GUI and load the theme either from stk's surface
path or an absolute position. This must be done before any widget 
is created. gui_init() installs gui_finalize() as exit function.
====================================================================
*/
void gui_init( char *theme_path )
{
    /* load theme */
    gui_theme = gui_theme_load( theme_path );
    /* all root widgets created. the list is not AUTO_DELETE
       to allow direct usage of gui_widget_delete() as well. */
    gui_root_stack = list_create( 
        LIST_NO_AUTO_DELETE, LIST_NO_CALLBACK );
    /* visible stack stores pointers to existing root widgets */
    gui_visible_stack = list_create( 
        LIST_NO_AUTO_DELETE, LIST_NO_CALLBACK );
    /* timed stack stores pointers to existing widgets */
    gui_timed_stack = list_create( 
        LIST_NO_AUTO_DELETE, LIST_NO_CALLBACK );
    /* install gui_finalize() */
    atexit( gui_finalize );
}

/*
====================================================================
Finalize GUI and free any memory allocated by gui_init(). Also
delete all widgets that were not destroyed at this point.
====================================================================
*/
void gui_finalize( void )
{
    GuiWidget *widget;
    if ( gui_root_stack ) {
        list_reset( gui_root_stack );
        while ( ( widget = list_next( gui_root_stack ) ) )
            gui_widget_delete( &widget );
        list_delete( gui_root_stack );
        gui_root_stack = 0;
    }
    if ( gui_visible_stack ) {
        list_delete( gui_visible_stack );
        gui_visible_stack = 0;
    }
    if ( gui_timed_stack ) {
        list_delete( gui_timed_stack );
        gui_timed_stack = 0;
    }
    gui_theme_delete( &gui_theme );
    printf( "GUI finalized\n" );
}

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

WARNING: If a root widget closes itself it's okay but if the 
previous widget in visible_stack is closed the entry pointer 
becomes invalid when handling the SDL event. FIXME
====================================================================
*/
int gui_dispatch_event( SDL_Event *sdl_event, int ms )
{
    ListEntry *entry;
    GuiEvent gui_event;
    GuiWidget *widget;
    int ret = 0;
    /* timer event */
    if ( ms > 0 ) {
        /* of all edits only one needs to blink it's
           cursor and handle a pressed key so we by-pass
           the clean way of raising a GUI_TIME_PASSED
           event and update this edit directly. yes, this
           is a hack but it speeds up dispatching and is
           within the GUI. no one is hurt. */
        widget = gui_key_widget;
        if ( !widget ) widget = gui_focused_widget;
        if ( widget && widget->type == GUI_EDIT )
            gui_edit_update( widget, ms );
        /* GUI_TIME_PASSED event */
        gui_event.type = GUI_TIME_PASSED;
        gui_event.time.ms = ms;
        entry = gui_timed_stack->head->next;
        while ( entry != gui_timed_stack->tail ) {
            widget = (GuiWidget*)entry->item;
            if ( !widget->visible ) continue;
            gui_widget_call_user_event_handler( 
                widget, &gui_event );
            entry = entry->next;
        }
    }
    /* sdl event */
    if ( sdl_event ) 
        ret = gui_handle_event( sdl_event );
    return ret;
}

/*
====================================================================
Run all visible root widgets until the 'master' widget is closed.
If there is no root widget open gui_run() will immediately exit.
If 'master' is 0 the gui is run until all root widgets are closed.
====================================================================
*/
void gui_run( GuiWidget *master )
{
    SDL_Event event;
    int ms;
    
    stk_timer_reset();
    while ( 1 ) {
        /* break conditions */
        if ( stk_quit_request ) break;
        if ( master ) {
            if ( !master->visible ) break; }
        else 
            if ( gui_visible_stack->count == 0 ) break;
        /* get time */
        ms = stk_timer_get_time();
        /* handle event */
        if ( SDL_PollEvent( &event ) )
            gui_dispatch_event( &event, ms );
        else
            gui_dispatch_event( 0, ms );
        /* update display */
        stk_display_update( STK_UPDATE_RECTS );
        /* wait some time */
        SDL_Delay( 5 );
    }
}

/*
====================================================================
Install a special event filter to improve usage of scrollbars.
gui_disable_event_filter() restores the previous filter.
====================================================================
*/
void gui_enable_event_filter()
{
    /* already in use? */
    if ( SDL_GetEventFilter() == gui_event_filter )
        return;
    /* install */
    gui_old_event_filter = SDL_GetEventFilter();
    SDL_SetEventFilter( gui_event_filter );
    gui_sdl_motion_event.type = SDL_NOEVENT;
    gui_uses_event_filter = 1;
}
void gui_disable_event_filter()
{
    SDL_SetEventFilter( gui_old_event_filter );
    gui_uses_event_filter = 0;
}

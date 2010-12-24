/***************************************************************************
                          gui_widget.c  -  description
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
#include "gui_widget.h"

extern GuiTheme *gui_theme;
extern SDL_Surface *stk_display;
extern List *gui_root_stack;
extern List *gui_visible_stack;
extern List *gui_timed_stack;

GuiWidget *gui_key_widget = 0;
    /* is the last child widget of _all_ visible key grabbing
       widgets that was clicked on */
GuiWidget *gui_clicked_widget = 0;
    /* is the last child widget of _all_ visible widgets 
       that was clicked on (used to clear 'pressed' flag) */
GuiWidget *gui_focused_widget = 0;
    /* the very toplevel widget of all visible widgets 
       the mouse pointer hovers above */
       
/*
====================================================================
LOCALS
====================================================================
*/

/*
====================================================================
Free the memory of a widget and all its subwidgets.
====================================================================
*/
static void gui_widget_delete_intern( GuiWidget *widget )
{
    GuiWidget *child;
    if ( widget ) {
        /* go recursive */
        list_reset( widget->widgets );
        while ( ( child = list_next( widget->widgets ) ) )
            gui_widget_delete_intern( child );
        /* free common data */
        list_delete( widget->widgets );
        stk_surface_free( &widget->surface );
        /* free special data */
        widget->default_event_handler( 
            widget, gui_event_get_simple( GUI_DESTROY ) );
        /* widget itself */
        free( widget );
    }
}

/*
====================================================================
Draw the widget and its children. gui_widget_draw() disabled
the display monitoring and will add the widgets screen_region
as update rectangle.
====================================================================
*/
static void gui_widget_draw_intern( GuiWidget *widget )
{
    GuiWidget *child = 0;
    /* draw widget */
    widget->default_event_handler( 
        widget, gui_event_get_simple( GUI_DRAW ) );
    /* draw children */
    list_reset( widget->widgets );
    while ( ( child = list_next( widget->widgets ) ) )
        gui_widget_draw_intern( child );
}

/*
====================================================================
Set visible flag to 'visible' for widget and all children
recursively.
====================================================================
*/
static void gui_widget_show_intern( GuiWidget *widget, int visible )
{
    GuiWidget *child;
    widget->visible = visible;
    list_reset( widget->widgets );
    while ( ( child = list_next( widget->widgets ) ) )
        gui_widget_show_intern( child, visible );
}

/*
====================================================================
If there are any root windows above this widget in the stack that 
overlap this one they need to be redrawn. We clip to the widgets 
screen region and redraw all root widgets above.
====================================================================
*/
static void gui_widget_redraw_overlapping_roots( 
    GuiWidget *widget )
{
    ListEntry *entry;
    if ( gui_visible_stack->count > 1 )
    if ( list_last( gui_visible_stack ) != widget->root ) {
        /* clip */
        stk_surface_clip( stk_display, 
            widget->screen_region.x, widget->screen_region.y,
            widget->screen_region.w, widget->screen_region.h );
        /* get first root widget above */
        entry = list_entry( 
            gui_visible_stack, widget->root );
        if ( !entry ) {
            fprintf( stderr, 
                "'visible_stack' corrupted: root widget not found\n" );
            return;
        }
        entry = entry->next;
        /* redraw */
        while ( entry != gui_visible_stack->tail ) {
            gui_widget_draw_intern( (GuiWidget*)entry->item );
            entry = entry->next;
        }
        /* unclip */
        stk_surface_clip( stk_display, 0,0,-1,-1 );
    }
}

/*
====================================================================
Move position by relative value for all widgets.
====================================================================
*/
static void gui_widget_move_intern( 
    GuiWidget *widget, int rel_x, int rel_y)
{
    GuiWidget *child;
    widget->screen_region.x += rel_x;
    widget->screen_region.y += rel_y;
    widget->parent_region.x += rel_x;
    widget->parent_region.y += rel_y;
    list_reset( widget->widgets );
    while ( ( child = list_next( widget->widgets ) ) )
        gui_widget_move_intern( child, rel_x, rel_y );
}

/*
====================================================================
PUBLICS
====================================================================
*/

/*
====================================================================
GUI events
====================================================================
*/

/*
====================================================================
Return pointer to simple event (one that doesn't need
additional data)
====================================================================
*/
static GuiEvent aux_event;
GuiEvent *gui_event_get_simple( int type )
{
    aux_event.type = type;
    return &aux_event;
}

/*
====================================================================
Initiate a GUI event from an SDL event.
====================================================================
*/
void gui_event_init( GuiEvent *gui_event, SDL_Event *sdl_event )
{
    int i;
    memset( gui_event, 0, sizeof( GuiEvent ) );
    switch ( sdl_event->type ) {
        case SDL_MOUSEMOTION:
            gui_event->type = GUI_MOTION;
            gui_event->motion.x = sdl_event->motion.x;
            gui_event->motion.y = sdl_event->motion.y;
            gui_event->motion.xrel = sdl_event->motion.xrel;
            gui_event->motion.yrel = sdl_event->motion.yrel;
            gui_event->motion.state = sdl_event->motion.state;
            for ( i = 1; i <= 3; i++ ) 
                if ( sdl_event->motion.state & SDL_BUTTON(i) ) {
                    gui_event->motion.button = i;
                    break;
                }
            break;
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            if ( sdl_event->type == SDL_MOUSEBUTTONUP )
                gui_event->type = GUI_BUTTON_RELEASED;
            else
                gui_event->type = GUI_BUTTON_PRESSED;
            gui_event->button.x = sdl_event->button.x;
            gui_event->button.y = sdl_event->button.y;
            gui_event->button.button = sdl_event->button.button;
            break;
        case SDL_KEYUP:
        case SDL_KEYDOWN:
            if ( sdl_event->type == SDL_KEYUP )
                gui_event->type = GUI_KEY_RELEASED;
            else
                gui_event->type = GUI_KEY_PRESSED;
            gui_event->key.keysym = sdl_event->key.keysym.sym;
            gui_event->key.unicode = sdl_event->key.keysym.unicode;
            break;
    }
}

/*
====================================================================
GUI widget
====================================================================
*/

/*
====================================================================
Create a basic widget and setup things all different widget types
have in common. If a parent is specified this widget is added to 
it's 'widgets' list. Per default following events are enabled:
GUI_ACTIVATED, GUI_DEACTIVATED, GUI_DRAW, GUI_DESTROY, 
GUI_FOCUS_IN, GUI_FOCUS_OUT. 'x' or 'y' -1 means to center the
widget.
====================================================================
*/
GuiWidget* gui_widget_create( 
    GuiWidget *parent, int type, 
    int x, int y, int width, int height,
    void (*default_event_handler)(GuiWidget*,GuiEvent*),
    void (*user_event_handler)(GuiWidget*,GuiEvent*) )
{
    GuiWidget *widget = calloc( 1, sizeof( GuiWidget ) );
    if ( widget == 0 )
        GUI_ABORT( "Out Of Memory" )
    /* create empty children list */
    if ( ( widget->widgets = 
               list_create( LIST_NO_AUTO_DELETE, 
                            LIST_NO_CALLBACK ) ) == 0 )
        GUI_ABORT( "Out Of Memory" )
    /* various assignments */
    if ( parent ) {
        widget->parent = parent;
        list_add( widget->parent->widgets, widget );
    }
    widget->type = type;
    widget->root = (parent==0)?widget:parent->root;
    widget->active = 1;
    widget->default_event_handler = default_event_handler;
    widget->user_event_handler = user_event_handler;
    /* adjust x,y */
    if ( x == -1 ) {
        if ( parent )
            x = ( parent->width - width ) / 2;
        else
            x = ( stk_display->w - width ) / 2;
    }
    if ( y == -1 ) {
        if ( parent )
            y = ( parent->height - height ) / 2;
        else
            y = ( stk_display->h - height ) / 2;
    }
    /* region in parent */
    if ( parent == 0 ) {
        widget->parent_region.x = x;
        widget->parent_region.y = y;
    }
    else {
        widget->parent_region.x = x + widget->parent->border;
        widget->parent_region.y = y + widget->parent->border;
    }
    widget->parent_region.w = width;
    widget->parent_region.h = height;
    /* screen region */
    if ( widget->parent == 0 )
        widget->screen_region = widget->parent_region;
    else {
        widget->screen_region.x = 
            widget->parent_region.x + parent->screen_region.x;
        widget->screen_region.y =
            widget->parent_region.y + parent->screen_region.y;
        widget->screen_region.w = widget->parent_region.w;
        widget->screen_region.h = widget->parent_region.h;
    }
    /* children size */
    widget->width = width; widget->height = height;
    /* events */
    gui_widget_enable_event( widget, GUI_ACTIVATED );
    gui_widget_enable_event( widget, GUI_DEACTIVATED );
    gui_widget_enable_event( widget, GUI_DRAW );
    gui_widget_enable_event( widget, GUI_DESTROY );
    gui_widget_enable_event( widget, GUI_FOCUS_IN );
    gui_widget_enable_event( widget, GUI_FOCUS_OUT );
    /* if this is a root widget add it to root_stack */
    if ( widget->root == widget )
        list_add( gui_root_stack, widget );
    /* done */
    return widget;
}

/*
====================================================================
This function will delete a root widget including all subwidgets.
Subwidgets can't be directly deleted. Resets the widget 
pointer to NULL.
====================================================================
*/
void gui_widget_delete( GuiWidget **widget )
{
    if ( *widget == 0 ) return;
    if ( (*widget)->root != *widget ) {
        fprintf( stderr, "You can only delete root widgets!\n" );
        return;
    }
    if ( (*widget)->visible ) 
        list_delete_item( gui_visible_stack, *widget );
    if ( (*widget)->event_mask & GUI_TIME_PASSED )
        list_delete_item( gui_timed_stack, *widget );
    list_delete_item( gui_root_stack, *widget );
    gui_widget_delete_intern( *widget );
    *widget = 0;
}


/*
====================================================================
If button is deactivated no input events (key,button,motion)
are handled.
====================================================================
*/
void gui_widget_set_active( GuiWidget *widget, int active )
{
    int type;
    if ( widget->active == active ) return;
    widget->active = active;
    type = (active)?GUI_ACTIVATED:GUI_DEACTIVATED;
    widget->default_event_handler( 
        widget, gui_event_get_simple( type ) );
    gui_widget_call_user_event_handler( 
        widget, gui_event_get_simple( type ) );
}

/*
====================================================================
Draw the widget and its children if visible.
====================================================================
*/
void gui_widget_draw( GuiWidget *widget )
{
    /* update only if visible */
    if ( !widget->visible ) return;
    /* recursively draw widget */
    gui_widget_draw_intern( widget );
    /* redraw higher level roots that overlap this widget. */
    gui_widget_redraw_overlapping_roots( widget );
    /* store update rect */
    stk_display_store_rect( &widget->screen_region );
}

/*
====================================================================
Set 'visible' flag and draw widget (store update rects)
if either parent is visible or it has no parent. 
(thus is a root window). If it is a root window add it to the
root window stack. This new window will handle incoming events
first. 
====================================================================
*/
void gui_widget_show( GuiWidget *widget )
{
    if ( widget->visible ) return;
    if ( widget->root == widget ) {
        list_add( gui_visible_stack, widget );
        if ( widget->event_mask & GUI_TIME_PASSED )
            list_add( gui_timed_stack, widget ); 
#ifdef GUI_DEBUG
        printf( "show root widget: %i,%i,%i,%i\n", 
                widget->screen_region.x, widget->screen_region.y,
                widget->screen_region.w, widget->screen_region.h );
#endif
        /* if this root widget grabs the input the old 
           gui_key_widget/gui_clicked_widget is obsolete */
        if ( widget->grab_input ) {
            if ( gui_key_widget ) {
                gui_key_widget->default_event_handler(
                    gui_key_widget, 
                    gui_event_get_simple( GUI_FOCUS_OUT ) );
                gui_key_widget = 0;
            }
            gui_clicked_widget = 0;
        }
        /* maybe there is a default key grab widget? */
        if ( widget->default_key_widget )
            gui_key_widget = widget->default_key_widget;
    }
    gui_widget_show_intern( widget, 1 );
    if ( widget->parent == 0 || widget->parent->visible )
        gui_widget_draw( widget );
}

/*
====================================================================
Clear 'visible' flag and restore widget if parent is visible.
If there is no parent (thus is a root window) remove it from
stack and redraw the underlying window (which regains control). If 
a root widget is hidden the background cannot be restored
as it is unknown.
====================================================================
*/
void gui_widget_hide( GuiWidget *widget )
{
    ListEntry *entry;
    if ( !widget->visible ) return;
    gui_widget_show_intern( widget, 0 );
    if ( widget->root == widget ) {
        entry = list_entry( gui_visible_stack, widget );
        if ( entry ) {
            /* remove */
            entry = entry->next;
            list_delete_entry( 
                gui_visible_stack, entry->prev );
            /* FIX ME! redraw all open roots */
            entry = gui_visible_stack->head->next;
            while ( entry != gui_visible_stack->tail ) {
                gui_widget_draw( (GuiWidget*)entry->item );
                entry = entry->next;
            }
        }
        if ( widget->event_mask & GUI_TIME_PASSED )
            list_delete_item( gui_timed_stack, widget ); 
    }
    else {
        /* draw parent's background */
        stk_surface_blit( widget->parent->surface,
            widget->parent_region.x, widget->parent_region.y,
            widget->parent_region.w, widget->parent_region.h,
            stk_display, 
            widget->screen_region.x, widget->screen_region.y );
        /* redraw higher level roots that overlap this widget. */
        gui_widget_redraw_overlapping_roots( widget );
        /* store update rect */
        stk_display_store_rect( &widget->screen_region );
    }
    /* check if gui_key_widget is still valid */
    if ( gui_key_widget )
    if ( widget == gui_key_widget || 
         widget == gui_key_widget->parent ||
         widget == gui_key_widget->root )
        gui_key_widget = 0;
}

/*
====================================================================
Modify the event mask of a widget to define which events will
be passed to user_event_handler.
====================================================================
*/
void gui_widget_enable_event( GuiWidget *widget, int event )
{
    widget->event_mask |= (1L << event);
}
void gui_widget_disable_event( GuiWidget *widget, int event )
{
    widget->event_mask &= ~(1L << event);
}

/*
====================================================================
Pass GuiEvent to user defined callback if it has been installed
and the event mask flag is True for this event.
====================================================================
*/
void gui_widget_call_user_event_handler(
    GuiWidget *widget, GuiEvent *event )
{
    if ( widget->user_event_handler )
        if ( widget->event_mask & (1L << event->type) )
            widget->user_event_handler( widget, event );
}

/*
====================================================================
Handle the GUI event by calling the default_event_handler()
and the user_event_handler() if one has been installed.
====================================================================
*/
void gui_widget_handle_event( GuiWidget *widget, GuiEvent *event )
{
    widget->default_event_handler( widget, event );
    gui_widget_call_user_event_handler( widget, event );
}

/*
====================================================================
Move widget within parent window by a relative value. If the 
widget is visible the changes will be drawn to screen.
====================================================================
*/
void gui_widget_move( GuiWidget *widget, int rel_x, int rel_y )
{
    if ( !widget->visible )
        gui_widget_move_intern( widget, rel_x, rel_y );
    else {
        gui_widget_hide( widget );
        gui_widget_move_intern( widget, rel_x, rel_y );
        gui_widget_show( widget );
    }
}

/*
====================================================================
Move widget within parent window by an absolute value. If the 
widget is visible the changes will be drawn to screen.
====================================================================
*/
void gui_widget_warp( GuiWidget *widget, int abs_x, int abs_y )
{
    if ( widget->parent )
        gui_widget_move( widget,
            abs_x - widget->parent_region.x + 
                widget->parent->border,
            abs_y - widget->parent_region.y + 
                widget->parent->border );
    else
        gui_widget_move( widget,
            abs_x - widget->parent_region.x,
            abs_y - widget->parent_region.y );
}

/*
====================================================================
Apply parents background or wallpaper within the frame (if
any) of the widget's surface keeping the frame.
====================================================================
*/
void gui_widget_apply_wallpaper( 
    GuiWidget *widget, SDL_Surface *wallpaper, int alpha )
{
    if ( widget->parent )
        stk_surface_blit( widget->parent->surface,
                          widget->parent_region.x +
                          widget->border, 
                          widget->parent_region.y + 
                          widget->border, 
                          widget->parent_region.w -
                          (widget->border<<1), 
                          widget->parent_region.h -
                          (widget->border<<1), 
                          widget->surface, 
                          widget->border, widget->border );
    else
        stk_surface_apply_wallpaper(
            widget->surface, widget->border, widget->border, 
            widget->parent_region.w - (widget->border<<1), 
            widget->parent_region.h - (widget->border<<1), 
            wallpaper, alpha );
}

/*
====================================================================
Browse the widget tree and set 'focused' true for all widgets
that have the mouse pointer above them. 'focused_widget'
returns the deepest widget that is focused.
====================================================================
*/
void gui_widget_update_focus( 
    GuiWidget *widget, int mx, int my, GuiWidget **focused_widget )
{
    GuiWidget *child;
    if ( !widget->active )
        return;
    if ( !STK_IN_RECT( widget->screen_region, mx, my ) )
        return;
    widget->focused = 1;
    *focused_widget = widget;
    /* handle children recursively */
    list_reset( widget->widgets );
    while ( ( child = list_next( widget->widgets ) ) )
        gui_widget_update_focus( child, mx, my, focused_widget );
}

/*
====================================================================
Get direct access to widget's surface.
====================================================================
*/
SDL_Surface *gui_widget_get_surface( GuiWidget *widget )
{
    return widget->surface;
}

/*
====================================================================
That key grabbing child of a root widget.
====================================================================
*/
void gui_widget_set_default_key_widget( 
    GuiWidget *root, GuiWidget *key_widget )
{
    if ( root->root != root ) return;
    root->default_key_widget = key_widget;
}

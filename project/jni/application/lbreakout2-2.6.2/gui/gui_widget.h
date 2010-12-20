/***************************************************************************
                          gui_widget.h  -  description
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

#ifndef __GUI_WIDGET_H
#define __GUI_WIDGET_H

#include "list.h"
#include "stk.h"
#include "gui_theme.h"

//#define GUI_DEBUG

#define GUI_ABORT( msg ) \
 { fprintf( stderr, "Fatal GUI Error!\n%s\n", msg ); exit( 1 ); }
 
/*
====================================================================
GUI events
====================================================================
*/

/* types */
enum {
    GUI_NONE = 0,
    GUI_DESTROY,            /* widget is deleted */
    GUI_ACTIVATED,
    GUI_DEACTIVATED,        /* handle _input_ events or not */
    GUI_DRAW,               /* draw widget to stk_display */
    GUI_TIME_PASSED,        /* msecs since last TIME_PASSED event */
    GUI_FOCUS_IN,           /* widget lost focus */
    GUI_FOCUS_OUT,          /* widget gained focus */
    GUI_MOTION,             /* motion within widget */
    GUI_BUTTON_PRESSED,     
    GUI_BUTTON_RELEASED,    /* mouse button */
    GUI_KEY_PRESSED,
    GUI_KEY_RELEASED,       /* key */
    GUI_CLICKED,            /* mouse button pressed + released */
    GUI_CHANGED,            /* type-dependent data changed */
    GUI_ITEM_SELECTED,      
    GUI_ITEM_UNSELECTED     /* list item (un)selected */
};

/* event */
typedef union {
    int type;
    struct {
        int type;
        int x, y, button;
    } button;
    struct {
        int type;
        int x, y, xrel, yrel;
        int button; /* first button pressed */
        int state; /* full button mask */
    } motion;
    struct {
        int type;
        int keysym;
        int unicode;
    } key;
    struct {
        int type;
        int x, y;
    } item;
    struct {
        int type;
        int ms;
    } time;
} GuiEvent;

/*
====================================================================
Return pointer to simple event (one that doesn't need
additional data)
====================================================================
*/
GuiEvent *gui_event_get_simple( int type );

/*
====================================================================
Initiate a GUI event from an SDL event.
====================================================================
*/
void gui_event_init( GuiEvent *gui_event, SDL_Event *sdl_event );

/*
====================================================================
GUI widget
====================================================================
*/

/* types */
enum {
    GUI_BOX = 0,
    GUI_BUTTON,
    GUI_LABEL,
    GUI_ICON,
    GUI_PROGRESSBAR,
    GUI_RADIOGROUP,
    GUI_SCROLLBAR,
    GUI_EDIT,
    GUI_LIST,
    GUI_SPINBUTTON
};

/* GUI widget - 
   for simplicity only one is used for all types */
typedef struct _GuiWidget {
    struct _GuiWidget *parent; /* widget's parent */
    struct _GuiWidget *root;   /* widget's root (top parent) */
    List *widgets;             /* widget's children */
    struct _GuiWidget *focused_widget; /* recursivly in widgets */
    struct _GuiWidget *default_key_widget; 
        /* child 'default_key_widget' will grab key input if this
           is a root widget that is shown */
    int  type;       /* button, label, icon ... */
    int  visible;    /* gui_widget_draw() updates screen */
    int  active;     /* accept events */
    int  focused;    /* mouse pointer somewhere in widget */
    int  grab_input; /* deny event handling to lower roots */
    int  grab_keys;  /* grab key input if clicked */
    int  grab_focus; /* grab focus IF PRESSED thus motion events
                        are directly passed to widgets handler.
                        the focus is not updated until mouse button
                        is released again. */
    int  pressed;    /* memorize 'button_pressed' event */
    SDL_Rect screen_region; /* absolute region in screen (clipped) */
    SDL_Rect parent_region; /* relative region in parent */
    /* CALLBACKS */
    void (*default_event_handler)(struct _GuiWidget*,GuiEvent*); 
        /* handles event and updates a widget (graphics etc) */
    void (*user_event_handler)(struct _GuiWidget*,GuiEvent*); 
        /* user's possibility to react on event. is NOT called
           from the default_event_handler but from 
           gui_widget_handle_event() itself. */
    /* USED BY ALL WIDGETS BUT TYPE DEPENDANT */
    int  border;        /* size of frame around widget */
    int  width, height; /* widget's size w/o border */
    int  event_mask;    /* events passed to user's event_handler */
    SDL_Surface *surface; /* picture of widget */
    /* SPECS */
    union {
        /* LABEL */
        struct {
            StkFont *font; 
            int     align; /* alignment of text */
            char    *text; /* text */
        } label;
        /* PROGRESSBAR */
        struct {
            int max;    /* maximum value */
            int value;  /* current value */
            int length; /* current length */
            SDL_Surface *wallpaper; /* current beam wallpaper */
        } progressbar;
        /* RADIOGROUP */
        struct {
            int min; /* minimum selections required */
            int max; /* minimum selections allowed */
            int size; /* number of items */
            int *checks;      /* 'checked' flag for each item */
            int check_count;  /* number of selections */
            int single_check; /* id of last item selected */
            int x, y;   /* position of first checkbox in parent */
            int offset; /* offset from one item to next */
        } radiogroup;
        /* SCROLLBAR */
        struct {
            int vertical;    /* vertical scrollbar? */
            int button_size; /* size of (square) buttons */
            int value;       /* current value */
            int min, max;    /* range of value */
            int step;        /* offset for up/down */
            int jump;        /* offset for pgup/pgdown */
            struct _GuiWidget *inc;
            struct _GuiWidget *dec; /* pointers to 'widgets' */
            struct _GuiWidget *track; /* special widget that is NOT
                                         in the 'widgets' list */
        } scrollbar;
        /* EDIT */
        struct {
            int  filter[SDLK_LAST]; /* characters accepted */
            int  multi_line; /* single-line edit or text area? */
            int  size;    /* character limit */
            char *buffer; /* string of edit */
            char *display_buffer; /* contains 'height' lines of 
                length 'width' separated by \0 which will be 
                displayed when drawing. is rendered by 
                gui_edit_adjust_cursor() */
            int  length; /* current string length */
            int  width;  /* characters per line */
            int  height; /* number of lines */
            int  x, y;   /* position in visible characters */
            int  y_offset; /* used to center single-line edits */
            int  pos;    /* position of edit cursor */
            int  start;  /* first character displayed */
            int  line;   /* first line displayed (start/width) */
        } edit;
        /* LIST */
        struct {
            int columns;        /* (fixed) number of columns */
            int item_width;
            int item_height;    /* item size */
            int gap;            /* space between items */
            int rows_per_page;  /* number of rows displayed */
            int item_count;     /* number of items */
            int rows;           /* number of rows */
            int select_type;    /* no, single, multi select */
            int *checks; /* NO_SELECT:     unused
                            SINGLE_SELECT: id of selected item
                            MULTI_SELECT:  flags for all items 
                                           whether they are 
                                           selected or not */
            SDL_Surface 
                *render_buffer; /* 'render_item' renders item 
                                    into this surface*/
            int (*render_item)(int,int,SDL_Surface*); /* user
                defined render callback to render item x,y into
                surface. This item is only displayed
                if 'render_item' returns True. */
            struct _GuiWidget  *scrollbar; /* pointer to 
                                              'widgets' */
        } list;
        /* SPINBUTTON */
        struct {
            int       min, max, step; /* range of value */
            int       value;          /* value */
            struct _GuiWidget *edit;  /* pointer to edit */
            struct _GuiWidget *inc;
            struct _GuiWidget *dec;   /* pointer to buttons */
        } spinbutton;
    } spec;
} GuiWidget;

/*
====================================================================
Create a basic widget and setup things all different widget types
have in common. If a parent is specified this widget is added to 
it's 'widgets' list. 'x' or 'y' -1 means to center the
widget.
====================================================================
*/
GuiWidget* gui_widget_create( 
    GuiWidget *parent, int type, 
    int x, int y, int width, int height,
    void (*default_event_handler)(GuiWidget*,GuiEvent*),
    void (*user_event_handler)(GuiWidget*,GuiEvent*) );

/*
====================================================================
This function will delete a root widget including all subwidgets.
Subwidgets can't be directly deleted. Resets the widget 
pointer to NULL.
====================================================================
*/
void gui_widget_delete( GuiWidget **widget );

/*
====================================================================
If button is deactivated no input events (key,button,motion)
are handled.
====================================================================
*/
void gui_widget_set_active( GuiWidget *widget, int active );

/*
====================================================================
Draw the widget and its children if visible.
====================================================================
*/
void gui_widget_draw( GuiWidget *widget );

/*
====================================================================
Set 'visible' flag and draw widget (store update rects)
if either parent is visible or it has no parent. 
(thus is a root window). If it is a root window add it to the
root window stack. This new window will handle incoming events
first. 
====================================================================
*/
void gui_widget_show( GuiWidget *widget );

/*
====================================================================
Clear 'visible' flag and restore widget if parent is visible.
If there is no parent (thus is a root window) remove it from
stack and redraw the underlying window (which regains control). If 
a root widget is hidden the background cannot be restored
as it is unknown.
====================================================================
*/
void gui_widget_hide( GuiWidget *widget );

/*
====================================================================
Modify the event mask of a widget to define which events will
be passed to user_event_handler. Update timed_stack if 
GUI_TIME_PASSED is enabled/disabled.
====================================================================
*/
void gui_widget_enable_event( GuiWidget *widget, int event );
void gui_widget_disable_event( GuiWidget *widget, int event );

/*
====================================================================
Pass GuiEvent to user defined callback if it has been installed
and the event mask flag is True for this event.
====================================================================
*/
void gui_widget_call_user_event_handler(
    GuiWidget *widget, GuiEvent *event );
    
/*
====================================================================
Handle the GUI event by calling the default_event_handler()
and the user_event_handler() if one has been installed.
====================================================================
*/
void gui_widget_handle_event( GuiWidget *widget, GuiEvent *event );

/*
====================================================================
Move widget within parent window by a relative value. If the 
widget is visible the changes will be drawn to screen.
====================================================================
*/
void gui_widget_move( GuiWidget *widget, int rel_x, int rel_y );

/*
====================================================================
Move widget within parent window by an absolute value. If the 
widget is visible the changes will be drawn to screen.
====================================================================
*/
void gui_widget_warp( GuiWidget *widget, int abs_x, int abs_y );

/*
====================================================================
Apply parents background or wallpaper within the frame (if
any) of the widget's surface.
====================================================================
*/
void gui_widget_apply_wallpaper( 
    GuiWidget *widget, SDL_Surface *wallpaper, int alpha );
    
/*
====================================================================
Browse the widget tree and set 'focused' true for all widgets
that have the mouse pointer above them. 'focused_widget'
returns the deepest widget that is focused.
====================================================================
*/
void gui_widget_update_focus( 
    GuiWidget *widget, int mx, int my, GuiWidget **focused_widget );
    
/*
====================================================================
Get direct access to widget's surface.
====================================================================
*/
SDL_Surface *gui_widget_get_surface( GuiWidget *widget );

/*
====================================================================
That key grabbing child of a root widget.
====================================================================
*/
void gui_widget_set_default_key_widget( 
    GuiWidget *root, GuiWidget *key_widget );

#endif

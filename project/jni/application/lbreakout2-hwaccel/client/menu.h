/***************************************************************************
                          menu.h  -  description
                             -------------------
    begin                : Thu Sep 20 2001
    copyright            : (C) 2001 by Michael Speck
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

#ifndef __MENU_H
#define __MENU_H

/*
====================================================================
Menu
====================================================================
*/
enum { MENU_LAYOUT_CENTERED = 0 };
typedef struct _Menu {
    char    *name;              /* name of menu -- duplicated */
    int     layout;
    int     x, y, w, h;         /* layout */
    List    *items;             /* list of menu items */
    Item    *cur_item;          /* currently highlighted item */
    struct _Menu   *parent;     /* parent menu */
    StkFont    *font, *hfont, *cfont; /* if not set the default fonts are used */
} Menu;

/*
====================================================================
Create menu and center it in x,y,w,h by menu_adjust() later.
====================================================================
*/
Menu *menu_create( char *name, int layout, int x, int y, int w, int h, int border, int add_to_list );
/*
====================================================================
Delete menu and all submenus starting with root menu
====================================================================
*/
void menu_delete( void *menu );
/*
====================================================================
Add item to menu. Add item::menu to children list if
ITEM_LINK.
====================================================================
*/
void menu_add( Menu *menu, Item *item );
/*
====================================================================
Adjust position and size of all entries according to layout.
====================================================================
*/
void menu_adjust( Menu *menu );
/*
====================================================================
Select menu: clear current item and set extern variable cur_menu
====================================================================
*/
void menu_select( Menu *menu );
/*
====================================================================
Show/hide all items
====================================================================
*/
void menu_hide( Menu *menu );
void menu_show( Menu *menu );
/*
====================================================================
modify items according to event (if any) and return action id.
====================================================================
*/
int menu_handle_event( Menu *menu, SDL_Event *event );
/*
====================================================================
Update alpha of items
====================================================================
*/
void menu_update( Menu *menu, int ms );
/*
====================================================================
Select/unselect item: update menu::cur_item und item::highlight
====================================================================
*/
void menu_unselect_cur_item( Menu *menu );
void menu_select_item( Menu *menu, Item *item );
/*
====================================================================
Go one valid menu item up or down.
====================================================================
*/
void menu_up( Menu *menu );
void menu_down( Menu *menu );
/*
====================================================================
Set fonts of menu and items.
====================================================================
*/
void menu_set_fonts( 
    Menu *menu, StkFont *cfont, StkFont *font, StkFont *hfont );
/*
====================================================================
Set background of menu and items.
====================================================================
*/
void menu_set_bkgnd( Menu *menu, SDL_Surface *bkgnd );

#endif

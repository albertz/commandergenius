/***************************************************************************
                          item.h  -  description
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

#ifndef __ITEM_H
#define __ITEM_H

#include "hint.h"

/*
====================================================================
Item of menu
====================================================================
*/
enum {
    ITEM_SEPARATOR = 0,
    ITEM_SWITCH,
    ITEM_SWITCH_X,
    ITEM_RANGE,
    ITEM_KEY,
    ITEM_EDIT,
    ITEM_LINK,
    ITEM_ACTION
};
typedef struct {
    Value   *value;         /* value of item */
    int     type;           /* type as listed above */
    char    *name;          /* name of item -- duplicated */
    int     item_id;        /* id of item (returned as action if ITEM_ACTION) */
    int     x, y, w, h;     /* position and size in screen */
    int     nx, ny;         /* position where name is drawn */
    int     nalign;         /* align of name: either left-aligned
                               or centered (only if link to another menu) */
    int     vx, vy;         /* position where value string is drawn right-aligned */
    int     valign;         /* alignment of value string */
    float   alpha;          /* alpha of normal font (which vanishes when selected) */
    float   halpha;         /* alpha of highlight font */
    int     highlighted;    /* keep alpha at 0 (will raise to 255 if not set) */
    void    (*callback)();  /* if value of item has been modified this functions is called */
    void    *link;          /* menu link */
    Font    *font;          /* if set use this font instead of standard font */
    Hint    *hint;          /* if not NULL this hint is displayed when quick-help's enabled:
                               created by item itself */
} Item;

/*
====================================================================
Create item.
Return Value: item
====================================================================
*/
Item *item_create_separator( char *name );
Item *item_create_range(     char *name, char *hint, int  *val_int, int min, int max, int step );
Item *item_create_switch(    char *name, char *hint, int  *val_int, char *str_off, char *str_on );
Item *item_create_switch_x(  char *name, char *hint, int  *val_int, char **strings, int count );
Item *item_create_key(       char *name, char *hint, int  *val_int, int *filter );
Item *item_create_edit(      char *name, char *hint, char *val_str, int limit );
Item *item_create_link(      char *name, char *hint, void *menu );
Item *item_create_action(    char *name, char *hint, int item_id );
/*
====================================================================
Delete item (void pointer for compatiblity when using with list)
====================================================================
*/
void item_delete( void *item );
/*
====================================================================
Adjust alignment of name and value strings
====================================================================
*/
void item_adjust( Item *item );
/*
====================================================================
Hide/Show item
====================================================================
*/
void item_hide( Item *item );
void item_show( Item *item );
/*
====================================================================
Update alpha of item
====================================================================
*/
void item_update_alpha( Item *item, int ms );
/*
====================================================================
Check if position's on item.
====================================================================
*/
int item_focus( Item *item, int x, int y );

#endif

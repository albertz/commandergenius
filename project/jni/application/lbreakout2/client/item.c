/***************************************************************************
                          item.c  -  description
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

#include "manager.h"

extern SDL_Surface *stk_display;
extern SDL_Surface *mbkgnd;
extern StkFont *mfont, *mhfont;

/*
====================================================================
Locals
====================================================================
*/

float alpha_change = 0.4;

/*
====================================================================
Create a basic item.
====================================================================
*/
Item *item_create_basic( int type, char *name, char *hint )
{
    Item *item = calloc( 1, sizeof( Item ) );
    /* name */
    item->name = strdup( name );
    /* id */
    item->item_id = -1;
    /* alpha */
    item->halpha = 0;
    item->alpha = 255;
    /* type */
    item->type = type;
    /* hint if  astring was passed */
    if ( hint == 0 )
        item->hint = 0;
    else
        item->hint = hint_create( 0, 0, hint ); /* the position will be set when this item is adjusted in menu adjust */
    /* fonts */
    item->font = mfont;
    item->hfont = mhfont;
    /* background */
    item->bkgnd = mbkgnd;
    return item;
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Create item.
Return Value: item
====================================================================
*/
Item *item_create_separator( char *name )
{
    Item *item = item_create_basic( ITEM_SEPARATOR, name, 0 );
    return item;
}
Item *item_create_range(     char *name, char *hint, int  *val_int, int min, int max, int step )
{
    Item *item = item_create_basic( ITEM_RANGE, name, hint );
    item->value = value_create_range_int( val_int, min, max, step );
    return item;
}
Item *item_create_switch(    char *name, char *hint, int  *val_int, char *str_off, char *str_on )
{
    char **names = calloc( 2, sizeof( char* ) );
    Item *item = item_create_basic( ITEM_SWITCH, name, hint );
    names[0] = strdup( str_off );
    names[1] = strdup( str_on );
    item->value = value_create_range_str( val_int, names, 2 );
    free( names[0] ); free( names[1] ); free( names );
    return item;
}
Item *item_create_switch_x(  char *name, char *hint, int  *val_int, char **strings, int count )
{
    Item *item = item_create_basic( ITEM_SWITCH_X, name, hint );
    item->value = value_create_range_str( val_int, strings, count );
    return item;
}
Item *item_create_key(       char *name, char *hint, int  *val_int, int *filter )
{
    Item *item = item_create_basic( ITEM_KEY, name, hint );
    item->value = value_create_key( val_int, filter );
    return item;
}
Item *item_create_edit(      char *name, char *hint, char *val_str, int limit )
{
    Item *item = item_create_basic( ITEM_EDIT, name, hint );
    item->value = value_create_edit( val_str, limit );
    return item;
}
Item *item_create_link(      char *name, char *hint, void *menu )
{
    Item *item = item_create_basic( ITEM_LINK, name, hint );
    item->link = menu;
    return item;
}
Item *item_create_action(    char *name, char *hint, int item_id )
{
    Item *item = item_create_basic( ITEM_ACTION, name, hint );
    item->item_id = item_id;
    return item;
}
/*
====================================================================
Delete item (void pointer for compatiblity when using with list)
====================================================================
*/
void item_delete( void *pitem )
{
    Item *item = (Item*)pitem;
    if ( !item ) return;
    if ( item->name ) free( item->name );
    if ( item->value ) value_delete( item->value );
    if ( item->hint ) hint_delete( item->hint );
    free( item );
}
/*
====================================================================
Adjust alignment of name and value strings
====================================================================
*/
void item_adjust( Item *item )
{
    /* alignment - name */
    item->nx = 0; item->ny = item->h / 2; item->nalign = STK_FONT_ALIGN_LEFT | STK_FONT_ALIGN_CENTER_Y;
    if ( item->type == ITEM_LINK || item->type == ITEM_ACTION || item->type == ITEM_SEPARATOR ) {
        item->nx = item->w / 2;
        item->nalign = STK_FONT_ALIGN_CENTER_X | STK_FONT_ALIGN_CENTER_Y;
    }
    item->vx = item->w - 1; item->vy = item->h / 2; item->valign = STK_FONT_ALIGN_RIGHT | STK_FONT_ALIGN_CENTER_Y;
    /* hint position */
    if ( item->hint )
        hint_set_pos( item->hint, item->x + item->w * 0.6, item->y );
}
/*
====================================================================
Hide/Show item
====================================================================
*/
void item_hide( Item *item )
{
    stk_surface_blit( item->bkgnd, 
        item->x, item->y, item->w, item->h,
        stk_display, item->x, item->y );
    stk_display_store_drect();
}
void item_show( Item *item )
{
    /* name */
    item->font->align = item->nalign;
    item->hfont->align = item->nalign;
    stk_font_write( item->font, stk_display, 
        item->x + item->nx, item->y + item->ny, 
        item->alpha, item->name );
    if ( item->halpha > 0 )
        stk_font_write( item->hfont, stk_display, 
            item->x + item->nx, item->y + item->ny, 
            item->halpha, item->name );
    stk_display_store_drect();
    /* value string */
    if ( item->value ) {
        item->font->align = item->valign;
        item->hfont->align = item->valign;
        /* Is this a selected edit item?  */
        if (item->highlighted && (item->type == ITEM_EDIT))
        {
            write_text_with_cursor( item->font, stk_display, 
                item->x + item->vx, item->y + item->vy, 
                item->value->val_str, item->alpha );
            if ( item->halpha > 0 )
                write_text_with_cursor( item->hfont, stk_display, 
                    item->x + item->vx, item->y + item->vy, 
                    item->value->val_str, item->halpha );
        }
        else
        {
            stk_font_write( item->font, stk_display, 
                item->x + item->vx, item->y + item->vy, 
                item->alpha, item->value->val_str );
            if ( item->halpha > 0 )
                stk_font_write( item->hfont, stk_display, 
                    item->x + item->vx, item->y + item->vy, 
                    item->halpha, item->value->val_str );
        }
        stk_display_store_drect();
    }
}
/*
====================================================================
Update alpha of item
====================================================================
*/
void item_update_alpha( Item *item, int ms )
{
    if ( item->highlighted ) {
        item->halpha = 255;
        item->alpha = 0;
    }
    else {
        if ( item->halpha > 0 ) {
            item->halpha -= alpha_change * ms;
            if ( item->halpha < 0 ) item->halpha = 0;
        }
        if ( item->alpha < 255 ) {
            item->alpha += alpha_change * ms;
            if ( item->alpha > 255 ) item->alpha = 255;
        }
    }
}
/*
====================================================================
Check if position's on item.
====================================================================
*/
int item_focus( Item *item, int x, int y )
{
    if ( item->type == ITEM_SEPARATOR ) return 0; /* may never be focused */
    return ( x >= item->x && y >= item->y && x < item->x + item->w && y < item->y + item->h );
}

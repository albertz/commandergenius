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

extern Sdl sdl;
extern SDL_Surface *mbkgnd;
extern Font *mfont, *mhfont;

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
    item->halpha = 255;
    item->alpha = 0;
    /* type */
    item->type = type;
    /* hint if  astring was passed */
    if ( hint == 0 )
        item->hint = 0;
    else
        item->hint = hint_create( 0, 0, hint ); /* the position will be set when this item is adjusted in menu adjust */
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
    item->nx = 0; item->ny = item->h / 2; item->nalign = ALIGN_X_LEFT | ALIGN_Y_CENTER;
    if ( item->type == ITEM_LINK || item->type == ITEM_ACTION || item->type == ITEM_SEPARATOR ) {
        item->nx = item->w / 2;
        item->nalign = ALIGN_X_CENTER | ALIGN_Y_CENTER;
    }
    item->vx = item->w - 1; item->vy = item->h / 2; item->valign = ALIGN_X_RIGHT | ALIGN_Y_CENTER;
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
    DEST( sdl.screen, item->x, item->y, item->w, item->h );
    SOURCE( mbkgnd, item->x, item->y );
    blit_surf();
    add_refresh_rect( item->x, item->y, item->w, item->h );
}
void item_show( Item *item )
{
    Font *font = mfont;
    /* name */
    if ( item->font ) font = item->font; /* manual override if special font was set */
    font->align = item->nalign;
    mhfont->align = item->nalign;
    write_text( font, sdl.screen, item->x + item->nx, item->y + item->ny, item->name, item->alpha );
    if ( item->halpha < 255 )
        write_text( mhfont, sdl.screen, item->x + item->nx, item->y + item->ny, item->name, item->halpha );
    /* value string */
    if ( item->value ) {
        font->align = item->valign;
        mhfont->align = item->valign;
        write_text( font, sdl.screen, item->x + item->vx, item->y + item->vy, item->value->val_str, item->alpha );
        if ( item->halpha < 255 )
            write_text( mhfont, sdl.screen, item->x + item->vx, item->y + item->vy, item->value->val_str, item->halpha );
    }
    add_refresh_rect( item->x, item->y, item->w, item->h );
}
/*
====================================================================
Update alpha of item
====================================================================
*/
void item_update_alpha( Item *item, int ms )
{
    if ( item->highlighted ) {
        item->halpha = 0;
        item->alpha = 255;
    }
    else {
        if ( item->halpha < 255 ) {
            item->halpha += alpha_change * ms;
            if ( item->halpha > 255 ) item->halpha = 255;
        }
        if ( item->alpha > 0 ) {
            item->alpha -= alpha_change * ms;
            if ( item->alpha < 0 ) item->alpha = 0;
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

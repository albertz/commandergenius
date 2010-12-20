/***************************************************************************
                          list.c  -  description
                             -------------------
    begin                : Sun Sep 2 2001
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

#include <stdlib.h>

#include "list.h"

/*
====================================================================
Create a new list
  auto_delete:  Free memory of data pointer when deleting entry
  callback:     Use this callback to free memory of data including
                the data pointer itself.
Return Value: List pointer
====================================================================
*/
List *list_create( int auto_delete, void (*callback)(void*) )
{
    List *list = calloc( 1, sizeof( List ) );
    list->head.next = &list->tail;
    list->head.prev = &list->head;
    list->tail.next = &list->tail;
    list->tail.prev = &list->head;
    list->auto_delete = auto_delete;
    list->callback = callback;
    list->cur_entry = &list->head;
    return list;
}
/*
====================================================================
Delete list and entries.
====================================================================
*/
void list_delete( List *list )
{
    list_clear( list );
    free( list );
}
/*
====================================================================
Delete all entries but keep the list. Reset current_entry to head
pointer.
====================================================================
*/
void list_clear( List *list )
{
    while( !list_empty( list ) ) list_delete_pos( list, 0 );
}
/*
====================================================================
Insert new item at position.
Return Value: True if successful else False.
====================================================================
*/
int list_insert( List *list, void *item, int pos )
{
    int         i;
    List_Entry    *cur = &list->head;
    List_Entry    *new_entry = 0;

    /* check if insertion possible */
    if ( pos < 0 || pos > list->count ) return 0;
    if ( item == 0 ) return 0;
    /* get to previous entry */
    for (i = 0; i < pos; i++) cur = cur->next;
    /* create and anchor new entry */
    new_entry = calloc( 1, sizeof( List_Entry ) );
    new_entry->item = item;
    new_entry->next = cur->next;
    new_entry->prev = cur;
    cur->next->prev = new_entry;
    cur->next = new_entry;
    list->count++;
    return 1;
}
/*
====================================================================
Add new item at the end of the list.
====================================================================
*/
int list_add( List *list, void *item )
{
    List_Entry    *new_entry = 0;
    /* check if insertion possible */
    if ( item == 0 ) return 0;
    /* create and anchor new entry */
    new_entry = calloc( 1, sizeof( List_Entry ) );
    new_entry->item = item;
    new_entry->next = &list->tail;
    new_entry->prev = list->tail.prev;
    list->tail.prev->next = new_entry;
    list->tail.prev = new_entry;
    list->count++;
    return 1;
}
/*
====================================================================
Delete item at position. If this was the current entry update
current_entry to valid previous pointer.
Return Value: True if successful else False.
====================================================================
*/
int list_delete_pos( List *list, int pos )
{
    int         i;
    List_Entry  *cur = &list->head;

    /* check if deletion possbile */
    if ( list_empty( list ) ) return 0;
    if ( pos < 0 || pos >= list->count ) return 0;
    /* get to correct entry */
    for ( i = 0; i <= pos; i++ ) cur = cur->next;
    /* modify anchors */
    cur->next->prev = cur->prev;
    cur->prev->next = cur->next;
    /* decrease counter */
    list->count--;
    /* check current_entry */
    if ( list->cur_entry == cur )
        list->cur_entry = list->cur_entry->prev;
    /* free memory */
    if ( list->auto_delete ) {
        if ( list->callback )
            (list->callback)( cur->item );
        else
            free( cur->item );
    }
    free( cur );
    return 1;
}
/*
====================================================================
Delete item if in list. If this was the current entry update
current_entry to valid previous pointer.
Return Value: True if successful else False.
====================================================================
*/
int list_delete_item( List *list, void *item )
{
    return list_delete_pos( list, list_check( list, item ) );
}
/*
====================================================================
Delete entry.
Return Value: True if successful else False.
====================================================================
*/
int list_delete_entry( List *list, List_Entry *entry )
{
    /* delete possible? */
    if ( entry == 0 ) return 0;
    if ( list->count == 0 ) return 0;
    if ( entry == &list->head || entry == &list->tail ) return 0;
    /* adjust anchor and counter */
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    list->count--;
    /* check current_entry */
    if ( list->cur_entry == entry )
        list->cur_entry = list->cur_entry->prev;
    /* free memory */
    if ( list->auto_delete ) {
        if ( list->callback )
            (list->callback)( entry->item );
        else
            free( entry->item );
    }
    free( entry );
    return 1;
}
/*
====================================================================
Get item from position if in list.
Return Value: Item pointer if found else Null pointer.
====================================================================
*/
void* list_get( List *list, int pos )
{
    int i;
    List_Entry *cur = &list->head;

    if ( pos < 0 || pos >= list->count ) return 0;
    for ( i = 0; i <= pos; i++ ) cur = cur->next;
    return cur->item;
}
/*
====================================================================
Check if item's in list.
Return Value: Position of item else -1.
====================================================================
*/
int list_check( List *list, void *item )
{
    int pos = -1;
    List_Entry *cur = list->head.next;
    while ( cur != &list->tail ) {
        pos++;
        if ( cur->item == item ) break;
        cur = cur->next;
    }
    return pos;
}
/*
====================================================================
Return first item stored in list and set current_entry to this
entry.
Return Value: Item pointer if found else Null pointer.
====================================================================
*/
void* list_first( List *list )
{
    list->cur_entry = list->head.next;
    return list->head.next->item;
}
/*
====================================================================
Return last item stored in list and set current_entry to this
entry.
Return Value: Item pointer if found else Null pointer.
====================================================================
*/
void* list_last( List *list )
{
    list->cur_entry = list->tail.prev;
    return list->tail.prev->item;
}
/*
====================================================================
Return item in current_entry.
Return Value: Item pointer if found else Null pointer.
====================================================================
*/
void* list_current( List *list )
{
    return list->cur_entry->item;
}
/*
====================================================================
Reset current_entry to head of list.
====================================================================
*/
void list_reset( List *list )
{
    list->cur_entry = &list->head;
}
/*
====================================================================
Get next item and update current_entry (reset if tail reached)
Return Value: Item pointer if found else Null (if tail of list).
====================================================================
*/
void* list_next( List *list )
{
    list->cur_entry = list->cur_entry->next;
    if ( list->cur_entry == &list->tail ) list_reset( list );
    return list->cur_entry->item;
}
/*
====================================================================
Get previous item and update current_entry.
Return Value: Item pointer if found else Null (if head of list).
====================================================================
*/
void* list_prev( List *list )
{
    list->cur_entry = list->cur_entry->prev;
    return list->cur_entry->item;
}
/*
====================================================================
Delete the current entry if not tail or head. This is the entry
that contains the last returned item by list_next/prev().
Return Value: True if it  was a valid deleteable entry.
====================================================================
*/
int list_delete_current( List *list )
{
	if ( list->cur_entry == 0 || list->cur_entry == &list->head || list->cur_entry == &list->tail ) return 0;
	list_delete_entry( list, list->cur_entry );
	return 1;
}
/*
====================================================================
Check if list is empty.
Return Value: True if list counter is 0 else False.
====================================================================
*/
int list_empty( List *list )
{
    return list->count == 0;
}
/*
====================================================================
Return entry containing the passed item.
Return Value: True if entry found else False.
====================================================================
*/
List_Entry *list_entry( List *list, void *item )
{
    List_Entry *entry = list->head.next;
    while ( entry != &list->tail ) {
        if ( entry->item == item ) return entry;
        entry = entry->next;
    }
    return 0;
}

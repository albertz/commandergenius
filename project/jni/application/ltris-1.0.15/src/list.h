/***************************************************************************
                          list.h  -  description
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

#ifndef __LIST_H
#define __LIST_H

#ifdef __cplusplus
extern "C" {
#endif

/*
====================================================================
Dynamic list handling data as void pointers.
====================================================================
*/
typedef struct _List_Entry {
    struct _List_Entry      *next;
    struct _List_Entry      *prev;
    void                    *item;
} List_Entry;
typedef struct {
    int             auto_delete;
    int             count;
    List_Entry      head;
    List_Entry      tail;
    void            (*callback)(void*);
    List_Entry      *cur_entry;
} List;

/*
====================================================================
Create a new list
  auto_delete:  Free memory of data pointer when deleting entry
  callback:     Use this callback to free memory of data including
                the data pointer itself.
Return Value: List pointer
====================================================================
*/
enum { LIST_NO_AUTO_DELETE = 0, LIST_AUTO_DELETE };
enum { LIST_NO_CALLBACK = 0 };
List *list_create( int auto_delete, void (*callback)(void*) );
/*
====================================================================
Delete list and entries.
====================================================================
*/
void list_delete( List *list );
/*
====================================================================
Delete all entries but keep the list. Reset current_entry to head
pointer.
====================================================================
*/
void list_clear( List *list );
/*
====================================================================
Insert new item at position.
Return Value: True if successful else False.
====================================================================
*/
int list_insert( List *list, void *item, int pos );
/*
====================================================================
Add new item at the end of the list.
====================================================================
*/
int list_add( List *list, void *item );
/*
====================================================================
Delete item at pos. If this was the current entry update
current_entry to valid previous pointer.
Return Value: True if successful else False.
====================================================================
*/
int list_delete_pos( List *list, int pos );
/*
====================================================================
Delete item if in list. If this was the current entry update
current_entry to valid previous pointer.
Return Value: True if successful else False.
====================================================================
*/
int list_delete_item( List *list, void *item );
/*
====================================================================
Delete entry.
====================================================================
*/
int list_delete_entry( List *list, List_Entry *entry );
/*
====================================================================
Get item from position if in list.
Return Value: Item pointer if found else Null pointer.
====================================================================
*/
void* list_get( List *list, int pos );
/*
====================================================================
Check if item's in list.
Return Value: Position of item else -1.
====================================================================
*/
int list_check( List *list, void *item );
/*
====================================================================
Return first item stored in list and set current_entry to this
entry.
Return Value: Item pointer if found else Null pointer.
====================================================================
*/
void* list_first( List *list );
/*
====================================================================
Return last item stored in list and set current_entry to this
entry.
Return Value: Item pointer if found else Null pointer.
====================================================================
*/
void* list_last( List *list );
/*
====================================================================
Return item in current_entry.
Return Value: Item pointer if found else Null pointer.
====================================================================
*/
void* list_current( List *list );
/*
====================================================================
Reset current_entry to head of list.
====================================================================
*/
void list_reset( List *list );
/*
====================================================================
Get next item and update current_entry (reset if tail reached).
Return Value: Item pointer if found else Null (if tail of list).
====================================================================
*/
void* list_next( List *list );
/*
====================================================================
Get previous item and update current_entry.
Return Value: Item pointer if found else Null (if head of list).
====================================================================
*/
void* list_prev( List *list );
/*
====================================================================
Delete the current entry if not tail or head. This is the entry
that contains the last returned item by list_next/prev().
Return Value: True if it  was a valid deleteable entry.
====================================================================
*/
int list_delete_current( List *list );
/*
====================================================================
Check if list is empty.
Return Value: True if list counter is 0 else False.
====================================================================
*/
int list_empty( List *list );
/*
====================================================================
Return entry containing the passed item.
Return Value: True if entry found else False.
====================================================================
*/
List_Entry *list_entry( List *list, void *item );

#ifdef __cplusplus
};
#endif

#endif

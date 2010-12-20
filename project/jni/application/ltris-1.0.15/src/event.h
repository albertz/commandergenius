/***************************************************************************
                          event.h  -  description
                             -------------------
    begin                : Sat Sep 8 2001
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

/*
====================================================================
Event filter used to get motion x.
====================================================================
*/
int event_filter( const SDL_Event *event );
/*
====================================================================
Reset event states
====================================================================
*/
void event_reset();
/*
====================================================================
Grab or release input. Hide cursor if events are kept in window.
Use relative mouse motion and grab if config tells so.
====================================================================
*/
void event_grab_input();
void event_ungrab_input();
/*
====================================================================
Poll next event and set key and mousestate.
Return Value: True if event occured
====================================================================
*/
int event_poll( SDL_Event *event );
/*
====================================================================
Block/unblock motion event
====================================================================
*/
void event_block_motion( int block );

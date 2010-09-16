/***************************************************************************
                          event.c  -  description
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

#include "lbreakout.h"
#include "event.h"

extern int stk_quit_request;

/*
====================================================================
Event filter that blocks all events. Used to clear the SDL
event queue.
====================================================================
*/
static int all_filter( const SDL_Event *event ) { return 0; }

/*
====================================================================
Handle SDL_QUIT events directly and kill all motion events.
====================================================================
*/
int event_filter( const SDL_Event *event )
{
	if ( event->type == SDL_QUIT ) {
		stk_quit_request = 1;
		return 0;
	}
	if ( event->type == SDL_MOUSEMOTION ) 
		return 0;
	return 1;
}

/*
====================================================================
Clear the SDL event key (keydown events)
====================================================================
*/
void event_clear_sdl_queue()
{
    SDL_EventFilter old_filter;
    SDL_Event event;
    old_filter = SDL_GetEventFilter();
    SDL_SetEventFilter( all_filter );
    while ( SDL_PollEvent( &event ) );
    SDL_SetEventFilter( old_filter );
}

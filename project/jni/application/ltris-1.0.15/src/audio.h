/***************************************************************************
                          sound.h  -  description
                             -------------------
    begin                : Sun Jul 29 2001
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

#ifndef __AUDIO_H
#define __AUDIO_H

#ifdef SOUND

/*
====================================================================
Wrapper for the SDL_mixer functions.
====================================================================
*/

/*
====================================================================
Initiate/close audio
====================================================================
*/
int audio_open();
void audio_close();

/*
====================================================================
Sound chunk
====================================================================
*/
typedef Mix_Chunk Sound_Chunk;
Sound_Chunk* sound_chunk_load( char *file_name );
void sound_chunk_free( Sound_Chunk *chunk );

/*
====================================================================
Sound stuff
====================================================================
*/
void sound_enable( int enable );
void sound_volume( int level /*0 - 127*/ );
void sound_play( Sound_Chunk *chunk );

/*
====================================================================
Music stuff
====================================================================
*/
void music_volume( int level );

#endif

#endif

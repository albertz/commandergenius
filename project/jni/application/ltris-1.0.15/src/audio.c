/***************************************************************************
                          sound.c  -  description
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

#ifdef SOUND

#include "ltris.h"
#include "audio.h"

/*
====================================================================
If audio device was properly initiated this flag is set.
If this flag is not set; no action will be taken for audio.
====================================================================
*/
int audio_ok = 0;
/*
====================================================================
If this flag is not set no sound is played.
====================================================================
*/
int sound_enabled = 1;

/*
====================================================================
Initiate/close audio
====================================================================
*/
int audio_open()
{
    if ( Mix_OpenAudio( MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 256 ) < 0 ) {
        fprintf( stderr, "audio_open: can't open audio device: %s\n", SDL_GetError() );
        audio_ok = 0;
        return 0;
    }
    audio_ok = 1;
    return 1;
}
void audio_close()
{
    if ( !audio_ok ) return;
    Mix_CloseAudio();
}

/*
====================================================================
Sound chunk
====================================================================
*/
Sound_Chunk* sound_chunk_load( char *file_name )
{
    char path[512];
    Mix_Chunk *chunk = 0;

    if ( !audio_ok ) {
        fprintf( stderr,
                 "sound_chunk_load: cannot load WAV '%s' as audio device isn't initated properly\n",
                 file_name );
        return 0;
    }

    /* use SRCDIR/sounds as source directory */
    sprintf( path, "%s/sounds/%s", SRC_DIR, file_name );

    chunk = Mix_LoadWAV( path );
    if ( chunk == 0 )
        fprintf( stderr, "chunk_load: couldn't load WAV '%s': %s\n", path, SDL_GetError() );

    return (Sound_Chunk*)chunk;
}
void sound_chunk_free( Sound_Chunk *chunk )
{
    if ( !audio_ok ) return;
    Mix_FreeChunk( (Mix_Chunk*)chunk );
}

/*
====================================================================
Sound stuff
====================================================================
*/

/*
====================================================================
Enable/disable sound
====================================================================
*/
void sound_enable( int enable )
{
    if ( !audio_ok ) return;
    sound_enabled = enable;
    if ( !enable )
        Mix_Pause( -1 ); /* stop all sound channels */
}
/*
====================================================================
Set general volume of sounds. 0 - 127
====================================================================
*/
void sound_volume( int level )
{
    if ( !audio_ok ) return;
    if ( level < 0 ) level = 0;
    if ( level > 127 ) level = 127;
    Mix_Volume( -1, level ); /* all sound channels */
}
/*
====================================================================
Play a chunk.
====================================================================
*/
void sound_play( Sound_Chunk *chunk )
{
    if ( !audio_ok ) return;
    if ( !sound_enabled ) return;
    /* use first free sound channel and play sound one time */
    Mix_PlayChannel( -1, (Mix_Chunk*)chunk, 0 );
}

/*
====================================================================
Music stuff
====================================================================
*/

/*
====================================================================
Set general volume of sounds. 0 - 127
====================================================================
*/
void music_volume( int level )
{
    /* not implemented */
}

#endif

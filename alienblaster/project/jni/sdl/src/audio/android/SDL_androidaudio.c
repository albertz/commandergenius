/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2009 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org

    This file written by Ryan C. Gordon (icculus@icculus.org)
*/
#include "SDL_config.h"

/* Output audio to nowhere... */

#include "SDL_rwops.h"
#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audiomem.h"
#include "../SDL_audio_c.h"
#include "../SDL_audiodev_c.h"
#include "SDL_androidaudio.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include <jni.h>
#include <android/log.h>

#define ANDROIDAUD_DRIVER_NAME         "android"

/* Audio driver functions */
static int ANDROIDAUD_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void ANDROIDAUD_WaitAudio(_THIS);
static void ANDROIDAUD_PlayAudio(_THIS);
static Uint8 *ANDROIDAUD_GetAudioBuf(_THIS);
static void ANDROIDAUD_CloseAudio(_THIS);

/* Audio driver bootstrap functions */
static int ANDROIDAUD_Available(void)
{
	return(1);
}

static void ANDROIDAUD_DeleteDevice(SDL_AudioDevice *device)
{
	SDL_free(device->hidden);
	SDL_free(device);
}

static SDL_AudioDevice *ANDROIDAUD_CreateDevice(int devindex)
{
	SDL_AudioDevice *this;

	/* Initialize all variables that we clean on shutdown */
	this = (SDL_AudioDevice *)SDL_malloc(sizeof(SDL_AudioDevice));
	if ( this ) {
		SDL_memset(this, 0, (sizeof *this));
		this->hidden = (struct SDL_PrivateAudioData *)
				SDL_malloc((sizeof *this->hidden));
	}
	if ( (this == NULL) || (this->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( this ) {
			SDL_free(this);
		}
		return(0);
	}
	SDL_memset(this->hidden, 0, (sizeof *this->hidden));

	/* Set the function pointers */
	this->OpenAudio = ANDROIDAUD_OpenAudio;
	this->WaitAudio = ANDROIDAUD_WaitAudio;
	this->PlayAudio = ANDROIDAUD_PlayAudio;
	this->GetAudioBuf = ANDROIDAUD_GetAudioBuf;
	this->CloseAudio = ANDROIDAUD_CloseAudio;

	this->free = ANDROIDAUD_DeleteDevice;

	return this;
}

AudioBootStrap ANDROIDAUD_bootstrap = {
	ANDROIDAUD_DRIVER_NAME, "SDL Android audio driver",
	ANDROIDAUD_Available, ANDROIDAUD_CreateDevice
};


static SDL_mutex * audioMutex = NULL;
static SDL_cond * audioCond = NULL;
static unsigned char * audioBuffer = NULL;
static size_t audioBufferSize = 0;
static SDL_AudioSpec *audioFormat = NULL;
static int audioInitialized = 0;

/* This function waits until it is possible to write a full sound buffer */
static void ANDROIDAUD_WaitAudio(_THIS)
{
	/* We will block in PlayAudio(), do nothing here */
}

static Uint8 *ANDROIDAUD_GetAudioBuf(_THIS)
{
	return(this->hidden->mixbuf);
}

static void ANDROIDAUD_CloseAudio(_THIS)
{
	SDL_mutex * audioMutex1;
	
	if ( this->hidden->mixbuf != NULL ) {
		SDL_FreeAudioMem(this->hidden->mixbuf);
		this->hidden->mixbuf = NULL;
	}
	if( audioMutex != NULL )
	{
		audioMutex1 = audioMutex;
		SDL_mutexP(audioMutex1);
		audioInitialized = 0;
		SDL_CondSignal(audioCond);
		audioMutex = NULL;
		SDL_DestroyCond(audioCond);
		audioCond = NULL;
		audioFormat = NULL;
		audioBuffer = NULL;
		audioBufferSize = 0;
		SDL_mutexV(audioMutex1);
		SDL_DestroyMutex(audioMutex1);
	}
}

static int ANDROIDAUD_OpenAudio(_THIS, SDL_AudioSpec *spec)
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio 1");
	if( ! (spec->format == AUDIO_S8 || spec->format == AUDIO_S16) )
		return (-1); // TODO: enable format conversion? Don't know how to do that in SDL

	/* Allocate mixing buffer */
	this->hidden->mixlen = spec->size;
	this->hidden->mixbuf = (Uint8 *) SDL_AllocAudioMem(this->hidden->mixlen);
	if ( this->hidden->mixbuf == NULL ) {
		return(-1);
	}
	SDL_memset(this->hidden->mixbuf, spec->silence, spec->size);

	if( audioMutex == NULL )
	{
		audioInitialized = 0;
		audioFormat = spec;
		audioBuffer = this->hidden->mixbuf;
		audioBufferSize = this->hidden->mixlen;
		audioMutex = SDL_CreateMutex();
		audioCond = SDL_CreateCond();
	}

	__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio 2");
	
	SDL_mutexP(audioMutex);
	
	while( !audioInitialized )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio 3");
		if( SDL_CondWaitTimeout( audioCond, audioMutex, 500 ) != 0 )
		{
			__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio() failed! timeout when waiting callback");
			SDL_mutexV(audioMutex);
			ANDROIDAUD_CloseAudio(this);
			return(-1);
		}
	}
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio 4");

	audioFormat = NULL;
	
	SDL_mutexV(audioMutex);

	__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio 5");

	return(0);
}

static void ANDROIDAUD_PlayAudio(_THIS)
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_PlayAudio 1");
	SDL_mutexP(audioMutex);

	audioBuffer = this->hidden->mixbuf;
	audioBufferSize = this->hidden->mixlen;
	
	while( audioBuffer != NULL )
		SDL_CondWait( audioCond, audioMutex );

	SDL_mutexV(audioMutex);
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_PlayAudio 2");
}

#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)

extern jintArray JAVA_EXPORT_NAME(AudioThread_nativeAudioInit) (JNIEnv * env, jobject jobj)
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "AudioThread_nativeAudioInit 1");
	jintArray ret = NULL;
	int initData[4] = { 0, 0, 0, 0 }; // { rate, channels, encoding, bufsize };
	
	if( audioMutex == NULL )
		return;

	__android_log_print(ANDROID_LOG_INFO, "libSDL", "AudioThread_nativeAudioInit 2");
	
	SDL_mutexP(audioMutex);
	
	if( audioInitialized == 0 )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "AudioThread_nativeAudioInit 3");
		initData[0] = audioFormat->freq;
		initData[1] = audioFormat->channels;
		initData[2] = ( audioFormat->format == AUDIO_S16 ) ? 1 : 0;
		initData[3] = audioFormat->size;
		ret=(*env)->NewIntArray(env, 4);
		(*env)->SetIntArrayRegion(env, ret, 0, 4, (jint *)initData);
		audioInitialized = 1;
		SDL_CondSignal(audioCond);
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "AudioThread_nativeAudioInit 4");
	}
	
	SDL_mutexV(audioMutex);

	__android_log_print(ANDROID_LOG_INFO, "libSDL", "AudioThread_nativeAudioInit 5");
	
	return (ret);
};

extern jint JAVA_EXPORT_NAME(AudioThread_nativeAudioBuffer) ( JNIEnv * env, jobject jobj, jbyteArray data )
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "AudioThread_nativeAudioBuffer 1");
	int ret = 0;
	
	if( audioMutex == NULL )
		return;
	
	SDL_mutexP(audioMutex);
	
	if( !audioInitialized )
		ret = -1;

	if( audioBuffer == NULL )
	{
		ret = 0;
	}
	else
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "AudioThread_nativeAudioBuffer 2");
		(*env)->SetByteArrayRegion(env, data, 0, audioBufferSize, (jbyte *)audioBuffer);
		ret = audioBufferSize;
		audioBuffer = NULL;
		audioBufferSize = 0;
		SDL_CondSignal(audioCond);
	}

	SDL_mutexV(audioMutex);

	__android_log_print(ANDROID_LOG_INFO, "libSDL", "AudioThread_nativeAudioBuffer 3");
	return ret;
};


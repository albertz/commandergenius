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
#include <string.h> // for memset()

#define _THIS	SDL_AudioDevice *this

// TODO: make audio single-threaded, the same way as video

/* Audio driver functions */
static int ANDROIDAUD_OpenAudio(_THIS, const char *devname, int iscapture);
static void ANDROIDAUD_WaitAudio(_THIS);
static void ANDROIDAUD_PlayAudio(_THIS);
static Uint8 *ANDROIDAUD_GetAudioBuf(_THIS);
static void ANDROIDAUD_CloseAudio(_THIS);

/* Audio driver bootstrap functions */
static int ANDROIDAUD_Available(void)
{
	return(1);
}

static void ANDROIDAUD_DeleteDevice()
{
}

static int ANDROIDAUD_CreateDevice(SDL_AudioDriverImpl * impl)
{
	/* Set the function pointers */
	impl->OpenDevice = ANDROIDAUD_OpenAudio;
	impl->WaitDevice = ANDROIDAUD_WaitAudio;
	impl->PlayDevice = ANDROIDAUD_PlayAudio;
	impl->GetDeviceBuf = ANDROIDAUD_GetAudioBuf;
	impl->CloseDevice = ANDROIDAUD_CloseAudio;
	impl->Deinitialize = ANDROIDAUD_DeleteDevice;
	impl->OnlyHasDefaultOutputDevice = 1;

	return 1;
}

AudioBootStrap ANDROIDAUD_bootstrap = {
	"android", "SDL Android audio driver",
	ANDROIDAUD_CreateDevice, 0
};


static unsigned char * audioBuffer = NULL;
static size_t audioBufferSize = 0;

// Extremely wicked JNI environment to call Java functions from C code
static jbyteArray audioBufferJNI = NULL;
static JNIEnv * jniEnv = NULL;
static jclass JavaAudioThreadClass = NULL;
static jobject JavaAudioThread = NULL;
static jmethodID JavaFillBuffer = NULL;
static jmethodID JavaInitAudio = NULL;
static jmethodID JavaDeinitAudio = NULL;


static Uint8 *ANDROIDAUD_GetAudioBuf(_THIS)
{
	return(audioBuffer);
}

static void ANDROIDAUD_CloseAudio(_THIS)
{
	(*jniEnv)->DeleteGlobalRef(jniEnv, audioBufferJNI);
	audioBufferJNI = NULL;
	audioBuffer = NULL;
	audioBufferSize = 0;
	
	(*jniEnv)->CallIntMethod( jniEnv, JavaAudioThread, JavaDeinitAudio );
	
	if ( this->hidden != NULL ) {
		SDL_free(this->hidden);
		this->hidden = NULL;
	}
}

static int ANDROIDAUD_OpenAudio(_THIS, const char *devname, int iscapture)
{
	SDL_AudioSpec *audioFormat = &this->spec;
	jintArray initArray = NULL;
	int initData[4] = { 0, 0, 0, 0 }; // { rate, channels, encoding, bufsize };
	jobject * bufferObj;
	jboolean isCopy = JNI_TRUE;
	unsigned char *audioBuffer;
	int audioBufferSize;
	int bytesPerSample;

	this->hidden = (struct SDL_PrivateAudioData *) SDL_malloc((sizeof *this->hidden));
	if ( this->hidden == NULL ) {
		SDL_OutOfMemory();
		return(-1);
	}
	SDL_memset(this->hidden, 0, (sizeof *this->hidden));

	if( ! (this->spec.format == AUDIO_S8 || this->spec.format == AUDIO_S16) )
	{
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "Application requested unsupported audio format - only S8 and S16 are supported");
		return (-1); // TODO: enable format conversion? Don't know how to do that in SDL
	}

	
	initData[0] = audioFormat->freq;
	initData[1] = audioFormat->channels;
	bytesPerSample = (audioFormat->format & 0xFF) / 8;
	initData[2] = ( bytesPerSample == 2 ) ? 1 : 0;
	audioFormat->format = ( bytesPerSample == 2 ) ? AUDIO_S16 : AUDIO_S8;
	initData[3] = audioFormat->size;
	initArray = (*jniEnv)->NewIntArray(jniEnv, 4);
	(*jniEnv)->SetIntArrayRegion(jniEnv, initArray, 0, 4, (jint *)initData);
	
	bufferObj = (*jniEnv)->CallObjectMethod( jniEnv, JavaAudioThread, JavaInitAudio, initArray );

	if( ! bufferObj )
	{
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_OpenAudio(): failed to get audio buffer from JNI");
		ANDROIDAUD_CloseAudio(this);
		return(-1);
	}

	audioBufferJNI = (jbyteArray*)(*jniEnv)->NewGlobalRef(jniEnv, bufferObj);
	audioBufferSize = (*jniEnv)->GetArrayLength(jniEnv, audioBufferJNI);
	audioBuffer = (unsigned char *) (*jniEnv)->GetByteArrayElements(jniEnv, audioBufferJNI, &isCopy);
	if( isCopy == JNI_TRUE )
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "ANDROIDAUD_OpenAudio(): JNI returns a copy of byte array - no audio will be played");

	bytesPerSample = (audioFormat->format & 0xFF) / 8;
	audioFormat->samples = audioBufferSize / bytesPerSample / audioFormat->channels;
	audioFormat->size = audioBufferSize;
	SDL_memset(audioBuffer, audioFormat->silence, audioFormat->size);

	SDL_CalculateAudioSpec(&this->spec);
	
	return(1);
}

/* This function waits until it is possible to write a full sound buffer */
static void ANDROIDAUD_WaitAudio(_THIS)
{
	/* We will block in PlayAudio(), do nothing here */
}

static void ANDROIDAUD_PlayAudio(_THIS)
{
	(*jniEnv)->ReleaseByteArrayElements(jniEnv, audioBufferJNI, (jbyte *)audioBuffer, 0);
	audioBuffer == NULL;

	(*jniEnv)->CallIntMethod( jniEnv, JavaAudioThread, JavaDeinitAudio );

	jboolean isCopy = JNI_TRUE;
	audioBuffer = (unsigned char *) (*jniEnv)->GetByteArrayElements(jniEnv, audioBufferJNI, &isCopy);
	if( isCopy == JNI_TRUE )
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "ANDROIDAUD_PlayAudio() JNI returns a copy of byte array - that's slow");

}

#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)

extern int JAVA_EXPORT_NAME(AudioThread_nativeAudioInitJavaCallbacks) (JNIEnv * env, jobject thiz)
{
	__android_log_print(ANDROID_LOG_INFO, "libSDL", "nativeAudioInitJavaCallbacks(): enter");
	jniEnv = env;
	JavaAudioThread = thiz;
	
	JavaAudioThreadClass = (*jniEnv)->GetObjectClass(jniEnv, thiz);
	JavaFillBuffer = (*jniEnv)->GetMethodID(jniEnv, JavaAudioThreadClass, "fillBuffer", "()I");
	JavaInitAudio = (*jniEnv)->GetMethodID(jniEnv, JavaAudioThreadClass, "initAudio", "([I)[B");
	JavaDeinitAudio = (*jniEnv)->GetMethodID(jniEnv, JavaAudioThreadClass, "deinitAudio", "()I");
	if( ! JavaFillBuffer )
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "nativeAudioInitJavaCallbacks(): JavaFillBuffer is NULL");
	if( ! JavaInitAudio )
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "nativeAudioInitJavaCallbacks(): JavaInitAudio is NULL");
	if( ! JavaInitAudio )
		__android_log_print(ANDROID_LOG_ERROR, "libSDL", "nativeAudioInitJavaCallbacks(): JavaDeinitAudio is NULL");

	
}

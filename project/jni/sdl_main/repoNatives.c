#include <jni.h>
#include <android/log.h>
//#include <GLES/gl.h>
//#include <GLES/glext.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <string.h> // for memset()

/*#include "SDL_config.h"
#include "SDL_version.h"

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "SDL_mutex.h"
#include "SDL_thread.h"
#include "SDL_android.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "../SDL_sysvideo.h"
#include "SDL_androidvideo.h"*/
#include "jniwrapperstuff.h"


JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeSetEnv) ( JNIEnv*  env, jobject thiz, jstring j_name, jstring j_value )
{
    jboolean iscopy;
    const char *name = (*env)->GetStringUTFChars(env, j_name, &iscopy);
    const char *value = (*env)->GetStringUTFChars(env, j_value, &iscopy);
    setenv(name, value, 1);
    (*env)->ReleaseStringUTFChars(env, j_name, name);
    (*env)->ReleaseStringUTFChars(env, j_value, value);
}

JNIEXPORT jint JNICALL
JAVA_EXPORT_NAME(Settings_nativeChmod) ( JNIEnv*  env, jobject thiz, jstring j_name, jint mode )
{
    jboolean iscopy;
    const char *name = (*env)->GetStringUTFChars(env, j_name, &iscopy);
    int ret = chmod(name, mode);
    (*env)->ReleaseStringUTFChars(env, j_name, name);
    return (ret == 0);
}

JNIEXPORT void JNICALL
JAVA_EXPORT_NAME(Settings_nativeChdir) ( JNIEnv*  env, jobject thiz, jstring j_dir )
{
    jboolean iscopy;
    const char *dirname = (*env)->GetStringUTFChars(env, j_dir, &iscopy);
    chdir(dirname);
    (*env)->ReleaseStringUTFChars(env, j_dir, dirname);
}

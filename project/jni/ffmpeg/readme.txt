FFMPEG for Android was downlaoded from
http://sourceforge.net/projects/ffmpeg4android/

To include this library to your Android project, do following changes:

jni/Application.mk:

APP_MODULES += avcodec \
               avdevice \
               avfilter \
               avformat \
               avresample \
               avutil \
               swresample \
               swscale

jni/<your-library>/Android.mk:

LOCAL_SHARED_LIBRARIES += avcodec \
                          avdevice \
                          avfilter \
                          avformat \
                          avresample \
                          avutil \
                          swresample \
                          swscale

LOCAL_C_INCLUDES += jni/ffmpeg/include

and somewhere in Java code:

// Order is important, they depend on each other
String ffmpeg_libs[] = {
    "avutil",
    "avcodec",
    "avformat",
    "avresample",
    "swresample",
    "swscale",
    "avfilter",
    "avdevice"
};
for(String l : ffmpeg_libs) {
    System.loadLibrary(l);
}

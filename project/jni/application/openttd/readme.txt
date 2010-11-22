Quick compilation guide:
Download my GIT repo from https://github.com/pelya/commandergenius,
then install Android SDK 2.2, NDK r4b, and "ant" tool, then launch commands
    rm project/jni/application/src
    ln -s scummvm project/jni/application/src
    cd project && android update project -p .
then download OpenTTD 1.0.5 into the dir project/jni/application/openttd/openttd
(or create a symlink to it if you already have downloaded it), then apply patch openttd-trunk-android.patch
and launch build.sh. That should be it.
If you want to build OpenTTD trunk you should add line "--with-lzma=sdl-config" to "configure" parameters.
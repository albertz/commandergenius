Quick build instructions: you have to download GIT repo from https://github.com/pelya/commandergenius ,
then install Android SDK 4.0.3, NDK r8b from http://develoiper.android.com/ , and Apache Ant tool (and add them to your PATH),
then you should launch commands
    rm project/jni/application/src
    ln -s ufoai project/jni/application/src
    ./changeAppSettings.sh -a
    android update project -p project
then download UFO:AI GIT into the dir project/jni/application/ufoai/ufoai
(or create a symlink to it if you already have downloaded it), and launch build.sh. That should be it.
It will create file project/bin/MainActivity-debug.apk which you can rename to ufoai.apk, sign with release key and put to Releases page.

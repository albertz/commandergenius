Quick build instructions: you have to download GIT repo from https://github.com/pelya/commandergenius ,
then install Android SDK 2.2, NDK r4b from http://develoiper.android.com , and Apache Ant tool, then you should launch commands
    rm project/jni/application/src
    ln -s ufoai project/jni/application/src
    cd project && android update project -p .
then download UFO:AI GIT into the dir project/jni/application/ufoai/ufoai
(or create a symlink to it if you already have downloaded it), and launch build.sh. That should be it.

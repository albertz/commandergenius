Quick compilation guide:
Download my GIT repo from https://github.com/pelya/commandergenius,
then install Android SDK 2.2, NDK r4b (NDK r5/r5b will not compile this),
and "ant" tool, then launch commands
    rm project/jni/application/src
    ln -s vcmi project/jni/application/src
    cd project && android update project -p .
then download OpenTTD 1.1.1 into the dir project/jni/application/openttd/openttd
(or create a symlink to it if you already have downloaded it), then apply patch openttd-trunk-android.patch
and launch build.sh. That should be it.

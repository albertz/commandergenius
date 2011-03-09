Quick compilation guide:
Download my GIT repo from https://github.com/pelya/commandergenius,
then install Android SDK 2.2, NDK r4b, and "ant" tool, then launch commands
    rm project/jni/application/src
    ln -s scummvm project/jni/application/src
    cd project && android update project -p .
then download OpenTTD r21531 into the dir project/jni/application/openttd/openttd,
apply ChillCore patch onto it, apply my patch openttd-trunk-android.patch
and launch build.sh. That should be it.
There will be error with "sqrtl" function not defined, change it to "sqrt".
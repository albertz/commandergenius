Quick compilation guide:
Download my GIT repo from https://github.com/pelya/commandergenius,
then install Android SDK, Android NDK, and "ant" tool, then launch commands
    rm project/jni/application/src
    ln -s openttd project/jni/application/src
    ./ChangeAppSettings.sh -a
    android update project -p project -t android-12
then download OpenTTD source release into the dir project/jni/application/openttd/openttd
(or create a symlink to it), then apply patch openttd-trunk-android.patch
and launch build.sh. That should be it.

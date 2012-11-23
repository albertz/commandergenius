Quick compilation guide:
Download my GIT repo from https://github.com/pelya/commandergenius,
then install Android SDK, Android NDK, and "ant" tool, then launch commands
    rm project/jni/application/src
    ln -s openttd project/jni/application/src
    ./changeAppSettings.sh -a
    android update project -p project
then launch build.sh. That should be it.

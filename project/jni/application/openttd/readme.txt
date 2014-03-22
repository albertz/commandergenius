Quick compilation guide:
Install liblzma-dev, it's needed for configure script
sudo apt-get install liblzma-dev
Download my Git repo from https://github.com/pelya/commandergenius,
then install Android SDK, Android NDK, and "ant" tool, then launch commands
    git submodule update --init --recursive
    rm project/jni/application/src
    ln -s openttd project/jni/application/src
    ./changeAppSettings.sh -a
    android update project -p project
then launch build.sh. That should be it.

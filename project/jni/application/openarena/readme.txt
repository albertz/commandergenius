Quick compilation guide for Debian/Ubuntu (Windows is not supported, MacOsX should be okay though):
Download SDL Git repo from https://github.com/pelya/commandergenius,
install latest Android SDK, latest Android NDK, and "ant" tool (sudo apt-get install ant),
then launch commands:
    git submodule update --init project/jni/application/openarena/engine
    git submodule update --init project/jni/application/openarena/vm
    rm project/jni/application/src # ignore the error
    ln -s openarena project/jni/application/src
    ./changeAppSettings.sh -a
    android update project -p project
    ./build.sh
That should do it.

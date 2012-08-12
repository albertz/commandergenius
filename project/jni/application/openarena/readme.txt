Quick compilation guide for Debian/Ubuntu (Windows is not supported, MacOsX should be okay though):
Download SDL Git repo from https://github.com/pelya/commandergenius,
install latest Android SDK, Android NDK r8b or newer, and "ant" tool (sudo apt-get install ant),
then launch commands:
    git submodule update --init project/jni/application/openarena/engine
    rm project/jni/application/src # ignore the error
    ln -s openarena project/jni/application/src
    ./ChangeAppSettings.sh -a
    android update project -p project -t android-12
    ./build.sh
That should do it.

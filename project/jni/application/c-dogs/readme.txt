Quick compilation guide for Debian/Ubuntu (Windows is not supported, MacOsX should be okay though):
Download SDL Git repo from https://github.com/pelya/commandergenius,
install latest Android SDK, Android NDK r8b or newer, and "ant" tool (sudo apt-get install ant),
then launch commands:
    git submodule update --init --recursive project/jni/application/c-dogs/src
    rm project/jni/application/src
    ln -s c-dogs project/jni/application/src
    ./build.sh
That should do it.

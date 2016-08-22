Quick compilation guide for Debian/Ubuntu (Windows is not supported, MacOsX should be okay though):
Download SDL Git repo from https://github.com/pelya/commandergenius,
install latest Java, Android SDK, Android NDK, and "ant" tool (sudo apt-get install ant openjdk-8-jdk),
then launch commands:
    git submodule update --init project/jni/application/ninslash/src
    rm project/jni/application/src # ignore the error
    ln -s ninslash project/jni/application/src
    ./changeAppSettings.sh -a
    android update project -p project -t android-23
    ./build.sh
That should do it.

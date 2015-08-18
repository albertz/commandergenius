Quick compilation guide for Debian/Ubuntu (Windows is not supported, MacOsX should be okay though):
Download SDL Git repo from https://github.com/pelya/commandergenius,
install latest Android SDK, Android NDK r8b or newer, and "ant" tool (sudo apt-get install ant),
then launch commands:
    git submodule update --init project/jni/application/teeworlds/src
    rm project/jni/application/src # ignore the error
    ln -s teeworlds project/jni/application/src
    ./changeAppSettings.sh -a
    android update project -p project
    ./build.sh
That should do it.

Bot sources are not available, because of cheaters that use aimbot client.
They are available on request from here:
https://www.teeworlds.com/forum/viewtopic.php?id=11397

You should download the SDL for Andorid repository from https://github.com/pelya/commandergenius
and also the MilkyTracker sources - extract them to the directory  project/jni/application/milkytracker in the SDL repo.
To compile you will need to install need a recent Android SDK and NDK r8, and put the dirs
android-ndk-r8
android-ndk-r8/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin
android-sdk-linux
android-sdk-linux/tools
android-sdk-linux/platform-tools
into your $PATH, then you'll need to go to the SDL repo root and run commands
rm project/jni/application/src
ln -s milkytracker project/jni/application/src
./ChangeAppSettings.sh -a
android update project -p project -t android-12
./build.sh
and that should be it - it will compile an .apk file and will try to install it onto your device, if it's connected.
You'll need to do all of this on Linux or MacOsX, Windows is not supported
(but you may try to set up a Linux environment on Windows using https://sourceforge.net/projects/portableubuntu/ for example).

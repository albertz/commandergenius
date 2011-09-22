Quick compilation guide:
Download my GIT repo from https://github.com/pelya/commandergenius,
then install Android SDK 3.1 from http://developer.android.com,
NDK r5 from http://crystax.net/ and "ant" tool, then launch commands
    android update project -p project
    rm project/jni/application/src
    ln -s vcmi project/jni/application/src
    echo > project/jni/application/vcmi/libvcmi.so
then download VCMI into the dir project/jni/application/vcmi/vcmi
(or create a symlink to it if you already have downloaded it), then apply patch vcmi-android.diff,
then launch build.sh - it will fail at link stage, remove dummy libvcmi.so from
project/jni/application/vcmi/vcmi and from project/obj/local/armeabi, launch "make" from directory
project/jni/application/vcmi, then launch build.sh again.

To run it you should have complete installation of Heroes 3: Wake of Gods on your SD card on your device,
in the directory app-data/eu.vcmi

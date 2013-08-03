Quick compilation guide:
Download my GIT repo from https://github.com/pelya/commandergenius,
then install Android SDK and NDK from http://developer.android.com,
ANT, patch and Subversion tools, then launch commands
    android update project -p project
    rm project/jni/application/src
    ln -s vcmi project/jni/application/src
    mkdir -p project/obj/local/armeabi-v7a/
    cp -f project/jni/boost/lib/arm-linux-androideabi-4.6/* project/obj/local/armeabi-v7a/
then launch build.sh.

To run it you should have complete installation of Heroes 3: Wake of Gods
on your SD card, in the directory Android/data/eu.vcmi/files

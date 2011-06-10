Quick compilation guide:
Download my GIT repo from https://github.com/pelya/commandergenius,
then install Android SDK 2.2 from http://developer.android.com,
NDK r5 beta3 from http://crystax.net (I'll move the sources to the official NDK r5c over time)
and "ant" tool, then launch commands
    rm project/jni/application/src
    ln -s vcmi project/jni/application/src
    cd project && android update project -p .
then download VCMI into the dir project/jni/application/vcmi/vcmi
(or create a symlink to it if you already have downloaded it), then apply patch vcmi-android.diff,
launch "make" from directory project/jni/application/vcmi (it will create some shared libs),
then launch build.sh.
To run it you should have complete installation of Heroes 3: Wake of Gods on your SD card on your device,
in the directory app-data/eu.vcmi

#!/bin/bash

# Set here your own NDK path if needed
# export PATH=$PATH:~/src/endless_space/android-ndk-r4

cd project && nice -n5 ndk-build V=1 -j2 && ant debug && cd bin && adb install -r DemoActivity-debug.apk


#!/bin/bash

# Set here your own NDK path if needed
export PATH=$PATH:~/src/endless_space/android-ndk-r4

cd project && ndk-build -j2 V=1 && ant debug && cd bin && adb install -r DemoActivity-debug.apk


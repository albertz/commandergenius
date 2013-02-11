#!/bin/sh
adb shell rm /sdcard/Android/data/ws.openarena.sdl/files/libsdl-DownloadFinished-10.flag
[ -n "$1" ] && adb shell rm -r /sdcard/Android/data/ws.openarena.sdl/files/.openarena

#!/bin/sh
adb shell rm /sdcard/Android/data/ws.openarena.sdl/files/libsdl-DownloadFinished-1.flag
adb shell rm /mnt/extSdCard/Android/data/ws.openarena.sdl/files/libsdl-DownloadFinished-1.flag
[ -n "$1" ] && adb shell rm -r /sdcard/Android/data/ws.openarena.sdl/files/.openarena

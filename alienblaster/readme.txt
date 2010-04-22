This is Alien Blaster game ported to Google Android (original sources: http://www.schwardtnet.de/alienblaster/ ).
I did not change anything in Alien Blaster sources, except for SCREEN_WIDTH,
SCREEN_HEIGHT and BIT_DEPTH constants in global.h, to support 320x430x16bpp video mode,
and also made audio initialize after main() has been called, not inside static initializers (ugh)

This should be compiled with Android 1.6 SDK and NDK - google for them and install them as described in their docs.
You'll need to install Eclipse or Ant too
Then symlink this dir to <android-ndk>/apps under the name "alienblaster":
	ln -s `pwd` <android-ndk>/apps/alienblaster
Then make symling of alienblaster/src dir into project/jni/alienblaster/src - this should be absolute path:
	ln -s /home/user/sources/AlienBlaster/src project/jni/alienblaster/src
Then go to <android-ndk> dir and execute:
	make APP=alienblaster V=1
Hopefully it will compile the file project/libs/armeabi/libalienblaster.so
Then you'll have to compile Android .apk package with Java wrapper code for this lib:
Go to "project" directory and type
	android update project -p .
	ant debug
That will create file project/bin/DemoActivity-debug.apk - use "adb install" to test it
Then you can test it by launching Alien Blaster icon from Android applications menu.
It's designed for 640x480, and GUI elements are drawn out of place, but you can play the game.
Note: The game enforces vertical screen orientation, but you may open your keyboard and use it for 
additional keys - the phone will just keep current screen orientation.
Fire key is Call key ( = left Ctrl for SDL ), Change weapon is Menu key ( = left Alt for SDL )
Note that you may use Volume up/down and Camera keys as game inputs -
you'll have to redefine them in game keyconfig menu.
Other keys like Home, Back and End Call will force application quit, and because
the app itself does not handle SDL_QUIT event correctly (asks for confirmation),
it will stay in memory until you reboot device. The same will happen if the phone 
goes to sleep, so hit keyboard often please. (actually it won't stay in memory - it will crash :P )
To exit correctly press Menu key - it's redirected to Escape.

When porting you own app, first of all ensure that your application supports
one of 320x200, 320x240 or 320x430 display resolutions and 16 bits per pixel
(320x430 is resolution for HTC devices, if other vendors will produce Android phones it may differ).
You may try to use 640x480x16, and scroll through the larger underlying screen with accelerometer, 
but it will give you 2 FPS at most because I've screwed up with optimizations.
In the future I'm planning to use accelerometer as a SDL joystick.

Replace all strings "alienblaster" and "de.schwardtnet.alienblaster" with
the name of your application and your reversed webpage address (or any unique string):
	Application.mk:2
	project/AndroidManifest.xml:3
	project/src/DemoActivity.java:42 and 71
	project/jni/Android.mk:3 and 10
	project/res/values/strings.xml:3
	(that's all, maybe I forgot something)

Make directory project/jni/<yourapp>, copy there file project/jni/alienblaster/Android.mk and edit it -
rename all "alienblaster" strings to your app name, add subdirs of your app under "CG_SUBDIRS := src"
and change "LOCAL_CPP_EXTENSION := .cc" to an extension your C++ files are using
Then repeat steps:
	make APP=<yourapp> V=1
	ant debug

Application data is not bundled with app itself - it should be downloaded from net on first run.
Create .ZIP file with your application data, and put it somewhere on HTTP server,
then replace project/src/DemoActivity.java:73 with download URL.
If your app data is bigger than 5 megabytes it's better to store it on SD card -
set DownloadToSdcard variable to true in project/src/DemoActivity.java:78,
and set appropriate path in project/jni/Android.mk:10
(I did not test the code with SD card, so it may fail)

If you'll add new libs add them to project/jni/, copy Android.mk from existing lib, and
add libname to Application.mk and project/jni/<yourapp>/Android.mk

Audio formats currently supported are AUDIO_S8 and AUDIO_S16 (signed 8-bit and 16-bit PCM)

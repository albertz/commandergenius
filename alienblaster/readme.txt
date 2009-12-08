This is Alien Blaster game ported to Google Android.
I did not change anything in Alien Blaster sources, except for SCREEN_WIDTH,
SCREEN_HEIGHT and BIT_DEPTH constants in global.h, to support 320x430x16bpp video mode.

This should be compiled with Android 1.6 SDK and NDK - google for them and install them as described in their docs.
You'll need to install Eclipse or Ant too
Then symlink this dir to <android-ndk>/apps under the name "alienblaster":
	ln -s `pwd` <android-ndk>/apps/alienblaster
Then make symling of alienbuster/src dir into project/jni/alienblaster/src - this should be absolute path:
	ln -s /home/user/sources/AlienBlaster/src project/jni/alienblaster/src
Then go to <android-ndk> dir and execute:
	make APP=alienblaster V=1
Hopefully it will compile the file project/libs/armeabi/libalienblaster.so
Then you'll have to compile Android .apk package with Java wrapper code for this lib:
Go to "project" directory and type
	android update project -p .
	ant debug
That will create file project/bin/DemoActivity-debug.apk - use "adb install" to test it

Alien Blaster data can be downloaded from http://www.schwardtnet.de/alienblaster/ -
download alienblaster-1.1.0.tgz, unpack it and execute
	adb shell
	<in adb shell>:
		su
		mkdir /data/data/de.schwardtnet.alienblaster/files
		exit
	adb push alienblaster /data/data/de.schwardtnet.alienblaster/files
Then you can test it by launching Alien Blaster icon from Android applications menu.
It's designed for 640x480, and GUI elements are drawn out of place, but you can play the game.
Note: You should play it with vertical screen orientation (keyboard is closed)
Fire key is Call key ( = left Ctrl for SDL ), Change weapon is Menu key ( = left Alt for SDL )
Note that you may use Volume up/down and Camera keys as game inputs -
you'll have to redefine them in game keyconfig menu.
Other keys like Home, Back and End Call will force application quit, and because
the app itself does not handle SDL_QUIT event correctly (asks for confirmation),
it will stay in memory until you reboot device. The same will happen if the phone 
goes to sleep, so hit keyboard often please.
To exit correctly press Menu key - it's redirected to Escape.

When porting you own app, first of all ensure that your application supports
one of 320x200, 320x240 or 320x430 display resolutions and 16 bits per pixel
(320x430 is resolution for HTC devices, if other vendors will produce Android phones it may differ).

Replace all strings "alienblaster" and "de.schwardtnet.alienblaster" with
the name of your application and your reversed webpage address (or any unique string):
	Application.mk:2
	project/AndroidManifest.xml:3
	project/src/DemoActivity.java:42 and 57
	project/jni/Android.mk:3 and 10
	project/res/values/strings.xml:3
	(that's all, maybe I forgot something)

Make directory project/jni/<yourapp>, copy there file project/jni/alienblaster/Android.mk and edit it -
rename all "alienblaster" strings to your app name, add subdirs of your app under "CG_SUBDIRS := src"
and change "LOCAL_CPP_EXTENSION := .cc" to an extension your C++ files are using
Then repeat steps:
	make APP=<yourapp> V=1
	ant debug

Currently you have to copy your application data by hand using "adb push" (it can copy directories too),
into the path specified in project/jni/Android.mk:10,
in the future I'll add some handler to Java sources that downloads data from Internet and
puts it into given folder before executing main code.

If you'll add new libs add them to project/jni/, copy Android.mk from existing lib, and
add libname to Application.mk and project/jni/<yourapp>/Android.mk

Note that there's still no sound in SDL, only video and keyboard/mouse


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
	adb push alienblaster /sdcard/alienblaster
Then you can test it by launching Alien Blaster icon from Android applications menu.
It's designed for 640x480, but with bit of luck you can redefine your keys and play the game a bit.

When porting you own app, replace "alienblaster" and "de.schwardtnet.alienblaster" with
the name of your application and your reversed webpage address everywhere:
	Application.mk:2
	project/AndroidManifest.xml:3
	project/src/DemoActivity.java:42 and 57
	project/jni/Android.mk:3 and 10
	project/res/values/strings.xml:3
	(that's all, maybe I forgot something)

Make directory project/jni/<yourapp>, copy there file project/jni/alienblaster/Android.mk and edit it
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


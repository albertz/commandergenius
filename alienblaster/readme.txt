This is Alien Blaster game ported to Google Android (original sources: http://www.schwardtnet.de/alienblaster/ ).
I did not change anything in Alien Blaster sources, except for SCREEN_WIDTH,
SCREEN_HEIGHT and BIT_DEPTH constants in global.h, to support 320x480x16bpp video mode,
and also made audio initialize after main() has been called, not inside static initializers.

This should be compiled with Android 1.6 SDK and NDK - google for them and install them as described in their docs.
You'll need to install Ant too
Then symlink this dir to <android-ndk>/apps under the name "alienblaster":
	ln -s `pwd` <android-ndk>/apps/alienblaster
Then go to <android-ndk> dir and execute:
	make APP=alienblaster V=1
Hopefully it will compile a bunch of libs under project/libs/armeabi
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
Other keys like Home, Search and End Call will force application quit, and because
the app itself does not handle SDL_QUIT event correctly (asks for confirmation),
it will stay in memory until you reboot device (actually it won't stay in memory - it will crash :P ).
To exit correctly press Menu key - it's redirected to Escape.

This port also supports GL ES + SDL combo - there is GLXGears demo app in project/jni/application/glxgears,
remove all files from project/jni/application/src and put glxgears.c there to check if it works.
Note that GL ES is NOT pure OpenGL - there are no glBegin() and glEnd() call and other widely used functions,
and generally it will take a lot of effort to port pure OpenGL application to GL ES.

When porting you own app, first of all ensure that your application supports
one of 320x200, 320x240 or 480x320 display resolutions and 16 bits per pixel
(480x320 is native resolution for G1, Supersonic has 800x480).
Also there is 640x480 mode, but it's very slow on G1 to copy from memory surface (2 FPS max).
SDL_ListModes()[0] will always return native screen resolution, other modes are there for compatibility.

To compile your own app, put your app sources into project/jni/application dir (remove Alien Blaster first),
and launch script ChangeAppSettings.sh - it will put the name of your app in several places in sources.
The C++ files shall have .cpp extension to be compiled.

Then repeat steps:
	make APP=alienblaster V=1
	ant debug

Application data is not bundled with app itself - it should be downloaded from net on first run.
Create .ZIP file with your application data, and put it somewhere on HTTP server - ChangeAppSettings.sh
will ask you for the URL.
If your app data is bigger than 5 megabytes it's better to store it on SD card, 
internal flash on Android is very limited.

If you'll add new libs add them to project/jni/, copy Android.mk from existing lib, and
add libname to Application.mk and project/jni/<yourapp>/Android.mk

Audio formats currently supported are AUDIO_S8 and AUDIO_S16 (signed 8-bit and 16-bit PCM).

Known bugs:

1. Application will crash on exit or when you're pressing "Home" button - the correct behavior for Android apps
is to stay in memory and go to foreground when you're launching app again, that's not working yet because
app will lose OpenGL context (there are rumors that it won't lose GL context in 2.1 SDK).

2. Multitouch support - it is available since 2.0 SDK, but not on 1.6 - I'll have to follow
http://devtcg.blogspot.com/2009/12/gracefully-supporting-multiple-android.html to make it compile.

3. In the future I'm planning to use accelerometer as a SDL joystick.

4. Many SDL games require keyboard, and newer phones have only touchscreen - there should be configuration screen
where you can toggle on-screen keyboard, bind existing keys like volume up/down to PgUp/PgDown for example,
and configure other actions like accelerometer tilt as keypresses.

5. Export phone vibrator to SDL - interface is available in SDL 1.3

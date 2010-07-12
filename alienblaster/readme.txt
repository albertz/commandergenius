This is Alien Blaster game ported to Google Android (original sources: http://www.schwardtnet.de/alienblaster/ ).
I did not change anything in Alien Blaster sources, except for SCREEN_WIDTH,
SCREEN_HEIGHT and BIT_DEPTH constants in global.h, to support 320x480x16bpp video mode,
and also made audio initialize after main() has been called, not inside static initializers.

This should be compiled with Android 2.2 SDK and NDK r4 - google for them and install them as described in their docs
(the application will run on Android 1.6 and above, 2.2 is the first version where you can debug it).
You'll need to install Ant too.
Go to "project" directory and launch command
	android update project -p .
Then go back, edit file build.sh if needed to add NDK dir to your PATH, then launch it.
Hopefully it will compile a bunch of libs under project/libs/armeabi,
create file project/bin/DemoActivity-debug.apk and install it on your device or emulator.
Then you can test it by launching Alien Blaster icon from Android applications menu.
It's designed for 640x480, so if you have smaller screen it will be resized.
Note: The game enforces horizontal screen orientation, you may open your keyboard and use it for 
additional keys - the phone will just keep current screen orientation.
Note that you may use Volume up/down and Camera keys as game inputs -
you'll have to redefine them in game keyconfig menu.
Keys Home, Search and End Call will force application quit, and because
of a bug in my SDL implementation application will crash.
Back key is mapped to Escape, and both Menu and cursor keys center / trackball click are mapped to Enter.
Newer Android phones like HTC Evo have no keyboard at all, so there are just 3 usable keys - 
Menu, Volume Up and Volume Down (and Escape of course).
Because of that the accelerometer is configured to trigger cursor key events.

This port also supports GL ES + SDL combo - there is GLXGears demo app in project/jni/application/glxgears,
remove all files from project/jni/application/src and put glxgears.c there to check if it works.
Note that GL ES is NOT pure OpenGL - there are no glBegin() and glEnd() call and other widely used functions,
and generally it will take a lot of effort to port pure OpenGL application to GL ES.

When porting you own app, first of all ensure that your application supports
native RGB_565 pixel format and AUDIO_S8 or AUDIO_S16 audio format
(yes, there is RGB_565 pixelformat even for OpenGL, not BGR_565 as all other OpenGL implementation have).
HTC G1/Nexus One has native screen resolution 480x320, HTC Evo has 800x480, so design your app to support
any screen resolution.
SDL_ListModes()[0] will always return native screen resolution, you may use 640x480 or 800x600
but it will be resized to fit the screen.
Also make sure that your HW textures are not wider than 1024 pixels, or it will fail to allocate such
texture on HTC G1. Software surfaces may be of any size of course (but you don't want to do expensive memcpy).
There is a trick to make 

To compile your own app, put your app sources into project/jni/application dir (remove Alien Blaster first),
and launch script ChangeAppSettings.sh - it will put the name of your app in several places in sources.
The C++ files shall have .cpp extension to be compiled, rename them if necessary.
Then you should launch script ChangeAppSettings.sh - it will ask you few questions,
and change several values across project files.
Also you can replace icon image at project/res/drawable/icon.png.
Then you can launch build.sh and hope for the best.

Application data is not bundled with app itself - it should be downloaded from net on first run.
Create .ZIP file with your application data, and put it somewhere on HTTP server - ChangeAppSettings.sh
will ask you for the URL.
If your app data is bigger than 5 megabytes it's better to store it on SD card,
internal flash on Android is very limited.
If you'll release new version of data files you should change download URL and update your app as well -
the app will re-download the data if URL does not match the saved URL from previous download.

If you'll add new libs - add them to project/jni/, copy Android.mk from existing lib, and
add libname to project/jni/<yourapp>/Android.mk

To debug your application add tag 'android:debuggable="true"' to 'application' element in AndroidManifest.xml,
recmpile and reinstall your app to Android 2.2 emulator or Android 2.2 device, go to "project" dir and launch command
	ndk-gdb --verbose --start --force
then when it fails enter command
	target remote:5039 (then it will fail again)
Note that it's extremely buggy, and I had no any success in debugging my app with ndk-gdb.
So it's best to debug with code like:
	__android_log_print(ANDROID_LOG_INFO, "My App", "We somehow reached execution point #224");
and then watching "adb logcat" output.

Known bugs:

0. Revert to widely-used SDL 1.2, backport changes that enable hardware acceleration from SDL 1.3
Ideally ChangeAppSettings.sh should ask you what SDL version you want to use.

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

This is libSDL 1.2 and 1.3 ported to Google Android (also bunch of other libs included).
Alien Blaster game is used as working example (original sources: http://www.schwardtnet.de/alienblaster/ ).

Installation
============

This should be compiled with Android 2.2 SDK and NDK r4b - google for them and install them as described in their docs
(the application will run on Android 1.6 and above).
You'll need to install Ant too.
The most supported environnment for that port is Linux, MacOs should be okay too, 
however if you'll use launchConfigure.sh script you'll have to replace "linux-x86" to "darwin-x86" inside it.
If you're developing on Windows you'd better install andLinux or Ubuntu+Wubi, to get proper Linux environment
running inside Windows, then install Linux toolchain on it. I was told andLinux compiles faster than Cygwin.
Also you'll need full set of Linux utils and symlinks support to launch ChangeAppSettings.sh (sh, grep, sed, tr).

How to compile Alien Blaster demo application
=============================================

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
Back key is mapped to Escape, and Menu is mapped to Enter.
Newer Android phones like HTC Evo have no keyboard at all, so there are just 3 usable keys - 
Menu, Volume Up and Volume Down (and Escape of course).
Because of that the accelerometer is configured to trigger cursor key events.

This port also supports GL ES + SDL combo - there is GLXGears demo app in project/jni/application/glxgears,
remove project/jni/application/src symlink and make new one pointing to glxgears, 
also you'll have to enable Z-Buffer in ChangeAppSettings.sh.
Note that GL ES is NOT pure OpenGL - there are no glBegin() and glEnd() call and other widely used functions,
and generally it will take a lot of effort to port pure OpenGL application to GL ES.

How to compile your own application
===================================

When porting you own app, first of all ensure that your application supports
native RGB_565 pixel format and AUDIO_S8 or AUDIO_S16 audio format
(there is RGB_565 pixelformat even for OpenGL, not BGR_565 as all other OpenGL implementation have).
Colorkey images are supported using RGBA_5551 pixelformat with 1-bit alpha (SDL does conversion internally,
for you they are just RGB_565 surfaces), alpha surfaces have RGBA_4444 format. 
See file project/jni/application/alienblaster/SdlForwardCompat.h
to learn how to make your application use SDL 1.3 instead of SDL 1.2 without much pain.
HTC G1/Nexus One has native screen resolution 480x320, HTC Evo has 800x480, you may toggle automatic
screen resizing in ChangeAppSettings.sh and draw to virtual 640x480 screen - 
it will be HW accelerated and will not impact performance much.
SDL_ListModes()[0] will always return native screen resolution.
Also make sure that your HW textures are not wider than 1024 pixels, or it will fail to allocate such
texture on HTC G1. Software surfaces may be of any size of course (but you don't want to do expensive memcpy).

Alternatively, SDL 1.2 is available too, you may use it with SW video as usual, however if you want HW acceleration
there are few restrictions: you cannot currently blit SW surface to screen, it should be only HW surface,
also alpha-surfaces seem to not work (did not check it thoroughly) - you still can use per-surface alpha.
Basically your code should be like:

// Init HW-accelerated video
SDL_SetVideoMode( 640, 480, 16, SDL_DOUBLEBUF | SDL_HWSURFACE );
// Load graphics
SDL_Surface *sprite = IMG_Load( "sprite.png" );
// Set pink color as transparent
SDL_SetColorKey( sprite, SDL_SRCCOLORKEY, SDL_MapRGB(sprite->format, 255, 0, 255) );
// Create HW-accelerated surface
SDL_Surface * hwSprite = SDL_DisplayFormat(sprite);
// Set per-surface alpha, if necessary
SDL_SetAlpha( hwSprite, SDL_SRCALPHA, 128 );
// Blit it in HW-accelerated way
SDL_BlitSurface(hwSprite, sourceRect, SDL_GetVideoSurface(), &targetRect);
// Wrong, blitting SW surfaces to screen not supported
SDL_BlitSurface(sprite, sourceRect, SDL_GetVideoSurface(), &targetRect);
// Wrong, copying from video surface not supported
SDL_BlitSurface(SDL_GetVideoSurface(), sourceRect, sprite, &targetRect);

To compile your own app, put your app sources into project/jni/application dir, and change symlink "src"
to point to your app, then launch script ChangeAppSettings.sh - it will ask few questions and modify some Java code.
The C++ files shall have .cpp extension to be compiled, rename them if necessary.
Also you can replace icon image at project/res/drawable/icon.png.
Then you can launch build.sh.

The NDK has RTTI and exceptions disabled for C++ code, if you need them you may download modified NDK from
http://www.crystax.net/android/ndk-r4.php
Unzip it, and put in your PATH instead of original NDK - do not rename the target dir, my makefiles will
check if there's "crystax" string in path to gcc toolchain, and will disable STLPort because CrystaX's
NDK already contains STL library.

Application data is not bundled with app itself - it should be downloaded from net on first run.
Create .ZIP file with your application data, and put it somewhere on HTTP server - ChangeAppSettings.sh
will ask you for the URL.
If you'll release new version of data files you should change download URL and update your app as well -
the app will re-download the data if URL does not match the saved URL from previous download.

If you'll add new libs - add them to project/jni/, copy Android.mk from existing lib, and
add libname to project/jni/<yourapp>/Android.mk

How to compile your own application using automake/configure scripts
====================================================================

There is limited support for "configure" scripts, I've managed to compile lbreakout2 that way,
though ./configure scripts tend to have stupid bugs in various places, avoid using that method if you can.
1. Download lbreakout2-2.6.1.tar.gz from http://lgames.sourceforge.net/, unpack it to project/jni/application dir.
2. Determine libraries needed for your app, launch ChangeAppSettings.sh, select correct libSDL version
   (1.2 for lbreakout2), and correct libs (sdl_mixer sdl_image sdl_net for lbreakout2), also change name etc.
3. Launch ./build.sh, wait till it builds all .so files
4. Go to project/jni/application/lbreakout2-2.6.1 dir, and launch command
   ../launchConfigure.sh --disable-install --enable-sdl-net LIBS=-lintl
5. Watch how ./configure configures, if it fails fix launchConfigure.sh, rinse and repeat.
6. Launch make, and pray. If you're lucky it will create application binary (lbreakout2-2.6.1/client/lbreakout2)
7. Move the application binary to dir project/libs/armeabi, rename it to libapplication.so (overwrite old file)
8. Run command "arm-eabi-strip --strip-debug libapplication.so", you can find arm-eabi-strip under your NDK dir.
9. Run "ant debug" or "ant release" from project dir, install to device & enjoy.

Quick guide to debug native code
================================

To debug your application add tag 'android:debuggable="true"' to 'application' element in AndroidManifest.xml,
recmpile and reinstall your app to Android 2.2 emulator or Android 2.2 device, go to "project" dir and launch command
	ndk-gdb --verbose --start --force
then when it fails enter command
	target remote:5039 (then it will fail again)
Note that it's extremely buggy, and I had no any success in debugging my app with ndk-gdb.
So it's best to debug with code like:
	__android_log_print(ANDROID_LOG_INFO, "My App", "We somehow reached execution point #224");
and then watching "adb logcat" output.

If your application crashed, you should use following steps:

1. Gather the crash report from "adb logcat" - it should contain stack trace, if it does not then you're unlucky,

I/DEBUG   (   51): *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
I/DEBUG   (   51): Build fingerprint: 'sprint/htc_supersonic/supersonic/supersonic:2.1-update1/ERE27/194487:userdebug/test-keys'
I/DEBUG   (   51): pid: 915, tid: 924  >>> de.schwardtnet.alienblaster <<<
I/DEBUG   (   51): signal 11 (SIGSEGV), fault addr deadbaad
I/DEBUG   (   51):  r0 00000000  r1 afe133f1  r2 00000027  r3 00000058
I/DEBUG   (   51):  r4 afe3ae08  r5 00000000  r6 00000000  r7 70477020
I/DEBUG   (   51):  r8 000000b0  r9 ffffff20  10 48552868  fp 00000234
I/DEBUG   (   51):  ip 00002ee4  sp 485527f8  lr deadbaad  pc afe10aac  cpsr 60000030
I/DEBUG   (   51):          #00  pc 00010aac  /system/lib/libc.so
I/DEBUG   (   51):          #01  pc 0000c00e  /system/lib/libc.so
I/DEBUG   (   51):          #02  pc 0000c0a4  /system/lib/libc.so
I/DEBUG   (   51):          #03  pc 0002ca00  /data/data/de.schwardtnet.alienblaster/lib/libsdl.so
I/DEBUG   (   51):          #04  pc 00028b6e  /data/data/de.schwardtnet.alienblaster/lib/libsdl.so
I/DEBUG   (   51):          #05  pc 0002d080  /data/data/de.schwardtnet.alienblaster/lib/libsdl.so

2. Go to project/bin/ndk/local/armeabi dir, find there the library mentioned in stacktrace 
(libsdl.so in our example), copy the address of the first line of stacktrace (0002ca00), and execute command

gdb libsdl.so -ex "list *0x0002ca00"

It will output the exact line in your source where the application crashed.

Android Application lifectcle support
=====================================

Application may be put to background at any time, for example if user gets phone call onto the device.
The application will lose OpenGL context then, and has to re-create it when put to foreground.

The SDL provides function 
SDL_ANDROID_SetApplicationPutToBackgroundCallback( callback_t appPutToBackground, callback_t appRestored );
where callback_t is function pointer of type "void (*) void".
The default callbacks will call another Android-specific functions:
SDL_ANDROID_PauseAudioPlayback() and SDL_ANDROID_ResumeAudioPlayback()
which will pause and resume audio from HW layer, so appplication does not need to destroy and re-init audio.

If you're using pure SDL 1.2 API (with or without HW acceleration) you don't need to worry about anything -
the SDL itself will re-create GL textures and fill them with pixel data from existing SDL HW surfaces,
so you may leave the callbacks to defaults.

If you're using SDL 1.3 API and using SDL_Texture, then the textures pixeldata is lost - you will need 
to call SDL_UpdateTexture() to refill texture pixeldata from appRestored() callback for all your textures.
If you're using compatibility API with SDL_Surfaces you don't have to worry about that.

If you're using SDL with OpenGL with either SDL 1.2 or SDL 1.3, the situation is even more grim -
not only all your GL textures are lost, but all GL matrices, blend modes, etc. has to be re-created.

OS may decide there's too little free RAM left on device, and kill background applications 
without notice, so it vill be good to create temporary savegame etc. from appPutToBackground() callback.

Also it's a good practice to pause any application audio, especially if the user gets phone call,
and if you won't set your own callbacks the default callbacks will do exactly that.
There are circumstances when you want to avoid that, for example if the application is audio player,
or if application gets some notification over network (for example you're running a game server,
and want a beep when someone connects to you) - you may unpause audio for some short time then.

The application is not allowed to do any GFX output without OpenGL context (or it will crash),
that's why SDL_Flip() call will block until we're re-acquired context, and the callbacks will be called 
from inside SDL_Flip(). so you won't receive SDL_WINDOWEVENT_HIDDEN / SDL_WINDOWEVENT_SHOWN,
because if SDL sends them the application will get them only after SDL_Flip() successfully
re-acquired GL context, and it's too late to pause audio and save application state,
so please use callbacks instead of SDL window events on Android OS.

The whole idea behind callbacks is that the existing application should not be modified to
operate correctly - the whole time in background will just look to app as one very long SDL_Flip(),
so it's good idea to implement some maximum time cap on game frame, so it won't process
the game to the end level 'till the app is in background, or calculate the difference in time
between appPutToBackground() and appRestored() and update game time variables.

Known bugs
==========

1. Merge all config screens into single big config screen, make option to rerun config.

2. Fix on-screen keyboard, add more keys and more options, make possible for application to control it.

3. Add full QWERTY on-screen keyboard.

4. Add trackball sensitivity and accelerometer sensitivity config.

5. Export phone vibrator to SDL - interface is available in SDL 1.3

6. HDMI output (HTC Evo and Samsung Epic support that):
HDMI output will be tricky - I've read the doc here: 
https://docs.google.com/View?id=dhtsnvs6_57d2hpqtgr#4_1_HDMI_output_support_338048
It says that in order to output something to HDMI you need to have a VideoView class visible on screen: 
http://developer.android.com/reference/android/widget/VideoView.html .
This class does not have any method like "showMyOwnCustomImage()", 
it just plays the video from the given path, so obvious solution is to create 
special FIFO file or open a socket, point the VideoView to play this FIFO or socket, 
and then write raw uncompressed video frames to that FIFO with some small header so that 
VideoView will recognize it as a proper video format.
UQM gives 5 FPS without such hacks, if I'll implement that FPS will drop to 1-2 
(maybe not that bad, I have to actually try that), because you'll have to do huge memcpy(), 
plus VideoView will contain some buffer to ensure the playback is smooth, 
so the data on your TV will lag halfsecond behind the data on the device screen.

7. Make app data to come inside .apk file in assets instead of downloading it from net.

8. OpenTyrian: 
  1. Navigating game menus downwards with trackball skips events, that does not happen
     when navigting upwards.
  2. The detail level can be set to WILD by pressing "w" key in gameplay escape menu, expose that through interface.

9. Ur-Quan Masters: add Russian, Deutsch and Slovak translations: http://wiki.uqm.stack.nl/Translations

Games to port
=============

TeeWorlds
SuperTux
LBreakout2
Commander Genius (only data files for shareware version available for free)
OpenJazz (only data files for shareware version available for free)
OpenLieroX (will be damn hard to do, I wrote the code partially)
ScummVM (they already have their own port, yet it's unfinished)
Widelands (http://wl.widelands.org/)

License information
===================

The libSDL port itself is licensed under LGPL.
It contains separate libraries under project/jni, each of which has it's own license,
I've tried to compile all LGPL-ed libs as shared libs but you should anyway inspect the licenses
of the libraries you're linking to if you're creating closed-source app.

The "Ultimate Droid" button theme by Sean Stieber is licensed under Creative Commons - Attribution license.

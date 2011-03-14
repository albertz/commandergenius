This is libSDL 1.2 and 1.3 ported to Google Android (also bunch of other libs included).

Installation
============

This should be compiled with Android 2.2 SDK (API level 8) and NDK r4b,
google for them and install them as described in their docs.
The application will run on Android OS 1.6 and above, don't mind the 2.2 dependency.
If you need support for C++ RTTI and exceptions you may use CrystaX NDK based on r4b:
http://www.crystax.net/android/ndk-r4.php
NDK r5b WILL CRASH on Android OS lower than 2.2, however SDL build system supports it.
You'll need to install Java Ant too.
The most supported environment for this port is Linux, MacOs should be okay too.
If you're developing under Windows you'd better install andLinux or Ubuntu+Wubi, to get proper Linux environment
running inside Windows, then install Linux toolchain on it. I was told andLinux compiles faster than Cygwin.
Also you'll need full set of Linux utils and symlinks support to launch ChangeAppSettings.sh (sh, grep, sed, tr).
http://www.pocketmagic.net/?p=1332 - guide how to set up environment in Cygwin.

How to compile demo application
===============================

Launch commands
	rm project/jni/application/src
	ln -s ballfield project/jni/application/src
	ChangeAppSettings.sh -a
	android update project -p project
Then edit file build.sh if needed to add NDK dir to your PATH, then launch it.
It will compile a bunch of libs under project/libs/armeabi,
create file project/bin/DemoActivity-debug.apk and install it on your device or emulator.
Then you can test it by launching Ballfield icon from Android applications menu.
It's designed for 320x240, so if you have smaller screen it will be resized.

The game enforces horizontal screen orientation, you may slide-open your keyboard if you have it
and use it for additional keys - the device will just keep current screen orientation.
Recent Android phone models like HTC Evo have no keyboard at all, on-screen keyboard built into libSDL
is available for such devices - it has joystick (which can be configured as arrow buttons or analog joystick),
and 6 configurable keys, full text input is toggled with 7-th key. Both user and application may redefine
button layout and returned keycodes, and also toggle full text input - see SDL_screenkeyboard.h.

This port also supports GL ES + SDL combo - there is GLXGears demo app in project/jni/application/glxgears,
remove project/jni/application/src symlink and make new one pointing to glxgears, then run build.sh
Note that GL ES is NOT pure OpenGL - there are no glBegin() and glEnd() call and other widely used functions,
and generally it will take a lot of effort to port pure OpenGL application to GL ES.

How to compile your own application
===================================

You may find quick Android game porting manual at http://anddev.at.ua/src/porting_manual.txt

If you're porting existing app which uses SDL 1.2 please always use SW mode:
neither SDL_SetVideoMode() call nor SDL_CreateRGBSurface() etc functions shall contain SDL_HWSURFACE flags.
The BPP in SDL_SetVideoMode() shall be set to 16, and audio format - to AUDIO_S8 or AUDIO_S16.

The native Android pixel format is RGB_565, even for OpenGL, not BGR_565 as all other OpenGL implementation have.

Colorkey images are supported using RGBA_5551 pixelformat with 1-bit alpha -
SDL does conversion internally, for you they are just RGB_565 surfaces.
Alpha surfaces have RGBA_4444 format.

To compile your own app, put your app sources into project/jni/application dir (or create symlink to them),
and change symlink "src" to point to your app:

	cp -r /path/to/my/app project/jni/application/myapp
or
	ln -s /path/to/my/app project/jni/application/myapp
then
	rm project/jni/application/src
	ln -s myapp project/jni/application/src
(the second one should be relative link without slashes)

Also your main() function name should be redefined to SDL_main(), if you'll include SDL.h it will do it automatically.

Then launch script ChangeAppSettings.sh - it will ask few questions and copy some Java files - 
there's no way around it, because Java does not support preprocessor.
You may take AndroidAppSettings.cfg file from some other application to get some sane defaults,
you may launch ChangeAppSettings.sh with -a or -v parameter to skip questions altogether or to ask only version code.
The C++ files shall have .cpp extension to be compiled, rename them if necessary.
Also you can replace icon image at project/res/drawable/icon.png and image project/res/drawable/publisherlogo.png.
Then you can launch build.sh.

Application data may be bundled with app itself, or downloaded from net on first run.
Create .ZIP file with your application data, and put it on HTTP server, or to "project/jni/application/src/AndroidData" dir -
ChangeAppSettings.sh will ask you for the URL, if URL won't contain "http://" it will try to unzip file from AndroidData dir.
Note that there is limit on maximum .APK file size on Market, like 50 Mb or so, so big files should be downloaded by HTTP.
Also many older devices cannot extract files bigger than 1 Mb from .apk container, so it's strongly advised to split
all your data to several small zipfiles if you're putting it inside .apk.
Also you may specify additional downloads through ChangeAppSettings.sh, such as hi-res texture pack etc.
If you'll release new version of data files you should change download URL or data file name and update your app as well -
the app will re-download the data if URL does not match the saved URL from previous download.

All devices have different screen resolutions, you may toggle automatic
screen resizing in ChangeAppSettings.sh and draw to virtual 640x480 screen -
it will be HW accelerated and will not impact performance much.
SDL_ListModes()[0] will always return native screen resolution.
Also make sure that your HW textures are not wider than 1024 pixels, or it will fail to allocate such
texture on HTC G1, and many other low-end devices. Software surfaces may be of any size of course.

If you want HW acceleration there are some limitations:
You cannot blit SW surface to screen, it should be only HW surface.
You can use colorkey, per-surface alpha and per-pixel alpha with HW surfaces.
If you're using SDL 1.3 always use SDL_Texture, if you'll be using SDL_Surface with SDL 1.3 it will switch to SW mode.
Also the screen is always double-buffered, and after each SDL_Flip() there is garbage in pixel buffer,
so forget about dirty rects and partial screen updates - you have to re-render whole picture each frame.
Single-buffer rendering might be possible with techniques like glFramebufferTexture2D(),
however it is not present on all devices, so I won't do that.
Basically your code should be like this for SDL 1.2:

// ----- HW-accelerated video output for Android example
// Init HW-accelerated video
SDL_SetVideoMode( 640, 480, 16, SDL_DOUBLEBUF | SDL_HWSURFACE );

// Load graphics
SDL_Surface *sprite = IMG_Load( "sprite.png" );
SDL_Surface * hwSprite;

if( sprite->format->Amask )
{
	// Surface contains per-pixel alpha, convert it to HW-accelerated format
	hwSprite = SDL_DisplayFormatAlpha(sprite);
}
else
{
	// Set pink color as transparent
	SDL_SetColorKey( sprite, SDL_SRCCOLORKEY, SDL_MapRGB(sprite->format, 255, 0, 255) );
	// Create HW-accelerated surface
	hwSprite = SDL_DisplayFormat(sprite);
	// Set per-surface alpha, if necessary
	SDL_SetAlpha( hwSprite, SDL_SRCALPHA, 128 );
}

// Blit it in HW-accelerated way
SDL_BlitSurface(hwSprite, sourceRect, SDL_GetVideoSurface(), &targetRect);

// Render the resulting video frame to device screen
SDL_Flip(SDL_GetVideoSurface());

// Supported, but VERY slow (slower than blitting in SW mode)
SDL_BlitSurface(sprite, sourceRect, SDL_GetVideoSurface(), &targetRect);

// Supported, but VERY slow (use in cases where you need to take a screenshot)
SDL_BlitSurface(SDL_GetVideoSurface(), sourceRect, sprite, &targetRect);

// ----- End of example

If you'll add new libs - add them to project/jni/, copy Android.mk from existing lib, and
add libname to project/jni/<yourapp>/Android.mk
Also you'll need to move all include files to <libname>/include dir.
If lib contains "configure" script - go to lib dir and execute command "../launchConfigureLib.sh" - it will
launch "configure" with appropriate environment and will create the "config.h" file at least, though linking
will most probably fail because of ranlib - just edit Android.mk to compile lib sources and remove all tools and tests.

MIDI support can be emulated via SDL_mixer lib (it uses Timidity internally)- download file
http://www.libsdl.org/projects/mixer/timidity/timidity.tar.gz
unpack it and put "timidity" dir into your game data zipfile.
Or you may paste this URL directly as an optional download in ChangeAppSettings.sh:
MIDI music support (18 Mb)|http://sourceforge.net/projects/libsdl-android/files/timidity.zip/download

The ARM architecture has some limitations which you have to be aware about -
if you'll access integer that's not 4-byte aligned you'll get garbage instead of correct value,
and it's processor-model specific - it may work on some devices and do not work on another ones -
you may wish to check your code in Android 1.6 emulator from time to time to catch such bugs.

char * p = 0x13; // Non-4 byte aligned pointer
int i = (int *) p; // We have garbage inside i now
memcpy( &i, p, sizeof(int) ); // The correct way to dereference a non-aligned pointer

This compiler flags will catch most obvious errors, you may add them to AppCflags var in settings:
-Werror=strict-aliasing -Werror=cast-align -Werror=pointer-arith -Werror=address

The application will automatically get moved to SD-card on Android 2.2 or newer,
(or you can install app2sd for older, but rooted phones),
however the shared libraries have to be stored on the device internal storage,
and that may be not desired for older phones with very little storage.
The script app2sd.sh will re-package your .apk file in such a way that
the shared libraries will not be extracted by Android OS but by application itself,
and it will remove them from internal storage right after starting up,
so you still need that space free, but only temporarily. 
However your application will start up slower.


How to compile your own application using automake/configure scripts
====================================================================

There is limited support for "configure" scripts, I'm compiling scummvm this way,
though ./configure scripts tend to have stupid bugs in various places, avoid using that method if you can.
You should enable custom build script in ChangeAppSettings.sh, and you should create script
AndroidBuild.sh and put it under project/jni/application/src dir. The AndroidBuild.sh script should 
generate file project/jni/application/src/libapplication.so, which will be copied into .apk file by build system.
There is helper script project/jni/application/setEnvironment.sh which will set CFLAGS and LDFLAGS
for configure script and makefile, see AndroidBuild.sh in project/jni/application/scummvm dir for reference.

Android Application lifecycle support
=====================================

Application may be put to background at any time, for example if user gets phone call onto the device.
The application will lose OpenGL context then, and has to re-create it when put to foreground.

The SDL provides function 
SDL_ANDROID_SetApplicationPutToBackgroundCallback( callback_t appPutToBackground, callback_t appRestored );
where callback_t is function pointer of type "void (*) void".
The default callbacks will call another Android-specific functions:
SDL_ANDROID_PauseAudioPlayback() and SDL_ANDROID_ResumeAudioPlayback()
which will pause and resume audio from HW layer, so appplication does not need to destroy and re-init audio.
Also, the usual event SDL_ACTIVEEVENT with flag SDL_APPACTIVE will be sent when that happens,
and also SDL_VIDEORESIZE event will be sent (the same behavior as in MacOsX SDL implementation).
If you're using OpenAL for an audio playback you have to call functions al_android_pause_playback()
and al_android_resume_playback() by yourself when SDL calls your callbacks.

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
and want a beep when someone connects to you) - you may unpause audio for some short time,
that will require another thread to watch the network, because main thread will be blocked inside SDL_Flip().

The application is not allowed to do any GFX output without OpenGL context (or it will crash),
that's why SDL_Flip() call will block until we're re-acquired context, and the callbacks will be called
from inside SDL_Flip().
The whole idea behind callbacks is that the existing application should not be modified to
operate correctly - the whole time in background will just look to app as one very long SDL_Flip(),
so it's good idea to implement some maximum time cap on game frame, so it won't process
the game to the end level 'till the app is in background, or calculate the difference in time
between appPutToBackground() and appRestored() and update game time variables.

Alternatively, you may enable option for unblocked SDL_Flip() in ChangeAppSettings script,
then you'll have to implement special event loop right after each SDL_Flip() call:

SDL_Flip(SDL_GetVideoSurface());
SDL_Event evt;
while( SDL_PollEvent(&evt) )
{
	if( evt.type == SDL_ACTIVEEVENT && evt.active.gain == 0 && evt.active.state & SDL_APPACTIVE )
	{
		// We've lost GL context, we are not allowed to do any GFX output here, or app will crash!
		while( 1 )
		{
			SDL_PollEvent(&evt);
			if( evt.type == SDL_ACTIVEEVENT && evt.active.gain && evt.active.state & SDL_APPACTIVE )
			{
				SDL_Flip(SDL_GetVideoSurface()); // One SDL_Flip() call is required here to restore OpenGL context
				// Re-load all textures, matrixes and all other GL states if we're in SDL+OpenGL mode
				// Re-load all images to SDL_Texture if we're using it
				// Now we can draw
				break;
			}
			// Process network stuff, maybe play some sounds using SDL_ANDROID_PauseAudioPlayback() / SDL_ANDROID_ResumeAudioPlayback()
			SDL_Delay(300);
		}
	}
}

Note that I did not test that code yet, so test reports are appreciated.

Quick guide to debug native code
================================

The debugging of multi-threaded apps is not supported with NDK r4 or r4b, you'll need NDK r5b and Android 2.3 emulator or device.
To debug your application add tag 'android:debuggable="true"' to 'application' element in AndroidManifest.xml,
recmpile and reinstall your app, go to "project" dir and launch command
	ndk-gdb --verbose --start --force
then if it fails enter command
	target remote:5039
You can also debug by adding extensive logs to your app:
	__android_log_print(ANDROID_LOG_INFO, "My App", "We somehow reached execution point #224");
and then watching "adb logcat" output.
You may wish to uncomment line
#define printf __SDL_android_printf
inside file project/jni/SDL-1.3/include/SDL_android_printf.h - this will make printf() to output to logcat,
however it will trigger lot of warnings in system headers, so it's disabled by default.

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

<your NDK path>/build/prebuilt/linux-x86/arm-eabi-4.4.0/bin/arm-eabi-gdb libsdl.so -ex "list *0x0002ca00"

It will output the exact line in your source where the application crashed.

If your application does not work for unknown reasons, there may be the case when it exports some symbol
that clash with exports from system libraries - run checkExports.sh to check this.

If your application fails to load past startup SDL logo with error

I/dalvikvm( 3401): Unable to dlopen(/data/data/com.svc/lib/libapplication.so): Cannot load library: alloc_mem_region[847]: OOPS:  1268 cannot map library 'libapplication.so'. no vspace available.

that means you're allocating huge data buffer in heap (that may be C static or global buffer variable).
Use command "objdump -x libapplication.so", it might output something like this:

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
 13 .bss          0b64544c  00051670  00051670  0005066c  2**3

and below

0848c8c8 g     O .bss   0320a000 decoder_svc_PictureBuffer_RefY

which means your BSS segment eats up 191 Mb of RAM, and symbol 'decoder_svc_PictureBuffer_RefY' eats up 52 Mb,
while heap memory limit on most phones is 24 Mb.

License information
===================

The libSDL port itself is licensed under LGPL, so you may use it for commercial app without releasing sources,
however you'll have to release the file AndroidAppSettings.cfg to allow linking newer version of libSDL with
your compiled application, as LGPL requires.
It contains separate libraries under project/jni, each of which has it's own license,
I've tried to compile all LGPL-ed libs as shared libs but you should anyway inspect the licenses
of the libraries you're linking to if you're creating closed-source app.
libmad and liblzo2 are licensed under GPL, so if you're planning to make commercial app you should avoid
using them, otherwise you'll have to release your application sources under GPL too.

The "Ultimate Droid" button theme by Sean Stieber is licensed under Creative Commons - Attribution license.

#!/bin/sh

AUTO=a
CHANGED=
JAVA_SRC_PATH=project/java

if [ "X$1" = "X-a" ]; then
	AUTO=a
	shift
fi
if [ "X$1" = "X-v" ]; then
	AUTO=v
	shift
fi
if [ "X$1" = "X-u" ]; then
	CHANGED=1
	AUTO=a
	shift
fi
if [ "X$1" = "X-h" ]; then
	echo "Usage: $0 [-a] [-v] [-u]"
	echo "       -a: auto-update project files without asking questions, it's the default action"
	echo "       -v: ask for new version number on terminal"
	echo "       -u: update AndroidAppSettings.cfg, this may add new config options to it"
	exit
fi

if [ "$#" -gt 0 ]; then
	echo "Switching build target to $1"
	if [ -e project/jni/application/$1 ]; then
		rm -f project/jni/application/src
		ln -s "$1" project/jni/application/src
	else
		echo "Error: no app $1 under project/jni/application"
		echo "Available applications:"
		cd project/jni/application
		for f in *; do
			if [ -e "$f/AndroidAppSettings.cfg" ]; then
				echo "$f"
			fi
		done
		exit 1
	fi
	shift
fi

. ./AndroidAppSettings.cfg

var=""

if [ "$LibSdlVersion" = "2.0" ]; then
    JAVA_SRC_PATH=project/javaSDL2 
fi

if [ "$CompatibilityHacks" = y ]; then
	SwVideoMode=y
fi

if [ "$SwVideoMode" = "y" ]; then
	NeedDepthBuffer=n
	NeedStencilBuffer=n
	NeedGles2=n
fi


if [ "$AppUsesJoystick" != "y" ]; then
	AppUsesSecondJoystick=n
fi

MenuOptionsAvailable=
for FF in Menu MenuMisc MenuMouse MenuKeyboard ; do
	MenuOptionsAvailable1=`grep 'extends Menu' $JAVA_SRC_PATH/Settings$FF.java | sed "s/.* class \(.*\) extends .*/Settings$FF.\1/" | tr '\n' ' '`
	MenuOptionsAvailable="$MenuOptionsAvailable $MenuOptionsAvailable1"
done

FirstStartMenuOptionsDefault='new SettingsMenuMisc.ShowReadme(), (AppUsesMouse \&\& \! ForceRelativeMouseMode \? new SettingsMenuMouse.DisplaySizeConfig(true) : new SettingsMenu.DummyMenu()), new SettingsMenuMisc.OptionalDownloadConfig(true), new SettingsMenuMisc.GyroscopeCalibration()'

if [ -z "$CompatibilityHacksForceScreenUpdate" ]; then
	CompatibilityHacksForceScreenUpdate=$CompatibilityHacks
fi

if [ -z "$CompatibilityHacksForceScreenUpdateMouseClick" ]; then
	CompatibilityHacksForceScreenUpdateMouseClick=y
fi

if [ -z "$TouchscreenKeysTheme" ]; then
	TouchscreenKeysTheme=2
fi


if [ -z "$AppVersionCode" -o "-$AUTO" != "-a" ]; then
echo
echo -n "Application version code (integer) ($AppVersionCode): "
read var
if [ -n "$var" ] ; then
	AppVersionCode="$var"
	CHANGED=1
fi
fi

if [ -z "$AppVersionName" -o "-$AUTO" != "-a" ]; then
echo
echo -n "Application user-visible version name (string) ($AppVersionName): "
read var
if [ -n "$var" ] ; then
	AppVersionName="$var"
	CHANGED=1
fi
fi

if [ -z "$ResetSdlConfigForThisVersion" -o "-$AUTO" != "-a" ]; then
echo
echo -n "Reset SDL config when updating application to the new version (y) / (n) ($ResetSdlConfigForThisVersion): "
read var
if [ -n "$var" ] ; then
	ResetSdlConfigForThisVersion="$var"
	CHANGED=1
fi
fi

if [ "-$AUTO" != "-a" ]; then
echo
echo -n "Delete application data files when upgrading (specify file/dir paths separated by spaces): ($DeleteFilesOnUpgrade): "
read var
if [ -n "$var" ] ; then
	DeleteFilesOnUpgrade="$var"
	CHANGED=1
fi
fi

if [ -n "$CHANGED" ]; then
cat /dev/null > AndroidAppSettings.cfg
echo "# The application settings for Android libSDL port" >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Specify application name (e.x. My Application)" >> AndroidAppSettings.cfg
echo AppName=\"$AppName\" >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Specify reversed site name of application (e.x. com.mysite.myapp)" >> AndroidAppSettings.cfg
echo AppFullName=$AppFullName >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application version code (integer)" >> AndroidAppSettings.cfg
echo AppVersionCode=$AppVersionCode >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application user-visible version name (string)" >> AndroidAppSettings.cfg
echo AppVersionName=\"$AppVersionName\" >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Specify path to download application data in zip archive in the form 'Description|URL|MirrorURL^Description2|URL2|MirrorURL2^...'" >> AndroidAppSettings.cfg
echo "# If you'll start Description with '!' symbol it will be enabled by default, other downloads should be selected by user from startup config menu" >> AndroidAppSettings.cfg
echo "# If the URL in in the form ':dir/file.dat:http://URL/' it will be downloaded as binary BLOB to the application dir and not unzipped" >> AndroidAppSettings.cfg
echo "# If the URL does not contain 'http://' it is treated as file from 'project/jni/application/src/AndroidData' dir -" >> AndroidAppSettings.cfg
echo "# these files are put inside .apk package by build system" >> AndroidAppSettings.cfg
echo "# You can specify Google Play expansion files in the form 'obb:main.12345' or 'obb:patch.12345' where 12345 is the app version, first associated with the file" >> AndroidAppSettings.cfg
echo AppDataDownloadUrl=\"$AppDataDownloadUrl\" >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Reset SDL config when updating application to the new version (y) / (n)" >> AndroidAppSettings.cfg
echo ResetSdlConfigForThisVersion=$ResetSdlConfigForThisVersion >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Delete application data files when upgrading (specify file/dir paths separated by spaces)" >> AndroidAppSettings.cfg
echo DeleteFilesOnUpgrade=\"$DeleteFilesOnUpgrade\" >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Here you may type readme text, which will be shown during startup. Format is:" >> AndroidAppSettings.cfg
echo "# Text in English, use \\\\\\\\\\\\\\\\n to separate lines (that's four backslashes)^de:Text in Deutsch^ru:Text in Russian^button:Button that will open some URL:http://url-to-open/" >> AndroidAppSettings.cfg
echo ReadmeText=\'$ReadmeText\' | sed 's/\\\\n/\\\\\\\\n/g' >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# libSDL version to use (1.2/1.3/2.0)" >> AndroidAppSettings.cfg
echo LibSdlVersion=$LibSdlVersion >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Specify screen orientation: (v)ertical/(p)ortrait or (h)orizontal/(l)andscape" >> AndroidAppSettings.cfg
echo ScreenOrientation=$ScreenOrientation >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Video color depth - 16 BPP is the fastest and supported for all modes, 24 bpp is supported only" >> AndroidAppSettings.cfg
echo "# with SwVideoMode=y, SDL_OPENGL mode supports everything. (16)/(24)/(32)" >> AndroidAppSettings.cfg
echo VideoDepthBpp=$VideoDepthBpp >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Enable OpenGL depth buffer (needed only for 3-d applications, small speed decrease) (y) or (n)" >> AndroidAppSettings.cfg
echo NeedDepthBuffer=$NeedDepthBuffer >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Enable OpenGL stencil buffer (needed only for 3-d applications, small speed decrease) (y) or (n)" >> AndroidAppSettings.cfg
echo NeedStencilBuffer=$NeedStencilBuffer >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Try to use GLES 2.x context - will revert to GLES 1.X if unsupported by device" >> AndroidAppSettings.cfg
echo "# you need this option only if you're developing 3-d app (y) or (n)" >> AndroidAppSettings.cfg
echo NeedGles2=$NeedGles2 >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Use glshim library for provide OpenGL 1.x functionality to OpenGL ES accelerated cards (y) or (n)" >> AndroidAppSettings.cfg
echo UseGlshim=$UseGlshim >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application uses software video buffer - you're calling SDL_SetVideoMode() without SDL_HWSURFACE and without SDL_OPENGL," >> AndroidAppSettings.cfg
echo "# this will allow small speed optimization. Enable this even when you're using SDL_HWSURFACE. (y) or (n)" >> AndroidAppSettings.cfg
echo SwVideoMode=$SwVideoMode >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application video output will be resized to fit into native device screen (y)/(n)" >> AndroidAppSettings.cfg
echo SdlVideoResize=$SdlVideoResize >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application resizing will keep 4:3 aspect ratio, with black bars at sides (y)/(n)" >> AndroidAppSettings.cfg
echo SdlVideoResizeKeepAspect=$SdlVideoResizeKeepAspect >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Do not allow device to sleep when the application is in foreground, set this for video players or apps which use accelerometer" >> AndroidAppSettings.cfg
echo InhibitSuspend=$InhibitSuspend >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Create Android service, so the app is less likely to be killed while in background" >> AndroidAppSettings.cfg
echo CreateService=$CreateService >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application does not call SDL_Flip() or SDL_UpdateRects() appropriately, or draws from non-main thread -" >> AndroidAppSettings.cfg
echo "# enabling the compatibility mode will force screen update every 100 milliseconds, which is laggy and inefficient (y) or (n)" >> AndroidAppSettings.cfg
echo CompatibilityHacksForceScreenUpdate=$CompatibilityHacksForceScreenUpdate >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application does not call SDL_Flip() or SDL_UpdateRects() after mouse click (ScummVM and all Amiga emulators do that) -" >> AndroidAppSettings.cfg
echo "# force screen update by moving mouse cursor a little after each click (y) or (n)" >> AndroidAppSettings.cfg
echo CompatibilityHacksForceScreenUpdateMouseClick=$CompatibilityHacksForceScreenUpdateMouseClick >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application initializes SDL audio/video inside static constructors (which is bad, you won't be able to run ndk-gdb) (y)/(n)" >> AndroidAppSettings.cfg
echo CompatibilityHacksStaticInit=$CompatibilityHacksStaticInit >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# On-screen Android soft text input emulates hardware keyboard, this will only work with Hackers Keyboard app (y)/(n)" >> AndroidAppSettings.cfg
echo CompatibilityHacksTextInputEmulatesHwKeyboard=$CompatibilityHacksTextInputEmulatesHwKeyboard >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Built-in text input keyboards with custom layouts for emulators, requires CompatibilityHacksTextInputEmulatesHwKeyboard=y" >> AndroidAppSettings.cfg
echo "# 0 or empty - standard Android keyboard" >> AndroidAppSettings.cfg
echo "# 1 - Simple QWERTY keyboard, no function keys, no arrow keys" >> AndroidAppSettings.cfg
echo "# 2 - Commodore 64 keyboard" >> AndroidAppSettings.cfg
echo "# 3 - Amiga keyboard" >> AndroidAppSettings.cfg
echo "# 4 - Atari800 keyboard" >> AndroidAppSettings.cfg
echo TextInputKeyboard=$TextInputKeyboard >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Hack for broken devices: prevent audio chopping, by sleeping a bit after pushing each audio chunk (y)/(n)" >> AndroidAppSettings.cfg
echo CompatibilityHacksPreventAudioChopping=$CompatibilityHacksPreventAudioChopping >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Hack for broken apps: application ignores audio buffer size returned by SDL (y)/(n)" >> AndroidAppSettings.cfg
echo CompatibilityHacksAppIgnoresAudioBufferSize=$CompatibilityHacksAppIgnoresAudioBufferSize >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Hack for VCMI: preload additional shared libraries before aplication start" >> AndroidAppSettings.cfg
echo CompatibilityHacksAdditionalPreloadedSharedLibraries=\"$CompatibilityHacksAdditionalPreloadedSharedLibraries\" >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Hack for Free Heroes 2, which redraws the screen inside SDL_PumpEvents(): slow and compatible SDL event queue -" >> AndroidAppSettings.cfg
echo "# do not use it with accelerometer/gyroscope, or your app may freeze at random (y)/(n)" >> AndroidAppSettings.cfg
echo CompatibilityHacksSlowCompatibleEventQueue=$CompatibilityHacksSlowCompatibleEventQueue >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Save and restore OpenGL state when drawing on-screen keyboard for apps that use SDL_OPENGL" >> AndroidAppSettings.cfg
echo CompatibilityHacksTouchscreenKeyboardSaveRestoreOpenGLState=$CompatibilityHacksTouchscreenKeyboardSaveRestoreOpenGLState >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application uses SDL_UpdateRects() properly, and does not draw in any region outside those rects." >> AndroidAppSettings.cfg
echo "# This improves drawing speed, but I know only one application that does that, and it's written by me (y)/(n)" >> AndroidAppSettings.cfg
echo CompatibilityHacksProperUsageOfSDL_UpdateRects=$CompatibilityHacksProperUsageOfSDL_UpdateRects >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application uses mouse (y) or (n), this will show mouse emulation dialog to the user" >> AndroidAppSettings.cfg
echo AppUsesMouse=$AppUsesMouse >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application needs two-button mouse, will also enable advanced point-and-click features (y) or (n)" >> AndroidAppSettings.cfg
echo AppNeedsTwoButtonMouse=$AppNeedsTwoButtonMouse >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Right mouse button can do long-press/drag&drop action, necessary for some games (y) or (n)" >> AndroidAppSettings.cfg
echo "# If you disable it, swiping with two fingers will send mouse wheel events" >> AndroidAppSettings.cfg
echo RightMouseButtonLongPress=$RightMouseButtonLongPress >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Show SDL mouse cursor, for applications that do not draw cursor at all (y) or (n)" >> AndroidAppSettings.cfg
echo ShowMouseCursor=$ShowMouseCursor >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Screen follows mouse cursor, when it's covered by soft keyboard, this works only in software video mode (y) or (n)" >> AndroidAppSettings.cfg
echo ScreenFollowsMouse=$ScreenFollowsMouse >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Generate more touch events, by default SDL generates one event per one video frame, this is useful for drawing apps (y) or (n)" >> AndroidAppSettings.cfg
echo GenerateSubframeTouchEvents=$GenerateSubframeTouchEvents >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Force relative (laptop) mouse movement mode, useful when both on-screen keyboard and mouse are needed (y) or (n)" >> AndroidAppSettings.cfg
echo ForceRelativeMouseMode=$ForceRelativeMouseMode >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Show on-screen dpad/joystick, that will act as arrow keys (y) or (n)" >> AndroidAppSettings.cfg
echo AppNeedsArrowKeys=$AppNeedsArrowKeys >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# On-screen dpad/joystick will appear under finger when it touches the screen (y) or (n)" >> AndroidAppSettings.cfg
echo "# Joystick always follows finger, so moving mouse requires touching the screen with other finger" >> AndroidAppSettings.cfg
echo FloatingScreenJoystick=$FloatingScreenJoystick >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application needs text input (y) or (n), enables button for text input on screen" >> AndroidAppSettings.cfg
echo AppNeedsTextInput=$AppNeedsTextInput >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application uses joystick (y) or (n), the on-screen DPAD will be used as joystick 0 axes 0-1" >> AndroidAppSettings.cfg
echo "# This will disable AppNeedsArrowKeys option" >> AndroidAppSettings.cfg
echo AppUsesJoystick=$AppUsesJoystick >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application uses second on-screen joystick, as SDL joystick 0 axes 2-3 (y)/(n)" >> AndroidAppSettings.cfg
echo AppUsesSecondJoystick=$AppUsesSecondJoystick >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application uses third on-screen joystick, as SDL joystick 0 axes 20-21 (y)/(n)" >> AndroidAppSettings.cfg
echo AppUsesThirdJoystick=$AppUsesThirdJoystick >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application uses accelerometer (y) or (n), the accelerometer will be used as joystick 1 axes 0-1 and 5-7" >> AndroidAppSettings.cfg
echo AppUsesAccelerometer=$AppUsesAccelerometer >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application uses gyroscope (y) or (n), the gyroscope will be used as joystick 1 axes 2-4" >> AndroidAppSettings.cfg
echo AppUsesGyroscope=$AppUsesGyroscope >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application uses orientation sensor (y) or (n), reported as joystick 1 axes 8-10" >> AndroidAppSettings.cfg
echo AppUsesOrientationSensor=$AppUsesOrientationSensor >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Use gyroscope to move mouse cursor (y) or (n), it eats battery, and can be disabled in settings, do not use with AppUsesGyroscope setting" >> AndroidAppSettings.cfg
echo MoveMouseWithGyroscope=$MoveMouseWithGyroscope >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application uses multitouch (y) or (n), multitouch events are passed as SDL_JOYBALLMOTION events for the joystick 0" >> AndroidAppSettings.cfg
echo AppUsesMultitouch=$AppUsesMultitouch >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application records audio (it will use any available source, such a s microphone)" >> AndroidAppSettings.cfg
echo "# API is defined in file SDL_android.h: int SDL_ANDROID_OpenAudioRecording(SDL_AudioSpec *spec); void SDL_ANDROID_CloseAudioRecording(void);" >> AndroidAppSettings.cfg
echo "# This option will add additional permission to Android manifest (y)/(n)" >> AndroidAppSettings.cfg
echo AppRecordsAudio=$AppRecordsAudio >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application needs to access SD card. If your data files are bigger than 5 Mb, enable it. (y) / (n)" >> AndroidAppSettings.cfg
echo AccessSdCard=$AccessSdCard >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application needs Internet access. If you disable it, you'll have to bundle all your data files inside .apk (y) / (n)" >> AndroidAppSettings.cfg
echo AccessInternet=$AccessInternet >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Immersive mode - Android will hide on-screen Home/Back keys. Looks bad if you invoke Android keyboard. (y) / (n)" >> AndroidAppSettings.cfg
echo ImmersiveMode=$ImmersiveMode >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application implements Android-specific routines to put to background, and will not draw anything to screen" >> AndroidAppSettings.cfg
echo "# between SDL_ACTIVEEVENT lost / gained notifications - you should check for them" >> AndroidAppSettings.cfg
echo "# rigth after SDL_Flip(), if (n) then SDL_Flip() will block till app in background (y) or (n)" >> AndroidAppSettings.cfg
echo "# This option is reported to be buggy, sometimes failing to restore video state" >> AndroidAppSettings.cfg
echo NonBlockingSwapBuffers=$NonBlockingSwapBuffers >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Redefine common hardware keys to SDL keysyms" >> AndroidAppSettings.cfg
echo "# BACK hardware key is available on all devices, MENU is available on pre-ICS devices, other keys may be absent" >> AndroidAppSettings.cfg
echo "# SEARCH and CALL by default return same keycode as DPAD_CENTER - one of those keys is available on most devices" >> AndroidAppSettings.cfg
echo "# Use word NO_REMAP if you want to preserve native functionality for certain key (volume keys are 3-rd and 4-th)" >> AndroidAppSettings.cfg
echo "# Keys: TOUCHSCREEN (works only when AppUsesMouse=n), DPAD_CENTER/SEARCH, VOLUMEUP, VOLUMEDOWN, MENU, BACK, CAMERA" >> AndroidAppSettings.cfg
echo RedefinedKeys=\"$RedefinedKeys\" >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Number of virtual keyboard keys (currently 6 is maximum)" >> AndroidAppSettings.cfg
echo AppTouchscreenKeyboardKeysAmount=$AppTouchscreenKeyboardKeysAmount >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Redefine on-screen keyboard keys to SDL keysyms - 6 keyboard keys + 4 multitouch gestures (zoom in/out and rotate left/right)" >> AndroidAppSettings.cfg
echo RedefinedKeysScreenKb=\"$RedefinedKeysScreenKb\" >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Names for on-screen keyboard keys, such as Fire, Jump, Run etc, separated by spaces, they are used in SDL config menu" >> AndroidAppSettings.cfg
echo RedefinedKeysScreenKbNames=\"$RedefinedKeysScreenKbNames\" >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# On-screen keys theme" >> AndroidAppSettings.cfg
echo "# 0 = Ultimate Droid by Sean Stieber (green, with cross joystick)" >> AndroidAppSettings.cfg
echo "# 1 = Simple Theme by Beholder (white, with cross joystick)" >> AndroidAppSettings.cfg
echo "# 2 = Sun by Sirea (yellow, with round joystick)" >> AndroidAppSettings.cfg
echo "# 3 = Keen by Gerstrong (multicolor, with round joystick)" >> AndroidAppSettings.cfg
echo "# 4 = Retro by Santiago Radeff (red/white, with cross joystick)" >> AndroidAppSettings.cfg
echo "# 5 = GameBoy from RetroArch" >> AndroidAppSettings.cfg
echo "# 6 = PlayStation from RetroArch" >> AndroidAppSettings.cfg
echo "# 7 = SuperNintendo from RetroArch" >> AndroidAppSettings.cfg
echo "# 8 = DualShock from RetroArch" >> AndroidAppSettings.cfg
echo "# 9 = Nintendo64 from RetroArch" >> AndroidAppSettings.cfg
echo TouchscreenKeysTheme=$TouchscreenKeysTheme >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Redefine gamepad keys to SDL keysyms, button order is:" >> AndroidAppSettings.cfg
echo "# A B X Y L1 R1 L2 R2 LThumb RThumb" >> AndroidAppSettings.cfg
echo RedefinedKeysGamepad=\"$RedefinedKeysGamepad\" >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# How long to show startup menu button, in msec, 0 to disable startup menu" >> AndroidAppSettings.cfg
echo StartupMenuButtonTimeout=$StartupMenuButtonTimeout >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Menu items to hide from startup menu, available menu items:" >> AndroidAppSettings.cfg
echo "# $MenuOptionsAvailable" >> AndroidAppSettings.cfg
echo HiddenMenuOptions=\'$HiddenMenuOptions\' >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Menu items to show at startup - this is Java code snippet, leave empty for default" >> AndroidAppSettings.cfg
echo "# $FirstStartMenuOptionsDefault" >> AndroidAppSettings.cfg
echo "# Available menu items:" >> AndroidAppSettings.cfg
echo "# $MenuOptionsAvailable" >> AndroidAppSettings.cfg
echo FirstStartMenuOptions=\'$FirstStartMenuOptions\' >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Minimum amount of RAM application requires, in Mb, SDL will print warning to user if it's lower" >> AndroidAppSettings.cfg
echo AppMinimumRAM=$AppMinimumRAM >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# GCC version, 4.6 (default) or 4.8, CLANG is not supported yet" >> AndroidAppSettings.cfg
echo NDK_TOOLCHAIN_VERSION=$NDK_TOOLCHAIN_VERSION >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Specify architectures to compile, 'all' or 'y' to compile for all architectures." >> AndroidAppSettings.cfg
echo "# Available architectures: armeabi armeabi-v7a armeabi-v7a-hard x86 mips" >> AndroidAppSettings.cfg
echo MultiABI=\'$MultiABI\' >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Optional shared libraries to compile - removing some of them will save space" >> AndroidAppSettings.cfg
echo "# MP3 support by libMAD is encumbered by patents and libMAD is GPL-ed" >> AndroidAppSettings.cfg
grep 'Available' project/jni/SettingsTemplate.mk >> AndroidAppSettings.cfg
echo CompiledLibraries=\"$CompiledLibraries\" >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application uses custom build script AndroidBuild.sh instead of Android.mk (y) or (n)" >> AndroidAppSettings.cfg
echo CustomBuildScript=$CustomBuildScript >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Aditional CFLAGS for application" >> AndroidAppSettings.cfg
echo AppCflags=\'$AppCflags\' >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Additional LDFLAGS for application" >> AndroidAppSettings.cfg
echo AppLdflags=\'$AppLdflags\' >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# If application has headers with the same name as system headers, this option tries to fix compiler flags to make it compilable" >> AndroidAppSettings.cfg
echo AppOverlapsSystemHeaders=$AppOverlapsSystemHeaders >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Build only following subdirs (empty will build all dirs, ignored with custom script)" >> AndroidAppSettings.cfg
echo AppSubdirsBuild=\'$AppSubdirsBuild\' >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Exclude these files from build" >> AndroidAppSettings.cfg
echo AppBuildExclude=\'$AppBuildExclude\' >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Application command line parameters, including app name as 0-th param" >> AndroidAppSettings.cfg
echo AppCmdline=\'$AppCmdline\' >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Screen size is used by Google Play to prevent an app to be installed on devices with smaller screens" >> AndroidAppSettings.cfg
echo "# Minimum screen size that application supports: (s)mall / (m)edium / (l)arge" >> AndroidAppSettings.cfg
echo MinimumScreenSize=$MinimumScreenSize >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Your AdMob Publisher ID, (n) if you don't want advertisements" >> AndroidAppSettings.cfg
echo AdmobPublisherId=$AdmobPublisherId >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Your AdMob test device ID, to receive a test ad" >> AndroidAppSettings.cfg
echo AdmobTestDeviceId=$AdmobTestDeviceId >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Your AdMob banner size (BANNER/FULL_BANNER/LEADERBOARD/MEDIUM_RECTANGLE/SMART_BANNER/WIDE_SKYSCRAPER/FULL_WIDTH:Height/Width:AUTO_HEIGHT/Width:Height)" >> AndroidAppSettings.cfg
echo AdmobBannerSize=$AdmobBannerSize >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
echo "# Google Play Game Services application ID, required for cloud saves to work" >> AndroidAppSettings.cfg
echo GooglePlayGameServicesId=$GooglePlayGameServicesId >> AndroidAppSettings.cfg
echo >> AndroidAppSettings.cfg
fi

AppShortName=`echo $AppName | sed 's/ //g'`
DataPath="$AppFullName"
AppFullNameUnderscored=`echo $AppFullName | sed 's/[.]/_/g'`
AppSharedLibrariesPath=/data/data/$AppFullName/lib
ScreenOrientation1=sensorPortrait
HorizontalOrientation=false

UsingSdl13=false
if [ "$LibSdlVersion" = "1.3" ] ; then
	UsingSdl13=true
fi

UsingSdl20=false
if [ "$LibSdlVersion" = "2.0" ] ; then
	UsingSdl20=true
fi

if [ "$ScreenOrientation" = "h" -o "$ScreenOrientation" = "l" ] ; then
	ScreenOrientation1=sensorLandscape
	HorizontalOrientation=true
fi

AppDataDownloadUrl1="`echo $AppDataDownloadUrl | sed 's/[&]/%26/g'`"

if [ "$SdlVideoResize" = "y" ] ; then
	SdlVideoResize=1
else
	SdlVideoResize=0
fi

if [ "$SdlVideoResizeKeepAspect" = "y" ] ; then
	SdlVideoResizeKeepAspect=true
else
	SdlVideoResizeKeepAspect=false
fi

if [ "$InhibitSuspend" = "y" ] ; then
	InhibitSuspend=true
else
	InhibitSuspend=false
fi

if [ "$CreateService" = "y" ] ; then
	CreateService=true
else
	CreateService=false
fi

if [ "$NeedDepthBuffer" = "y" ] ; then
	NeedDepthBuffer=true
else
	NeedDepthBuffer=false
fi

if [ "$NeedStencilBuffer" = "y" ] ; then
	NeedStencilBuffer=true
else
	NeedStencilBuffer=false
fi

if [ "$NeedGles2" = "y" ] ; then
	NeedGles2=true
else
	NeedGles2=false
fi

if [ "$UseGlshim" = "y" ] ; then
	UseGlshimCFlags=-DUSE_GLSHIM=1
else
	UseGlshim=
	UseGlshimCFlags=
fi

if [ "$SwVideoMode" = "y" ] ; then
	SwVideoMode=true
else
	SwVideoMode=false
fi

if [ "$CompatibilityHacksForceScreenUpdate" = "y" ] ; then
	CompatibilityHacksForceScreenUpdate=true
else
	CompatibilityHacksForceScreenUpdate=false
fi

if [ "$CompatibilityHacksForceScreenUpdateMouseClick" = "y" ] ; then
	CompatibilityHacksForceScreenUpdateMouseClick=true
else
	CompatibilityHacksForceScreenUpdateMouseClick=false
fi

if [ "$CompatibilityHacksStaticInit" = "y" ] ; then
	CompatibilityHacksStaticInit=true
else
	CompatibilityHacksStaticInit=false
fi

if [ "$CompatibilityHacksTextInputEmulatesHwKeyboard" = "y" ] ; then
	CompatibilityHacksTextInputEmulatesHwKeyboard=true
else
	CompatibilityHacksTextInputEmulatesHwKeyboard=false
fi

if [ -z "$TextInputKeyboard" ] ; then
	TextInputKeyboard=0
fi

if [ "$CompatibilityHacksPreventAudioChopping" = "y" ] ; then
	CompatibilityHacksPreventAudioChopping=-DSDL_AUDIO_PREVENT_CHOPPING_WITH_DELAY=1
else
	CompatibilityHacksPreventAudioChopping=
fi

if [ "$CompatibilityHacksAppIgnoresAudioBufferSize" = "y" ] ; then
	CompatibilityHacksAppIgnoresAudioBufferSize=-DSDL_AUDIO_APP_IGNORES_RETURNED_BUFFER_SIZE=1
else
	CompatibilityHacksAppIgnoresAudioBufferSize=
fi

if [ "$CompatibilityHacksSlowCompatibleEventQueue" = "y" ]; then
	CompatibilityHacksSlowCompatibleEventQueue=-DSDL_COMPATIBILITY_HACKS_SLOW_COMPATIBLE_EVENT_QUEUE=1
else
	CompatibilityHacksSlowCompatibleEventQueue=
fi

if [ "$CompatibilityHacksTouchscreenKeyboardSaveRestoreOpenGLState" = "y" ]; then
	CompatibilityHacksTouchscreenKeyboardSaveRestoreOpenGLState=-DSDL_TOUCHSCREEN_KEYBOARD_SAVE_RESTORE_OPENGL_STATE=1
else
	CompatibilityHacksTouchscreenKeyboardSaveRestoreOpenGLState=
fi

if [ "$CompatibilityHacksProperUsageOfSDL_UpdateRects" = "y" ]; then
	CompatibilityHacksProperUsageOfSDL_UpdateRects=-DSDL_COMPATIBILITY_HACKS_PROPER_USADE_OF_SDL_UPDATERECTS=1
else
	CompatibilityHacksProperUsageOfSDL_UpdateRects=
fi

if [ "$AppUsesMouse" = "y" ] ; then
	AppUsesMouse=true
else
	AppUsesMouse=false
fi

if [ "$AppNeedsTwoButtonMouse" = "y" ] ; then
	AppNeedsTwoButtonMouse=true
else
	AppNeedsTwoButtonMouse=false
fi

if [ "$RightMouseButtonLongPress" = "n" ] ; then
	RightMouseButtonLongPress=false
else
	RightMouseButtonLongPress=true
fi

if [ "$ForceRelativeMouseMode" = "y" ] ; then
	ForceRelativeMouseMode=true
else
	ForceRelativeMouseMode=false
fi

if [ "$ShowMouseCursor" = "y" ] ; then
	ShowMouseCursor=true
else
	ShowMouseCursor=false
fi

if [ "$ScreenFollowsMouse" = "y" ] ; then
	ScreenFollowsMouse=true
else
	ScreenFollowsMouse=false
fi

if [ "$GenerateSubframeTouchEvents" = "y" ] ; then
	GenerateSubframeTouchEvents=true
else
	GenerateSubframeTouchEvents=false
fi

if [ "$AppNeedsArrowKeys" = "y" ] ; then
	AppNeedsArrowKeys=true
else
	AppNeedsArrowKeys=false
fi

if [ "$FloatingScreenJoystick" = "y" ] ; then
	FloatingScreenJoystick=true
else
	FloatingScreenJoystick=false
fi

if [ "$AppNeedsTextInput" = "y" ] ; then
	AppNeedsTextInput=true
else
	AppNeedsTextInput=false
fi

if [ "$AppUsesJoystick" = "y" ] ; then
	AppUsesJoystick=true
else
	AppUsesJoystick=false
fi

if [ "$AppUsesSecondJoystick" = "y" ] ; then
	AppUsesSecondJoystick=true
else
	AppUsesSecondJoystick=false
fi

if [ "$AppUsesThirdJoystick" = "y" ] ; then
	AppUsesThirdJoystick=true
else
	AppUsesThirdJoystick=false
fi

if [ "$AppUsesAccelerometer" = "y" ] ; then
	AppUsesAccelerometer=true
else
	AppUsesAccelerometer=false
fi

if [ "$AppUsesGyroscope" = "y" ] ; then
	AppUsesGyroscope=true
else
	AppUsesGyroscope=false
fi

if [ "$AppUsesOrientationSensor" = "y" ] ; then
	AppUsesOrientationSensor=true
else
	AppUsesOrientationSensor=false
fi

if [ "$MoveMouseWithGyroscope" = "y" ] ; then
	MoveMouseWithGyroscope=true
else
	MoveMouseWithGyroscope=false
fi

if [ "$AppUsesMultitouch" = "y" ] ; then
	AppUsesMultitouch=true
else
	AppUsesMultitouch=false
fi

if [ "$NonBlockingSwapBuffers" = "y" ] ; then
	NonBlockingSwapBuffers=true
else
	NonBlockingSwapBuffers=false
fi

if [ "$ResetSdlConfigForThisVersion" = "y" ] ; then
	ResetSdlConfigForThisVersion=true
else
	ResetSdlConfigForThisVersion=false
fi

KEY2=0
for KEY in $RedefinedKeys; do
	RedefinedKeycodes="$RedefinedKeycodes -DSDL_ANDROID_KEYCODE_$KEY2=$KEY"
	KEY2=`expr $KEY2 '+' 1`
done

KEY2=0
for KEY in $RedefinedKeysScreenKb; do
	RedefinedKeycodesScreenKb="$RedefinedKeycodesScreenKb -DSDL_ANDROID_SCREENKB_KEYCODE_$KEY2=$KEY"
	KEY2=`expr $KEY2 '+' 1`
done

KEY2=0
for KEY in $RedefinedKeysGamepad; do
	RedefinedKeycodesGamepad="$RedefinedKeycodesGamepad -DSDL_ANDROID_GAMEPAD_KEYCODE_$KEY2=$KEY"
	KEY2=`expr $KEY2 '+' 1`
done

if [ "$MultiABI" = "y" ] ; then
	MultiABI="all"
elif [ "$MultiABI" = "n" ] ; then
	MultiABI="armeabi"
else
	MultiABI="$MultiABI"
fi

LibrariesToLoad="\\\"sdl_native_helpers\\\", \\\"sdl-$LibSdlVersion\\\""
StaticLibraries=`grep 'APP_AVAILABLE_STATIC_LIBS' project/jni/SettingsTemplate.mk | sed 's/.*=\(.*\)/\1/'`
for lib in $CompiledLibraries; do
	process=true
	for lib1 in $StaticLibraries; do
		if [ "$lib" = "$lib1" ]; then process=false; fi
	done
	if $process; then
		LibrariesToLoad="$LibrariesToLoad, \\\"$lib\\\""
	fi
done

MainLibrariesToLoad=""
for lib in $CompatibilityHacksAdditionalPreloadedSharedLibraries; do
	MainLibrariesToLoad="$MainLibrariesToLoad \\\"$lib\\\","
done

MainLibrariesToLoad="$MainLibrariesToLoad \\\"application\\\", \\\"sdl_main\\\""


if [ "$CustomBuildScript" = "n" ] ; then
	CustomBuildScript=
fi

HiddenMenuOptions1=""
for F in $HiddenMenuOptions; do
	HiddenMenuOptions1="$HiddenMenuOptions1 new $F(),"
done

FirstStartMenuOptions1=""
for F in $FirstStartMenuOptions; do
	FirstStartMenuOptions1="$FirstStartMenuOptions1 new $F(),"
done

#if [ -z "$FirstStartMenuOptions" ]; then
#	FirstStartMenuOptions="$FirstStartMenuOptionsDefault"
#fi

ReadmeText="`echo $ReadmeText | sed 's/\"/\\\\\\\\\"/g' | sed 's/[&%]//g'`"


SEDI="sed -i"
if uname -s | grep -i "darwin" > /dev/null ; then
	SEDI="sed -i.killme.tmp" # MacOsX version of sed is buggy, and requires a mandatory parameter
fi


rm -rf project/src
mkdir -p project/src
cd $JAVA_SRC_PATH
for F in *.java; do
	echo '// DO NOT EDIT THIS FILE - it is automatically generated, ALL YOUR CHANGES WILL BE OVERWRITTEN, edit the file under $JAVA_SRC_PATH dir' | cat - $F > ../src/$F
done

if [ -e ../jni/application/src/java.diff ]; then patch -d ../src --no-backup-if-mismatch < ../jni/application/src/java.diff || exit 1 ; fi
if [ -e ../jni/application/src/java.patch ]; then patch -d ../src --no-backup-if-mismatch < ../jni/application/src/java.patch || exit 1 ; fi
if ls ../jni/application/src/*.java > /dev/null 2>&1; then cp -f ../jni/application/src/*.java ../src ; fi

for F in ../src/*.java; do
	echo Patching $F
	$SEDI "s/^package .*;/package $AppFullName;/" $F
done

cd ../..

# In case we use SDL2 let simlink the SDLActivity source file
if [ "$LibSdlVersion" = "2.0" ] ; then
	ln -s ../jni/sdl-2.0/android-project/src/org/libsdl/app/SDLActivity.java project/src/SDLActivity.java
fi


echo Patching project/AndroidManifest.xml
cat project/AndroidManifestTemplate.xml | \
	sed "s/package=.*/package=\"$AppFullName\"/" | \
	sed "s/android:screenOrientation=.*/android:screenOrientation=\"$ScreenOrientation1\"/" | \
	sed "s^android:versionCode=.*^android:versionCode=\"$AppVersionCode\"^" | \
	sed "s^android:versionName=.*^android:versionName=\"$AppVersionName\"^" > \
	project/AndroidManifest.xml
if [ "$AdmobPublisherId" = "n" -o -z "$AdmobPublisherId" ] ; then
	$SEDI "/==ADMOB==/ d" project/AndroidManifest.xml
	AdmobPublisherId=""
else
	F=$JAVA_SRC_PATH/admob/Advertisement.java
	echo Patching $F
	echo '// DO NOT EDIT THIS FILE - it is automatically generated, edit file under $JAVA_SRC_PATH dir' > project/src/Advertisement.java
	cat $F | sed "s/^package .*;/package $AppFullName;/" >> project/src/Advertisement.java
fi

echo "-keep class $AppFullName.** { *; }" > project/proguard-local.cfg

if [ "$AppRecordsAudio" = "n" -o -z "$AppRecordsAudio" ] ; then
	$SEDI "/==RECORD_AUDIO==/ d" project/AndroidManifest.xml
fi

case "$MinimumScreenSize" in
	n|m)
		$SEDI "/==SCREEN-SIZE-SMALL==/ d" project/AndroidManifest.xml
		$SEDI "/==SCREEN-SIZE-LARGE==/ d" project/AndroidManifest.xml
		;;
	l)
		$SEDI "/==SCREEN-SIZE-SMALL==/ d" project/AndroidManifest.xml
		$SEDI "/==SCREEN-SIZE-NORMAL==/ d" project/AndroidManifest.xml
		;;
	*)
		$SEDI "/==SCREEN-SIZE-NORMAL==/ d" project/AndroidManifest.xml
		$SEDI "/==SCREEN-SIZE-LARGE==/ d" project/AndroidManifest.xml
		;;
esac

if [ "$AccessSdCard" = "n" ]; then
	$SEDI "/==EXTERNAL_STORAGE==/ d" project/AndroidManifest.xml
else
	$SEDI "/==NOT_EXTERNAL_STORAGE==/ d" project/AndroidManifest.xml
fi

if [ "$AccessInternet" = "n" ]; then
	$SEDI "/==INTERNET==/ d" project/AndroidManifest.xml
fi

if [ "$ImmersiveMode" = "n" ]; then
	ImmersiveMode=false
else
	ImmersiveMode=true
fi

echo Patching project/src/Globals.java
$SEDI "s/public static String ApplicationName = .*;/public static String ApplicationName = \"$AppShortName\";/" project/src/Globals.java
$SEDI "s/public static final boolean Using_SDL_1_3 = .*;/public static final boolean Using_SDL_1_3 = $UsingSdl13;/" project/src/Globals.java

$SEDI "s/public static final boolean Using_SDL_2_0 = .*;/public static final boolean Using_SDL_2_0 = $UsingSdl20;/" project/src/Globals.java

# Work around "Argument list too long" problem when compiling VICE
#$SEDI "s@public static String DataDownloadUrl = .*@public static String DataDownloadUrl = \"$AppDataDownloadUrl1\";@" project/src/Globals.java
$SEDI "s@public static String\[\] DataDownloadUrl = .*@public static String[] DataDownloadUrl = { ### };@" project/src/Globals.java
echo "$AppDataDownloadUrl1" | tr '^' '\n' | while read URL; do $SEDI "s@###@\"$URL\", ###@" project/src/Globals.java ; done
$SEDI "s@###@@" project/src/Globals.java

$SEDI "s/public static boolean SwVideoMode = .*;/public static boolean SwVideoMode = $SwVideoMode;/" project/src/Globals.java
$SEDI "s/public static int VideoDepthBpp = .*;/public static int VideoDepthBpp = $VideoDepthBpp;/" project/src/Globals.java
$SEDI "s/public static boolean NeedDepthBuffer = .*;/public static boolean NeedDepthBuffer = $NeedDepthBuffer;/" project/src/Globals.java
$SEDI "s/public static boolean NeedStencilBuffer = .*;/public static boolean NeedStencilBuffer = $NeedStencilBuffer;/" project/src/Globals.java
$SEDI "s/public static boolean NeedGles2 = .*;/public static boolean NeedGles2 = $NeedGles2;/" project/src/Globals.java
$SEDI "s/public static boolean CompatibilityHacksVideo = .*;/public static boolean CompatibilityHacksVideo = $CompatibilityHacksForceScreenUpdate;/" project/src/Globals.java
$SEDI "s/public static boolean CompatibilityHacksStaticInit = .*;/public static boolean CompatibilityHacksStaticInit = $CompatibilityHacksStaticInit;/" project/src/Globals.java
$SEDI "s/public static boolean CompatibilityHacksTextInputEmulatesHwKeyboard = .*;/public static boolean CompatibilityHacksTextInputEmulatesHwKeyboard = $CompatibilityHacksTextInputEmulatesHwKeyboard;/" project/src/Globals.java
$SEDI "s/public static int TextInputKeyboard = .*;/public static int TextInputKeyboard = $TextInputKeyboard;/" project/src/Globals.java
$SEDI "s/public static boolean CompatibilityHacksForceScreenUpdateMouseClick = .*;/public static boolean CompatibilityHacksForceScreenUpdateMouseClick = $CompatibilityHacksForceScreenUpdateMouseClick;/" project/src/Globals.java
$SEDI "s/public static boolean HorizontalOrientation = .*;/public static boolean HorizontalOrientation = $HorizontalOrientation;/" project/src/Globals.java
$SEDI "s^public static boolean KeepAspectRatioDefaultSetting = .*^public static boolean KeepAspectRatioDefaultSetting = $SdlVideoResizeKeepAspect;^" project/src/Globals.java
$SEDI "s/public static boolean InhibitSuspend = .*;/public static boolean InhibitSuspend = $InhibitSuspend;/" project/src/Globals.java
$SEDI "s/public static boolean CreateService = .*;/public static boolean CreateService = $CreateService;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesMouse = .*;/public static boolean AppUsesMouse = $AppUsesMouse;/" project/src/Globals.java
$SEDI "s/public static boolean AppNeedsTwoButtonMouse = .*;/public static boolean AppNeedsTwoButtonMouse = $AppNeedsTwoButtonMouse;/" project/src/Globals.java
$SEDI "s/public static boolean RightMouseButtonLongPress = .*;/public static boolean RightMouseButtonLongPress = $RightMouseButtonLongPress;/" project/src/Globals.java
$SEDI "s/public static boolean ForceRelativeMouseMode = .*;/public static boolean ForceRelativeMouseMode = $ForceRelativeMouseMode;/" project/src/Globals.java
$SEDI "s/public static boolean ShowMouseCursor = .*;/public static boolean ShowMouseCursor = $ShowMouseCursor;/" project/src/Globals.java
$SEDI "s/public static boolean ScreenFollowsMouse = .*;/public static boolean ScreenFollowsMouse = $ScreenFollowsMouse;/" project/src/Globals.java
$SEDI "s/public static boolean GenerateSubframeTouchEvents = .*;/public static boolean GenerateSubframeTouchEvents = $GenerateSubframeTouchEvents;/" project/src/Globals.java
$SEDI "s/public static boolean AppNeedsArrowKeys = .*;/public static boolean AppNeedsArrowKeys = $AppNeedsArrowKeys;/" project/src/Globals.java
$SEDI "s/public static boolean FloatingScreenJoystick = .*;/public static boolean FloatingScreenJoystick = $FloatingScreenJoystick;/" project/src/Globals.java
$SEDI "s/public static boolean AppNeedsTextInput = .*;/public static boolean AppNeedsTextInput = $AppNeedsTextInput;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesJoystick = .*;/public static boolean AppUsesJoystick = $AppUsesJoystick;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesSecondJoystick = .*;/public static boolean AppUsesSecondJoystick = $AppUsesSecondJoystick;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesThirdJoystick = .*;/public static boolean AppUsesThirdJoystick = $AppUsesThirdJoystick;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesAccelerometer = .*;/public static boolean AppUsesAccelerometer = $AppUsesAccelerometer;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesGyroscope = .*;/public static boolean AppUsesGyroscope = $AppUsesGyroscope;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesOrientationSensor = .*;/public static boolean AppUsesOrientationSensor = $AppUsesOrientationSensor;/" project/src/Globals.java
$SEDI "s/public static boolean MoveMouseWithGyroscope = .*;/public static boolean MoveMouseWithGyroscope = $MoveMouseWithGyroscope;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesMultitouch = .*;/public static boolean AppUsesMultitouch = $AppUsesMultitouch;/" project/src/Globals.java
$SEDI "s/public static boolean NonBlockingSwapBuffers = .*;/public static boolean NonBlockingSwapBuffers = $NonBlockingSwapBuffers;/" project/src/Globals.java
$SEDI "s/public static boolean ResetSdlConfigForThisVersion = .*;/public static boolean ResetSdlConfigForThisVersion = $ResetSdlConfigForThisVersion;/" project/src/Globals.java
$SEDI "s/public static boolean ImmersiveMode = .*;/public static boolean ImmersiveMode = $ImmersiveMode;/" project/src/Globals.java
$SEDI "s|public static String DeleteFilesOnUpgrade = .*;|public static String DeleteFilesOnUpgrade = \"$DeleteFilesOnUpgrade\";|" project/src/Globals.java
$SEDI "s/public static int AppTouchscreenKeyboardKeysAmount = .*;/public static int AppTouchscreenKeyboardKeysAmount = $AppTouchscreenKeyboardKeysAmount;/" project/src/Globals.java
$SEDI "s@public static String\\[\\] AppTouchscreenKeyboardKeysNames = .*;@public static String[] AppTouchscreenKeyboardKeysNames = \"$RedefinedKeysScreenKbNames\".split(\" \");@" project/src/Globals.java
$SEDI "s/public static int TouchscreenKeyboardTheme = .*;/public static int TouchscreenKeyboardTheme = $TouchscreenKeysTheme;/" project/src/Globals.java
$SEDI "s/public static int StartupMenuButtonTimeout = .*;/public static int StartupMenuButtonTimeout = $StartupMenuButtonTimeout;/" project/src/Globals.java
$SEDI "s/public static int AppMinimumRAM = .*;/public static int AppMinimumRAM = $AppMinimumRAM;/" project/src/Globals.java
$SEDI "s/public static SettingsMenu.Menu HiddenMenuOptions .*;/public static SettingsMenu.Menu HiddenMenuOptions [] = { $HiddenMenuOptions1 };/" project/src/Globals.java
[ -n "$FirstStartMenuOptions1" ] && $SEDI "s@public static SettingsMenu.Menu FirstStartMenuOptions .*;@public static SettingsMenu.Menu FirstStartMenuOptions [] = { $FirstStartMenuOptions1 };@" project/src/Globals.java
$SEDI "s%public static String ReadmeText = .*%public static String ReadmeText = \"$ReadmeText\";%" project/src/Globals.java
$SEDI "s%public static String CommandLine = .*%public static String CommandLine = \"$AppCmdline\";%" project/src/Globals.java
$SEDI "s%public static String AdmobPublisherId = .*%public static String AdmobPublisherId = \"$AdmobPublisherId\";%" project/src/Globals.java
$SEDI "s/public static String AdmobTestDeviceId = .*/public static String AdmobTestDeviceId = \"$AdmobTestDeviceId\";/" project/src/Globals.java
$SEDI "s/public static String AdmobBannerSize = .*/public static String AdmobBannerSize = \"$AdmobBannerSize\";/" project/src/Globals.java
$SEDI "s%public static String GooglePlayGameServicesId = .*%public static String GooglePlayGameServicesId = \"$GooglePlayGameServicesId\";%" project/src/Globals.java
$SEDI "s/public static String AppLibraries.*/public static String AppLibraries[] = { $LibrariesToLoad };/" project/src/Globals.java
$SEDI "s/public static String AppMainLibraries.*/public static String AppMainLibraries[] = { $MainLibrariesToLoad };/" project/src/Globals.java


echo Patching project/jni/Settings.mk
echo '# DO NOT EDIT THIS FILE - it is automatically generated, edit file SettingsTemplate.mk' > project/jni/Settings.mk
cat project/jni/SettingsTemplate.mk | \
	sed "s/APP_MODULES := .*/APP_MODULES := application sdl-$LibSdlVersion sdl_main sdl_native_helpers jpeg png ogg flac vorbis freetype $CompiledLibraries/" | \
	sed "s/APP_ABI := .*/APP_ABI := $MultiABI/" | \
	sed "s/SDL_JAVA_PACKAGE_PATH := .*/SDL_JAVA_PACKAGE_PATH := $AppFullNameUnderscored/" | \
	sed "s^SDL_CURDIR_PATH := .*^SDL_CURDIR_PATH := $DataPath^" | \
	sed "s^SDL_VIDEO_RENDER_RESIZE := .*^SDL_VIDEO_RENDER_RESIZE := $SdlVideoResize^" | \
	sed "s^COMPILED_LIBRARIES := .*^COMPILED_LIBRARIES := $CompiledLibraries^" | \
	sed "s^APPLICATION_ADDITIONAL_CFLAGS :=.*^APPLICATION_ADDITIONAL_CFLAGS := $AppCflags^" | \
	sed "s^APPLICATION_ADDITIONAL_LDFLAGS :=.*^APPLICATION_ADDITIONAL_LDFLAGS := $AppLdflags^" | \
	sed "s^APPLICATION_OVERLAPS_SYSTEM_HEADERS :=.*^APPLICATION_OVERLAPS_SYSTEM_HEADERS := $AppOverlapsSystemHeaders^" | \
	sed "s^USE_GLSHIM :=.*^USE_GLSHIM := $UseGlshim^" | \
	sed "s^SDL_ADDITIONAL_CFLAGS :=.*^SDL_ADDITIONAL_CFLAGS := \
		$RedefinedKeycodes \
		$RedefinedKeycodesScreenKb \
		$RedefinedKeycodesGamepad \
		$CompatibilityHacksPreventAudioChopping \
		$CompatibilityHacksAppIgnoresAudioBufferSize \
		$CompatibilityHacksSlowCompatibleEventQueue \
		$CompatibilityHacksTouchscreenKeyboardSaveRestoreOpenGLState \
		$CompatibilityHacksProperUsageOfSDL_UpdateRects \
		$UseGlshimCFlags^" | \
	sed "s^APPLICATION_SUBDIRS_BUILD :=.*^APPLICATION_SUBDIRS_BUILD := $AppSubdirsBuild^" | \
	sed "s^APPLICATION_BUILD_EXCLUDE :=.*^APPLICATION_BUILD_EXCLUDE := $AppBuildExclude^" | \
	sed "s^APPLICATION_CUSTOM_BUILD_SCRIPT :=.*^APPLICATION_CUSTOM_BUILD_SCRIPT := $CustomBuildScript^" | \
	sed "s^SDL_VERSION :=.*^SDL_VERSION := $LibSdlVersion^" | \
	sed "s^NDK_TOOLCHAIN_VERSION :=.*^NDK_TOOLCHAIN_VERSION := $NDK_TOOLCHAIN_VERSION^" >> \
	project/jni/Settings.mk

echo Patching strings.xml
rm -rf project/res/values*/strings.xml
cd $JAVA_SRC_PATH/translations
for F in */strings.xml; do
	mkdir -p ../../res/`dirname $F`
	cat $F | \
	sed "s^[<]string name=\"app_name\"[>].*^<string name=\"app_name\">$AppName</string>^" > \
	../../res/$F
done
cd ../../..

SDK_DIR=`grep '^sdk.dir' project/local.properties | sed 's/.*=//'`
mkdir -p project/libs

if [ "$GooglePlayGameServicesId" = "n" -o -z "$GooglePlayGameServicesId" ] ; then
	$SEDI "/==GOOGLEPLAYGAMESERVICES==/ d" project/AndroidManifest.xml
	GooglePlayGameServicesId=""
	grep 'google-play-services' project/local.properties > /dev/null && {
		$SEDI 's/.*google-play-services.*//g' project/local.properties
		rm -f project/libs/android-support-v4.jar
	}
else
	for F in $JAVA_SRC_PATH/googleplaygameservices/*.java; do
		OUT=`echo $F | sed 's@.*/@@'` # basename tool is not available everywhere
		echo Patching $F
		echo '// DO NOT EDIT THIS FILE - it is automatically generated, edit file under $JAVA_SRC_PATH dir' > project/src/$OUT
		cat $F | sed "s/^package .*;/package $AppFullName;/" >> project/src/$OUT
	done
	$SEDI "s/==GOOGLEPLAYGAMESERVICES_APP_ID==/$GooglePlayGameServicesId/g" project/res/values/strings.xml
	grep 'google-play-services' project/local.properties > /dev/null || {
		# Ant is way too smart, and adds current project path in front of the ${sdk.dir}
		echo 'android.library.reference.1=../../../../../../../../../../../../../../${sdk.dir}/extras/google/google_play_services/libproject/google-play-services_lib' >> project/local.properties
		echo 'android.library.reference.2=../../../../../../../../../../../../../../${sdk.dir}/extras/android/compatibility/v7/mediarouter' >> project/local.properties
		echo 'android.library.reference.3=../../../../../../../../../../../../../../${sdk.dir}/extras/android/compatibility/v7/appcompat' >> project/local.properties
		echo 'android.library.reference.4=../../../../../../../../../../../../../../${sdk.dir}/extras/android/compatibility/v7/palette' >> project/local.properties
		echo 'proguard.config=proguard.cfg;proguard-local.cfg' >> project/local.properties
		ln -s -f $SDK_DIR/extras/android/compatibility/v4/android-support-v4.jar project/libs
	}
	[ -e $SDK_DIR/extras/google/google_play_services/libproject/google-play-services_lib/build.xml ] || \
		android update project -t android-23 -p $SDK_DIR/extras/google/google_play_services/libproject/google-play-services_lib
	[ -e $SDK_DIR/extras/android/compatibility/v7/mediarouter/build.xml ] || { \
		android update project -t android-23 -p $SDK_DIR/extras/android/compatibility/v7/mediarouter
		echo 'android.library.reference.1=../../../../../../../../../../../../../../${sdk.dir}/extras/android/compatibility/v7/appcompat' >> $SDK_DIR/extras/android/compatibility/v7/mediarouter/local.properties
	}
	[ -e $SDK_DIR/extras/android/compatibility/v7/appcompat/build.xml ] || \
		android update project -t android-23 -p $SDK_DIR/extras/android/compatibility/v7/appcompat
	[ -e $SDK_DIR/extras/android/compatibility/v7/palette/build.xml ] || \
		android update project -t android-23 -p $SDK_DIR/extras/android/compatibility/v7/palette && \
		mkdir -p $SDK_DIR/extras/android/compatibility/v7/palette/src
fi

if [ -e project/jni/application/src/project.patch ]; then patch -p1 --dry-run -f -R < project/jni/application/src/project.patch > /dev/null 2>&1 || patch -p1 --no-backup-if-mismatch < project/jni/application/src/project.patch || exit 1 ; fi

echo Cleaning up dependencies
rm -rf project/libs/*/* project/gen
for OUT in obj; do
rm -rf project/$OUT/local/*/objs*/sdl_main/* project/$OUT/local/*/libsdl_main.so
rm -rf project/$OUT/local/*/libsdl-*.so
rm -rf project/$OUT/local/*/libsdl_*.so
rm -rf project/$OUT/local/*/objs*/sdl-*/src/*/android
rm -rf project/$OUT/local/*/objs*/sdl-*/src/video/SDL_video.o
rm -rf project/$OUT/local/*/objs*/sdl-*/SDL_renderer_gles.o
rm -rf project/$OUT/local/*/objs*/sdl_*
rm -rf project/obj/local/*/objs*/lzma/src/XZInputStream.o
rm -rf project/obj/local/*/objs*/liblzma.so
# Do not rebuild several huge libraries that do not depend on SDL version
for LIB in freetype intl jpeg png lua mad tremor xerces xml2 curl lua mikmod \
			boost boost_signals boost_thread boost_filesystem boost_date_time boost_system boost_regex boost_iostreams boost_program_options \
			ffmpeg swscale avcodec avdevice avresample avutil avfilter swresample avformat; do
	for ARCH in armeabi armeabi-v7a; do
		if [ -e "project/$OUT/local/$ARCH/objs*/$LIB" ] ; then
			find project/$OUT/local/$ARCH/objs*/$LIB -name "*.o" | xargs touch -c
		fi
	done
done
done
rm -rf project/bin/classes
rm -rf project/bin/res

./copyAssets.sh || exit 1

rm -rf project/jni/android-support
ln -s "`which ndk-build | sed 's@/ndk-build@@'`/sources/android/support" project/jni/android-support

rm -rf project/res/drawable/banner.png
if [ -e project/jni/application/src/banner.png ]; then
	ln -s ../../jni/application/src/banner.png project/res/drawable/banner.png
else
	ln -s ../../themes/tv-banner-placeholder.png project/res/drawable/banner.png
fi

if uname -s | grep -i "darwin" > /dev/null ; then
	find project/src -name "*.killme.tmp" -delete
fi

echo Compiling prebuilt libraries

make -C project/jni -f Makefile.prebuilt

echo Done

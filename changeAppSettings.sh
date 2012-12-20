#!/bin/sh

CHANGE_APP_SETTINGS_VERSION=17
AUTO=
CHANGED=

if [ "X$1" = "X-a" ]; then
	AUTO=a
fi
if [ "X$1" = "X-v" ]; then
	AUTO=v
fi

. ./AndroidAppSettings.cfg

#if [ "$CHANGE_APP_SETTINGS_VERSION" != "$AppSettingVersion" ]; then
#	AUTO=
#fi

var=""

echo
echo "===== libSDL on Android configuration ====="
echo
echo "If you will supply empty string as answer the previous value will be used"

if [ -z "$LibSdlVersion" -o -z "$AUTO" ]; then
echo
echo -n "libSDL version to use (1.2 or 1.3, specify 1.3 for SDL2) ($LibSdlVersion): "
read var
if [ -n "$var" ] ; then
	LibSdlVersion="$var"
	CHANGED=1
fi
fi

if [ -z "$AppName" -o -z "$AUTO" ]; then
echo
echo -n "Specify application name (e.x. My Application) ($AppName): "
read var
if [ -n "$var" ] ; then
	AppName="$var"
	CHANGED=1
fi
fi

if [ -z "$AppFullName" -o -z "$AUTO" ]; then
echo
echo -n "Specify reversed site name of application (e.x. com.mysite.myapp) ($AppFullName): "
read var
if [ -n "$var" ] ; then
	AppFullName="$var"
	CHANGED=1
fi
fi

if [ -z "$ScreenOrientation" -o -z "$AUTO" ]; then
echo
echo -n "Specify screen orientation: (v)ertical or (h)orizontal ($ScreenOrientation): "
read var
if [ -n "$var" ] ; then
	ScreenOrientation="$var"
	CHANGED=1
fi
fi

if [ -z "$AppDataDownloadUrl" -o -z "$AUTO" ]; then
echo
echo "Specify path to download application data in zip archive in the form 'Description|URL|MirrorURL|...'"
echo "You may specify additional paths to additional downloads delimited by newlines (empty line to finish)"
echo "If you'll start Description with '!' symbol it will be enabled by default, other downloads should be selected by user from config menu"
echo "If the URL in in the form ':dir/file.dat:http://URL/' it will be downloaded as-is to the application dir and not unzipped"
echo "If the URL does not contain 'http://' it is treated as file from 'project/jni/application/src/AndroidData' dir -"
echo "these files are put inside .apk package by build system"
echo "Also please avoid 'https://' URLs, many Android devices do not have trust certificates and will fail to connect to SF.net over HTTPS"
echo " "
echo "`echo $AppDataDownloadUrl | tr '^' '\\n'`"
echo
AppDataDownloadUrl1=""
while true; do
	read var
	if [ -n "$var" ] ; then
		if [ -z "$AppDataDownloadUrl1" ]; then
			AppDataDownloadUrl1="$var"
		else
			AppDataDownloadUrl1="$AppDataDownloadUrl1^$var"
		fi
	else
		break
	fi
done
if [ -n "$AppDataDownloadUrl1" ] ; then
	AppDataDownloadUrl="$AppDataDownloadUrl1"
	CHANGED=1
fi
fi

if [ "$LibSdlVersion" = "1.2" ]; then
	if [ -z "$CompatibilityHacks" -o -z "$AUTO" ]; then
		echo
		echo "Application does not call SDL_Flip() or SDL_UpdateRects() appropriately, or draws from non-main thread -"
		echo -n "enabling the compatibility mode will force screen update every halfsecond (y) or (n) ($CompatibilityHacks): "
		read var
		if [ -n "$var" ] ; then
			CompatibilityHacks="$var"
			CHANGED=1
		fi
	fi
	if [ "$CompatibilityHacks" = y ]; then
		SwVideoMode=y
	else
		if [ -z "$SwVideoMode" -o -z "$AUTO" ]; then
			echo
			echo "Application uses software video buffer - you're calling SDL_SetVideoMode() without SDL_HWSURFACE and without SDL_OPENGL,"
			echo -n "this will allow small speed optimization (y) or (n) ($SwVideoMode): "
			read var
			if [ -n "$var" ] ; then
				SwVideoMode="$var"
				CHANGED=1
			fi
		fi
	fi
else
	SwVideoMode=n
fi

if [ -z "$CompatibilityHacksStaticInit" -o -z "$AUTO" ]; then
echo
echo -n "Application initializes SDL audio/video inside static constructors (which is bad, you won't be able to run ndk-gdb) (y)/(n) ($CompatibilityHacksStaticInit): "
read var
if [ -n "$var" ] ; then
	CompatibilityHacksStaticInit="$var"
	CHANGED=1
fi
fi

if [ -z "$VideoDepthBpp" -o -z "$AUTO" ]; then
echo
echo "Video color depth - 16 BPP is the fastest and supported for all modes, 24 bpp is supported only for"
echo -n "software video mode, SDL_OPENGL mode supports everything (16)/(24)/(32) ($VideoDepthBpp): "
read var
if [ -n "$var" ] ; then
	VideoDepthBpp="$var"
	CHANGED=1
fi
fi

if [ "$SwVideoMode" = "y" ]; then
NeedDepthBuffer=n
NeedStencilBuffer=n
NeedGles2=n
else

if [ -z "$NeedDepthBuffer" -o -z "$AUTO" ]; then
echo
echo -n "Enable OpenGL depth buffer (needed only for 3-d applications, small speed decrease) (y) or (n) ($NeedDepthBuffer): "
read var
if [ -n "$var" ] ; then
	NeedDepthBuffer="$var"
	CHANGED=1
fi
fi

if [ -z "$NeedStencilBuffer" -o -z "$AUTO" ]; then
echo
echo -n "Enable OpenGL stencil buffer (needed only for 3-d applications, small speed decrease) (y) or (n) ($NeedDepthBuffer): "
read var
if [ -n "$var" ] ; then
	NeedStencilBuffer="$var"
	CHANGED=1
fi
fi

if [ -z "$NeedGles2" -o -z "$AUTO" ]; then
echo
echo "Try to use GLES 2.x context - will revert to GLES 1.X if unsupported by device"
echo -n "you need this option only if you're developing 3-d app (y) or (n) ($NeedGles2): "
read var
if [ -n "$var" ] ; then
	NeedGles2="$var"
	CHANGED=1
fi
fi

fi

if [ -z "$SdlVideoResize" -o -z "$AUTO" ]; then
echo
echo "Application window should be resized to fit into native device screen (480x320 or 800x480) (y) or (n) ($SdlVideoResize): "
read var
if [ -n "$var" ] ; then
	SdlVideoResize="$var"
	CHANGED=1
fi
fi

if [ -z "$SdlVideoResizeKeepAspect" -o -z "$AUTO" ]; then
echo
echo -n "Application resizing should use 4:3 aspect ratio, creating black bars (y) or (n) ($SdlVideoResizeKeepAspect): "
read var
if [ -n "$var" ] ; then
	SdlVideoResizeKeepAspect="$var"
	CHANGED=1
fi
fi

if [ -z "$AppUsesMouse" -o -z "$AUTO" ]; then
echo
echo -n "Application uses mouse (y) or (n) ($AppUsesMouse): "
read var
if [ -n "$var" ] ; then
	AppUsesMouse="$var"
	CHANGED=1
fi
fi

if [ -z "$AppNeedsTwoButtonMouse" -o -z "$AUTO" ]; then
echo
echo -n "Application needs two-button mouse, will also enable advanced point-and-click features (y) or (n) ($AppNeedsTwoButtonMouse): "
read var
if [ -n "$var" ] ; then
	AppNeedsTwoButtonMouse="$var"
	CHANGED=1
fi
fi

if [ -z "$ForceRelativeMouseMode" -o -z "$AUTO" ]; then
echo
echo -n "Force relative (laptop) mouse movement mode, useful when both on-screen keyboard and mouse are needed (y) or (n) ($ForceRelativeMouseMode): "
read var
if [ -n "$var" ] ; then
	ForceRelativeMouseMode="$var"
	CHANGED=1
fi
fi

if [ -z "$ShowMouseCursor" -o -z "$AUTO" ]; then
echo
echo -n "Show SDL mouse cursor, for applications that do not draw cursor at all (y) or (n) ($ShowMouseCursor): "
read var
if [ -n "$var" ] ; then
	ShowMouseCursor="$var"
	CHANGED=1
fi
fi

if [ -z "$AppNeedsArrowKeys" -o -z "$AUTO" ]; then
echo
echo "Application needs arrow keys (y) or (n), if (y) the accelerometer or touchscreen keyboard"
echo -n "will be used as arrow keys if phone does not have dpad/trackball ($AppNeedsArrowKeys): "
read var
if [ -n "$var" ] ; then
	AppNeedsArrowKeys="$var"
	CHANGED=1
fi
fi

if [ -z "$AppNeedsTextInput" -o -z "$AUTO" ]; then
echo
echo -n "Application needs text input (y) or (n), enables button for text input on screen ($AppNeedsTextInput): "
read var
if [ -n "$var" ] ; then
	AppNeedsTextInput="$var"
	CHANGED=1
fi
fi

if [ -z "$CompatibilityHacksTextInputEmulatesHwKeyboard" -o -z "$AUTO" ]; then
echo
echo -n "On-screen Android soft text input emulates hardware keyboard, this will only work with Hackers Keyboard app (y)/(n) ($CompatibilityHacksTextInputEmulatesHwKeyboard): "
read var
if [ -n "$var" ] ; then
	CompatibilityHacksTextInputEmulatesHwKeyboard="$var"
	CHANGED=1
fi
fi

if [ -z "$CompatibilityHacksPreventAudioChopping" -o -z "$AUTO" ]; then
echo
echo -n "Hack for broken devices: prevent audio chopping, by sleeping a bit after pushing each audio chunk (y)/(n) ($CompatibilityHacksPreventAudioChopping): "
read var
if [ -n "$var" ] ; then
	CompatibilityHacksPreventAudioChopping="$var"
	CHANGED=1
fi
fi

if [ -z "$CompatibilityHacksAppIgnoresAudioBufferSize" -o -z "$AUTO" ]; then
echo
echo -n "Hack for broken apps: application ignores audio buffer size returned by SDL (y)/(n) ($CompatibilityHacksAppIgnoresAudioBufferSize): "
read var
if [ -n "$var" ] ; then
	CompatibilityHacksAppIgnoresAudioBufferSize="$var"
	CHANGED=1
fi
fi

if [ -z "$AppUsesJoystick" -o -z "$AUTO" ]; then
echo
echo "Application uses joystick (y) or (n), the on-screen DPAD will be used"
echo -n " as joystick 0 axes 0-1, you will also need to set AppNeedsArrowKeys=y ($AppUsesJoystick): "
read var
if [ -n "$var" ] ; then
	AppUsesJoystick="$var"
	CHANGED=1
fi
fi

if [ -z "$AppUsesAccelerometer" -o -z "$AUTO" ]; then
echo
echo -n "Application uses accelerometer (y) or (n), the accelerometer will be used as joystick 0 axes 2-3 ($AppUsesAccelerometer): "
read var
if [ -n "$var" ] ; then
	AppUsesAccelerometer="$var"
	CHANGED=1
fi
fi

if [ -z "$AppUsesMultitouch" -o -z "$AUTO" ]; then
echo
echo "Application uses multitouch (y) or (n), multitouch events are passed as SDL_JOYBUTTONDOWN/SDL_JOYBALLMOTION events"
echo -n " for the joystick 0, or additionally as SDL_FINGERDOWN/UP/MOTION events in SDL 1.3 ($AppUsesMultitouch): "
read var
if [ -n "$var" ] ; then
	AppUsesMultitouch="$var"
	CHANGED=1
fi
fi

if [ -z "$NonBlockingSwapBuffers" -o -z "$AUTO" ]; then
echo
echo "Application implements Android-specific routines to put to background, and will not draw anything to screen"
echo "between SDL_ACTIVEEVENT lost / gained notifications - you should check for them"
echo -n "rigth after SDL_Flip(), if (n) then SDL_Flip() will block till app in background (y) or (n) ($NonBlockingSwapBuffers): "
read var
if [ -n "$var" ] ; then
	NonBlockingSwapBuffers="$var"
	CHANGED=1
fi
fi

if [ -z "$InhibitSuspend" -o -z "$AUTO" ]; then
echo
echo "Prevent device from going to sleep while application is running (y) or (n) - this setting is"
echo -n "applied automatically if you're using accelerometer, but may be useful for video players etc ($InhibitSuspend): "
read var
if [ -n "$var" ] ; then
	InhibitSuspend="$var"
	CHANGED=1
fi
fi

if [ -z "$RedefinedKeys" -o -z "$AUTO" ]; then
echo
echo "Redefine common keys to SDL keysyms"
echo "BACK hardware key is available on all devices, MENU is available on pre-ICS devices, other keys may be absent"
echo "SEARCH and CALL by default return same keycode as DPAD_CENTER - one of those keys is available on most devices"
echo "Use word NO_REMAP if you want to preserve native functionality for certain key (volume keys typically)"
echo "TOUCHSCREEN DPAD_CENTER/SEARCH VOLUMEUP VOLUMEDOWN MENU BACK CAMERA - Java keycodes"
echo "$RedefinedKeys - current SDL keycodes"
echo -n ": "
read var
if [ -n "$var" ] ; then
	RedefinedKeys="$var"
	CHANGED=1
fi
fi

if [ -z "$AppTouchscreenKeyboardKeysAmount" -o -z "$AUTO" ]; then
echo
echo -n "Number of virtual keyboard keys (currently 7 is maximum) ($AppTouchscreenKeyboardKeysAmount): "
read var
if [ -n "$var" ] ; then
	AppTouchscreenKeyboardKeysAmount="$var"
	CHANGED=1
fi
fi

if [ -z "$AppTouchscreenKeyboardKeysAmountAutoFire" -o -z "$AUTO" ]; then
echo
echo -n "Number of virtual keyboard keys that support autofire (currently 2 is maximum) ($AppTouchscreenKeyboardKeysAmountAutoFire): "
read var
if [ -n "$var" ] ; then
	AppTouchscreenKeyboardKeysAmountAutoFire="$var"
	CHANGED=1
fi
fi

if [ -z "$RedefinedKeysScreenKb" -o -z "$AUTO" ]; then
if [ -z "$RedefinedKeysScreenKb" ]; then
	RedefinedKeysScreenKb="$RedefinedKeys"
	CHANGED=1
fi
echo
echo "Redefine on-screen keyboard keys to SDL keysyms - 6 keyboard keys + 4 multitouch gestures (zoom in/out and rotate left/right)"
echo "$RedefinedKeysScreenKb - current SDL keycodes"
echo -n ": "
read var
if [ -n "$var" ] ; then
	RedefinedKeysScreenKb="$var"
	CHANGED=1
fi
fi

if [ -z "$StartupMenuButtonTimeout" -o -z "$AUTO" ]; then
echo
echo -n "How long to show startup menu button, in msec, 0 to disable startup menu ($StartupMenuButtonTimeout): "
read var
if [ -n "$var" ] ; then
	StartupMenuButtonTimeout="$var"
	CHANGED=1
fi
fi

if [ -z "$AUTO" ]; then
echo
echo "Menu items to hide from startup menu, available menu items:"
echo `grep 'extends Menu' project/java/Settings.java | sed 's/.* class \(.*\) extends .*/\1/'`
echo "($HiddenMenuOptions)"
echo -n ": "
read var
if [ -n "$var" ] ; then
	HiddenMenuOptions="$var"
	CHANGED=1
fi
fi

FirstStartMenuOptionsDefault='(AppUsesMouse \&\& \! ForceRelativeMouseMode ? new Settings.DisplaySizeConfig(true) : new Settings.DummyMenu()), new Settings.OptionalDownloadConfig(true)'
if [ -z "$AUTO" ]; then
echo
echo "Menu items to show at startup - this is Java code snippet, leave empty for default"
echo $FirstStartMenuOptionsDefault
echo "Available menu items:"
echo `grep 'extends Menu' project/java/Settings.java | sed 's/.* class \(.*\) extends .*/new Settings.\1(), /'`
echo "Current value: " "$FirstStartMenuOptions"
echo -n ": "
read var
if [ -n "$var" ] ; then
	FirstStartMenuOptions="$var"
	CHANGED=1
fi
fi

if [ -z "$MultiABI" -o -z "$AUTO" ]; then
echo
echo "Enable multi-ABI binary, with hardware FPU support - it will also work on old devices,"
echo -n "but .apk size is 2x bigger (y) / (n) / (x86) / (all) ($MultiABI): "
read var
if [ -n "$var" ] ; then
	MultiABI="$var"
	CHANGED=1
fi
fi

if [ -z "$AppMinimumRAM" -o -z "$AUTO" ]; then
echo
echo -n "Minimum amount of RAM application requires, in Mb, SDL will print warning to user if it's lower ($AppMinimumRAM): "
read var
if [ -n "$var" ] ; then
	AppMinimumRAM="$var"
	CHANGED=1
fi
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

if [ -z "$ResetSdlConfigForThisVersion" -o -z "$AUTO" ]; then
echo
echo -n "Reset SDL config when updating application to the new version (y) / (n) ($ResetSdlConfigForThisVersion): "
read var
if [ -n "$var" ] ; then
	ResetSdlConfigForThisVersion="$var"
	CHANGED=1
fi
fi

if [ -z "$DeleteFilesOnUpgrade" -o -z "$AUTO" ]; then
echo
echo -n "Delete application data files when upgrading (specify file/dir paths separated by spaces): ($DeleteFilesOnUpgrade): "
read var
if [ -n "$var" ] ; then
	DeleteFilesOnUpgrade="$var"
	CHANGED=1
fi
fi

if [ -z "$CustomBuildScript" -o -z "$AUTO" ]; then
echo
echo -n "Application uses custom build script AndroidBuild.sh instead of Android.mk (y) or (n) ($CustomBuildScript): "
read var
if [ -n "$var" ] ; then
	CustomBuildScript="$var"
	CHANGED=1
fi
fi

if [ -z "$AUTO" ]; then
echo
echo -n "Aditional CFLAGS for application ($AppCflags): "
read var
if [ -n "$var" ] ; then
	AppCflags="$var"
	CHANGED=1
fi
fi

if [ -z "$AUTO" ]; then
echo
echo "Optional shared libraries to compile - removing some of them will save space"
echo "MP3 support by libMAD is encumbered by patents and libMAD is GPL-ed"
grep 'Available' project/jni/SettingsTemplate.mk 
grep 'depends on' project/jni/SettingsTemplate.mk
echo "Current: $CompiledLibraries"
echo -n ": "
read var
if [ -n "$var" ] ; then
	CompiledLibraries="$var"
	CHANGED=1
fi
fi

if [ -z "$AUTO" ]; then
echo
echo -n "Additional LDFLAGS for application ($AppLdflags): "
read var
if [ -n "$var" ] ; then
	AppLdflags="$var"
	CHANGED=1
fi
fi

if [ -z "$AUTO" ]; then
echo
echo -n "Build only following subdirs (empty will build all dirs, ignored with custom script) ($AppSubdirsBuild): "
read var
if [ -n "$var" ] ; then
	AppSubdirsBuild="$var"
	CHANGED=1
fi
fi

if [ -z "$AUTO" ]; then
echo
echo -n "Application command line parameters, including app name as 0-th param ($AppCmdline): "
read var
if [ -n "$var" ] ; then
	AppCmdline="$var"
	CHANGED=1
fi
fi

if [ -z "$ReadmeText" -o -z "$AUTO" ]; then
echo
echo "Here you may type some short readme text - it is currently not used anywhere"
echo "Current text:"
echo
echo "`echo $ReadmeText | tr '^' '\\n'`"
echo
echo "New text (empty line to finish):"
echo

ReadmeText1=""
while true; do
	read var
	if [ -n "$var" ] ; then
		if [ -n "$ReadmeText1" ] ; then
			ReadmeText1="$ReadmeText1^$var"
		else
			ReadmeText1="$var"
		fi
	else
		break
	fi
done
if [ -n "$ReadmeText1" ] ; then
	ReadmeText="$ReadmeText1"
	CHANGED=1
fi
fi

if [ -z "$AUTO" -o -z "$AdmobPublisherId" ]; then
echo
echo -n "Your AdMob Publisher ID, (n) if you don't want advertisements ($AdmobPublisherId): "
read var
if [ -n "$var" ] ; then
	AdmobPublisherId="$var"
	CHANGED=1
fi
fi

if [ "$AdmobPublisherId" '!=' "n" ]; then
if [ -z "$AUTO" -o -z "$AdmobTestDeviceId" ]; then
echo
echo -n "Your AdMob test device ID, to receive a test ad ($AdmobTestDeviceId): "
read var
if [ -n "$var" ] ; then
	AdmobTestDeviceId="$var"
	CHANGED=1
fi
fi
if [ -z "$AUTO" -o -z "$AdmobBannerSize" ]; then
echo
echo -n "Your AdMob banner size (BANNER/IAB_BANNER/IAB_LEADERBOARD/IAB_MRECT/IAB_WIDE_SKYSCRAPER/SMART_BANNER) ($AdmobBannerSize): "
read var
if [ -n "$var" ] ; then
	AdmobBannerSize="$var"
	CHANGED=1
fi
fi
fi

if [ -z "$AUTO" -o -z "$MinimumScreenSize" ]; then
echo
echo "Screen size is used by Google Play to prevent an app to be installed on devices with smaller screens"
echo -n "Minimum screen size that application supports: (s)mall / (m)edium / (l)arge ($MinimumScreenSize): "
read var
if [ -n "$var" ] ; then
	MinimumScreenSize="$var"
	CHANGED=1
fi
fi

echo

if [ -n "$CHANGED" ]; then
cat /dev/null > AndroidAppSettings.cfg
echo "# The application settings for Android libSDL port" >> AndroidAppSettings.cfg
echo AppSettingVersion=$CHANGE_APP_SETTINGS_VERSION >> AndroidAppSettings.cfg
echo LibSdlVersion=$LibSdlVersion >> AndroidAppSettings.cfg
echo AppName=\"$AppName\" >> AndroidAppSettings.cfg
echo AppFullName=$AppFullName >> AndroidAppSettings.cfg
echo ScreenOrientation=$ScreenOrientation >> AndroidAppSettings.cfg
echo InhibitSuspend=$InhibitSuspend >> AndroidAppSettings.cfg
echo AppDataDownloadUrl=\"$AppDataDownloadUrl\" >> AndroidAppSettings.cfg
echo VideoDepthBpp=$VideoDepthBpp >> AndroidAppSettings.cfg
echo NeedDepthBuffer=$NeedDepthBuffer >> AndroidAppSettings.cfg
echo NeedStencilBuffer=$NeedStencilBuffer >> AndroidAppSettings.cfg
echo NeedGles2=$NeedGles2 >> AndroidAppSettings.cfg
echo SwVideoMode=$SwVideoMode >> AndroidAppSettings.cfg
echo SdlVideoResize=$SdlVideoResize >> AndroidAppSettings.cfg
echo SdlVideoResizeKeepAspect=$SdlVideoResizeKeepAspect >> AndroidAppSettings.cfg
echo CompatibilityHacks=$CompatibilityHacks >> AndroidAppSettings.cfg
echo CompatibilityHacksStaticInit=$CompatibilityHacksStaticInit >> AndroidAppSettings.cfg
echo CompatibilityHacksTextInputEmulatesHwKeyboard=$CompatibilityHacksTextInputEmulatesHwKeyboard >> AndroidAppSettings.cfg
echo CompatibilityHacksPreventAudioChopping=$CompatibilityHacksPreventAudioChopping >> AndroidAppSettings.cfg
echo CompatibilityHacksAppIgnoresAudioBufferSize=$CompatibilityHacksAppIgnoresAudioBufferSize >> AndroidAppSettings.cfg
echo AppUsesMouse=$AppUsesMouse >> AndroidAppSettings.cfg
echo AppNeedsTwoButtonMouse=$AppNeedsTwoButtonMouse >> AndroidAppSettings.cfg
echo ShowMouseCursor=$ShowMouseCursor >> AndroidAppSettings.cfg
echo ForceRelativeMouseMode=$ForceRelativeMouseMode >> AndroidAppSettings.cfg
echo AppNeedsArrowKeys=$AppNeedsArrowKeys >> AndroidAppSettings.cfg
echo AppNeedsTextInput=$AppNeedsTextInput >> AndroidAppSettings.cfg
echo AppUsesJoystick=$AppUsesJoystick >> AndroidAppSettings.cfg
echo AppUsesAccelerometer=$AppUsesAccelerometer >> AndroidAppSettings.cfg
echo AppUsesMultitouch=$AppUsesMultitouch >> AndroidAppSettings.cfg
echo NonBlockingSwapBuffers=$NonBlockingSwapBuffers >> AndroidAppSettings.cfg
echo RedefinedKeys=\"$RedefinedKeys\" >> AndroidAppSettings.cfg
echo AppTouchscreenKeyboardKeysAmount=$AppTouchscreenKeyboardKeysAmount >> AndroidAppSettings.cfg
echo AppTouchscreenKeyboardKeysAmountAutoFire=$AppTouchscreenKeyboardKeysAmountAutoFire >> AndroidAppSettings.cfg
echo RedefinedKeysScreenKb=\"$RedefinedKeysScreenKb\" >> AndroidAppSettings.cfg
echo StartupMenuButtonTimeout=$StartupMenuButtonTimeout >> AndroidAppSettings.cfg
echo HiddenMenuOptions=\'$HiddenMenuOptions\' >> AndroidAppSettings.cfg
echo FirstStartMenuOptions=\'$FirstStartMenuOptions\' >> AndroidAppSettings.cfg
echo MultiABI=$MultiABI >> AndroidAppSettings.cfg
echo AppMinimumRAM=$AppMinimumRAM >> AndroidAppSettings.cfg
echo AppVersionCode=$AppVersionCode >> AndroidAppSettings.cfg
echo AppVersionName=\"$AppVersionName\" >> AndroidAppSettings.cfg
echo ResetSdlConfigForThisVersion=$ResetSdlConfigForThisVersion >> AndroidAppSettings.cfg
echo DeleteFilesOnUpgrade=\"$DeleteFilesOnUpgrade\" >> AndroidAppSettings.cfg
echo CompiledLibraries=\"$CompiledLibraries\" >> AndroidAppSettings.cfg
echo CustomBuildScript=$CustomBuildScript >> AndroidAppSettings.cfg
echo AppCflags=\'$AppCflags\' >> AndroidAppSettings.cfg
echo AppLdflags=\'$AppLdflags\' >> AndroidAppSettings.cfg
echo AppSubdirsBuild=\'$AppSubdirsBuild\' >> AndroidAppSettings.cfg
echo AppCmdline=\'$AppCmdline\' >> AndroidAppSettings.cfg
echo ReadmeText=\'$ReadmeText\' >> AndroidAppSettings.cfg
echo MinimumScreenSize=$MinimumScreenSize >> AndroidAppSettings.cfg
echo AdmobPublisherId=$AdmobPublisherId >> AndroidAppSettings.cfg
echo AdmobTestDeviceId=$AdmobTestDeviceId >> AndroidAppSettings.cfg
echo AdmobBannerSize=$AdmobBannerSize >> AndroidAppSettings.cfg
fi

AppShortName=`echo $AppName | sed 's/ //g'`
DataPath="$AppFullName"
AppFullNameUnderscored=`echo $AppFullName | sed 's/[.]/_/g'`
AppSharedLibrariesPath=/data/data/$AppFullName/lib
ScreenOrientation1=portrait
HorizontalOrientation=false

UsingSdl13=false
if [ "$LibSdlVersion" = "1.3" ] ; then
	UsingSdl13=true
fi

if [ "$ScreenOrientation" = "h" ] ; then
	ScreenOrientation1=landscape
	HorizontalOrientation=true
fi

AppDataDownloadUrl1="`echo $AppDataDownloadUrl | sed 's/[&]/%26/g'`"

if [ "$SdlVideoResize" = "y" ] ; then
	SdlVideoResize=1
else
	SdlVideoResize=0
fi

if [ "$SdlVideoResizeKeepAspect" = "y" ] ; then
	SdlVideoResizeKeepAspect=1
else
	SdlVideoResizeKeepAspect=0
fi

if [ "$InhibitSuspend" = "y" ] ; then
	InhibitSuspend=true
else
	InhibitSuspend=false
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

if [ "$SwVideoMode" = "y" ] ; then
	SwVideoMode=true
else
	SwVideoMode=false
fi

if [ "$CompatibilityHacks" = "y" ] ; then
	CompatibilityHacks=true
else
	CompatibilityHacks=false
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

if [ "$AppNeedsArrowKeys" = "y" ] ; then
	AppNeedsArrowKeys=true
else
	AppNeedsArrowKeys=false
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

if [ "$AppUsesAccelerometer" = "y" ] ; then
	AppUsesAccelerometer=true
else
	AppUsesAccelerometer=false
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

if [ "$MultiABI" = "y" ] ; then
	MultiABI="armeabi armeabi-v7a"
elif [ "$MultiABI" = "x86" ] ; then
	MultiABI="armeabi x86"
elif [ "$MultiABI" = "all" ] ; then
	MultiABI="all" # Starting form NDK r7
else
	MultiABI="armeabi"
fi

LibrariesToLoad="\\\"sdl-$LibSdlVersion\\\""
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

if [ "$CustomBuildScript" = "n" ] ; then
	CustomBuildScript=
fi

HiddenMenuOptions1=""
for F in $HiddenMenuOptions; do
	HiddenMenuOptions1="$HiddenMenuOptions1 new Settings.$F(),"
done

if [ -z "$FirstStartMenuOptions" ]; then
	FirstStartMenuOptions="$FirstStartMenuOptionsDefault"
fi

ReadmeText="`echo $ReadmeText | sed 's/\"/\\\\\\\\\"/g' | sed 's/[&%]//g'`"

rm -rf project/src
mkdir -p project/src
cd project/java
for F in *.java; do
	echo Patching $F
	echo '// DO NOT EDIT THIS FILE - it is automatically generated, edit file under project/java dir' > ../src/$F
	cat $F | sed "s/package .*;/package $AppFullName;/" >> ../src/$F
done
cd ../..

echo Patching project/AndroidManifest.xml
cat project/AndroidManifestTemplate.xml | \
	sed "s/package=.*/package=\"$AppFullName\"/" | \
	sed "s/android:screenOrientation=.*/android:screenOrientation=\"$ScreenOrientation1\"/" | \
	sed "s^android:versionCode=.*^android:versionCode=\"$AppVersionCode\"^" | \
	sed "s^android:versionName=.*^android:versionName=\"$AppVersionName\"^" > \
	project/AndroidManifest.xml
if [ "$AdmobPublisherId" = "n" -o -z "$AdmobPublisherId" ] ; then
	sed -i "/==ADMOB==/ d" project/AndroidManifest.xml
	AdmobPublisherId=""
else
	F=project/java/admob/Advertisement.java
	echo Patching $F
	echo '// DO NOT EDIT THIS FILE - it is automatically generated, edit file under project/java dir' > project/src/Advertisement.java
	cat $F | sed "s/package .*;/package $AppFullName;/" >> project/src/Advertisement.java
fi

case "$MinimumScreenSize" in
	n|m)
		sed -i "/==SCREEN-SIZE-SMALL==/ d" project/AndroidManifest.xml
		sed -i "/==SCREEN-SIZE-LARGE==/ d" project/AndroidManifest.xml
		;;
	l)
		sed -i "/==SCREEN-SIZE-SMALL==/ d" project/AndroidManifest.xml
		sed -i "/==SCREEN-SIZE-NORMAL==/ d" project/AndroidManifest.xml
		;;
	*)
		sed -i "/==SCREEN-SIZE-NORMAL==/ d" project/AndroidManifest.xml
		sed -i "/==SCREEN-SIZE-LARGE==/ d" project/AndroidManifest.xml
		;;
esac

echo Patching project/src/Globals.java
cat project/src/Globals.java | \
	sed "s/public static String ApplicationName = .*;/public static String ApplicationName = \"$AppShortName\";/" | \
	sed "s/public static final boolean Using_SDL_1_3 = .*;/public static final boolean Using_SDL_1_3 = $UsingSdl13;/" | \
	sed "s@public static String DataDownloadUrl = .*@public static String DataDownloadUrl = \"$AppDataDownloadUrl1\";@" | \
	sed "s/public static boolean SwVideoMode = .*;/public static boolean SwVideoMode = $SwVideoMode;/" | \
	sed "s/public static int VideoDepthBpp = .*;/public static int VideoDepthBpp = $VideoDepthBpp;/" | \
	sed "s/public static boolean NeedDepthBuffer = .*;/public static boolean NeedDepthBuffer = $NeedDepthBuffer;/" | \
	sed "s/public static boolean NeedStencilBuffer = .*;/public static boolean NeedStencilBuffer = $NeedStencilBuffer;/" | \
	sed "s/public static boolean NeedGles2 = .*;/public static boolean NeedGles2 = $NeedGles2;/" | \
	sed "s/public static boolean CompatibilityHacksVideo = .*;/public static boolean CompatibilityHacksVideo = $CompatibilityHacks;/" | \
	sed "s/public static boolean CompatibilityHacksStaticInit = .*;/public static boolean CompatibilityHacksStaticInit = $CompatibilityHacksStaticInit;/" | \
	sed "s/public static boolean CompatibilityHacksTextInputEmulatesHwKeyboard = .*;/public static boolean CompatibilityHacksTextInputEmulatesHwKeyboard = $CompatibilityHacksTextInputEmulatesHwKeyboard;/" | \
	sed "s/public static boolean HorizontalOrientation = .*;/public static boolean HorizontalOrientation = $HorizontalOrientation;/" | \
	sed "s/public static boolean InhibitSuspend = .*;/public static boolean InhibitSuspend = $InhibitSuspend;/" | \
	sed "s/public static boolean AppUsesMouse = .*;/public static boolean AppUsesMouse = $AppUsesMouse;/" | \
	sed "s/public static boolean AppNeedsTwoButtonMouse = .*;/public static boolean AppNeedsTwoButtonMouse = $AppNeedsTwoButtonMouse;/" | \
	sed "s/public static boolean ForceRelativeMouseMode = .*;/public static boolean ForceRelativeMouseMode = $ForceRelativeMouseMode;/" | \
	sed "s/public static boolean ShowMouseCursor = .*;/public static boolean ShowMouseCursor = $ShowMouseCursor;/" | \
	sed "s/public static boolean AppNeedsArrowKeys = .*;/public static boolean AppNeedsArrowKeys = $AppNeedsArrowKeys;/" | \
	sed "s/public static boolean AppNeedsTextInput = .*;/public static boolean AppNeedsTextInput = $AppNeedsTextInput;/" | \
	sed "s/public static boolean AppUsesJoystick = .*;/public static boolean AppUsesJoystick = $AppUsesJoystick;/" | \
	sed "s/public static boolean AppUsesAccelerometer = .*;/public static boolean AppUsesAccelerometer = $AppUsesAccelerometer;/" | \
	sed "s/public static boolean AppUsesMultitouch = .*;/public static boolean AppUsesMultitouch = $AppUsesMultitouch;/" | \
	sed "s/public static boolean NonBlockingSwapBuffers = .*;/public static boolean NonBlockingSwapBuffers = $NonBlockingSwapBuffers;/" | \
	sed "s/public static boolean ResetSdlConfigForThisVersion = .*;/public static boolean ResetSdlConfigForThisVersion = $ResetSdlConfigForThisVersion;/" | \
	sed "s|public static String DeleteFilesOnUpgrade = .*;|public static String DeleteFilesOnUpgrade = \"$DeleteFilesOnUpgrade\";|" | \
	sed "s/public static int AppTouchscreenKeyboardKeysAmount = .*;/public static int AppTouchscreenKeyboardKeysAmount = $AppTouchscreenKeyboardKeysAmount;/" | \
	sed "s/public static int AppTouchscreenKeyboardKeysAmountAutoFire = .*;/public static int AppTouchscreenKeyboardKeysAmountAutoFire = $AppTouchscreenKeyboardKeysAmountAutoFire;/" | \
	sed "s/public static int StartupMenuButtonTimeout = .*;/public static int StartupMenuButtonTimeout = $StartupMenuButtonTimeout;/" | \
	sed "s/public static int AppMinimumRAM = .*;/public static int AppMinimumRAM = $AppMinimumRAM;/" | \
	sed "s/public static Settings.Menu HiddenMenuOptions .*;/public static Settings.Menu HiddenMenuOptions [] = { $HiddenMenuOptions1 };/" | \
	sed "s@public static Settings.Menu FirstStartMenuOptions .*;@public static Settings.Menu FirstStartMenuOptions [] = { $FirstStartMenuOptions };@" | \
	sed "s%public static String ReadmeText = .*%public static String ReadmeText = \"$ReadmeText\";%" | \
	sed "s%public static String CommandLine = .*%public static String CommandLine = \"$AppCmdline\";%" | \
	sed "s/public static String AdmobPublisherId = .*/public static String AdmobPublisherId = \"$AdmobPublisherId\";/" | \
	sed "s/public static String AdmobTestDeviceId = .*/public static String AdmobTestDeviceId = \"$AdmobTestDeviceId\";/" | \
	sed "s/public static String AdmobBannerSize = .*/public static String AdmobBannerSize = \"$AdmobBannerSize\";/" | \
	sed "s/public static String AppLibraries.*/public static String AppLibraries[] = { $LibrariesToLoad };/" > \
	project/src/Globals.java.1
mv -f project/src/Globals.java.1 project/src/Globals.java

echo Patching project/jni/Settings.mk
echo '# DO NOT EDIT THIS FILE - it is automatically generated, edit file SettingsTemplate.mk' > project/jni/Settings.mk
cat project/jni/SettingsTemplate.mk | \
	sed "s/APP_MODULES := .*/APP_MODULES := application sdl-$LibSdlVersion sdl_main stlport jpeg png ogg flac vorbis freetype $CompiledLibraries/" | \
	sed "s/APP_ABI := .*/APP_ABI := $MultiABI/" | \
	sed "s/SDL_JAVA_PACKAGE_PATH := .*/SDL_JAVA_PACKAGE_PATH := $AppFullNameUnderscored/" | \
	sed "s^SDL_CURDIR_PATH := .*^SDL_CURDIR_PATH := $DataPath^" | \
	sed "s^SDL_VIDEO_RENDER_RESIZE := .*^SDL_VIDEO_RENDER_RESIZE := $SdlVideoResize^" | \
	sed "s^SDL_VIDEO_RENDER_RESIZE_KEEP_ASPECT := .*^SDL_VIDEO_RENDER_RESIZE_KEEP_ASPECT := $SdlVideoResizeKeepAspect^" | \
	sed "s^COMPILED_LIBRARIES := .*^COMPILED_LIBRARIES := $CompiledLibraries^" | \
	sed "s^APPLICATION_ADDITIONAL_CFLAGS :=.*^APPLICATION_ADDITIONAL_CFLAGS := $AppCflags^" | \
	sed "s^APPLICATION_ADDITIONAL_LDFLAGS :=.*^APPLICATION_ADDITIONAL_LDFLAGS := $AppLdflags^" | \
	sed "s^SDL_ADDITIONAL_CFLAGS :=.*^SDL_ADDITIONAL_CFLAGS := $RedefinedKeycodes $RedefinedKeycodesScreenKb $CompatibilityHacksPreventAudioChopping $CompatibilityHacksAppIgnoresAudioBufferSize^" | \
	sed "s^APPLICATION_SUBDIRS_BUILD :=.*^APPLICATION_SUBDIRS_BUILD := $AppSubdirsBuild^" | \
	sed "s^APPLICATION_CUSTOM_BUILD_SCRIPT :=.*^APPLICATION_CUSTOM_BUILD_SCRIPT := $CustomBuildScript^" | \
	sed "s^SDL_VERSION :=.*^SDL_VERSION := $LibSdlVersion^"  >> \
	project/jni/Settings.mk

echo Patching strings.xml
rm -rf project/res/values*
cd project/java/translations
for F in */strings.xml; do
	mkdir -p ../../res/`dirname $F`
	cat $F | \
	sed "s^[<]string name=\"app_name\"[>].*^<string name=\"app_name\">$AppName</string>^" > \
	../../res/$F
done
cd ../../..

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
# Do not rebuild several huge libraries that do not depend on SDL version
for LIB in freetype intl jpeg png lua mad stlport tremor xerces xml2; do
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

echo Done

You will need to install some packages to your Debian/Ubuntu first:

sudo apt-get install bison libpixman-1-dev \
libxfont-dev libxkbfile-dev libpciaccess-dev \
xutils-dev xcb-proto python-xcbgen xsltproc \
x11proto-bigreqs-dev x11proto-composite-dev \
x11proto-core-dev x11proto-damage-dev \
x11proto-dmx-dev x11proto-dri2-dev x11proto-fixes-dev \
x11proto-fonts-dev x11proto-gl-dev \
x11proto-input-dev x11proto-kb-dev \
x11proto-print-dev x11proto-randr-dev \
x11proto-record-dev x11proto-render-dev \
x11proto-resource-dev x11proto-scrnsaver-dev \
x11proto-video-dev x11proto-xcmisc-dev \
x11proto-xext-dev x11proto-xf86bigfont-dev \
x11proto-xf86dga-dev x11proto-xf86dri-dev \
x11proto-xf86vidmode-dev x11proto-xinerama-dev \
libxmuu-dev libxt-dev libsm-dev libice-dev \
libxrender-dev libxrandr-dev curl

Then run commands:

git clone git@github.com:pelya/commandergenius.git sdl-android
cd sdl-android
git submodule update --init project/jni/application/xserver/xserver
rm project/jni/application/src
ln -s xserver-gimp project/jni/application/src
./changeAppSettings.sh -a
android update project -p project
./build.sh

To build system image, download repo:

https://github.com/pelya/cuntubuntu

install dependencies described in it's readme,
then launch script img-debug-wheezy-armhf-gimp.sh from directory img.
This will create Debian system image under directory
dist-debug-wheezy-armhf-gimp. Execute commands (<SDL> is path to this repo):

sudo cp -a <SDL>/project/jni/application/src/xserver/data/usr/ dist-debug-wheezy-armhf-gimp/usr/
sudo cp -a <SDL>/project/jni/application/src/xserver/android/xhost dist-debug-wheezy-armhf-gimp/usr/bin/
sudo cp -a <SDL>/project/jni/application/src/xserver/android/xkbcomp dist-debug-wheezy-armhf-gimp/usr/bin/
sudo cp -a <SDL>/project/jni/application/src/xserver/android/xli dist-debug-wheezy-armhf-gimp/usr/bin/

cd dist-debug-wheezy-armhf-gimp
sudo tar cvzf ../dist-debug-wheezy-armhf-gimp.tar.gz *

Upload resulting system image somewhere, and change download URL inside
AndroidAppSettings.cfg, then recompile .apk file.

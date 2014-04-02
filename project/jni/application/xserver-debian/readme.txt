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

You will need both xcb-proto and python-xcbgen packages
to have version 1.10-1, you may download newer packages
from http://packages.ubuntu.com/ or https://www.debian.org/distrib/packages

Then run commands:

git clone git@github.com:pelya/commandergenius.git sdl-android
cd sdl-android
git submodule update --init --recursive project/jni/application/xserver-debian/xserver
rm project/jni/application/src
ln -s xserver-debian project/jni/application/src
./changeAppSettings.sh -a
android update project -p project
./build.sh

To build system image, download repo:

https://github.com/pelya/cuntubuntu

Install dependencies described in it's readme,
then go to directory img and launch script

./img-debug-wheezy-proot.sh

This will create Debian system image.

Upload resulting system image somewhere, and change download URL inside
AndroidAppSettings.cfg, then recompile .apk file.

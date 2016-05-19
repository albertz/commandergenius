You will need to install some packages to your Debian/Ubuntu first.

You will need both xcb-proto and python-xcbgen packages to have version 1.10-1 or newer,
so you need to build on at least Debian Jessie or newer distribution.

Install following packages, assuming fresh Debian Jessie installation for x86_64 architecture:

sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install bison libpixman-1-dev libxfont-dev libxkbfile-dev libpciaccess-dev xutils-dev \
xcb-proto python-xcbgen xsltproc x11proto-bigreqs-dev x11proto-composite-dev x11proto-core-dev \
x11proto-damage-dev x11proto-dmx-dev x11proto-dri2-dev x11proto-fixes-dev x11proto-fonts-dev \
x11proto-gl-dev x11proto-input-dev x11proto-kb-dev x11proto-print-dev x11proto-randr-dev \
x11proto-record-dev x11proto-render-dev x11proto-resource-dev x11proto-scrnsaver-dev \
x11proto-video-dev x11proto-xcmisc-dev x11proto-xext-dev x11proto-xf86bigfont-dev \
x11proto-xf86dga-dev x11proto-xf86dri-dev x11proto-xf86vidmode-dev x11proto-xinerama-dev \
libxmuu-dev libxt-dev libsm-dev libice-dev libxrender-dev libxrandr-dev xfonts-utils \
curl autoconf automake libtool pkg-config libjpeg-dev libpng-dev git mc locales \
openjdk-7-jdk ant make zip libstdc++6:i386 libgcc1:i386 zlib1g:i386 libncurses5:i386

Install Android NDK r11c and Android SDK with Android 6.0 framework, they must be in your $PATH.

Download SDL repo, select xserver project, and build it:

git clone git@github.com:pelya/commandergenius.git sdl-android
cd sdl-android
git submodule update --init --recursive
rm -f project/jni/application/src
ln -s xserver project/jni/application/src
./build.sh

That's all.

Following instructions are to set up a fresh compilation environment inside a chroot,
use them only if you fail to build on your current setup.

Install debootstrap and schroot onto your host Debian or Ubuntu:

sudo apt-get install schroot debootstrap

sudo debootstrap --arch=amd64 --components=sudo jessie jessie-x64 http://ftp.de.debian.org/debian/

Configure schroot to log into it - add file /etc/schroot/chroot.d/jessie-x64.conf with following content:

[jessie-x64]
description=Debian Jessie x64
type=directory
directory=/path/to/jessie-x64
users=your-user-ID
groups=your-user-ID
root-groups=root
aliases=x64

Log into it with command

schroot -c jessie-x64

Configure apt sources in the resulting Jessie installation - edit file /etc/apt/sources.list to look like this:

deb http://ftp.de.debian.org/debian/ jessie contrib main non-free
deb-src http://ftp.de.debian.org/debian/ jessie main contrib
deb http://security.debian.org/ jessie/updates contrib main non-free
deb-src http://security.debian.org/ jessie/updates main contrib
deb http://ftp.de.debian.org/debian/ jessie-updates contrib main non-free
deb-src http://ftp.de.debian.org/debian/ jessie-updates main contrib
deb http://ftp.de.debian.org/debian/ jessie-backports contrib main non-free
deb-src http://ftp.de.debian.org/debian/ jessie-backports contrib main

Install necessary Debian packages, as described above.
Configure locales with command:

sudo dpkg-reconfigure locales

Select en_US.UTF-8, we don't need any weird non-English languages here.
Exit chroot and log into it again.
Install Android NDK r10e and Android SDK with Android 5.1 framework, they must be in your $PATH.
Then download SDL repo, select xserver project, and build it, as described above.

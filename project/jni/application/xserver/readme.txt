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
libxrender-dev libxrandr-dev curl autoconf automake libtool \
pkg-config libjpeg-dev libpng-dev

You will need both xcb-proto and python-xcbgen packages
to have version 1.10-1, you may download newer packages
from http://packages.ubuntu.com/ or https://www.debian.org/distrib/packages

Then run commands:

git clone git@github.com:pelya/commandergenius.git sdl-android
cd sdl-android
git submodule update --init project/jni/application/xserver/xserver
rm project/jni/application/src
ln -s xserver project/jni/application/src
./changeAppSettings.sh -a
android update project -p project
./build.sh

New releases of XSDL contain statically linked xkbcomp, xli and xhost executables
for XSDL, because NDK r10c toolchain for some reason builds xkbcomp, which crashes
on Toshiba AT-330 with Android 4.0.3. To create these executables, you will need
to create Debian x86 and armhf chroot installations, like this:

sudo apt-get install qemu-user-static

sudo qemu-debootstrap --arch=i386 --verbose \
        --components=main,universe,restricted,multiverse \
        --include=fakeroot,libc-bin,locales-all,build-essential,sudo \
        wheezy wheezy-x86 http://ftp.ua.debian.org/debian/

sudo qemu-debootstrap --arch=armhf --verbose \
        --components=main,universe,restricted,multiverse \
        --include=fakeroot,libc-bin,locales-all,build-essential,sudo \
        wheezy wheezy-armhf http://ftp.ua.debian.org/debian/

sudo qemu-debootstrap --arch=mipsel --verbose \
        --components=main,universe,restricted,multiverse \
        --include=fakeroot,libc-bin,locales-all,build-essential,sudo \
        wheezy wheezy-mipsel http://ftp.ua.debian.org/debian/

Put this into /etc/apt/sources.list in each chroot, then do sudo apt-get update:

deb http://http.debian.net/debian/ wheezy contrib main non-free
deb-src http://http.debian.net/debian/ wheezy main contrib

deb http://security.debian.org/ wheezy/updates contrib main non-free
deb-src http://security.debian.org/ wheezy/updates main contrib

deb http://http.debian.net/debian/ wheezy-updates contrib main non-free
deb-src http://http.debian.net/debian/ wheezy-updates main contrib

deb http://http.debian.net/debian/ wheezy-backports contrib main non-free
deb-src http://http.debian.net/debian/ wheezy-backports contrib main

Go to each of these chroots using chroot or schroot command, install all packages
mentioned in the previous section, then download and compile static executables:

wget http://cgit.freedesktop.org/xorg/app/xhost/snapshot/xhost-1.0.6.tar.gz
wget http://cgit.freedesktop.org/xorg/app/xkbcomp/snapshot/xkbcomp-1.2.4.tar.gz
apt-get source xli
wget https://github.com/kfish/xsel/archive/master.tar.gz

xhost:
./autogen.sh
env XHOST_LIBS="-static -lX11 -lxcb -lXau -lXdmcp -lXmuu -lpthread" ./configure
make V=1

xkbcomp:
./autogen.sh
env XKBCOMP_LIBS="-static -lxkbfile -lX11 -lxcb -lXau -lXdmcp -lXmuu -lpthread" ./configure
make V=1

xli:
cat debian/patches/series | while read F ; do patch -p1 < debian/patches/$F ; done
xmkmf
env EXTRA_LIBRARIES="-static -lxcb -lXau -lXdmcp -lXmuu -lpthread -ldl" make -e

xsel:
./autogen.sh
env LIBS="-static -lX11 -lxcb -lXau -lXdmcp -lXmuu -lpthread" ./configure ; make
make V=1

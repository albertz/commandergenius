To build system image, download repo:

https://github.com/pelya/cuntubuntu

install dependencies described in it's readme,
then launch commands:

cd cuntubuntu/img
./img-gimp-jessie-armhf.sh
./img-gimp-jessie-x86.sh
./img-gimp-jessie--prepare-obb.sh

This will create Debian system image dist-gimp-jessie.tar.xz
Upload resulting system image somewhere, and change download URL inside
AndroidAppSettings.cfg, then recompile .apk file.

Then follow instructions in project/jni/application/xserver/readme.txt

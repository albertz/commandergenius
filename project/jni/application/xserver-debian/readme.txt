To build system image, download repo:

https://github.com/pelya/cuntubuntu

Install dependencies described in it's readme,
then launch commands:

cd cuntubuntu/img
./img-debian-jessie-armhf.sh
./img-debian-jessie-x86.sh
./img-debian-jessie--prepare-obb.sh

This will create Debian system image dist-debian-jessie.tar.xz
Upload resulting system image somewhere, and change download URL inside
AndroidAppSettings.cfg, then recompile .apk file.

Then follow instructions in project/jni/application/xserver/readme.txt

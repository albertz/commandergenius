#!/bin/sh

for f in ../UltimateDroid/*.png; do
	newname=`echo $f | sed 's@.*/@@' | tr '[A-Z]' '[a-z]'`.raw
	./converter $f ../../res/raw/$newname
done

rm -f ../../res/raw/ultimatedroid.raw

for F in \
dpadbutton \
leftbuttonpressed \
rightbuttonpressed \
upbuttonpressed \
downbuttonpressed \
\
button1auto \
button1autoanim \
button2auto \
button2autoanim \
\
button1 \
button1pressed \
button2 \
button2pressed \
button3 \
button3pressed \
button4 \
button4pressed \
button5 \
button5pressed \
button6 \
button6pressed \
button7 \
button7 \
; do
	if [ \! -e ../../res/raw/ultimatedroid$F.png.raw ]; then
		echo Cannot find ../../res/raw/ultimatedroid$F.png.raw - check if all files are in place
		exit 1
	fi
	cat ../../res/raw/ultimatedroid$F.png.raw >> ../../res/raw/ultimatedroid.raw
done

rm ../../res/raw/ultimatedroid*.png.raw

gzip -9 < ../../res/raw/ultimatedroid.raw > ../../res/raw/ultimatedroid.raw.gz
mv -f ../../res/raw/ultimatedroid.raw.gz ../../res/raw/ultimatedroid.raw
echo "// Touchscreen theme to be included directly into the code witohut Java resources, not used yet"> ../touchscreentheme.h
echo "unsigned char * ultimateDroidTheme[] = { 00, // Dummy byte, skip it" >> ../touchscreentheme.h
cat ../../res/raw/ultimatedroid.raw | od -t x1 -v -A n | tr " " "," >> ../touchscreentheme.h
echo "};" >> ../touchscreentheme.h

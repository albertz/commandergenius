#!/bin/sh

# Ultimate Droid by Sean Stieber

for f in ../UltimateDroid/*.png; do
	newname=`echo $f | sed 's@.*/@@' | tr '[A-Z]' '[a-z]'`.raw
	./converter $f ../../res/raw/$newname 16
done

rm -f ../../res/raw/ultimatedroid.raw
printf '\000\000\000\030' > ../../res/raw/ultimatedroid.raw # size, 030 = 24

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
mouse_pointer \
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
echo "unsigned char * UltimateDroidTheme[] = { 00 // Dummy byte, skip it" >> ../touchscreentheme.h
cat ../../res/raw/ultimatedroid.raw | od -t x1 -v -A n | tr " " "," >> ../touchscreentheme.h
echo "};" >> ../touchscreentheme.h


# Simple Theme by Dmitry Matveev


for f in ../SimpleTheme/*.png; do
	newname=simpletheme`echo $f | sed 's@.*/@@' | tr '[A-Z]' '[a-z]'`.raw
	./converter $f ../../res/raw/$newname 16
done

rm -f ../../res/raw/simpletheme.raw
printf '\000\000\000\030' > ../../res/raw/simpletheme.raw # size, 030 = 24

for F in \
dpad \
dpad_left \
dpad_right \
dpad_up \
dpad_down \
\
1auto_pressed \
1auto \
2auto_pressed \
2auto \
\
1 \
1pressed \
2 \
2pressed \
3 \
3pressed \
4 \
4pressed \
5 \
5pressed \
6 \
6pressed \
keyboard \
keyboard \
mouse_pointer \
; do
	if [ \! -e ../../res/raw/simpletheme$F.png.raw ]; then
		echo Cannot find ../../res/raw/simpletheme$F.png.raw - check if all files are in place
		exit 1
	fi
	cat ../../res/raw/simpletheme$F.png.raw >> ../../res/raw/simpletheme.raw
done

rm ../../res/raw/simpletheme*.png.raw

gzip -9 < ../../res/raw/simpletheme.raw > ../../res/raw/simpletheme.raw.gz
mv -f ../../res/raw/simpletheme.raw.gz ../../res/raw/simpletheme.raw
#echo "// Touchscreen theme to be included directly into the code witohut Java resources, not used yet"> ../touchscreentheme.h
echo "unsigned char * SimpleTheme[] = { 00 // Dummy byte, skip it" >> ../touchscreentheme.h
cat ../../res/raw/simpletheme.raw | od -t x1 -v -A n | tr " " "," >> ../touchscreentheme.h
echo "};" >> ../touchscreentheme.h


# Abstract Sun Icon Set by Sirea (Martina Šmejkalová)
for f in ../Sun/*.png; do
	newname=`echo $f | sed 's@.*/@@' | tr '[A-Z]' '[a-z]'`.raw
	./converter $f ../../res/raw/$newname 32
done

rm -f ../../res/raw/sun.raw
printf '\000\000\000\012' > ../../res/raw/sun.raw # size, 012 = 10

for F in \
sun-2.ico-10 \
\
sun-4.ico-10 \
sun-5.ico-10 \
sun-6.ico-10 \
sun-7.ico-10 \
\
sun-3.ico-10 \
sun-9.ico-10 \
sun-8.ico-10 \
sun-1.ico-10 \
\
sun-mouse_pointer \
; do
	if [ \! -e ../../res/raw/$F.png.raw ]; then
		echo Cannot find ../../res/raw/$F.png.raw - check if all files are in place
		exit 1
	fi
	
	cat ../../res/raw/$F.png.raw >> ../../res/raw/sun.raw
done

rm ../../res/raw/sun*.png.raw

gzip -9 < ../../res/raw/sun.raw > ../../res/raw/sun.raw.gz
mv -f ../../res/raw/sun.raw.gz ../../res/raw/sun.raw
echo "unsigned char * SunTheme[] = { 00 // Dummy byte, skip it" >> ../touchscreentheme.h
cat ../../res/raw/sun.raw | od -t x1 -v -A n | tr " " "," >> ../touchscreentheme.h
echo "};" >> ../touchscreentheme.h

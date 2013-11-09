#!/bin/sh

echo "Copying app data files from project/jni/application/src/AndroidData to project/assets"
mkdir -p project/assets
rm -f project/assets/*
if [ -d "project/jni/application/src/AndroidData" ] ; then
	cp -L project/jni/application/src/AndroidData/* project/assets/
	exit 0
	for F in project/assets/*; do
		if [ `cat $F | wc -c` -gt 1000000 ] ; then
			echo "The file $F is bigger than 1 megabyte - splitting it into smaller chunks"
			split -b 1000000 -a 3 -d $F $F && rm $F || { echo "Error: 'split' command not installed" ; exit 1 ; }
		fi
	done
fi
exit 0

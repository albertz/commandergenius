#!/bin/sh
diff -u -r openttd-*-orig openttd-*-armeabi-v7a | filterdiff --strip=1 > openttd-trunk-android.patch

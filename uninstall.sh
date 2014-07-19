#!/bin/sh
adb uninstall `grep AppFullName AndroidAppSettings.cfg | sed 's/.*=//'`

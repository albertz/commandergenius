
# The namespace in Java file, with dots replaced with underscores
SDL_JAVA_PACKAGE_PATH := de_schwardtnet_alienblaster

# Path to files with application data - they should be downloaded from Internet on first app run inside
# Java sources, or unpacked from resources (TODO)
# Typically /sdcard/alienblaster 
# Or /data/data/de.schwardtnet.alienblaster/files if you're planning to unpack data in application private folder
# Your application will just set current directory there
SDL_CURDIR_PATH := /sdcard/alienblaster

include $(call all-subdir-makefiles)

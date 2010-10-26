To compile ScummVM add the ScummVM SVN to directory "scummvm" (or make a symlink to it),
apply the patch 'scummvm-trunk-53822.diff' to the SVN,
then from dir 'project/jni/application' execute commands

rm src
ln -s scummvm src

Then go to project root and launch 'build.sh' - that should be it.
You need to have Android SDK 2.2 and NDK r4b installed in your PATH.

To compile ScummVM add the ScummVM SVN branch 1.2.0 to directory "scummvm" (or make a symlink to it),
apply patch scummvm-1.2.0.diff, then from dir 'project/jni/application' execute commands

rm src
ln -s scummvm src

Then go to project root and launch 'build.sh' - that should be it.
You need to have Android SDK 2.2 and NDK r4b installed in your PATH.

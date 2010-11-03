To compile ScummVM add the ScummVM SVN to directory "scummvm" (or make a symlink to it),
then overwrite "configure" script with file "configure-androidsdl",
then from dir 'project/jni/application' execute commands

rm src
ln -s scummvm src

Then go to project root and launch 'build.sh' - that should be it.
You need to have Android SDK 2.2 and NDK r4b installed in your PATH.

If you're using ScummVM trunk you'll probably need to apply a patch 
'scummvm-trunk-53822.diff' to the SVN, branch 1.2.0 compiles without changes.

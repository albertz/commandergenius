To summarize: you have to download my GIT repo from https://github.com/pelya/commandergenius,
then install Android SDK 2.2, NDK r4b, and "ant" tool, then you should launch commands
    rm project/jni/application/src
    ln -s scummvm project/jni/application/src
    cd project && android update project -p .
then download ScummVM 1.2 SVN into the dir project/jni/application/scummvm/scummvm
(or create a symlink to it if you already have downloaded it), then apply patch project/jni/application/scummvm/scummvm-1.2.0.diff
onto ScummVM, and launch build.sh. That should be it.
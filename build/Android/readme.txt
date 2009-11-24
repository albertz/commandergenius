This should be compiled with Android 1.6 SDK and NDK - google for them and install them as described in their docs.
You'll need to install Eclipse too (or Ant, though I did not test it with Ant)
Then symlink this dir to <android-ndk>/apps under the name "commandergenius":
	ln -s `pwd` <android-ndk>/apps
Then make symling of ../../src dir into project/jni/commandergenius - this should be full path:
	ln -s `pwd`/../../src project/jni/commandergenius/
Then go to <android-ndk> dir and execute:
	make APP=commandergenius V=1
Hopefully it will compile the file project/libs/armeabi/libcommandergenius.so

Then we'll have to compile Android .apk package with Java wrapper code for this lib (TODO)



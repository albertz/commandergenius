LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := commandergenius

CG_SUBDIRS := \
src \
src/ai \
src/cinematics \
src/dialog \
src/include \
src/include/vorbis \
src/include/gui \
src/include/fileio \
src/vorbis \
src/vorticon \
src/hqp \
src/sdl \
src/sdl/sound \
src/sdl/video \
src/scale2x \
src/fileio \


LOCAL_CFLAGS := $(foreach D, $(CG_SUBDIRS), -I$(CG_SRCDIR)/$(D)) \
				-I$(LOCAL_PATH)/../sdl/include \
				-I$(LOCAL_PATH)/../stlport/stlport

LOCAL_CPP_EXTENSION := .cpp

$(CG_SRCDIR):
	$(error Please create symlink of "<CommanderGenius>/src" dir into "<CommanderGenius>/build/Android/project/jni/commandergenius", with full path)
	
LOCAL_SRC_FILES := $(foreach F, $(CG_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))

LOCAL_STATIC_LIBRARIES := sdl stlport

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog

# Print an error to user
out/apps/commandergenius//libcommandergenius.so: $(CG_SRCDIR)
out/apps/commandergenius/libcommandergenius.so: $(CG_SRCDIR)

include $(BUILD_SHARED_LIBRARY)


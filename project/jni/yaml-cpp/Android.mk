LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := yaml-cpp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/src

LOCAL_SRC_FILES := \
    src/aliascontent.cpp \
    src/conversion.cpp \
    src/emitter.cpp \
    src/emitterstate.cpp \
    src/emitterutils.cpp \
    src/exp.cpp \
    src/iterator.cpp \
    src/map.cpp \
    src/node.cpp \
    src/null.cpp \
    src/ostream.cpp \
    src/parser.cpp \
    src/parserstate.cpp \
    src/regex.cpp \
    src/scalar.cpp \
    src/scanner.cpp \
    src/scanscalar.cpp \
    src/scantag.cpp \
    src/scantoken.cpp \
    src/sequence.cpp \
    src/simplekey.cpp \
    src/stream.cpp \
    src/tag.cpp

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_LDLIBS :=

include $(BUILD_STATIC_LIBRARY)


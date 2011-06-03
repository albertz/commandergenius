LOCAL_PATH := $(call my-dir)
BOOST_MODULE := date_time
include $(LOCAL_PATH)/boost.mk
BOOST_MODULE := filesystem
include $(LOCAL_PATH)/boost.mk
BOOST_MODULE := iostreams
include $(LOCAL_PATH)/boost.mk
BOOST_MODULE := program_options
include $(LOCAL_PATH)/boost.mk
BOOST_MODULE := regex
include $(LOCAL_PATH)/boost.mk
BOOST_MODULE := signals
include $(LOCAL_PATH)/boost.mk
BOOST_MODULE := system
include $(LOCAL_PATH)/boost.mk
BOOST_MODULE := thread
include $(LOCAL_PATH)/boost.mk

LOCAL_PATH := $(call my-dir)
FFMPEG_MODULE := avcodec
include $(LOCAL_PATH)/ffmpeg.mk
FFMPEG_MODULE := avcore
include $(LOCAL_PATH)/ffmpeg.mk
FFMPEG_MODULE := avdevice
include $(LOCAL_PATH)/ffmpeg.mk
FFMPEG_MODULE := avfilter
include $(LOCAL_PATH)/ffmpeg.mk
FFMPEG_MODULE := avformat
include $(LOCAL_PATH)/ffmpeg.mk
FFMPEG_MODULE := avutil
include $(LOCAL_PATH)/ffmpeg.mk
FFMPEG_MODULE := swscale
include $(LOCAL_PATH)/ffmpeg.mk

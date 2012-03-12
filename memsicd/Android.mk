LOCAL_PATH:= $(call my-dir)

#
# memsicd - OuNao mod
# based on eCompass code by Mark Pedley / Freescale
#

include $(CLEAR_VARS)
LOCAL_SRC_FILES := memsicd.c

LOCAL_MODULE := memsicd

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := libcutils

include $(BUILD_EXECUTABLE)

LOCAL_PATH := $(call my-dir)

## Make camera wrapper

include $(CLEAR_VARS)

LOCAL_C_FLAGS        += -O3
LOCAL_MODULE_TAGS    := optional
LOCAL_MODULE_PATH    := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE         := camera.$(TARGET_BOOTLOADER_BOARD_NAME)
LOCAL_SRC_FILES      := cameraHal.cpp
LOCAL_PRELINK_MODULE := false

LOCAL_SHARED_LIBRARIES := liblog libdl libutils libcamera_client libbinder libcutils libhardware libcamera libui

LOCAL_C_INCLUDES       := frameworks/base/services \
                          frameworks/base/include \
                          hardware/libhardware/include \
                          hardware

include $(BUILD_SHARED_LIBRARY)

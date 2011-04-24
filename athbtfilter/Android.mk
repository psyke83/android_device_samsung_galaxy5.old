ifeq ($(WIFI_DRIVER_MODULE_NAME),ar6000)

LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional

# From autoconf-generated Makefile
abtfilt_SOURCES = abtfilt_bluez_dbus.c \
		 abtfilt_core.c \
	  	 abtfilt_main.c \
     		 abtfilt_utils.c \
	 	 abtfilt_wlan.c \
		 btfilter_action.c \
		 btfilter_core.c 

LOCAL_SRC_FILES:= $(abtfilt_SOURCES)

LOCAL_SHARED_LIBRARIES := \
	 	libdbus \
		libbluetooth

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	external/bluetooth/bluez/include/bluetooth \
	external/bluetooth/bluez/lib/bluetooth \
	$(call include-path-for, dbus) \
	$(call include-path-for, bluez-libs)

LOCAL_CFLAGS+= \
	-DDBUS_COMPILATION -DDISABLE_MASTER_MODE -DABF_DEBUG


LOCAL_MODULE := abtfilt

#LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)

include $(BUILD_EXECUTABLE)

endif #ar6000

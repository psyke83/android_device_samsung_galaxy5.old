# Copyright (C) 2011 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# This file sets variables that control the way modules are built
# throughout the system. It should not be used to conditionally
# disable makefiles (the proper mechanism to control what gets
# included in a build is to use PRODUCT_PACKAGES in a product
# definition file).
#

# WARNING: This line must come *before* including the proprietary
# variant, so that it gets overwritten by the parent (which goes
# against the traditional rules of inheritance).
USE_CAMERA_STUB := true

# inherit from the proprietary version
-include vendor/samsung/galaxy5/BoardConfigVendor.mk

TARGET_SPECIFIC_HEADER_PATH := device/samsung/galaxy5/include

TARGET_BOARD_PLATFORM := msm7k
TARGET_ARCH_VARIANT := armv6-vfp
TARGET_CPU_ABI := armeabi-v6l
TARGET_CPU_ABI2 := armeabi

# Target properties
TARGET_BOOTLOADER_BOARD_NAME := galaxy5
TARGET_OTA_ASSERT_DEVICE := galaxy5,GT-I5500,GT-I5500M,GT-I5500L

# Target information
TARGET_NO_BOOTLOADER := true
TARGET_NO_KERNEL := false
TARGET_NO_RADIOIMAGE := true
TARGET_NO_RECOVERY := false

# Bluetooth
BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_BCM := true

# QCOM
#BOARD_USES_QCOM_HARDWARE := true
BOARD_USES_QCOM_LIBRPC := true
BOARD_USES_QCOM_LIBS := true

# GPS
BOARD_USES_QCOM_GPS := true
BOARD_VENDOR_QCOM_GPS_LOC_API_AMSS_VERSION := 50000
BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE := galaxy5

# Browser
JS_ENGINE := v8
HTTP := chrome
WITH_JIT := true
TARGET_WEBKIT_USE_MORE_MEMORY := true

# USB mass storage
BOARD_UMS_LUNFILE := /sys/devices/platform/msm_hsusb/gadget/lun0/file
TARGET_USE_CUSTOM_LUN_FILE_PATH := "/sys/devices/platform/msm_hsusb/gadget/lun0/file"

# ICS Stuff 
BOARD_USE_LEGACY_TOUCHSCREEN := true
BOARD_USE_LEGACY_TRACKPAD := true

# Audio
BOARD_PROVIDES_LIBAUDIO := true

# RIL
BOARD_PROVIDES_LIBRIL := true

# Graphics
#BOARD_ADRENO_DECIDE_TEXTURE_TARGET := true # may be useful, but seems to require QCOM_HARDWARE
BOARD_AVOID_DRAW_TEXTURE_EXTENSION := true # not sure
BOARD_CACHE_BOOTANIM := true
BOARD_EGL_CFG := device/samsung/galaxy5/prebuilt/lib/egl/egl.cfg
BOARD_USE_FROYO_LIBCAMERA := true
BOARD_USE_NASTY_PTHREAD_CREATE_HACK := true # not sure - maybe for proprietary libs
BOARD_USE_SKIA_LCDTEXT := true # not sure
COMMON_GLOBAL_CFLAGS += -DBINDER_COMPAT
COMMON_GLOBAL_CFLAGS += -DMISSING_EGL_EXTERNAL_IMAGE 
COMMON_GLOBAL_CFLAGS += -DMISSING_EGL_PIXEL_FORMAT_YV12
COMMON_GLOBAL_CFLAGS += -DMISSING_GRALLOC_BUFFERS
COMMON_GLOBAL_CFLAGS += -DTARGET_MSM7x27
COMMON_GLOBAL_CFLAGS += -DUNABLE_TO_DEQUEUE
TARGET_BOARD_PLATFORM_GPU := qcom-adreno200 # not used anymore?
TARGET_FORCE_CPU_UPLOAD := true
TARGET_LIBAGL_USE_GRALLOC_COPYBITS := true # needed
TARGET_NO_INITLOGO := true


# WiFI
BOARD_WPA_SUPPLICANT_DRIVER := WEXT
BOARD_WEXT_NO_COMBO_SCAN    := true
WPA_SUPPLICANT_VERSION := VER_0_6_X
#BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_ath6kl
WIFI_BAND  := 802_11_ABG
BOARD_HOSTAPD_DRIVER := WEXT
#BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_ath6kl
BOARD_WLAN_DEVICE := ath6kl

WIFI_DRIVER_MODULE_PATH     := "/system/wifi/ar6000.ko"
WIFI_DRIVER_MODULE_NAME     :=  "ar6000"
WIFI_DRIVER_MODULE_ARG      :=  ""

# Recovery
BOARD_CUSTOM_RECOVERY_KEYMAPPING := ../../device/samsung/galaxy5/recovery/recovery_keys.c
BOARD_USE_CUSTOM_RECOVERY_FONT := "<font_7x16.h>"

# Kernel
TARGET_KERNEL_SOURCE := kernel/samsung/codeaurora_gio2europa
TARGET_KERNEL_CONFIG := europa_rev02_cm9_defconfig
#TARGET_PREBUILT_KERNEL := device/samsung/galaxy5/prebuilt/kernel
BOARD_NAND_PAGE_SIZE := 4096
BOARD_NAND_SPARE_SIZE := 128
BOARD_KERNEL_BASE := 0x00200000
BOARD_KERNEL_CMDLINE := 
BOARD_KERNEL_PAGESIZE := 4096

# cat /proc/LinuStoreIII/bmlinfo
# FSR VERSION: FSR_1.2.1p1_b139_RTM
# minor       position           size     units       id
#    1: 0x00000000-0x00180000 0x00180000      6        1
#    2: 0x00180000-0x00200000 0x00080000      2        2
#    3: 0x00200000-0x002c0000 0x000c0000      3        3
#    4: 0x002c0000-0x01bc0000 0x01900000    100        4
#    5: 0x01bc0000-0x03400000 0x01840000     97       23
#    6: 0x03400000-0x03900000 0x00500000     20       25
#    7: 0x03900000-0x05200000 0x01900000    100        5
#    8: 0x05200000-0x05400000 0x00200000      8        6 
#    9: 0x05400000-0x05e00000 0x00a00000     40        7
#   10: 0x05e00000-0x06800000 0x00a00000     40        8
#   11: 0x06800000-0x068c0000 0x000c0000      3        9
#   12: 0x068c0000-0x11a40000 0x0b180000    710       21
#   13: 0x11a40000-0x1dc00000 0x0c1c0000    775       22
#   14: 0x1dc00000-0x1f500000 0x01900000    100       24

BOARD_BOOTIMAGE_PARTITION_SIZE := 10485760
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 10485760
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 186122240
BOARD_USERDATAIMAGE_PARTITION_SIZE := 203161600

# (BOARD_KERNEL_PAGESIZE * 64) ???
BOARD_FLASH_BLOCK_SIZE := 4096

# FM Radio
BOARD_HAVE_FM_RADIO := true
BOARD_GLOBAL_CFLAGS += -DHAVE_FM_RADIO
BOARD_FM_DEVICE := bcm2049
#BOARD_USE_BROADCOM_FM_VOLUME_HACK := true

# 3G
BOARD_MOBILEDATA_INTERFACE_NAME := "pdp0"

# NEEDS CHECKING (RECOVERY)
BOARD_BML_BOOT := /dev/block/bml9
BOARD_BML_RECOVERY := /dev/block/bml10
#BOARD_RECOVERY_HANDLES_MOUNT := true
#BOARD_HAS_DOWNLOAD_MODE := true
#BOARD_LDPI_RECOVERY := true
#TARGET_RECOVERY_INITRC := device/samsung/galaxy5/recovery/recovery.rc
#BOARD_CUSTOM_GRAPHICS := ../../../device/samsung/galaxy5/recovery/graphics.c
#BOARD_CUSTOM_RECOVERY_KEYMAPPING:= ../../device/samsung/galaxy5/recovery/recovery_ui.c
#TARGET_RECOVERY_PRE_COMMAND := "echo 3 > /proc/sys/vm/drop_caches; echo 604800 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq; sync"
TARGET_USERIMAGES_USE_EXT4 := true

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

DEVICE_PACKAGE_OVERLAYS += device/samsung/galaxy5/overlay

PRODUCT_PACKAGES += \
    brcm_patchram_plus \
    copybit.galaxy5 \
    gps.galaxy5 \
    gralloc.galaxy5 \
    libOmxCore \
    lights.msm7k \
    rzscontrol \
    screencap \
    setup_fs

# Recovery tools
PRODUCT_PACKAGES += \
    dump_image \
    e2fsck \
    erase_image \
    flash_image \
    make_ext4fs

# Service Mode Secret Code
PRODUCT_PACKAGES += \
    SamsungServiceMode

# MADTeam modifications
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/bin/madconfig:system/bin/madconfig \
    device/samsung/galaxy5/prebuilt/etc/init.d/02madteam:system/etc/init.d/02madteam \
    device/samsung/galaxy5/prebuilt/etc/init.d/05mountsd:system/etc/init.d/05mountsd \
    device/samsung/galaxy5/prebuilt/modules/synaptics_i2c_rmi4.ko:system/lib/modules/synaptics_i2c_rmi4.ko \
    frameworks/base/data/etc/android.hardware.touchscreen.xml:system/etc/permissions/touchscreen/android.hardware.touchscreen.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/touchscreen/android.hardware.touchscreen.multitouch.distinct.xml

# Hardware properties
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/base/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.xml:system/etc/permissions/android.hardware.touchscreen.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/base/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml

# Kernel modules
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/modules/acc_cal_param.ko:root/lib/modules/acc_cal_param.ko \
    device/samsung/galaxy5/prebuilt/modules/fsr.ko:root/lib/modules/fsr.ko \
    device/samsung/galaxy5/prebuilt/modules/fsr_stl.ko:root/lib/modules/fsr_stl.ko \
    device/samsung/galaxy5/prebuilt/modules/ramzswap.ko:system/lib/modules/ramzswap.ko \
    device/samsung/galaxy5/prebuilt/modules/rfs_glue.ko:root/lib/modules/rfs_glue.ko \
    device/samsung/galaxy5/prebuilt/modules/rfs_fat.ko:root/lib/modules/rfs_fat.ko \
    device/samsung/galaxy5/prebuilt/modules/sec_param.ko:root/lib/modules/sec_param.ko

# Board-specific init & scripts
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/init.gt-i5500.rc:root/init.gt-i5500.rc \
    device/samsung/galaxy5/prebuilt/ueventd.gt-i5500.rc:root/ueventd.gt-i5500.rc

# Workaround for wifi & bluetooth mac addresses
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/bin/get_macaddrs:system/bin/get_macaddrs

# Bluetooth
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/etc/bluetooth/audio.conf:system/etc/bluetooth/audio.conf

# Wifi
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/etc/dhcpd/dhcpcd.conf:system/etc/dhcpcd/dhcpcd.conf \
    device/samsung/galaxy5/prebuilt/etc/wifi/hostapd.conf:system/etc/wifi/hostapd.conf \
    device/samsung/galaxy5/prebuilt/etc/wifi/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf \
    device/samsung/galaxy5/prebuilt/modules/ar6000.ko:system/wifi/ar6000.ko

# VOLD configuration
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/etc/vold.fstab:system/etc/vold.fstab

# Media profiles
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/etc/media_profiles.xml:/system/etc/media_profiles.xml

# Audio
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/etc/AudioFilter.csv:system/etc/AudioFilter.csv \
    device/samsung/galaxy5/prebuilt/etc/AutoVolumeControl.txt:system/etc/AutoVolumeControl.txt

# Enable Google-specific location features,
# like NetworkLocationProvider and LocationCollector
PRODUCT_PROPERTY_OVERRIDES += \
     ro.com.google.locationfeatures=1 \
     ro.com.google.networklocation=1

# Extended JNI checks
# The extended JNI checks will cause the system to run more slowly, but they can spot a variety of nasty bugs 
# before they have a chance to cause problems.
# Default=true for development builds, set by android buildsystem.
PRODUCT_PROPERTY_OVERRIDES += \
    ro.kernel.android.checkjni=0 \
    dalvik.vm.checkjni=false

# RIL properties
PRODUCT_PROPERTY_OVERRIDES += \
    rild.libargs=-d/dev/smd0 \
    rild.libpath=/system/lib/libsec-ril.so \
    ro.telephony.ril_class=samsung \
    ro.telephony.sends_barcount=1

# Keymap properties
PRODUCT_PROPERTY_OVERRIDES += \
    android.keychar.europa_keypad0=/system/usr/keychars/europa_keypad0.kcm.bin \
    android.keylayout.europa_headset=/system/usr/keylayout/europa_headset.kl \
    android.keylayout.europa_keypad0=/system/usr/keylayout/europa_keypad0.kl \
    hw.keyboards.0.devname=europa_keypad0 \
    hw.keyboards.65538.devname=europa_headset

# Networking properties
PRODUCT_PROPERTY_OVERRIDES += \
    mobiledata.interfaces=pdp0,wlan0,gprs,ppp0 \
    wifi.interface=wlan0 \
    wifi.supplicant_scan_interval=60

# Performance & graphics properties
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.heapsize=24m \
    persist.sys.purgeable_assets=1 \
    persist.sys.use_dithering=1 \
    ro.media.dec.jpeg.memcap=20000000 \
    ro.opengles.version=131072 \
    ro.sf.lcd_density=120

# Compcache properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.compcache.default=10

ifeq ($(TARGET_PREBUILT_KERNEL),)
    LOCAL_KERNEL := device/samsung/galaxy5/prebuilt/kernel
else
    LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

PRODUCT_COPY_FILES += \
    $(LOCAL_KERNEL):kernel

$(call inherit-product-if-exists, vendor/samsung/galaxy5/galaxy5-vendor.mk)

$(call inherit-product, build/target/product/full_base.mk)

# The gps config appropriate for this device
$(call inherit-product, device/common/gps/gps_eu_supl.mk)

# LDPI assets
PRODUCT_LOCALES += ldpi mdpi

PRODUCT_NAME := samsung_galaxy5
PRODUCT_DEVICE := galaxy5
PRODUCT_MODEL := Galaxy5
PRODUCT_BRAND := samsung
PRODUCT_MANUFACTURER := samsung

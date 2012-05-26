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

DEVICE_PACKAGE_OVERLAYS := $(LOCAL_PATH)/overlay

# Discard inherited values and use our own instead.
PRODUCT_NAME := samsung_galaxy5
PRODUCT_DEVICE := galaxy5
PRODUCT_MODEL := Samsung Galaxy5

PRODUCT_AAPT_CONFIG := mdpi hdpi
PRODUCT_AAPT_PREF_CONFIG := mdpi
PRODUCT_LOCALES += mdpi

# Graphics 
PRODUCT_PACKAGES += \
    copybit.galaxy5 \
    gralloc.galaxy5

# Audio
PRODUCT_PACKAGES += \
    audio.a2dp.default \
    audio_policy.galaxy5 \
    audio.primary.galaxy5 \

# Zram
PRODUCT_PACKAGES += \
    hwprops \
    rzscontrol

# Video decoding
PRODUCT_PACKAGES += \
    libstagefrighthw \
    libopencorehw \
    libmm-omxcore \
    libOmxCore

# Other
PRODUCT_PACKAGES += \
    camera.galaxy5 \
    FileManager \
    g5sensord \
    gps.galaxy5 \
    lights.galaxy5 \
    Music \
    sensors.galaxy5 \
    setup_fs

# Hardware permissions
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.location.xml:system/etc/permissions/android.hardware.location.xml \
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
    device/samsung/galaxy5/prebuilt/modules/rfs_glue.ko:root/lib/modules/rfs_glue.ko \
    device/samsung/galaxy5/prebuilt/modules/rfs_fat.ko:root/lib/modules/rfs_fat.ko \
    device/samsung/galaxy5/prebuilt/modules/sec_param.ko:root/lib/modules/sec_param.ko

# Vold and USB
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/etc/vold.fstab:system/etc/vold.fstab \
    device/samsung/galaxy5/prebuilt/etc/start_usb0.sh:system/etc/start_usb0.sh \
    device/samsung/galaxy5/prebuilt/etc/media_profiles.xml:system/etc/media_profiles.xml

# Init files
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/init.gt-i5500board.rc:root/init.gt-i5500board.rc \
    device/samsung/galaxy5/prebuilt/init.gt-i5500board.usb.rc:root/init.gt-i5500board.usb.rc \
    device/samsung/galaxy5/prebuilt/initlogo.rle:root/initlogo.rle \
    device/samsung/galaxy5/prebuilt/ueventd.gt-i5500board.rc:root/ueventd.gt-i5500board.rc

# Wi-Fi related
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/bin/get_macaddrs:system/bin/get_macaddrs \
    device/samsung/galaxy5/prebuilt/etc/dhcpcd/dhcpcd.conf:system/etc/dhcpcd/dhcpcd.conf \
    device/samsung/galaxy5/prebuilt/etc/wifi/hostapd.conf:system/etc/wifi/hostapd.conf \
    device/samsung/galaxy5/prebuilt/etc/wifi/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf \
    device/samsung/galaxy5/prebuilt/modules/ar6000.ko:system/wifi/ar6000.ko

# Audio
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/etc/AudioFilter.csv:system/etc/AudioFilter.csv \
    device/samsung/galaxy5/prebuilt/etc/AutoVolumeControl.txt:system/etc/AutoVolumeControl.txt

# Bluetooth
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/etc/bluetooth/audio.conf:system/etc/bluetooth/audio.conf

# Touchscreen & keys
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/usr/idc/synaptics-rmi-touchscreen.idc:system/usr/idc/synaptics-rmi-touchscreen.idc \
    device/samsung/galaxy5/prebuilt/usr/keylayout/synaptics.kl:system/usr/keylayout/synaptics.kl

# Other
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/etc/sysctl.conf:system/etc/sysctl.conf

# MADTEAM & ICS HACKS
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/bin/madconfig:system/bin/madconfig \
    device/samsung/galaxy5/prebuilt/etc/init.d/02madteam:system/etc/init.d/02madteam \
    frameworks/base/data/etc/android.hardware.touchscreen.xml:system/madteam/etc/permissions/android.hardware.touchscreen.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/madteam/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml

# Ensure that /cache is never used for deodexing
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.dexopt-data-only=1

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
    ro.telephony.ril_class=SamsungRIL \
    ro.telephony.sends_barcount=1 \
    ro.telephony.default_network=0 \
    ro.telephony.call_ring.delay=0 \
    ro.telephony.call_ring.multiple=false

# Networking properties
PRODUCT_PROPERTY_OVERRIDES += \
    mobiledata.interfaces=pdp0,wlan0,gprs,ppp0 \
    wifi.interface=wlan0 \
    wifi.supplicant_scan_interval=45

# Performance & graphics properties
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.heapsize=24m \
    persist.sys.purgeable_assets=1 \
    persist.sys.use_dithering=1 \
    ro.media.dec.jpeg.memcap=20000000 \
    ro.opengles.version=131072 \
    ro.sf.lcd_density=120 \
    ro.vold.umsdirtyratio=20

# Touchscreen properties
PRODUCT_PROPERTY_OVERRIDES += \
    view.fading_edge_length=8 \
    view.minimum_fling_velocity=25 \
    view.scroll_friction=0.008 \
    view.touch_slop=15

# Compcache/ZRAM properties
PRODUCT_PROPERTY_OVERRIDES += \
    ro.zram.default=18

# Audio attenuation
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.extamp-filter=1 \
    persist.sys.fm-attn=0 \
    persist.sys.headset-attn=0 \
    persist.sys.speaker-attn=0

# NEW ICS properties (may need verification/testing)
PRODUCT_PROPERTY_OVERRIDES += \
    ro.config_datause_iface=pdp0 \
    ro.secure=0 \
    ro.ril.hsxpa=1 \
    ro.ril.gprsclass=10 \
    ro.telephony.ril.v3=icccardstatus,datacall,signalstrength,facilitylock \
    mobiledata.interfaces=pdp0,rmnet0,rmnet1,rmnet2 \
    net.qtaguid_enabled=0 \
    debug.gr.swapinterval=0 \
    persist.sys.usb.config=mass_storage,adb \
    sys.usb.config=mass_storage,adb \
    persist.service.adb.enable=1


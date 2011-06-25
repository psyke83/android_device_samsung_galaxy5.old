#$(call inherit-product, build/target/product/full.mk)

DEVICE_PACKAGE_OVERLAYS += device/samsung/galaxy5/overlay

ifeq ($(TARGET_PREBUILT_KERNEL),)
	LOCAL_KERNEL := device/samsung/galaxy5/kernel
else
	LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

PRODUCT_COPY_FILES += \
    $(LOCAL_KERNEL):kernel

PRODUCT_PACKAGES += \
    librs_jni \
    libOmxCore \
    libOmxVidEnc \
    libmm-omxcore \
    brcm_patchram_plus \
    gps.galaxy5 \
    gralloc.galaxy5 \
    copybit.galaxy5 \
    lights.msm7k \
    screencap

# Live wallpaper packages
PRODUCT_PACKAGES += \
    LiveWallpapers \
    LiveWallpapersPicker \
    MagicSmokeWallpapers \
    VisualizationWallpapers \

# Publish that we support the live wallpaper feature.
PRODUCT_COPY_FILES += \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:/system/etc/permissions/android.software.live_wallpaper.xml

# Device-specific keymaps
PRODUCT_COPY_FILES += \
    vendor/samsung/galaxy5/proprietary/usr/keylayout/AVRCP.kl:system/usr/keylayout/AVRCP.kl \
    vendor/samsung/galaxy5/proprietary/usr/keylayout/europa_headset.kl:system/usr/keylayout/europa_headset.kl \
    vendor/samsung/galaxy5/proprietary/usr/keylayout/europa_keypad0.kl:system/usr/keylayout/europa_keypad0.kl \
    vendor/samsung/galaxy5/proprietary/usr/keylayout/qwerty.kl:system/usr/keylayout/qwerty.kl \
    vendor/samsung/galaxy5/proprietary/usr/keychars/europa_keypad0.kcm.bin:system/usr/keychars/europa_keypad0.kcm.bin \
    vendor/samsung/galaxy5/proprietary/usr/keychars/qwerty.kcm.bin:system/usr/keychars/qwerty.kcm.bin \
    vendor/samsung/galaxy5/proprietary/usr/keychars/qwerty2.kcm.bin:system/usr/keychars/qwerty2.kcm.bin

# Board-specific init
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/files/ueventd.gt-i5500.rc:root/ueventd.gt-i5500.rc \
    device/samsung/galaxy5/files/init.gt-i5500.rc:root/init.gt-i5500.rc \
    device/samsung/galaxy5/files/etc/sysctl.conf:system/etc/sysctl.conf \
    device/samsung/galaxy5/files/bin/get_macaddrs:system/bin/get_macaddrs \
    device/samsung/galaxy5/files/bin/mad2sd:sbin/mad2sd \
    device/samsung/galaxy5/files/etc/init.d/02galaxy5:system/etc/init.d/02galaxy5

# Sensors
PRODUCT_COPY_FILES += \
    vendor/samsung/galaxy5/proprietary/lib/hw/sensors.default.so:system/lib/hw/sensors.default.so \
    vendor/samsung/galaxy5/proprietary/bin/memsicd:system/bin/memsicd

# 3D
PRODUCT_COPY_FILES += \
    vendor/samsung/galaxy5/proprietary/lib/egl/libEGL_adreno200.so:system/lib/egl/libEGL_adreno200.so \
    vendor/samsung/galaxy5/proprietary/lib/egl/libGLESv1_CM_adreno200.so:system/lib/egl/libGLESv1_CM_adreno200.so \
    vendor/samsung/galaxy5/proprietary/lib/egl/libGLESv2_adreno200.so:system/lib/egl/libGLESv2_adreno200.so \
    vendor/samsung/galaxy5/proprietary/lib/egl/libq3dtools_adreno200.so:system/lib/egl/libq3dtools_adreno200.so \
    vendor/samsung/galaxy5/proprietary/lib/libgsl.so:system/lib/libgsl.so \
    vendor/samsung/galaxy5/proprietary/etc/firmware/yamato_pfp.fw:system/etc/firmware/yamato_pfp.fw \
    vendor/samsung/galaxy5/proprietary/etc/firmware/yamato_pm4.fw:system/etc/firmware/yamato_pm4.fw

# Camera
PRODUCT_COPY_FILES += \
    vendor/samsung/galaxy5/proprietary/lib/liboemcamera.so:system/lib/liboemcamera.so \
    vendor/samsung/galaxy5/proprietary/lib/libmmipl.so:system/lib/libmmipl.so \
    vendor/samsung/galaxy5/proprietary/lib/libmmjpeg.so:system/lib/libmmjpeg.so

# Wifi
PRODUCT_COPY_FILES += \
    vendor/samsung/galaxy5/proprietary/wifi/ath6k/AR6003/hw2.0/athtcmd_ram.bin:system/wifi/ath6k/AR6003/hw2.0/athtcmd_ram.bin \
    vendor/samsung/galaxy5/proprietary/wifi/ath6k/AR6003/hw2.0/athwlan.bin.z77:system/wifi/ath6k/AR6003/hw2.0/athwlan.bin.z77 \
    vendor/samsung/galaxy5/proprietary/wifi/ath6k/AR6003/hw2.0/bdata.SD31.bin:system/wifi/ath6k/AR6003/hw2.0/bdata.SD31.bin \
    vendor/samsung/galaxy5/proprietary/wifi/ath6k/AR6003/hw2.0/data.patch.bin:system/wifi/ath6k/AR6003/hw2.0/data.patch.bin \
    vendor/samsung/galaxy5/proprietary/wifi/ath6k/AR6003/hw2.0/otp.bin.z77:system/wifi/ath6k/AR6003/hw2.0/otp.bin.z77 \
    vendor/samsung/galaxy5/proprietary/bin/wlan_tool:system/bin/wlan_tool \
    vendor/samsung/galaxy5/proprietary/bin/wmiconfig:system/bin/wmiconfig \
    vendor/samsung/galaxy5/proprietary/bin/hostapd:system/bin/hostapd \
    vendor/samsung/galaxy5/proprietary/bin/hostapd_cli:system/bin/hostapd_cli \
    vendor/samsung/galaxy5/proprietary/bin/hostapd_wps:system/bin/hostapd_wps \
    device/samsung/galaxy5/files/wifi/ar6000.ko:system/wifi/ar6000.ko \
    device/samsung/galaxy5/files/etc/wifi/hostapd.conf:system/etc/wifi/hostapd.conf \
    device/samsung/galaxy5/files/etc/wifi/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf \
    device/samsung/galaxy5/files/etc/dhcpd/dhcpcd.conf:system/etc/dhcpcd/dhcpcd.conf

# SD Card
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/files/etc/vold.fstab:system/etc/vold.fstab

# Media configuration xml file
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/files/etc/media_profiles.xml:/system/etc/media_profiles.xml

# Audio
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/files/etc/AudioFilter.csv:system/etc/AudioFilter.csv \
    vendor/samsung/galaxy5/proprietary/lib/liba2dp.so:system/lib/liba2dp.so \
    vendor/samsung/galaxy5/proprietary/lib/libaudioeq.so:system/lib/libaudioeq.so \
    device/samsung/galaxy5/files/etc/AutoVolumeControl.txt:system/etc/AutoVolumeControl.txt

# Device permissions
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/base/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/base/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/base/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml

# Move dalvik-cache to /data
PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.dexopt-data-only=1

# Samsung RIL
PRODUCT_COPY_FILES += \
    vendor/samsung/galaxy5/proprietary/lib/libsec-ril.so:system/lib/libsec-ril.so \
    vendor/samsung/galaxy5/proprietary/lib/libsecril-client.so:system/lib/libsecril-client.so \
    vendor/samsung/galaxy5/proprietary/lib/libseccamera.so:system/lib/libseccamera.so \
    vendor/samsung/galaxy5/proprietary/lib/libsecgps.so:system/lib/libsecgps.so

# Bluetooth
PRODUCT_COPY_FILES += \
    vendor/samsung/galaxy5/proprietary/bin/BCM2049B0_BCM20780B0_002.001.022.0170.0174.hcd:system/bin/BCM2049B0_BCM20780B0_002.001.022.0170.0174.hcd

# OMX libraries
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/files/lib/libmm-adspsvc.so:system/lib/libmm-adspsvc.so \
    device/samsung/galaxy5/files/lib/libOmxAacDec.so:system/lib/libOmxAacDec.so \
    device/samsung/galaxy5/files/lib/libOmxAmrRtpDec.so:system/lib/libOmxAmrRtpDec.so \
    device/samsung/galaxy5/files/lib/libOmxH264Dec.so:system/lib/libOmxH264Dec.so \
    device/samsung/galaxy5/files/lib/libOmxQcelpDec.so:system/lib/libOmxQcelpDec.so \
    device/samsung/galaxy5/files/lib/libOmxAacEnc.so:system/lib/libOmxAacEnc.so \
    device/samsung/galaxy5/files/lib/libOmxAmrwbDec.so:system/lib/libOmxAmrwbDec.so \
    device/samsung/galaxy5/files/lib/libOmxMp3Dec.so:system/lib/libOmxMp3Dec.so \
    device/samsung/galaxy5/files/lib/libOmxVidEnc.so:system/lib/libOmxVidEnc.so \
    device/samsung/galaxy5/files/lib/libOmxAmrDec.so:system/lib/libOmxAmrDec.so \
    device/samsung/galaxy5/files/lib/libOmxEvrcDec.so:system/lib/libOmxEvrcDec.so \
    device/samsung/galaxy5/files/lib/libOmxMpeg4Dec.so:system/lib/libOmxMpeg4Dec.so \
    device/samsung/galaxy5/files/lib/libOmxWmaDec.so:system/lib/libOmxWmaDec.so \
    device/samsung/galaxy5/files/lib/libOmxAmrEnc.so:system/lib/libOmxAmrEnc.so \
    device/samsung/galaxy5/files/lib/libOmxEvrcEnc.so:system/lib/libOmxEvrcEnc.so \
    device/samsung/galaxy5/files/lib/libOmxQcelp13Enc.so:system/lib/libOmxQcelp13Enc.so \
    device/samsung/galaxy5/files/lib/libOmxWmvDec.so:system/lib/libOmxWmvDec.so

$(call inherit-product, device/common/gps/gps_us_supl.mk)

$(call inherit-product, build/target/product/full_base.mk)

PRODUCT_BUILD_PROP_OVERRIDES += BUILD_UTC_DATE=0
PRODUCT_NAME := galaxy5
PRODUCT_BRAND := Samsung
PRODUCT_DEVICE := galaxy5
PRODUCT_MODEL := Samsung Galaxy5
PRODUCT_MANUFACTURER := Samsung
PRODUCT_BUILD_PROP_OVERRIDES += PRODUCT_NAME=galaxy5

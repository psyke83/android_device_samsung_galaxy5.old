ifeq ($(TARGET_PREBUILT_KERNEL),)
	LOCAL_KERNEL := device/samsung/galaxy5/kernel
else
	LOCAL_KERNEL := $(TARGET_PREBUILT_KERNEL)
endif

PRODUCT_COPY_FILES += \
    $(LOCAL_KERNEL):kernel

PRODUCT_PACKAGES += \
    libOmxCore \
    libOmxVidEnc \
    brcm_patchram_plus \
    abtfilt \
    gps.galaxy5 \
    gralloc.galaxy5 \
    copybit.galaxy5 \
    lights.msm7k \
    screencap \
    rzscontrol

# Recovery tools
PRODUCT_PACKAGES += \
    flash_image \
    dump_image \
    erase_image \
    make_ext4fs \
    e2fsck

# Boot screen
#PRODUCT_COPY_FILES += \
#    device/samsung/galaxy5/files/root/EUROPA.rle:root/EUROPA.rle \
#    device/samsung/galaxy5/files/root/EUROPA.rle:recovery/root/EUROPA.rle

# Live wallpaper packages
PRODUCT_PACKAGES += \
    LiveWallpapers \
    LiveWallpapersPicker \
    MagicSmokeWallpapers \
    VisualizationWallpapers \
    librs_jni

# Hardware properties
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/base/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/base/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/base/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml

# Publish that we support the live wallpaper feature.
PRODUCT_COPY_FILES += \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:/system/etc/permissions/android.software.live_wallpaper.xml

# Kernel modules
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/files/root/lib/modules/fsr.ko:root/lib/modules/fsr.ko \
    device/samsung/galaxy5/files/root/lib/modules/fsr_stl.ko:root/lib/modules/fsr_stl.ko \
    device/samsung/galaxy5/files/root/lib/modules/acc_cal_param.ko:root/lib/modules/acc_cal_param.ko \
    device/samsung/galaxy5/files/root/lib/modules/sec_param.ko:root/lib/modules/sec_param.ko \
    device/samsung/galaxy5/files/root/lib/modules/ramzswap.ko:system/lib/modules/ramzswap.ko

# Recovery kernel modules / files
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/files/root/lib/modules/rfs_glue.ko:recovery/root/lib/modules/rfs_glue.ko \
    device/samsung/galaxy5/files/root/lib/modules/rfs_fat.ko:recovery/root/lib/modules/rfs_fat.ko \
    device/samsung/galaxy5/recovery.fstab_full:recovery/root/etc/recovery.fstab \
    device/samsung/galaxy5/files/bin/fat.format:recovery/root/sbin/fat.format

# Device-specific keymaps
PRODUCT_COPY_FILES += \
    vendor/samsung/galaxy5/proprietary/usr/keylayout/europa_headset.kl:system/usr/keylayout/europa_headset.kl \
    vendor/samsung/galaxy5/proprietary/usr/keylayout/europa_keypad0.kl:system/usr/keylayout/europa_keypad0.kl \
    vendor/samsung/galaxy5/proprietary/usr/keylayout/qwerty.kl:system/usr/keylayout/qwerty.kl \
    vendor/samsung/galaxy5/proprietary/usr/keychars/europa_keypad0.kcm.bin:system/usr/keychars/europa_keypad0.kcm.bin

# Board-specific init
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/files/ueventd.gt-i5500.rc:root/ueventd.gt-i5500.rc \
    device/samsung/galaxy5/files/init.gt-i5500.rc:root/init.gt-i5500.rc \
    device/samsung/galaxy5/files/bin/get_macaddrs:system/bin/get_macaddrs \
    device/samsung/galaxy5/files/bin/madconfig:system/bin/madconfig \
    device/samsung/galaxy5/files/etc/sysctl.conf:system/etc/sysctl.conf \
    device/samsung/galaxy5/files/etc/init.d/02madteam:system/etc/init.d/02madteam \
    device/samsung/galaxy5/files/etc/init.d/05mountsd:system/etc/init.d/05mountsd

# Sensors
PRODUCT_COPY_FILES += \
    vendor/samsung/galaxy5/proprietary/lib/hw/sensors.default.so:system/lib/hw/sensors.default.so \
    vendor/samsung/galaxy5/proprietary/bin/memsicd:system/bin/memsicd

# 3D
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/files/lib/egl/libEGL_adreno200.so:system/lib/egl/libEGL_adreno200.so \
    device/samsung/galaxy5/files/lib/egl/libGLESv1_CM_adreno200.so:system/lib/egl/libGLESv1_CM_adreno200.so \
    device/samsung/galaxy5/files/lib/egl/libGLESv2_adreno200.so:system/lib/egl/libGLESv2_adreno200.so \
    device/samsung/galaxy5/files/lib/egl/libq3dtools_adreno200.so:system/lib/egl/libq3dtools_adreno200.so \
    device/samsung/galaxy5/files/lib/libgsl.so:system/lib/libgsl.so \
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
    device/samsung/galaxy5/files/wifi/ar6000.ko:system/wifi/ar6000.ko \
    vendor/samsung/galaxy5/proprietary/bin/wlan_tool:system/bin/wlan_tool \
    vendor/samsung/galaxy5/proprietary/bin/wmiconfig:system/bin/wmiconfig \
    vendor/samsung/galaxy5/proprietary/bin/hostapd:system/bin/hostapd \
    vendor/samsung/galaxy5/proprietary/bin/hostapd_cli:system/bin/hostapd_cli \
    vendor/samsung/galaxy5/proprietary/bin/hostapd_wps:system/bin/hostapd_wps \
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

# Samsung RIL
PRODUCT_COPY_FILES += \
    vendor/samsung/galaxy5/proprietary/bin/rild:system/bin/rild \
    vendor/samsung/galaxy5/proprietary/bin/qmuxd:system/bin/qmuxd \
    vendor/samsung/galaxy5/proprietary/lib/libril.so:system/lib/libril.so \
    vendor/samsung/galaxy5/proprietary/lib/libsec-ril.so:system/lib/libsec-ril.so \
    vendor/samsung/galaxy5/proprietary/lib/libsecril-client.so:system/lib/libsecril-client.so \
    vendor/samsung/galaxy5/proprietary/lib/libdiag.so:system/lib/libdiag.so

# Bluetooth
PRODUCT_COPY_FILES += \
    vendor/samsung/galaxy5/proprietary/bin/BCM2049B0_BCM20780B0_002.001.022.0170.0174.hcd:system/bin/BCM2049B0_BCM20780B0_002.001.022.0170.0174.hcd

# OMX libraries
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/files/lib/libmm-adspsvc.so:system/lib/libmm-adspsvc.so \
    device/samsung/galaxy5/files/lib/libomx_amrenc_sharedlibrary.so:system/lib/libomx_amrenc_sharedlibrary.so \
    device/samsung/galaxy5/files/lib/libOmxEvrcEnc.so:system/lib/libOmxEvrcEnc.so \
    device/samsung/galaxy5/files/lib/libOmxWmaDec.so:system/lib/libOmxWmaDec.so \
    device/samsung/galaxy5/files/lib/libOmxH264Dec.so:system/lib/libOmxH264Dec.so \
    device/samsung/galaxy5/files/lib/libomx_sharedlibrary.so:system/lib/libomx_sharedlibrary.so \
    device/samsung/galaxy5/files/lib/libOmxQcelp13Enc.so:system/lib/libOmxQcelp13Enc.so \
    device/samsung/galaxy5/files/lib/libOmxMp3Dec.so:system/lib/libOmxMp3Dec.so \
    device/samsung/galaxy5/files/lib/libOmxAacEnc.so:system/lib/libOmxAacEnc.so \
    device/samsung/galaxy5/files/lib/libOmxAmrDec.so:system/lib/libOmxAmrDec.so \
    device/samsung/galaxy5/files/lib/libOmxAdpcmDec.so:system/lib/libOmxAdpcmDec.so \
    device/samsung/galaxy5/files/lib/libomx_m4vdec_sharedlibrary.so:system/lib/libomx_m4vdec_sharedlibrary.so \
    device/samsung/galaxy5/files/lib/libOmxWmvDec.so:system/lib/libOmxWmvDec.so \
    device/samsung/galaxy5/files/lib/libomx_amrdec_sharedlibrary.so:system/lib/libomx_amrdec_sharedlibrary.so \
    device/samsung/galaxy5/files/lib/libOmxAacDec.so:system/lib/libOmxAacDec.so \
    device/samsung/galaxy5/files/lib/libOmxAmrEnc.so:system/lib/libOmxAmrEnc.so \
    device/samsung/galaxy5/files/lib/libOmxEvrcDec.so:system/lib/libOmxEvrcDec.so \
    device/samsung/galaxy5/files/lib/libOmxAmrRtpDec.so:system/lib/libOmxAmrRtpDec.so \
    device/samsung/galaxy5/files/lib/libomx_mp3dec_sharedlibrary.so:system/lib/libomx_mp3dec_sharedlibrary.so \
    device/samsung/galaxy5/files/lib/libOmxAmrwbDec.so:system/lib/libOmxAmrwbDec.so \
    device/samsung/galaxy5/files/lib/libOmxMpeg4Dec.so:system/lib/libOmxMpeg4Dec.so \
    device/samsung/galaxy5/files/lib/libOmxQcelpDec.so:system/lib/libOmxQcelpDec.so \
    device/samsung/galaxy5/files/lib/libomx_aacdec_sharedlibrary.so:system/lib/libomx_aacdec_sharedlibrary.so \
    device/samsung/galaxy5/files/lib/libomx_avcdec_sharedlibrary.so:system/lib/libomx_avcdec_sharedlibrary.so

$(call inherit-product, build/target/product/full_base.mk)

# Inherit some common cyanogenmod stuff.
$(call inherit-product, vendor/cyanogen/products/common_full.mk)

# The gps config appropriate for this device
$(call inherit-product, device/common/gps/gps_us_supl.mk)

# Include GSM stuff
$(call inherit-product, vendor/cyanogen/products/gsm.mk)

# Broadcom FM radio
$(call inherit-product, vendor/cyanogen/products/bcm_fm_radio.mk)

$(call inherit-product-if-exists, vendor/samsung/galaxy5/galaxy5-vendor.mk)

DEVICE_PACKAGE_OVERLAYS += device/samsung/galaxy5/overlay

# LDPI assets
PRODUCT_LOCALES += ldpi mdpi
PRODUCT_PACKAGE_OVERLAYS += vendor/cyanogen/overlay/ldpi
PRODUCT_COPY_FILES += \
    vendor/cyanogen/prebuilt/ldpi/media/bootanimation.zip:system/media/bootanimation.zip

PRODUCT_PROPERTY_OVERRIDES += \
    ro.modversion=CyanogenMod-7.1.0-RC1-MADTEAM

PRODUCT_NAME := full_galaxy5
PRODUCT_BRAND := samsung
PRODUCT_DEVICE := galaxy5
PRODUCT_MODEL := GT-I5500
PRODUCT_MANUFACTURER := samsung

## This device is the Samsung Galaxy 5 (I5500)

# The gps config appropriate for this device
$(call inherit-product, device/common/gps/gps_us_supl.mk)

# Add device package overlay
DEVICE_PACKAGE_OVERLAYS += device/samsung/galaxy5/overlay

# Add LDPI assets, in addition to MDPI
PRODUCT_LOCALES += ldpi mdpi

# Extra overlay for LDPI
PRODUCT_PACKAGE_OVERLAYS += vendor/cyanogen/overlay/ldpi

# Broadcom FM radio
$(call inherit-product, vendor/cyanogen/products/bcm_fm_radio.mk)

# Copy bootanimation
PRODUCT_COPY_FILES += \
    vendor/cyanogen/prebuilt/ldpi/media/bootanimation.zip:system/media/bootanimation.zip


# HAL libs and other system binaries
PRODUCT_PACKAGES += \
    abtfilt \
    copybit.galaxy5 \
    gps.galaxy5 \
    gralloc.galaxy5 \
    libOmxCore

# Live wallpaper packages
PRODUCT_PACKAGES += \
    LiveWallpapers \
    LiveWallpapersPicker \
    MagicSmokeWallpapers \
    VisualizationWallpapers \
    librs_jni

# Publish that we support the live wallpaper feature.
PRODUCT_COPY_FILES += \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:/system/etc/permissions/android.software.live_wallpaper.xml

# Board-specific init
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/ueventd.qct.rc:root/ueventd.qct.rc \
    device/samsung/galaxy5/init.qcom.rc:root/init.qcom.rc

## RIL related stuff
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/spn-conf.xml:system/etc/spn-conf.xml \
    vendor/samsung/galaxy5/proprietary/bin/qmuxd:system/bin/qmuxd \
    vendor/samsung/galaxy5/proprietary/lib/liboncrpc.so:system/lib/liboncrpc.so \
    vendor/samsung/galaxy5/proprietary/lib/libmmgsdilib.so:system/lib/libmmgsdilib.so \
    vendor/samsung/galaxy5/proprietary/lib/libgsdi_exp.so:system/lib/libgsdi_exp.so \
    vendor/samsung/galaxy5/proprietary/lib/libgstk_exp.so:system/lib/libgstk_exp.so \
    vendor/samsung/galaxy5/proprietary/lib/libwms.so:system/lib/libwms.so \
    vendor/samsung/galaxy5/proprietary/lib/libnv.so:system/lib/libnv.so \
    vendor/samsung/galaxy5/proprietary/lib/libdss.so:system/lib/libdss.so \
    vendor/samsung/galaxy5/proprietary/lib/libqmi.so:system/lib/libqmi.so \
    vendor/samsung/galaxy5/proprietary/lib/libdiag.so:system/lib/libdiag.so \
    vendor/samsung/galaxy5/proprietary/lib/libpbmlib.so:system/lib/libpbmlib.so \
    vendor/samsung/galaxy5/proprietary/lib/libauth.so:system/lib/libauth.so \
    vendor/samsung/galaxy5/proprietary/lib/liboem_rapi.so:system/lib/liboem_rapi.so \
    vendor/samsung/galaxy5/proprietary/lib/libdsm.so:system/lib/libdsm.so \
    vendor/samsung/galaxy5/proprietary/lib/libqueue.so:system/lib/libqueue.so \
    vendor/samsung/galaxy5/proprietary/lib/libcm.so:system/lib/libcm.so \
    vendor/samsung/galaxy5/proprietary/lib/libdll.so:system/lib/libdll.so \
    vendor/samsung/galaxy5/proprietary/bin/rild:system/bin/rild \
    vendor/samsung/galaxy5/proprietary/lib/libril.so:system/lib/libril.so \
    vendor/samsung/galaxy5/proprietary/lib/libreference-ril.so:system/lib/libreference-ril.so \
    vendor/samsung/galaxy5/proprietary/lib/libseccamera.so:system/lib/libseccamera.so \
    vendor/samsung/galaxy5/proprietary/lib/libseccameraadaptor.so:system/lib/libseccameraadaptor.so \
    vendor/samsung/galaxy5/proprietary/lib/libsecgps.so:system/lib/libsecgps.so \
    vendor/samsung/galaxy5/proprietary/lib/libsecril-client.so:system/lib/libsecril-client.so \
    device/samsung/galaxy5/prebuilt/libsec-ril.so:system/lib/libsec-ril.so

## OMX proprietaries
PRODUCT_COPY_FILES += \
    vendor/samsung/galaxy5/proprietary/lib/libmm-adspsvc.so:system/lib/libmm-adspsvc.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxAacDec.so:system/lib/libOmxAacDec.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxAmrRtpDec.so:system/lib/libOmxAmrRtpDec.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxH264Dec.so:system/lib/libOmxH264Dec.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxQcelpDec.so:system/lib/libOmxQcelpDec.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxAacEnc.so:system/lib/libOmxAacEnc.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxAmrwbDec.so:system/lib/libOmxAmrwbDec.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxMp3Dec.so:system/lib/libOmxMp3Dec.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxVidEnc.so:system/lib/libOmxVidEnc.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxAmrDec.so:system/lib/libOmxAmrDec.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxEvrcDec.so:system/lib/libOmxEvrcDec.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxMpeg4Dec.so:system/lib/libOmxMpeg4Dec.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxWmaDec.so:system/lib/libOmxWmaDec.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxAmrEnc.so:system/lib/libOmxAmrEnc.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxEvrcEnc.so:system/lib/libOmxEvrcEnc.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxQcelp13Enc.so:system/lib/libOmxQcelp13Enc.so \
    vendor/samsung/galaxy5/proprietary/lib/libOmxWmvDec.so:system/lib/libOmxWmvDec.so

## Hardware properties 
PRODUCT_COPY_FILES += \
    frameworks/base/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/base/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/base/data/etc/android.hardware.telephony.gsm.xml:system/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/base/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/base/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/base/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/base/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml

## Camera proprietaries
PRODUCT_COPY_FILES += \
    vendor/samsung/galaxy5/proprietary/lib/liboemcamera.so:system/lib/liboemcamera.so \
    vendor/samsung/galaxy5/proprietary/lib/libmmjpeg.so:system/lib/libmmjpeg.so \
    vendor/samsung/galaxy5/proprietary/lib/libmmipl.so:system/lib/libmmipl.so

## Atheros AR6003 firmware
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/athtcmd_ram.bin:system/wifi/ath6k/AR6003/hw2.0/athtcmd_ram.bin \
    device/samsung/galaxy5/prebuilt/athwlan.bin.z77:system/wifi/ath6k/AR6003/hw2.0/athwlan.bin.z77 \
    device/samsung/galaxy5/prebuilt/bdata.SD31.bin:system/wifi/ath6k/AR6003/hw2.0/bdata.SD31.bin \
    device/samsung/galaxy5/prebuilt/data.patch.bin:system/wifi/ath6k/AR6003/hw2.0/data.patch.bin \
    device/samsung/galaxy5/prebuilt/otp.bin.z77:system/wifi/ath6k/AR6003/hw2.0/otp.bin.z77 \
    device/samsung/galaxy5/prebuilt/ar6000.ko:system/wifi/ar6000.ko \
    device/samsung/galaxy5/hostapd.conf:system/etc/wifi/hostapd.conf \
    vendor/samsung/galaxy5/proprietary/bin/hostapd:system/bin/hostapd

## Other libraries and proprietary binaries
PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/prebuilt/init.qcom.bt.sh:system/bin/init.qcom.bt.sh \
    vendor/samsung/galaxy5/proprietary/lib/hw/sensors.default.so:system/lib/hw/sensors.default.so \
    device/samsung/galaxy5/AutoVolumeControl.txt:system/etc/AutoVolumeControl.txt \
    device/samsung/galaxy5/AudioFilter.csv:system/etc/AudioFilter.csv \
    vendor/samsung/galaxy5/proprietary/lib/liba2dp.so:system/lib/liba2dp.so \
    vendor/samsung/galaxy5/proprietary/lib/libaudioeq.so:system/lib/libaudioeq.so \
    vendor/samsung/galaxy5/proprietary/lib/egl/egl.cfg:system/lib/egl/egl.cfg \
    vendor/samsung/galaxy5/proprietary/lib/egl/libGLESv1_CM_adreno200.so:system/lib/egl/libGLESv1_CM_adreno200.so \
    vendor/samsung/galaxy5/proprietary/lib/egl/libq3dtools_adreno200.so:system/lib/egl/libq3dtools_adreno200.so \
    vendor/samsung/galaxy5/proprietary/lib/egl/libEGL_adreno200.so:system/lib/egl/libEGL_adreno200.so \
    vendor/samsung/galaxy5/proprietary/lib/egl/libGLESv2_adreno200.so:system/lib/egl/libGLESv2_adreno200.so \
    vendor/samsung/galaxy5/proprietary/etc/firmware/yamato_pfp.fw:system/etc/firmware/yamato_pfp.fw \
    vendor/samsung/galaxy5/proprietary/etc/firmware/yamato_pm4.fw:system/etc/firmware/yamato_pm4.fw \
    vendor/samsung/galaxy5/proprietary/lib/libgsl.so:system/lib/libgsl.so \
    vendor/samsung/galaxy5/proprietary/bin/memsicd:system/bin/memsicd \
    device/samsung/galaxy5/prebuilt/framework.jar:system/framework/framework.jar

PRODUCT_COPY_FILES += \
    device/samsung/galaxy5/media_profiles.xml:system/etc/media_profiles.xml \
    device/samsung/galaxy5/dhcpcd.conf:system/etc/dhcpcd/dhcpcd.conf \
    device/samsung/galaxy5/vold.fstab:system/etc/vold.fstab \
    device/samsung/galaxy5/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf \
    device/samsung/galaxy5/02fixperms:system/etc/init.d/02fixperms

$(call inherit-product, build/target/product/full_base.mk)

PRODUCT_BUILD_PROP_OVERRIDES += BUILD_UTC_DATE=0
PRODUCT_NAME := galaxy5
PRODUCT_DEVICE := galaxy5
PRODUCT_MODEL := Samsung Galaxy5

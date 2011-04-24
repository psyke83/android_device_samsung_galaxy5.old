#!/bin/sh

mkdir -p ../../../vendor/samsung/galaxy5/proprietary

DIRS="
bin
etc/firmware
lib/egl
lib/hw
wifi/ath6k/AR6003/hw2.0
"

for DIR in $DIRS; do
	mkdir -p ../../../vendor/samsung/galaxy5/proprietary/$DIR
done

FILES="
bin/qmuxd

etc/firmware/yamato_pfp.fw
etc/firmware/yamato_pm4.fw

lib/egl/egl.cfg
lib/egl/libEGL_adreno200.so
lib/egl/libGLESv1_CM_adreno200.so
lib/egl/libGLESv2_adreno200.so
lib/egl/libq3dtools_adreno200.so

lib/hw/gralloc.default.so
lib/hw/gralloc.msm7k.so

lib/hw/sensors.default.so

lib/liba2dp.so
lib/libaudioeq.so

lib/libauth.so
lib/libcm.so
lib/libdiag.so
lib/libdll.so
lib/libdsm.so
lib/libdss.so
lib/libgsdi_exp.so
lib/libgsl.so
lib/libgstk_exp.so
lib/libmmgsdilib.so
lib/libnv.so
lib/liboem_rapi.so
lib/liboncrpc.so
lib/libpbmlib.so
lib/libqmi.so
lib/libqueue.so
lib/libril.so
lib/libwms.so

bin/hostapd
bin/memsicd
bin/rild
lib/libreference-ril.so
lib/libseccamera.so
lib/libseccameraadaptor.so
lib/libsecgps.so
lib/libsecril-client.so

lib/libmmipl.so
lib/libmmjpeg.so
lib/liboemcamera.so

lib/libmm-adspsvc.so
lib/libOmxAacDec.so
lib/libOmxAacEnc.so
lib/libOmxAmrDec.so
lib/libOmxAmrEnc.so
lib/libOmxAmrRtpDec.so
lib/libOmxAmrwbDec.so
lib/libOmxEvrcDec.so
lib/libOmxEvrcEnc.so
lib/libOmxH264Dec.so
lib/libOmxMp3Dec.so
lib/libOmxMpeg4Dec.so
lib/libOmxQcelp13Enc.so
lib/libOmxQcelpDec.so
lib/libOmxVidEnc.so
lib/libOmxWmaDec.so
lib/libOmxWmvDec.so
"

for FILE in $FILES; do
	cp  ~/Desktop/system.jpl/$FILE ../../../vendor/samsung/galaxy5/proprietary/$FILE
done


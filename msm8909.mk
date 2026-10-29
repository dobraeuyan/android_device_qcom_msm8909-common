DEVICE_PATH := device/qcom/msm8909-common

DEVICE_PACKAGE_OVERLAYS += $(DEVICE_PATH)/overlay

PRODUCT_AAPT_CONFIG := normal large xlarge mdpi hdpi xhdpi

PRODUCT_ART_TARGET_INCLUDE_DEBUG_BUILD := false
PRODUCT_DEX_PREOPT_BOOT_IMAGE_PROFILE_LOCATION := frameworks/base/config/boot-image-profile.txt
PRODUCT_MINIMIZE_JAVA_DEBUG_INFO := true
PRODUCT_USE_PROFILE_FOR_BOOT_IMAGE := true

PRODUCT_PACKAGES += \
    InProcessNetworkStack \
    RemovePackages

PRODUCT_PACKAGES += \
    vendor.qti.hardware.alarm@1.0-impl \
    vendor.qti.hardware.alarm@1.0-service

PRODUCT_PACKAGES += \
    android.hardware.audio@2.0-service \
    android.hardware.audio@4.0-impl \
    android.hardware.audio.effect@4.0-impl \
    audio.primary.msm8909 \
    audio.a2dp.default \
    audio.r_submix.default \
    audio.usb.default \
    libqcomvisualizer \
    libqcompostprocbundle \
    libqcomvoiceprocessing \
    libaudioroute \
    libaudio-resampler \
    tinymix \
    tinyplay

PRODUCT_PACKAGES += \
    android.hardware.camera.provider@2.4-impl \
    android.hardware.camera.provider@2.4-service \
    libxml2 \
    Snap

PRODUCT_PACKAGES += \
    android.hardware.configstore@1.1-service

PRODUCT_PACKAGES += \
    android.hardware.bluetooth@1.0-service.msm8909 \
    libbt-vendor

PRODUCT_PACKAGES += \
    android.hardware.graphics.allocator@2.0-impl \
    android.hardware.graphics.allocator@2.0-service \
    android.hardware.graphics.composer@2.1-impl \
    android.hardware.graphics.composer@2.1-service \
    android.hardware.graphics.mapper@2.0-impl \
    android.hardware.renderscript@1.0-impl \
    gralloc.msm8909 \
    hwcomposer.msm8909 \
    libgenlock \
    libtinyxml

PRODUCT_BUILD_PROP_OVERRIDES += \
    PRIVATE_BUILD_DESC="hq_msm8909-user 8.1.0 OPM1.171019.026 56 release-keys"

BUILD_FINGERPRINT := Lenovo/LenovoTB-X104F/X104F:8.1.0/OPM1.171019.026/053_221021_ROW:user/release-keys

TARGET_HAS_FM_RADIO ?= true

ifeq ($(TARGET_HAS_FM_RADIO),true)
PRODUCT_PACKAGES += \
    FMRadio \
    libfmjni \
    vendor.qti.hardware.fm@1.0
endif

PRODUCT_PACKAGES += \
    android.hardware.gatekeeper@1.0-service

PRODUCT_PACKAGES += \
    android.hardware.gnss@1.0-service \
    libizatapi_shim \
    libperipheral_client \
    libslim_shim \
    libwpa_client

PRODUCT_PACKAGES += \
    android.hardware.health@2.0-service \
    android.hardware.health@2.0-impl \
    android.hardware.health@2.0-impl.recovery

PRODUCT_PACKAGES += \
    android.hardware.light@2.0-impl \
    android.hardware.light@2.0-service \
    lights.msm8909

PRODUCT_PACKAGES += \
    android.hardware.thermal@2.0-service.msm8909

PRODUCT_PACKAGES += \
    vendor.lineage.livedisplay@2.0-service-sysfs \
    vendor.lineage.livedisplay@2.0-service-pa

PRODUCT_PACKAGES += \
    android.hardware.cas@1.0 \
    android.hardware.drm@1.0-impl \
    android.hardware.drm@1.0-service \
    android.hardware.drm@1.2-service.clearkey \
    android.hardware.drm@1.2-service.widevine \
    android.hardware.media.omx@1.0-service \
    libwvdrm_shim \
    libc2dcolorconvert \
    libOmxAacEnc \
    libOmxAmrEnc \
    libOmxCore \
    libOmxEvrcEnc \
    libOmxQcelp13Enc \
    libOmxVdec \
    libOmxVenc \
    libstagefrighthw

PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_video_le.xml

PRODUCT_PACKAGES += \
    android.hardware.memtrack@1.0-impl \
    android.hardware.memtrack@1.0-service \
    memtrack.msm8909

PRODUCT_PACKAGES += \
    com.qualcomm.qti.wifidisplayhal@1.0-impl

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.software.midi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.midi.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/handheld_core_hardware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/handheld_core_hardware.xml

PRODUCT_PACKAGES += \
    android.hardware.power@1.0-impl \
    android.hardware.power@1.0-service \
    power.qcom

PRODUCT_PACKAGES += \
    init.bt_bdaddr.sh \
    init.msm.usb.configfs.rc \
    init.qcom.post_boot.sh \
    init.qcom.rc \
    init.qcom.usb.rc \
    init.qcom.usb.sh \
    init.recovery.qcom.rc \
    ueventd.rc

PRODUCT_PACKAGES += \
    android.hardware.sensors@1.0-impl \
    android.hardware.sensors@1.0-service

$(call inherit-product-if-exists, build/make/target/product/product_launched_with_o_mr1.mk)

PRODUCT_PACKAGES += \
    timekeep \
    TimeKeep

PRODUCT_PACKAGES += \
    vendor.lineage.trust@1.0-service

PRODUCT_PACKAGES += \
    android.hardware.usb@1.0 \
    android.hardware.usb@1.0-service.basic

PRODUCT_PACKAGES += \
    android.hardware.vibrator@1.0-impl \
    android.hardware.vibrator@1.0-service \
    vibrator.default

PRODUCT_PACKAGES += \
    android.hardware.wifi@1.0-service \
    android.hardware.wifi@1.1 \
    android.hardware.wifi.supplicant@1.0 \
    hostapd \
    libcurl \
    libkeystore-wifi-hidl \
    libkeystore-engine-wifi-hidl \
    wpa_supplicant \
    wpa_supplicant.conf

PRODUCT_PACKAGES += \
    WfdProvider

$(call inherit-product, vendor/qcom/msm8909-common/msm8909-vendor.mk)

PRODUCT_PACKAGES += \
    android.hardware.radio@1.0 \
    android.hardware.radio@1.1 \
    android.hardware.radio.config@1.0 \
    android.hardware.radio.deprecated@1.0 \
    libril \
    librilutils \
    rild

PRODUCT_COPY_FILES += \
    device/qcom/msm8909-common/vintf/manifest-radio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/vintf/manifest/manifest-radio.xml

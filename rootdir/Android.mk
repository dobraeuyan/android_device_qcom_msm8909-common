LOCAL_PATH := $(call my-dir)

# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE := init.qcom.usb.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES := bin/init.qcom.usb.sh
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := init.qcom.post_boot.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SRC_FILES := bin/init.qcom.post_boot.sh
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin
include $(BUILD_PREBUILT)

# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_MODULE := init.msm.usb.configfs.rc
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := etc/init.msm.usb.configfs.rc
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/init/hw
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := init.qcom.usb.rc
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := etc/init.qcom.usb.rc
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/init/hw
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := init.qcom.rc
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := etc/init.qcom.rc
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/init/hw
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := ueventd.rc
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := etc/ueventd.rc
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := init.recovery.qcom.rc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := etc/init.recovery.qcom.rc
LOCAL_MODULE_PATH := $(TARGET_RECOVERY_ROOT_OUT)
include $(BUILD_PREBUILT)

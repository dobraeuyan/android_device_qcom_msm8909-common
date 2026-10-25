LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_BOARD_PLATFORM),msm8909)

include $(call all-subdir-makefiles,$(LOCAL_PATH))

include $(CLEAR_VARS)

ifeq ($(call project-path-for,qcom-display),hardware/qcom/display/msm8909)
include hardware/qcom/display/msm8909/Android.mk
endif

WLAN_PRIMA_DIR := $(TARGET_OUT_VENDOR)/firmware/wlan/prima

WCNSS_CFG_INI := $(WLAN_PRIMA_DIR)/WCNSS_qcom_cfg.ini
$(WCNSS_CFG_INI): $(LOCAL_INSTALLED_MODULE)
	@echo "WCNSS_qcom_cfg.ini Firmware link: $@"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf /data/misc/wifi/$(notdir $@) $@

PERSIST_DICT := $(WLAN_PRIMA_DIR)/WCNSS_wlan_dictionary.dat
$(PERSIST_DICT): $(LOCAL_INSTALLED_MODULE)
	@echo "WCNSS_wlan_dictionary.dat Firmware link: $@"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf /mnt/per/$(notdir $@) $@

ifneq ($(TARGET_WCNSS_WLAN_NV_IN_PERSIST),false)
PERSIST_NV := $(WLAN_PRIMA_DIR)/WCNSS_qcom_wlan_nv.bin
$(PERSIST_NV): $(LOCAL_INSTALLED_MODULE)
	@echo "WCNSS_qcom_wlan_nv.bin Firmware link: $@"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf /mnt/per/$(notdir $@) $@
endif

RFS_APQ_GNSS_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/apq/gnss
$(RFS_APQ_GNSS_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS APQ GNSS symlinks"
	@mkdir -p $@/readonly/vendor
	$(hide) ln -sf /mnt/fwmt $@/readonly/firmware
	$(hide) ln -sf /vendor/firmware $@/readonly/vendor/firmware
	$(hide) ln -sf /mnt/per/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /data/vendor/tombstones/rfs/modem $@/ramdumps
	$(hide) ln -sf /mnt/per/rfs/apq/gnss $@/readwrite
	$(hide) ln -sf /mnt/per/rfs/shared $@/shared

RFS_MDM_ADSP_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/mdm/adsp
$(RFS_MDM_ADSP_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MDM ADSP symlinks"
	@mkdir -p $@/readonly/vendor
	$(hide) ln -sf /mnt/fwmt $@/readonly/firmware
	$(hide) ln -sf /vendor/firmware $@/readonly/vendor/firmware
	$(hide) ln -sf /mnt/per/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /data/vendor/tombstones/rfs/lpass $@/ramdumps
	$(hide) ln -sf /mnt/per/rfs/mdm/adsp $@/readwrite
	$(hide) ln -sf /mnt/per/rfs/shared $@/shared

RFS_MDM_MPSS_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/mdm/mpss
$(RFS_MDM_MPSS_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MDM MPSS symlinks"
	@mkdir -p $@/readonly/vendor
	$(hide) ln -sf /mnt/fwmt $@/readonly/firmware
	$(hide) ln -sf /vendor/firmware $@/readonly/vendor/firmware
	$(hide) ln -sf /mnt/per/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /data/vendor/tombstones/rfs/modem $@/ramdumps
	$(hide) ln -sf /mnt/per/rfs/mdm/mpss $@/readwrite
	$(hide) ln -sf /mnt/per/rfs/shared $@/shared

RFS_MDM_SLPI_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/mdm/slpi
$(RFS_MDM_SLPI_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MDM SLPI symlinks"
	@mkdir -p $@/readonly
	$(hide) ln -sf /mnt/fwmt $@/readonly/firmware
	$(hide) ln -sf /mnt/per/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /data/vendor/tombstones/rfs/slpi $@/ramdumps
	$(hide) ln -sf /mnt/per/rfs/mdm/slpi $@/readwrite
	$(hide) ln -sf /mnt/per/rfs/shared $@/shared

RFS_MDM_TN_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/mdm/tn
$(RFS_MDM_TN_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MDM TN symlinks"
	@mkdir -p $@/readonly
	$(hide) ln -sf /mnt/fwmt $@/readonly/firmware
	$(hide) ln -sf /mnt/per/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /data/vendor/tombstones/rfs/tn $@/ramdumps
	$(hide) ln -sf /mnt/per/rfs/mdm/tn $@/readwrite
	$(hide) ln -sf /mnt/per/rfs/shared $@/shared

RFS_MSM_ADSP_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/msm/adsp
$(RFS_MSM_ADSP_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MSM ADSP symlinks"
	@mkdir -p $@/readonly/vendor
	$(hide) ln -sf /mnt/fwmt $@/readonly/firmware
	$(hide) ln -sf /vendor/firmware $@/readonly/vendor/firmware
	$(hide) ln -sf /mnt/per/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /data/vendor/tombstones/rfs/lpass $@/ramdumps
	$(hide) ln -sf /mnt/per/rfs/msm/adsp $@/readwrite
	$(hide) ln -sf /mnt/per/rfs/shared $@/shared

RFS_MSM_MPSS_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/msm/mpss
$(RFS_MSM_MPSS_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MSM MPSS symlinks"
	@mkdir -p $@/readonly/vendor
	$(hide) ln -sf /mnt/fwmt $@/readonly/firmware
	$(hide) ln -sf /vendor/firmware $@/readonly/vendor/firmware
	$(hide) ln -sf /mnt/per/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /data/vendor/tombstones/rfs/modem $@/ramdumps
	$(hide) ln -sf /mnt/per/rfs/msm/mpss $@/readwrite
	$(hide) ln -sf /mnt/per/rfs/shared $@/shared

RFS_MSM_SLPI_SYMLINKS := $(TARGET_OUT_VENDOR)/rfs/msm/slpi
$(RFS_MSM_SLPI_SYMLINKS): $(LOCAL_INSTALLED_MODULE)
	@echo "Creating RFS MSM SLPI symlinks"
	@mkdir -p $@/readonly
	$(hide) ln -sf /mnt/fwmt $@/readonly/firmware
	$(hide) ln -sf /mnt/per/hlos_rfs/shared $@/hlos
	$(hide) ln -sf /data/vendor/tombstones/rfs/slpi $@/ramdumps
	$(hide) ln -sf /mnt/per/rfs/msm/slpi $@/readwrite
	$(hide) ln -sf /mnt/per/rfs/shared $@/shared

ALL_DEFAULT_INSTALLED_MODULES += \
	$(WCNSS_CFG_INI) $(PERSIST_DICT) \
	$(PERSIST_NV) \
	$(RFS_APQ_GNSS_SYMLINKS) \
	$(RFS_MDM_ADSP_SYMLINKS) \
	$(RFS_MDM_MPSS_SYMLINKS) \
	$(RFS_MDM_SLPI_SYMLINKS) \
	$(RFS_MDM_TN_SYMLINKS) \
	$(RFS_MSM_ADSP_SYMLINKS) \
	$(RFS_MSM_MPSS_SYMLINKS) \
	$(RFS_MSM_SLPI_SYMLINKS)

endif

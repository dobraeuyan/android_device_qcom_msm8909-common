#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0

set -e

DEVICE_COMMON=msm8909-common
VENDOR_COMMON=qcom

MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "$MY_DIR" ]]; then MY_DIR="$PWD"; fi

LINEAGE_ROOT="$MY_DIR"/../../..

HELPER="$LINEAGE_ROOT"/vendor/lineage/build/tools/extract_utils.sh
if [ ! -f "$HELPER" ]; then
    echo "Unable to find helper script at $HELPER"
    exit 1
fi
. "$HELPER"

setup_vendor "$DEVICE_COMMON" "$VENDOR_COMMON" "$LINEAGE_ROOT" true false "msm8909"

write_headers "x104f x104f1 x104l x104x bamboo x103f a16"

write_makefiles "$MY_DIR"/proprietary-files.txt true

write_footers

# ---------------------------------------------------------------------------
export PRODUCTMK="$LINEAGE_ROOT"/vendor/"$VENDOR_COMMON"/"$DEVICE_COMMON"/msm8909-ril-vendor.mk
write_makefile_header "$PRODUCTMK"
write_makefiles "$MY_DIR"/proprietary-files-ril.txt true

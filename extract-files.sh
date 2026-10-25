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

function blob_fixup() {
    case "$1" in
        vendor/bin/btnvtool | \
        vendor/bin/tftp_server | \
        vendor/bin/time_daemon | \
        vendor/bin/wcnss_service | \
        vendor/bin/xtwifi-client | \
        vendor/lib/libbtnv.so | \
        vendor/lib/libdrmfs.so | \
        vendor/lib/libGPTEE_vendor.so | \
        vendor/lib/libmm-qdcm.so | \
        vendor/lib/libsdm-color.so | \
        vendor/lib/sensor_calibrate.so | \
        vendor/lib/sensors.native.so)
            sed -i 's|/persist|/mnt/per|g; s|/firmware|/mnt/fwmt|g' "$2"
            ;;

        vendor/etc/audio_effects.conf)
            sed -i '/#DOLBY_ENABLE/,/#DOLBY_END/d; /add by dongkai  for dolby ABH-1898/d' "$2"
            ;;

        vendor/etc/media_codecs_vendor_audio.xml)
            sed -i '/add by dongkai for dolby ABH-1898 start/,/add by dongkai for dolby ABH-1898 end/d' "$2"
            ;;

    esac
}

while [ "$1" != "" ]; do
    case $1 in
        -n | --no-cleanup )     CLEAN_VENDOR=false
                                ;;
        -s | --section )        shift
                                SECTION=$1
                                CLEAN_VENDOR=false
                                ;;
        * )                     SRC=$1
                                ;;
    esac
    shift
done

if [ -z "$SRC" ]; then
    SRC=adb
fi

setup_vendor "$DEVICE_COMMON" "$VENDOR_COMMON" "$LINEAGE_ROOT" true "$CLEAN_VENDOR" "msm8909"

extract "$MY_DIR"/proprietary-files.txt "$SRC" "$SECTION"
extract "$MY_DIR"/proprietary-files-ril.txt "$SRC" "$SECTION"

"$MY_DIR"/setup-makefiles.sh

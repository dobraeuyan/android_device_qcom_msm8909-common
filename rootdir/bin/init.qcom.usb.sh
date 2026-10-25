#!/vendor/bin/sh
# Copyright (c) 2012-2017, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#     * Neither the name of The Linux Foundation nor the names of its
#       contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
# ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

chown -h root.system /sys/devices/platform/msm_hsusb/gadget/wakeup
chmod -h 220 /sys/devices/platform/msm_hsusb/gadget/wakeup

if [ -f /sys/devices/soc0/hw_platform ]; then
    soc_hwplatform=`cat /sys/devices/soc0/hw_platform` 2> /dev/null
else
    soc_hwplatform=`cat /sys/devices/system/soc/soc0/hw_platform` 2> /dev/null
fi

usb_config=`getprop persist.sys.usb.config`
case "$usb_config" in
    "" | "mtp,adb" | "diag,adb" | "diag,serial_smd,rmnet_qti_bam,adb")
        setprop persist.sys.usb.config adb
        ;;
    *) ;;
esac

if [ -d /config/usb_gadget ]; then
    setprop sys.usb.rndis.func.name "rndis"
    setprop sys.usb.rmnet.func.name "rmnet_bam"
    setprop sys.usb.rmnet.inst.name "rmnet"
    setprop sys.usb.dpl.inst.name "dpl"

    setprop sys.usb.controller "msm_hsusb"

    serialnumber=`cat /config/usb_gadget/g1/strings/0x409/serialnumber` 2> /dev/null
    if [ "$serialnumber" == "" ]; then
        echo 1234567 > /config/usb_gadget/g1/strings/0x409/serialnumber
    fi

    persist_comp=`getprop persist.sys.usb.config`
    comp=`getprop sys.usb.config`
    if [ "$comp" != "$persist_comp" ]; then
        setprop sys.usb.config $persist_comp
    fi

    setprop sys.usb.configfs 1
else
    persist_comp=`getprop persist.sys.usb.config`
    comp=`getprop sys.usb.config`
    if [ "$comp" != "$persist_comp" ]; then
        setprop sys.usb.config $persist_comp
    fi
fi

diag_extra=`getprop persist.sys.usb.config.extra`
if [ "$diag_extra" == "" ]; then
    setprop persist.sys.usb.config.extra none
fi

setprop sys.usb.rps_mask 0

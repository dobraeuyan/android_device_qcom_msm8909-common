#! /vendor/bin/sh

target=`getprop ro.board.platform`
[ "$target" != "msm8909" ] && exit 0

write() {
    [ -e "$1" ] && echo "$2" > "$1"
}

set -- `grep MemTotal /proc/meminfo`
mem_kb=$2

if [ "$mem_kb" -le 1310720 ]; then
    zram_pct=60 swappiness=100
elif [ "$mem_kb" -le 1835008 ]; then
    zram_pct=45 swappiness=90
else
    zram_pct=35 swappiness=80
fi

write /proc/sys/kernel/sched_window_stats_policy 3
write /proc/sys/kernel/sched_ravg_hist_size 3

write /sys/module/msm_thermal/core_control/enabled 0

for cpu in /sys/devices/system/cpu/cpu[0-9]*; do
    write $cpu/online 1
done
for gov in /sys/devices/system/cpu/cpufreq/policy*/scaling_governor; do
    write $gov "schedutil"
done

write /sys/module/msm_thermal/core_control/enabled 1

hispeed_load=85
max_freq=`cat /sys/devices/system/cpu/cpufreq/policy0/cpuinfo_max_freq`

hispeed_freq=$max_freq
freq_floor=$((max_freq * hispeed_load / 100))
for f in `cat /sys/devices/system/cpu/cpufreq/policy0/scaling_available_frequencies`; do
    [ "$f" -ge "$freq_floor" ] && [ "$f" -lt "$hispeed_freq" ] && hispeed_freq=$f
done

for sched_dir in /sys/devices/system/cpu/cpufreq/schedutil \
                 /sys/devices/system/cpu/cpufreq/policy*/schedutil; do
    [ -d "$sched_dir" ] || continue
    write $sched_dir/up_rate_limit_us 0
    write $sched_dir/down_rate_limit_us 20000
    write $sched_dir/hispeed_load $hispeed_load
    write $sched_dir/hispeed_freq $hispeed_freq
    write $sched_dir/pl 1
done

write /dev/stune/top-app/schedtune.prefer_idle 1
write /dev/stune/top-app/schedtune.boost 0

write /sys/module/lpm_levels/parameters/sleep_disabled 0

if [ -d /sys/devices/system/cpu/cpu0/core_ctl ]; then
    eff_freq=$max_freq
    eff_floor=$((max_freq * 60 / 100))
    for f in `cat /sys/devices/system/cpu/cpufreq/policy0/scaling_available_frequencies`; do
        [ "$f" -ge "$eff_floor" ] && [ "$f" -lt "$eff_freq" ] && eff_freq=$f
    done
    echo $((eff_freq * 100 / max_freq)) $((eff_freq * 100 / max_freq)) \
         $((66 * 1000000 / max_freq)) $((55 * 1000000 / max_freq)) \
         > /sys/devices/system/cpu/cpu0/core_ctl/busy_up_thres
    write /sys/devices/system/cpu/cpu0/core_ctl/busy_down_thres $((33 * 1000000 / max_freq))
    write /sys/devices/system/cpu/cpu0/core_ctl/offline_delay_ms 100
fi

for dir in /sys/class/devfreq/*qcom,cpubw*; do
    write $dir/governor "bw_hwmon"
    write $dir/bw_hwmon/bw_step 60
    write $dir/bw_hwmon/guard_band_mbps 30
done
for dir in /sys/class/devfreq/*qcom,gpubw*; do
    write $dir/bw_hwmon/io_percent 40
    write $dir/bw_hwmon/bw_step 60
    write $dir/bw_hwmon/guard_band_mbps 30
done

write /sys/module/lowmemorykiller/parameters/enable_lmk 1
write /sys/module/lowmemorykiller/parameters/enable_adaptive_lmk 1
write /sys/module/lowmemorykiller/parameters/oom_reaper 1

if [ "$mem_kb" -le 1310720 ]; then
    minfree_now=`cat /sys/module/lowmemorykiller/parameters/minfree`
    write /sys/module/lowmemorykiller/parameters/minfree "3072,5120,8192,${minfree_now#*,*,*,}"
fi

adj_series=`cat /sys/module/lowmemorykiller/parameters/adj`
adj_1="${adj_series#*,}"
almk_adj="${adj_1%%,*}"
almk_adj=$(((almk_adj * 6) + 6))
write /sys/module/lowmemorykiller/parameters/adj_max_shift $almk_adj

write /sys/module/lowmemorykiller/parameters/vmpressure_file_min $((mem_kb / 4 * 22 / 100))

write /sys/module/vmpressure/parameters/allocstall_threshold 0

if [ "$(grep -c zram /proc/swaps)" -eq 0 ]; then
    if grep -q lz4 /sys/block/zram0/comp_algorithm 2>/dev/null; then
        write /sys/block/zram0/comp_algorithm lz4
    fi
    write /sys/block/zram0/disksize "$((mem_kb * zram_pct / 100))K"
    mkswap /dev/block/zram0
    swapon /dev/block/zram0 -p 32758
fi

write /proc/sys/vm/swappiness $swappiness

write /proc/sys/vm/vfs_cache_pressure 80
write /proc/sys/vm/dirty_expire_centisecs 1500
write /proc/sys/vm/dirty_writeback_centisecs 1500

setprop sys.post_boot.parsed 1

/*
 * Copyright (C) 2026 The LineageOS Project
 */

#define LOG_TAG "PowerHAL-msm8909"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <log/log.h>
#include <hardware/hardware.h>
#include <hardware/power.h>

#define NUM_CPUS 4

#define CORE_CTL_MIN_CPUS  "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
#define CPU_MIN_FREQ_FMT   "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq"
#define CPUINFO_MIN_FREQ   "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq"
#define CPUINFO_MAX_FREQ   "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq"

#define TOP_APP_STUNE      "/dev/stune/top-app/schedtune.boost"

#ifndef TAP_TO_WAKE_NODE
#define TAP_TO_WAKE_NODE   "/sys/devices/virtual/gesture_mode/tp_gesture_status/gesture"
#endif

#define INTERACTION_MS    900
#define INTERACTION_FREQ  998400
#define INTERACTION_CPUS  2
#define INTERACTION_STUNE 10

#define LAUNCH_MS         2000
#define LAUNCH_CPUS       4
#define LAUNCH_STUNE      30

static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  s_cond;
static pthread_t       s_worker;

static bool s_boost_active = false;
static long s_expiry_ms    = 0;
static int  s_req_freq     = 0;
static int  s_req_cpus     = 1;
static int  s_req_stune    = 0;

static int  s_base_freq    = 200000;
static int  s_base_cpus    = 1;
static int  s_base_stune   = 0;
static int  s_max_freq     = 1267200;

static long now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000L + ts.tv_nsec / 1000000L;
}

static void sysfs_write(const char *path, const char *val) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        ALOGV("open %s failed: %s", path, strerror(errno));
        return;
    }
    if (write(fd, val, strlen(val)) < 0)
        ALOGV("write %s=%s failed: %s", path, val, strerror(errno));
    close(fd);
}

static int sysfs_read_int(const char *path) {
    char buf[32];
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return -1;
    int n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0)
        return -1;
    buf[n] = '\0';
    return atoi(buf);
}

static void apply_boost(int freq, int min_cpus, int stune) {
    char v[16];
    int i;

    snprintf(v, sizeof(v), "%d", freq);
    for (i = 0; i < NUM_CPUS; i++) {
        char p[96];
        snprintf(p, sizeof(p), CPU_MIN_FREQ_FMT, i);
        sysfs_write(p, v);
    }

    snprintf(v, sizeof(v), "%d", min_cpus);
    sysfs_write(CORE_CTL_MIN_CPUS, v);

    snprintf(v, sizeof(v), "%d", stune);
    sysfs_write(TOP_APP_STUNE, v);
}

static void restore_baseline(void) {
    char v[16];
    int i;

    snprintf(v, sizeof(v), "%d", s_base_freq);
    for (i = 0; i < NUM_CPUS; i++) {
        char p[96];
        snprintf(p, sizeof(p), CPU_MIN_FREQ_FMT, i);
        sysfs_write(p, v);
    }

    snprintf(v, sizeof(v), "%d", s_base_cpus);
    sysfs_write(CORE_CTL_MIN_CPUS, v);

    snprintf(v, sizeof(v), "%d", s_base_stune);
    sysfs_write(TOP_APP_STUNE, v);

    s_req_freq  = 0;
    s_req_cpus  = s_base_cpus;
    s_req_stune = s_base_stune;
}

static void *boost_thread(void *unused) {
    (void)unused;

    pthread_mutex_lock(&s_lock);
    for (;;) {
        if (!s_boost_active) {
            pthread_cond_wait(&s_cond, &s_lock);
            continue;
        }

        long now = now_ms();
        if (now >= s_expiry_ms) {
            restore_baseline();
            s_boost_active = false;
            ALOGV("boost window ended, baseline restored");
            continue;
        }

        apply_boost(s_req_freq, s_req_cpus, s_req_stune);

        long wait_ms = s_expiry_ms - now;
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        ts.tv_sec  += wait_ms / 1000;
        ts.tv_nsec += (wait_ms % 1000) * 1000000L;
        if (ts.tv_nsec >= 1000000000L) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000L;
        }

        pthread_cond_timedwait(&s_cond, &s_lock, &ts);
    }
    pthread_mutex_unlock(&s_lock);
    return NULL;
}

static void request_boost(int freq, int min_cpus, int stune, int duration_ms) {
    pthread_mutex_lock(&s_lock);

    long expiry = now_ms() + duration_ms;
    if (!s_boost_active || expiry > s_expiry_ms)
        s_expiry_ms = expiry;

    if (freq > s_req_freq)
        s_req_freq = freq;
    if (min_cpus > s_req_cpus)
        s_req_cpus = min_cpus;
    if (stune > s_req_stune)
        s_req_stune = stune;

    s_boost_active = true;
    pthread_cond_signal(&s_cond);
    pthread_mutex_unlock(&s_lock);
}

static void power_init(struct power_module *module) {
    (void)module;

    int v;
    if ((v = sysfs_read_int(CPUINFO_MIN_FREQ)) > 0)
        s_base_freq = v;
    if ((v = sysfs_read_int(CPUINFO_MAX_FREQ)) > 0)
        s_max_freq = v;
    if ((v = sysfs_read_int(CORE_CTL_MIN_CPUS)) >= 1)
        s_base_cpus = v;
    s_req_cpus = s_base_cpus;
    s_req_stune = s_base_stune;

    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&s_cond, &attr);
    pthread_condattr_destroy(&attr);

    pthread_create(&s_worker, NULL, boost_thread, NULL);

    ALOGI("msm8909 power HAL ready (floor=%d max=%d base_min_cpus=%d)",
          s_base_freq, s_max_freq, s_base_cpus);
}

static void power_set_interactive(struct power_module *module, int on) {
    (void)module;

    if (!on) {

        pthread_mutex_lock(&s_lock);
        s_expiry_ms = 0;
        s_boost_active = true;
        pthread_cond_signal(&s_cond);
        pthread_mutex_unlock(&s_lock);
        return;
    }

    request_boost(INTERACTION_FREQ, INTERACTION_CPUS, INTERACTION_STUNE, INTERACTION_MS);
}

static void power_hint(struct power_module *module, power_hint_t hint, void *data) {
    (void)module;

    switch (hint) {
    case POWER_HINT_INTERACTION: {
        int duration = (data != NULL) ? *((int *)data) : 0;
        if (duration <= 0)
            duration = INTERACTION_MS;
        request_boost(INTERACTION_FREQ, INTERACTION_CPUS, INTERACTION_STUNE, duration);
        break;
    }
    case POWER_HINT_LAUNCH: {
        int on = (data != NULL) ? *((int *)data) : 1;
        if (on)
            request_boost(s_max_freq, LAUNCH_CPUS, LAUNCH_STUNE, LAUNCH_MS);
        break;
    }
    default:
        break;
    }
}

static void power_set_feature(struct power_module *module, feature_t feature, int state) {
    (void)module;

    switch (feature) {
    case POWER_FEATURE_DOUBLE_TAP_TO_WAKE:
        sysfs_write(TAP_TO_WAKE_NODE, state ? "1" : "0");
        break;
    default:
        break;
    }
}

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_2,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "Snapdragon 210 Power HAL",
        .author = "The LineageOS Project",
        .methods = &power_module_methods,
    },
    .init = power_init,
    .setInteractive = power_set_interactive,
    .powerHint = power_hint,
    .setFeature = power_set_feature,
};

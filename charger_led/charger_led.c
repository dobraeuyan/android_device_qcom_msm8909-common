#include <errno.h>
#include <fcntl.h>
#include <linux/netlink.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BATTERY_STATUS_PATH "/sys/class/power_supply/battery/status"
#define BATTERY_CAPACITY_PATH "/sys/class/power_supply/battery/capacity"
#define LED_BRIGHTNESS_PATH "/sys/class/leds/white-notification/brightness"
#define LED_BLINK_PATH "/sys/class/leds/white-notification/blink"

enum led_mode {
    LED_MODE_UNKNOWN = -1,
    LED_MODE_OFF = 0,
    LED_MODE_BLINK = 1,
    LED_MODE_SOLID = 2,
};

static volatile sig_atomic_t g_running = 1;

static void handle_signal(int sig)
{
    (void)sig;
    g_running = 0;
}

static int read_str(const char *path, char *buf, size_t size)
{
    int fd;
    ssize_t n;

    if (size == 0) {
        return -EINVAL;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -errno;
    }

    n = read(fd, buf, size - 1);
    close(fd);
    if (n < 0) {
        return -errno;
    }

    while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r' || buf[n - 1] == ' ')) {
        n--;
    }
    buf[n] = '\0';
    return 0;
}

static int write_str(const char *path, const char *val)
{
    int fd;
    size_t len;
    ssize_t n;

    fd = open(path, O_WRONLY);
    if (fd < 0) {
        return -errno;
    }

    len = strlen(val);
    n = write(fd, val, len);
    close(fd);
    return n < 0 ? -errno : 0;
}

static enum led_mode compute_led_mode(void)
{
    char status[32];
    char cap_buf[16];
    int capacity = 0;

    if (read_str(BATTERY_STATUS_PATH, status, sizeof(status)) < 0) {
        return LED_MODE_OFF;
    }

    if (strcmp(status, "Full") == 0) {
        return LED_MODE_SOLID;
    }

    if (strcmp(status, "Charging") == 0) {
        if (read_str(BATTERY_CAPACITY_PATH, cap_buf, sizeof(cap_buf)) == 0) {
            capacity = atoi(cap_buf);
        }
        return capacity >= 90 ? LED_MODE_SOLID : LED_MODE_BLINK;
    }

    return LED_MODE_OFF;
}

static void apply_led_mode(enum led_mode mode, enum led_mode *last_mode)
{
    int rc = 0;

    if (mode == *last_mode) {
        return;
    }

    switch (mode) {
    case LED_MODE_BLINK:
        rc = write_str(LED_BLINK_PATH, "1000 1000 226\n");
        break;
    case LED_MODE_SOLID:
        rc = write_str(LED_BRIGHTNESS_PATH, "226\n");
        break;
    case LED_MODE_OFF:
    default:
        rc = write_str(LED_BRIGHTNESS_PATH, "0\n");
        break;
    }

    if (rc == 0) {
        *last_mode = mode;
    }
}

static bool is_power_supply_uevent(const char *buf, ssize_t len)
{
    const char *end = buf + len;
    const char *p = buf;

    while (p < end) {
        if (strcmp(p, "SUBSYSTEM=power_supply") == 0) {
            return true;
        }
        p += strlen(p) + 1;
    }
    return false;
}

static int open_uevent_socket(void)
{
    struct sockaddr_nl addr;
    int rcvbuf = 64 * 1024;
    int fd;

    fd = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_KOBJECT_UEVENT);
    if (fd < 0) {
        return -errno;
    }

    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = 0;
    addr.nl_groups = 1;

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        int err = -errno;
        close(fd);
        return err;
    }

    return fd;
}

int main(void)
{
    struct sigaction sa;
    struct pollfd pfd;
    enum led_mode last_mode = LED_MODE_UNKNOWN;
    char buf[4096];
    int uevent_fd;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    uevent_fd = open_uevent_socket();
    if (uevent_fd < 0) {
        return 1;
    }

    apply_led_mode(compute_led_mode(), &last_mode);

    pfd.fd = uevent_fd;
    pfd.events = POLLIN;

    while (g_running) {
        int ret = poll(&pfd, 1, -1);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        if (pfd.revents & POLLIN) {
            ssize_t len = recv(uevent_fd, buf, sizeof(buf) - 1, 0);
            if (len > 0) {
                buf[len] = '\0';
                if (is_power_supply_uevent(buf, len)) {
                    apply_led_mode(compute_led_mode(), &last_mode);
                }
            }
        }
    }

    close(uevent_fd);
    return 0;
}

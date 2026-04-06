#include "sysmon/collectors/smart/smart.h"

#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void sysmon_smart_copy_text(char *dst, size_t dst_size, const char *src) {
    size_t n;

    if (dst_size == 0) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    n = strnlen(src, dst_size - 1);
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static int sysmon_smart_is_candidate_device(const char *name) {
    if (name == NULL || name[0] == '\0' || name[0] == '.') {
        return 0;
    }

    if (strncmp(name, "loop", 4) == 0 ||
        strncmp(name, "ram", 3) == 0 ||
        strncmp(name, "dm-", 3) == 0 ||
        strncmp(name, "md", 2) == 0 ||
        strncmp(name, "zram", 4) == 0) {
        return 0;
    }

    return 1;
}

static int sysmon_smartctl_available(void) {
    return system("smartctl --version >/dev/null 2>&1") == 0;
}

static void sysmon_smart_query_device(const char *device_path, int *supported_out, char *health_out, size_t health_size) {
    char cmd[256];
    FILE *fp;
    char line[512];

    *supported_out = -1;
    sysmon_smart_copy_text(health_out, health_size, "unknown");

    (void)snprintf(cmd, sizeof(cmd), "smartctl -H -i %s 2>/dev/null", device_path);
    fp = popen(cmd, "r");
    if (fp == NULL) {
        sysmon_smart_copy_text(health_out, health_size, "query_error");
        return;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, "SMART support is:") != NULL) {
            if (strstr(line, "Available") != NULL || strstr(line, "Enabled") != NULL) {
                *supported_out = 1;
            } else if (strstr(line, "Unavailable") != NULL || strstr(line, "Disabled") != NULL) {
                *supported_out = 0;
            }
        }

        if (strstr(line, "SMART overall-health self-assessment test result:") != NULL) {
            if (strstr(line, "PASSED") != NULL) {
                sysmon_smart_copy_text(health_out, health_size, "passed");
            } else if (strstr(line, "FAILED") != NULL) {
                sysmon_smart_copy_text(health_out, health_size, "failed");
            }
        } else if (strstr(line, "SMART Health Status:") != NULL) {
            if (strstr(line, "OK") != NULL) {
                sysmon_smart_copy_text(health_out, health_size, "passed");
            } else if (strstr(line, "FAIL") != NULL) {
                sysmon_smart_copy_text(health_out, health_size, "failed");
            }
        }
    }

    (void)pclose(fp);
}

int sysmon_collect_smart(sysmon_snapshot_t *snapshot) {
    DIR *block_dir;
    struct dirent *entry;
    size_t count = 0;
    int has_smartctl = 0;

    if (snapshot == NULL) {
        return -1;
    }

    snapshot->smart_count = 0;

    block_dir = opendir("/sys/block");
    if (block_dir == NULL) {
        snapshot->sample_count++;
        return 0;
    }

    has_smartctl = sysmon_smartctl_available();

    while ((entry = readdir(block_dir)) != NULL) {
        char device_path[64];
        int supported = -1;
        char health[32];
        size_t prefix_len;

        if (!sysmon_smart_is_candidate_device(entry->d_name)) {
            continue;
        }

        sysmon_smart_copy_text(device_path, sizeof(device_path), "/dev/");
        prefix_len = strnlen(device_path, sizeof(device_path));
        if (prefix_len < sizeof(device_path)) {
            sysmon_smart_copy_text(device_path + prefix_len,
                                   sizeof(device_path) - prefix_len,
                                   entry->d_name);
        }
        sysmon_smart_copy_text(snapshot->smart_entries[count].device_path,
                               sizeof(snapshot->smart_entries[count].device_path),
                               device_path);

        if (has_smartctl) {
            sysmon_smart_query_device(device_path, &supported, health, sizeof(health));
        } else {
            supported = -1;
            sysmon_smart_copy_text(health, sizeof(health), "smartctl_missing");
        }

        snapshot->smart_entries[count].smart_supported = supported;
        sysmon_smart_copy_text(snapshot->smart_entries[count].smart_health,
                               sizeof(snapshot->smart_entries[count].smart_health),
                               health);

        count++;
        if (count >= SYSMON_SMART_MAX_ENTRIES) {
            break;
        }
    }

    closedir(block_dir);

    snapshot->smart_count = count;
    snapshot->sample_count++;
    return 0;
}

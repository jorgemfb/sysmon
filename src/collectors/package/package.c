#include "sysmon/collectors/package/package.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void sysmon_package_copy_text(char *dst, size_t dst_size, const char *src) {
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

static int sysmon_package_count_lines(const char *cmd, uint64_t *count_out) {
    FILE *fp;
    char line[512];
    uint64_t count = 0;

    if (cmd == NULL || count_out == NULL) {
        return -1;
    }

    fp = popen(cmd, "r");
    if (fp == NULL) {
        return -1;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (line[0] != '\0' && line[0] != '\n') {
            count++;
        }
    }

    (void)pclose(fp);
    *count_out = count;
    return 0;
}

static int sysmon_package_has_pacman(void) {
    return system("pacman --version >/dev/null 2>&1") == 0;
}

int sysmon_collect_package(sysmon_snapshot_t *snapshot) {
    uint64_t total_count = 0;
    uint64_t explicit_count = 0;

    if (snapshot == NULL) {
        return -1;
    }

    snapshot->package_total_installed_count = 0;
    snapshot->package_explicit_installed_count = 0;
    snapshot->package_source_available = 0;
    sysmon_package_copy_text(snapshot->package_manager, sizeof(snapshot->package_manager), "none");

    if (!sysmon_package_has_pacman()) {
        snapshot->sample_count++;
        return 0;
    }

    snapshot->package_source_available = 1;
    sysmon_package_copy_text(snapshot->package_manager, sizeof(snapshot->package_manager), "pacman");

    if (sysmon_package_count_lines("pacman -Qq 2>/dev/null", &total_count) == 0) {
        snapshot->package_total_installed_count = total_count;
    }

    if (sysmon_package_count_lines("pacman -Qeq 2>/dev/null", &explicit_count) == 0) {
        snapshot->package_explicit_installed_count = explicit_count;
    }

    if (snapshot->package_explicit_installed_count > snapshot->package_total_installed_count) {
        snapshot->package_explicit_installed_count = snapshot->package_total_installed_count;
    }

    snapshot->sample_count++;
    return 0;
}

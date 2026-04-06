#include "sysmon/collectors/memory/memory.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

int sysmon_collect_memory(sysmon_snapshot_t *snapshot) {
    FILE *fp;
    char line[256];
    uint64_t total_kib = 0;
    uint64_t available_kib = 0;
    uint64_t used_kib = 0;

    if (snapshot == NULL) {
        return -1;
    }

    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        return -2;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        unsigned long long value = 0ULL;

        if (sscanf(line, "MemTotal: %llu kB", &value) == 1) {
            total_kib = (uint64_t)value;
            continue;
        }

        if (sscanf(line, "MemAvailable: %llu kB", &value) == 1) {
            available_kib = (uint64_t)value;
            continue;
        }

        if (available_kib == 0 && sscanf(line, "MemFree: %llu kB", &value) == 1) {
            available_kib = (uint64_t)value;
        }
    }

    fclose(fp);

    if (total_kib == 0) {
        return -3;
    }

    if (available_kib > total_kib) {
        available_kib = total_kib;
    }

    used_kib = total_kib - available_kib;

    snapshot->memory_total_kib = total_kib;
    snapshot->memory_available_kib = available_kib;
    snapshot->memory_used_kib = used_kib;
    snapshot->sample_count++;

    return 0;
}

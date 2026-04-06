#include "sysmon/collectors/cpu/cpu.h"

#include <stddef.h>
#include <stdio.h>

typedef struct sysmon_cpu_times {
    uint64_t total;
    uint64_t idle;
} sysmon_cpu_times_t;

static int sysmon_cpu_read_times(sysmon_cpu_times_t *out) {
    FILE *fp;
    char line[512];
    unsigned long long values[10] = {0ULL};
    int scanned;
    size_t i;
    uint64_t total = 0;

    if (out == NULL) {
        return -1;
    }

    fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        return -2;
    }

    if (fgets(line, sizeof(line), fp) == NULL) {
        fclose(fp);
        return -3;
    }

    fclose(fp);

    scanned = sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                     &values[0], &values[1], &values[2], &values[3], &values[4],
                     &values[5], &values[6], &values[7], &values[8], &values[9]);
    if (scanned < 4) {
        return -4;
    }

    for (i = 0; i < (size_t)scanned; ++i) {
        total += (uint64_t)values[i];
    }

    out->total = total;
    out->idle = (uint64_t)values[3] + (scanned > 4 ? (uint64_t)values[4] : 0ULL);
    return 0;
}

int sysmon_collect_cpu(sysmon_snapshot_t *snapshot) {
    static int has_previous = 0;
    static sysmon_cpu_times_t previous = {0ULL, 0ULL};
    sysmon_cpu_times_t current;
    uint64_t delta_total;
    uint64_t delta_idle;
    double usage = 0.0;
    int rc;

    if (snapshot == NULL) {
        return -1;
    }

    rc = sysmon_cpu_read_times(&current);
    if (rc != 0) {
        return rc;
    }

    if (has_previous) {
        if (current.total >= previous.total && current.idle >= previous.idle) {
            delta_total = current.total - previous.total;
            delta_idle = current.idle - previous.idle;

            if (delta_total > 0) {
                if (delta_idle > delta_total) {
                    delta_idle = delta_total;
                }

                usage = ((double)(delta_total - delta_idle) * 100.0) / (double)delta_total;
                if (usage < 0.0) {
                    usage = 0.0;
                }
                if (usage > 100.0) {
                    usage = 100.0;
                }
            }
        }
    }

    snapshot->cpu_usage_percent = usage;
    snapshot->sample_count++;

    previous = current;
    has_previous = 1;
    return 0;
}

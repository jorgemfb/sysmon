#ifndef SYSMON_CORE_SNAPSHOT_H
#define SYSMON_CORE_SNAPSHOT_H

#include <stddef.h>
#include <stdint.h>

typedef struct sysmon_snapshot {
    char identity_hostname[256];
    char identity_username[256];
    char identity_kernel[256];
    uint64_t identity_uptime_seconds;
    size_t sample_count;
} sysmon_snapshot_t;

int sysmon_snapshot_init(sysmon_snapshot_t *snapshot);

#endif

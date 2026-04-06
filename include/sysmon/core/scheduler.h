#ifndef SYSMON_CORE_SCHEDULER_H
#define SYSMON_CORE_SCHEDULER_H

#include <stdbool.h>
#include <stdint.h>

#include "sysmon/core/registry.h"
#include "sysmon/core/snapshot.h"

typedef struct sysmon_scheduler {
    volatile bool running;
    uint64_t last_run_ms[SYSMON_REGISTRY_MAX_MODULES];
} sysmon_scheduler_t;

int sysmon_scheduler_init(sysmon_scheduler_t *scheduler);
int sysmon_scheduler_run_once(sysmon_scheduler_t *scheduler, sysmon_registry_t *registry, sysmon_snapshot_t *snapshot);

#endif
